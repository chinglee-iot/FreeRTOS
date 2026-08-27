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
#include "timers.h"

/* Test includes. */
#include "unity.h"
#include "unity_memory.h"
#include "../global_vars.h"

/* Test includes. */
#include "unity.h"
#include "unity_memory.h"
#include "../global_vars.h"
#include "../smp_utest_common.h"
#include "../granular_lock_utest_common.h"

/* Mock includes. */
#include "mock_fake_assert.h"
#include "mock_fake_port.h"
#include "mock_portmacro.h"

/* ===========================  EXTERN VARIABLES  =========================== */
extern volatile BaseType_t xYieldPendings[ configNUMBER_OF_CORES ];

/* ============================  Unity Fixtures  ============================ */

/*! called before each testcase */
void setUp( void )
{
    granularLocksSetUp();
}

/*! called after each testcase */
void tearDown( void )
{
    granularLocksTearDown();
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

void test_granular_locks_tasks_scheduler_suspension_core_yield( void )
{
    TaskHandle_t xTaskHandles[ configNUMBER_OF_CORES + 1 ] = { NULL };
    uint32_t i;

    /* Create a running task. */
    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[ configNUMBER_OF_CORES ] );

    /* Create configNUMBER_OF_CORES tasks of equal priority */
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[ i ] );
    }

    vTaskStartScheduler();

    /* Timer task will run on core 0 and the test task runs on core 1. */
    verifySmpTask( &xTaskHandles[ configNUMBER_OF_CORES ], eRunning, 1 );
    vSetCurrentCore( 1 );

    /* The current core suspend the scheduler. */
    vTaskSuspendAll();
    {
        /* Raise the priority of other tasks. Now the task has higher priority than current.
         * It should request the core 1 task to yield. The scheduler is suspended. The
         * scheduler will set yield pending of this core. */
        for( i = 0; i < configNUMBER_OF_CORES; i++ )
        {
            vTaskPrioritySet( xTaskHandles[ i ], 3 );
        }

        /* Disable preemption of the test task. When entering a TCB critical section,
         * the scheduler checks the current task's run state. However, because the
         * scheduler is suspended during this test, context switches are deferred
         * until it is resumed. This creates an infinite loop. To fix this, the core
         * that suspended the scheduler should not handle the yield request until the
         * scheduler is resumed. */
        vTaskPreemptionDisable( NULL );
        TEST_ASSERT_EQUAL( pdTRUE, xYieldPendings[ 1 ] );
        verifySmpTask( &xTaskHandles[ configNUMBER_OF_CORES ], eRunning, 1 );
        
        vTaskPreemptionEnable( NULL );
    }
    ( void ) xTaskResumeAll();
    
    /* After resume the scheduler, the core should handle the context switch. */
    verifySmpTask( &xTaskHandles[ configNUMBER_OF_CORES ], eReady, -1 );
}
