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
 * Ping Powersave Application
 *
 * This application snippet demonstrates how to minimise power
 * consumption for a connected device that does NOT require high
 * throughput. Please read the WICED Powersave Application Note
 * to familiarise yourself with the WICED powersave implementation
 * before using this application.
 *
 * Features demonstrated
 *  - Wi-Fi client mode
 *  - ICMP ping
 *  - MCU powersave
 *  - Wi-Fi powersave
 *
 * Application Instructions
 *   1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *      in the wifi_config_dct.h header file to match your Wi-Fi access point
 *   2. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *
 *  After initialisation, the application loops through the following
 *  sequence forever:
 *   - sends an ICMP ping to the gateway
 *   - suspends network timers
 *   - enters low power mode for WIFI_SLEEP_TIME seconds
 *   - resumes network timers
 *
 * NOTES :
 *   To use a Wi-Fi powersave mode that enables higher throughput,
 *   comment the USE_POWERSAVE_POLL directive
 *
 */

#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define WIFI_SLEEP_TIME      (1000 * MILLISECONDS)
#define USE_POWERSAVE_POLL

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

static wiced_result_t send_ping( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start(void)
{
    /* Initialise the WICED device */
    wiced_init();

    /* Configure the device */
    /* wiced_configure_device( app_config ); */ /* Config bypassed in local makefile and wifi_config_dct.h */

    /* Bring up the network interface and connect to the Wi-Fi network */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    /* Enable MCU powersave */
    /* WARNING: Please read the WICED Powersave API documentation before enabling MCU powersave */
    /* wiced_platform_mcu_enable_powersave(); */

    /* Enable Wi-Fi powersave */
#ifdef USE_POWERSAVE_POLL
    wiced_wifi_enable_powersave();
#else
    const uint8_t return_to_sleep_delay = 10;
    wiced_wifi_enable_powersave_with_throughput( return_to_sleep_delay );
#endif

    while (1)
    {
        /* Send an ICMP ping to the gateway */
        send_ping( );

        /* Suspend network timers */
        wiced_network_suspend();

        /* Sleep for a while */
        wiced_rtos_delay_milliseconds( WIFI_SLEEP_TIME );

        /* Resume network timers */
        wiced_network_resume();
    }
}


static wiced_result_t send_ping( void )
{
    const uint32_t     ping_timeout = 1000;
    uint32_t           elapsed_ms;
    wiced_result_t     status;
    wiced_ip_address_t ping_target_ip;

    wiced_ip_get_gateway_address( WICED_STA_INTERFACE, &ping_target_ip );
    status = wiced_ping( WICED_STA_INTERFACE, &ping_target_ip, ping_timeout, &elapsed_ms );

    if ( status == WICED_SUCCESS )
    {
        WPRINT_APP_INFO(( "Ping Reply %lums\n", elapsed_ms ));
    }
    else if ( status == WICED_TIMEOUT )
    {
        WPRINT_APP_INFO(( "Ping timeout\n" ));
    }
    else
    {
        WPRINT_APP_INFO(( "Ping error\n" ));
    }

    return WICED_SUCCESS;
}
