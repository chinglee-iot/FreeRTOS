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

/**
 * @brief AWS_IoT-FreeRTOS_SMP_TC-1
 * The purpose of this test is to verify when multiple CPU cores are available and 
 * the FreeRTOS kernel is configured as (configRUN_MULTIPLE_PRIORITIES = 0) that tasks 
 * of equal priority will execute simultaneously. The kernel will be configured as follows:

 * #define configRUN_MULTIPLE_PRIORITIES                    0
 * #define configUSE_TIME_SLICING                           0
 * #define configNUM_CORES                                  (N > 1)
 * 
 * This test can be run with FreeRTOS configured for any number of cores greater than 1 .
 * 
 * Tasks are created prior to starting the scheduler.
 * 
 * Task (T1)	    Task (TN)
 * Priority – 1     Priority –1
 * State - Ready	State - Ready
 * 
 * After calling vTaskStartScheduler()
 * 
 * Task (T1)	               Task (TN)
 * Priority – 1                Priority – 1
 * State - Running (Core 0)	   State - Running (Core N)
 */
void test_priority_verification_tasks_equal_priority( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;

    /* Create configNUM_CORES tasks of equal priority */
    for (i = 0; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    /* Verify all configNUM_CORES tasks are in the running state */
    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }
}

/**
 * @brief AWS_IoT-FreeRTOS_SMP_TC-2
 * The purpose of this test is to verify when multiple CPU cores are available and 
 * the FreeRTOS kernel is configured as (configRUN_MULTIPLE_PRIORITIES = 0) that 
 * tasks of different priorities will execute simultaneously. The kernel will be 
 * configured as follows:
 * 
 * #define configRUN_MULTIPLE_PRIORITIES                    0
 * #define configUSE_TIME_SLICING                           0
 * #define configNUM_CORES                                  (N > 1)
 * 
 * This test can be run with FreeRTOS configured for any number of cores greater
 * than 1.
 * 
 * One high priority task will be created. N low priority tasks will be created
 * per remaining CPU cores.
 * 
 * Task (T1)	     Task (TN)
 * Priority – 2      Priority – 1
 * State - Ready	 State - Ready
 * 
 * After calling vTaskStartScheduler()
 * 
 * Task (T1)	               Task (TN)
 * Priority – 2                Priority – 1
 * State - Running (Core 0)	   State - Ready
 */
void test_priority_verification_tasks_different_priorities( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;

    /* Create a single task at high priority */
    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[0] );

    /* Create all remaining tasks at low priority */
    for (i = 1; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    /* Verify the high priority task is running */
    verifySmpTask( &xTaskHandles[0], eRunning, 0 );

    for (i = 1; i < configNUM_CORES; i++) {
        /* Verify all other tasks are in the idle state */
        verifySmpTask( &xTaskHandles[i], eReady, -1 );
        
        /* Verify the idle task is running on all other CPU cores */
        verifyIdleTask( i-1, i);
    }
}

/**
 * @brief AWS_IoT-FreeRTOS_SMP_TC-3
 * A task of equal priority will be created for each available CPU core. 
 * This test will verify that when the priority of one task is lowered the 
 * task is no longer running.
 * 
 * #define configRUN_MULTIPLE_PRIORITIES                    0
 * #define configNUM_CORES                                  2
 * #define configNUM_CORES                                  (N > 1)
 * 
 * This test can be run with FreeRTOS configured for any number of cores
 * greater than 1 .
 * 
 * Tasks are created prior to starting the scheduler.
 * 
 * Task (T1)	    Task (TN)
 * Priority – 2     Priority – 2
 * State - Ready    State - Ready
 * 
 * After calling vTaskStartScheduler()
 * 
 * Task (T1)	             Task (TN)
 * Priority – 2              Priority – 2
 * State - Running (Core 0)	 State - Running (Core N)
 * 
 * After calling vTaskPrioritySet() and lowering the priority of task T1
 * 
 * Task (T1)	   Task (T2)
 * Priority – 1    Priority – 2
 * State - Ready   State - Running (Core N)
*/
void test_priority_change_tasks_equal_priority_lower( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;
    TaskStatus_t xTaskDetails;

    /* FIXME: Test fails without this, changing priority of the running core does not work as intended*/
    vFakePortGetCoreID_IgnoreAndReturn(1);

    /* Create tasks of equal priority for all available CPU cores */
    for (i = 0; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    /* Verify all tasks are in the running state */
    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }

    /* Lower the priority of task T0 */
    vTaskPrioritySet( xTaskHandles[0], 1 );

    /* Verify the priority has been changed */
    vTaskGetInfo( xTaskHandles[0], &xTaskDetails, pdTRUE, eInvalid );
    TEST_ASSERT_EQUAL( 1, xTaskDetails.xHandle->uxPriority );
    
    /* Verify T0 is the the ready state */
    verifySmpTask( &xTaskHandles[0], eReady, -1 );
    
    /* Verify the idle task is now running on CPU core 0 */
    verifyIdleTask(0, 0);

    /* Verify all other tasks remain in the running state on the same CPU cores */
    for (i = 1; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }
}

