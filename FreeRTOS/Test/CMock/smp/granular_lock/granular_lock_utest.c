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

/* ===========================  EXTERN VARIABLES  =========================== */
extern portSPINLOCK_TYPE xTaskSpinlock;
extern portSPINLOCK_TYPE xISRSpinlock;

/* ===========================  GLOBAL VARIABLES  =========================== */

static TaskHandle_t xTaskHandles[ configNUMBER_OF_CORES ] = { NULL };
uint32_t xPortCriticalNestingCount[ configNUMBER_OF_CORES ] = { 0U };
static int xQueueTestMode = 0;
static BaseType_t xCoreYields[ configNUMBER_OF_CORES ] = { 0 };

/* ============================  Callback Functions  ============================ */
void vFakePortInitSpinlock_callback( portSPINLOCK_TYPE *pxSpinlock, int cmock_num_calls )
{
    TEST_ASSERT_NOT_EQUAL( NULL, pxSpinlock );

    pxSpinlock->uxLockCount = 0;
    pxSpinlock->xOwnerCore = -1;
}

static void vYieldCores( void )
{
    BaseType_t i;
    BaseType_t xPreviousCoreId = portGET_CORE_ID();

    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        if( xCoreYields[ i ] == pdTRUE )
        {
            vSetCurrentCore( i );
            printf( "%s:%d %ld\r\n", __FUNCTION__, __LINE__, i );
            xCoreYields[ i ] = pdFALSE;
            vTaskSwitchContext( i );
        }
    }

    vSetCurrentCore( xPreviousCoreId );
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

    /* Check if the lock is acquired by any core. */
    if( ( xTaskSpinlock.uxLockCount == 0U ) && ( xISRSpinlock.uxLockCount == 0U ) )
    {
        vYieldCores();
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

void vFakePortYieldCore_callback( int xCoreID,
                                  int cmock_num_calls )
{
    BaseType_t xCoreInCritical = pdFALSE;
    BaseType_t xPreviousCoreId;

    /* Check if the lock is acquired by any core. */
    if( ( xTaskSpinlock.uxLockCount != 0U ) || ( xISRSpinlock.uxLockCount != 0U ) )
    {
        xCoreInCritical = pdTRUE;
    }

    if( xCoreInCritical == pdTRUE )
    {
        /* If a is in the critical section, pend the core yield until the
         * task spinlock is released. */
        xCoreYields[ xCoreID ] = pdTRUE;
    }
    else
    {
        /* No task is in the critical section. We can yield this core. */
        xPreviousCoreId = portGET_CORE_ID();
        vSetCurrentCore( xCoreID );
        vTaskSwitchContext( xCoreID );
        vSetCurrentCore( xPreviousCoreId );
    }
}

int xTraceBlockingOnQueueReceiveCallback( void * pxQueue )
{
    int xReturn;

    ( void ) pxQueue;

    if( xQueueTestMode == 0 )
    {
        /* After the queue lock is acquired but before the task state is changed, task B
         * running on core 1 delete task A. Trace macro is used to simulate asynchronous
         * behavior. */
        vSetCurrentCore( 1 );
        vTaskDelete( xTaskHandles[ 0 ] );
        vSetCurrentCore( 0 );
        xReturn = 0;
    }
    else if( xQueueTestMode == 1 )
    {
        /* After the queue lock is acquired but before the task state is changed, task B
         * running on core 1 delete task A. Trace macro is used to simulate asynchronous
         * behavior. */
        vSetCurrentCore( 1 );
        vTaskSuspend( xTaskHandles[ 0 ] );
        vSetCurrentCore( 0 );
        xReturn = 0;
    }
    else if( xQueueTestMode == 2 )
    {
        static int xTaskCallNumbers = 0;

        if( xTaskCallNumbers == 0 )
        {
            /* After the queue lock is acquired but before the task state is changed, task B
             * running on core 1 delete task A. Trace macro is used to simulate asynchronous
             * behavior. */
            printf( "%s:%d\r\n", __FUNCTION__, __LINE__ );
            vSetCurrentCore( 1 );
            vTaskSuspend( xTaskHandles[ 0 ] );
            vSetCurrentCore( 0 );
            xReturn = 0;
        }
        else
        {
            printf( "%s:%d\r\n", __FUNCTION__, __LINE__ );
            xReturn = 0;
        }
        xTaskCallNumbers++;
    }

    return xReturn;
}

int xTraceUnblockingOnQueueReceiveCallback( void * pxQueue )
{
    int xReturn;

    ( void ) pxQueue;

    if( xQueueTestMode == 0 )
    {
        /* vTaskDelete test. */
        xReturn = 1;
    }
    else if( xQueueTestMode == 1 )
    {
        /* vTaskSuspend test. */
        xReturn = 1;
    }
    else if( xQueueTestMode == 2 )
    {
        static int xTaskCallNumbers = 0;
        if( xTaskCallNumbers == 0 )
        {
            /* Keep looping in the xQueueReceive function. */
            printf( "%s:%d\r\n", __FUNCTION__, __LINE__ );
            vSetCurrentCore( 1 );
            vTaskResume( xTaskHandles[ 0 ] );
            vSetCurrentCore( 0 );
            xReturn = 0;
        }
        else
        {
            printf( "%s:%d\r\n", __FUNCTION__, __LINE__ );
            xReturn = 1;
        }
        xTaskCallNumbers++;
    }

    return xReturn;
}


/* ============================  Unity Fixtures  ============================ */
/*! called before each testcase */
void setUp( void )
{
    commonSetUp();
    vFakePortInitSpinlock_Stub( vFakePortInitSpinlock_callback );
    vFakePortReleaseSpinlock_Stub( vFakePortReleaseSpinlock_callback );
    vFakePortGetSpinlock_Stub( vFakePortGetSpinlock_callback );
    vFakePortYieldCore_StubWithCallback( vFakePortYieldCore_callback );
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
void test_xQueueReceive_delete_state_nullification( void )
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
     * task 1 tries to delete task 0. This is done in the trace macro xTraceBlockingOnQueueReceiveCallback. */
    xQueueTestMode = 0;
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

void test_xQueueReceive_suspend_state_nullification( void )
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
     * task 1 tries to suspend task 0. This is done in the trace macro xTraceBlockingOnQueueReceiveCallback. */
    xQueueTestMode = 1;
    ( void ) xQueueReceive( xQueueHandle, queueBuffer, 10U );

    /* Task 0 continues to run until queue is unlocked and preemption enabled. Then
     * task 0 is switched out to run an idle task on core 0. */
    verifySmpTask( &xTaskHandles[ 0 ], eSuspended, -1 );

    /* Verify all configNUMBER_OF_CORES tasks are in the running state */
    for( i = 1; i < configNUMBER_OF_CORES; i++ )
    {
        verifySmpTask( &xTaskHandles[ i ], eRunning, i );
    }
}

void test_xQueueReceive_suspend_state_resume( void )
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
    xQueueTestMode = 2;
    printf( "%s:%d\r\n", __FUNCTION__, __LINE__ );
    ( void ) xQueueReceive( xQueueHandle, queueBuffer, 10U );

    /* Task 0 continues to run until queue is unlocked and preemption enabled. Then
     * task 0 is switched out to run an idle task on core 0. */
    verifySmpTask( &xTaskHandles[ 0 ], eBlocked, -1 );

    /* Verify all configNUMBER_OF_CORES tasks are in the running state */
    for( i = 1; i < configNUMBER_OF_CORES; i++ )
    {
        verifySmpTask( &xTaskHandles[ i ], eRunning, i );
    }
}
