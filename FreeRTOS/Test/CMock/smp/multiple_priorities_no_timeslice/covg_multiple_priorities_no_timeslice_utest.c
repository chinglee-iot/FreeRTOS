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
#define tskSTACK_FILL_BYTE      ( 0xa5U )


/* ===========================  EXTERN VARIABLES  =========================== */
extern volatile UBaseType_t uxCurrentNumberOfTasks;
extern volatile UBaseType_t uxDeletedTasksWaitingCleanUp;
extern volatile UBaseType_t uxSchedulerSuspended;
extern volatile TCB_t *  pxCurrentTCBs[ configNUMBER_OF_CORES ];
extern volatile BaseType_t xSchedulerRunning;
extern volatile TickType_t xTickCount;
extern List_t xSuspendedTaskList;
extern List_t xPendingReadyList;
extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t ulValue = 0;
    BaseType_t xReturn;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, ulValue, eNoAction, NULL);

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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t prevValue;
    uint32_t ulValue = 0;
    BaseType_t xReturn;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, ulValue, eNoAction, &prevValue);

    TEST_ASSERT( xReturn == pdPASS );
    TEST_ASSERT_EQUAL(0, prevValue);
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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t prevValue;
    BaseType_t xReturn;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eSetValueWithOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x3, eSetBits, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t prevValue;
    BaseType_t xReturn;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x2, eSetValueWithOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eIncrement, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t prevValue;
    BaseType_t xReturn;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x2, eSetValueWithOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t prevValue;
    BaseType_t xReturn;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x2, eSetValueWithoutOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    UBaseType_t xidx = 0;
    uint32_t prevValue;
    BaseType_t xReturn;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x2, eSetValueWithoutOverwrite, &prevValue);
    xTaskTCBs[0].ucNotifyState[ xidx ] = /*taskNOTIFICATION_RECEIVED*/ ( ( uint8_t ) 2 );
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x3, eSetValueWithoutOverwrite, &prevValue);
    xReturn = xTaskGenericNotify( &xTaskTCBs[0], xidx, 0x0, eNoAction, &prevValue);

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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eReady;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eReady;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    vTaskGetInfo( &xTaskTCBs[0], &pxTaskStatus, xFreeStackSpace, taskState);
    TEST_ASSERT_EQUAL((UBaseType_t)0, pxTaskStatus.xTaskNumber);
    TEST_ASSERT_EQUAL(eRunning, pxTaskStatus.eCurrentState);
    TEST_ASSERT_EQUAL((UBaseType_t)1, pxTaskStatus.uxCurrentPriority);
    TEST_ASSERT_EQUAL((UBaseType_t)0, pxTaskStatus.uxBasePriority);
    TEST_ASSERT_EQUAL(0, pxTaskStatus.usStackHighWaterMark);
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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eSuspended;

    /* Setup the variables and structure. */
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );

    xTaskTCBs[ 0 ].uxPriority = 2;
    xTaskTCBs[ 0 ].xTaskRunState = -1;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];
    listINSERT_END( &xSuspendedTaskList, &xTaskTCBs[ 0 ].xStateListItem );

    uxTopReadyPriority = 2;
    uxSchedulerSuspended = pdTRUE;

    xTaskTCBs[0].xEventListItem.pxContainer = (struct xLIST *) 1;
    vTaskGetInfo( &xTaskTCBs[0], &pxTaskStatus, xFreeStackSpace, taskState);
    TEST_ASSERT_EQUAL((UBaseType_t)0, pxTaskStatus.xTaskNumber);
    TEST_ASSERT_EQUAL(eBlocked, pxTaskStatus.eCurrentState);
    TEST_ASSERT_EQUAL((UBaseType_t)2, pxTaskStatus.uxCurrentPriority);
    TEST_ASSERT_EQUAL((UBaseType_t)0, pxTaskStatus.uxBasePriority);
    TEST_ASSERT_EQUAL(0, pxTaskStatus.usStackHighWaterMark);
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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdFALSE;
    eTaskState taskState = eSuspended;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = configNUMBER_OF_CORES;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    vTaskGetInfo( &xTaskTCBs[0], &pxTaskStatus, xFreeStackSpace, taskState);
    TEST_ASSERT_EQUAL((UBaseType_t)0, pxTaskStatus.xTaskNumber);
    TEST_ASSERT_EQUAL(eSuspended, pxTaskStatus.eCurrentState);
    TEST_ASSERT_EQUAL((UBaseType_t)1, pxTaskStatus.uxCurrentPriority);
    TEST_ASSERT_EQUAL((UBaseType_t)0, pxTaskStatus.uxBasePriority);
    TEST_ASSERT_EQUAL(0, pxTaskStatus.usStackHighWaterMark);
}

