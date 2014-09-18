/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include "../console.h"
#include "stdlib.h"
#include "wwd_debug.h"
#include "string.h"
#include "wps_host.h"
#include "wiced_wps.h"
#include "../wifi/wifi.h"
#include "wiced_management.h"
#include "wwd_crypto.h"
#include "wiced_framework.h"
#include "wps_host_interface.h"
#include "wiced_time.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define IS_DIGIT(c) ((c >= '0') && (c <= '9'))

/******************************************************
 *                    Constants
 ******************************************************/
#define MAX_CREDENTIAL_COUNT   5
#define MAX_SSID_LEN 32
#define MAX_PASSPHRASE_LEN 64
#define JOIN_ATTEMPT_TIMEOUT   60000

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
static int is_digit_str( const char* str );
static int validate_checksum( const char* str );
static int compute_checksum(unsigned long int PIN);
static void dehyphenate_pin(char* str );
static int generate_wps_pin( char* pin_string );
static wiced_result_t internal_start_registrar( wiced_wps_mode_t mode, const wiced_wps_device_detail_t* details, char* password, wiced_wps_credential_t* credentials, uint16_t credential_count );

/******************************************************
 *               Variable Definitions
 ******************************************************/
static const wiced_wps_device_detail_t enrollee_details =
{
    .device_name     = PLATFORM,
    .manufacturer    = "Broadcom",
    .model_name      = PLATFORM,
    .model_number    = "1.0",
    .serial_number   = "1408248",
    .device_category = WICED_WPS_DEVICE_COMPUTER,
    .sub_category    = 7,
    .config_methods  = WPS_CONFIG_LABEL | WPS_CONFIG_VIRTUAL_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_DISPLAY_PIN
};

static const wiced_wps_device_detail_t device_details =
{
    .device_name     = "Wiced",
    .manufacturer    = "Broadcom",
    .model_name      = "BCM943362",
    .model_number    = "Wiced",
    .serial_number   = "12345670",
    .device_category = PRIMARY_DEVICE_NETWORK_INFRASTRUCTURE,
    .sub_category    = 1,
    .config_methods  = WPS_CONFIG_LABEL | WPS_CONFIG_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_DISPLAY_PIN,
};

static wps_agent_t* workspace = NULL;


/******************************************************
 *               Function Definitions
 ******************************************************/

