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
 * Powercycle Application
 *
 * This application snippet demonstrates how to minimise power
 * consumption for a connected device. The app initialises the
 * WICED device, connects to and pings the network, then
 * powers off the Wi-Fi device for a fixed time period. This
 * process repeats forever. Please read the WICED Powersave
 * Application Note to familiarise yourself with the WICED
 * powersave implementation before using this application.
 *
 * Features demonstrated
 *  - Wi-Fi client mode
 *  - ICMP ping
 *  - MCU powersave
 *  - Wi-Fi powersave
 *  - Wi-Fi poweroff
 *
 * Application Instructions
 *   1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *      in the wifi_config_dct.h header file to match your Wi-Fi access point
 *   2. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *
 * The application loops through the following sequence forever:
 *      - displays WICED startup information on the terminal
 *      - joins the AP specified in wifi_config_dct.h
 *      - turns on MCU & Wi-Fi powersave
 *      - sends an ICMP ping to the gateway, and
 *      - enters low power mode for WIFI_SLEEP_TIME seconds
 *
 * NOTES :
 *  1. The Wi-Fi device can only be powered down if the WICED hardware
 *     platform has the ability to cut power to the Wi-Fi chip.
 *  2. Wi-Fi powerdown does *NOT* currently work for FreeRTOS/LwIP
 *     since LwIP does not provide a clean way to terminate the
 *     network thread
 *
 */

#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define WIFI_SLEEP_TIME     (5 * SECONDS)

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
    while (1)
    {
        /* Initialise the WICED device */
        wiced_init();

        /* Configure the device */
        /* wiced_configure_device( app_config ); */  /* Config bypassed in local makefile and wifi_config_dct.h */

        /* Bring up the network interface and connect to the Wi-Fi network */
        wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

        /* Enable MCU powersave */
        /* WARNING: Please read the WICED Powersave API documentation before enabling MCU powersave */
        /* wiced_platform_mcu_enable_powersave(); */

        /* Enable Wi-Fi powersave */
        wiced_wifi_enable_powersave();

        /* Send an ICMP ping to the gateway */
        send_ping( );

        /* De-initialise the Wi-Fi interface which powers down the Wi-Fi device */
        WPRINT_APP_INFO(( "Wi-Fi Powerdown\n\n" ));
        wiced_deinit( );

        /* Sleep for a while */
        wiced_rtos_delay_milliseconds( WIFI_SLEEP_TIME );
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
