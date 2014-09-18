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
 *  Implements initialisation and other management functions for WWD system
 *
 */

#include <string.h>
#include "wwd_management.h"
#include "wwd_events.h"
#include "wwd_wlioctl.h"
#include "wwd_assert.h"
#include "wwd_wifi.h"
#include "platform/wwd_bus_interface.h"
#include "platform/wwd_platform_interface.h"
#include "network/wwd_network_interface.h"
#include "network/wwd_buffer_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "internal/wwd_sdpcm.h"
#include "internal/wwd_internal.h"
#include "internal/wwd_thread.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"


/******************************************************
 *             Constants
 ******************************************************/

#define MAX_POST_SET_COUNTRY_RETRY  3

/******************************************************
 *             Static Variables
 ******************************************************/

/******************************************************
 *             Function definitions
 ******************************************************/

/**
 * Initialises WWD System
 *
 * - Initialises the required parts of the hardware platform
 *   i.e. pins for SDIO/SPI, interrupt, reset, power etc.
 *
 * - Initialises the WWD thread which arbitrates access
 *   to the SDIO/SPI bus
 * - Sets the country code to set which radio channels are
 *   allowed
 * - Clear the events mask
 * - Bring the Wireless interface "Up"
 *
 * @param country : enumerated country identifier, which sets the allowed
 *                  radio channels according to country regulations
 * @param buffer_interface_arg : abstract parameter which is passed
 *                        to the buffer interface during init.
 *                        Look in @host_buffer_init for details
 *                        of the argument required for your particular buffering system
 *
 * @return WWD_SUCCESS if initialization is successful, error code otherwise
 */

wwd_result_t wwd_management_init( wiced_country_code_t country, /*@null@*/ void* buffer_interface_arg )
{
    wwd_result_t retval;

    retval = host_buffer_init( buffer_interface_arg );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not initialize buffer interface\n"));
        return retval;
    }

    /* Remember the country code */
    wwd_wlan_status.country_code = country;

    return wwd_management_wifi_on( );
}

/**
 * Turn on the Wi-Fi device
 *
 * - Initialises the required parts of the hardware platform
 *   i.e. pins for SDIO/SPI, interrupt, reset, power etc.
 *
 * - Bring the Wireless interface "Up"
 * - Initialises the WWD thread which arbitrates access
 *   to the SDIO/SPI bus
 *
 * @return WWD_SUCCESS if initialization is successful, error code otherwise
 */
wwd_result_t wwd_management_wifi_on( void )
{
    wl_country_t*  country_struct;
    uint32_t*      ptr;
    wwd_result_t   retval;
    wiced_buffer_t buffer;
    uint8_t*       event_mask;
    uint32_t*      data;
    uint32_t       counter;
#ifdef WICED_DISABLE_AMPDU_TX
    uint8_t        i;
#endif
#ifdef MAC_ADDRESS_SET_BY_HOST
    wiced_mac_t mac_address;
#endif

    if ( wwd_wlan_status.state == WLAN_UP )
    {
        return WWD_SUCCESS;
    }

    retval = host_platform_init( );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not initialize platform interface\n"));
        return retval;
    }

    retval = host_platform_bus_init( );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not initialize platform bus interface\n"));
        return retval;
    }

    retval = wwd_bus_init( );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not initialize bus\n"));
        return retval;
    }

    retval = wwd_thread_init( );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not initialize WWD thread\n"));
        return retval;
    }

    /* Enable 32K WLAN sleep clock */
    host_platform_init_wlan_powersave_clock();

    host_platform_bus_enable_interrupt( );

#ifdef MAC_ADDRESS_SET_BY_HOST
    /* See <WICED-SDK>/generated_mac_address.txt for info about setting the MAC address  */
    host_platform_get_mac_address(&mac_address);
    wwd_wifi_set_mac_address(mac_address);