int join_wps( int argc, char* argv[] )
{
    int a;
    char* ip = NULL;
    char* netmask = NULL;
    char* gateway = NULL;
    wiced_result_t result = WICED_ERROR;
    wiced_wps_credential_t credential[MAX_CREDENTIAL_COUNT];
    char pin_string[9];

    memset( credential, 0, MAX_CREDENTIAL_COUNT*sizeof( wiced_wps_credential_t ) );

    if (wwd_wifi_is_ready_to_transceive(WWD_STA_INTERFACE) == WWD_SUCCESS)
    {
        wiced_network_down( WWD_STA_INTERFACE );
    }

    if ( strcmp( argv[1], "pin" ) == 0 )
    {
        /* PIN mode*/
        if ( argc == 6 ) /* PIN is supplied */
        {
            ip      = argv[3];
            netmask = argv[4];
            gateway = argv[5];
        }
        else if ( argc == 5 ) /* PIN is auto-generated */
        {
            ip      = argv[2];
            netmask = argv[3];
            gateway = argv[4];
        }

        if ( argc == 3 || argc == 6)
        {
            if ( is_digit_str(argv[2]) && ( ( strlen( argv[2] ) == 4 ) || ( strlen( argv[2] ) == 8 ) ) )
            {
                if ( strlen( argv[2] ) == 8 )
                {
                    if ( !validate_checksum( argv[2] ) )
                    {
                        WPRINT_APP_INFO(("Invalid PIN checksum\r\n"));
                        return ( ERR_CMD_OK );
                    }
                }
                WPRINT_APP_INFO(("Starting Enrollee in PIN mode\r\n"));
                result = wiced_wps_enrollee(WICED_WPS_PIN_MODE, &enrollee_details, argv[2], credential, MAX_CREDENTIAL_COUNT);
            }
            else
            {
                WPRINT_APP_INFO(("PIN must be 4 or 8 digits\r\n"));
                return ( ERR_CMD_OK );
            }
        }
        else if ( argc == 2 || argc == 5)
        {
            generate_wps_pin( pin_string );
            if ( validate_checksum( pin_string ) )
            {
                WPRINT_APP_INFO(("Starting Enrollee in PIN mode\r\n"));
                result = wiced_wps_enrollee(WICED_WPS_PIN_MODE, &enrollee_details, pin_string, credential, MAX_CREDENTIAL_COUNT);
            }
            else
            {
                WPRINT_APP_INFO(("Invalid PIN checksum, try again\r\n"));
            }
        }
        else
        {
            return ERR_UNKNOWN;
        }
    }
    else if ( strcmp( argv[1], "pbc" ) == 0 )
    {
        /* Push Button mode */
        if ( argc == 5 )
        {
            ip      = argv[2];
            netmask = argv[3];
            gateway = argv[4];
        }
        if (argc == 2 || argc == 5)
        {
            WPRINT_APP_INFO(("Starting Enrollee in PBC mode\r\n"));
            result = wiced_wps_enrollee(WICED_WPS_PBC_MODE, &enrollee_details, "00000000", credential, MAX_CREDENTIAL_COUNT);

            switch(result)
            {
                case WWD_SUCCESS:
                    WPRINT_APP_INFO(("WPS Successful\r\n"));
                    break;

                case WWD_WPS_PBC_OVERLAP:
                    WPRINT_APP_INFO(("PBC overlap detected - wait and try again\r\n"));
                    break;

                default:
                    /* WPS failed. Abort */
                   WPRINT_APP_INFO(("WPS failed\r\n"));
                   break;
            }
        }
        else
        {
            return ERR_UNKNOWN;
        }
    }
    else
    {
        return ERR_UNKNOWN;
    }

    /* Check if WPS was successful */
    if ( credential[0].ssid.length != 0 )
    {
        for (a=0; credential[a].ssid.length != 0; ++a)
        {
            WPRINT_APP_INFO(("SSID: %s\r\n",credential[a].ssid.value));
            WPRINT_APP_INFO(("Security: "));
            switch ( credential[a].security )
            {
                case WICED_SECURITY_OPEN:
                    WPRINT_APP_INFO(("Open\r\n"));
                    break;
                case WICED_SECURITY_WEP_PSK:
                    WPRINT_APP_INFO(("WEP PSK\r\n"));
                    break;
                case WICED_SECURITY_WPA_TKIP_PSK:
                case WICED_SECURITY_WPA_AES_PSK:
                    WPRINT_APP_INFO(("WPA PSK\r\n"));
                    break;
                case WICED_SECURITY_WPA2_AES_PSK:
                case WICED_SECURITY_WPA2_TKIP_PSK:
                case WICED_SECURITY_WPA2_MIXED_PSK:
                    WPRINT_APP_INFO(("WPA2 PSK\r\n"));
                    break;
                default:
                    break;
            }
            if ( credential[a].security != WICED_SECURITY_OPEN )
            {
                WPRINT_APP_INFO(("Network key: "));
                if ( credential[a].passphrase_length != 64 )
                {
                    WPRINT_APP_INFO(("%s\r\n", credential[a].passphrase));
                }
                else
                {
                    WPRINT_APP_INFO(("%.64s\r\n", credential[a].passphrase));
                }
            }
        }


        /* Join AP */
        int ret;
        wiced_wps_credential_t* cred;
        a = 0;
        uint32_t start_time = host_get_time( );
        do
        {
            if (( host_get_time( ) - start_time ) > JOIN_ATTEMPT_TIMEOUT)
            {
                return ERR_UNKNOWN;
            }
            cred = &credential[a];
            WPRINT_APP_INFO(("Joining : %s\r\n", cred->ssid.value));
            ret = wifi_join( (char*)cred->ssid.value, cred->security, (uint8_t*) cred->passphrase, cred->passphrase_length, ip, netmask, gateway );
            if (ret != ERR_CMD_OK)
            {
                WPRINT_APP_INFO(("Failed to join  : %s   .. retrying\r\n", cred->ssid.value));
                ++a;
                if (credential[a].ssid.length == 0)
                {
                    a = 0;
                }
            }
        }
        while (ret != ERR_CMD_OK);
        WPRINT_APP_INFO(("Successfully joined : %s\r\n", cred->ssid.value));
    }

    return ERR_CMD_OK;
}


