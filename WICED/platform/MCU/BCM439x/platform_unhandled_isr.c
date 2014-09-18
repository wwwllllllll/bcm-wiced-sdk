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
 * Defines BCM439x default unhandled ISR and default mappings to unhandled ISR
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

PLATFORM_SET_DEFAULT_ISR( NMIException        , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( HardFaultException  , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( MemManageException  , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( BusFaultException   , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( UsageFaultException , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( DebugMonitor        , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved016_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved017_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved018_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved019_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved020_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved021_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved022_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved023_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved024_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved025_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved026_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved027_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved028_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved029_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved030_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PTU1_irq            , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( DmaDoneInt_irq      , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved033_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved034_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( WAKEUP_irq          , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( GPIOA_BANK0_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved037_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved038_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved039_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved040_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( GPIOA_BANK1_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved042_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved043_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved044_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved045_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved046_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved047_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved048_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved049_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved050_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved051_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved052_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved053_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved054_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved055_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved056_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved057_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved058_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved059_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved060_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( WL2APPS_irq         , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( WlanReady_irq       , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( Reserved063_irq     , UnhandledInterrupt )
PLATFORM_SET_DEFAULT_ISR( PTU2_irq            , UnhandledInterrupt )
