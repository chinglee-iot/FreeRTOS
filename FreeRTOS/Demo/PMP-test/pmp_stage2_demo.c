#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "riscv-virt.h"
#include "pmp_apis.h"

extern volatile int xMemoryAcceccExceptionFlag;

/* Declaration when these variable are defined in code instead of being
 * exported from linker scripts. */
extern uint32_t __privileged_functions_start__[];
extern uint32_t __privileged_functions_end__[];
extern uint32_t __FLASH_segment_start__[];
extern uint32_t __FLASH_segment_end__[];
extern uint32_t __privileged_data_start__[];
extern uint32_t __privileged_data_end__[];

extern StaticTask_t * pxCurrentTCB;

static void prvPmpstage2DemoThread( void *pvParameters )
{
    volatile uint32_t testValue;
    BaseType_t xMachineModeDemo = pdFALSE;
    char temp[128];

    while( 1 )
    {
        if( portIS_PRIVILEGED() == pdTRUE )
        {
            xMachineModeDemo = pdTRUE;

            vSendStringRaw( "==========================================\r\n" );
            vSendStringRaw( "=========== check machine mode ===========\r\n" );
            vSendStringRaw( "==========================================\r\n" );
            /* Check the current mode. */

            vSendStringRaw( "=========== Read privileged function from machine mode ===========\r\n" );
            /* Access the memory address. */
            xMemoryAcceccExceptionFlag = 0;
            snprintf( temp, 128, "Access 0x%08x\r\n", __privileged_functions_start__ );
            vSendStringRaw( temp );
            testValue = *( ( volatile int *) __privileged_functions_start__ );
            if( xMemoryAcceccExceptionFlag == 1 )
            {
                vSendStringRaw( "Memory access test from m-mode failed\r\n" );
            }

            xMemoryAcceccExceptionFlag = 0;
            snprintf( temp, 128, "Access 0x%08x\r\n", ( __privileged_functions_end__ - sizeof( uint32_t ) ) );
            vSendStringRaw( temp );
            testValue = *( ( volatile int *)( __privileged_functions_end__ - sizeof( uint32_t ) ) );
            if( xMemoryAcceccExceptionFlag == 1 )
            {
                vSendStringRaw( "Memory access test from m-mode failed\r\n" );
            }

            vSendStringRaw( "=========== Read privileged data from machine mode ===========\r\n" );
            /* Access the memory address. */
            xMemoryAcceccExceptionFlag = 0;
            snprintf( temp, 128, "Access 0x%08x\r\n", __privileged_data_start__ );
            vSendStringRaw( temp );
            testValue = *( ( volatile int *) __privileged_data_start__ );
            if( xMemoryAcceccExceptionFlag == 1 )
            {
                vSendStringRaw( "Memory access test from m-mode failed\r\n" );
            }

            xMemoryAcceccExceptionFlag = 0;
            snprintf( temp, 128, "Access 0x%08x\r\n", ( __privileged_data_end__ - sizeof( uint32_t ) ) );
            vSendStringRaw( temp );
            testValue = *( ( volatile int *)( __privileged_data_end__ - sizeof( uint32_t ) ) );
            if( xMemoryAcceccExceptionFlag == 1 )
            {
                vSendStringRaw( "Memory access test from m-mode failed\r\n" );
            }

            vSendStringRaw( "=========== portSWITCH_TO_USER_MODE ===========\r\n" );

            /* Check the current mode. */
            if( portIS_PRIVILEGED() == pdTRUE )
            {
                portSWITCH_TO_USER_MODE();
            }
            else
            {
                vSendStringRaw( "=========== Task is user mode ===========\r\n" );
            }
        }
        else
        {
            vSendStringRaw( "==========================================\r\n" );
            vSendStringRaw( "=========== check user mode ==============\r\n" );
            vSendStringRaw( "==========================================\r\n" );
            vTaskDelay( pdMS_TO_TICKS( 5000 ) );
        }

        /* Check the current mode. */
        if( portIS_PRIVILEGED() == pdTRUE )
        {
            vSendStringRaw( "ERROR : Task is not in user mode\r\n" );
        }

        vSendStringRaw( "=========== Read privileged function from user mode ===========\r\n" );
        /* Access the memory address. */
        xMemoryAcceccExceptionFlag = 0;
        snprintf( temp, 128, "Access 0x%08x\r\n", __privileged_functions_start__ );
        vSendStringRaw( temp );
        testValue = *( ( volatile int *) __privileged_functions_start__ );
        if( xMemoryAcceccExceptionFlag == 0 )
        {
            vSendStringRaw( "Memory access test from u-mode failed\r\n" );
        }

        xMemoryAcceccExceptionFlag = 0;
        snprintf( temp, 128, "Access 0x%08x\r\n", ( __privileged_functions_end__ - sizeof( uint32_t ) ) );
        vSendStringRaw( temp );
        testValue = *( ( volatile int *)( __privileged_functions_end__ - sizeof( uint32_t ) ) );
        if( xMemoryAcceccExceptionFlag == 0 )
        {
            vSendStringRaw( "Memory access test from u-mode failed\r\n" );
        }

        vSendStringRaw( "=========== Read privileged data from user mode ===========\r\n" );
        /* Access the memory address. */
        xMemoryAcceccExceptionFlag = 0;
        snprintf( temp, 128, "Access 0x%08x\r\n", __privileged_data_start__ );
        vSendStringRaw( temp );
        testValue = *( ( volatile int *) __privileged_data_start__ );
        if( xMemoryAcceccExceptionFlag == 0 )
        {
            vSendStringRaw( "Memory access test from u-mode failed\r\n" );
        }

        xMemoryAcceccExceptionFlag = 0;
        snprintf( temp, 128, "Access 0x%08x\r\n", ( __privileged_data_end__ - sizeof( uint32_t ) ) );
        vSendStringRaw( temp );
        testValue = *( ( volatile int *)( __privileged_data_end__ - sizeof( uint32_t ) ) );
        if( xMemoryAcceccExceptionFlag == 0 )
        {
            vSendStringRaw( "Memory access test from u-mode failed\r\n" );
        }

        vSendStringRaw( "=========== test done ===========\r\n" );

        if( xMachineModeDemo == pdTRUE )
        {
            vSendStringRaw( "Return to machine mode\r\n" );
            portRAISE_PRIVILEGE();
        }

        vTaskDelay( pdMS_TO_TICKS( 10000 ) );
    }
}

StackType_t xTaskStack[ 2048 ];

static const TaskParameters_t xTaskDefinition =
{
    prvPmpstage2DemoThread,   /* pvTaskCode */
    "PMP_DEMO_2-2",
    2048,
    NULL,            /* pvParameters */
    tskIDLE_PRIORITY + 1,
    xTaskStack,      /* puxStackBuffer - the array to use as the task stack. */

    /* xRegions - In this case only one of the three user definable regions is
    actually used.  The parameters are used to set the region to read only. */
    {
        /* Base address   Length                    Parameters */
        { 0, 0, 0 }
    }
};

int pmp_stage2_demo( void )
{
	vSendString( "Hello FreeRTOS!" );

    xTaskCreate( prvPmpstage2DemoThread,
                 "PMP_DEMO_2-1",
                 2048,
                 NULL,
                 ( tskIDLE_PRIORITY + 2 ) | portPRIVILEGE_BIT,
                 NULL ); 

    xTaskCreateRestricted( &xTaskDefinition, NULL );


	vTaskStartScheduler();

    return 0;
}