/*!
 ******************************************************************************
 * Starts a WPS Registrar as specified by the provided arguments
 *
 * @return  0 for success, otherwise error
 */

int start_registrar( int argc, char* argv[] )
{
    static wiced_wps_credential_t credential;
    static char pin[10];
    platform_dct_wifi_config_t* dct_wifi_config;

    wiced_result_t result = WICED_ERROR;

    if ( wwd_wifi_is_ready_to_transceive( WICED_AP_INTERFACE ) != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Use start_ap command to bring up AP interface first\r\n"));
        return ERR_CMD_OK;
    }

    /* Read config to get internal AP settings */
    wiced_dct_read_lock( (void**) &dct_wifi_config, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );

    /* Copy config into the credential to be used by WPS */
    strncpy((char*)&credential.ssid.value, (char*)dct_wifi_config->soft_ap_settings.SSID.value, MAX_SSID_LEN);
    credential.ssid.length = strlen((char*)&credential.ssid.value);
    credential.security = dct_wifi_config->soft_ap_settings.security;
    memcpy((char*)&credential.passphrase, (char*)&dct_wifi_config->soft_ap_settings.security_key, MAX_PASSPHRASE_LEN);
    credential.passphrase_length = dct_wifi_config->soft_ap_settings.security_key_length;

    wiced_dct_read_unlock( (void*) dct_wifi_config, WICED_FALSE );

    /* PIN mode */
    if ( ( strcmp( argv[1], "pin" ) == 0 ) && argc >= 3 )
    {
        memset(pin, 0, sizeof(pin));
        strncpy( pin, argv[2], ( sizeof(pin) - 1 ));
        if ( argc == 4 ) /* Then PIN may be in the form nnnn nnnn */
        {
            if ( ( strlen( argv[2] ) == 4 ) && ( strlen( argv[3] ) == 4 ) )
            {
                strncat( pin, argv[3], 4 );
            }
            else
            {
                WPRINT_APP_INFO(("Invalid PIN format\r\n"));
                return ( ERR_CMD_OK );
            }
            argc--;
        }
        if ( argc == 3 )
        {
            if ( strlen( pin ) == 9 ) /* Then PIN may be in the form nnnn-nnnn */
            {
                dehyphenate_pin( pin );
            }
            /* WPRINT_APP_INFO(("pin %s\r\n", pin)); */
            if ( is_digit_str(pin) && ( ( strlen( pin ) == 4 ) || ( strlen( pin ) == 8 ) ) )
            {
                if ( strlen( pin ) == 8 )
                {
                    if ( !validate_checksum( pin ) )
                    {
                        WPRINT_APP_INFO(("Invalid PIN checksum\r\n"));
                        return ( ERR_CMD_OK );
                    }
                }
                WPRINT_APP_INFO(("Starting Registrar in PIN mode\r\n"));
                result = internal_start_registrar(WICED_WPS_PIN_MODE, &device_details, pin, &credential, 1);
            }
            else
            {
                WPRINT_APP_INFO(("PIN must be 4 or 8 digits\r\n"));
                return ( ERR_CMD_OK );
            }
        }
        else
        {
            return ERR_UNKNOWN;
        }
    }
    else if ( strcmp( argv[1], "pbc" ) == 0 )
    {
        /* Push Button mode */
        if (argc == 2)
        {
            WPRINT_APP_INFO(("Starting registrar in PBC mode\r\n"));
            result = internal_start_registrar(WICED_WPS_PBC_MODE, &device_details, "00000000", &credential, 1);

            switch(result)
            {
                case WWD_SUCCESS:
                    /* WPRINT_APP_INFO(("Registrar starting\r\n")); */
                    break;

                case WWD_WPS_PBC_OVERLAP:
                    WPRINT_APP_INFO(("PBC overlap detected\r\n"));
                    break;

                default:
                    /* WPS failed. Abort */
                   WPRINT_APP_INFO(("WPS failed\r\n"));
                   break;
            }
        }
        else
        {
            return ERR_UNKNOWN;
        }
    }
    else
    {
        return ERR_UNKNOWN;
    }

    return ERR_CMD_OK;
}