#endif

    /* Turn off SDPCM TX Glomming */
    /* Note: This is only required for later chips.
     * The 4319 has glomming off by default however the 43362 has it on by default.
     */
    data = wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_TX_GLOM );
    if ( data == NULL )
    {
        wiced_assert( "Could not get buffer for IOVAR", 0 != 0 );
        /*@-unreachable@*/ /* Lint: Reachable after hitting assert */
        return WWD_BUFFER_ALLOC_FAIL;
        /*@-unreachable@*/
    }
    *data = 0;
    retval = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    wiced_assert("Could not turn off TX glomming\n", (retval == WWD_SUCCESS) || (retval == WWD_UNSUPPORTED) );

    /* Turn APSTA on */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 4, IOVAR_STR_APSTA );
    if ( data == NULL )
    {
        wiced_assert( "Could not get buffer for IOVAR", 0 != 0 );
        return WWD_BUFFER_ALLOC_FAIL;
    }
    *data = (uint32_t) 1;
    /* This will fail on manufacturing test build since it does not have APSTA available */
    retval = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    if ( ( retval != WWD_SUCCESS ) && ( retval != WWD_UNSUPPORTED ) )
    {
        /* Could not turn on APSTA */
        WPRINT_WWD_DEBUG( ("Could not turn on APSTA\n") );
    }

#ifdef WICED_DISABLE_AMPDU_TX
    /* Disable AMPDU for TX */
    for ( i = 0; i < 8; ++i )
    {
        struct ampdu_tid_control* ampdu_tid;
        ampdu_tid = (struct ampdu_tid_control*) wiced_get_iovar_buffer( &buffer, (uint16_t) sizeof(struct ampdu_tid_control), IOVAR_STR_AMPDU_TID );
        if ( ampdu_tid == NULL )
        {
            wiced_assert( "Could not get buffer for IOVAR", 0 != 0 );
            return WICED_ERROR;
        }
        ampdu_tid->tid    = i;
        ampdu_tid->enable = 0;
        retval = wiced_send_iovar( SDPCM_SET, buffer, 0, SDPCM_STA_INTERFACE );
        if ( retval != WICED_SUCCESS )
        {
            WPRINT_WWD_ERROR( ("Could not disable ampdu for tid: %u\n", i) );
        }
    }
#endif

    retval = wwd_wifi_set_ampdu_parameters();
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not set AMPDU parameters\r\n"));
        return retval;
    }

#ifdef WICED_STARTUP_DELAY
    (void) host_rtos_delay_milliseconds( (uint32_t) WWD_STARTUP_DELAY );
