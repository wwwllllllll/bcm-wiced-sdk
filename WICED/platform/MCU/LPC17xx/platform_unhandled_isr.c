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
PLATFORM_SET_DEFAULT_ISR( WDT_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TIMER0_irq             , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TIMER1_irq             , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TIMER2_irq             , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( TIMER3_irq             , UnhandledInterrupt )
//PLATFORM_SET_DEFAULT_ISR( UART0_irq            , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( UART1_irq              , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( UART2_irq              , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( UART3_irq              , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PWM1_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( I2C0_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( I2C1_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( I2C2_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( SPI_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( SSP0_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( SSP1_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PLL0_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( RTC_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( EINT0_irq              , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( EINT1_irq              , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( EINT2_irq              , UnhandledInterrupt )
//PLATFORM_SET_DEFAULT_ISR( EINT3_irq            , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( ADC_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( BOD_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( USB_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( CAN_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( DMA_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( I2S_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( ETH_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( RIT_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( MCPWM_irq              , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( QEI_irq                , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PLL1_irq               , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( USBActivity_irq        , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( CANActivity_irq        , UnhandledInterrupt )


