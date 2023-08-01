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
    char temp[128];

    /* This is a hack set the pmp for stack before drop to user mode. */
    struct pmp_config xPmpConfig;
    size_t uxTaskStackPointer;

    uxTaskStackPointer = pxCurrentTCB->pxDummy6;

    xPmpConfig.R = 0;
    xPmpConfig.W = 0;
    xPmpConfig.L = METAL_PMP_UNLOCKED;
    xPmpConfig.X = 0;
    xPmpConfig.A = METAL_PMP_OFF;

    /* Stack base address. */
    xPortPmpSetRegion( 0, xPmpConfig, uxTaskStackPointer >> 2 );

    xPmpConfig.R = 1;
    xPmpConfig.W = 1;
    xPmpConfig.L = METAL_PMP_UNLOCKED;
    xPmpConfig.X = 0;
    xPmpConfig.A = METAL_PMP_TOR;
    xPortPmpSetRegion( 1, xPmpConfig, ( uxTaskStackPointer + 2048 * sizeof( StackType_t ) ) >> 2 );

    vSendStringRaw( "=========== check machine mode ===========\r\n" );
    /* Check the current mode. */
    if( portIS_PRIVILEGED() == pdFALSE )
    {
        vSendStringRaw( "Task mode is not machine mode\r\n" );
    }

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
    portSWITCH_TO_USER_MODE();

    /* Check the current mode. */
    if( portIS_PRIVILEGED() == pdTRUE )
    {
        vSendStringRaw( "portSWITCH_TO_USER_MODE failed\r\n" );
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

    while( 1 );
}

int pmp_stage2_demo( void )
{
	vSendString( "Hello FreeRTOS!" );

    xTaskCreate( prvPmpstage2DemoThread,
                 "PMP_DEMO_2",
                 2048,
                 NULL,
                 tskIDLE_PRIORITY + 1,
                 NULL );

	vTaskStartScheduler();

    return 0;
}