static wiced_result_t internal_start_registrar( wiced_wps_mode_t mode, const wiced_wps_device_detail_t* details, char* password, wiced_wps_credential_t* credentials, uint16_t credential_count )
{
    wiced_result_t result;

    if ( workspace == NULL )
    {
        workspace = calloc_named("wps", 1, sizeof(wps_agent_t));
        if ( workspace == NULL )
        {
            return WICED_OUT_OF_HEAP_SPACE;
        }

        besl_wps_init( workspace, (besl_wps_device_detail_t*) details, WPS_REGISTRAR_AGENT, WICED_AP_INTERFACE );
    }
    else
    {
        if (workspace->wps_result == WPS_COMPLETE ||
            workspace->wps_result == WPS_PBC_OVERLAP ||
            workspace->wps_result == WPS_TIMEOUT ||
            workspace->wps_result == WPS_ABORTED)
        {
            besl_wps_deinit( workspace );
            besl_wps_init( workspace, (besl_wps_device_detail_t*) details, WPS_REGISTRAR_AGENT, WICED_AP_INTERFACE );
        }
        else if (workspace->wps_result != WPS_NOT_STARTED )
        {
            if ( ( workspace->wps_mode == WPS_PBC_MODE ) && ( mode == WICED_WPS_PBC_MODE ) )
            {
                WPRINT_APP_INFO(("Restarting 2 minute window\r\n"));
                besl_wps_restart( workspace );
            }
            else
            {
                WPRINT_APP_INFO(("WPS already running\r\n"));
            }
            return WWD_SUCCESS;
        }
    }

    result = besl_wps_start( workspace, mode, password, (besl_wps_credential_t*) credentials, credential_count );
    if ( result == WICED_WWD_WPS_PBC_OVERLAP )
    {
        WPRINT_APP_INFO(("WPS fail - PBC overlap\r\n"));
        return result;
    }
    else if ( result != WICED_SUCCESS )
    {
        besl_wps_deinit( workspace );
        free( workspace );
        workspace = NULL;
        return result;
    }

    return WWD_SUCCESS;
}


int stop_registrar( int argc, char* argv[] )
{
    if ( workspace == NULL )
    {
        WPRINT_APP_INFO(("WPS Registrar not yet initialized\r\n"));
    }
    else if (workspace->wps_result != WPS_NOT_STARTED )
    {
        if ( besl_wps_abort( workspace ) == BESL_SUCCESS )
        {
            WPRINT_APP_INFO(("WPS Registrar stopped\r\n"));
        }
    }
    else
    {
        WPRINT_APP_INFO(("WPS Registrar not running\r\n"));
    }
    return ERR_CMD_OK;
}


static int is_digit_str( const char* str )
{
    int res = 0;
    int i = 0;

    if ( str != NULL )
    {
        i = strlen(str);
        res = 1;
        while ( i > 0 )
        {
            if ( !IS_DIGIT(*str) )
            {
                res = 0;
                break;
            }
            str++;
            i--;
        }
    }

    return res;
}


