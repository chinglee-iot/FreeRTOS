/**
 * @file performance_comparison.c
 * @brief Measures the elapsed time of queue/semaphore operations that check
 * listLIST_IS_EMPTY() on an event list (xTasksWaitingToSend or
 * xTasksWaitingToReceive) before calling xTaskRemoveFromEventList(), e.g.
 * the code path in xQueueGenericSend() that checks
 * listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ). Used to
 * evaluate the performance impact of moving that emptiness check into
 * xTaskRemoveFromEventList() itself.
 *
 * The only thing that changes the branch under test is whether the event
 * list the function checks is empty or not, so every function gets exactly
 * two scenarios: "no waiting <sender|receiver>" (list empty,
 * listLIST_IS_EMPTY() == pdTRUE, the guarded block is skipped) and
 * "waiting <sender|receiver>" (list non-empty, listLIST_IS_EMPTY() ==
 * pdFALSE, xTaskRemoveFromEventList() is called). Which of two tasks has
 * the higher priority does not change which branch runs; where a helper
 * task is used at all, its priority is chosen purely to make settling the
 * queue back to the required precondition between samples automatic
 * (giving the helper a higher priority than the timed task means the
 * RTOS's own synchronous yield does the settling; see prvTimedTask).
 *
 * Each scenario is described by a ScenarioConfig_t: how to create the
 * queue/semaphore under test, an optional helper task (and/or one-off
 * extra setup/teardown) that establishes the required event-list state,
 * and the operation under test, which is timed over uxNumSamples samples
 * by a single generic timed task. Adding a scenario for another function
 * only requires a new operation function and a new ScenarioConfig_t; the
 * runner, timing loop, reporting and teardown are shared.
 *
 * Two functions that check listLIST_IS_EMPTY() on an event list are not
 * covered here:
 *   - prvUnlockQueue() only runs its checks when an ISR calls a *FromISR
 *     queue function while a task is inside the few-instruction window
 *     between prvLockQueue() and prvUnlockQueue() in a blocking call. That
 *     window cannot be entered by scheduling tasks against each other; it
 *     requires a genuine interrupt landing inside it, which is
 *     platform-specific and not reliably hittable for a sampled loop. Not
 *     covered here.
 *   - prvNotifyQueueSetContainer() only exists when configUSE_QUEUE_SETS
 *     == 1, and is covered by the
 *     Test_PerformanceComparison_prvNotifyQueueSetContainer_* scenarios,
 *     which are compiled out otherwise. See the board's FreeRTOSConfig.h:
 *     configUSE_QUEUE_SETS also changes which branch xQueueSend(),
 *     xQueueSendFromISR() and xQueueGiveFromISR() take even for a queue
 *     that is *not* a set member (an extra "is this queue in a set"
 *     check), so those scenarios should be run once with
 *     configUSE_QUEUE_SETS == 1 and once with it == 0 to isolate that
 *     overhead. See README.md.
 *
 * This is a single core evaluation; no core affinity is used.
 */

/*-----------------------------------------------------------*/
#include  <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#ifndef TEST_CONFIG_H
    #error test_config.h must be included at the end of FreeRTOSConfig.h.
#endif

#if ( configTARGET_TEST_USE_CUSTOM_SETTING == 1 )
    #include "test_setting_config.h"
#endif

#include "test_default_setting_config.h"

/*-----------------------------------------------------------*/

#ifndef testGET_TIME_FUNCTION
    #error testGET_TIME_FUNCTION must be defined to run the test
#endif

#ifndef testPRINTF_FUNCTION
    #error testPRINTF_FUNCTION must be defined to run the test
#endif

#ifndef testNUM_SAMPLES
    #define testNUM_SAMPLES ( 16384U )
#endif

/* xQueuePeek()'s "waiting receiver" scenario is the only one that cannot
 * settle synchronously (see prvPeekSettlePostSampleHook) and so pays for a
 * real tick per sample; it uses this smaller sample count to keep the run
 * time reasonable. */
#ifndef testNUM_SETTLE_SAMPLES
    #define testNUM_SETTLE_SAMPLES ( 512U )
#endif

#define testHIGHER_PRIORITY    ( configMAX_PRIORITIES - 2 )
#define testLOWER_PRIORITY     ( configMAX_PRIORITIES - 3 )
#define testLOWEST_PRIORITY    ( configMAX_PRIORITIES - 4 )

