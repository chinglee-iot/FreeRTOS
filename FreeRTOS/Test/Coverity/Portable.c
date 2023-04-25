/*
 * FreeRTOS <DEVELOPMENT BRANCH>
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
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 */

/* Include standard libraries */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "list.h"
#include "queue.h"
#include "portable.h"

volatile BaseType_t xInsideInterrupt = pdFALSE;
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
BaseType_t isInISR = ( BaseType_t ) 0;

extern void vTaskEnterCritical( void );
extern void vTaskExitCritical( void );
extern void vTaskYieldWithinAPI( void );
extern TaskHandle_t xTaskGetCurrentTaskHandle( void );

/* Provide a main function for the build to succeed. */
int main()
{
    vTaskStartScheduler();

    return 0;
}
/*-----------------------------------------------------------*/

void * pvPortMalloc( size_t xWantedSize )
{
    ( void ) xWantedSize;
    return NULL;
}
/*-----------------------------------------------------------*/

void vPortFree( void * pv )
{
    ( void ) pv;
}
/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook( void )
{
    return;
}
/*-----------------------------------------------------------*/

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
    ( void ) ppxTimerTaskTCBBuffer;
    ( void ) ppxTimerTaskStackBuffer;
    ( void ) pulTimerTaskStackSize;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    return;
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    return;
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    return;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    return;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    return pdPASS;
}
/*-----------------------------------------------------------*/

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters,
                                     BaseType_t xRunPrivileged )
{
    ( void ) pxTopOfStack;
    ( void ) pxCode;
    ( void ) pvParameters;
    ( void ) xRunPrivileged;

    return NULL;
}
/*-----------------------------------------------------------*/

void vPortGenerateSimulatedInterrupt()
{
    return;
}
/*-----------------------------------------------------------*/

void portAssert()
{
    return;
}
/*-----------------------------------------------------------*/

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
    ( void ) ppxIdleTaskTCBBuffer;
    ( void ) ppxIdleTaskStackBuffer;
    ( void ) pulIdleTaskStackSize;
}
/*-----------------------------------------------------------*/

void vPortStoreTaskMPUSettings( xMPU_SETTINGS * xMPUSettings,
                                const struct xMEMORY_REGION * const xRegions,
                                StackType_t * pxBottomOfStack,
                                uint32_t ulStackDepth )
{
    ( void ) xMPUSettings;
    ( void ) xRegions;
    ( void ) pxBottomOfStack;
    ( void ) ulStackDepth;
}
/*-----------------------------------------------------------*/

void vPortDefineHeapRegions( const HeapRegion_t * const pxHeapRegions )
{
    ( void ) pxHeapRegions;
}
/*-----------------------------------------------------------*/

void vPortGetHeapStats( HeapStats_t * pxHeapStats )
{
    ( void ) pxHeapStats;
}
/*-----------------------------------------------------------*/

void * pvPortCalloc( size_t xNum,
                     size_t xSize )
{
    ( void ) xNum;
    ( void ) xSize;
    return NULL;
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{

}
/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void )
{
    return 0;
}
/*-----------------------------------------------------------*/

size_t xPortGetMinimumEverFreeHeapSize( void )
{
    return 0;
}
/*-----------------------------------------------------------*/

void vApplicationMinimalIdleHook( void )
{
    return;
}
/*-----------------------------------------------------------*/

void vResetPrivilege( void )
{
    return;
}
/*-----------------------------------------------------------*/

BaseType_t xIsPrivileged( void )
{
    static BaseType_t xIdxIsPrivileged = 0;
    BaseType_t xReturn = pdTRUE;

    xIdxIsPrivileged++;
    if( xIdxIsPrivileged % 2 == 0 )
    {
        xReturn = pdFALSE;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

void vRaisePrivilege( void )
{
    return;
}
/*-----------------------------------------------------------*/
