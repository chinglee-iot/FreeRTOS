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
extern List_t xSuspendedTaskList;
extern List_t xPendingReadyList;
extern volatile UBaseType_t uxTopReadyPriority;
extern volatile BaseType_t xYieldPendings[ configNUMBER_OF_CORES ];


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

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_CORE_AFFINITY                         1
    #define configUSE_TASK_PREEMPTION_DISABLE               1

Coverage for 
        static void vTaskPreemptionEnable( void );
        covers the deafult state when the function is just called
*/
void test_task_preemption_enable( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
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
    vTaskPreemptionEnable( xTaskHandles[0] );

}

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_CORE_AFFINITY                         1
    #define configUSE_TASK_PREEMPTION_DISABLE               1

Coverage for 
        static void vTaskPreemptionEnable( void );
        covers the deafult state when xSchedulerRunning is set to False
*/
void test_task_preemption_enable_branch_xSchedulerRunning_False( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    uint32_t i;

    /* Create tasks of equal priority */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }

    //Tasks are created and a task is passed but scheduler is never ran
    vTaskPreemptionEnable( xTaskHandles[0] );

} 

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_CORE_AFFINITY                         1
    #define configUSE_TASK_PREEMPTION_DISABLE               1

Coverage for 
        static void vTaskPreemptionEnable( void );
        covers the deafult state when NULL task is passed
*/
void test_task_preemption_enable_branch_NULL_task( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    uint32_t i;

    /* Create tasks of equal priority */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    /* task T0 */
    vTaskPreemptionEnable( NULL );

} 


/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_CORE_AFFINITY                         1
    #define configUSE_TASK_PREEMPTION_DISABLE               1

Coverage for 
        static void vTaskPreemptionEnable( void );
        covers the deafult state when passed task's xTaskRunState task is greater than number of cores
*/
void test_task_preemption_enable_branch_Rand_task( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    uint32_t i;

    /* Create tasks of equal priority */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    /* task T0 */
    xTaskHandles[0]->xTaskRunState = configNUMBER_OF_CORES+1;
    vTaskPreemptionEnable( xTaskHandles[0] );

} 

/*
The kernel will be configured as follows:
    #define configNUMBER_OF_CORES                               (N > 1)
    #define configUSE_CORE_AFFINITY                         1
    #define configUSE_TASK_PREEMPTION_DISABLE               1

Coverage for 
        static void vTaskPreemptionEnable( void );
        covers the deafult state when passed task's xTaskRunState task is negative
*/
void test_task_preemption_enable_branch_negative_task( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    uint32_t i;

    /* Create tasks of equal priority */
    for (i = 0; i < configNUMBER_OF_CORES; i++) {
        xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i] );
    }

    vTaskStartScheduler();

    /* task T0 */
    xTaskHandles[0]->xTaskRunState = -1;
    vTaskPreemptionEnable( xTaskHandles[0] );

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
 * @brief xTaskGenericNotify - function to notify a task.
 *
 * <b>Coverage</b>
 * @code{c}
 *              case eNoAction:
 *              ...
 *                    break;
 *
 * @endcode
 *
 *  Cover the eNoAction action type with an explicit task specified.
 */
void test_coverage_xTaskGenericNotify_with_eAction_equalto_eNoAction( void )
{
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t *prevValue = NULL;
    uint32_t ulValue = 0;
    uint32_t i;
    BaseType_t xReturn;

    /* Setup the variables and structure. */
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskTCBs[ i ].uxPriority = 1;
        xTaskTCBs[ i ].xTaskRunState = i;
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

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, ulValue, eNoAction, prevValue);

    /* Validations. In single priority test, the calling core is requested to yield
     * since a higher priority task is resumed. */
    TEST_ASSERT( xReturn == pdPASS );
}

/**
 * @brief xTaskGenericNotify - function to notify a task.
 *
 * <b>Coverage</b>
 * @code{c}
 *              case eNoAction:
 *              ...
 *                    break;
 *
 * @endcode
 *
 *  Cover the eNoAction action type, while requesting the previous value
 *  and with an explicit task specified.
 */
void test_coverage_xTaskGenericNotify_with_eAction_equalto_eNoAction_prevValue( void )
{
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t prevValue;
    uint32_t ulValue = 0;
    uint32_t i;
    BaseType_t xReturn;

    /* Setup the variables and structure. */
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskTCBs[ i ].uxPriority = 1;
        xTaskTCBs[ i ].xTaskRunState = i;
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

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, ulValue, eNoAction, &prevValue);

    /* Validations. In single priority test, the calling core is requested to yield
     * since a higher priority task is resumed. */
    TEST_ASSERT( xReturn == pdPASS );
}

/**
 * @brief xTaskGenericNotify - function to notify a task.
 *
 * <b>Coverage</b>
 * @code{c}
 *         case eSetBits:
 *                    pxTCB->ulNotifiedValue[ uxIndexToNotify ] |= ulValue;
 *                    break;
 * @endcode
 *
 *  Cover the eSetBits action type, while requesting the previous value
 *  and with an explicit task specified.
 */