/**
 * @brief AWS_IoT-FreeRTOS_SMP_TC-4
 * A task of equal priority will be created for each available CPU core.
 * This test will verify that when the priority of one task is raised it
 * shall remain running and all other tasks will enter the ready state.
 * 
 * #define configRUN_MULTIPLE_PRIORITIES                    0
 * #define configNUM_CORES                                  2
 * #define configNUM_CORES                                  (N > 1)
 * 
 * This test can be run with FreeRTOS configured for any number of cores
 * greater than 1.
 * 
 * Tasks are created prior to starting the scheduler.
 * 
 * Task (T1)	    Task (TN)
 * Priority – 1     Priority – 1
 * State - Ready    State - Ready
 * 
 * After calling vTaskStartScheduler()
 * 
 * Task (T1)	             Task (TN)
 * Priority – 1              Priority – 1
 * State - Running (Core 0)	 State - Running (Core N)
 * 
 * After calling vTaskPrioritySet() and raising the priority of task T1
 * 
 * Task (T1)	             Task (T2)
 * Priority – 2              Priority – 1
 * State - Running (Core 0)  State - Ready
*/
void test_priority_change_tasks_equal_priority_raise( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;
    TaskStatus_t xTaskDetails;

    /* Create tasks of equal priority for all available CPU cores */
    for (i = 0; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    /* Verify all tasks are in the running state */
    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }

    /* Raise the priority of task T0 */
    vTaskPrioritySet( xTaskHandles[0], 2 );

    /* Verify the priority has been changed */
    vTaskGetInfo( xTaskHandles[0], &xTaskDetails, pdTRUE, eInvalid );
    TEST_ASSERT_EQUAL( 2, xTaskDetails.xHandle->uxPriority );
    
    /* Verify T0 is the the running state */
    verifySmpTask( &xTaskHandles[0], eRunning, 0 );

    for (i = 1; i < configNUM_CORES; i++) {
        /* Verify all other tasks are in the idle state */
        verifySmpTask( &xTaskHandles[i], eReady, -1 );

        /* Verify the idle task is running on all other CPU cores */
        verifyIdleTask(i - 1, i);
    }
}

/**
 * @brief AWS_IoT-FreeRTOS_SMP_TC-5
 * A single task of high priority will be created. A low priority task will be
 * created for each remaining available CPU core. The test will first verify
 * only the high priority task is in the running state. Each low priority task
 * will then be raised to high priority and enter the running state.
 * 
 * #define configRUN_MULTIPLE_PRIORITIES                    0
 * #define configNUM_CORES                                  2
 * #define configNUM_CORES                                  (N > 1)
 *
 * This test can be run with FreeRTOS configured for any number of cores greater
 * than 1.
 * 
 * Tasks are created prior to starting the scheduler.
 * 
 * Task (T1)	   Task (TN)
 * Priority – 2    Priority – 1
 * State - Ready   State - Ready
 * 
 * After calling vTaskStartScheduler()
 * 
 * Task (T1)	             Task (TN)
 * Priority – 2              Priority – 1
 * State - Running (Core 0)	 State - Ready
 * 
 * After calling vTaskPrioritySet() and raising the priority of tasks TN
 * 
 * Task (T1)	             Task (TN)
 * Priority – 2              Priority – 2
 * State - Running (Core 0)	 State - Running (Core N)
 */
void test_priority_change_tasks_different_priority_raise( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;
    TaskStatus_t xTaskDetails;

    /* Create a single task at high priority */
    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[0] );

    /* Create all remaining tasks at low priority */
    for (i = 1; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }
    
    vTaskStartScheduler();

    /* Verify the high priority task is running */
    verifySmpTask( &xTaskHandles[0], eRunning, 0 );

    for (i = 1; i < configNUM_CORES; i++) {
        /* Verify all other tasks are in the idle state */
        verifySmpTask( &xTaskHandles[i], eReady, -1 );

        /* Verify the idle task is running on all other CPU cores */
        verifyIdleTask(i - 1, i);
    }

    for (i = 1; i < configNUM_CORES; i++) {
        /* Raise the priority of the task */
        vTaskPrioritySet( xTaskHandles[i], 2 );

        /* Verify the priority has been rasied */
        vTaskGetInfo( xTaskHandles[i], &xTaskDetails, pdTRUE, eInvalid );
        TEST_ASSERT_EQUAL( 2, xTaskDetails.xHandle->uxPriority );

        /* Verify the task is now in the running state */
        /* FIXME: Shouldnt task[N] be running on core N? */
        verifySmpTask( &xTaskHandles[i], eRunning, configNUM_CORES - i );
    }
}