static void test_prvInitialiseStack( TCB_t *pxTCB, const uint32_t ulStackDepth )
{
    StackType_t * pxTopOfStack;

    pxTopOfStack = NULL;

    /* Allocate space for the stack used by the task being created.
     * The base of the stack memory stored in the TCB so the task can
     * be deleted later if required. */
    pxTCB->pxStack = ( StackType_t * ) pvPortMallocStack( ( ( ( size_t ) ulStackDepth ) * sizeof( StackType_t ) ) ); /*lint !e961 MISRA exception as the casts are only redundant for some ports. */
    ( void ) memset( pxTCB->pxStack, ( int ) tskSTACK_FILL_BYTE, ( size_t ) ulStackDepth * sizeof( StackType_t ) );

    if( pxTCB->pxStack != NULL )
    {
        /* Calculate the top of stack address.  This depends on whether the stack
         * grows from high memory to low (as per the 80x86) or vice versa.
         * portSTACK_GROWTH is used to make the result positive or negative as required
         * by the port. */
        #if ( portSTACK_GROWTH < 0 )
        {
            pxTopOfStack = &( pxTCB->pxStack[ ulStackDepth - ( uint32_t ) 1 ] );
            pxTopOfStack = ( StackType_t * ) ( ( ( portPOINTER_SIZE_TYPE ) pxTopOfStack ) & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) ); /*lint !e923 !e9033 !e9078 MISRA exception.  Avoiding casts between pointers and integers is not practical.  Size differences accounted for using portPOINTER_SIZE_TYPE type.  Checked by assert(). */

            /* Check the alignment of the calculated top of stack is correct. */
            configASSERT( ( ( ( portPOINTER_SIZE_TYPE ) pxTopOfStack & ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) == 0UL ) );

            #if ( configRECORD_STACK_HIGH_ADDRESS == 1 )
            {
                /* Also record the stack's high address, which may assist
                 * debugging. */
                pxTCB->pxEndOfStack = pxTopOfStack;
            }
            #endif /* configRECORD_STACK_HIGH_ADDRESS */
        }
        #else /* portSTACK_GROWTH */
        {
            pxTopOfStack = pxTCB->pxStack;

            /* Check the alignment of the stack buffer is correct. */
            configASSERT( ( ( ( portPOINTER_SIZE_TYPE ) pxTCB->pxStack & ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) == 0UL ) );

            /* The other extreme of the stack space is required if stack checking is
             * performed. */
            pxTCB->pxEndOfStack = pxTCB->pxStack + ( ulStackDepth - ( uint32_t ) 1 );
        }
        #endif /* portSTACK_GROWTH */
    }

    (void)pxTopOfStack;
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
 * Cover the case where xFreeStackSpace is pdTRUE, avoiding the free
 * stack space query.
 */
void test_coverage_vTaskGetInfo_get_free_stack_space( void )
{
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    TaskStatus_t pxTaskStatus;
    BaseType_t xFreeStackSpace = pdTRUE;
    eTaskState taskState = eReady;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    test_prvInitialiseStack( &xTaskTCBs[0], configMINIMAL_STACK_SIZE );
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdTRUE;

    vTaskGetInfo( &xTaskTCBs[0], &pxTaskStatus, xFreeStackSpace, taskState);
    TEST_ASSERT_EQUAL((UBaseType_t)0, pxTaskStatus.xTaskNumber);
    TEST_ASSERT_EQUAL(eRunning, pxTaskStatus.eCurrentState);
    TEST_ASSERT_EQUAL((UBaseType_t)1, pxTaskStatus.uxCurrentPriority);
    TEST_ASSERT_EQUAL((UBaseType_t)0, pxTaskStatus.uxBasePriority);
    TEST_ASSERT_EQUAL(69, pxTaskStatus.usStackHighWaterMark);
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
    TCB_t xTaskTCBs[ 1U ] = { NULL };

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdFALSE;

    vTaskResume( &xTaskTCBs[0] );
    /* in this case no state is changed and so no assertion can be made to validate the operation. */
}

/**
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
    TCB_t xTaskTCBs[ 1U ] = { NULL };
    BaseType_t xYieldRequired;

    xTaskTCBs[ 0 ].uxPriority = 1;
    xTaskTCBs[ 0 ].xTaskRunState = 0;
    xYieldPendings[ 0 ] = pdFALSE;
    pxCurrentTCBs[ 0 ] = &xTaskTCBs[ 0 ];

    uxTopReadyPriority = 1;
    uxSchedulerSuspended = pdFALSE;

    vFakePortAssertIfInterruptPriorityInvalid_Ignore();
    xYieldRequired = xTaskResumeFromISR( &xTaskTCBs[0] );

    TEST_ASSERT_EQUAL(pdFALSE, xYieldRequired);
}

/**
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
    TCB_t xTaskTCBs[ configNUMBER_OF_CORES + 1U ] = { NULL };
    uint32_t i;
    BaseType_t xYieldRequired;
    UBaseType_t uxPriority;

    for(
        uxPriority = ( UBaseType_t ) 0U;
        uxPriority < ( UBaseType_t ) configMAX_PRIORITIES;
        uxPriority++)
    {
        vListInitialise( &( pxReadyTasksLists[ uxPriority ] ) );
    }
    vListInitialise( &xSuspendedTaskList );
    vListInitialise( &xPendingReadyList );

    for( i = 0; i < configNUMBER_OF_CORES; i++ )
    {
        xTaskTCBs[ i ].uxPriority = 1;
        xTaskTCBs[ i ].xTaskRunState = i;
        vListInitialiseItem( &( xTaskTCBs[i].xStateListItem ) );
        listSET_LIST_ITEM_OWNER( &( xTaskTCBs[i].xStateListItem ), &xTaskTCBs[i] );
        xYieldPendings[ i ] = pdTRUE;
        pxCurrentTCBs[ i ] = &xTaskTCBs[ i ];
    }

    xTaskTCBs[ configNUMBER_OF_CORES ].uxPriority = 2;
    vListInitialiseItem( &( xTaskTCBs[configNUMBER_OF_CORES].xStateListItem ) );
    listSET_LIST_ITEM_OWNER( &( xTaskTCBs[configNUMBER_OF_CORES].xStateListItem ), &xTaskTCBs[i] );
    listINSERT_END( &xSuspendedTaskList, &xTaskTCBs[ i ].xStateListItem );
    uxTopReadyPriority = 1;

    uxSchedulerSuspended = pdFALSE;

    vFakePortAssertIfInterruptPriorityInvalid_Ignore();
    xYieldRequired = xTaskResumeFromISR( &xTaskTCBs[configNUMBER_OF_CORES] );

    TEST_ASSERT_EQUAL(pdTRUE, xYieldRequired);
}
