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
 * @brief vTaskPreemptionEnable - Enable preemption of a task when scheduler is not running
 *
 * The xPreemptionDisable of the task will be set to pdFALSE.
 *
 * <b>Coverage</b>
 * @code{c}
 * if( xSchedulerRunning != pdFALSE )
 * {
 *     if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 *     {
 *         xCoreID = ( BaseType_t ) pxTCB->xTaskRunState;
 *         prvYieldCore( xCoreID );
 *     }
 * }
 * @endcode
 * ( xSchedulerRunning != pdFALSE ) is false.
 */
void test_coverage_vTaskPreemptionEnable_scheduler_not_running( void )
{
    TCB_t xTaskTCB = { NULL };

    /* Setup variables. */
    xTaskTCB.xPreemptionDisable = pdTRUE;

    /* Clear callback in commonSetUp. */
    vFakePortEnterCriticalSection_StubWithCallback( NULL );
    vFakePortExitCriticalSection_StubWithCallback( NULL );

    /* Expections. */
    vFakePortEnterCriticalSection_Expect();
    vFakePortExitCriticalSection_Expect();

    /* API call. */
    vTaskPreemptionEnable( &xTaskTCB );

    /* Validation. */
    TEST_ASSERT( xTaskTCB.xPreemptionDisable == pdFALSE );
}

/**
 * @brief vTaskPreemptionEnable - Enable preemption of a task when scheduler is running
 *
 * The xPreemptionDisable of the task will be set to pdFALSE.
 *
 * <b>Coverage</b>
 * @code{c}
 * if( xSchedulerRunning != pdFALSE )
 * {
 *     if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 *     {
 *         xCoreID = ( BaseType_t ) pxTCB->xTaskRunState;
 *         prvYieldCore( xCoreID );
 *     }
 * }
 * @endcode
 * ( xSchedulerRunning != pdFALSE ) is true.
 * ( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE ) is false.
 */
void test_coverage_vTaskPreemptionEnable_scheduler_running( void )
{
    TCB_t xTaskTCB = { NULL };

    /* Setup variable. */
    xTaskTCB.xPreemptionDisable = pdTRUE;
    xTaskTCB.xTaskRunState = -1; /* taskTASK_NOT_RUNNING. */

    xSchedulerRunning = pdTRUE;
    
    /* Clear callback in commonSetUp. */
    vFakePortEnterCriticalSection_StubWithCallback( NULL );
    vFakePortExitCriticalSection_StubWithCallback( NULL );

    /* Expections. */
    vFakePortEnterCriticalSection_Expect();
    vFakePortExitCriticalSection_Expect();

    /* API call. */
    vTaskPreemptionEnable( &xTaskTCB );

    /* Validation. */
    TEST_ASSERT( xTaskTCB.xPreemptionDisable == pdFALSE );
}

/**
 * @brief vTaskPreemptionEnable - Enable preemption of a task with NULL handle
 *
 * The xPreemptionDisable of the task on core 0 will be set to pdFALSE.
 *
 * <b>Coverage</b>
 * @code{c}
 * pxTCB = prvGetTCBFromHandle( xTask );
 * @endcode
 * prvGetTCBFromHandle( xTask ) parameter xTask is NULL.
 */
void test_coverage_vTaskPreemptionEnable_null_handle( void )
{
    TCB_t xTaskTCB = { NULL };
    UBaseType_t uxInterruptMask = 0x12345678;

    xTaskTCB.xPreemptionDisable  = pdTRUE;
    xTaskTCB.xTaskRunState = -1; /* taskTASK_NOT_RUNNING. */
    pxCurrentTCBs[ 0 ] = &xTaskTCB;

    /* Clear callback in commonSetUp. */
    vFakePortEnterCriticalSection_StubWithCallback( NULL );
    vFakePortExitCriticalSection_StubWithCallback( NULL );
    ulFakePortSetInterruptMask_StopIgnore();
    vFakePortClearInterruptMask_StubWithCallback( NULL );
    vFakePortGetCoreID_StubWithCallback( NULL );

    /* Expections. */
    vFakePortEnterCriticalSection_Expect();
    ulFakePortSetInterruptMask_ExpectAndReturn( uxInterruptMask );
    vFakePortGetCoreID_ExpectAndReturn( 0 );
    vFakePortClearInterruptMask_Expect( uxInterruptMask );
    vFakePortExitCriticalSection_Expect();

    /* API call. */
    vTaskPreemptionEnable( NULL );

    /* Expection. */
    TEST_ASSERT( pxCurrentTCBs[ 0 ]->xPreemptionDisable == pdFALSE );
}

