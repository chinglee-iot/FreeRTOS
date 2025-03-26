/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
/*! @file single_priority_no_timeslice_utest.c */

/* C runtime includes. */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Task includes */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "event_groups.h"
#include "queue.h"

/* Test includes. */
#include "unity.h"
#include "unity_memory.h"
#include "../global_vars.h"
#include "../smp_utest_common.h"

/* Mock includes. */
#include "mock_timers.h"
#include "mock_fake_assert.h"
#include "mock_fake_port.h"
#include "mock_portmacro.h"

/* ===========================  GLOBAL VARIABLES  =========================== */

static TaskHandle_t xTaskHandles[ configNUMBER_OF_CORES ] = { NULL };
uint32_t xPortCriticalNestingCount[ configNUMBER_OF_CORES ] = { 0U };

/* ============================  Callback Functions  ============================ */
void vFakePortInitSpinlock_callback( portSPINLOCK_TYPE *pxSpinlock, int cmock_num_calls )
{
    TEST_ASSERT_NOT_EQUAL( NULL, pxSpinlock );

    pxSpinlock->uxLockCount = 0;
    pxSpinlock->xOwnerCore = -1;
}

void vFakePortReleaseSpinlock_callback( BaseType_t xCoreID, portSPINLOCK_TYPE *pxSpinlock, int cmock_num_calls )
{
    TEST_ASSERT_NOT_EQUAL( NULL, pxSpinlock );
    TEST_ASSERT_NOT_EQUAL( -1, pxSpinlock->xOwnerCore );
    TEST_ASSERT_NOT_EQUAL( 0, pxSpinlock->uxLockCount );
    TEST_ASSERT_EQUAL( xCoreID, pxSpinlock->xOwnerCore );

    pxSpinlock->uxLockCount = pxSpinlock->uxLockCount - 1U;
    if( pxSpinlock->uxLockCount == 0U )
    {
        pxSpinlock->xOwnerCore = -1;
    }    
}

void vFakePortGetSpinlock_callback( BaseType_t xCoreID, portSPINLOCK_TYPE *pxSpinlock, int cmock_num_calls )
{
    TEST_ASSERT_NOT_EQUAL( NULL, pxSpinlock );
    
    if( pxSpinlock->uxLockCount == 0 )
    {
        // TEST_ASSERT_EQUAL( -1, pxSpinlock->xOwnerCore );
        pxSpinlock->uxLockCount = pxSpinlock->uxLockCount + 1U;
        pxSpinlock->xOwnerCore = xCoreID;
    }
    else
    {
        TEST_ASSERT_EQUAL( xCoreID, pxSpinlock->xOwnerCore );
        pxSpinlock->uxLockCount = pxSpinlock->uxLockCount + 1U;
    }
}

int xTraceUnblockingOnQueueReceiveCallback( void * pxQueue )
{
    ( void ) pxQueue;
    return 1;
}

int xTraceBlockingOnQueueReceiveCallback( void * pxQueue )
{
    ( void ) pxQueue;

    /* After the queue lock is acquired but before the task state is changed, task B
     * running on core 1 delete task A. Trace macro is used to simulate asynchronous
     * behavior. */
    vSetCurrentCore( 1 );
    vTaskDelete( xTaskHandles[ 0 ] );
    vSetCurrentCore( 0 );
    return 0;
}

/* ============================  Unity Fixtures  ============================ */
/*! called before each testcase */
void setUp( void )
{
    commonSetUp();
    vFakePortInitSpinlock_Stub( vFakePortInitSpinlock_callback );
    vFakePortReleaseSpinlock_Stub( vFakePortReleaseSpinlock_callback );
    vFakePortGetSpinlock_Stub( vFakePortGetSpinlock_callback );
}

/*! called after each testcase */
void tearDown( void )
{
    commonTearDown();
}

/*! called at the beginning of the whole suite */
void suiteSetUp()
{
}

/*! called at the end of the whole suite */
int suiteTearDown( int numFailures )
{
    return numFailures;
}

/* ==============================  Test Cases  ============================== */

/**
 * @brief AWS_IoT-FreeRTOS_SMP_TC-TBD
  1. Task A has preemption disabled and holds the queue lock
  2. Task B calls vTaskDelete(Task A), placing Task A on xTasksWaitingTermination list
  3. Task A continues running since preemption is disabled
  4. When Task A calls vTaskPlaceOnEventList():
    - First it uses vListInsert to put itself on the event list
    - Then it calls prvAddCurrentTaskToDelayedList which:
        - Removes the task from the ready list (uxListRemove)
        - Places it on either the suspended or delayed list based on timeout
    - This removes Task A from where Task B had placed it (xTasksWaitingTermination)
  This creates a race condition where Task A effectively "escapes" deletion by Task B when it places itself on an event list.
 */
void test_queue_receive_state_nullification( void )
{
    uint32_t i;
    QueueHandle_t xQueueHandle;
    uint8_t queueBuffer[ 4 ];

    /* Create configNUMBER_OF_CORES tasks of equal priority */
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[ i ] );
    }

    vTaskStartScheduler();
    
    /* Task 0 calls xQueueReceive to place itself in a event list for waiting. */
    xQueueHandle = xQueueCreate( 4U, 1U );
    TEST_ASSERT_NOT_EQUAL( NULL, xQueueHandle );
    
    /* After task 0 get the queue lock but before task 0 calls vTaskPlaceOnEventList
     * task 1 tries to delete task 0. This is done in the prvGET_SPINLOCK callback. */
    ( void ) xQueueReceive( xQueueHandle, queueBuffer, 10U );

    /* Task 0 continues to run until queue is unlocked and preemption enabled. Then
     * task 0 is switched out to run an idle task on core 0. */
    verifySmpTask( &xTaskHandles[ 0 ], eDeleted, -1 );

    /* Verify all configNUMBER_OF_CORES tasks are in the running state */
    for( i = 1; i < configNUMBER_OF_CORES; i++ )
    {
        verifySmpTask( &xTaskHandles[ i ], eRunning, i );
    }
}