static int generate_wps_pin( char* pin_string )
{
    uint16_t r[2];
    uint32_t random = 0;
    int i, checksum;

    memset( pin_string, '0', 9 );

    /* Generate a random number between 1 and 9999999 */
    while ( random == 0 )
    {
        wwd_wifi_get_random( r, 4 );
        random = (uint32_t)(r[0] * r[1]) % 9999999;
    }

    checksum = compute_checksum( random ); /* Compute checksum which will become the eighth digit */

    i = 8;
    pin_string[i] = '\0';
    i--;
    pin_string[i] = checksum + '0';
    i--;

    do {       /* generate digits */
        pin_string[i] = random % 10 + '0';   /* get next digit */
        i--;
    } while ((random /= 10) > 0);     /* delete it */

    WPRINT_APP_INFO(("Enter this PIN in the other device: %s\r\n", pin_string));

    return ERR_CMD_OK;
}


static int validate_checksum( const char* str )
{
    unsigned long int PIN;
    unsigned long int accum = 0;

    PIN = (unsigned long int) atoi( str );

    accum += 3 * ((PIN / 10000000) % 10);
    accum += 1 * ((PIN / 1000000) % 10);
    accum += 3 * ((PIN / 100000) % 10);
    accum += 1 * ((PIN / 10000) % 10);
    accum += 3 * ((PIN / 1000) % 10);
    accum += 1 * ((PIN / 100) % 10);
    accum += 3 * ((PIN / 10) % 10);
    accum += 1 * ((PIN / 1) % 10);

    return (0 == (accum % 10));
}


static int compute_checksum(unsigned long int PIN)
{
    unsigned long int accum = 0;

    PIN *= 10;
    accum += 3 * ((PIN / 10000000) % 10);
    accum += 1 * ((PIN / 1000000) % 10);
    accum += 3 * ((PIN / 100000) % 10);
    accum += 1 * ((PIN / 10000) % 10);
    accum += 3 * ((PIN / 1000) % 10);
    accum += 1 * ((PIN / 100) % 10);
    accum += 3 * ((PIN / 10) % 10);

    int digit = (accum % 10);

    return (10 - digit) % 10;
}


static void dehyphenate_pin(char* str )
{
    int i;

    for ( i = 4; i < 9; i++ )
    {
        str[i] = str[i+1];
    }
}

int force_alignment( int argc, char* argv[] )
{
    volatile uint32_t* configuration_control_register = (uint32_t*)0xE000ED14;
    *configuration_control_register |= (1 << 3);
    return ERR_CMD_OK;
}

wiced_result_t enable_ap_registrar_events( void )
{
    wiced_result_t result;

    if ( workspace == NULL )
    {
        workspace = calloc_named("wps", 1, sizeof(wps_agent_t));
        if ( workspace == NULL )
        {
            WPRINT_APP_INFO(("Error calloc wps\r\n"));
            stop_ap(0, NULL);
            return WICED_OUT_OF_HEAP_SPACE;
        }
    }

    if ( ( result = besl_wps_init( workspace, (besl_wps_device_detail_t*) &device_details, WPS_REGISTRAR_AGENT, WWD_AP_INTERFACE ) ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Error besl init %u\r\n", (unsigned int)result));
        stop_ap(0, NULL);
        return result;
    }
    if ( ( result = besl_wps_management_set_event_handler( workspace, WICED_TRUE ) ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Error besl setting event handler %u\r\n", (unsigned int)result));
        stop_ap(0, NULL);
        return result;
    }
    return WICED_SUCCESS;
}

void disable_ap_registrar_events( void )
{
    if (workspace != NULL)
    {
        besl_wps_management_set_event_handler( workspace, WICED_FALSE );
        besl_wps_deinit( workspace );
        free( workspace );
        workspace = NULL;
    }
}
