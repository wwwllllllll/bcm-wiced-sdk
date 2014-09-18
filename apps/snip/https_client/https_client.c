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
 * HTTPS Client Application
 *
 * This application snippet demonstrates how to use the WICED
 * DNS (Domain Name Service) & HTTPS API
 *
 * Features demonstrated
 *  - Wi-Fi client mode
 *  - DNS lookup
 *  - Secure HTTPS client connection
 *
 * Application Instructions
 * 1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *    in the wifi_config_dct.h header file to match your Wi-Fi access point
 * 2. Connect a PC terminal to the serial port of the WICED Eval board,
 *    then build and download the application as described in the WICED
 *    Quick Start Guide
 *
 * After the download completes, the application :
 *  - Connects to the Wi-Fi network specified
 *  - Resolves the http://google.com IP address using a DNS lookup
 *  - Downloads https://google.com using a secure HTTPS connection
 *  - Prints the downloaded HTML to the UART
 *
 */

#include <stdlib.h>
#include "wiced.h"
#include "http.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define BUFFER_LENGTH     (2048)

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

static const char google_root_ca_certificate[] =
    "-----BEGIN CERTIFICATE-----\n"\
    "MIIDIDCCAomgAwIBAgIENd70zzANBgkqhkiG9w0BAQUFADBOMQswCQYDVQQGEwJV\n"\
    "UzEQMA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2Vy\n"\
    "dGlmaWNhdGUgQXV0aG9yaXR5MB4XDTk4MDgyMjE2NDE1MVoXDTE4MDgyMjE2NDE1\n"\
    "MVowTjELMAkGA1UEBhMCVVMxEDAOBgNVBAoTB0VxdWlmYXgxLTArBgNVBAsTJEVx\n"\
    "dWlmYXggU2VjdXJlIENlcnRpZmljYXRlIEF1dGhvcml0eTCBnzANBgkqhkiG9w0B\n"\
    "AQEFAAOBjQAwgYkCgYEAwV2xWGcIYu6gmi0fCG2RFGiYCh7+2gRvE4RiIcPRfM6f\n"\
    "BeC4AfBONOziipUEZKzxa1NfBbPLZ4C/QgKO/t0BCezhABRP/PvwDN1Dulsr4R+A\n"\
    "cJkVV5MW8Q+XarfCaCMczE1ZMKxRHjuvK9buY0V7xdlfUNLjUA86iOe/FP3gx7kC\n"\
    "AwEAAaOCAQkwggEFMHAGA1UdHwRpMGcwZaBjoGGkXzBdMQswCQYDVQQGEwJVUzEQ\n"\
    "MA4GA1UEChMHRXF1aWZheDEtMCsGA1UECxMkRXF1aWZheCBTZWN1cmUgQ2VydGlm\n"\
    "aWNhdGUgQXV0aG9yaXR5MQ0wCwYDVQQDEwRDUkwxMBoGA1UdEAQTMBGBDzIwMTgw\n"\
    "ODIyMTY0MTUxWjALBgNVHQ8EBAMCAQYwHwYDVR0jBBgwFoAUSOZo+SvSspXXR9gj\n"\
    "IBBPM5iQn9QwHQYDVR0OBBYEFEjmaPkr0rKV10fYIyAQTzOYkJ/UMAwGA1UdEwQF\n"\
    "MAMBAf8wGgYJKoZIhvZ9B0EABA0wCxsFVjMuMGMDAgbAMA0GCSqGSIb3DQEBBQUA\n"\
    "A4GBAFjOKer89961zgK5F7WF0bnj4JXMJTENAKaSbn+2kmOeUJXRmm/kEd5jhW6Y\n"\
    "7qj/WsjTVbJmcVfewCHrPSqnI0kBBIZCe/zuf6IWUrVnZ9NA2zsmWLIodz2uFHdh\n"\
    "1voqZiegDfqnc1zqcPGUIWVEX/r87yloqaKHee9570+sB3c4\n"\
    "-----END CERTIFICATE-----\n";

uint8_t buffer[BUFFER_LENGTH];

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    wiced_ip_address_t ip_address;
    wiced_result_t     result;

    wiced_init( );
    wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);

    /* Configure the device */
    /* wiced_configure_device( app_config ); */ /* Config bypassed in local makefile and wifi_config_dct.h */

    WPRINT_APP_INFO( ( "Resolving IP address of HTTPS server\n" ) );
    wiced_hostname_lookup("www.google.com", &ip_address, 10000);
    WPRINT_APP_INFO( ( "Server is at %u.%u.%u.%u\n",   (uint8_t)(GET_IPV4_ADDRESS(ip_address) >> 24),
                                                       (uint8_t)(GET_IPV4_ADDRESS(ip_address) >> 16),
                                                       (uint8_t)(GET_IPV4_ADDRESS(ip_address) >> 8),
                                                       (uint8_t)(GET_IPV4_ADDRESS(ip_address) >> 0) ) );

    WPRINT_APP_INFO( ( "Getting '/'...\n" ) );

    /* Initialize the root CA certificate */
    wiced_tls_init_root_ca_certificates( google_root_ca_certificate );

    result = wiced_https_get( &ip_address, SIMPLE_GET_REQUEST, buffer, BUFFER_LENGTH, "www.google.com" );
    if ( result == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "Server returned\n%s", buffer ) );
    }
    else
    {
        WPRINT_APP_INFO( ( "Get failed: %u\n", result ) );
    }

    wiced_deinit();
}
