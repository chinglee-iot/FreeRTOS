/*
 * FreeRTOS V202212.00
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/**
 * @file lock_contention_end_to_end.c
 * @brief End to end speed test of lock contention scenario
 *
 * This tests recreates a common lock contention scenario under the BKL (i.e.
 * global locking) scheme. Each core has a producer task, consumer task, and a
 * queue.This is to simulate two independent stream of data being independently
 * processed in parallel on each core.
 *
 * The lower throughput under BKL due to lock contention between the multiple
 * cores should be demonstrated by this test.
 *
 * Procedure:
 *   - For each core
 *      - Create a queue
 *      - Create a producer task that sends data to the queue
 *      - Create a consumer task that reads data from the queue
 *   - Repeat for testNUM_SAMPLES iterations
 *      - Start the producer and consumer tasks
 *      - Measure time elapsed for producer/consumer task to send/receive
 *        testNUM_ITEMS to/from the queue
 */

/*-----------------------------------------------------------*/
#include  <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "test_macro_definitions.h"

/*-----------------------------------------------------------*/

#ifndef TEST_CONFIG_H
    #error test_config.h must be included at the end of FreeRTOSConfig.h.
#endif

#if ( configNUMBER_OF_CORES < 2 )
    #error This test is for FreeRTOS SMP and therefore, requires at least 2 cores.
#endif /* if configNUMBER_OF_CORES != 2 */

#if ( configRUN_MULTIPLE_PRIORITIES != 1 )
    #error configRUN_MULTIPLE_PRIORITIES must be set to 1 for this test.
#endif /* if ( configRUN_MULTIPLE_PRIORITIES != 1 ) */

/*-----------------------------------------------------------*/

/* Vary testNUMBER_OF_TASK_PAIRS from 1 to configNUMBER_OF_CORES to demonstrate
 * granular lock isolation. */
#ifndef testNUMBER_OF_TASK_PAIRS
    #define testNUMBER_OF_TASK_PAIRS    ( configNUMBER_OF_CORES )
#endif

#ifndef testGET_TIME_FUNCTION
    #error testGET_TIME_FUNCTION must be defined to run the test.
#endif

#ifndef testPRINTF_FUNCTION
    #error testPRINTF_FUNCTION must be defined to display test results.
#endif

/* Compare testSHARED_QUEUE set to 0 vs. 1 to demonstrate granular lock parallelism. */
#ifndef testSHARED_QUEUE
    #define testSHARED_QUEUE      ( 0 )
#endif

/*-----------------------------------------------------------*/

#define testNUM_SAMPLES           ( 128U )
#define testNUM_ITEMS             ( 256U )

/* Dedicated bit for the main task to participate in the rendezvous, distinct
 * from the per-task-pair bits ( 1 << testIndex ) used by the producer/consumer
 * pairs. */
#define TEST_MAIN_TASK_BIT        ( 1 << ( configNUMBER_OF_CORES ) )

/* All bits that must be set for a rendezvous to release: one per task pair,
 * plus the main task's bit. */
#define TEST_ALL_SYNC_BITS        ( ( ( 1 << testNUMBER_OF_TASK_PAIRS ) - 1 ) | TEST_MAIN_TASK_BIT )

#define TEST_PRODUCER_PRIORITY    ( configMAX_PRIORITIES - 4 )
#define TEST_CONSUMER_PRIORITY    ( configMAX_PRIORITIES - 5 )

/*-----------------------------------------------------------*/

/**
 * @brief Test case "Lock Contention"
 */
static void Test_LockContentionEndToEnd( void );
/*-----------------------------------------------------------*/

/**
 * @brief Producer task run on each core
 */
static void prvProducerTask( void * pvParameters );
/*-----------------------------------------------------------*/

/**
 * @brief Consumer task run on each core
 */
static void prvConsumerTask( void * pvParameters );
/*-----------------------------------------------------------*/

/**
 * @brief Handles of the queues accessed by each core
 */
static QueueHandle_t xQueueHandles[ testNUMBER_OF_TASK_PAIRS ];
/*-----------------------------------------------------------*/

/**
 * @brief Handles of the producer tasks created for each core
 */
static TaskHandle_t xProducerTaskHandles[ testNUMBER_OF_TASK_PAIRS ];
/*-----------------------------------------------------------*/

/**
 * @brief Handles of the consumer tasks created for each core
 */
