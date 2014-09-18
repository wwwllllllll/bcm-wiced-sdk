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
 *
 * Debugging Watchdog Reset
 *
 * 1. If a watchdog reset occured previously, a warning message will be printed to the console
 * 2. To obtain useful information for debugging, run WICED with JTAG debug mode and attempt to reproduce the issue
 * 3. While running in JTAG debug mode, WICED starts a shadow watchdog timer.
 *    - It utilises STM32F2 TIM7.
 *    - It's expiry time is set 90% of that of the actual watchdog.
 *    - It also get reloaded together with the actual watchdog when the watchdog kick function is called.
 *    - When the problem is reproduced, a break point is generated from the shadow watchdog ISR - shadow_watchdog_irq()
 * 4. If the code breaks in shadow_watchdog_irq(), it means that the independent watchdog is about to bite.
 *    - Observe the Debug view and examine where the software gets stuck and why the watchdog wasn't kicked.
 *    - Click "Resume" to continue and let the actual watchdog take effect.
 */

#include "platform_cmsis.h"
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_stdio.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "platform_assert.h"
#include "wwd_assert.h"
#include "wwd_rtos.h"
#include "wiced_defaults.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define WATCHDOG_PRESCALER              (IWDG_Prescaler_256)
#define WATCHDOG_TIMEOUT_MULTIPLIER     (184)
#define DBG_WATCHDOG_TIMEOUT_MULTIPLIER (2250)
#define DBG_WATCHDOG_PRESCALER          (24000)

#ifdef APPLICATION_WATCHDOG_TIMEOUT_SECONDS
#define WATCHDOG_TIMEOUT                (APPLICATION_WATCHDOG_TIMEOUT_SECONDS * WATCHDOG_TIMEOUT_MULTIPLIER)
#define DBG_WATCHDOG_TIMEOUT            (APPLICATION_WATCHDOG_TIMEOUT_SECONDS * DBG_WATCHDOG_TIMEOUT_MULTIPLIER)
#else
#define WATCHDOG_TIMEOUT                (MAX_WATCHDOG_TIMEOUT_SECONDS * WATCHDOG_TIMEOUT_MULTIPLIER)
#define DBG_WATCHDOG_TIMEOUT            (MAX_WATCHDOG_TIMEOUT_SECONDS * DBG_WATCHDOG_TIMEOUT_MULTIPLIER)
#endif /* APPLICATION_WATCHDOG_TIMEOUT_SECONDS */

#if (defined(APPLICATION_WATCHDOG_TIMEOUT_SECONDS) && (APPLICATION_WATCHDOG_TIMEOUT_SECONDS > MAX_WATCHDOG_TIMEOUT_SECONDS))
#error APPLICATION_WATCHDOG_TIMEOUT_SECONDS must NOT be larger than 22 seconds
#endif

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

#ifndef WICED_DISABLE_WATCHDOG
static void init_dbg_watchdog  ( void );
static void reload_dbg_watchdog( void );
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_watchdog_init( void )
{
#ifndef WICED_DISABLE_WATCHDOG
    /* Allow writign to the Watchdog registers */
    IWDG_WriteAccessCmd( IWDG_WriteAccess_Enable );

    /* Watchdog frequency calculations
     *
     * LSI : min 17kHz, typ 32kHz, max 47kHz
     *
     * Set prescaler to divide by 256
     *
     * Watchdog count freq: min 66.4Hz, typ 125Hz, max 183.6Hz
     *
     * Reload value maximum = 4095,
     * so minimum time to reset with this value is 22.3 seconds
     *
     */
    /* Set the Watchdog prescaler to LSI/256 */
    IWDG_SetPrescaler(WATCHDOG_PRESCALER);

    /* Set the reload value to obtain the requested minimum time to reset */
    IWDG_SetReload( (uint16_t)WATCHDOG_TIMEOUT );

    /* Start the watchdog */
    IWDG_ReloadCounter();
    IWDG_Enable();

    /* shadow watchdog for debugging lockup issue. Enabled only during debugging */
    if ( DBGMCU->APB1FZ & DBGMCU_APB1_FZ_DBG_IWDEG_STOP )
    {
        init_dbg_watchdog();
    }

    return PLATFORM_SUCCESS;
#else
    return PLATFORM_FEATURE_DISABLED;
#endif
}

