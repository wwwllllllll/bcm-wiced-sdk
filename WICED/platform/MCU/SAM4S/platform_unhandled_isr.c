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
 * Defines STM32F2xx default unhandled ISR and default mappings to unhandled ISR
 */
#include <stdint.h>
#include "platform_cmsis.h"
#include "platform_assert.h"
#include "platform_constants.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos.h"

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

extern void UnhandledInterrupt( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

PLATFORM_DEFINE_ISR( UnhandledInterrupt )
{
    uint32_t active_interrupt_vector = (uint32_t) ( SCB->ICSR & 0x3fU );

    /* This variable tells you which interrupt vector is currently active */
    (void)active_interrupt_vector;
    WICED_TRIGGER_BREAKPOINT( );

    while( 1 )
    {
    }
}

/******************************************************
 *          Default IRQ Handler Declarations
 ******************************************************/

PLATFORM_SET_DEFAULT_ISR( NMIException           , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( HardFaultException     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( MemManageException     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( BusFaultException      , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( UsageFaultException    , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( DebugMonitor           , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( SUPPLY_CTRL_irq        , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( RESET_CTRL_irq         , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( RTC_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( RTT_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( WDT_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PMC_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( EEFC_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( UART0_irq              , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( UART1_irq              , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( SMC_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PIO_CTRL_A_irq         , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PIO_CTRL_B_irq         , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PIO_CTRL_C_irq         , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( USART0_irq             , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( USART1_irq             , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( MCI_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TWI0_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TWI1_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( SPI_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( SSC_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TC0_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TC1_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TC2_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TC3_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TC4_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TC5_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( ADC_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( DAC_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PWM_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( CRCCU_ir               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( AC_irq                 , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( USB_irq                , UnhandledInterrupt )