static TaskHandle_t xConsumerTaskHandles[ testNUMBER_OF_TASK_PAIRS ];
/*-----------------------------------------------------------*/

/**
 * @brief Start times of each iteration for each core
 */
static UBaseType_t uxStartTimes[ testNUMBER_OF_TASK_PAIRS ];
/*-----------------------------------------------------------*/

/**
 * @brief Cumulative elapsed time of all iterations for each core
 */
static UBaseType_t uxElapsedCumulative[ testNUMBER_OF_TASK_PAIRS ];
/*-----------------------------------------------------------*/

/**
 * @brief Event group used to rendezvous the main task with all consumer
 * tasks at the start and end of each iteration
 */
static EventGroupHandle_t xSyncEventGroup;

/*-----------------------------------------------------------*/

static void prvProducerTask( void * pvParameters )
{
    int iSamples;
    int iItems;
    BaseType_t testIndex = ( BaseType_t ) ( pvParameters );

    #if ( testSHARED_QUEUE == 1 )
        BaseType_t testQueueIndex = 0;
    #else
        BaseType_t testQueueIndex = testIndex;
    #endif

    for( iSamples = 0; iSamples < testNUM_SAMPLES; iSamples++ )
    {
        /* Wait to be started by consumer task */
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

        /* Record the start time for this sample */
        uxStartTimes[ testIndex ] = testGET_TIME_FUNCTION();

        for( iItems = 0; iItems < testNUM_ITEMS; iItems++ )
        {
            BaseType_t xReturn;
            xReturn = xQueueSend( xQueueHandles[ testQueueIndex ], &iItems, portMAX_DELAY );
            TEST_ASSERT_EQUAL_MESSAGE( pdTRUE, xReturn, "xQueueSend() failed" );
        }
    }

    /* Wait to be deleted */
    vTaskSuspend( NULL );
}
/*-----------------------------------------------------------*/

static void prvConsumerTask( void * pvParameters )
{
    int iSamples;
    int iItems;
    BaseType_t testIndex = ( BaseType_t ) ( pvParameters );

    #if ( testSHARED_QUEUE == 1 )
        BaseType_t testQueueIndex = 0;
    #else
        BaseType_t testQueueIndex = testIndex;
    #endif

    for( iSamples = 0; iSamples < testNUM_SAMPLES; iSamples++ )
    {
        /* Rendezvous with the main task and the other consumer tasks before
         * starting this iteration */
        ( void ) xEventGroupSync( xSyncEventGroup,
                                  ( 1U << ( testIndex ) ), /* Bits to set */
                                  TEST_ALL_SYNC_BITS,      /* Bits to wait for */
                                  portMAX_DELAY );         /* Wait indefinitely */

        /* Start the producer task */
        xTaskNotifyGive( xProducerTaskHandles[ testIndex ] );

        /* Consume queue messages sent by producer tasks */
        for( iItems = 0; iItems < testNUM_ITEMS; iItems++ )
        {
            int Temp;
            BaseType_t xReturn;

            xReturn = xQueueReceive( xQueueHandles[ testQueueIndex ], &Temp, portMAX_DELAY );
            TEST_ASSERT_EQUAL_MESSAGE( pdTRUE, xReturn, "xQueueReceive() failed" );
            #if ( testSHARED_QUEUE == 0 )
                TEST_ASSERT_EQUAL_MESSAGE( iItems, Temp, "Received incorrect value" );
            #endif
        }

        /* Record end time for this iteration and add it to the cumulative count */
        uxElapsedCumulative[ testIndex ] += testGET_TIME_FUNCTION() - uxStartTimes[ testIndex ];

        /* Rendezvous with the main task and the other consumer tasks at the
         * end of this iteration */
        ( void ) xEventGroupSync( xSyncEventGroup,
                                  ( 1U << ( testIndex ) ), /* Bits to set */
                                  TEST_ALL_SYNC_BITS,      /* Bits to wait for */
                                  portMAX_DELAY );         /* Wait indefinitely */
    }

    /* Wait to be deleted */
    vTaskSuspend( NULL );
}
/*-----------------------------------------------------------*/

