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
 * Defines BCM439x non-WWD bus functions
 */
#include <stdint.h>
#include "wwd_constants.h"
#include "wwd_assert.h"
#include "platform_peripheral.h"
#include "platform_config.h"
#include "wiced_platform.h"
#include "platform/wwd_platform_interface.h"
#include "platform_cmsis.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

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
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wwd_result_t host_platform_init( void )
{
    return WWD_SUCCESS;
}

void host_platform_reset_wifi( wiced_bool_t reset_asserted )
{
    UNUSED_PARAMETER( reset_asserted );
}

void host_platform_power_wifi( wiced_bool_t power_enabled )
{
    if ( power_enabled == WICED_FALSE )
    {
#if 0
        /* en_WLAN_SRAM  = 0 */
        *(volatile int *)0x00640160 &= ~(0x1);
#else
        /* en_WLAN_SRAM  = 1 */
        *(volatile int *) 0x00640160 |= ( 0x1 );
#endif
        //    *(int *)0x00640084 |= 0xc;
        /* Watchdog reset */
        *(volatile int *) 0x700080 = 1;
        host_rtos_delay_milliseconds( 100 );
        /* set apps2wl_wl_wake = 0 and apps2wl_wl_pwrup = 0 */
        *(volatile int *) 0x0064015c &= ~( 0x3 );
        /* delay 100 ms */
        host_rtos_delay_milliseconds( 100 );
    }
}

wwd_result_t host_platform_deinit( void )
{
    host_platform_power_wifi( WICED_FALSE );
    return WWD_SUCCESS;
}

uint32_t host_platform_get_cycle_count( void )
{
    /* From the ARM Cortex-M3 Techinical Reference Manual
     * 0xE0001004  DWT_CYCCNT  RW  0x00000000  Cycle Count Register */
    return DWT->CYCCNT;
}

wwd_result_t host_platform_init_wlan_powersave_clock( void )
{
    return WWD_SUCCESS;
}

wwd_result_t host_platform_deinit_wlan_powersave_clock( void )
{
    return WWD_SUCCESS;
}

wiced_bool_t host_platform_is_in_interrupt_context( void )
{
    /* From the ARM Cortex-M3 Techinical Reference Manual
     * 0xE000ED04   ICSR    RW [a]  Privileged  0x00000000  Interrupt Control and State Register */
    uint32_t active_interrupt_vector = (uint32_t)( SCB ->ICSR & SCB_ICSR_VECTACTIVE_Msk );

    if ( active_interrupt_vector != 0 )
    {
        return WICED_TRUE;
    }
    else
    {
        return WICED_FALSE;
    }
}
