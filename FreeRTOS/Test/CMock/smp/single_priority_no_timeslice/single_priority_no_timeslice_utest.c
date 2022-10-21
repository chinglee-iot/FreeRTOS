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
/*! @file smp_utest.c */

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

/* Mock includes. */
#include "mock_timers.h"
#include "mock_fake_assert.h"
#include "mock_fake_port.h"

/* ===========================  EXTERN VARIABLES  =========================== */

extern volatile UBaseType_t uxDeletedTasksWaitingCleanUp;

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

/* ==============================  Test Cases  ============================== */

/* SMP-TC-1 */
void test_smp1( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;

    for (i = 0; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }
}

/* SMP-TC-2 */
void test_smp2( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[0] );

    for (i = 1; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    verifySmpTask( &xTaskHandles[0], eRunning, 0 );

    for (i = 1; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eReady, -1 );
        verifyIdleTask( i-1, i);
    }
}

/* SMP-TC-3 */
void test_smp3( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;
    TaskStatus_t xTaskDetails;

    vFakePortGetCoreID_IgnoreAndReturn(1); //Test fails without this, changing priority of the running core does not work as intended

    for (i = 0; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }

    vTaskPrioritySet( xTaskHandles[0], 1 );

    vTaskGetInfo( xTaskHandles[0], &xTaskDetails, pdTRUE, eInvalid );
    TEST_ASSERT_EQUAL( 1, xTaskDetails.xHandle->uxPriority );
    
    verifySmpTask( &xTaskHandles[0], eReady, -1 );
    
    verifyIdleTask(0, 0);

    for (i = 1; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }
}

/* SMP-TC-4 */
void test_smp4( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;
    TaskStatus_t xTaskDetails;

    for (i = 0; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }

    vTaskPrioritySet( xTaskHandles[0], 2 );

    verifySmpTask( &xTaskHandles[0], eRunning, 0 );
    vTaskGetInfo( xTaskHandles[0], &xTaskDetails, pdTRUE, eInvalid );
    TEST_ASSERT_EQUAL( 2, xTaskDetails.xHandle->uxPriority );

    for (i = 1; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eReady, -1 );
        verifyIdleTask(i - 1, i);
    }
}

/* SMP-TC-5 */
void test_smp5( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;
    TaskStatus_t xTaskDetails;

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[0] );

    for (i = 1; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    verifySmpTask( &xTaskHandles[0], eRunning, 0 );

    for (i = 1; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eReady, -1 );
        verifyIdleTask(i - 1, i);
    }

    for (i = 1; i < configNUM_CORES; i++) {
        vTaskPrioritySet( xTaskHandles[i], 2 );

        vTaskGetInfo( xTaskHandles[i], &xTaskDetails, pdTRUE, eInvalid );
        TEST_ASSERT_EQUAL( 2, xTaskDetails.xHandle->uxPriority );

        verifySmpTask( &xTaskHandles[i], eRunning, configNUM_CORES - i ); //Why reverse order?
    }

}

/* SMP-TC-6 */
void test_smp6( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskStatus_t xTaskDetails;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    
    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifyIdleTask(0, 1);

    vTaskPrioritySet( xHandleTask1, 1 );
    vTaskSwitchContext(); //WHY is this needed, why do cores shuffle?

    verifySmpTask( &xHandleTask1, eRunning, 1 );
    vTaskGetInfo( xHandleTask1, &xTaskDetails, pdTRUE, eInvalid );
    TEST_ASSERT_EQUAL( 1, xTaskDetails.xHandle->uxPriority );
    
    verifySmpTask( &xHandleTask2, eRunning, 0 );
}

/* SMP-TC-7 */
void test_smp7( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );

    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    vTaskDelete(xHandleTask1);

    TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp );
    verifySmpTask( &xHandleTask1, eDeleted, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
}