/**
 * @brief vTaskPreemptionEnable - Enable preemption of a task when scheduler is running
 *
 * The xPreemptionDisable of the task will be set to pdFALSE. The xTaskRunState is
 * set to greater than ( configNUMBER_OF_CORES - 1 ).
 *
 * <b>Coverage</b>
 * @code{c}
 * if( xSchedulerRunning != pdFALSE )
 * {
 *     if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 *     {
 *         xCoreID = ( BaseType_t ) pxTCB->xTaskRunState;
 *         prvYieldCore( xCoreID );
 *     }
 * }
 * @endcode
 * ( xSchedulerRunning != pdFALSE ) is true.
 * ( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE ) is false.
 */
void test_coverage_vTaskPreemptionEnable_task_not_running_gt_cores( void )
{
    TCB_t xTaskTCB = { NULL };

    /* Setup variables. */
    xTaskTCB.xPreemptionDisable  = pdTRUE;
    xTaskTCB.xTaskRunState = configNUMBER_OF_CORES;

    xSchedulerRunning = pdTRUE;

    /* Clear callback in commonSetUp. */
    vFakePortEnterCriticalSection_StubWithCallback( NULL );
    vFakePortExitCriticalSection_StubWithCallback( NULL );

    /* Expections. */
    vFakePortEnterCriticalSection_Expect();
    vFakePortExitCriticalSection_Expect();

    /* API call. */
    vTaskPreemptionEnable( &xTaskTCB );

    /* Validation. */
    TEST_ASSERT( xTaskTCB.xPreemptionDisable == pdFALSE );
}

/**
 * @brief vTaskPreemptionEnable - Enable preemption of a task when the task is running
 *
 * The xPreemptionDisable of the task will be set to pdFALSE.
 *
 * <b>Coverage</b>
 * @code{c}
 * if( xSchedulerRunning != pdFALSE )
 * {
 *     if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 *     {
 *         xCoreID = ( BaseType_t ) pxTCB->xTaskRunState;
 *         prvYieldCore( xCoreID );
 *     }
 * }
 * @endcode
 * ( xSchedulerRunning != pdFALSE ) is true.
 * ( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE ) is true.
 */
void test_coverage_vTaskPreemptionEnable_task_running( void )
{
    TCB_t xTaskTCB = { NULL };

    /* Setup variables. */
    xTaskTCB.xPreemptionDisable  = pdTRUE;
    xTaskTCB.xTaskRunState = 0;

    xSchedulerRunning = pdTRUE;

    /* Clear callback in commonSetUp. */
    vFakePortCheckIfInISR_StopIgnore();
    vFakePortEnterCriticalSection_StubWithCallback( NULL );
    vFakePortExitCriticalSection_StubWithCallback( NULL );
    vFakePortGetCoreID_StubWithCallback( NULL );

    /* Expections. */
    vFakePortEnterCriticalSection_Expect();
    vFakePortCheckIfInISR_ExpectAndReturn( 1 ); /* Expection in prvYieldCore. */
    vFakePortGetCoreID_ExpectAndReturn( 0 );    /* Expection in prvYieldCore. */
    vFakePortExitCriticalSection_Expect();

    /* API call. */
    vTaskPreemptionEnable( &xTaskTCB );

    /* Validation. */
    TEST_ASSERT( xTaskTCB.xPreemptionDisable == pdFALSE );
}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_CORE_AFFINITY                         1