#endif

    /* Send set country command */
    /* This is the first time that the WLAN chip is required to respond
     * in it's normal run mode.
     * If you are porting a new system and it stalls here, it could
     * be one of the following problems:
     *   - Bus interrupt not triggering properly - the WLAN chip is unable to signal the host that there is data available.
     *   - Timing problems - if the timeouts on semaphores are not working correctly, then the
     *                       system might think that the IOCTL has timed out much faster than it should do.
     *
     */
    country_struct = (wl_country_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) sizeof(wl_country_t), IOVAR_STR_COUNTRY );
    if ( country_struct == NULL )
    {
        wiced_assert( "Could not get buffer for IOCTL", 0 != 0 );
        return WWD_BUFFER_ALLOC_FAIL;
    }
    memset(country_struct, 0, sizeof(wl_country_t));

    ptr  = (uint32_t*)country_struct->ccode;
    *ptr = (uint32_t) wwd_wlan_status.country_code & 0x00ffffff;
    ptr  = (uint32_t*)country_struct->country_abbrev;
    *ptr = (uint32_t) wwd_wlan_status.country_code & 0x00ffffff;
    country_struct->rev = (int32_t) ( ( wwd_wlan_status.country_code & 0xff000000 ) >> 24 );

    retval = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    if ( retval != WWD_SUCCESS )
    {
        /* Could not set wifi country */
        WPRINT_WWD_ERROR(("Could not set Country code\n"));
        return retval;
    }

    /* NOTE: The set country command requires time to process on the WLAN firmware and the following IOCTL may fail on initial attempts therefore we try a few times */

    /* Set the event mask, indicating initially we do not want any asynchronous events */
    for ( counter = 0, retval = WWD_PENDING; retval != WWD_SUCCESS && counter < (uint32_t)MAX_POST_SET_COUNTRY_RETRY; ++counter )
    {
        event_mask = (uint8_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) WL_EVENTING_MASK_LEN, IOVAR_STR_EVENT_MSGS );
        if ( event_mask == NULL )
        {
            wiced_assert( "Could not get buffer for IOVAR", 0 != 0 );
            return WWD_BUFFER_ALLOC_FAIL;
        }
        memset( event_mask, 0, (size_t) WL_EVENTING_MASK_LEN );
        retval = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    }
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not set Event mask\n"));
        return retval;
    }

    /* Send UP command */
    data = wwd_sdpcm_get_ioctl_buffer( &buffer, 0 );
    if ( data == NULL )
    {
        wiced_assert( "Could not get buffer for IOCTL", 0 != 0 );
        return WWD_BUFFER_ALLOC_FAIL;
    }
    retval = wwd_sdpcm_send_ioctl( SDPCM_SET, (uint32_t) WLC_UP, buffer, 0, WWD_STA_INTERFACE );
    if ( retval != WWD_SUCCESS )
    {
        /* Note: System may time out here if bus interrupts are not working properly */
        WPRINT_WWD_ERROR(("Error enabling 802.11\n"));
        return retval;
    }

    /* Set the GMode */
    data = wwd_sdpcm_get_ioctl_buffer( &buffer, (uint16_t) 4 );
    if ( data == NULL )
    {
        wiced_assert( "Could not get buffer for IOCTL", 0 != 0 );
        return WWD_BUFFER_ALLOC_FAIL;
    }
    *data = (uint32_t) GMODE_AUTO;
    retval = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_SET_GMODE, buffer, 0, WWD_STA_INTERFACE );
    wiced_assert("Failed to set GMode\n", retval == WWD_SUCCESS );

    wwd_wlan_status.state = WLAN_UP;

    return WWD_SUCCESS;
}

/**
 * Turn off the Wi-Fi device
 *
 * - De-Initialises the required parts of the hardware platform
 *   i.e. pins for SDIO/SPI, interrupt, reset, power etc.
 *
 * - Bring the Wireless interface "Down"
 * - De-Initialises the WWD thread which arbitrates access
 *   to the SDIO/SPI bus
 *
 * @return WWD_SUCCESS if deinitialization is successful, error code otherwise
 */
wwd_result_t wwd_management_wifi_off( void )
{
    wwd_result_t retval;

    if ( wwd_wlan_status.state == WLAN_DOWN )
    {
        return WWD_SUCCESS;
    }

    /* Send DOWN command */
    (void) wwd_wifi_set_down( WWD_STA_INTERFACE ); /* Ignore return - ensuring wifi will be brought down even if bus is completely broken */

    host_platform_bus_disable_interrupt( );

    wwd_thread_quit( );

    retval = wwd_bus_deinit( );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_DEBUG(("Error de-initializing bus\n"));
        return retval;
    }

    retval = host_platform_bus_deinit( );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_DEBUG(("Error de-initializing platform bus interface\n"));
        return retval;
    }

    retval = host_platform_deinit( );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_DEBUG(("Error de-initializing platform interface\n"));
        return retval;
    }

    /* Disable 32K WLAN sleep clock */
    host_platform_deinit_wlan_powersave_clock();

    wwd_wlan_status.state = WLAN_DOWN;
    return WWD_SUCCESS;
}

void wwd_set_country( wiced_country_code_t code )
{
    wwd_wlan_status.country_code = code;
}