/*-----------------------------------------------------------*/

/**
 * @brief Signature of the operation under test. Invoked and timed once
 * per sample; must return pdTRUE on success.
 */
typedef BaseType_t ( * testTimedOperation_t )( void );

/**
 * @brief Signature of an optional, untimed hook run once before the first
 * sample and after every sample, to establish and then restore the
 * precondition the operation under test requires.
 */
typedef void ( * testPostSampleHook_t )( void );

/**
 * @brief Signature used to create the queue or semaphore under test.
 */
typedef QueueHandle_t ( * testQueueCreateFunction_t )( void );

/**
 * @brief Signature of an optional one-off setup/teardown step, e.g.
 * creating a queue set and adding the test queue to it, or pre-loading an
 * item that is never consumed.
 */
typedef void ( * testExtraSetupFunction_t )( void );
typedef void ( * testExtraTeardownFunction_t )( void );

/**
 * @brief Describes one performance scenario. Passed by prvRunScenario()
 * so that neither the runner nor the generic timed task need know about
 * any concrete queue API - new scenarios plug in without modifying
 * either. pxHelperTaskFunction may be NULL for a "no waiting" scenario,
 * in which case no helper task is created at all.
 */
typedef struct ScenarioConfig
{
    const char * pcScenarioName;
    const char * pcOperationName;
    testQueueCreateFunction_t pxQueueCreateFunction;
    testExtraSetupFunction_t pxExtraSetupFunction;
    testExtraTeardownFunction_t pxExtraTeardownFunction;
    TaskFunction_t pxHelperTaskFunction;
    const char * pcHelperTaskName;
    UBaseType_t uxHelperTaskPriority;
    testTimedOperation_t pxTimedOperation;
    testPostSampleHook_t pxPostSampleHook;
    UBaseType_t uxTimedTaskPriority;
    UBaseType_t uxNumSamples;
} ScenarioConfig_t;

/*-----------------------------------------------------------*/

/**
 * @brief Queue or semaphore shared between the helper task(s) (if any)
 * and the timed task.
 */
static QueueHandle_t xTestQueueHandle;

#if ( configUSE_QUEUE_SETS == 1 )

/**
 * @brief Queue set used only by the prvNotifyQueueSetContainer() scenarios.
 */
    static QueueSetHandle_t xTestQueueSetHandle;
#endif

/**
 * @brief Second helper task, used only by the xQueuePeek() "waiting
 * receiver" scenario, which needs two tasks simultaneously blocked on
 * xTasksWaitingToReceive.
 */
static TaskHandle_t xSecondHelperTaskHandle;

/**
 * @brief Task that keeps the queue's event list in the state required by
 * the scenario, e.g. blocked waiting to receive, or continuously trying
 * to send. NULL for scenarios with no helper task.
 */
static TaskHandle_t xHelperTaskHandle;

/**
 * @brief Task that repeatedly calls, and times, the operation under test.
 */
static TaskHandle_t xTimedTaskHandle;

/**
 * @brief Handle of the task running the test, notified by the timed task
 * once it has completed all samples.
 */
static TaskHandle_t xMainTaskHandle;

/**
 * @brief Cumulative elapsed time of all samples, filled in by the timed task.
 */
static UBaseType_t uxElapsedCumulative;
/*-----------------------------------------------------------*/

static QueueHandle_t prvCreateIntQueue( void )
{
    return xQueueCreate( 1, sizeof( int ) );
}
/*-----------------------------------------------------------*/

static QueueHandle_t prvCreateBinarySemaphore( void )
{
    /* Created "empty": a task taking it will block immediately. */
    return xSemaphoreCreateBinary();
}
/*-----------------------------------------------------------*/

static QueueHandle_t prvCreateFullCountingSemaphore( void )
{
    /* Max count 1, created already at that count: a task giving it will
     * block immediately. */
    return xSemaphoreCreateCounting( 1, 1 );
}
/*-----------------------------------------------------------*/

