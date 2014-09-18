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
#include "wwd_constants.h"
#include <wiced_utilities.h>
#include <resources.h>

/******************************************************
 *                      Macros
 ******************************************************/

#define CMP_MAC( a, b )  (((a[0])==(b[0]))&& \
                          ((a[1])==(b[1]))&& \
                          ((a[2])==(b[2]))&& \
                          ((a[3])==(b[3]))&& \
                          ((a[4])==(b[4]))&& \
                          ((a[5])==(b[5])))

#define NULL_MAC( a )  (((a[0])==0)&& \
                        ((a[1])==0)&& \
                        ((a[2])==0)&& \
                        ((a[3])==0)&& \
                        ((a[4])==0)&& \
                        ((a[5])==0))

/******************************************************
 *                    Constants
 ******************************************************/

#define SSID_FIELD_NAME            "ssid"
#define SECURITY_FIELD_NAME        "at0"
#define CHANNEL_FIELD_NAME         "chan"
#define BSSID_FIELD_NAME           "bssid"
#define PASSPHRASE_FIELD_NAME      "ap0"
#define PIN_FIELD_NAME             "pin"

#define APP_SCRIPT_PT1     "var elem_num = "
#define APP_SCRIPT_PT2     ";\n var labelname = \""
#define APP_SCRIPT_PT3     "\";\n var fieldname  = \"v"
#define APP_SCRIPT_PT4     "\";\n var fieldvalue = \""
#define APP_SCRIPT_PT5     "\";\n"


#define SCAN_SCRIPT_PT1    "var elem_num = "
#define SCAN_SCRIPT_PT2    ";\n var SSID = \""
#define SCAN_SCRIPT_PT3    "\";\n var RSSIstr  = \""
#define SCAN_SCRIPT_PT4    "\";\n var SEC = "
#define SCAN_SCRIPT_PT5    ";\n var CH  = "
#define SCAN_SCRIPT_PT6    ";\n var BSSID  = \""
#define SCAN_SCRIPT_PT7    "\";\n"

/* Signal strength defines (in dBm) */
#define RSSI_VERY_POOR             -85
#define RSSI_POOR                  -70
#define RSSI_GOOD                  -55
#define RSSI_VERY_GOOD             -40
#define RSSI_EXCELLENT             -25
#define RSSI_VERY_POOR_STR         "Very Poor"
#define RSSI_POOR_STR              "Poor"
#define RSSI_GOOD_STR              "Good"
#define RSSI_VERY_GOOD_STR         "Very good"
#define RSSI_EXCELLENT_STR         "Excellent"