Coverage for 
    UBaseType_t vTaskCoreAffinityGet( const TaskHandle_t xTask )
        with a created task handel for xTask
*/
void test_task_Core_Affinity_Get( void )
{
    //Reset all the globals to gain the deafult null state
    memset(xTaskHandles, 0, sizeof(TaskHandle_t) * configNUMBER_OF_CORES );

    uint32_t i;

    /* Create tasks of equal priority */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    /* Verify tasks are running */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }

    /* task T0 */
    vTaskCoreAffinityGet( xTaskHandles[0] );

}
/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_CORE_AFFINITY                         1

Coverage for 
    UBaseType_t vTaskCoreAffinityGet( const TaskHandle_t xTask )
        with a NULL for xTask
*/
void test_task_Core_Affinity_Get_with_null_task( void )
{
    //Reset all the globals to gain the deafult null state
    memset(xTaskHandles, 0, sizeof(TaskHandle_t) * configNUMBER_OF_CORES );

    uint32_t i;

    /* Create tasks of equal priority */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    /* Verify tasks are running */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        verifySmpTask( &xTaskHandles[i], eRunning, i );
    }
    vTaskCoreAffinityGet( NULL );
}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_TRACE_FACILITY                         1

Coverage for 
    UBaseType_t uxTaskGetTaskNumber( TaskHandle_t xTask )
    and
    void vTaskSetTaskNumber( TaskHandle_t xTask,
                             const UBaseType_t uxHandle )
    
    Sets a non-null task's number as taskNumber and then fetches it
*/
void test_task_set_get_task_number_not_null_task( void )
{
    TaskHandle_t xTaskHandles[3] = { NULL };
    uint32_t i;
    UBaseType_t taskNumber = 1;
    UBaseType_t returntaskNumber;


    /* Create  tasks of equal priority */
    for (i = 0; i < (2); i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );
    }

    /* Create a single equal priority task */   
    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );

    vTaskStartScheduler();

    vTaskSetTaskNumber(xTaskHandles[0], taskNumber);
    
    /* Set CPU core affinity on the last task for the last CPU core */
    returntaskNumber = uxTaskGetTaskNumber(xTaskHandles[0]);
    
    TEST_ASSERT_EQUAL( returntaskNumber,  taskNumber);
    
}
/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_TRACE_FACILITY                         1

Coverage for 
    UBaseType_t uxTaskGetTaskNumber( TaskHandle_t xTask )
    and
    void vTaskSetTaskNumber( TaskHandle_t xTask,
                             const UBaseType_t uxHandle )
    
    
    Sets a null task's number as taskNumber and then fetches it 
*/
void test_task_set_get_task_number_null_task( void )
{
    TaskHandle_t xTaskHandles[3] = { NULL };
    UBaseType_t taskNumber = 1;
    UBaseType_t returntaskNumber;

    vTaskStartScheduler();
    
    vTaskSetTaskNumber(xTaskHandles[0], taskNumber);
    
    /* Set CPU core affinity on the last task for the last CPU core */
    returntaskNumber = uxTaskGetTaskNumber(xTaskHandles[0]);
    
    TEST_ASSERT_EQUAL( 0U , returntaskNumber);
}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define INCLUDE_uxTaskGetStackHighWaterMark             1

Coverage for 
    UBaseType_t uxTaskGetStackHighWaterMark( TaskHandle_t xTask )
        By passing a valid created Task
*/
void test_task_get_stack_high_water_mark( void )
{
    TaskHandle_t xTaskHandles[1] = { NULL };

    /* Create  tasks  */
    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[0] );

    vTaskStartScheduler();

    uxTaskGetStackHighWaterMark(xTaskHandles[0]);

}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define INCLUDE_uxTaskGetStackHighWaterMark             1

