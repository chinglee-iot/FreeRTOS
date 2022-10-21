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
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    
    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    
    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
}

/* SMP-TC-2 */
void test_smp2( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    
    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    
    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
}

/* SMP-TC-3 */
void test_smp3( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;
    TaskHandle_t xHandleTask4 = NULL;
    
    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );
    xTaskCreate( vSmpTestTask, "SMP Task 4", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask4 );  
    
    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
    verifySmpTask( &xHandleTask4, eReady, -1 );

    vTaskSuspend(xHandleTask1);

    verifySmpTask( &xHandleTask1, eSuspended, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eRunning, 0 );
    verifySmpTask( &xHandleTask4, eReady, -1 );
}

/* SMP-TC-4 */
void test_smp4( void )
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

/* SMP-TC-5 */
void test_smp5( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    
    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );

    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask3 );

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifySmpTask( &xHandleTask3, eRunning, 1 );
}

/* SMP-TC-6 */
void test_smp6( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    
    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    
    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );

    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    vTaskDelete(xHandleTask2);

    TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp );
    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eDeleted, -1 );
    verifyIdleTask(0, 1);
}

/* SMP-TC-7 */
void test_smp7( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 3, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );

    TEST_ASSERT_EQUAL( 0, uxDeletedTasksWaitingCleanUp );

    vTaskDelete(xHandleTask1);

    TEST_ASSERT_EQUAL( 1, uxDeletedTasksWaitingCleanUp );
    verifySmpTask( &xHandleTask1, eDeleted, -1 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eRunning, 0 );
}

/* SMP-TC-8 */
void test_smp8( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;
    TaskStatus_t xTaskDetails;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 4, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 3, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask3 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );

    vTaskPrioritySet( xHandleTask1, 1 );
    vTaskSwitchContext(0); //WHYYYYYY

    verifySmpTask( &xHandleTask1, eReady, -1 );
    vTaskGetInfo( xHandleTask1, &xTaskDetails, pdTRUE, eInvalid );
    TEST_ASSERT_EQUAL( 1, xTaskDetails.xHandle->uxPriority );

    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eRunning, 0 );
}

/* SMP-TC-9 */
void test_smp9( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;
    TaskStatus_t xTaskDetails;

    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 3, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );

    vTaskPrioritySet( xHandleTask3, 4 );
    //vTaskSwitchContext(0); //WHYYYYYY not here but needed above????

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eReady, -1 );
    verifySmpTask( &xHandleTask3, eRunning, 1 );
    vTaskGetInfo( xHandleTask3, &xTaskDetails, pdTRUE, eInvalid );
    TEST_ASSERT_EQUAL( 4, xTaskDetails.xHandle->uxPriority );
}

/* SMP-TC-10 */
void test_smp10( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    UBaseType_t uxCoreAffinityMask;
    
    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    
    uxCoreAffinityMask = ( 1 << 1 );
    vTaskCoreAffinitySet( xHandleTask1, uxCoreAffinityMask );
    TEST_ASSERT_EQUAL( uxCoreAffinityMask, vTaskCoreAffinityGet( xHandleTask1 ) );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 1 );
    verifySmpTask( &xHandleTask2, eRunning, 0 );
}

/* SMP-TC-11 */
void test_smp11( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    UBaseType_t uxCoreAffinityMask;
    
    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask2 );
    
    uxCoreAffinityMask = ( 1 << 1 );
    vTaskCoreAffinitySet( xHandleTask1, uxCoreAffinityMask );
    TEST_ASSERT_EQUAL( uxCoreAffinityMask, vTaskCoreAffinityGet( xHandleTask1 ) );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 1 );
    verifySmpTask( &xHandleTask2, eRunning, 0 );
}


/* SMP-TC-12 */
void test_smp12( void )
{
    TaskHandle_t xHandleTask1 = NULL;
    TaskHandle_t xHandleTask2 = NULL;
    TaskHandle_t xHandleTask3 = NULL;
    UBaseType_t uxCoreAffinityMask;
    
    xTaskCreate( vSmpTestTask, "SMP Task 1", configMINIMAL_STACK_SIZE, NULL, 3, &xHandleTask1 );
    xTaskCreate( vSmpTestTask, "SMP Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleTask2 );
    xTaskCreate( vSmpTestTask, "SMP Task 3", configMINIMAL_STACK_SIZE, NULL, 1, &xHandleTask3 );
  
    uxCoreAffinityMask = ( 1 << 0 );
    vTaskCoreAffinitySet( xHandleTask3, uxCoreAffinityMask );
    TEST_ASSERT_EQUAL( uxCoreAffinityMask, vTaskCoreAffinityGet( xHandleTask3 ) );

    vTaskStartScheduler();

    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eRunning, 1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );

    vTaskSuspend(xHandleTask2);

    //this dosnt seem right, cores should evict and shuffle?
    verifySmpTask( &xHandleTask1, eRunning, 0 );
    verifySmpTask( &xHandleTask2, eSuspended, -1 );
    verifySmpTask( &xHandleTask3, eReady, -1 );
}