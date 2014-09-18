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

int reboot( int argc, char *argv[] )
{
    WPRINT_APP_INFO( ( "Rebooting...\n" ) );
    host_rtos_delay_milliseconds( 1000 );

    /* Reboot the ARM Cortex M3 */
    *SCB_AIRCR = SCB_AIRCR_SYSRESETREQ | SCB_AIRCR_VECTKEY;

    /* Never reached */
    return 0;
}