static void prvReceiverTask( void * pvParameters )
{
    int iReceived;

    ( void ) pvParameters;

    for( ; ; )
    {
        ( void ) xQueueReceive( xTestQueueHandle, &iReceived, portMAX_DELAY );
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Keeps trying to send to the (length 1) queue so that, once it is
 * full, this task sits blocked on xTasksWaitingToSend.
 */
static void prvContinuousSenderTask( void * pvParameters )
{
    int iSend = 0;

    ( void ) pvParameters;

    for( ; ; )
    {
        ( void ) xQueueSend( xTestQueueHandle, &iSend, portMAX_DELAY );
    }
}
/*-----------------------------------------------------------*/

static void prvSemaphoreReceiverTask( void * pvParameters )
{
    ( void ) pvParameters;

    for( ; ; )
    {
        ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) xTestQueueHandle, portMAX_DELAY );
    }
}
/*-----------------------------------------------------------*/

/**
 * @brief Keeps trying to give the (max count 1) semaphore directly through
 * xQueueGenericSend(), so that once it is at its max count this task sits
 * blocked on xTasksWaitingToSend. The public xSemaphoreGive() cannot be
 * used for this: it always uses a 0 block time and so never blocks.
 */
static void prvBlockingGiverTask( void * pvParameters )
{
    ( void ) pvParameters;

    for( ; ; )
    {
        ( void ) xQueueGenericSend( xTestQueueHandle, NULL, portMAX_DELAY, queueSEND_TO_BACK );
    }
}
/*-----------------------------------------------------------*/

#if ( configUSE_QUEUE_SETS == 1 )
    static void prvQueueSetReceiverTask( void * pvParameters )
    {
        ( void ) pvParameters;

        for( ; ; )
        {
            ( void ) xQueueSelectFromSet( xTestQueueSetHandle, portMAX_DELAY );
        }
    }
#endif /* configUSE_QUEUE_SETS */
/*-----------------------------------------------------------*/

static BaseType_t prvSendOperation( void )
{
    int iSend = 0;

    return xQueueSend( xTestQueueHandle, &iSend, 0 );
}
/*-----------------------------------------------------------*/

static BaseType_t prvResetOperation( void )
{
    return xQueueReset( xTestQueueHandle );
}
/*-----------------------------------------------------------*/

static BaseType_t prvReceiveOperation( void )
{
    int iReceived;

    return xQueueReceive( xTestQueueHandle, &iReceived, 0 );
}
/*-----------------------------------------------------------*/

static BaseType_t prvPeekOperation( void )
{
    int iPeeked;

    return xQueuePeek( xTestQueueHandle, &iPeeked, 0 );
}
/*-----------------------------------------------------------*/

static BaseType_t prvSemaphoreTakeOperation( void )
{
    return xSemaphoreTake( ( SemaphoreHandle_t ) xTestQueueHandle, 0 );
}
/*-----------------------------------------------------------*/

/**
 * @brief xQueueSendFromISR(), xQueueReceiveFromISR() and xQueueGiveFromISR()
 * assume ISR-context semantics: nothing at or below the caller's own
 * priority can preempt it. Calling them from a task does not satisfy that
 * on its own, so interrupts are masked for the duration of the call to
 * emulate it and keep real interrupts (e.g. the tick) out of the timed
 * window.
 */
static BaseType_t prvSendFromISROperation( void )
{
    int iSend = 0;
    BaseType_t xReturn;

    taskDISABLE_INTERRUPTS();
    xReturn = xQueueSendFromISR( xTestQueueHandle, &iSend, NULL );
    taskENABLE_INTERRUPTS();

    return xReturn;
}
/*-----------------------------------------------------------*/

static BaseType_t prvReceiveFromISROperation( void )
{
    int iReceived;
    BaseType_t xReturn;

    taskDISABLE_INTERRUPTS();
    xReturn = xQueueReceiveFromISR( xTestQueueHandle, &iReceived, NULL );
    taskENABLE_INTERRUPTS();

    return xReturn;
}
/*-----------------------------------------------------------*/

static BaseType_t prvGiveFromISROperation( void )
{
    BaseType_t xReturn;

    taskDISABLE_INTERRUPTS();
    xReturn = xQueueGiveFromISR( xTestQueueHandle, NULL );
    taskENABLE_INTERRUPTS();

    return xReturn;
}
/*-----------------------------------------------------------*/

/**
 * @brief Drains the one item xQueueSend()/xQueueSendFromISR() just
 * enqueued, so the queue has room again next sample. Used by "no waiting
 * receiver" scenarios, which never create a receiver task, so nothing
 * else would ever remove that item.
 */