void test_coverage_xTaskGenericNotify_with_eAction_equalto_eSetBits_prevValue( void )
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

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eSetValueWithOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x3, eSetBits, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

    /* Validations. In single priority test, the calling core is requested to yield
     * since a higher priority task is resumed. */
    TEST_ASSERT_EQUAL_UINT32(0x3, prevValue);
    TEST_ASSERT( xReturn == pdPASS );
}

/**
 * @brief xTaskGenericNotify - function to notify a task.
 *
 * <b>Coverage</b>
 * @code{c}
 *         case eIncrement:
 *                    ( pxTCB->ulNotifiedValue[ uxIndexToNotify ] )++;
 *                    break;
 * @endcode
 *
 *  Cover the eIncrement action type, while requesting the previous value
 *  and with an explicit task specified.
 */
void test_coverage_xTaskGenericNotify_with_eAction_equalto_eIncrement_prevValue( void )
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

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x2, eSetValueWithOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eIncrement, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

    /* Validations. In single priority test, the calling core is requested to yield
     * since a higher priority task is resumed. */
    TEST_ASSERT_EQUAL_UINT32(0x3, prevValue);
    TEST_ASSERT( xReturn == pdPASS );
}

/**
 * @brief xTaskGenericNotify - function to notify a task.
 *
 * <b>Coverage</b>
 * @code{c}
 *     case eSetValueWithOverwrite:
 *                    pxTCB->ulNotifiedValue[ uxIndexToNotify ] = ulValue;
 *                    break;
 * @endcode
 *
 *  Cover the eSetValueWithOverwrite action type, while requesting the previous value
 *  and with an explicit task specified.
 */
void test_coverage_xTaskGenericNotify_with_eAction_equalto_eSetValueWithOverwrite_prevValue( void )
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

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x2, eSetValueWithOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

    /* Validations. In single priority test, the calling core is requested to yield
     * since a higher priority task is resumed. */
    TEST_ASSERT_EQUAL_UINT32(0x2, prevValue);
    TEST_ASSERT( xReturn == pdPASS );
}

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
 *  Cover the eSetValueWithoutOverwrite action type, while requesting the previous value
 *  and with an explicit task specified. Branch 1 of 2.
 */
void test_coverage_xTaskGenericNotify_with_eAction_equalto_eSetValueWithoutOverwrite_prevValue( void )
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

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x2, eSetValueWithoutOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

    /* Validations. In single priority test, the calling core is requested to yield
     * since a higher priority task is resumed. */
    TEST_ASSERT_EQUAL_UINT32(0x2, prevValue);
    TEST_ASSERT( xReturn == pdPASS );
}

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
void test_coverage_xTaskGenericNotify_with_eAction_equalto_eSetValueWithoutOverwrite_branch_taskNOTIFICATION_RECEIVED( void )
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

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x2, eSetValueWithoutOverwrite, &prevValue);
    xTaskTCBs[0].ucNotifyState[ xidx ] = /*taskNOTIFICATION_RECEIVED*/ ( ( uint8_t ) 2 );
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x3, eSetValueWithoutOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

    /* Validations. In single priority test, the calling core is requested to yield
     * since a higher priority task is resumed. */
    TEST_ASSERT_EQUAL_UINT32(0x2, prevValue);
    TEST_ASSERT( xReturn == pdPASS );
}

/**
 * @brief vTaskGetInfo - populate TaskStatus_t and eTaskState
 *
 * <b>Coverage</b>
 * @code{c}
 *        pxTCB = prvGetTCBFromHandle( xTask );
 *          ...
 * @endcode
 *
 * Cover the case where xTask is NULL, and the current task is implicitly
 * referenced and returned by prvGetTCBFromHandle(...);
 */
void test_coverage_vTaskGetInfo_implicit_task( void )
{
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
    uint32_t i;
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eReady;

    /* Setup the variables and structure. */
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskTCBs[ i ].uxPriority = 1;
        xTaskTCBs[ i ].xTaskRunState = i;
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

    vTaskGetInfo( NULL, &pxTaskStatus, xFreeStackSpace, taskState);
    TEST_ASSERT_EQUAL((BaseType_t)1, pxTaskStatus.uxCurrentPriority);
}

/**
 * @brief vTaskGetInfo - populate TaskStatus_t and eTaskState
 *
 * <b>Coverage</b>
 * @code{c}
 *        pxTCB = prvGetTCBFromHandle( xTask );
 *          ...
 * @endcode
 *
 * Cover the case where xTask is specified rather than NULL.
 */
void test_coverage_vTaskGetInfo_explicit_task( void )
{
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
    uint32_t i;
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eReady;

    /* Setup the variables and structure. */
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskTCBs[ i ].uxPriority = 1;
        xTaskTCBs[ i ].xTaskRunState = i;
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

    vTaskGetInfo( &xTaskTCBs[0], &pxTaskStatus, xFreeStackSpace, taskState);
    TEST_ASSERT_EQUAL((BaseType_t)1, pxTaskStatus.uxCurrentPriority);
}

