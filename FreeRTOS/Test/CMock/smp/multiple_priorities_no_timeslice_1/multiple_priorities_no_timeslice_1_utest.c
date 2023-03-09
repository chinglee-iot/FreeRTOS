/*
 * FreeRTOS V202012.00
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
/*! @file multiple_priorities_no_timeslice_utest.c */

/* C runtime includes. */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Tasl includes */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "event_groups.h"
#include "queue.h"

/* Test includes. */
#include "unity.h"
#include "unity_memory.h"
#include "../global_vars.h"
#include "../smp_utest_common.h"
#include <assert.h>

/* Mock includes. */
#include "mock_timers.h"
#include "mock_fake_assert.h"
#include "mock_fake_port.h"
#include "mock_list.h"
#include "mock_list_macros.h"
#include "mock_local_portable.h"




/* ===========================  EXTERN VARIABLES  =========================== */

extern volatile UBaseType_t uxDeletedTasksWaitingCleanUp;
extern volatile UBaseType_t xSchedulerRunning;
extern volatile BaseType_t xYieldPendings[ configNUMBER_OF_CORES ];
extern volatile TCB_t *  pxCurrentTCBs[ configNUMBER_OF_CORES ];

/* ============================  Unity Fixtures  ============================ */
/*! called before each testcase */
void setUp(void)
{
    // commonSetUp();
}

/*! called after each testcase */
void tearDown(void)
{
    // commonTearDown();
}

/*! called at the beginning of the whole suite */
void suiteSetUp()
{
}

/*! called at the end of the whole suite */
int suiteTearDown(int numFailures)
{
    return numFailures;
}

/* ==============================  Test Cases  ============================== */

/**
 * @brief vTaskSuspend - suspends the running task
 *
 * This test is used to suspend a running task when task is actively 
 * running and not scheduled to yield.
 *
 * <b>Coverage</b>
 * @code{c}
 * if( xSchedulerRunning != pdFALSE )
 * {        
 *      if( xTaskRunningOnCore == portGET_CORE_ID() )
 *      {      
 *          configASSERT( uxSchedulerSuspended == 0 );
 *          vTaskYieldWithinAPI();
 *      }
 *      else
 *      {
 *          prvYieldCore( xTaskRunningOnCore );
 *      }
 * }
 * else 
 * {        
 *      mtCOVERAGE_TEST_MARKER();
 * }
 * @endcode
 * ( xSchedulerRunning != pdFALSE ) is false.
 */

void test_vTaskSuspend_scheduler_running_false(void)
{
   
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };

    xTaskTCBs[ 0 ].uxPriority = 1;
    // xTaskTCBs[ 0 ].xTaskRunState = 1;
    xYieldPendings[ 0 ]= pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[0];
    pxCurrentTCBs[ 0 ]->xTaskRunState = 1;
    xSchedulerRunning = pdFALSE;

    vFakePortEnterCriticalSection_Expect();
    uxListRemove_ExpectAnyArgsAndReturn(pdTRUE);
    listLIST_ITEM_CONTAINER_ExpectAnyArgsAndReturn(NULL);
    vListInsertEnd_ExpectAnyArgs();
    vFakePortExitCriticalSection_Expect();
  
    vTaskSuspend(&xTaskTCBs[0]);

    TEST_ASSERT_EQUAL( pdFALSE, xYieldPendings[ 0 ]);
    TEST_ASSERT_EQUAL( 1, pxCurrentTCBs[ 0 ]->xTaskRunState);

}

/**
 * @brief vTaskSuspend - suspends the task
 *
 * This test is used to ensure that one of the macro's conditions 
 * where TaskRunState is greater than zero is set to false.
 *
 * <b>Coverage</b>
 * @code{c}
 * if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 * @endcode
 * ( taskTASK_IS_RUNNING( pxTCB ) ) is false.
 */

void test_vTaskSuspend_running_state_below_range(void)
{   
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
 
    xTaskTCBs[ 0 ].uxPriority = 1;
    // xTaskTCBs[ 0 ].xTaskRunState = 1;
    xYieldPendings[ 0 ]= pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[0];
    pxCurrentTCBs[ 0 ]->xTaskRunState = -1;
    xSchedulerRunning = pdFALSE;

    vFakePortEnterCriticalSection_Expect();
    uxListRemove_ExpectAnyArgsAndReturn(pdTRUE);
    listLIST_ITEM_CONTAINER_ExpectAnyArgsAndReturn(NULL);
    vListInsertEnd_ExpectAnyArgs();
    vFakePortExitCriticalSection_Expect();
  
    vTaskSuspend(&xTaskTCBs[0]);

    TEST_ASSERT_EQUAL( pdFALSE, xYieldPendings[ 0 ]);
    TEST_ASSERT_EQUAL( -1, pxCurrentTCBs[ 0 ]->xTaskRunState);

}

/**
 * @brief vTaskSuspend - suspends the task
 *
 * This test is used to cover the case where the other macro condition where 
 * TaskRunState is less than configNUMBER_OF_CORES is set to false.
 *
 * <b>Coverage</b>
 * @code{c}
 * if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 * @endcode
 * ( taskTASK_IS_RUNNING( pxTCB ) ) is false.
 */

void test_vTaskSuspend_running_state_above_range(void)
{   
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
 
    xTaskTCBs[ 0 ].uxPriority = 1;
    // xTaskTCBs[ 0 ].xTaskRunState = 1;
    xYieldPendings[ 0 ]= pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[0];
    pxCurrentTCBs[ 0 ]->xTaskRunState = configNUMBER_OF_CORES + 1;

    vFakePortEnterCriticalSection_Expect();
    uxListRemove_ExpectAnyArgsAndReturn(pdTRUE);
    listLIST_ITEM_CONTAINER_ExpectAnyArgsAndReturn(NULL);
    vListInsertEnd_ExpectAnyArgs();
    vFakePortExitCriticalSection_Expect();
  
    vTaskSuspend(&xTaskTCBs[0]);

    TEST_ASSERT_EQUAL( pdFALSE, xYieldPendings[ 0 ]);
    TEST_ASSERT_EQUAL( 17, pxCurrentTCBs[ 0 ]->xTaskRunState);

}