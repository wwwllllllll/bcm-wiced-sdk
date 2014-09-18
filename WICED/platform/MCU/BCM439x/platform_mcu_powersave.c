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
 * Defines 4390 MCU powersave functions
 */
#include <string.h>
#include "platform_cmsis.h"
#include "platform_init.h"
#include "platform_sleep.h"
#include "platform_config.h"
#include "platform_peripheral.h"
#include "platform_assert.h"
#include "platform_dct.h"
#include "wwd_constants.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define PDS_DELAY_FROM_STARTUP_MS  ( 5000 )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
static uint32_t power_down_sleep_mode_hook( uint32_t delay_ms );
#else
static uint32_t idle_power_down_hook( uint32_t delay_ms );
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
static volatile wiced_bool_t wake_up_interrupt_triggered = WICED_FALSE;
static volatile uint32_t     clock_needed_counter        = 0;
#endif

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_mcu_powersave_disable( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    /* Atomic operation starts */
    WICED_DISABLE_INTERRUPTS();

    /* Increment counter to indicate CPU clock is needed preventing CPU from entering PDS mode */
    clock_needed_counter++;

    /* Atomic operation ends */
    WICED_ENABLE_INTERRUPTS();
#endif
    return PLATFORM_SUCCESS;
}

platform_result_t platform_mcu_powersave_enable( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    /* Atomic operation starts */
    WICED_DISABLE_INTERRUPTS();

    /* Increment counter to indicate CPU clock is needed preventing CPU from entering PDS mode */
    clock_needed_counter -= ( clock_needed_counter > 0 ) ? 1 : 0;

    /* Atomic operation ends */
    WICED_ENABLE_INTERRUPTS();
#endif
    return PLATFORM_SUCCESS;
}

void platform_mcu_powersave_exit_notify( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    wake_up_interrupt_triggered = WICED_TRUE;
#endif
}

void platform_idle_hook( void )
{
//    __asm("wfi");
}

uint32_t platform_power_down_hook( uint32_t delay_ms )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    return power_down_sleep_mode_hook( delay_ms );
#else
    return idle_power_down_hook( delay_ms );
#endif
}

#ifndef WICED_DISABLE_MCU_POWERSAVE
static uint32_t power_down_sleep_mode_hook( uint32_t delay_ms )
{
    wiced_bool_t pds_delay_from_startup_elapsed = ( host_rtos_get_time() > PDS_DELAY_FROM_STARTUP_MS ) ? WICED_TRUE : WICED_FALSE;
    uint32_t     time_elapsed_ms                = 0;

    /* Criteria to enter PDS mode
     * 1. Clock needed counter is 0 and WICED system tick has progressed over 5 seconds.
     *    This is to give OpenOCD enough time to poke the JTAG tap before the CPU enters PDS mode.
     */
    if ( ( clock_needed_counter == 0 ) && ( pds_delay_from_startup_elapsed == WICED_TRUE ) )
    {
        /* Start atomic operation */
        WICED_DISABLE_INTERRUPTS();

        /* Ensure deep sleep bit is enabled, otherwise system doesn't go into deep sleep */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

        /* Disable SysTick */
        SysTick->CTRL &= ( ~( SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk ) );

        /* Atomic operation ends */
        WICED_ENABLE_INTERRUPTS();

        platform_mcu_powersave_enter_pds_mode( delay_ms );

        time_elapsed_ms = delay_ms;
    }

    /* Start atomic operation */
    WICED_DISABLE_INTERRUPTS();

    /* Switch SysTick back on */
    SysTick->CTRL |= ( SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk );

    /* Clear flag indicating interrupt triggered by wake up pin */
    wake_up_interrupt_triggered = WICED_FALSE;

    /* Atomic operation ends */
    WICED_ENABLE_INTERRUPTS();

    /* Return total time in milliseconds */
    return time_elapsed_ms;
}
#else
static uint32_t idle_power_down_hook( uint32_t delay_ms  )
{
    UNUSED_PARAMETER( delay_ms );
    WICED_ENABLE_INTERRUPTS();
    __asm__("WFI");
    return 0;
}
#endif
