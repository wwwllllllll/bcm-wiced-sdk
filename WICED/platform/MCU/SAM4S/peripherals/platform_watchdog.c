/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "platform_cmsis.h"
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_stdio.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "platform_assert.h"
#include "wwd_assert.h"
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
 *             WICED Function Definitions
 ******************************************************/

/******************************************************
 *             SAM4S Function Definitions
 ******************************************************/

platform_result_t platform_watchdog_init( void )
{
#ifndef WICED_DISABLE_WATCHDOG
    /* WARNING: SAM4S watchdog is NOT implemented yet. Watchdog is disabled at start-up */
    wdt_disable( WDT );
#endif /* WICED_DISABLE_WATCHDOG */
    return PLATFORM_SUCCESS;
}

platform_result_t platform_watchdog_kick( void )
{
    /* WARNING: SAM4S watchdog is NOT implemented yet */
    return PLATFORM_UNSUPPORTED;
}

wiced_bool_t platform_watchdog_check_last_reset( void )
{
    /* WARNING: SAM4S watchdog is NOT implemented yet */
    return WICED_FALSE;
}
