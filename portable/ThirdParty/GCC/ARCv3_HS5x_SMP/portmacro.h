/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2024 Synopsys, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
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

#ifndef PORTMACRO_H
#define PORTMACRO_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

#include "FreeRTOS.h"

/* record stack high address for stack check */
#ifndef configRECORD_STACK_HIGH_ADDRESS
    #define configRECORD_STACK_HIGH_ADDRESS    1
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR          char
#define portFLOAT         float
#define portDOUBLE        double
#define portLONG          long
#define portSHORT         short
#define portSTACK_TYPE    unsigned int
#define portBASE_TYPE     portLONG

#ifndef Asm
    #define Asm           __asm__ volatile
#endif

/*
 *  normal constants
 */
#ifndef NULL
    #define NULL    0           /* invalid pointer */
#endif /* NULL */

#ifndef true
    #define true    1           /* true */
#endif /* true */

#ifndef false
    #define false    0          /* false */
#endif /* false */

typedef portSTACK_TYPE   StackType_t;
typedef long             BaseType_t;
typedef unsigned long    UBaseType_t;

#if ( configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_16_BITS )
    typedef uint16_t     TickType_t;
    #define portMAX_DELAY    ( TickType_t ) 0xffff
#elif ( configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_32_BITS )
    typedef uint32_t     TickType_t;
    #define portMAX_DELAY    ( TickType_t ) 0xffffffffUL
#else
    #error configTICK_TYPE_WIDTH_IN_BITS set to unsupported tick type width.
#endif

#define portNO_CRITICAL_NESTING    ( ( uint32_t ) 0 )
#define portSTACK_GROWTH           ( -1 )
#define portTICK_PERIOD_MS         ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT         8
#define portNOP()               Asm( "nop_s" );
#define IPM_ENABLE_ALL             1

#define portYIELD_FROM_ISR()    vPortYieldFromIsr()
#define portYIELD()             vPortYield()

/* Critical section management. */
#define portDISABLE_INTERRUPTS() \
    {                            \
        Asm( "clri" );           \
        Asm( "" ::: "memory" );  \
    }                            \

#define portENABLE_INTERRUPTS() \
    {                           \
        Asm( "" ::: "memory" ); \
        Asm( "seti" );          \
    }                           \

extern volatile unsigned int ulCriticalNesting[configNUMBER_OF_CORES];

#define portGET_CRITICAL_NESTING_COUNT()          ( ulCriticalNesting[ portGET_CORE_ID() ] )
#define portSET_CRITICAL_NESTING_COUNT( x )       ( ulCriticalNesting[ portGET_CORE_ID() ] = ( x ) )
#define portINCREMENT_CRITICAL_NESTING_COUNT()    ( ulCriticalNesting[ portGET_CORE_ID() ]++ )
#define portDECREMENT_CRITICAL_NESTING_COUNT()    ( ulCriticalNesting[ portGET_CORE_ID() ]-- )

#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )    void vFunction( void * pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )          void vFunction( void * pvParameters )

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()              do {} while( 0 )     /* we use the timer */
#define portALT_GET_RUN_TIME_COUNTER_VALUE( dest )            ( dest = xTickCount )

#if defined( __MW__ )
    extern void task_end_hook( void * pxTCB );
    #define portCLEAN_UP_TCB( pxTCB )    task_end_hook( ( void * ) pxTCB )
#endif

void vPortYield( void );
void vPortYieldFromIsr( void );

/* For SMP */
#define portNUM_SPINLOCKS   (2 + configNUM_USER_SPINLOCKS)
#define portTASK_LOCK_ID    0
#define portISR_LOCK_ID     1
#define portSPINLOCK_FREE   0xFFFFFFFF

uint32_t arconnect_get_core_id(void);
uint32_t cpu_lock_save( void );
void cpu_unlock_restore( const uint32_t status );

void vPortYieldCore( int xCoreID );
void vTaskEnterCritical( void );
void vTaskExitCritical( void );
UBaseType_t vTaskEnterCriticalFromISR( void );
void vTaskExitCriticalFromISR( UBaseType_t uxSavedInterruptStatus );
void vPortInitSpinLock( void );
void vPortGetSpinLock( uint32_t lock );
void vPortReleaseSpinLock( uint32_t lock );

void vPortArconnectIciHandler(void *p_excinf);

#define portGET_CORE_ID()                                       arconnect_get_core_id()
#define portYIELD_CORE( xCoreID )                               vPortYieldCore( (xCoreID) )
#define portSET_INTERRUPT_MASK()                                cpu_lock_save()
#define portCLEAR_INTERRUPT_MASK( ulState )                     cpu_unlock_restore( (ulState) )
#define portSET_INTERRUPT_MASK_FROM_ISR()                       cpu_lock_save()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( ulState )            cpu_unlock_restore( (ulState) )
#define portGET_TASK_LOCK()                                     vPortGetSpinLock( (portTASK_LOCK_ID) )
#define portRELEASE_TASK_LOCK()                                 vPortReleaseSpinLock( (portTASK_LOCK_ID) )
#define portGET_ISR_LOCK()                                      vPortGetSpinLock( (portISR_LOCK_ID) )
#define portRELEASE_ISR_LOCK()                                  vPortReleaseSpinLock( (portISR_LOCK_ID) )
#define portENTER_CRITICAL()                                    vTaskEnterCritical()
#define portEXIT_CRITICAL()                                     vTaskExitCritical()
#define portENTER_CRITICAL_FROM_ISR()                           vTaskEnterCriticalFromISR()
#define portEXIT_CRITICAL_FROM_ISR( uxSavedInterruptStatus )    vTaskExitCriticalFromISR( (uxSavedInterruptStatus) )
/* End For SMP */

/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* PORTMACRO_H */