Coverage for 
        UBaseType_t uxTaskGetStackHighWaterMark( TaskHandle_t xTask )
        By passing a NULL as a  Task
*/
void test_task_get_stack_high_water_mark_NULL_task( void )
{
    TaskHandle_t xTaskHandles[1] = { NULL };

    /* Create  tasks  */
    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[0] );

    vTaskStartScheduler();

    //NULL task for code coverage
    uxTaskGetStackHighWaterMark( NULL );

}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define INCLUDE_uxTaskGetStackHighWaterMark              1
    #define configUSE_MUTEXES                                1
Coverage for: 
        TaskHandle_t xTaskGetCurrentTaskHandleCPU( BaseType_t xCoreID )
*/
void test_task_get_current_task_handle_cpu ( void )
{
    TaskHandle_t xTaskHandles[1] = { NULL };

    /* Create  tasks  */
    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[0] );

    vTaskStartScheduler();

    xTaskGetCurrentTaskHandleCPU( vFakePortGetCoreID() );

}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_TRACE_FACILITY                         1
    #define configUSE_STATS_FORMATTING_FUNCTIONS             1

Coverage for: 
        void vTaskList( char * pcWriteBuffer )
        if( pxTaskStatusArray != NULL ) = False
*/
void test_v_task_list_case_no_task_created( void )
{
    static char	buff[ 800 ] = { 0 };
 
    //Call the List
    vTaskList(buff);

}
/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_TRACE_FACILITY                         1
    #define configUSE_STATS_FORMATTING_FUNCTIONS             1

Coverage for: 
        void vTaskList( char * pcWriteBuffer )
        case eDeleted
*/
void test_v_task_list_case_eDeleted( void )
{
    static char	buff[ 800 ] = { 0 };

    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES+1] = { NULL };

    uint32_t i;

    /* Create tasks of equal priority for all available CPU cores */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 3, &xTaskHandles[i] );
    }

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );

    vTaskStartScheduler();
 
    vTaskDelete(xTaskHandles[0]);

    //Call the List
    vTaskList(buff);

    /* Delete all priority task responsibly*/
    for (i = 1; i < configNUMBER_OF_CORES; i++) {
        vTaskDelete(xTaskHandles[i]);
    }

}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_TRACE_FACILITY                         1
    #define configUSE_STATS_FORMATTING_FUNCTIONS             1

Coverage for: 
        void vTaskList( char * pcWriteBuffer )
        case eSuspended
*/
void test_v_task_list_case_eSuspended( void )
{
    static char	buff[ 800 ] = { 0 };

    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES+1] = { NULL };

    uint32_t i;

    /* Create tasks of equal priority for all available CPU cores */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 3, &xTaskHandles[i] );
    }

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );

    vTaskStartScheduler();
 
    vTaskSuspend(xTaskHandles[1]);

    //Call the List
    vTaskList(buff);


    /* Delete all priority task responsibly*/
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        vTaskDelete(xTaskHandles[i]);
    }

}


/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_TRACE_FACILITY                         1
    #define configUSE_STATS_FORMATTING_FUNCTIONS             1

Coverage for: 
        void vTaskList( char * pcWriteBuffer )
        case eBlocked
*/
void test_v_task_list_case_eblocked( void )
{
    static char	buff[ 800 ] = { 0 };

    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };

    uint32_t i;

    /* Create tasks of equal priority for all available CPU cores */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    /* Delay the task running on core ID 0 for 1 ticks. The task will be put into pxDelayedTaskList and added back to ready list after 1 tick. */
    vTaskDelay( 1 );

    //Call the List
    vTaskList(buff);

     /* Delete all priority task responsibly*/
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        vTaskDelete(xTaskHandles[i]);
    }
}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_TRACE_FACILITY                         1
    #define configUSE_STATS_FORMATTING_FUNCTIONS             1

Coverage for: 
        void vTaskList( char * pcWriteBuffer )
        and
        static char * prvWriteNameToBuffer( char * pcBuffer,
                                            const char * pcTaskName )