#define CAPTIVE_PORTAL_REDIRECT_PAGE \
    "<html><head>" \
    "<meta http-equiv=\"refresh\" content=\"0; url=/config/device_settings.html\">" \
    "</head></html>"

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct
{
    wiced_tcp_stream_t* stream;
    wiced_semaphore_t semaphore;
    int result_count;
} process_scan_data_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static int32_t        process_app_settings_page ( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_wps_go            ( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_scan              ( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_connect           ( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static int32_t        process_config_save       ( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body );
static wiced_result_t scan_handler              ( wiced_scan_handler_result_t* malloced_scan_result );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/**
 * URL Handler List
 */
START_OF_HTTP_PAGE_DATABASE(config_http_page_database)
    ROOT_HTTP_PAGE_REDIRECT("/config/device_settings.html"),
    { "/images/brcmlogo.png",            "image/png", WICED_RESOURCE_URL_CONTENT,                           .url_content.resource_data  = &resources_images_DIR_brcmlogo_png,      },
    { "/images/brcmlogo_line.png",       "image/png", WICED_RESOURCE_URL_CONTENT,                           .url_content.resource_data  = &resources_images_DIR_brcmlogo_line_png, },
    { "/wpad.dat",                       "application/x-ns-proxy-autoconfig", WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_scripts_DIR_wpad_dat,         },
    { "/scan_results.txt",               "text/plain",                        WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data   = {process_scan,                  0 }          },
    { "/images/64_0bars.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_64_0bars_png,           },
    { "/images/64_1bars.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_64_1bars_png,           },
    { "/images/64_2bars.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_64_2bars_png,           },
    { "/images/64_3bars.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_64_3bars_png,           },
    { "/images/64_4bars.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_64_4bars_png,           },
    { "/images/64_5bars.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_64_5bars_png,           },
    { "/images/tick.png",                "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_tick_png,               },
    { "/images/cross.png",               "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_cross_png,              },
    { "/images/lock.png",                "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_lock_png,               },
    { "/images/progress.gif",            "image/gif",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_progress_gif,           },
    { "/config/device_settings.html",    "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data   = {process_app_settings_page,     0 } },
    { "/config/scan_page_outer.html",    "text/html",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_config_DIR_scan_page_outer_html,   },
    { "/scripts/general_ajax_script.js", "application/javascript",            WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_scripts_DIR_general_ajax_script_js,},
    { "/images/wps_icon.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_wps_icon_png,           },
    { "/images/scan_icon.png",           "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_scan_icon_png,          },
    { "/images/favicon.ico",             "image/vnd.microsoft.icon",          WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_favicon_ico,            },
    { "/images/brcmlogo.png",            "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_brcmlogo_png,           },
    { "/images/brcmlogo_line.png",       "image/png",                         WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_images_DIR_brcmlogo_line_png,      },
    { "/styles/buttons.css",             "text/css",                          WICED_RESOURCE_URL_CONTENT,   .url_content.resource_data  = &resources_styles_DIR_buttons_css,            },
    { "/connect",                        "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data   = {process_connect,               0 }          },
    { "/wps_go",                         "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data   = {process_wps_go,                0 }          },
    { "/config_save",                    "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data   = {process_config_save,           0 }          },
    { IOS_CAPTIVE_PORTAL_ADDRESS,        "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {CAPTIVE_PORTAL_REDIRECT_PAGE, sizeof(CAPTIVE_PORTAL_REDIRECT_PAGE) } },
    /* Add more pages here */
END_OF_HTTP_PAGE_DATABASE();

extern const configuration_entry_t* app_configuration;
extern wiced_http_server_t*         http_server;
extern wiced_bool_t                 config_use_wps;
extern char                         config_wps_pin[9];

/******************************************************
 *               Function Definitions
 ******************************************************/

int32_t process_app_settings_page( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    const configuration_entry_t* config_entry;
    char                  temp_buf[10];
    const resource_hnd_t* end_str_res;
    uint32_t              utoa_size;
    char                  config_count[2] = {'0','0'};

    UNUSED_PARAMETER( url_parameters );
    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( http_message_body );

    wiced_tcp_stream_write_resource( stream, &resources_config_DIR_device_settings_html );

    /* Write the app configuration table */
    if( app_configuration != NULL )
    {
        for (config_entry = app_configuration; config_entry->name != NULL; ++config_entry)
        {

            /* Write the table entry start html direct from resource file */
            switch (config_entry->data_type)
            {
                case CONFIG_STRING_DATA:
                wiced_tcp_stream_write_resource( stream, &resources_config_DIR_device_settings_html_dev_settings_str );
                    break;
                case CONFIG_UINT8_DATA:
                case CONFIG_UINT16_DATA:
                case CONFIG_UINT32_DATA:
                    wiced_tcp_stream_write_resource( stream, &resources_config_DIR_device_settings_html_dev_settings_int );
                    break;
                default:
                    wiced_tcp_stream_write(stream, "error", 5);
                    break;
            }

            /* Output javascript to fill the table entry */

            wiced_tcp_stream_write( stream, APP_SCRIPT_PT1, sizeof(APP_SCRIPT_PT1)-1 );
            wiced_tcp_stream_write( stream, config_count, 2 );
            wiced_tcp_stream_write( stream, APP_SCRIPT_PT2, sizeof(APP_SCRIPT_PT2)-1 );
            wiced_tcp_stream_write( stream, config_entry->name, (uint16_t) strlen( config_entry->name ) );
            wiced_tcp_stream_write( stream, APP_SCRIPT_PT3, sizeof(APP_SCRIPT_PT3)-1 );
            wiced_tcp_stream_write( stream, config_count, 2 );
            wiced_tcp_stream_write( stream, APP_SCRIPT_PT4, sizeof(APP_SCRIPT_PT4)-1 );

            /* Fill in current value */
            switch (config_entry->data_type)
            {
                case CONFIG_STRING_DATA:
                    {
                        char* str_ptr = NULL;
                        wiced_dct_read_lock( (void**)&str_ptr, WICED_FALSE, DCT_APP_SECTION, config_entry->dct_offset, config_entry->data_size );
                        wiced_tcp_stream_write(stream, str_ptr, (uint16_t) strlen( str_ptr ) );
                        wiced_dct_read_unlock( str_ptr, WICED_FALSE );
                        end_str_res = &resources_config_DIR_device_settings_html_dev_settings_str_end;
                    }
                    break;
                case CONFIG_UINT8_DATA:
                    {
                        uint8_t * data;
                        wiced_dct_read_lock( (void**)&data, WICED_FALSE, DCT_APP_SECTION, config_entry->dct_offset, config_entry->data_size );
                        memset(temp_buf, ' ', 3);
                        utoa_size = utoa(*data, (char*)temp_buf, 0, 3);
                        wiced_dct_read_unlock( data, WICED_FALSE );
                        wiced_tcp_stream_write(stream, temp_buf, (uint16_t) utoa_size);
                        end_str_res = &resources_config_DIR_device_settings_html_dev_settings_int_end;
                    }
                    break;
                case CONFIG_UINT16_DATA:
                    {
                        uint16_t * data;
                        wiced_dct_read_lock( (void**)&data, WICED_FALSE, DCT_APP_SECTION, config_entry->dct_offset, config_entry->data_size );
                        memset(temp_buf, ' ', 5);
                        utoa_size = utoa(*data, (char*)temp_buf, 0, 5);
                        wiced_dct_read_unlock( data, WICED_FALSE );
                        wiced_tcp_stream_write(stream, temp_buf, (uint16_t) utoa_size);
                        end_str_res = &resources_config_DIR_device_settings_html_dev_settings_int_end;
                    }
                    break;
                case CONFIG_UINT32_DATA:
                    {
                        uint32_t * data;
                        wiced_dct_read_lock( (void**)&data, WICED_FALSE, DCT_APP_SECTION, config_entry->dct_offset, config_entry->data_size );
                        memset(temp_buf, ' ', 10);
                        utoa_size = utoa(*data, (char*)temp_buf, 0, 10);
                        wiced_dct_read_unlock( data, WICED_FALSE );
                        wiced_tcp_stream_write(stream, temp_buf, (uint16_t) utoa_size);
                        end_str_res = &resources_config_DIR_device_settings_html_dev_settings_int_end;
                    }
                    break;
                default:
                    wiced_tcp_stream_write(stream, "error", 5);
                    end_str_res = NULL;
                    break;
            }

            wiced_tcp_stream_write(stream, APP_SCRIPT_PT5, sizeof(APP_SCRIPT_PT5)-1);
            wiced_tcp_stream_write_resource(stream, end_str_res);


            if (config_count[1] == '9')
            {
                ++config_count[0];
                config_count[1] = '0';
            }
            else
            {
                ++config_count[1];
            }
        }
    }
    wiced_tcp_stream_write_resource( stream, &resources_config_DIR_device_settings_html_dev_settings_bottom );

    return 0;
}


static wiced_result_t scan_handler( wiced_scan_handler_result_t* malloced_scan_result )
{
    process_scan_data_t* scan_data = (process_scan_data_t*)malloced_scan_result->user_data;

    malloc_transfer_to_curr_thread( malloced_scan_result );

    /* Check if scan is not finished */
    if ( malloced_scan_result->status == WICED_SCAN_INCOMPLETE )
    {
        char temp_buffer[70];
        char* temp_ptr;
        uint16_t temp_length;
        int i;

        wiced_tcp_stream_t* stream = scan_data->stream;

        /* Result ID */
        temp_length = (uint16_t) sprintf( temp_buffer, "%d\n", scan_data->result_count );
        scan_data->result_count++;
        wiced_tcp_stream_write(stream, temp_buffer, temp_length);

        /* SSID */
        temp_ptr = temp_buffer;
        for( i = 0; i < malloced_scan_result->ap_details.SSID.length; i++)
        {
            temp_ptr += sprintf( temp_ptr, "%02X", malloced_scan_result->ap_details.SSID.value[i] );
        }
        temp_ptr += sprintf( temp_ptr, "\n" );

        wiced_tcp_stream_write(stream, temp_buffer, (uint32_t)( temp_ptr - temp_buffer ) );

        /* Security */
        temp_length = (uint16_t) sprintf( temp_buffer, "%d\n", malloced_scan_result->ap_details.security );
        wiced_tcp_stream_write(stream, temp_buffer, temp_length);
        temp_length = (uint16_t) sprintf( temp_buffer, "%s\n", (   malloced_scan_result->ap_details.security == WICED_SECURITY_OPEN )? "OPEN" :
                                                               ( ( malloced_scan_result->ap_details.security & WEP_ENABLED   ) != 0 )? "WEP"  :
                                                               ( ( malloced_scan_result->ap_details.security & WPA_SECURITY  ) != 0 )? "WPA"  :
                                                               ( ( malloced_scan_result->ap_details.security & WPA2_SECURITY ) != 0 )? "WPA2" : "UNKNOWN" );
        wiced_tcp_stream_write(stream, temp_buffer, temp_length);

        /* RSSI */
        temp_length = (uint16_t) sprintf( temp_buffer, "%d\n", malloced_scan_result->ap_details.signal_strength );
        wiced_tcp_stream_write(stream, temp_buffer, temp_length);

        /* Channel */
        temp_length = (uint16_t) sprintf( temp_buffer, "%d\n", malloced_scan_result->ap_details.channel );
        wiced_tcp_stream_write(stream, temp_buffer, temp_length);

        /* BSSID */
        temp_length = (uint16_t) sprintf( temp_buffer, "%02X%02X%02X%02X%02X%02X\n", malloced_scan_result->ap_details.BSSID.octet[0], malloced_scan_result->ap_details.BSSID.octet[1], malloced_scan_result->ap_details.BSSID.octet[2], malloced_scan_result->ap_details.BSSID.octet[3], malloced_scan_result->ap_details.BSSID.octet[4], malloced_scan_result->ap_details.BSSID.octet[5] );
        wiced_tcp_stream_write(stream, temp_buffer, temp_length);

        /* Remembered */
        temp_length = (uint16_t) sprintf( temp_buffer, "%d\n", 0 );
        wiced_tcp_stream_write(stream, temp_buffer, temp_length);


    }
    else
    {
        wiced_rtos_set_semaphore( &scan_data->semaphore );
    }

    free(malloced_scan_result);

    return WICED_SUCCESS;
}


static int32_t process_scan( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    process_scan_data_t scan_data;

    UNUSED_PARAMETER( url_parameters );
    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( http_message_body );

    scan_data.stream = stream;
    scan_data.result_count = 0;

    /* Initialise the semaphore that will tell us when the scan is complete */
    wiced_rtos_init_semaphore(&scan_data.semaphore);

    wiced_tcp_stream_write(stream, "http\n", sizeof("http\n"));

    /* Start the scan */
    wiced_wifi_scan_networks( scan_handler, &scan_data );

    /* Wait until scan is complete */
    wiced_rtos_get_semaphore(&scan_data.semaphore, WICED_WAIT_FOREVER);

    /* Clean up */
    wiced_rtos_deinit_semaphore(&scan_data.semaphore);

    return 0;
}


static int32_t process_wps_go( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    unsigned int url_parameters_len;

    UNUSED_PARAMETER( stream );
    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( http_message_body );

    url_parameters_len = strlen(url_parameters);

    /* client has signalled to start client mode via WPS. */
    config_use_wps = WICED_TRUE;

    /* Check if config method is PIN */
    if ( ( strlen( PIN_FIELD_NAME ) + 1 < url_parameters_len ) &&
         ( 0 == strncmp( url_parameters, PIN_FIELD_NAME "=", strlen( PIN_FIELD_NAME ) + 1 ) ) )
    {
        unsigned int pinlen = 0;

        url_parameters += strlen( PIN_FIELD_NAME ) + 1;

        /* Find length of pin */
        while ( ( url_parameters[pinlen] != '&'    ) &&
                ( url_parameters[pinlen] != '\n'   ) &&
                ( url_parameters[pinlen] != '\x00' ) &&
                ( url_parameters_len > 0 ) )
        {
            pinlen++;
            url_parameters_len--;
        }
        memcpy( config_wps_pin, url_parameters, pinlen );
        config_wps_pin[pinlen] = '\x00';
    }
    else
    {
        config_wps_pin[0] = '\x00';
    }

    /* Config has been set. Turn off HTTP server */
    wiced_http_server_stop(http_server);
    return 1;
}


/**
 * URL handler for signaling web server shutdown
 *
 * The reception of this web server request indicates that the client wants to
 * start the appliance, after shutting down the access point, DHCP server and web server
 * Decodes the URL parameters into the connection configuration buffer, then signals
 * for the web server to shut down
 *
 * @param  socket  : a handle for the TCP socket over which the data will be sent
 * @param  url_parameters     : a byte array containing any parameters included in the URL
 * @param  url_parameters_len : size of the url_parameters byte array in bytes
 */
static int32_t process_connect( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    /* This is the first part of the platform_dct_wifi_config_t structure */
    struct
    {
        wiced_bool_t             device_configured;
        wiced_config_ap_entry_t  ap_entry;
    } temp_config;

    UNUSED_PARAMETER( http_message_body );
    UNUSED_PARAMETER( stream );
    UNUSED_PARAMETER( arg );

    memset( &temp_config, 0, sizeof(temp_config) );

    /* First, parse AP details */
    while (url_parameters[0] == 'a' && url_parameters[3] == '=')
    {
        uint8_t ap_index;
        const char* end_of_value;

        /* Extract the AP index and check validity */
        ap_index = (uint8_t)( url_parameters[2] - '0' );
        if (ap_index >= CONFIG_AP_LIST_SIZE)
        {
            return -1;
        }

        /* Find the end of the value */
        end_of_value = &url_parameters[4];
        while( (*end_of_value != '&') && (*end_of_value != '\x00') && (*end_of_value != '\n') )
        {
            ++end_of_value;
        }

        /* Parse either the SSID or PSK*/
        if ( url_parameters[1] == 's' )
        {
            memcpy( temp_config.ap_entry.details.SSID.value, &url_parameters[4], (size_t) ( end_of_value - &url_parameters[4] ) );
            temp_config.ap_entry.details.SSID.length = (uint8_t) ( end_of_value - &url_parameters[4] );
            temp_config.ap_entry.details.SSID.value[temp_config.ap_entry.details.SSID.length] = 0;
        }
        else if (url_parameters[1] == 'p')
        {
            temp_config.ap_entry.security_key_length = (uint8_t) ( end_of_value - &url_parameters[4] );
            memcpy( temp_config.ap_entry.security_key, &url_parameters[4], temp_config.ap_entry.security_key_length);
            temp_config.ap_entry.security_key[temp_config.ap_entry.security_key_length] = 0;
        }
        else if (url_parameters[1] == 't')
        {
            temp_config.ap_entry.details.security = (wiced_security_t) atoi( &url_parameters[4] );
        }
        else
        {
            return -1;
        }
        url_parameters = end_of_value + 1;
    }

    /* Save updated config details */
    temp_config.device_configured = WICED_TRUE;
    wiced_dct_write( &temp_config, DCT_WIFI_CONFIG_SECTION, 0, sizeof(temp_config) );

    /* Config has been set. Turn off HTTP server */
    wiced_http_server_stop(http_server);
    return 0;
}

static int32_t process_config_save( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_message_body )
{
    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( http_message_body );

    if ( app_configuration != NULL )
    {
        uint32_t earliest_offset = 0xFFFFFFFF;
        uint32_t end_of_last_offset = 0x0;
        const configuration_entry_t* config_entry;
        uint8_t* app_dct;

        /* Calculate how big the app config details are */
        for ( config_entry = app_configuration; config_entry->name != NULL; ++config_entry )
        {
            if ( config_entry->dct_offset < earliest_offset )
            {
                earliest_offset = config_entry->dct_offset;
            }
            if ( config_entry->dct_offset + config_entry->data_size > end_of_last_offset )
            {
                end_of_last_offset = config_entry->dct_offset + config_entry->data_size;
            }
        }

        wiced_dct_read_lock( (void**)&app_dct, WICED_TRUE, DCT_APP_SECTION, earliest_offset, end_of_last_offset - earliest_offset );
        if ( app_dct != NULL )
        {
            while ( url_parameters[0] == 'v' && url_parameters[3] == '=' )
            {
                /* Extract the variable index and check validity */
                uint16_t variable_index = (uint16_t) ( ( ( url_parameters[1] - '0' ) << 8 ) | ( url_parameters[2] - '0' ) );

                /* Find the end of the value */
                const char* end_of_value = &url_parameters[4];
                while ( ( *end_of_value != '&' ) && ( *end_of_value != '\n' ) )
                {
                    ++end_of_value;
                }

                /* Parse param */
                config_entry = &app_configuration[variable_index];
                switch ( config_entry->data_type )
                {
                    case CONFIG_STRING_DATA:
                        memcpy( (uint8_t*) ( app_dct + config_entry->dct_offset ), &url_parameters[4], (size_t) ( end_of_value - &url_parameters[4] ) );
                        ( (uint8_t*) ( app_dct + config_entry->dct_offset ) )[end_of_value - &url_parameters[4]] = 0;
                        break;
                    case CONFIG_UINT8_DATA:
                        *(uint8_t*) ( app_dct + config_entry->dct_offset - earliest_offset ) = (uint8_t) atoi( &url_parameters[4] );
                        break;
                    case CONFIG_UINT16_DATA:
                        *(uint16_t*) ( app_dct + config_entry->dct_offset - earliest_offset ) = (uint16_t) atoi( &url_parameters[4] );
                        break;
                    case CONFIG_UINT32_DATA:
                        *(uint32_t*) ( app_dct + config_entry->dct_offset - earliest_offset ) = (uint32_t) atoi( &url_parameters[4] );
                        break;
                    default:
                        break;
                }

                url_parameters = end_of_value + 1;
            }

            /* Write the app DCT */
            wiced_dct_write( app_dct, DCT_APP_SECTION, earliest_offset, end_of_last_offset - earliest_offset );

            wiced_dct_read_unlock( app_dct, WICED_TRUE );
        }
    }

    #define CONFIG_SAVE_SUCCESS  "Config saved"
    wiced_tcp_stream_write(stream, CONFIG_SAVE_SUCCESS, sizeof(CONFIG_SAVE_SUCCESS)-1);

    return 0;
}

