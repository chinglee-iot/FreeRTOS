#include <stdint.h>
#include <stdlib.h>

#include "FreeRTOS.h"

#include "riscv-virt.h"

#define METAL_MSTATUS_MPP 0x00001800UL

#define __ASM_STR(x) #x

#define METAL_CPU_GET_CSR(reg, value)                                          \
    __asm__ volatile("csrr %0, " __ASM_STR(reg) : "=r"(value));

#define EXTRACT_FIELD(val, which) \
	(((val) & (which)) / ((which) & ~((which)-1)))
    
typedef enum {
    METAL_IAM_EXCEPTION_CODE = 0x0,     /* Instruction address misaligned */
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

/* RISC-V exception always store the context on the stack and restore the context
 * from the stack. Update the register a0 in the stack to return system call return
 * value to the task. */
extern uint32_t * pxCurrentTCB;

/* Flag to indicate memory access exception for demo code. */
volatile int xMemoryAcceccExceptionFlag = 0;

static void user_mode_trap_handler( int xSyscallNumber )
{
	int inst_len = 4;
	uintptr_t value;
    char temp[64];
    uint32_t * uxTaskStackPointer;

    #if ( configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY == 1 )
        extern uint32_t __syscalls_flash_start__[];
        extern uint32_t __syscalls_flash_end__[];
    #endif

    /* Get the mstatus from the stack. */
    uxTaskStackPointer = pxCurrentTCB[ 0 ];

    if( xSyscallNumber == portECALL_IS_PRIVILEGED )
    {
        /* Store the value in a0. TODO : follow ABI coding convention. */
        uxTaskStackPointer[ 7 ] = pdFALSE;
    }
    else if( xSyscallNumber == portECALL_RAISE_PRIORITY )
    {
        #if ( configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY == 1 )
        if( ( uxTaskStackPointer[ 0 ] >= ( uint32_t ) __syscalls_flash_start__ ) &&
            ( uxTaskStackPointer[ 0 ] <= ( uint32_t ) __syscalls_flash_end__ ) )
        #endif
        /* Update the mstatus.MPP on to stack to return to M-mode. */
        {
            uxTaskStackPointer[ 30 ] = uxTaskStackPointer[ 30 ] | ( 0x3 << 11 );
        }
    }
}

#define SHARED_MEMORY_SIZE      32
extern volatile uint8_t ucROTaskFaultTracker[ SHARED_MEMORY_SIZE ];
extern void vHandleMemoryFault( uint32_t * pulFaultStackAddress );
void freertos_risc_v_application_exception_handler( void )
{
    volatile int xSyscallNumber;
    uint32_t * uxTaskStackPointer;

    volatile uint32_t mCause, mEpc, mStatus;
    char temp[128];    

    uxTaskStackPointer = pxCurrentTCB[ 0 ];

    /* Get the a7 register from stack. */
    xSyscallNumber = uxTaskStackPointer[ 14 ];

    asm( "csrr %[varCause], mcause\n"
         "csrr %[varEpc], mepc\n"
         "csrr %[varStatus], mstatus\n":
         [varCause] "=r" (mCause), [varEpc] "=r" (mEpc), [varStatus] "=r" (mStatus)
         :: );

#if DEBUG
    vSendStringRaw( "Exception\r\n" );

    snprintf( temp, 128, "mcause 0x%08lx\r\n", mCause );
    vSendStringRaw( temp );

    snprintf( temp, 128, "mepc 0x%08lx\r\n", mEpc );
    vSendStringRaw( temp );

    snprintf( temp, 128, "mstatus 0x%08lx\r\n", mStatus );
    vSendStringRaw( temp );

    snprintf( temp, 128, "xSyscallNumber 0x%08lx\r\n", xSyscallNumber );
    vSendStringRaw( temp );
#endif

    switch( mCause )
    {
        case METAL_II_EXCEPTION_CODE:
            /* User mode tries to set the priority. */
            break;
        case METAL_LAF_EXCEPTION_CODE:
        case METAL_SAMOAF_EXCEPTION_CODE : /* memory access violation. */
            xMemoryAcceccExceptionFlag = 1;
            // vHandleMemoryFault( uxTaskStackPointer );
            if( ucROTaskFaultTracker[ 0 ] == 1 )
            {
                /* Mark the fault as handled. */
                ucROTaskFaultTracker[ 0 ] = 0;
            }
            break;
        case METAL_ECALL_U_EXCEPTION_CODE : /* ECALL from u-mode. */
            user_mode_trap_handler( xSyscallNumber );
            break;
        case METAL_ECALL_M_EXCEPTION_CODE : /* ECALL from m-mode. */
            /* Currently we only support portIS_PRIVILEGED from M-mode. */
            /* Set the a0 register to pdTRUE on stack. */
            uxTaskStackPointer[ 7 ] = pdTRUE;
            break;
        default:
            while( 1 );
    }
}
