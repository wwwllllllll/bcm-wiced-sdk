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
 * Defines internal configuration of the BCM943362WCD8 board
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* MCU constants and options */
//#define CPU_CLOCK_HZ                   (123000000)
//#define PERIPHERAL_CLOCK_HZ            (123000000)
//
///* SDIO_4_BIT */
///* WAIT_MODE_SUPPORT */
///* WAIT_MODE_ENTER_DELAY_CYCLES = 3 */
///* WAIT_MODE_EXIT_DELAY_CYCLES  = 34 */
//#define __SAM4S16B__
//
///* ASF required defines */
//#define ARM_MATH_CM4                   (true)
//#define CONFIG_SYSCLK_SOURCE           (SYSCLK_SRC_PLLACK)
//#define CONFIG_SYSCLK_PRES             (SYSCLK_PRES_1)
//#define CONFIG_PLL0_SOURCE             (PLL_SRC_MAINCK_XTAL)
//#define CONFIG_PLL0_MUL                (41)
//#define CONFIG_PLL0_DIV                (4)
//#define BOARD                          (USER_BOARD)
//#define BOARD_FREQ_SLCK_XTAL           (32768)
//#define BOARD_FREQ_SLCK_BYPASS         (32768)
//#define BOARD_FREQ_MAINCK_XTAL         (12000000)
//#define BOARD_FREQ_MAINCK_BYPASS       (12000000)
//#define BOARD_OSC_STARTUP_US           (2000)
//#define BOARD_MCK                      (123000000)
//#define TRACE_LEVEL                    (0)
//#define TRACE_LEVEL                    (0)

/******************************************************
 *  Wi-Fi Options
 ******************************************************/

/*  GPIO pins are used to bootstrap Wi-Fi to SDIO or gSPI mode */
#define WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP

/*  Wi-Fi GPIO0 pin is used for out-of-band interrupt */
#define WICED_WIFI_OOB_IRQ_GPIO_PIN  ( 0 )

/*  Wi-Fi power pin is present */
#define WICED_USE_WIFI_POWER_PIN

/*  Wi-Fi reset pin is present */
#define WICED_USE_WIFI_RESET_PIN



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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

