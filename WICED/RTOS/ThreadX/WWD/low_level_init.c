/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <stdint.h>
#include "tx_api.h"
#include "tx_initialize.h"
#include "platform_assert.h"
#include "platform_config.h"
#include "platform_cmsis.h"
#include "platform_init.h"
#include "wwd_rtos.h"

#define CYCLES_PER_SYSTICK ( ( CPU_CLOCK_HZ / SYSTICK_FREQUENCY ) - 1 )

extern void _tx_timer_interrupt( void );

/*@external@*/ extern uint32_t interrupt_vector_table[];
extern void* _tx_initialize_unused_memory;
extern void* _tx_thread_system_stack_ptr;

/* ThreadX  kernel initialisation function */
void _tx_initialize_low_level( void )
{
    /* Setup some ThreadX internal values */
    _tx_initialize_unused_memory = (void*)0xbaadbaad;  /* TODO : add proper value here */
    _tx_thread_system_stack_ptr  = (void*)interrupt_vector_table[0];

    /* Setup the system tick */
    SysTick->LOAD = (uint32_t) ( CYCLES_PER_SYSTICK );
    SysTick->CTRL = (uint32_t) ( SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_CLKSOURCE_Msk );  /* clock source is processor clock - AHB */
}

/* ThreadX interrupt priority setup. Called by platform_init_mcu_infrastructure() */
void platform_init_rtos_irq_priorities( void )
{
    /* Setup the system handler priorities */
    NVIC_SetPriority( MemoryManagement_IRQn,  0 ); /* Mem Manage system handler priority    */
    NVIC_SetPriority( BusFault_IRQn        ,  0 ); /* Bus Fault system handler priority     */
    NVIC_SetPriority( UsageFault_IRQn      ,  0 ); /* Usage Fault system handler priority   */
    NVIC_SetPriority( SVCall_IRQn          , 15 ); /* SVCall system handler priority        */
    NVIC_SetPriority( DebugMonitor_IRQn    ,  0 ); /* Debug Monitor system handler priority */
    NVIC_SetPriority( PendSV_IRQn          , 15 ); /* PendSV system handler priority        */
    NVIC_SetPriority( SysTick_IRQn         ,  4 ); /* SysTick system handler priority       */
}

/* ThreadX System Tick IRQ handler */
WWD_RTOS_DEFINE_ISR( SysTickHandler )
{
    _tx_timer_interrupt();
}
