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
/*! @file granular_lock_tasks_utest.c */

/* C runtime includes. */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Task includes */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

/* Test includes. */
#include "unity.h"
#include "unity_memory.h"
#include "../global_vars.h"
#include "../granular_lock_utest_common.h"
#include "../smp_utest_common.h"

/* Mock includes. */
#include "mock_fake_assert.h"
#include "mock_fake_port.h"
#include "mock_portmacro.h"

/* ===========================  TEST MACROS  =========================== */


/* ===========================  GLOBAL VARIABLES  =========================== */
extern volatile TCB_t * pxCurrentTCBs[ configNUMBER_OF_CORES ];
extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
extern portSPINLOCK_TYPE xTaskSpinlock;
extern portSPINLOCK_TYPE xISRSpinlock;
extern volatile UBaseType_t uxSchedulerSuspended;
extern List_t xDelayedTaskList1;
extern List_t xDelayedTaskList2;
extern List_t * pxDelayedTaskList;
extern volatile BaseType_t xSchedulerRunning;
extern BaseType_t xNumOfOverflows;
extern BaseType_t xTickCount;
extern BaseType_t xYieldPendings[ configNUMBER_OF_CORES ];
extern BaseType_t xPendedTicks;

/* ============================  Unity Fixtures  ============================ */

