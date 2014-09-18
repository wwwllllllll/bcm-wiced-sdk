/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "wwd_debug.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_platform.h"
#include "console.h"
#include "wiced_framework.h"

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

static wiced_bool_t mcu_powersave_enabled        = WICED_FALSE;
static wiced_bool_t mcu_powersave_current_status = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/

int reboot( int argc, char *argv[] )
{
    WPRINT_APP_INFO( ( "Rebooting...\r\n" ) );
    host_rtos_delay_milliseconds( 1000 );

    shared_waf_api->platform_reboot();

    /* Never reached */
    return 0;
}

int mcu_powersave( int argc, char *argv[] )
{
    int a = atoi( argv[ 1 ] );

    mcu_powersave_enabled = ( a == 0 ) ? WICED_FALSE : WICED_TRUE;

    return ERR_CMD_OK;
}

int platform_enable_mcu_powersave( void )
{
    if ( mcu_powersave_enabled == WICED_TRUE )
    {
        wiced_platform_mcu_enable_powersave();
        mcu_powersave_current_status = WICED_TRUE;
    }

    return ERR_CMD_OK;
}

int platform_disable_mcu_powersave( void )
{
    if ( mcu_powersave_current_status == WICED_TRUE )
    {
        wiced_platform_mcu_disable_powersave();
        mcu_powersave_current_status = WICED_FALSE;
    }

    return ERR_CMD_OK;
}
