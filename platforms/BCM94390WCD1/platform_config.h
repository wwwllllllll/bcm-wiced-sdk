/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/*
 * Defines internal configuration of the BCM9490WCD1 board
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *  MCU Constants and Options
 ******************************************************/

/*  CPU clock : 48MHz */
#define CPU_CLOCK_HZ  ( 48000000 )

/*  WICED Resources uses a filesystem */
#define USES_RESOURCE_FILESYSTEM

/*  DCT is stored in external flash */
#define EXTERNAL_DCT

/*  OTP */
#define PLATFORM_HAS_OTP

/*  WARNING: MCU powersave isn't working yet for BCM439x */
#ifndef WICED_DISABLE_MCU_POWERSAVE
#define WICED_DISABLE_MCU_POWERSAVE
#endif

#ifdef __cplusplus
} /*extern "C" */
#endif