/**
 * @brief SMP-TC-1
 * The purpose of this test is to verify when a running task is deleted that a task
 * of equal priority in the ready state will be scheduled to run.  
 * 
 * Three tasks of equal priority are created prior to starting the schedular. 
 * 
 * Task (T1 - T3)
 * Priority – 1
 * State - Ready
 * 
 * After calling vTaskStartScheduler()
 * 
 * Task (T1 - T2)
 * Priority – 1
 * State - Running (Core 0/1)
 * 
 * Delete task (T1)
 * 
 * Task (T2 - T3)
 * Priority – 1
 * State - Running (Core 0/1)
*/
void test_SMP_TC_8( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    /* Create 3 tasks of equal priority */
    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );

    /* Start the schedular. Two of the tasks will be placed in the running state */
    vTaskStartScheduler();

    /* Verify two tasks are running and one task is ready */
    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );

    /* Verify there are no deleted tasks pending cleanup */
    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    /* Delete task 1 running on core 0. Task 3 should begin running */
    vTaskDelete(xHandleTask1);

    /* Verify a deleted task is now pending cleanup */
    TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp );

    /* Verify all 3 tasks are in their expected states */
    verifySmpTask( &xHandleTask1, eDeleted, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eRunning, 0 );
}

/* SMP-TC-9 */
void test_smp9( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );

    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    vTaskDelete(xHandleTask2);

    //TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp ); //Why dosnt this pass?
    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eDeleted, -1 );
}

/* SMP-TC-10 */
void test_smp10( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );

    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    vTaskDelete(xHandleTask1);

    TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp ); //Why dosnt this pass?
    verifySmpTask( &xHandleTask1, eDeleted, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 0 );
}

/* SMP-TC-11 */
void test_smp11( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifyIdleTask(0, 1);

    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
}

/* SMP-TC-12 */
void test_smp12( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifyIdleTask(0, 1);

    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifyIdleTask(0, 1);
}

/* SMP-TC-13 */
void test_smp13( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifyIdleTask(0, 1);

    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 3, &xHandleTask2 );

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );

    vTaskSwitchContext(); //WHY?

    verifySmpTask( &xHandleTask1, eReady, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 0 );
    verifyIdleTask(0, 1);
}

/* SMP-TC-14 */
void test_smp14( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );

    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask3 );

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
}

/* SMP-TC-15 */
void test_smp15( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );

    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
}

/* SMP-TC-16 */
void test_smp16( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );

    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 3, &xHandleTask3 );

    vTaskSwitchContext(); //WHY?

    verifySmpTask( &xHandleTask1, eReady, -1 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifySmpTask( &xHandleTask3, eRunning, 1 );
}

/* SMP-TC-17 */
void test_smp17( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifyIdleTask(0, 1);

    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask3 );

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifySmpTask( &xHandleTask3, eRunning, 1 );
}

/* SMP-TC-18 */
void test_smp18( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifyIdleTask(0, 1);

    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
    verifyIdleTask(0, 1);
}

/* SMP-TC-19 */
void test_smp19( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );

    vTaskSuspend(xHandleTask1);
    vTaskSwitchContext(); //WHY

    verifySmpTask( &xHandleTask1, eSuspended, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifyIdleTask(1, 0);

    vTaskResume(xHandleTask1);
    vTaskSwitchContext(); //WHY

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
}

/* SMP-TC-20 */
void test_smp20( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifyIdleTask(0, 1);

    vTaskSuspend(xHandleTask1);

    verifySmpTask( &xHandleTask1, eSuspended, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 0 );
    verifyIdleTask(0, 1);

    vTaskResume(xHandleTask1);
    vTaskSwitchContext(); //WHY

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifyIdleTask(0, 1);
}

/* SMP-TC-21 */
void test_smp21( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifyIdleTask(0, 1);

    vTaskSuspend(xHandleTask2);

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eSuspended, -1 );
    verifyIdleTask(0, 1);

    vTaskResume(xHandleTask2);

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifyIdleTask(0, 1);
}

/* SMP-TC-22 */
void test_smp22( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );

    vTaskSuspend(xHandleTask1);

    verifySmpTask( &xHandleTask1, eSuspended, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
    verifyIdleTask(0, 0);

    vTaskResume(xHandleTask1);
    vTaskSwitchContext(); //WHY

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
}

/* SMP-TC-23 */
void test_smp23( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
    verifyIdleTask(0, 1);

    vTaskSuspend(xHandleTask1);

    verifySmpTask( &xHandleTask1, eSuspended, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 0 );
    verifySmpTask( &xHandleTask3, eRunning, 1 );

    vTaskResume(xHandleTask1);
    vTaskSwitchContext(); //WHY

    verifySmpTask( &xHandleTask1, eRunning, 1 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
    verifyIdleTask(1, 0);
}

/* SMP-TC-24 */
void test_smp24( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );

    vTaskSuspend(xHandleTask1);

    verifySmpTask( &xHandleTask1, eSuspended, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eRunning, 0 );
}