static void prvDrainQueuePostSampleHook( void )
{
    int iDiscard;

    ( void ) xQueueReceive( xTestQueueHandle, &iDiscard, 0 );
}
/*-----------------------------------------------------------*/

/**
 * @brief Refills the item xQueueReceive()/xQueueReceiveFromISR() just
 * removed, so the queue has data again next sample. Used by "no waiting
 * sender" scenarios, which never create a sender task, so nothing else
 * would ever replace that item.
 */
static void prvRefillQueuePostSampleHook( void )
{
    int iSend = 0;

    ( void ) xQueueSend( xTestQueueHandle, &iSend, 0 );
}
/*-----------------------------------------------------------*/

/**
 * @brief Gives back the count xQueueSemaphoreTake() just took, so the
 * semaphore is available again next sample. Used by the "no waiting
 * sender" scenario, which never creates a giver task.
 */
static void prvGiveSemaphorePostSampleHook( void )
{
    ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) xTestQueueHandle );
}
/*-----------------------------------------------------------*/

/**
 * @brief Takes back the count xQueueGiveFromISR() just gave, so the
 * semaphore has room again next sample. Used by the "no waiting receiver"
 * scenario, which never creates a receiver task.
 */
static void prvTakeSemaphorePostSampleHook( void )
{
    ( void ) xSemaphoreTake( ( SemaphoreHandle_t ) xTestQueueHandle, 0 );
}
/*-----------------------------------------------------------*/

/**
 * @brief Used by the *FromISR "waiting" scenarios: unlike xQueueSend()/
 * xQueueReceive()/xQueueSemaphoreTake(), the *FromISR functions never
 * yield to the woken helper on their own. taskYIELD() hands the CPU to
 * the helper (created at a higher priority than the timed task), which
 * runs to completion and re-blocks, restoring the precondition for the
 * next sample.
 */
static void prvYieldPostSampleHook( void )
{
    taskYIELD();
}
/*-----------------------------------------------------------*/

/**
 * @brief Used by the xQueuePeek() "waiting receiver" scenario. Unlike the
 * other "waiting" scenarios, xQueuePeek() does not remove the item it
 * reads, so a single waiting receiver would always be removed from
 * xTasksWaitingToReceive by the very first send that supplies data (queue
 * emptiness, not priority, gates that removal) - it could never still be
 * on the list by the time xQueuePeek() runs. Two receiver tasks are used
 * instead: after vTaskDelay() lets both settle on the empty queue, a
 * single item wakes exactly one of them (consuming it as it re-blocks on
 * the next loop iteration) while the other remains on the list, giving
 * xQueuePeek() a non-empty xTasksWaitingToReceive with data present.
 */
static void prvPeekSettlePostSampleHook( void )
{
    int iSend = 0;

    vTaskDelay( 1 );
    ( void ) xQueueSend( xTestQueueHandle, &iSend, 0 );
}
/*-----------------------------------------------------------*/

/**
 * @brief One-off setup for the xQueuePeek() "no waiting receiver"
 * scenario: pre-loads a single item that is never removed (xQueuePeek()
 * never consumes it), so xTasksWaitingToReceive stays empty for the whole
 * run with no helper task and no post-sample hook required.
 */
static void prvPeekPreloadExtraSetup( void )
{
    int iSend = 0;

    ( void ) xQueueSend( xTestQueueHandle, &iSend, 0 );
}
/*-----------------------------------------------------------*/

static void prvPeekExtraSetup( void )
{
    BaseType_t xRet;

    xRet = xTaskCreate( prvReceiverTask,
                         "recv2",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         testLOWEST_PRIORITY,
                         &xSecondHelperTaskHandle );
    TEST_ASSERT_EQUAL_MESSAGE( pdTRUE, xRet, "Creating second helper task failed" );
}
/*-----------------------------------------------------------*/

static void prvPeekExtraTeardown( void )
{
    vTaskDelete( xSecondHelperTaskHandle );
    xSecondHelperTaskHandle = NULL;
}
/*-----------------------------------------------------------*/

#if ( configUSE_QUEUE_SETS == 1 )