/**
 * @brief vTaskGetInfo - populate TaskStatus_t and eTaskState
 *
 * <b>Coverage</b>
 * @code{c}
 *                if( listLIST_ITEM_CONTAINER( &( pxTCB->xEventListItem ) ) != NULL )
 *                {
 *                   pxTaskStatus->eCurrentState = eBlocked;
 *                }
 *                ...
 * @endcode
 *
 * Cover the case where the task is blocked.
 */
void test_coverage_vTaskGetInfo_blocked_task( void )
{
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
    uint32_t i;
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eSuspended;

    /* Setup the variables and structure. */
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskTCBs[ i ].uxPriority = 1;
        xTaskTCBs[ i ].xTaskRunState = i;
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

    xTaskTCBs[0].xTaskRunState = -1;
    xTaskTCBs[0].xEventListItem.pxContainer = (struct xLIST *) 1;
    vTaskGetInfo( &xTaskTCBs[0], &pxTaskStatus, xFreeStackSpace, taskState);
    TEST_ASSERT_EQUAL((BaseType_t)1, pxTaskStatus.uxCurrentPriority);
}

/**
 * @brief vTaskGetInfo - populate TaskStatus_t and eTaskState
 *
 * <b>Coverage</b>
 * @code{c}
 *       if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 *            {
 *                pxTaskStatus->eCurrentState = eRunning;
 *            }
 *            ...
 * @endcode
 *
 * Cover the case in the taskTASK_IS_RUNNING() macro where the xTaskRunState
 * is out of bounds.
 */
void test_coverage_vTaskGetInfo_oob_xTaskRunState( void )
{
   TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
    uint32_t i;
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eSuspended;

    /* Setup the variables and structure. */
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );
    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskTCBs[ i ].uxPriority = 1;
        xTaskTCBs[ i ].xTaskRunState = i;
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

    xTaskTCBs[0].xTaskRunState = configNUMBER_OF_CORES;
    vTaskGetInfo( &xTaskTCBs[0], &pxTaskStatus, xFreeStackSpace, taskState);
    TEST_ASSERT_EQUAL((BaseType_t)1, pxTaskStatus.uxCurrentPriority);
}

/**
 * @brief vTaskGetInfo - populate TaskStatus_t and eTaskState
 *
 * <b>Coverage</b>
 * @code{c}
 *       if( taskTASK_IS_RUNNING( pxTCB ) == pdTRUE )
 *            {
 *                pxTaskStatus->eCurrentState = eRunning;
 *            }
 *            ...
 * @endcode
 *
 * Cover the case where xFreeStackSpace is pdFALSE, avoiding the free
 * stack space query.
 */
void test_coverage_vTaskGetInfo_get_free_stack_space( void )
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };
    TaskStatus_t pxTaskStatus;
    UBaseType_t xidx;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eReady;

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0] );

    vTaskStartScheduler();

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vTaskGetInfo( xTaskHandles[0], &pxTaskStatus, xFreeStackSpace, taskState);
}

/**
 * @brief vTaskResume - resume a suspended task.
 *
 * <b>Coverage</b>
 * @code{c}
 *                if( prvTaskIsTaskSuspended( pxTCB ) != pdFALSE )
 *                {
 *            ...
 * @endcode
 *
 * Cover the case where the task requested to resume has not
 * been suspended.
 */
void test_coverage_vTaskResume_task_not_suspended( void )
{
    UBaseType_t xidx;
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0] );

    vTaskStartScheduler();

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vTaskResume( xTaskHandles[0] );
}

/*
* @brief xTaskResumeFromISR - resume a suspended task in the ISR context
*
* <b>Coverage</b>
* @code{c}
* ...
*        uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
* ...
* @endcode
*
* Cover the primary path where an ISR context task is being resumed
* that has not been suspended.
*/
void test_coverage_xTaskResumeFromISR ( void )
{
    UBaseType_t xidx;
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0] );

    vTaskStartScheduler();

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vFakePortAssertIfInterruptPriorityInvalid_Ignore();
    xTaskResumeFromISR( xTaskHandles[0] );
}

/*
* @brief xTaskResumeFromISR - resume a suspended task in the ISR context
*
* <b>Coverage</b>
* @code{c}
* ...
*        uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
* ...
* @endcode
*
* Cover the primary path where an ISR context task is being resumed.
*/
void test_coverage_xTaskResumeFromISR_suspended_suspendall ( void )
{
    UBaseType_t xidx;
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = { NULL };

    xTaskCreate( vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[0] );

    vTaskStartScheduler();

    for (xidx = 0; xidx < configNUMBER_OF_CORES ; xidx++) {
        xTaskIncrementTick_helper();
    }

    vTaskSuspend( xTaskHandles[0] );

    vFakePortAssertIfInterruptPriorityInvalid_Ignore();
    xTaskResumeFromISR( xTaskHandles[0] );
}