platform_result_t platform_watchdog_kick( void )
{
#ifndef WICED_DISABLE_WATCHDOG
    /* Reload IWDG counter */
    IWDG_ReloadCounter( );

    if ( DBGMCU->APB1FZ & DBGMCU_APB1_FZ_DBG_IWDEG_STOP )
    {
        reload_dbg_watchdog();
    }

    return PLATFORM_SUCCESS;
#else
    return PLATFORM_FEATURE_DISABLED;
#endif
}

wiced_bool_t platform_watchdog_check_last_reset( void )
{
#ifndef WICED_DISABLE_WATCHDOG
    if ( RCC->CSR & RCC_CSR_WDGRSTF )
    {
        /* Clear the flag and return */
        RCC->CSR |= RCC_CSR_RMVF;
        return WICED_TRUE;
    }
#endif

    return WICED_FALSE;
}

#ifndef WICED_DISABLE_WATCHDOG
static void init_dbg_watchdog( void )
{
    TIM_TimeBaseInitTypeDef tim_time_base_init_struct;

    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM7, ENABLE  );
    RCC_APB1PeriphResetCmd( RCC_APB1Periph_TIM7, DISABLE );

    /* Set dbg_watchdog timeout to 90% of the actual watchdog timeout to ensure it break before the actual watchdog bites
     * Timeout calculation
     * - Period per TIM clock cycle : 120MHz (CPU clock) / 2 (APB1 pre-scaler) / DBG_WATCHDOG_PRESCALER = 2.5ms
     *                                DBG_WATCHDOG_PRESCALER = 24000
     * - Timeout                    : ( 0.9 * 2.5ms * 1000 ) * timeout_in_seconds
     *                                DBG_WATCHDOG_TIMEOUT_MULTIPLIER =  ( 0.9 * 2.5ms * 1000 ) = 2250
     */

    tim_time_base_init_struct.TIM_Prescaler         = DBG_WATCHDOG_PRESCALER;
    tim_time_base_init_struct.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_time_base_init_struct.TIM_Period            = DBG_WATCHDOG_TIMEOUT;
    tim_time_base_init_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_time_base_init_struct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit( TIM7, &tim_time_base_init_struct );

    TIM_ClearITPendingBit( TIM7, TIM_IT_Update );
    TIM_ITConfig( TIM7, TIM_IT_Update, ENABLE );

    TIM_UpdateRequestConfig( TIM7, TIM_UpdateSource_Regular );

    NVIC_EnableIRQ( TIM7_IRQn );

    TIM_Cmd( TIM7, ENABLE );
}

static void reload_dbg_watchdog( void )
{
    TIM_TimeBaseInitTypeDef tim_time_base_init_struct;

    TIM_Cmd( TIM7, DISABLE );

    tim_time_base_init_struct.TIM_Prescaler         = DBG_WATCHDOG_PRESCALER;
    tim_time_base_init_struct.TIM_CounterMode       = TIM_CounterMode_Up;
    tim_time_base_init_struct.TIM_Period            = DBG_WATCHDOG_TIMEOUT;
    tim_time_base_init_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_time_base_init_struct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit( TIM7, &tim_time_base_init_struct );

    TIM_Cmd( TIM7, ENABLE );
}
#endif /* ifndef WICED_DISABLE_WATCHDOG */

/******************************************************
 *             IRQ Handlers Definition
 ******************************************************/

PLATFORM_DEFINE_ISR( dbg_watchdog_irq )
{
    /* If the code breaks here, it means that the independent watchdog is about to bite.
     * Observe the Debug view and examine where the software gets stuck and why
     * the watchdog wasn't kicked.
     * Click "Resume" to continue and let the actual watchdog take effect.
     */
    TIM7->SR = (uint16_t)~TIM_IT_Update;
    WICED_TRIGGER_BREAKPOINT( );
}

/******************************************************
 *               IRQ Handlers Mapping
 ******************************************************/

PLATFORM_MAP_ISR( dbg_watchdog_irq, TIM7_irq )

