/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *  Implementation of wiced_rtos.c for the case where no RTOS is used
 *
 *  This is a special implementation of the Wiced RTOS
 *  abstraction layer, which is designed to make Wiced work even with
 *  no RTOS.
 *  It provides Wiced with alternates to the functions for threads,
 *  semaphores and time functions.
 *
 *  Semaphores are simple integers, since accesses to an integer will be atomic.
 *
 */

#include "wwd_rtos.h"
#include <stdint.h>
#include "platform_config.h"
#include "platform_cmsis.h"
#include "platform_init.h"

volatile uint32_t noos_total_time = 0;

#define CYCLES_PER_SYSTICK  ( ( CPU_CLOCK_HZ / SYSTICK_FREQUENCY ) - 1 )

void NoOS_setup_timing( void )
{
    /* Setup the system handler priorities */
    SCB->SHP[11] = (uint8_t) 0x40; /* SysTick system handler priority */


    /* Setup the system tick */
    SysTick->LOAD = (uint32_t) ( CYCLES_PER_SYSTICK );
    SysTick->CTRL = (uint32_t) ( SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk );  /* clock source is processor clock - AHB */
}

void NoOS_stop_timing( void )
{
    /* Setup the system handler priorities */
    SCB->SHP[11] = (uint8_t) 0x40; /* SysTick system handler priority */


    /* Setup the system tick */
    SysTick->CTRL &= (uint32_t) ~( SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk );
}

/* NoOS interrupt priority setup. Called by platform_init_mcu_infrastructure() */
void platform_init_rtos_irq_priorities( void )
{
    /* Setup the system handler priorities */
    NVIC_SetPriority( MemoryManagement_IRQn,  0 ); /* Mem Manage system handler priority    */
    NVIC_SetPriority( BusFault_IRQn        ,  0 ); /* Bus Fault system handler priority     */
    NVIC_SetPriority( UsageFault_IRQn      ,  0 ); /* Usage Fault system handler priority   */
    NVIC_SetPriority( SVCall_IRQn          ,  0 ); /* SVCall system handler priority        */
    NVIC_SetPriority( DebugMonitor_IRQn    ,  0 ); /* Debug Monitor system handler priority */
    NVIC_SetPriority( PendSV_IRQn          ,  0 ); /* PendSV system handler priority        */
    NVIC_SetPriority( SysTick_IRQn         , 15 ); /* SysTick system handler priority       */
}

/* NoOS SysTick handler */
WWD_RTOS_DEFINE_ISR( NoOS_systick_irq )
{
    noos_total_time++;
}