*/
void test_v_task_list( void )
{
    static char	buff[ 800 ] = { 0 };

    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    uint32_t i;

    /* Create tasks of equal priority for all available CPU cores */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    vTaskList(buff);

    /* Delete all priority task responsibly*/
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        vTaskDelete(xTaskHandles[i]);
    }
}




/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define INCLUDE_xTaskDelayUntil                          1

Coverage for: 
        BaseType_t xTaskDelayUntil( TickType_t * const pxPreviousWakeTime,
                                    const TickType_t xTimeIncrement )
*/
void test_task_delay_until_with_config_assert( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    uint32_t i;
    TickType_t previousWakeTime = xTickCount - 3;

    /* Create tasks of equal priority for all available CPU cores */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    xTaskDelayUntil( &previousWakeTime , 4 );
    
}

/**
 * @brief prvAddNewTaskToReadyList - add a newly created task to the list of ready tasks
 *
 * This test creates two tasks, the second after suspending the first and then
 * starts the scheduler.
 *
 * <b>Coverage</b>
 * @code{c}
 * if( uxCurrentNumberOfTasks == ( UBaseType_t ) 1 )
 * @endcode
 * As two tasks arecreated, this covers both branches of the above conditional
 * in addition to the function body.
 */
void test_coverage_prvAddNewTaskToReadyList_create_two_tasks_with_the_first_suspended( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0] );
    vTaskSuspend(xTaskHandles[0]);

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[1] );

    vTaskStartScheduler();
}

#if 0
/**
 * @brief prvAddNewTaskToReadyList - add a new idle task to the list of ready tasks
 *
 * This test creates more tasks than ther are cores in order to test the
 * branch related to the limit condition of the for loop.
 *
 * <b>Coverage</b>
 * @code{c}
 * for( xCoreID = 0; xCoreID < configNUMBER_OF_CORES; xCoreID++ )
 * @endcode
 * for loop condition ( xCoreID < configNUMBER_OF_CORES ) is false.
 */
void coverage_prvAddNewTaskToReadyList_create_more_idle_tasks_than_cores( void )
{
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1 ] = { 0 };
    uint32_t i;

    /* Setup the variables and structure. */
    /* Initialize the idle priority ready list and set top ready priority to idle priority. */
    vListInitialise( &( pxReadyTasksLists[ tskIDLE_PRIORITY ] ) );
    uxTopReadyPriority = tskIDLE_PRIORITY;
    uxCurrentNumberOfTasks = 0;
    xSchedulerRunning = pdFALSE;

    /* Create idle tasks and add it into the ready list. Create one more idle priority level
     * in the loop. */
    for( i = 0; i < ( configNUMBER_OF_CORES + 1U ); i++ )
    {
        xTaskTCBs[ i ].uxPriority = tskIDLE_PRIORITY;
        xTaskTCBs[ i ].xStateListItem.pvOwner = &xTaskTCBs[ i ];
        xTaskTCBs[ i ].uxCoreAffinityMask = ( ( 1U << configNUMBER_OF_CORES ) - 1U );
        xTaskTCBs[ i ].uxTaskAttributes = -1;
        if( i < configNUMBER_OF_CORES )
        {
            /* Create idle tasks with equal number of cores. */
            pxCurrentTCBs[ i ] = &xTaskTCBs[ i ];
            xTaskTCBs[ i ].xTaskRunState = i;
            xTaskTCBs[ i ].xStateListItem.pxContainer = &pxReadyTasksLists[ tskIDLE_PRIORITY ];
            listINSERT_END( &pxReadyTasksLists[ tskIDLE_PRIORITY ], &xTaskTCBs[ i ].xStateListItem );
            uxCurrentNumberOfTasks = uxCurrentNumberOfTasks + 1;
        }
        else
        {
            /* Create one more idle task to be added to ready list. */
            xTaskTCBs[ i ].xTaskRunState = -1;  /* Set run state to taskTASK_NOT_RUNNING. */
        }
    }

    /* API calls. */
    prvAddNewTaskToReadyList( &xTaskTCBs[ configNUMBER_OF_CORES ] );

    /* Validateions. The run state of this task is still taskNOT_RUNNING ( -1 ). */
    configASSERT( xTaskTCBs[ configNUMBER_OF_CORES + 1U ].xTaskRunState == -1 );
}
#endif

