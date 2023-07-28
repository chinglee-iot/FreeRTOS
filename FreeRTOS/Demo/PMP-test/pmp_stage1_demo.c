#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "riscv-virt.h"
#include "pmp_apis.h"

extern volatile int xMemoryAcceccExceptionFlag;

static void prvPmpstage1DemoThread( void *pvParameters )
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
    xMemoryAcceccExceptionFlag = 0;
    *( ( volatile int *)0x800e0000 ) = 0x12345678;

    if( xMemoryAcceccExceptionFlag != 0 )
    {
        vSendStringRaw( "Memory access test from M-mode failed\r\n" );
    }

    /* Switch to user mode. */
    vSendStringRaw( "=========== portSWITCH_TO_USER_MODE ===========\r\n" );
    portSWITCH_TO_USER_MODE();

    /* Check the current mode. */
    if( portIS_PRIVILEGED() == pdTRUE )
    {
        vSendStringRaw( "portSWITCH_TO_USER_MODE failed\r\n" );
    }

    vSendStringRaw( "=========== Access memory from user mode ===========\r\n" );
    /* Access the memory address. */
    xMemoryAcceccExceptionFlag = 0;
    *( ( volatile int *)0x800e0000 ) = 0x12345678;

    if( xMemoryAcceccExceptionFlag == 0 )
    {
        vSendStringRaw( "Memory access test from -mode failed\r\n" );
    }
    
    /* Switch back to machine mode. */
    vSendStringRaw( "=========== portRAISE_PRIVILEGE ===========\r\n" );
    portRAISE_PRIVILEGE();

    /* Check the current mode. */
    if( portIS_PRIVILEGED() == pdFALSE )
    {
        vSendStringRaw( "portRAISE_PRIVILEGE failed\r\n" );
    }

    vSendStringRaw( "=========== Access memory from machine mode ===========\r\n" );
    xMemoryAcceccExceptionFlag = 0;
    *( ( volatile int *)0x800e0000 ) = 0x12345678;

    if( xMemoryAcceccExceptionFlag != 0 )
    {
        vSendStringRaw( "Memory access test from M-mode failed\r\n" );
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
    xMemoryAcceccExceptionFlag = 0;
    *( ( volatile int *)0x800e0000 ) = 0x12345678;

    if( xMemoryAcceccExceptionFlag == 0 )
    {
        vSendStringRaw( "Memory access test from -mode failed\r\n" );
    }

    vSendStringRaw( "=========== portRAISE_PRIVILEGE ===========\r\n" );
    /* Raise back the privilege level to call vTaskDelay. */
    portRAISE_PRIVILEGE();

    vSendStringRaw( "=========== Access memory from machine mode ===========\r\n" );
    xMemoryAcceccExceptionFlag = 0;
    *( ( volatile int *)0x800e0000 ) = 0x12345678;

    if( xMemoryAcceccExceptionFlag != 0 )
    {
        vSendStringRaw( "Memory access test from M-mode failed\r\n" );
    }

    vSendStringRaw( "=========== test done ===========\r\n" );

    while( 1 )
    {
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}

int pmp_stage1_demo( void )
{
	vSendString( "Hello FreeRTOS!" );

    xTaskCreate( prvPmpstage1DemoThread,
                 "PMP_DEMO_1",
                 configMINIMAL_STACK_SIZE * 2U,
                 NULL,
                 tskIDLE_PRIORITY + 1,
                 NULL );

	vTaskStartScheduler();

    return 0;
}
