/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* Hardware includes. */
#include "msp430.h"
#include "hal_MSP-EXP430F5438.h"

/* This demo uses Timer A1 to generate the time base for gathering run time
 * statistics information.  Run time statistics show much processing time has
 * been allocated to each task since the application booted. */

/* The time base for the run time stats is generated by a 16 bit timer.  Each
 * time the timer overflows ulStatsOverflowCount is incremented.  Therefore,
 * when converting the total run time to a 32 bit number, the most significant two
 * bytes are given by ulStatsOverflowCount and the least significant two bytes are
 * given by the current timer counter value.  Care must be taken with data
 * consistency when combining the two in case a timer overflow occurs as the
 * value is being read. */
volatile unsigned long ulStatsOverflowCount = 0;

/*-----------------------------------------------------------*/

void vConfigureTimerForRunTimeStats( void )
{
    /* Ensure the timer is stopped. */
    TA1CTL = 0;

    /* Run the timer from the ACLK/2. */
    TA1CTL = TASSEL_1 | ID__2;

    /* Clear everything to start with. */
    TA1CTL |= TACLR;

    /* Enable the interrupts. */
    TA1CCTL0 = CCIE;

    /* Start up clean. */
    TA1CTL |= TACLR;

    /* Continuous mode. */
    TA1CTL |= MC__CONTINOUS;
}
/*-----------------------------------------------------------*/

#pragma vector=TIMER1_A0_VECTOR
interrupt void prvRunTimeStatsOverflowISR( void )
{
    ulStatsOverflowCount++;
}
/*-----------------------------------------------------------*/
