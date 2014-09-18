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

#include <stdlib.h>
#include <string.h>
#include "wiced.h"
#include "http_server.h"
#include "dns_redirect.h"
#include <wiced_utilities.h>
#include "wiced_network.h"
#include <resources.h>
#include "wiced_framework.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/


static const wiced_wps_device_detail_t wps_details =
{
    .device_name     = PLATFORM,
    .manufacturer    = "Broadcom",
    .model_name      = PLATFORM,
    .model_number    = "2.0",
    .serial_number   = "1408248",
    .device_category = WICED_WPS_DEVICE_COMPUTER,
    .sub_category    = 7,
    .config_methods  = WPS_CONFIG_LABEL | WPS_CONFIG_VIRTUAL_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_DISPLAY_PIN
};

#define HTTPS_PORT   443
#define HTTP_PORT    80

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

static const wiced_ip_setting_t device_init_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
};

extern const wiced_http_page_t config_http_page_database[];
extern const wiced_http_page_t config_https_page_database[];

/* These are accessed by config_http_content.c */
const configuration_entry_t* app_configuration = NULL;

wiced_http_server_t*         http_server;

wiced_bool_t                 config_use_wps;
char                         config_wps_pin[9];

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_configure_device(const configuration_entry_t* config)
{
    wiced_bool_t* device_configured;

    if ( WICED_SUCCESS != wiced_dct_read_lock( (void**) &device_configured, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, OFFSETOF(platform_dct_wifi_config_t, device_configured), sizeof(wiced_bool_t) ) )
    {
        return WICED_ERROR;
    }


    if ( *device_configured != WICED_TRUE )
    {
        dns_redirector_t dns_redirector;

        wiced_init( );

        /* Configure variables */
        app_configuration = config;
        config_use_wps = WICED_FALSE;

        /* Prepare the HTTP server */
        http_server = MALLOC_OBJECT("http server", wiced_http_server_t);
        memset( http_server, 0, sizeof(wiced_http_server_t) );

        /* Start the AP */
        wiced_network_up( WICED_CONFIG_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &device_init_ip_settings );

        /* Start the DNS redirect server */
        wiced_dns_redirector_start( &dns_redirector, WICED_CONFIG_INTERFACE );

        /* Start the HTTP server */
        wiced_http_server_start( http_server, HTTP_PORT, config_http_page_database, WICED_CONFIG_INTERFACE );

        /* Wait for configuration to complete */
        wiced_rtos_thread_join( &http_server->thread );

        /* Cleanup HTTP server */
        wiced_http_server_stop(http_server);
        free( http_server );


        /* Cleanup DNS server */
        wiced_dns_redirector_stop(&dns_redirector);

        /* Turn off AP */
        wiced_network_down( WICED_CONFIG_INTERFACE );

        /* Check if WPS was selected */
        if (config_use_wps == WICED_TRUE)
        {
            wiced_result_t ret;
            wiced_wps_credential_t* wps_credentials = MALLOC_OBJECT("wps",wiced_wps_credential_t);

            if (config_wps_pin[0] == '\x00')
            {
                ret = wiced_wps_enrollee(WICED_WPS_PBC_MODE, &wps_details, "00000000", wps_credentials, 1);
            }
            else
            {
                ret = wiced_wps_enrollee(WICED_WPS_PIN_MODE, &wps_details, config_wps_pin, wps_credentials, 1);
            }

            if (ret == WICED_SUCCESS)
            {
                /* Write received credentials into DCT */
                struct
                {
                    wiced_bool_t             device_configured;
                    wiced_config_ap_entry_t  ap_entry;
                } temp_config;
                memset(&temp_config, 0, sizeof(temp_config));
                memcpy(&temp_config.ap_entry.details.SSID,     &wps_credentials->ssid, sizeof(wiced_ssid_t));
                memcpy(&temp_config.ap_entry.details.security, &wps_credentials->security, sizeof(wiced_security_t));
                memcpy(temp_config.ap_entry.security_key,       wps_credentials->passphrase, wps_credentials->passphrase_length);
                temp_config.ap_entry.security_key_length = wps_credentials->passphrase_length;
                temp_config.device_configured = WICED_TRUE;
                wiced_dct_write( &temp_config, DCT_WIFI_CONFIG_SECTION, 0, sizeof(temp_config) );
            }
            else
            {
                /* TODO: WPS failed.. Do something */
            }

            free(wps_credentials);
        }

        app_configuration = NULL;
    }
    wiced_dct_read_unlock( device_configured, WICED_FALSE );

    return WICED_SUCCESS;
}

