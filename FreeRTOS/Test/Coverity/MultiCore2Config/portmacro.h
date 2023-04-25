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

#ifndef PORTMACRO_H
#define PORTMACRO_H

/******************************************************************************
*   Defines
******************************************************************************/
/* Type definitions. */
#define portCHAR                 char
#define portFLOAT                float
#define portDOUBLE               double
#define portLONG                 long
#define portSHORT                short
#define portSTACK_TYPE           int32_t
#define portBASE_TYPE            long
#define portPOINTER_SIZE_TYPE    int32_t

#define configISR_STACK_SIZE_WORDS

typedef portSTACK_TYPE   StackType_t;
typedef int32_t          BaseType_t;
typedef uint32_t         UBaseType_t;

#if ( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t     TickType_t;
    #define portMAX_DELAY              ( TickType_t ) 0xffff
#else
    typedef uint32_t     TickType_t;
    #define portMAX_DELAY              ( TickType_t ) 0xffffffffUL

/* 32/64-bit tick type on a 32/64-bit architecture, so reads of the tick
 * count do not need to be guarded with a critical section. */
    #define portTICK_TYPE_IS_ATOMIC    1
#endif

/* Hardware specifics. */
#define portSTACK_GROWTH      ( -1 )
#define portTICK_PERIOD_MS    ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portINLINE

#if defined( __x86_64__ ) || defined( _M_X64 )
    #define portBYTE_ALIGNMENT    8
#else
    #define portBYTE_ALIGNMENT    4
#endif


#define portYIELD()    
#define portSOFTWARE_BARRIER()
#define portGET_CORE_ID()                         ( ( BaseType_t ) 1 )
#define portYIELD_CORE(...)
#define portSET_INTERRUPT_MASK(...)               ( ( UBaseType_t ) 0 )
#define portCLEAR_INTERRUPT_MASK(...)
#define portRELEASE_TASK_LOCK()
#define portGET_TASK_LOCK()
#define portRELEASE_ISR_LOCK()
#define portGET_ISR_LOCK()

extern BaseType_t isInISR;
#define portCHECK_IF_IN_ISR(...)                  ( isInISR++ > ( BaseType_t ) 2 )

#define portENTER_CRITICAL_FROM_ISR()             ( ( UBaseType_t ) 0 )
#define portEXIT_CRITICAL_FROM_ISR(...)


/* Simulated interrupts return pdFALSE if no context switch should be performed,
 * or a non-zero number if a context switch should be performed. */
#define portYIELD_FROM_ISR( x )       ( void ) x
#define portEND_SWITCHING_ISR( x )    portYIELD_FROM_ISR( ( x ) )

#define portCLEAN_UP_TCB( pxTCB )
#define portPRE_TASK_DELETE_HOOK( pvTaskToDelete, pxPendYield )
#define portDISABLE_INTERRUPTS()
#define portENABLE_INTERRUPTS()

/* Critical section handling. */
extern void vTaskEnterCritical( void );
extern void vTaskExitCritical( void );
#define portENTER_CRITICAL()
#define portEXIT_CRITICAL()

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )    void vFunction( const void * pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )          void vFunction( const void * pvParameters )

#define portINTERRUPT_YIELD    ( 0UL )

#define portGET_RUN_TIME_COUNTER_VALUE(...)         ( 100U )

extern void vResetPrivilege( void );
extern BaseType_t xIsPrivileged( void );
extern void vRaisePrivilege( void );
#define portIS_PRIVILEGED()         xIsPrivileged()
#define portRESET_PRIVILEGE()       vResetPrivilege()
#define portRAISE_PRIVILEGE()       vRaisePrivilege()

/**
 * @brief MPU settings as stored in the TCB.
 */
typedef struct MPU_SETTINGS
{
    uint32_t dummy;
} xMPU_SETTINGS;

#endif /* ifndef PORTMACRO_H */