/**
 * @brief Resets both the member queue and the set's own internal queue.
 * Needed unconditionally for both queue-set scenarios: the set's own
 * event list is only one part of the state - the member queue has its
 * own separate data buffer that nothing in xQueueSelectFromSet()'s wake
 * path drains, and prvNotifyQueueSetContainer() itself is a no-op once
 * the set's internal queue is full. Without resetting both, only the
 * first sample would ever reach the branch under test.
 */
    static void prvQueueSetPostSampleHook( void )
    {
        xQueueReset( xTestQueueHandle );
        xQueueReset( ( QueueHandle_t ) xTestQueueSetHandle );
    }
/*-----------------------------------------------------------*/

    static void prvQueueSetExtraSetup( void )
    {
        xTestQueueSetHandle = xQueueCreateSet( 1 );
        TEST_ASSERT_NOT_NULL_MESSAGE( xTestQueueSetHandle, "Queue set creation failed" );

        TEST_ASSERT_EQUAL_MESSAGE( pdPASS,
                                    xQueueAddToSet( xTestQueueHandle, xTestQueueSetHandle ),
                                    "Adding queue to set failed" );
    }
/*-----------------------------------------------------------*/

    static void prvQueueSetExtraTeardown( void )
    {
        ( void ) xQueueRemoveFromSet( xTestQueueHandle, xTestQueueSetHandle );
        vQueueDelete( ( QueueHandle_t ) xTestQueueSetHandle );
        xTestQueueSetHandle = NULL;
    }
#endif /* configUSE_QUEUE_SETS */
/*-----------------------------------------------------------*/

/**
 * @brief Repeatedly times pxConfig->pxTimedOperation() over
 * pxConfig->uxNumSamples samples, running pxConfig->pxPostSampleHook() (if
 * any) once before the first sample and after each one, to establish and
 * then restore the precondition the operation under test requires. Knows
 * nothing about which queue API it is timing.
 */
static void prvTimedTask( void * pvParameters )
{
    const ScenarioConfig_t * pxConfig = ( const ScenarioConfig_t * ) pvParameters;
    UBaseType_t uxSample;
    UBaseType_t uxTemp;

    uxElapsedCumulative = 0;

    if( pxConfig->pxPostSampleHook != NULL )
    {
        pxConfig->pxPostSampleHook();
    }

    for( uxSample = 0; uxSample < pxConfig->uxNumSamples; uxSample++ )
    {
        uxTemp = testGET_TIME_FUNCTION();
        TEST_ASSERT_EQUAL_MESSAGE( pdTRUE, pxConfig->pxTimedOperation(), "Timed operation failed" );
        uxElapsedCumulative += ( testGET_TIME_FUNCTION() - uxTemp );

        if( pxConfig->pxPostSampleHook != NULL )
        {
            pxConfig->pxPostSampleHook();
        }
    }

    /* Notify the main task that every sample has been taken. */
    xTaskNotifyGive( xMainTaskHandle );

    /* Wait to be deleted by the main task. */
    vTaskSuspend( NULL );
}
/*-----------------------------------------------------------*/

static void prvReportResults( const ScenarioConfig_t * pxConfig )
{
    testPRINTF_FUNCTION( "[%s] %s, accumulated elapsed time: %u\r\n",
                          pxConfig->pcScenarioName, pxConfig->pcOperationName,
                          ( unsigned int ) uxElapsedCumulative );
    testPRINTF_FUNCTION( "[%s] %s, average elapsed time: %u\r\n",
                          pxConfig->pcScenarioName, pxConfig->pcOperationName,
                          ( unsigned int ) ( uxElapsedCumulative / pxConfig->uxNumSamples ) );
}
/*-----------------------------------------------------------*/

static void prvTeardownScenario( const ScenarioConfig_t * pxConfig )
{
    vTaskDelete( xTimedTaskHandle );
    xTimedTaskHandle = NULL;

    if( xHelperTaskHandle != NULL )
    {
        vTaskDelete( xHelperTaskHandle );
        xHelperTaskHandle = NULL;
    }

    if( pxConfig->pxExtraTeardownFunction != NULL )
    {
        pxConfig->pxExtraTeardownFunction();
    }

    vQueueDelete( xTestQueueHandle );
    xTestQueueHandle = NULL;
}
/*-----------------------------------------------------------*/

/**
 * @brief Create the queue, run any extra setup, create the (optional)
 * helper and the timed task, wait for the timed task to finish taking all
 * its samples, print the results, then tear everything down again.
 */
