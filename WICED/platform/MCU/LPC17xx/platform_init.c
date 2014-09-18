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
 * Define default STM32F2xx initialisation functions
 */
#include "platform_init.h"
#include "platform_peripheral.h"
#include "platform_sleep.h"
#include "platform_config.h"
#include "platform_toolchain.h"
#include "platform/wwd_platform_interface.h"
//#include "fpu_init.h"

/******************************************************
 *                      Macros
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

/* common clock initialisation function
 * This brings up enough clocks to allow the processor to run quickly while initialising memory.
 * Other platform specific clock init can be done in init_platform() or init_host_mcu()
 */
WEAK void platform_init_system_clocks( void )
{

    /* CPU clock source starts with IRC */
    Chip_Clock_SetMainPllSource( SYSCTL_PLLCLKSRC_IRC );
    Chip_Clock_SetCPUClockSource( SYSCTL_CCLKSRC_SYSCLK );

    /* Enable main oscillator used for PLLs */
    LPC_SYSCTL->SCS = SYSCTL_OSCEC;
    while ( ( LPC_SYSCTL->SCS & SYSCTL_OSCSTAT ) == 0 )
    {
    }

    /* PLL0 clock source is 12MHz oscillator, PLL1 can only be the
     main oscillator */
    Chip_Clock_SetMainPllSource( SYSCTL_PLLCLKSRC_MAINOSC );

    /* Setup PLL0 for a 480MHz clock. It is divided by CPU Clock Divider to create CPU Clock.
     Input clock rate (FIN) is main oscillator = 12MHz
     FCCO is selected for PLL Output and it must be between 275 MHz to 550 MHz.
     FCCO = (2 * M * FIN) / N = integer multiplier of CPU Clock (120MHz) = 480MHz
     N = 1, M = 480 * 1/(2*12) = 20 */
    Chip_Clock_SetupPLL( SYSCTL_MAIN_PLL, PLL_M_CONSTANT - 1, PLL_N_CONSTANT - 1 );/* Multiply by PLL_M_CONSTANT, Divide by PLL_N_CONSTANT */

    /* Enable PLL0 */
    Chip_Clock_EnablePLL( SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE );

    /* Change the CPU Clock Divider setting for the operation with PLL0.
     Divide value = (480/120) = 4 */
    Chip_Clock_SetCPUClockDiv( 3 ); /* pre-minus 1 */

    while ( !Chip_Clock_IsMainPLLLocked( ) )
    {
    }

    /* Connect PLL0 */
    Chip_Clock_EnablePLL( SYSCTL_MAIN_PLL, SYSCTL_PLL_ENABLE | SYSCTL_PLL_CONNECT );

    /* Wait for PLL0 to be connected */
    while ( !Chip_Clock_IsMainPLLConnected( ) )
    {
    }

    /* Setup FLASH access to 5 clocks (120MHz clock) */
    Chip_FMC_SetFLASHAccess( FLASHTIM_120MHZ_CPU );

    /* Enable peripheral base clocks*/
    Chip_Clock_SetPCLKDiv( SYSCTL_PCLK_SPI, SYSCTL_CLKDIV_1 );

    Chip_RTC_Enable( LPC_RTC, ENABLE );
    Chip_Clock_SetCLKOUTSource( SYSCTL_CLKOUTSRC_RTC, 1 );
    Chip_Clock_EnableCLKOUT( );
}

WEAK void platform_init_memory( void )
{

}

void platform_init_mcu_infrastructure( void )
{
    uint8_t i;

    /* Initialise interrupt priorities */
    for ( i = 0; i < 34; i++ )
    {
        NVIC_SetPriority( i, 0xf );
    }
    platform_init_rtos_irq_priorities();
    platform_init_peripheral_irq_priorities();
}

void platform_init_connectivity_module( void )
{
    /* Ensure 802.11 device is in reset. */
    host_platform_init( );
}

WEAK void platform_init_external_devices( void )
{

}
