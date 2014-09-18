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
 * BCM439x interrupt handlers. BCM439x peripheral drivers are delivered in library format.
 * Common interrupt handlers are placed here to avoid the need to deliver RTOS-specific
 * peripheral libraries.
 */
#include "platform_isr_interface.h"
#include "platform_mcu_peripheral.h"
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

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

#ifdef DMA_ENABLED
extern void dma_isrHandler( void );
#endif

/******************************************************
 *            Interrupt Handler Definitions
 ******************************************************/

/**
 * Interrupt handler for DMA
 */
WWD_RTOS_DEFINE_ISR( dma_done_irq )
{
#ifdef DMA_ENABLED
    dma_isrHandler();
#endif
}
WWD_RTOS_MAP_ISR( dma_done_irq, DmaDoneInt_irq )

/**
 * Interrupt handler for GPIOA
 */
WWD_RTOS_DEFINE_ISR( gpioa_irq )
{
    platform_gpio_irq( );
}
WWD_RTOS_MAP_ISR( gpioa_irq, GPIOA_BANK0_irq ) /* Map all GPIOA Bank 0 interrupts to gpioa_irq */
WWD_RTOS_MAP_ISR( gpioa_irq, GPIOA_BANK1_irq ) /* Map all GPIOA Bank 1 interrupts to gpioa_irq */

/**
 * Interrupt handler for MCU wake up from power-down-sleep (PDS) mode
 */
WWD_RTOS_DEFINE_ISR( WAKEUP_irq )
{
    platform_mcu_powersave_wakeup_irq( );
}
