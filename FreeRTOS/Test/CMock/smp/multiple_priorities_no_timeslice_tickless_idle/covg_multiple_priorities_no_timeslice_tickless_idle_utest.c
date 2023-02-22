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
/*! @file single_priority_no_timeslice_covg_utest */

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
#include "list.h"
#include "mock_timers.h"
#include "mock_fake_assert.h"
#include "mock_fake_port.h"

#define taskTASK_YIELDING       ( TaskRunning_t ) ( -2 )


/* ===========================  EXTERN VARIABLES  =========================== */
extern volatile UBaseType_t uxCurrentNumberOfTasks;
extern volatile UBaseType_t uxDeletedTasksWaitingCleanUp;
extern volatile UBaseType_t uxSchedulerSuspended;
extern volatile TCB_t *  pxCurrentTCBs[ configNUMBER_OF_CORES ];
extern volatile BaseType_t xSchedulerRunning;
extern volatile TickType_t xTickCount;
extern List_t xSuspendedTaskList;
extern List_t xPendingReadyList;
extern volatile UBaseType_t uxTopReadyPriority;
extern volatile BaseType_t xYieldPendings[ configNUMBER_OF_CORES ];
extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];


/* ==============================  Global VARIABLES ============================== */
TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };

/* ============================  Unity Fixtures  ============================ */
/*! called before each testcase */
void setUp( void )
{
    commonSetUp();
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

/* ===========================  EXTERN FUNCTIONS  =========================== */
extern void vTaskEnterCritical(void);

/* ==============================  Helper functions for Test Cases  ============================== */


/* ==============================  Test Cases  ============================== */

/**
 * @brief xTaskGenericNotify - function to notify a task.
 *
 * <b>Coverage</b>
 * @code{c}
 *          case eSetValueWithoutOverwrite:
 *
 *                    if( ucOriginalNotifyState != taskNOTIFICATION_RECEIVED )
 *                    {
 *          ...
 * @endcode
 *
 *  Cover the eSetValueWithoutOverwrite action type with an explicit task specified.
 *  Branch 2 of 2.
 */
void test_coverage_xTaskGenericNotify_with_eAction_equalto_eSetValueWithoutOverwrite_branch_taskWAITING_NOTIFICATION( void )
{
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t prevValue;
    uint32_t i;
    BaseType_t xReturn;

    /* Setup the variables and structure. */
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskTCBs[ i ].uxPriority = 1;
        xTaskTCBs[ i ].xTaskRunState = i;
        vListInitialiseItem( &( xTaskTCBs[i].xStateListItem ) );
        xYieldPendings[ i ] = pdFALSE;
        pxCurrentTCBs[ i ] = &xTaskTCBs[ i ];
    }
    /* A suspended task is created to be resumed from ISR. The task has higher priority
     * than uxTopReadyPriority and the scheduler is suspended. The task will be added
     * to xPendingReadyList after resumed from ISR. */
    xTaskTCBs[ configNUMBER_OF_CORES ].uxPriority = 2;
    listINSERT_END( &xSuspendedTaskList, &xTaskTCBs[ i ].xStateListItem );
    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    listINSERT_END( &xSuspendedTaskList, &xTaskTCBs[ 0 ].xStateListItem );
    //xTaskTCBs[0].ucNotifyState[ xidx ] = /*taskWAITING_NOTIFICATION*/ ( ( uint8_t ) 1 );
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

    /* Validations. In single priority test, the calling core is requested to yield
     * since a higher priority task is resumed. */
    TEST_ASSERT( xReturn == pdPASS );
}
