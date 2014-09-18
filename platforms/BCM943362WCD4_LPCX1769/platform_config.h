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
 * Defines internal configuration of the BCM943362WCD4_LPCX1769 platform
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************
 *  MCU Constants and Options
 ******************************************************/

/*  CPU clock : 120 MHz */
#define CPU_CLOCK_HZ         ( 120000000 )

/*  32K crystal frequency */
//#define CRYSTAL_32K_FREQ_IN  ( 32768 )

/*  PLLM : 20 */
#define PLL_M_CONSTANT       ( 20 )

/*  PLLN : 1 */
#define PLL_N_CONSTANT       ( 1 )


/******************************************************
 *  Wi-Fi Options
 ******************************************************/

/*  Wi-Fi SPI rate in Hz */
#define SPI_BITRATE  ( 40000000 )

/*  GPIO pins are used to bootstrap Wi-Fi to SDIO or gSPI mode */
#define WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP

/*  Wi-Fi power pin is present */
#define WICED_USE_WIFI_POWER_PIN

/*  Wi-Fi reset pin is present */
#define WICED_USE_WIFI_RESET_PIN

#ifdef __cplusplus
} /* extern "C" */
#endif
