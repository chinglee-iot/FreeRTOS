#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "riscv-virt.h"
#include "pmp_apis.h"

#define NAPOT_SIZE 4

static volatile int test_flag = 0;

static volatile uint32_t uxTestMemory[ 128 ];    /* Bigger than number of regions. */

void freertos_risc_v_application_exception_handler( void )
{
    test_flag = 1;
#if 0
    uint32_t mCause, mEpc, mStatus;
    char temp[128];

    vSendString( "Exception\r\n" );

    asm( "csrr %[varCause], mcause\n"
         "csrr %[varEpc], mepc\n"
         "csrr %[varStatus], mstatus\n":
         [varCause] "=r" (mCause), [varEpc] "=r" (mEpc), [varStatus] "=r" (mStatus)
         :: );

    snprintf( temp, 128, "mcause 0x%08lx\r\n", mCause );
    vSendString( temp );

    snprintf( temp, 128, "mepc 0x%08lx\r\n", mEpc );
    vSendString( temp );

    snprintf( temp, 128, "mstatus 0x%08lx\r\n", mStatus );
    vSendString( temp );

    switch( mCause )
    {
        #if 0
        case 0x09 : /* ECALL from s-mode. */
            supervisor_mode_trap_handler( mEpc, mCause );
            break;

        case 0x08 : /* ECALL from u-mode. */
            user_mode_trap_handler( mEpc, mCause);
            break;
        #endif
            
        default:
            break;
    }
#endif
}

static void prvListPmpRegions( void )
{
    char temp[ 128 ];
    int xPmpRegions;
    struct pmp_config xPmpConfig;
    size_t pmp_address;
    int i;

    /* List all the PMP region. */
    xPmpRegions = xPortPmpGetNumRegions();
    for( i = 0; i < xPmpRegions; i++ )
    {
        xPortPmpGetRegion( i, &xPmpConfig, &pmp_address );
        snprintf( temp, 128, "PMP %d : 0x%08x R %d W %d X %d L %d A %d", i,
            pmp_address,
            xPmpConfig.R,
            xPmpConfig.W, 
            xPmpConfig.X, 
            xPmpConfig.L, 
            xPmpConfig.A );
        vSendString( temp );
    }
}

static void prvPmpStage1DemoThread( void *pvParameters )
{
    char temp[ 128 ];
    int xPmpRegions;
    struct pmp_config xPmpConfig;
    size_t protected_addr;
    size_t pmp_address = 0x0;
    int i, j;

    /* List all the PMP regions. */
    vSendString( "========= List initial value =========== " );
    prvListPmpRegions();

    /* Set all the PMP regions. */
    vSendString( "========= Init all the PMP =========== " );
    xPmpRegions = xPortPmpGetNumRegions();
    for( i = 0; i < xPmpRegions; i++ )
    {
        xPmpConfig.R = 1;
        xPmpConfig.W = 1;
        xPmpConfig.X = 1;
        xPmpConfig.L = 0;   /* Lock bit can only be set once. */
        xPmpConfig.A = METAL_PMP_NAPOT;

        xPortPmpSetRegion( i, xPmpConfig, pmp_address );
    }

    /* List all the PMP regions. */
    prvListPmpRegions();

    /* clear all the PMP regions. */
    vSendString( "========= clear all the PMP =========== " );
    xPmpRegions = xPortPmpGetNumRegions();
    for( i = 0; i < xPmpRegions; i++ )
    {
        xPmpConfig.R = 0;
        xPmpConfig.W = 0;
        xPmpConfig.X = 0;
        xPmpConfig.L = 0;
        xPmpConfig.A = METAL_PMP_OFF;

        xPortPmpSetRegion( i, xPmpConfig, pmp_address );
    }
    /* List all the PMP regions. */
    prvListPmpRegions();

    /* Lock the memory address. */
    vSendString( "========= Access the memory =========== " );
    for( i = 0; i < xPmpRegions; i++ )
    {
        xPmpConfig.R = 1;
        xPmpConfig.W = 0;
        xPmpConfig.X = 0;
        xPmpConfig.L = 1;
        xPmpConfig.A = METAL_PMP_NA4;

        xPortPmpSetRegion( i, xPmpConfig, ( size_t )( &uxTestMemory[ i ]) >> 2 );
        prvListPmpRegions();
        for( j = 0; j < xPmpRegions; j++ )
        {
            test_flag = 0;

            /* Write to the memory region. */
            uxTestMemory[ j ] = 0;

            strncpy( temp, "0,0 is 0", 128 );
            temp[0] += i;
            temp[2] += j;
            temp[7] += test_flag;
            vSendString( temp );
        }
    }

    while( 1 )
    {
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        vSendString( "Demo thread\r\n" );
    }
}

int pmp_stage1_demo( void )
{
	vSendString( "Hello FreeRTOS!" );

    xTaskCreate( prvPmpStage1DemoThread,
                "PMP_DEMO_1",
                configMINIMAL_STACK_SIZE * 2U,
                NULL,
                tskIDLE_PRIORITY + 1,
                NULL );

	vTaskStartScheduler();

    return 0;
}