/*! called before each testcase */
void setUp( void )
{
    int i;

    /* Use the common setup for the testing. */
    commonSetUp();

    /* Specify the granular lock specific implementation. */
    vFakePortInitSpinlock_StopIgnore();
    vFakePortReleaseSpinlock_StopIgnore();
    vFakePortGetSpinlock_StopIgnore();
    vFakePortYieldCore_StopIgnore();

    /* Interrupt masks. */
    ulFakePortSetInterruptMaskFromISR_StopIgnore();
    vFakePortClearInterruptMaskFromISR_StopIgnore();

    ulFakePortSetInterruptMask_StopIgnore();
    vFakePortClearInterruptMask_StopIgnore();

    vFakePortDisableInterrupts_StopIgnore();
    vFakePortEnableInterrupts_StopIgnore();

    for( i = 0; i < configMAX_PRIORITIES; i++ )
    {
        vListInitialise( &pxReadyTasksLists[ i ] );
    }

    vListInitialise( &xDelayedTaskList1 );
    vListInitialise( &xDelayedTaskList2 );
    pxDelayedTaskList = &xDelayedTaskList1;
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

/* ==============================  Test Cases tasks ============================== */

/**
 * @brief xTaskRemoveFromEventList - Remove a task from event list.
 *
 * The task event list item will be removed from event list.
 *
 * <b>Coverage</b>
 * @code{c}
 *
 * kernelENTER_CRITICAL();
 * {
 *     if( listLIST_IS_EMPTY( pxEventList ) == pdFALSE )
 *     {
 *         xReturn = prvTaskRemoveFromEventList( pxEventList );
 *     }
 *     else
 *     {
 *         xReturn = pdFALSE;
 *     }
 * }
 * kernelEXIT_CRITICAL();
 * @endcode
 * ( listLIST_IS_EMPTY( pxEventList ) == pdFALSE ) is true.
 */
void test_granular_locks_tasks_xTaskRemoveFromEventList_list_is_not_empty( void )
{
    TCB_t xTaskTCB = { NULL };
    List_t xEventList;
    BaseType_t xReturn;

    vListInitialise( &xEventList );
    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    xTaskTCB.xStateListItem.pvOwner = &xTaskTCB;
    listINSERT_END( &xEventList, &xTaskTCB.xEventListItem );
    listINSERT_END( &pxReadyTasksLists[ 0 ], &xTaskTCB.xStateListItem );
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    uxSchedulerSuspended = 0U;
    xSchedulerRunning = pdTRUE;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    xReturn = xTaskRemoveFromEventList( &xEventList );

    TEST_ASSERT_EQUAL( pdFALSE, xReturn );
    TEST_ASSERT_EQUAL( 0, xEventList.uxNumberOfItems );
}

/**
 * @brief xTaskRemoveFromEventList - Remove a task from event list.
 *
 * The event list is empty. xReturn is pdFALSE.
 *
 * <b>Coverage</b>
 * @code{c}
 *
 * kernelENTER_CRITICAL();
 * {
 *     if( listLIST_IS_EMPTY( pxEventList ) == pdFALSE )
 *     {
 *         xReturn = prvTaskRemoveFromEventList( pxEventList );
 *     }
 *     else
 *     {
 *         xReturn = pdFALSE;
 *     }
 * }
 * kernelEXIT_CRITICAL();
 * @endcode
 * ( listLIST_IS_EMPTY( pxEventList ) == pdFALSE ) is false.
 */
void test_granular_locks_tasks_xTaskRemoveFromEventList_list_is_empty( void )
{
    TCB_t xTaskTCB = { NULL };
    List_t xEventList;
    BaseType_t xReturn;

    vListInitialise( &xEventList );
    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    xSchedulerRunning = pdTRUE;
    pxCurrentTCBs[ 0 ] = &xTaskTCB;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    xReturn = xTaskRemoveFromEventList( &xEventList );

    TEST_ASSERT_EQUAL( pdFALSE, xReturn );
}

/**
 * @brief xTaskRemoveFromEventListFromISR - Remove a task from event list.
 *
 * The event list is not empty. Task is removed from event list and return value is pdTRUE.
 *
 * <b>Coverage</b>
 * @code{c}
 * BaseType_t xTaskRemoveFromEventListFromISR( const List_t * const pxEventList )
 * {
 *     BaseType_t xReturn;
 *
 *     UBaseType_t uxSavedInterruptStatus = kernelENTER_CRITICAL_FROM_ISR();
 *     {
 *         if( listLIST_IS_EMPTY( pxEventList ) == pdFALSE )
 *         {
 *             xReturn = prvTaskRemoveFromEventList( pxEventList );
 *         }
 *         else
 *         {
 *             xReturn = pdFALSE;
 *         }
 *     }
 *     kernelEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus );
 *
 *     return xReturn;
 * }
 * @endcode
 * ( listLIST_IS_EMPTY( pxEventList ) == pdFALSE ) is true.
 */
void test_granular_locks_tasks_xTaskRemoveFromEventListFromISR_list_is_not_empty( void )
{
    TCB_t xTaskTCB = { NULL };
    List_t xEventList;
    BaseType_t xReturn;

    vListInitialise( &xEventList );
    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    xTaskTCB.xStateListItem.pvOwner = &xTaskTCB;
    listINSERT_END( &xEventList, &xTaskTCB.xEventListItem );
    listINSERT_END( &pxReadyTasksLists[ 0 ], &xTaskTCB.xStateListItem );
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    uxSchedulerSuspended = 0U;
    xSchedulerRunning = pdTRUE;

    ulFakePortSetInterruptMaskFromISR_ExpectAndReturn( 0x12345678 );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortClearInterruptMaskFromISR_Expect( 0x12345678 );

    xReturn = xTaskRemoveFromEventListFromISR( &xEventList );

    TEST_ASSERT_EQUAL( pdFALSE, xReturn );
    TEST_ASSERT_EQUAL( 0, xEventList.uxNumberOfItems );
}

/**
 * @brief xTaskRemoveFromEventListFromISR - Remove a task from event list.
 *
 * The event list is empty and return value is pdFALSE;
 *
 * <b>Coverage</b>
 * @code{c}
 * BaseType_t xTaskRemoveFromEventListFromISR( const List_t * const pxEventList )
 * {
 *     BaseType_t xReturn;
 *
 *     UBaseType_t uxSavedInterruptStatus = kernelENTER_CRITICAL_FROM_ISR();
 *     {
 *         if( listLIST_IS_EMPTY( pxEventList ) == pdFALSE )
 *         {
 *             xReturn = prvTaskRemoveFromEventList( pxEventList );
 *         }
 *         else
 *         {
 *             xReturn = pdFALSE;
 *         }
 *     }
 *     kernelEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus );
 *
 *     return xReturn;
 * }
 * @endcode
 * ( listLIST_IS_EMPTY( pxEventList ) == pdFALSE ) is true.
 */
void test_granular_locks_tasks_xTaskRemoveFromEventListFromISR_list_is_empty( void )
{
    TCB_t xTaskTCB = { NULL };
    List_t xEventList;
    BaseType_t xReturn;

    vListInitialise( &xEventList );
    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    xSchedulerRunning = pdTRUE;

    ulFakePortSetInterruptMaskFromISR_ExpectAndReturn( 0x12345678 );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortClearInterruptMaskFromISR_Expect( 0x12345678 );

    xReturn = xTaskRemoveFromEventListFromISR( &xEventList );

    TEST_ASSERT_EQUAL( pdFALSE, xReturn );
    TEST_ASSERT_EQUAL( 0, xEventList.uxNumberOfItems );
}

/**
 * @brief vTaskInternalSetTimeOutState - Set internal timeout state.
 *
 * The timeout state is set correctly.
 * <b>Coverage</b>
 * @code{c}
 * void vTaskInternalSetTimeOutState( TimeOut_t * const pxTimeOut )
 * {
 *     ...
 *     kernelENTER_CRITICAL();
 *     ...
 *     kernelEXIT_CRITICAL();
 *     ...
 * }
 * @endcode
 */
void test_granular_locks_tasks_vTaskInternalSetTimeOutState( void )
{
    TCB_t xTaskTCB = { NULL };
    TimeOut_t xTimeout;

    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    xSchedulerRunning = pdTRUE;
    xNumOfOverflows = 0x11223344;
    xTickCount = 0x12345678;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    vTaskInternalSetTimeOutState( &xTimeout );

    TEST_ASSERT_EQUAL( 0x11223344, xTimeout.xOverflowCount );
    TEST_ASSERT_EQUAL( 0x12345678, xTimeout.xTimeOnEntering );
}

/**
 * @brief eTaskConfirmSleepModeStatus - Confirm current task sleep mode status.
 *
 * The timeout state is set correctly.
 *
 * <b>Coverage</b>
 * @code{c}
 * eSleepModeStatus eTaskConfirmSleepModeStatus( void )
 * {
 *     ...
 *     kernelENTER_CRITICAL();
 *     ...
 *     else if( xPendedTicks != 0U )
 *     {
 *         eReturn = eAbortSleep;
 *     }
 *     ...
 *     kernelEXIT_CRITICAL();
 *     ...
 * }
 * @endcode
 * ( xPendedTicks != 0U ) is true.
 */
void test_granular_locks_tasks_eTaskConfirmSleepModeStatus_yield_pending( void )
{
    eSleepModeStatus eReturn;
    TCB_t xTaskTCB = { NULL };

    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    xSchedulerRunning = pdTRUE;
    xNumOfOverflows = 0x11223344;
    xTickCount = 0x12345678;
    xPendedTicks = 1U;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    eReturn = eTaskConfirmSleepModeStatus();

    TEST_ASSERT_EQUAL( eAbortSleep, eReturn );
}

/**
 * @brief xTaskPriorityInherit - Mutex owner is NULL.
 *
 * Return value is pdFALSE.
 *
 * <b>Coverage</b>
 * @code{c}
 * BaseType_t xTaskPriorityInherit( TaskHandle_t const pxMutexHolder )
 * {
 *     ...
 *     kernelENTER_CRITICAL();
 *     ...
 *     kernelEXIT_CRITICAL();
 *     ...
 * }
 * @endcode
 */
void test_granular_locks_tasks_xTaskPriorityInherit_pxMutexHolder_is_NULL( void )
{
    TCB_t xTaskTCB = { NULL };
    BaseType_t xReturn;

    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    xSchedulerRunning = pdTRUE;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    xReturn = xTaskPriorityInherit( NULL );

    TEST_ASSERT_EQUAL( pdFALSE, xReturn );
}

/**
 * @brief xTaskPriorityDisinherit - Mutex owner is NULL.
 *
 * Return value is pdFALSE.
 *
 * <b>Coverage</b>
 * @code{c}
 * BaseType_t xTaskPriorityDisinherit( TaskHandle_t const pxMutexHolder )
 * {
 *     ...
 *     kernelENTER_CRITICAL();
 *     ...
 *     kernelEXIT_CRITICAL();
 *     ...
 * }
 * @endcode
 */
void test_granular_locks_tasks_xTaskPriorityDisinherit_pxMutexHolder_is_NULL( void )
{
    TCB_t xTaskTCB = { NULL };
    BaseType_t xReturn;

    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    xSchedulerRunning = pdTRUE;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    xReturn = xTaskPriorityDisinherit( NULL );

    TEST_ASSERT_EQUAL( pdFALSE, xReturn );
}

/**
 * @brief xTaskPriorityDisinherit - Mutex owner is NULL.
 *
 * The test shows result in coverage report.
 *
 * <b>Coverage</b>
 * @code{c}
 * void vTaskPriorityDisinheritAfterTimeout( TaskHandle_t const pxMutexHolder,
 *                                           UBaseType_t uxHighestPriorityWaitingTask )
 * {
 *     ...
 *     kernelENTER_CRITICAL();
 *     ...
 *     kernelEXIT_CRITICAL();
 *     ...
 * }
 * @endcode
 */
void test_granular_locks_tasks_vTaskPriorityDisinheritAfterTimeout_pxMutexHolder_is_NULL( void )
{
    TCB_t xTaskTCB = { NULL };

    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    xSchedulerRunning = pdTRUE;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    vTaskPriorityDisinheritAfterTimeout( NULL, configMAX_PRIORITIES );
}

/**
 * @brief pvTaskIncrementMutexHeldCount - uxMutexesHeld is increased.
 *
 * Return value is current TCB.
 *
 * <b>Coverage</b>
 * @code{c}
 * void TaskHandle_t pvTaskIncrementMutexHeldCount( void )
 * {
 *     ...
 *     kernelENTER_CRITICAL();
 *     ...
 *     kernelEXIT_CRITICAL();
 *     ...
 * }
 * @endcode
 */
void test_granular_locks_tasks_pvTaskIncrementMutexHeldCount( void )
{
    TaskHandle_t xTaskHandle;
    TCB_t xTaskTCB = { NULL };

    xTaskTCB.xEventListItem.pvOwner = &xTaskTCB;
    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    xSchedulerRunning = pdTRUE;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );
    ulFakePortSetInterruptMask_ExpectAndReturn( 0 );

    vFakePortClearInterruptMask_Expect( 0 );
    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    xTaskHandle = pvTaskIncrementMutexHeldCount();

    TEST_ASSERT_EQUAL( &xTaskTCB, xTaskHandle );
    TEST_ASSERT_EQUAL( 1U, xTaskTCB.uxMutexesHeld );
}

