/*
 * FreeRTOS V202212.00
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

/**
 * @file main.c
 * @brief The implementation of main function to start test runner task.
 *
 * Procedure:
 *   - Initialize environment
 *   - Run the test case
 */

/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "task.h"     /* RTOS task related API prototypes. */

#include "unity.h"    /* unit testing support functions */

#include <xcore/chanend.h>

#include "testing_main.h"

/*-----------------------------------------------------------*/

static void prvInitializeHardware( void );
extern void vRunTest( void );
/*-----------------------------------------------------------*/

/* Idle hook counter */
static unsigned long ulCnt = 0;
/*-----------------------------------------------------------*/

static void prvInitializeHardware( void )
{
    
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    char * pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) xTask;

    rtos_printf( "ERROR: Stack Overflow\n\0" );

    /* Run time stack overflow checking is performed if
     * configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     * function is called if a stack overflow is detected.  pxCurrentTCB can be
     * inspected in the debugger if the task name passed into this function is
     * corrupt. */
    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    rtos_printf( "ERROR: Malloc Failed\n\0" );

    for( ; ; )
    {
        /* Always running, put asm here to avoid optimization by compiler. */
        __asm volatile ( "nop" );
    }
}
/*-----------------------------------------------------------*/

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * puxIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &( xIdleTaskTCB );
    *ppxIdleTaskStackBuffer = &( uxIdleTaskStack[ 0 ] );
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*-----------------------------------------------------------*/

void vApplicationGetPassiveIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                           StackType_t ** ppxIdleTaskStackBuffer,
                                           configSTACK_DEPTH_TYPE * puxIdleTaskStackSize,
                                           BaseType_t xPassiveIdleTaskIndex )
{
    static StaticTask_t xIdleTaskTCBs[ configNUMBER_OF_CORES - 1 ];
    static StackType_t uxIdleTaskStacks[ configNUMBER_OF_CORES - 1 ][ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &( xIdleTaskTCBs[ xPassiveIdleTaskIndex ] );
    *ppxIdleTaskStackBuffer = &( uxIdleTaskStacks[ xPassiveIdleTaskIndex ][ 0 ] );
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*-----------------------------------------------------------*/

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    		StackType_t **ppxTimerTaskStackBuffer,
                                    		uint32_t *pulTimerTaskStackSize )
{
	static StaticTask_t xTimerTaskTCB;
	static StackType_t uxTimerTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    *pulTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    volatile BaseType_t xValue;
    uint32_t ulState;

	xValue = intCONST1;
	xValue += intCONST2;
	xValue *= intCONST3;
	xValue /= intCONST4;

	if( xValue != intEXPECTED_ANSWER )
	{
		rtos_printf("Error Occured at Idle Count: %u\n", ulCnt);
	}

	#if( configUSE_PREEMPTION == 0 )
	{
		taskYIELD();
	}
	#endif

	ulState = portDISABLE_INTERRUPTS();
	ulCnt++;
	portRESTORE_INTERRUPTS(ulState);
}
/*-----------------------------------------------------------*/

int c_main( void )
{
    prvInitializeHardware();

    vRunTest();

    vTaskStartScheduler();

    /* should never reach here */
    for( ;; );

    return 0;
}
/*-----------------------------------------------------------*/