static void prvRunScenario( const ScenarioConfig_t * pxConfig )
{
    BaseType_t xRet;

    xMainTaskHandle = xTaskGetCurrentTaskHandle();

    xTestQueueHandle = pxConfig->pxQueueCreateFunction();
    TEST_ASSERT_NOT_NULL_MESSAGE( xTestQueueHandle, "Queue creation failed" );

    if( pxConfig->pxExtraSetupFunction != NULL )
    {
        pxConfig->pxExtraSetupFunction();
    }

    if( pxConfig->pxHelperTaskFunction != NULL )
    {
        xRet = xTaskCreate( pxConfig->pxHelperTaskFunction,
                             pxConfig->pcHelperTaskName,
                             configMINIMAL_STACK_SIZE,
                             NULL,
                             pxConfig->uxHelperTaskPriority,
                             &xHelperTaskHandle );
        TEST_ASSERT_EQUAL_MESSAGE( pdTRUE, xRet, "Creating helper task failed" );
    }

    xRet = xTaskCreate( prvTimedTask,
                         "timed",
                         configMINIMAL_STACK_SIZE,
                         ( void * ) pxConfig,
                         pxConfig->uxTimedTaskPriority,
                         &xTimedTaskHandle );
    TEST_ASSERT_EQUAL_MESSAGE( pdTRUE, xRet, "Creating timed task failed" );

    /* Wait for the timed task to finish taking all its samples. */
    ( void ) ulTaskNotifyTake( pdTRUE, portMAX_DELAY );

    prvReportResults( pxConfig );
    prvTeardownScenario( pxConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueSend_NoWaitingReceiver( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "no waiting receiver",
        .pcOperationName = "xQueueSend() with empty xTasksWaitingToReceive",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxTimedOperation = prvSendOperation,
        .pxPostSampleHook = prvDrainQueuePostSampleHook,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueSend_WaitingReceiver( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "waiting receiver",
        .pcOperationName = "xQueueSend() with waiting receiver",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxHelperTaskFunction = prvReceiverTask,
        .pcHelperTaskName = "recv",
        .uxHelperTaskPriority = testHIGHER_PRIORITY,
        .pxTimedOperation = prvSendOperation,
        .uxTimedTaskPriority = testLOWER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueSendFromISR_NoWaitingReceiver( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "no waiting receiver",
        .pcOperationName = "xQueueSendFromISR() with empty xTasksWaitingToReceive",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxTimedOperation = prvSendFromISROperation,
        .pxPostSampleHook = prvDrainQueuePostSampleHook,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueSendFromISR_WaitingReceiver( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "waiting receiver",
        .pcOperationName = "xQueueSendFromISR() with waiting receiver",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxHelperTaskFunction = prvReceiverTask,
        .pcHelperTaskName = "recv",
        .uxHelperTaskPriority = testHIGHER_PRIORITY,
        .pxTimedOperation = prvSendFromISROperation,
        .pxPostSampleHook = prvYieldPostSampleHook,
        .uxTimedTaskPriority = testLOWER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueReceive_NoWaitingSender( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "no waiting sender",
        .pcOperationName = "xQueueReceive() with empty xTasksWaitingToSend",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxTimedOperation = prvReceiveOperation,
        .pxPostSampleHook = prvRefillQueuePostSampleHook,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueReceive_WaitingSender( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "waiting sender",
        .pcOperationName = "xQueueReceive() with waiting sender",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxHelperTaskFunction = prvContinuousSenderTask,
        .pcHelperTaskName = "send",
        .uxHelperTaskPriority = testHIGHER_PRIORITY,
        .pxTimedOperation = prvReceiveOperation,
        .uxTimedTaskPriority = testLOWER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueReceiveFromISR_NoWaitingSender( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "no waiting sender",
        .pcOperationName = "xQueueReceiveFromISR() with empty xTasksWaitingToSend",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxTimedOperation = prvReceiveFromISROperation,
        .pxPostSampleHook = prvRefillQueuePostSampleHook,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueReceiveFromISR_WaitingSender( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "waiting sender",
        .pcOperationName = "xQueueReceiveFromISR() with waiting sender",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxHelperTaskFunction = prvContinuousSenderTask,
        .pcHelperTaskName = "send",
        .uxHelperTaskPriority = testHIGHER_PRIORITY,
        .pxTimedOperation = prvReceiveFromISROperation,
        .pxPostSampleHook = prvYieldPostSampleHook,
        .uxTimedTaskPriority = testLOWER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueSemaphoreTake_NoWaitingSender( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "no waiting sender",
        .pcOperationName = "xQueueSemaphoreTake() with empty xTasksWaitingToSend",
        .pxQueueCreateFunction = prvCreateFullCountingSemaphore,
        .pxTimedOperation = prvSemaphoreTakeOperation,
        .pxPostSampleHook = prvGiveSemaphorePostSampleHook,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueSemaphoreTake_WaitingSender( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "waiting sender",
        .pcOperationName = "xQueueSemaphoreTake() with waiting sender",
        .pxQueueCreateFunction = prvCreateFullCountingSemaphore,
        .pxHelperTaskFunction = prvBlockingGiverTask,
        .pcHelperTaskName = "give",
        .uxHelperTaskPriority = testHIGHER_PRIORITY,
        .pxTimedOperation = prvSemaphoreTakeOperation,
        .uxTimedTaskPriority = testLOWER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueGiveFromISR_NoWaitingReceiver( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "no waiting receiver",
        .pcOperationName = "xQueueGiveFromISR() with empty xTasksWaitingToReceive",
        .pxQueueCreateFunction = prvCreateBinarySemaphore,
        .pxTimedOperation = prvGiveFromISROperation,
        .pxPostSampleHook = prvTakeSemaphorePostSampleHook,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueGiveFromISR_WaitingReceiver( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "waiting receiver",
        .pcOperationName = "xQueueGiveFromISR() with waiting receiver",
        .pxQueueCreateFunction = prvCreateBinarySemaphore,
        .pxHelperTaskFunction = prvSemaphoreReceiverTask,
        .pcHelperTaskName = "recv",
        .uxHelperTaskPriority = testHIGHER_PRIORITY,
        .pxTimedOperation = prvGiveFromISROperation,
        .pxPostSampleHook = prvYieldPostSampleHook,
        .uxTimedTaskPriority = testLOWER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueuePeek_NoWaitingReceiver( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "no waiting receiver",
        .pcOperationName = "xQueuePeek() with data present and empty xTasksWaitingToReceive",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxExtraSetupFunction = prvPeekPreloadExtraSetup,
        .pxTimedOperation = prvPeekOperation,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueuePeek_WaitingReceiver( void )
{
    /* Unlike the other "waiting" scenarios, the helper here is NOT given a
     * higher priority than the timed task - see prvPeekSettlePostSampleHook
     * for why xQueuePeek() needs two lower-priority receivers and explicit
     * settling instead. */
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "waiting receiver",
        .pcOperationName = "xQueuePeek() with data present and a receiver still waiting",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxExtraSetupFunction = prvPeekExtraSetup,
        .pxExtraTeardownFunction = prvPeekExtraTeardown,
        .pxHelperTaskFunction = prvReceiverTask,
        .pcHelperTaskName = "recv1",
        .uxHelperTaskPriority = testLOWER_PRIORITY,
        .pxTimedOperation = prvPeekOperation,
        .pxPostSampleHook = prvPeekSettlePostSampleHook,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SETTLE_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueReset_NoWaitingSender( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "no waiting sender",
        .pcOperationName = "xQueueReset() with empty xTasksWaitingToSend",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxHelperTaskFunction = prvContinuousSenderTask,
        .pcHelperTaskName = "send",
        .uxHelperTaskPriority = testLOWER_PRIORITY,
        .pxTimedOperation = prvResetOperation,
        .uxTimedTaskPriority = testHIGHER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

static void Test_PerformanceComparison_xQueueReset_WaitingSender( void )
{
    static const ScenarioConfig_t xConfig =
    {
        .pcScenarioName = "waiting sender",
        .pcOperationName = "xQueueReset() with waiting sender",
        .pxQueueCreateFunction = prvCreateIntQueue,
        .pxHelperTaskFunction = prvContinuousSenderTask,
        .pcHelperTaskName = "send",
        .uxHelperTaskPriority = testHIGHER_PRIORITY,
        .pxTimedOperation = prvResetOperation,
        .uxTimedTaskPriority = testLOWER_PRIORITY,
        .uxNumSamples = testNUM_SAMPLES
    };

    prvRunScenario( &xConfig );
}
/*-----------------------------------------------------------*/

#if ( configUSE_QUEUE_SETS == 1 )
    static void Test_PerformanceComparison_prvNotifyQueueSetContainer_NoWaitingReceiver( void )
    {
        static const ScenarioConfig_t xConfig =
        {
            .pcScenarioName = "no waiting receiver",
            .pcOperationName = "xQueueSend() to a queue-set member with empty container xTasksWaitingToReceive",
            .pxQueueCreateFunction = prvCreateIntQueue,
            .pxExtraSetupFunction = prvQueueSetExtraSetup,
            .pxExtraTeardownFunction = prvQueueSetExtraTeardown,
            .pxTimedOperation = prvSendOperation,
            .pxPostSampleHook = prvQueueSetPostSampleHook,
            .uxTimedTaskPriority = testHIGHER_PRIORITY,
            .uxNumSamples = testNUM_SAMPLES
        };

        prvRunScenario( &xConfig );
    }
/*-----------------------------------------------------------*/

    static void Test_PerformanceComparison_prvNotifyQueueSetContainer_WaitingReceiver( void )
    {
        static const ScenarioConfig_t xConfig =
        {
            .pcScenarioName = "waiting receiver",
            .pcOperationName = "xQueueSend() to a queue-set member with a waiting container receiver",
            .pxQueueCreateFunction = prvCreateIntQueue,
            .pxExtraSetupFunction = prvQueueSetExtraSetup,
            .pxExtraTeardownFunction = prvQueueSetExtraTeardown,
            .pxHelperTaskFunction = prvQueueSetReceiverTask,
            .pcHelperTaskName = "setrecv",
            .uxHelperTaskPriority = testHIGHER_PRIORITY,
            .pxTimedOperation = prvSendOperation,
            .pxPostSampleHook = prvQueueSetPostSampleHook,
            .uxTimedTaskPriority = testLOWER_PRIORITY,
            .uxNumSamples = testNUM_SAMPLES
        };

        prvRunScenario( &xConfig );
    }
#endif /* configUSE_QUEUE_SETS */
/*-----------------------------------------------------------*/

/* Runs before every test, put init calls here. */
testSETUP_FUNCTION_PROTOTYPE( setUp )
{
}
/*-----------------------------------------------------------*/

/* Runs after every test, put clean-up calls here. */
testTEARDOWN_FUNCTION_PROTOTYPE( tearDown )
{
}
/*-----------------------------------------------------------*/

testENTRY_FUNCTION_PROTOTYPE( vRunPerformanceComparison )
{
    testBEGIN_FUNCTION();

    /* xQueueSend. */
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueSend_NoWaitingReceiver );
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueSend_WaitingReceiver );

    /* xQueueSendFromISR. */
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueSendFromISR_NoWaitingReceiver );
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueSendFromISR_WaitingReceiver );

    /* xQueueReceive. */
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueReceive_NoWaitingSender );
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueReceive_WaitingSender );

    /* xQueueReceiveFromISR. */
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueReceiveFromISR_NoWaitingSender );
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueReceiveFromISR_WaitingSender );

    /* xQueueSemaphoreTake. */
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueSemaphoreTake_NoWaitingSender );
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueSemaphoreTake_WaitingSender );

    /* xQueueGiveFromISR. */
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueGiveFromISR_NoWaitingReceiver );
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueGiveFromISR_WaitingReceiver );

    /* xQueuePeek. */
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueuePeek_NoWaitingReceiver );
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueuePeek_WaitingReceiver );

    /* xQueueReset. */
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueReset_NoWaitingSender );
    testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_xQueueReset_WaitingSender );

    /* prvUnlockQueue: not covered, see file header comment. */

    /* prvNotifyQueueSetContainer. */
    #if ( configUSE_QUEUE_SETS == 1 )
        testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_prvNotifyQueueSetContainer_NoWaitingReceiver );
        testRUN_TEST_CASE_FUNCTION( Test_PerformanceComparison_prvNotifyQueueSetContainer_WaitingReceiver );
    #endif

    testEND_FUNCTION();
}
