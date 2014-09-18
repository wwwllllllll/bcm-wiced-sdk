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
 * STM32F2xx vector table
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

#ifndef SVC_irq
#define SVC_irq UnhandledInterrupt
#endif

#ifndef PENDSV_irq
#define PENDSV_irq UnhandledInterrupt
#endif

#ifndef SYSTICK_irq
#define SYSTICK_irq UnhandledInterrupt
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

extern void UnhandledInterrupt( void );
extern void reset_handler     ( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Pointer to stack location */
extern void* link_stack_end;

uint32_t interrupt_vector_table[] =
{
    (uint32_t)&link_stack_end       , // Initial stack location
    (uint32_t)reset_handler         , // Reset vector
    (uint32_t)NMIException          , // Non Maskable Interrupt
    (uint32_t)HardFaultException    , // Hard Fault interrupt
    (uint32_t)MemManageException    , // Memory Management Fault interrupt
    (uint32_t)BusFaultException     , // Bus Fault interrupt
    (uint32_t)UsageFaultException   , // Usage Fault interrupt
    (uint32_t)0                     , // Reserved
    (uint32_t)0                     , // Reserved
    (uint32_t)0                     , // Reserved
    (uint32_t)0                     , // Reserved
    (uint32_t)SVC_irq               , // SVC interrupt
    (uint32_t)DebugMonitor          , // Debug Monitor interrupt
    (uint32_t)0                     , // Reserved
    (uint32_t)PENDSV_irq            , // PendSV interrupt
    (uint32_t)SYSTICK_irq           , // Sys Tick Interrupt
    (uint32_t)SUPPLY_CTRL_irq       ,
    (uint32_t)RESET_CTRL_irq        ,
    (uint32_t)RTC_irq               ,
    (uint32_t)RTT_irq               ,
    (uint32_t)WDT_irq               ,
    (uint32_t)PMC_irq               ,
    (uint32_t)EEFC_irq              ,
    (uint32_t)0                     , // Reserved
    (uint32_t)UART0_irq             ,
    (uint32_t)UART1_irq             ,
    (uint32_t)SMC_irq               ,
    (uint32_t)PIO_CTRL_A_irq        ,
    (uint32_t)PIO_CTRL_B_irq        ,
    (uint32_t)PIO_CTRL_C_irq        ,
    (uint32_t)USART0_irq            ,
    (uint32_t)USART1_irq            ,
    (uint32_t)0                     , //Reserved
    (uint32_t)0                     , //Reserved
    (uint32_t)MCI_irq               ,
    (uint32_t)TWI0_irq              ,
    (uint32_t)TWI1_irq              ,
    (uint32_t)SPI_irq               ,
    (uint32_t)SSC_irq               ,
    (uint32_t)TC0_irq               ,
    (uint32_t)TC1_irq               ,
    (uint32_t)TC2_irq               ,
    (uint32_t)TC3_irq               ,
    (uint32_t)TC4_irq               ,
    (uint32_t)TC5_irq               ,
    (uint32_t)ADC_irq               ,
    (uint32_t)DAC_irq               ,
    (uint32_t)PWM_irq               ,
    (uint32_t)CRCCU_ir              ,
    (uint32_t)AC_irq                ,
    (uint32_t)USB_irq               ,
    (uint32_t)0
};

/******************************************************
 *               Function Definitions
 ******************************************************/