/**
 * @brief vTaskCoreAffinitySet - limit a task to a set of cores via a bitmask.
 *
 * This test calles vTaskCoreAffinitySet with a NULL task, implicitly referencing the
 * current task and setting the mask to 0xFF with the secheduler running.
 *
 * <b>Coverage</b>
 * @code{c}
 * pxTCB = prvGetTCBFromHandle( xTask );
 * ...
 * if( xSchedulerRunning != pdFALSE )
 *          {
 *              if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 * ...
 * @endcode
 */
void test_coverage_vTaskCoreAffinitySet_task_core_affinity_set_task_implied( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    UBaseType_t xidx;

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0] );

    vTaskStartScheduler();

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vTaskCoreAffinitySet(NULL, (UBaseType_t)0xFF);
}

/**
 * @brief vTaskCoreAffinitySet - limit a task to a set of cores via a bitmask.
 *
 * This test calles vTaskCoreAffinitySet with an explicit task reference
 * setting the mask to 0xFF adn then starting the scheduler.
 *
 * <b>Coverage</b>
 * @code{c}
 * pxTCB = prvGetTCBFromHandle( xTask );
 * ...
 * if( xSchedulerRunning != pdFALSE )
 *          {
 *              if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 * ...
 * @endcode
 */
void test_coverage_vTaskCoreAffinitySet_task_core_affinity_set_task_explicit( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0]);
    vTaskCoreAffinitySet(xTaskHandles[0], (UBaseType_t)0xFF);

    vTaskStartScheduler();
}

/**
 * @brief vTaskCoreAffinitySet - limit a task to a set of cores via a bitmask.
 *
 * This test calles vTaskCoreAffinitySet with an explicit task reference
 * setting the mask to one value initially, and then changing the mask while
 * the scheduler is active and the task is running.
 *
 * <b>Coverage</b>
 * @code{c}
 *                   if( ( uxCoreAffinityMask & ( 1 << xCoreID ) ) == 0 )
 *                   {
 *                       prvYieldCore( xCoreID );
 *                   }
 * ...
 * @endcode
 */
void test_coverage_vTaskCoreAffinitySet_task_core_affinity_change_while_running( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    UBaseType_t xidx;

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0]);
    vTaskCoreAffinitySet(xTaskHandles[0], (UBaseType_t)0x1);

    vTaskStartScheduler();

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vTaskCoreAffinitySet(xTaskHandles[0], (UBaseType_t)0x2);

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }
}

/**
 * @brief vTaskCoreAffinitySet - limit a task to a set of cores via a bitmask.
 *
 * This test calles vTaskCoreAffinitySet with an explicit task reference
 * setting the mask to one value initially, and then re-setting the mask to
 * the same value while the scheduler is active and the task is running.
 *
 * <b>Coverage</b>
 * @code{c}
 *                   if( ( uxCoreAffinityMask & ( 1 << xCoreID ) ) == 0 )
 *                   {
 *                       prvYieldCore( xCoreID );
 *                   }
 * ...
 * @endcode
 */
void test_coverage_vTaskCoreAffinitySet_task_core_affinity_change_while_suspended( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    UBaseType_t xidx;

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0]);
    vTaskCoreAffinitySet(xTaskHandles[0], (UBaseType_t)0x1);

    vTaskStartScheduler();

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vTaskSuspend(xTaskHandles[0]);

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vTaskCoreAffinitySet(xTaskHandles[0], (UBaseType_t)0x2);

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vTaskCoreAffinitySet(xTaskHandles[0], (UBaseType_t)0x2);

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }
}
