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
 * Concurrent APSTA Application
 *
 * This application snippet demonstrates how to use
 * the WICED Wi-Fi client and softAP interfaces at the same time.
 *
 * Features demonstrated
 *  - Wi-Fi client mode (to send a regular ICMP ping to an AP)
 *  - Wi-Fi softAP mode (to enable Wi-Fi clients to connect to the WICED webserver)
 *
 * To demonstrate the app, work through the following steps.
 *  1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *     in the wifi_config_dct.h header file to match your Wi-Fi access point
 *  2. Modify the SOFT_AP_SSID/SOFT_AP_PASSPHRASE Wi-Fi credentials
 *     as desired
 *  3. Plug the WICED eval board into your computer
 *  4. Open a terminal application and connect to the WICED eval board
 *  5. Build and download the application (to the WICED board)
 *
 * After the download completes, the terminal displays WICED startup
 * information and then :
 *  - Joins a Wi-Fi network and pings the gateway. Ping results are
 *    printed to the UART and appear on the terminal
 *  - Starts a softAP and a webserver on the AP interface
 *
 * To connect a Wi-Fi client (eg. computer) to the softAP webserver:
 *  - Connect your computer to the softAP SSID configured in wifi_config_dct.h
 *  - Open a web browser
 *  - Enter wiced.com as the URL; a simple web page appears
 *    (or alternately, enter 192.168.0.1 as the URL, this is the
 *     IP address of the softAP interface)
 *
 * TROUBLESHOOTING
 *   If you are having difficulty connecting the web browser to the
 *   WICED softAP webserver, try the following:
 *   1. Disconnect other network interfaces from the computer (eg. wired ethernet)
 *   2. Check that your computer received a valid IP address eg. 192.168.0.2
 *   3. Try clearing the web browser cache and try connecting again
 *
 */

#include "wiced.h"
#include "http_server.h"
#include "resources.h"
#include "dns_redirect.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define PING_PERIOD  1000
#define PING_TIMEOUT  900

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

static wiced_result_t send_ping ( void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const wiced_ip_setting_t ap_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
};

static wiced_http_server_t ap_http_server;

START_OF_HTTP_PAGE_DATABASE(ap_web_pages)
    ROOT_HTTP_PAGE_REDIRECT("/apps/apsta/ap_top.html"),
    { "/apps/apsta/ap_top.html",         "text/html",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_apps_DIR_apsta_DIR_ap_top_html,    },
    { "/images/favicon.ico",             "image/vnd.microsoft.icon",          WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_favicon_ico,            },
    { "/images/brcmlogo.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_brcmlogo_png,           },
    { "/images/brcmlogo_line.png",       "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_brcmlogo_line_png,      },
END_OF_HTTP_PAGE_DATABASE();

static dns_redirector_t    dns_redirector;
static wiced_timed_event_t ping_timed_event;
static wiced_ip_address_t  ping_target_ip;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start(void)
{
    /* Initialise the device */
    wiced_init();

    /* Bring up the STA (client) interface ------------------------------------------------------- */
    wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);

    /* The ping target is the gateway that the STA is connected to*/
    wiced_ip_get_gateway_address( WICED_STA_INTERFACE, &ping_target_ip );

    /* Print ping description to the UART */
    WPRINT_APP_INFO(("Pinging %u.%u.%u.%u every %ums with a %ums timeout.\n", (unsigned int)((GET_IPV4_ADDRESS(ping_target_ip) >> 24) & 0xFF),
                                                                              (unsigned int)((GET_IPV4_ADDRESS(ping_target_ip) >> 16) & 0xFF),
                                                                              (unsigned int)((GET_IPV4_ADDRESS(ping_target_ip) >>  8) & 0xFF),
                                                                              (unsigned int)((GET_IPV4_ADDRESS(ping_target_ip) >>  0) & 0xFF),
                                                                              PING_PERIOD,
                                                                              PING_TIMEOUT) );

    /* Setup a regular ping event and setup the callback to run in the networking worker thread */
    wiced_rtos_register_timed_event( &ping_timed_event, WICED_NETWORKING_WORKER_THREAD, &send_ping, PING_PERIOD, 0 );

    /* Bring up the softAP interface ------------------------------------------------------------- */
    wiced_network_up(WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &ap_ip_settings);

    /* Start a DNS redirect server to redirect wiced.com to the AP webserver database*/
    wiced_dns_redirector_start( &dns_redirector, WICED_AP_INTERFACE );

    /* Start a web server on the AP interface */
    wiced_http_server_start( &ap_http_server, 80, ap_web_pages, WICED_AP_INTERFACE );
}


/* Sends a ping to the target */
static wiced_result_t send_ping( void *arg )
{
    uint32_t elapsed_ms;
    wiced_result_t status;

    status = wiced_ping( WICED_STA_INTERFACE, &ping_target_ip, PING_TIMEOUT, &elapsed_ms );

    if ( status == WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Ping Reply : %lu ms\n", (unsigned long)elapsed_ms ));
    }
    else if ( status == WICED_TIMEOUT )
    {
        WPRINT_APP_INFO(("Ping timeout\n"));
    }
    else
    {
        WPRINT_APP_INFO(("Ping error\n"));
    }

    return WICED_SUCCESS;
}

