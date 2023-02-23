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

/* Mock includes. */
#include "mock_timers.h"
#include "mock_fake_assert.h"
#include "mock_fake_port.h"

/* ===========================  EXTERN VARIABLES  =========================== */

extern volatile UBaseType_t uxDeletedTasksWaitingCleanUp;

/* ============================  Unity Fixtures  ============================ */
/*! called before each testcase */
void setUp(void)
{
    commonSetUp();
}

/*! called after each testcase */
void tearDown(void)
{
    commonTearDown();
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

/*  Coverage for line 2787 where scheduler running 
    needs to be pdFalse inside vTaskSuspend function  */

void test_scheduler_running_false(void)
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = {NULL};
    uint32_t i;

    for (i = 0; i < configNUMBER_OF_CORES; i++)
    {
        xTaskCreate(vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i]);
    }
    xTaskHandles[0]->xTaskRunState = 1;

    vTaskSuspend(xTaskHandles[0]);
}

/*  Coverage for 2785 line where taskTASK_IS_RUNNING( pxTCB ) macro 
    is being tested inside vTaskSuspend function */

void test_task_suspend_enable_branch_Rand_task(void)
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = {NULL};
    uint32_t i;

    /* Create tasks of equal priority */
    for (i = 0; i < configNUMBER_OF_CORES; i++)
    {
        xTaskCreate(vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i]);
    }

    vTaskStartScheduler();

    /* task T0 */
    xTaskHandles[0]->xTaskRunState = configNUMBER_OF_CORES + 1;
    vTaskSuspend(xTaskHandles[0]);
}

/*  Coverage for 2785 line where taskTASK_IS_RUNNING( pxTCB ) macro 
    is being tested inside vTaskSuspend function  */

void test_task_suspend_enable_branch_negative_task(void)
{
    TaskHandle_t xTaskHandles[configNUMBER_OF_CORES] = {NULL};
    uint32_t i;

    /* Create tasks of equal priority */
    for (i = 0; i < configNUMBER_OF_CORES; i++)
    {
        xTaskCreate(vSmpTestTask, "SMP Task", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskHandles[i]);
    }

    vTaskStartScheduler();

    /* task T0 */
    xTaskHandles[0]->xTaskRunState = -1;
    vTaskSuspend(xTaskHandles[0]);
}


