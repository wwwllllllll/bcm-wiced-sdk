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
 * Define default BCM439x initialisation functions
 */
#include "platform_init.h"
#include "platform_peripheral.h"
#include "platform_sleep.h"
#include "platform_cmsis.h"
#include "platform_config.h"
#include "platform_toolchain.h"
#include "platform/wwd_platform_interface.h"
#include "network/wwd_network_constants.h"

#ifndef WICED_DISABLE_BOOTLOADER
#include "wiced_framework.h"
#endif

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

extern void platform_filesystem_init( void );

int platform_get_4390_rx_buffer_size( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

char bcm439x_platform_inited = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

/* BCM439x common clock initialisation function
 * - weak attribute is intentional in case a specific platform (board) needs to override this.
 */
WEAK void platform_init_system_clocks( void )
{
    init_4390_after_restart( );
}

WEAK void platform_init_memory( void )
{

}

void platform_init_mcu_infrastructure( void )
{
    uint8_t i;

    if ( bcm439x_platform_inited == 1 )
        return;

    init_4390_after_global_init();

    platform_mcu_powersave_disable( );

    platform_watchdog_init( );

    /* Initialise the interrupt priorities to a priority lower than 0 so that the BASEPRI register can mask them */
    for ( i = 0; i < 81; i++ )
    {
//        NVIC ->IP[i] = 0xff;
    }
    platform_init_rtos_irq_priorities( );
    platform_init_peripheral_irq_priorities( );

    /* Initialise peripheral transport units (PTU) before any other peripherals */
    platform_ptu1_init( );
    platform_ptu2_init( );

    /* Initialise GPIO IRQ manager */
    platform_gpio_irq_manager_init();

    /* Initialise filesystem */
    platform_filesystem_init();

    bcm439x_platform_inited = 1;
}

void platform_init_connectivity_module( void )
{
    /* Ensure 802.11 device is in reset. */
    host_platform_init( );
}

WEAK void platform_init_external_devices( void )
{

}

void platform_setup_otp( void )
{
    set_OTP_sflash_boot_439x( );
}

/* This function is needed so that the MTU size does not get compiled into the 4390 prebuilt library in releases */
int platform_get_4390_rx_buffer_size( void )
{
    return WICED_LINK_MTU;
}