/**
 * @brief xTaskIncrementTick - Increment tick when scheduler is suspended.
 *
 * xReturn is pdFALSE.
 *
 * <b>Coverage</b>
 * @code{c}
 * BaseType_t xTaskIncrementTick( void )
 * {
 *     ...
 *     uxSavedInterruptStatus = kernelENTER_CRITICAL_FROM_ISR();
 *     {
 *         ...
 *     }
 *     kernelEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus );
 * }
 * @endcode
 * ( listLIST_IS_EMPTY( pxEventList ) == pdFALSE ) is false.
 */
void test_granular_locks_tasks_xTaskIncrementTick_scheduler_suspended( void )
{
    BaseType_t xReturn;
    int i;
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES ] = { NULL };

    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        pxCurrentTCBs[ i ] = &xTaskTCBs[ i ];
    }

    xSchedulerRunning = pdTRUE;

    ulFakePortSetInterruptMaskFromISR_ExpectAndReturn( 0x12345678 );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortClearInterruptMaskFromISR_Expect( 0x12345678 );

    xReturn = xTaskIncrementTick();

    TEST_ASSERT_EQUAL( pdFALSE, xReturn );
}

/**
 * @brief xCurrentTaskPreemptionEnable - Enable preemption of current task.
 *
 * xReturn is pdFALSE. Task preemption disable count is decreased.
 *
 * <b>Coverage</b>
 * @code{c}
 * BaseType_t xCurrentTaskPreemptionEnable( void )
 * {
 *     ...
 * }
 * @endcode
 */
void test_granular_locks_tasks_xCurrentTaskPreemptionEnable_task_not_yielded( void )
{
    BaseType_t xReturn;
    TCB_t xTaskTCB = { NULL };

    pxCurrentTCBs[ 0 ] = &xTaskTCB;
    xTaskTCB.uxPreemptionDisable = 1U;
    xTaskTCB.xTaskRunState = 0U;
    xSchedulerRunning = pdTRUE;

    vFakePortDisableInterrupts_ExpectAndReturn( 0 );
    vFakePortGetSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortGetSpinlock_Expect( 0, &xISRSpinlock );

    ulFakePortSetInterruptMask_ExpectAndReturn( 0x12345678 );
    vFakePortClearInterruptMask_Expect( 0x12345678 );

    vFakePortReleaseSpinlock_Expect( 0, &xISRSpinlock );
    vFakePortReleaseSpinlock_Expect( 0, &xTaskSpinlock );
    vFakePortEnableInterrupts_Expect();

    xReturn = xCurrentTaskPreemptionEnable();

    TEST_ASSERT_EQUAL( pdFALSE, xReturn );
    TEST_ASSERT_EQUAL( 0U, xTaskTCB.uxPreemptionDisable );
}
