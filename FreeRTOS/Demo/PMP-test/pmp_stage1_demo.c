#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "riscv-virt.h"
#include "pmp_apis.h"

#define NAPOT_SIZE 4

#define METAL_MSTATUS_MPP 0x00001800UL

#define __ASM_STR(x) #x

#define METAL_CPU_GET_CSR(reg, value)                                          \
    __asm__ volatile("csrr %0, " __ASM_STR(reg) : "=r"(value));

#define EXTRACT_FIELD(val, which) \
	(((val) & (which)) / ((which) & ~((which)-1)))
    
typedef enum {
    METAL_IAM_EXCEPTION_CODE,     /* Instruction address misaligned */
    METAL_IAF_EXCEPTION_CODE,     /* Instruction access faultd */
    METAL_II_EXCEPTION_CODE,      /* Illegal instruction */
    METAL_BREAK_EXCEPTION_CODE,   /* Breakpoint */
    METAL_LAM_EXCEPTION_CODE,     /* Load address misaligned */
    METAL_LAF_EXCEPTION_CODE,     /* Load access fault */
    METAL_SAMOAM_EXCEPTION_CODE,  /* Store/AMO address misaligned */
    METAL_SAMOAF_EXCEPTION_CODE,  /* Store/AMO access fault */
    METAL_ECALL_U_EXCEPTION_CODE, /* Environment call from U-mode */
    METAL_R9_EXCEPTION_CODE,      /* Reserved */
    METAL_R10_EXCEPTION_CODE,     /* Reserved */
    METAL_ECALL_M_EXCEPTION_CODE, /* Environment call from M-mode */
    METAL_MAX_EXCEPTION_CODE,
} metal_exception_code_e;


int retTrap = 0;
int xSyscallNumber = 0;
extern uint32_t * pxCurrentTCB;

void user_mode_trap_handler( uintptr_t epc )
{
	int inst_len = 4;
	uintptr_t value;
    char temp[64];
    uint32_t * uxTaskStackPointer;

    vSendStringRaw("Caught ecall from user mode\n");

    if( xSyscallNumber == 0 )
    {
        /* Store the value in a0. TODO : follow ABI coding convention. */
        retTrap = pdFALSE;
    }
    else
    {
        METAL_CPU_GET_CSR(mstatus, value);
        value = EXTRACT_FIELD(value, METAL_MSTATUS_MPP);
        snprintf( temp, 32, "The mstatus.MPP is 0x%08x\n", value );
        vSendStringRaw( temp );

        /* Set the MPP to machine mode. TODO : RV32 and RV64 diff. */
        value = value | ( 0x3 << 11 );
        snprintf( temp, 32, "The new mstatus.MPP is 0x%08x\n", value );
        vSendStringRaw( temp );

        /* Get the mstatus from the stack. */
        uxTaskStackPointer = pxCurrentTCB[ 0 ];
        
        uxTaskStackPointer[ 30 ] = uxTaskStackPointer[ 30 ] | ( 0x3 << 11 );
    }
}

void freertos_risc_v_application_exception_handler( void )
{
    uint32_t mCause, mEpc, mStatus;
    char temp[128];
    
    vSendStringRaw( "Exception\r\n" );

    asm( "csrr %[varCause], mcause\n"
         "csrr %[varEpc], mepc\n"
         "csrr %[varStatus], mstatus\n":
         [varCause] "=r" (mCause), [varEpc] "=r" (mEpc), [varStatus] "=r" (mStatus)
         :: );

    snprintf( temp, 128, "mcause 0x%08lx\r\n", mCause );
    vSendStringRaw( temp );

    snprintf( temp, 128, "mepc 0x%08lx\r\n", mEpc );
    vSendStringRaw( temp );

    snprintf( temp, 128, "mstatus 0x%08lx\r\n", mStatus );
    vSendStringRaw( temp );

    switch( mCause )
    {
        case 0x07 : /* memory access violation. */
            break;
        case 0x08 : /* ECALL from u-mode. */
            user_mode_trap_handler( mEpc );
            break;
            
        default:
            while( 1 );
    }
}

static void prvPmpstage0DemoThread( void *pvParameters )
{
    struct pmp_config xPmpConfig;
    int i;
    int ret;
    int xPmpRegions;

	struct pmp_config config = {
		.L = METAL_PMP_UNLOCKED,
		.A = METAL_PMP_TOR,
		.X = 1,
		.W = 1,
		.R = 1,
	};

    xPmpRegions = xPortPmpGetNumRegions();
    for( i = 0; i < xPmpRegions; i++ )
    {
        xPmpConfig.R = 0;
        xPmpConfig.W = 0;
        xPmpConfig.X = 0;
        xPmpConfig.L = 0;
        xPmpConfig.A = METAL_PMP_OFF;

        xPortPmpSetRegion( i, xPmpConfig, 0x0 );
    }

    /* Set the PMP to all memory space for user mode. */
    /* ram (wxa) : ORIGIN = 0x80080000, LENGTH = 512K. */
    ret = xPortPmpSetRegion(1, config, 0x800e0000 >> 2);

    /* Set 0x800e0000 ~ 0x8010000 is non-writable for user. */
    // config.L = METAL_PMP_LOCKED;
    config.W = 0;
    config.R = 0;
    ret = xPortPmpSetRegion(2, config, 0x80100000 >> 2);
    
    vSendStringRaw( "=========== Access memory from machine mode ===========\r\n" );
    *( ( volatile int *)0x800e0000 ) = 0x12345678;

    /* Switch to user mode. */
    vSendStringRaw( "=========== portSWITCH_TO_USER_MODE ===========\r\n" );
    portSWITCH_TO_USER_MODE();

    /* Check the current mode. */
    if( portIS_PRIVILEGED() == pdTRUE )
    {
        vSendStringRaw( "portSWITCH_TO_USER_MODE failed\r\n" );
    }
    
    /* Switch back to machine mode. */
    vSendStringRaw( "=========== portRAISE_PRIVILEGE ===========\r\n" );
    portRAISE_PRIVILEGE();

    /* Check the current mode. */
    if( portIS_PRIVILEGED() == pdFALSE )
    {
        vSendStringRaw( "portRAISE_PRIVILEGE failed\r\n" );
    }

    /* Reset the privilege mode. */
    vSendStringRaw( "=========== portRESET_PRIVILEGE ===========\r\n" );
    portRESET_PRIVILEGE();

    /* Check the current mode. */
    if( portIS_PRIVILEGED() == pdTRUE )
    {
        vSendStringRaw( "portRESET_PRIVILEGE failed\r\n" );
    }
    
    vSendStringRaw( "=========== Access memory from user mode ===========\r\n" );
    /* Access the memory address. */
    /* ram (wxa) : ORIGIN = 0x80080000, LENGTH = 512K. */
    *( ( volatile int *)0x800e0000 ) = 0x12345678;

    vSendStringRaw( "=========== portRAISE_PRIVILEGE ===========\r\n" );
    portRAISE_PRIVILEGE();

    vSendStringRaw( "=========== test done ===========\r\n" );

    while( 1 )
    {
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        // vSendString( "Demo thread\r\n" );
    }
}

int pmp_stage1_demo( void )
{
	vSendString( "Hello FreeRTOS!" );

    xTaskCreate( prvPmpstage0DemoThread,
                "PMP_DEMO_1",
                configMINIMAL_STACK_SIZE * 2U,
                NULL,
                tskIDLE_PRIORITY + 1,
                NULL );

	vTaskStartScheduler();

    return 0;
}