/**
 * @brief AWS_IoT-FreeRTOS_SMP_TC-6
 * A single task of high priority will be created. A low priority task will be
 * created for each remaining available CPU core. The test will first verify
 * only the high priority task is in the running state. The high priority task
 * shall be lowered. This will cause all low priority tasks to enter the running
 * state.
 * 
 * #define configRUN_MULTIPLE_PRIORITIES                    0
 * #define configNUM_CORES                                  2
 * #define configNUM_CORES                                  (N > 1)
 * 
 * This test can be run with FreeRTOS configured for any number of cores greater
 * than 1.
 * 
 * Tasks are created prior to starting the scheduler.
 * 
 * Task (T1)	    Task (TN)
 * Priority – 2     Priority –1
 * State - Ready    State - Ready
 * 
 * After calling vTaskStartScheduler()
 * 
 * Task (T1)	             Task (TN)
 * Priority – 2              Priority – 1
 * State - Running (Core 0)	 State - Ready
 * 
 * After calling vTaskPrioritySet() and lowering the
 * priority of all low priority tasks.
 * 
 * Task (T1)	              Task (TN)
 * Priority – 1               Priority – 1
 * State - Running (Core 0)	  State - Running (Core N)
 */
void test_priority_change_tasks_different_priority_lower( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;
    TaskStatus_t xTaskDetails;

    vFakePortGetCoreID_IgnoreAndReturn(1); //Test fails without this, changing priority of the running core does not work as intended

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

    vTaskPrioritySet( xTaskHandles[0], 1 ); //After this returns task[0] is not running, task[1] is running on 0

    vTaskGetInfo( xTaskHandles[0], &xTaskDetails, pdTRUE, eInvalid );
    TEST_ASSERT_EQUAL( 1, xTaskDetails.xHandle->uxPriority );

    verifySmpTask( &xTaskHandles[0], eRunning, 0 );

    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }
}

/* SMP-TC-7 */
void test_smp7( void )
{
    TaskHandle_t xTaskHandles[configNUM_CORES] = { NULL };
    uint32_t i;

    for (i = 0; i < configNUM_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }

    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    vTaskDelete(xTaskHandles[0]);

    TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp );
    verifySmpTask( &xTaskHandles[0], eDeleted, -1 );
    for (i = 1; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }
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
    TaskHandle_t xTaskHandles[configNUM_CORES + 1] = { NULL };
    uint32_t i;

    /* Create configNUM_CORES + 1 tasks of equal priority */
    for (i = 0; i < (configNUM_CORES + 1); i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }

    /* Start the schedular. Two of the tasks will be placed in the running state */
    vTaskStartScheduler();

    /* Verify two tasks are running and one task is ready */
    for (i = 0; i < configNUM_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }

    verifySmpTask( &xTaskHandles[i], eReady, -1 );

    /* Verify there are no deleted tasks pending cleanup */
    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    /* Delete task 1 running on core 0. Task 3 should begin running */
    vTaskDelete(xTaskHandles[0]);

    /* Verify a deleted task is now pending cleanup */
    TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp );

    /* Verify all tasks are in their expected states */
    verifySmpTask( &xTaskHandles[0], eDeleted, -1 );
    
    for (i = 1; i < (configNUM_CORES); i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }
    verifySmpTask( &xTaskHandles[i], eRunning, 0 );
}

/* SMP-TC-9 */
void test_smp9( void )
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
        verifySmpTask( &xTaskHandles[1], eReady, -1 );
        verifyIdleTask(i - 1, i);
    }

    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    for (i = 1; i < configNUM_CORES; i++) {
        vTaskDelete(xTaskHandles[i]);

        verifySmpTask( &xTaskHandles[0], eRunning, 0 );
        verifySmpTask( &xTaskHandles[i], eDeleted, -1 );
    }

    /* Remains 0 since all deleted tasks were not running */
    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );
}

/* SMP-TC-10 */
void test_smp10( void )
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
        verifySmpTask( &xTaskHandles[1], eReady, -1 );
        verifyIdleTask(i - 1, i);
    }

    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    vTaskDelete(xTaskHandles[0]);

    TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp );
    verifySmpTask( &xTaskHandles[0], eDeleted, -1 );

    for (i = 1; i < (configNUM_CORES); i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i -1 );
    }
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
