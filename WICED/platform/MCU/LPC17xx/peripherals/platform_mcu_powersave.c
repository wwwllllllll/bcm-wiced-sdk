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
 *  MCU powersave implementation
 */
#include <stdint.h>
#include <string.h>
#include "platform_cmsis.h"
#include "core_cmFunc.h"
#include "platform_init.h"
#include "platform_constants.h"
#include "platform_assert.h"
#include "platform_peripheral.h"
#include "platform_isr_interface.h"
#include "platform_sleep.h"
#include "wwd_rtos.h"
#include "wiced_defaults.h"

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

platform_result_t platform_mcu_powersave_init( void )
{
    return PLATFORM_FEATURE_DISABLED;
}

platform_result_t platform_mcu_powersave_disable( void )
{
    return PLATFORM_FEATURE_DISABLED;
}

platform_result_t platform_mcu_powersave_enable( void )
{
    return PLATFORM_FEATURE_DISABLED;
}

void platform_mcu_powersave_exit_notify( void )
{
    return;
}

/******************************************************
 *               RTOS Powersave Hooks
 ******************************************************/

void platform_idle_hook( void )
{
    return;
}

uint32_t platform_power_down_hook( uint32_t sleep_ms )
{
    UNUSED_PARAMETER( sleep_ms );
    return 0;
}