static void Test_LockContentionEndToEnd( void )
{
    int iCore;
    int iIter;

    /* Run test for testNUM_SAMPLES number of iterations */
    for( iIter = 0; iIter < testNUM_SAMPLES; iIter++ )
    {
        /* Rendezvous with all consumer tasks before they start this iteration */
        ( void ) xEventGroupSync( xSyncEventGroup,
                                  TEST_MAIN_TASK_BIT, /* Bits to set */
                                  TEST_ALL_SYNC_BITS, /* Bits to wait for */
                                  portMAX_DELAY );    /* Wait indefinitely */

        /* Rendezvous with all consumer tasks once they complete this iteration */
        ( void ) xEventGroupSync( xSyncEventGroup,
                                  TEST_MAIN_TASK_BIT, /* Bits to set */
                                  TEST_ALL_SYNC_BITS, /* Bits to wait for */
                                  portMAX_DELAY );    /* Wait indefinitely */
    }

    /* Print average results */
    testPRINTF_FUNCTION( "Time taken to send %d items, averaged over %d samples\n", testNUM_ITEMS, testNUM_SAMPLES );

    for( iCore = 0; iCore < testNUMBER_OF_TASK_PAIRS; iCore++ )
    {
        testPRINTF_FUNCTION( "Core %d: %d\n", iCore, ( int ) ( uxElapsedCumulative[ iCore ] / testNUM_SAMPLES ) );
    }
}
/*-----------------------------------------------------------*/

/* Runs before every test, put init calls here. */
testSETUP_FUNCTION_PROTOTYPE( setUp )
{
    int i;
    BaseType_t xRet;

    /* Create event group. */
    xSyncEventGroup = xEventGroupCreate();
    TEST_ASSERT_NOT_NULL_MESSAGE( xSyncEventGroup, "Failed to create event group" );

    /* Create one queue for all the tasks. */
    #if ( testSHARED_QUEUE == 1 )
        xQueueHandles[ 0 ] = xQueueCreate( testNUM_ITEMS * testNUMBER_OF_TASK_PAIRS, sizeof( int ) );
    #endif

    /* Create separate queues and tasks for each core */
    for( i = 0; i < testNUMBER_OF_TASK_PAIRS; i++ )
    {
        /* Reset eplased cumulative time. */
        uxElapsedCumulative[ i ] = 0;

        /* Separate queues for each core */
        #if ( testSHARED_QUEUE == 0 )
            xQueueHandles[ i ] = xQueueCreate( testNUM_ITEMS, sizeof( int ) );
            TEST_ASSERT_NOT_NULL_MESSAGE( xQueueHandles[ i ], "Queue creation failed" );
        #endif

        /* A producer task sends to a dedicated queue. */
        xRet = xTaskCreate( prvProducerTask,
                            "prod",
                            configMINIMAL_STACK_SIZE * 4,
                            ( void * ) ( i ),
                            TEST_PRODUCER_PRIORITY,
                            &xProducerTaskHandles[ i ] );
        TEST_ASSERT_EQUAL_MESSAGE( pdTRUE, xRet, "Creating producer task failed" );

        /* A consumder task receives from a dedicated queue. */
        xRet = xTaskCreate( prvConsumerTask,
                            "con",
                            configMINIMAL_STACK_SIZE * 4,
                            ( void * ) ( i ),
                            TEST_CONSUMER_PRIORITY,
                            &xConsumerTaskHandles[ i ] );
        TEST_ASSERT_EQUAL_MESSAGE( pdTRUE, xRet, "Creating consumer task failed" );
    }
}
/*-----------------------------------------------------------*/

/* Runs after every test, put clean-up calls here. */
testTEARDOWN_FUNCTION_PROTOTYPE( tearDown )
{
    int i;

    #if ( testSHARED_QUEUE == 1 )
        vQueueDelete( xQueueHandles[ 0 ] );
        xQueueHandles[ 0 ] = NULL;
    #endif

    /* Delete tasks and queues */
    for( i = 0; i < testNUMBER_OF_TASK_PAIRS; i++ )
    {
        vTaskDelete( xProducerTaskHandles[ i ] );
        vTaskDelete( xConsumerTaskHandles[ i ] );
        #if ( testSHARED_QUEUE == 0 )
            vQueueDelete( xQueueHandles[ i ] );
            xQueueHandles[ i ] = NULL;
        #endif
    }
}
/*-----------------------------------------------------------*/

testENTRY_FUNCTION_PROTOTYPE( vRunLockContentionEndToEnd )
{
    testBEGIN_FUNCTION();

    testRUN_TEST_CASE_FUNCTION( Test_LockContentionEndToEnd );

    testEND_FUNCTION();
}
/*-----------------------------------------------------------*/
