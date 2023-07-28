#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "riscv-virt.h"
#include "pmp_apis.h"

#define TEST_PMP_CONFIG 1

extern volatile int xMemoryAcceccExceptionFlag;

/* Hardcoded size 16 regions * 8 bytes. */
static volatile uint32_t uxTestMemory[ 128 ]  __attribute__ ((aligned (8)));    /* Bigger than number of regions. */

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

static void prvPmpstage0DemoThread( void *pvParameters )
{
    char temp[ 128 ];
    int xPmpRegions;
    struct pmp_config xPmpConfig;
    size_t protected_addr;
    size_t pmp_address = 0x0;
    int i, j;

    /* Init all the PMP regions. */
    vSendString( "========= Init all the PMP =========== " );
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


    #if TEST_PMP_CONFIG <= 1
    {
        vSendString( "========= Access the memory =========== " );

        #if TEST_PMP_CONFIG == 0
            vSendString( "Test with NA4 mode" );
        #elif TEST_PMP_CONFIG == 1
            vSendString( "Test with NAPOT mode" );
        #endif

        for( i = 0; i < xPmpRegions; i++ )
        {
            xPmpConfig.R = 1;
            xPmpConfig.W = 0;
            xPmpConfig.X = 0;
            xPmpConfig.L = 1;
            
            #if TEST_PMP_CONFIG == 0
                xPmpConfig.A = METAL_PMP_NA4;

                xPortPmpSetRegion( i, xPmpConfig, ( size_t )( &uxTestMemory[ i ]) >> 2 );
            #elif TEST_PMP_CONFIG == 1
                xPmpConfig.A = METAL_PMP_NAPOT;

                xPortPmpSetRegion( i, xPmpConfig, xConvertNAPOTSize( &uxTestMemory[ i * 2 ], 8 ) );
            #endif
            for( j = 0; j < xPmpRegions; j++ )
            {
                xMemoryAcceccExceptionFlag = 0;

                /* Write to the memory region. */
                #if TEST_PMP_CONFIG == 0
                    uxTestMemory[ j  ] = 0;
                #elif TEST_PMP_CONFIG == 1
                    uxTestMemory[ j * 2 ] = 0;
                #endif

                if( ( i >= j ) && ( xMemoryAcceccExceptionFlag != 1 ) )
                {
                    strncpy( temp, "Loop 00, Access 00 has exception 0", 128 );
                    temp[5] += i/10;
                    temp[6] += i%10;
                    temp[16] += j/10;
                    temp[17] += j%10;
                    temp[33] += xMemoryAcceccExceptionFlag;
                    vSendString( temp );

                    prvListPmpRegions();
                    while( 1 );
                }
            }
        }
    }
    #else
    {
        /* Overlapping test. */
        vSendString( "========= Overlapping the memory =========== " );

        xPmpConfig.R = 0;
        xPmpConfig.W = 0;
        xPmpConfig.X = 0;
        xPmpConfig.L = 1;
        xPmpConfig.A = METAL_PMP_NA4;

        /* Define a region without write permission. */
        vSendString( "Define region 0 without write permission and access should except.\r\n" );

        xPmpConfig.W = 0;
        xPortPmpSetRegion( 0, xPmpConfig, ( size_t )( &uxTestMemory[ 0 ]) >> 2 );

        /* Access the memory should generate exception. */
        xMemoryAcceccExceptionFlag = 0;
        uxTestMemory[ 0 ] = 0;
        if( xMemoryAcceccExceptionFlag != 1 )
        {
            vSendString( "!!!Access the memory should generate exception.\r\n" );
        }

        /* Define another region with write permission. */
        vSendString( "Define region 1 overlap region 0 with write permission and access should except.\r\n" );
        xPmpConfig.W = 1;
        xPortPmpSetRegion( 1, xPmpConfig, ( size_t )( &uxTestMemory[ 0 ]) >> 2 );

        /* Access the memory should still generate exception. */
        xMemoryAcceccExceptionFlag = 0;
        uxTestMemory[ 0 ] = 0;
        if( xMemoryAcceccExceptionFlag != 1 )
        {
            vSendString( "!!!Access the memory should generate exception.\r\n" );
        }

        /* Define another region with write permission. */
        vSendString( "Define region 2 with write permission and access should not except.\r\n" );
        xPmpConfig.W = 1;
        xPortPmpSetRegion( 2, xPmpConfig, ( size_t )( &uxTestMemory[ 1 ]) >> 2 );

        /* Access the memory should not generate exception. */
        xMemoryAcceccExceptionFlag = 0;
        uxTestMemory[ 1 ] = 0;
        if( xMemoryAcceccExceptionFlag == 1 )
        {
            vSendString( "!!!Access the memory should not generate exception.\r\n" );
        }

        /* Define another region without write permission. */
        vSendString( "Define region 3 overlap region 2 with write permission and access should not except.\r\n" );
        xPmpConfig.W = 0;
        xPortPmpSetRegion( 3, xPmpConfig, ( size_t )( &uxTestMemory[ 1 ]) >> 2 );

        /* Access the memory should not generate exception. */
        xMemoryAcceccExceptionFlag = 0;
        uxTestMemory[ 1 ] = 0;
        if( xMemoryAcceccExceptionFlag == 1 )
        {
            vSendString( "!!!Access the memory should not generate exception.\r\n" );
        }
    }
    #endif
    
    vSendString( "========= Test done =========== " );

    while( 1 )
    {
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}

int pmp_stage0_demo( void )
{
	vSendString( "Hello FreeRTOS!" );

    xTaskCreate( prvPmpstage0DemoThread,
                 "PMP_DEMO_0",
                 configMINIMAL_STACK_SIZE * 2U,
                 NULL,
                 tskIDLE_PRIORITY + 1,
                 NULL );

	vTaskStartScheduler();

    return 0;
}
