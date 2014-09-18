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

/*---------------------------------------------------------------------------*/
/* from "Platform/common/ARM_Cortex_M3/crt0_gcc.c" */
#ifndef SCB_AIRCR_ADDRESS
    #define SCB_AIRCR_ADDRESS        ( 0xE000ED0C )
#endif /* SCB_AIRCR_ADDRESS */
#ifndef SCB_AIRCR_VECTKEY
    #define SCB_AIRCR_VECTKEY        ( 0x5FA << 16 )
#endif /* SCB_AIRCR_VECTKEY */
#ifndef SCB_AIRCR
    #define SCB_AIRCR                ( ( volatile unsigned long* ) SCB_AIRCR_ADDRESS )
#endif /* SCB_AIRCR */
#ifndef SCB_AIRCR_SYSRESETREQ
    #define SCB_AIRCR_SYSRESETREQ    ( 0x1 << 2 )
#endif /* SCB_AIRCR_SYSRESETREQ */
/*---------------------------------------------------------------------------*/

static wiced_bool_t mcu_powersave_enabled        = WICED_FALSE;
static wiced_bool_t mcu_powersave_current_status = WICED_FALSE;

int reboot( int argc, char *argv[] )
{
    WPRINT_APP_INFO( ( "Rebooting...\r\n" ) );
    host_rtos_delay_milliseconds( 1000 );

    /* Reboot the ARM Cortex M3 */
    *SCB_AIRCR = SCB_AIRCR_SYSRESETREQ | SCB_AIRCR_VECTKEY;

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
