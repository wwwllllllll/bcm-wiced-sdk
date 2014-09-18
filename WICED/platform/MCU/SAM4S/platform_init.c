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
 * Define default SAM4S initialisation functions
 */
#include "platform_init.h"
#include "platform_isr.h"
#include "platform_peripheral.h"
#include "platform_sleep.h"
#include "platform_config.h"
#include "platform_toolchain.h"
#include "platform/wwd_platform_interface.h"
#include "core_cm4.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define AIRCR_VECTKEY_MASK           ((uint32_t)0x05FA0000)
#define NVIC_PriorityGroup_4         ((uint32_t)0x300) /*!< 4 bits for pre-emption priority
                                                            0 bits for subpriority */

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

WEAK void platform_init_system_clocks( void )
{
    sysclk_init();

    /* Switch Slow Clock source to external 32K crystal */
    pmc_switch_sclk_to_32kxtal( 0 );
    while( pmc_osc_is_ready_32kxtal( ) == 0 )
    {
    }

    pmc_disable_udpck( );
}

WEAK void platform_init_memory( void )
{

}

void platform_init_mcu_infrastructure( void )
{
    uint8_t i;

#ifdef INTERRUPT_VECTORS_IN_RAM
    SCB->VTOR = 0x20000000; /* Change the vector table to point to start of SRAM */
#endif /* ifdef INTERRUPT_VECTORS_IN_RAM */

    /* Initialise watchdog */
    platform_watchdog_init( );

    /* Initialise interrupt priorities */
    for ( i = 0; i < 35; i++ )
    {
        NVIC_SetPriority( i, 0xf );
    }
    //NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

    /* set priority grouping inside cpu core */
    SCB->AIRCR = AIRCR_VECTKEY_MASK | NVIC_PriorityGroup_4;

    platform_init_rtos_irq_priorities();
    platform_init_peripheral_irq_priorities();

#ifndef WICED_DISABLE_MCU_POWERSAVE
    /* Initialise MCU powersave */
    platform_mcu_powersave_init( );
    platform_mcu_powersave_disable( );

    /* Initialise RTC */
//    platform_rtc_init( );
#endif /* ifndef WICED_DISABLE_MCU_POWERSAVE */
}

void platform_init_connectivity_module( void )
{
    /* Ensure 802.11 device is in reset. */
    host_platform_init( );
}

WEAK void platform_init_external_devices( void )
{

}

