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
 */
#include <stdint.h>
#include <string.h>
#include "platform_cmsis.h"
#include "platform_peripheral.h"
#include "platform_config.h"
#include "wwd_constants.h"
#include "wwd_platform_common.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"
#include "wiced_framework.h"

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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wwd_result_t host_platform_init( void )
{
    host_platform_deinit_wlan_powersave_clock( );

#if defined ( WICED_USE_WIFI_RESET_PIN )
    platform_gpio_init( &wifi_control_pins[WWD_PIN_RESET], OUTPUT_PUSH_PULL );
    host_platform_reset_wifi( WICED_TRUE );  /* Start wifi chip in reset */
#endif

#if defined ( WICED_USE_WIFI_POWER_PIN )
    platform_gpio_init( &wifi_control_pins[WWD_PIN_POWER], OUTPUT_PUSH_PULL );
    host_platform_power_wifi( WICED_FALSE ); /* Start wifi chip with regulators off */
#endif
    return WWD_SUCCESS;
}

wwd_result_t host_platform_deinit( void )
{
#if defined ( WICED_USE_WIFI_RESET_PIN )
    platform_gpio_init( &wifi_control_pins[WWD_PIN_RESET], OUTPUT_PUSH_PULL );
    host_platform_reset_wifi( WICED_TRUE );  /* Start wifi chip in reset */
#endif

#if defined ( WICED_USE_WIFI_POWER_PIN )
    platform_gpio_init( &wifi_control_pins[WWD_PIN_POWER], OUTPUT_PUSH_PULL );
    host_platform_power_wifi( WICED_FALSE ); /* Start wifi chip with regulators off */
#endif

    host_platform_deinit_wlan_powersave_clock( );

    return WWD_SUCCESS;
}

void host_platform_reset_wifi( wiced_bool_t reset_asserted )
{
#if defined ( WICED_USE_WIFI_RESET_PIN )
    ( reset_asserted == WICED_TRUE ) ? platform_gpio_output_low( &wifi_control_pins[ WWD_PIN_RESET ] ) : platform_gpio_output_high( &wifi_control_pins[ WWD_PIN_RESET ] );
#else
    UNUSED_PARAMETER( reset_asserted );
#endif
}

void host_platform_power_wifi( wiced_bool_t power_enabled )
{
#if   defined ( WICED_USE_WIFI_POWER_PIN ) && defined ( WICED_USE_WIFI_POWER_PIN_ACTIVE_HIGH )
    ( power_enabled == WICED_TRUE ) ? platform_gpio_output_high( &wifi_control_pins[WWD_PIN_POWER] ) : platform_gpio_output_low ( &wifi_control_pins[WWD_PIN_POWER] );
#elif defined ( WICED_USE_WIFI_POWER_PIN )
    ( power_enabled == WICED_TRUE ) ? platform_gpio_output_low ( &wifi_control_pins[WWD_PIN_POWER] ) : platform_gpio_output_high( &wifi_control_pins[WWD_PIN_POWER] );
#else
    UNUSED_PARAMETER( power_enabled );
#endif
}

uint32_t host_platform_get_cycle_count( void )
{
    /* From the ARM Cortex-M3 Techinical Reference Manual
     * 0xE0001004  DWT_CYCCNT  RW  0x00000000  Cycle Count Register */
    return DWT->CYCCNT;
}

wiced_bool_t host_platform_is_in_interrupt_context( void )
{
    /* From the ARM Cortex-M3 Techinical Reference Manual
     * 0xE000ED04   ICSR    RW [a]  Privileged  0x00000000  Interrupt Control and State Register */
    return ( ( SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk ) != 0 ) ? WICED_TRUE : WICED_FALSE;
}

void host_platform_get_mac_address( wiced_mac_t* mac )
{
#if !defined ( WICED_DISABLE_BOOTLOADER )
    wiced_mac_t* temp_mac;
    wiced_dct_read_lock( (void**) &temp_mac, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, OFFSETOF( platform_dct_wifi_config_t, mac_address ), sizeof(wiced_mac_t) );
    memcpy( mac->octet, temp_mac, sizeof(wiced_mac_t) );
    wiced_dct_read_unlock( temp_mac, WICED_FALSE );
#else
    UNUSED_PARAMETER( mac );
#endif
}

wwd_result_t host_platform_deinit_wlan_powersave_clock( void )
{
    platform_gpio_init( &wifi_control_pins[WWD_PIN_32K_CLK], OUTPUT_PUSH_PULL );
    platform_gpio_output_low( &wifi_control_pins[WWD_PIN_32K_CLK] );
    return WWD_SUCCESS;
}
