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
    (uint32_t)WWDG_irq              , // Window WatchDog
    (uint32_t)PVD_irq               , // PVD through EXTI Line detection
    (uint32_t)TAMP_STAMP_irq        , // Tamper and TimeStamps through the EXTI line
    (uint32_t)RTC_WKUP_irq          , // RTC Wakeup through the EXTI line
    (uint32_t)FLASH_irq             , // FLASH
    (uint32_t)RCC_irq               , // RCC
    (uint32_t)EXTI0_irq             , // EXTI Line0
    (uint32_t)EXTI1_irq             , // EXTI Line1
    (uint32_t)EXTI2_irq             , // EXTI Line2
    (uint32_t)EXTI3_irq             , // EXTI Line3
    (uint32_t)EXTI4_irq             , // EXTI Line4
    (uint32_t)DMA1_Stream0_irq      , // DMA1 Stream 0
    (uint32_t)DMA1_Stream1_irq      , // DMA1 Stream 1
    (uint32_t)DMA1_Stream2_irq      , // DMA1 Stream 2
    (uint32_t)DMA1_Stream3_irq      , // DMA1 Stream 3
    (uint32_t)DMA1_Stream4_irq      , // DMA1 Stream 4
    (uint32_t)DMA1_Stream5_irq      , // DMA1 Stream 5
    (uint32_t)DMA1_Stream6_irq      , // DMA1 Stream 6
    (uint32_t)ADC_irq               , // ADC1, ADC2 and ADC3s
    (uint32_t)CAN1_TX_irq           , // CAN1 TX
    (uint32_t)CAN1_RX0_irq          , // CAN1 RX0
    (uint32_t)CAN1_RX1_irq          , // CAN1 RX1
    (uint32_t)CAN1_SCE_irq          , // CAN1 SCE
    (uint32_t)EXTI9_5_irq           , // External Line[9:5]s
    (uint32_t)TIM1_BRK_TIM9_irq     , // TIM1 Break and TIM9
    (uint32_t)TIM1_UP_TIM10_irq     , // TIM1 Update and TIM10
    (uint32_t)TIM1_TRG_COM_TIM11_irq, // TIM1 Trigger and Commutation and TIM11
    (uint32_t)TIM1_CC_irq           , // TIM1 Capture Compare
    (uint32_t)TIM2_irq              , // TIM2
    (uint32_t)TIM3_irq              , // TIM3
    (uint32_t)TIM4_irq              , // TIM4
    (uint32_t)I2C1_EV_irq           , // I2C1 Event
    (uint32_t)I2C1_ER_irq           , // I2C1 Error
    (uint32_t)I2C2_EV_irq           , // I2C2 Event
    (uint32_t)I2C2_ER_irq           , // I2C2 Error
    (uint32_t)SPI1_irq              , // SPI1
    (uint32_t)SPI2_irq              , // SPI2
    (uint32_t)USART1_irq            , // USART1
    (uint32_t)USART2_irq            , // USART2
    (uint32_t)USART3_irq            , // USART3
    (uint32_t)EXTI15_10_irq         , // External Line[15:10]s
    (uint32_t)RTC_Alarm_irq         , // RTC Alarm (A and B) through EXTI Line
    (uint32_t)OTG_FS_WKUP_irq       , // USB OTG FS Wakeup through EXTI line
    (uint32_t)TIM8_BRK_TIM12_irq    , // TIM8 Break and TIM12
    (uint32_t)TIM8_UP_TIM13_irq     , // TIM8 Update and TIM13
    (uint32_t)TIM8_TRG_COM_TIM14_irq, // TIM8 Trigger and Commutation and TIM14
    (uint32_t)TIM8_CC_irq           , // TIM8 Capture Compare
    (uint32_t)DMA1_Stream7_irq      , // DMA1 Stream7
    (uint32_t)FSMC_irq              , // FSMC
    (uint32_t)SDIO_irq              , // SDIO
    (uint32_t)TIM5_irq              , // TIM5
    (uint32_t)SPI3_irq              , // SPI3
    (uint32_t)UART4_irq             , // UART4
    (uint32_t)UART5_irq             , // UART5
    (uint32_t)TIM6_DAC_irq          , // TIM6 and DAC1&2 underrun errors
    (uint32_t)TIM7_irq              , // TIM7
    (uint32_t)DMA2_Stream0_irq      , // DMA2 Stream 0
    (uint32_t)DMA2_Stream1_irq      , // DMA2 Stream 1
    (uint32_t)DMA2_Stream2_irq      , // DMA2 Stream 2
    (uint32_t)DMA2_Stream3_irq      , // DMA2 Stream 3
    (uint32_t)DMA2_Stream4_irq      , // DMA2 Stream 4
    (uint32_t)ETH_irq               , // Ethernet
    (uint32_t)ETH_WKUP_irq          , // Ethernet Wakeup through EXTI line
    (uint32_t)CAN2_TX_irq           , // CAN2 TX
    (uint32_t)CAN2_RX0_irq          , // CAN2 RX0
    (uint32_t)CAN2_RX1_irq          , // CAN2 RX1
    (uint32_t)CAN2_SCE_irq          , // CAN2 SCE
    (uint32_t)OTG_FS_irq            , // USB OTG FS
    (uint32_t)DMA2_Stream5_irq      , // DMA2 Stream 5
    (uint32_t)DMA2_Stream6_irq      , // DMA2 Stream 6
    (uint32_t)DMA2_Stream7_irq      , // DMA2 Stream 7
    (uint32_t)USART6_irq            , // USART6
    (uint32_t)I2C3_EV_irq           , // I2C3 event
    (uint32_t)I2C3_ER_irq           , // I2C3 error
    (uint32_t)OTG_HS_EP1_OUT_irq    , // USB OTG HS End Point 1 Out
    (uint32_t)OTG_HS_EP1_IN_irq     , // USB OTG HS End Point 1 In
    (uint32_t)OTG_HS_WKUP_irq       , // USB OTG HS Wakeup through EXTI
    (uint32_t)OTG_HS_irq            , // USB OTG HS
    (uint32_t)DCMI_irq              , // DCMI
    (uint32_t)CRYP_irq              , // CRYP crypto
    (uint32_t)HASH_RNG_irq          , // Hash and Rng
};

/******************************************************
 *               Function Definitions
 ******************************************************/
