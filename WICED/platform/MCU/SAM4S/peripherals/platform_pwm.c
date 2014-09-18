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
 *
 */
#include <stdint.h>
#include <string.h>
#include "platform_cmsis.h"
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_isr_interface.h"
#include "wwd_assert.h"

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
 *            WICED Function Definitions
 ******************************************************/
platform_result_t platform_pwm_init( const platform_pwm_t* pwm, uint32_t frequency, float duty_cycle )
{
    UNUSED_PARAMETER( pwm );
    UNUSED_PARAMETER( frequency );
    UNUSED_PARAMETER( duty_cycle );
    return PLATFORM_UNSUPPORTED;
}

platform_result_t platform_pwm_start( const platform_pwm_t* pwm )
{
    UNUSED_PARAMETER( pwm );
    return PLATFORM_UNSUPPORTED;
}

platform_result_t platform_pwm_stop( const platform_pwm_t* pwm )
{
    UNUSED_PARAMETER( pwm );
    return PLATFORM_UNSUPPORTED;
}




