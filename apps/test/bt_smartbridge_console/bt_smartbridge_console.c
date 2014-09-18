/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/**
 * @file
 *
 * This application provides a serial interface to demonstrate usage of
 * the WICED SmartBridge API
 */

#include <math.h>
#include "wiced.h"
#include "http_server.h"
#include "sntp.h"
#include "gedday.h"
#include "resources.h"
#include "wiced_bt.h"
#include "wiced_bt_constants.h"
#include "wiced_bt_smart_interface.h"
#include "wiced_bt_smartbridge.h"
#include "wiced_bt_smartbridge_gatt.h"
#include "bt_smartbridge_dct.h"
#include "bt_transport_thread.h"
#include "bt_stack.h"
#include "console.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_BT_SMART_CONNECTIONS  5

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
#pragma pack(1)
typedef struct
{
    wiced_iso8601_time_t time;
    char                 nul_terminator;
} wiced_iso8601_time_string_t;

typedef struct
{
    wiced_bt_device_address_t     address;
    wiced_bt_smart_address_type_t type;
    wiced_bt_smart_bond_info_t    bond_info;
} bt_smartbridge_bond_info_t;
#pragma pack()

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static wiced_result_t scan_complete_handler                   ( void );
static wiced_result_t scan_advertising_report_handler         ( const wiced_bt_smart_advertising_report_t* advertising_report );
static wiced_result_t disconnection_handler                   ( wiced_bt_smartbridge_socket_t* socket );
static wiced_result_t pairing_handler                         ( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_bond_info_t* bond_info );
static wiced_result_t notification_handler                    ( wiced_bt_smartbridge_socket_t* socket, uint16_t attribute_handle );
static wiced_result_t find_bond_info                          ( const wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type, wiced_bt_smart_bond_info_t* bond_info );
static wiced_result_t store_bond_info                         ( const wiced_bt_smart_bond_info_t* bond_info );
static wiced_result_t convert_hex_str_to_uint8_t_array        ( const char* hex_string, uint8_t array[], uint32_t size );
static wiced_result_t convert_str_to_device_address           ( const char* addr_string, wiced_bt_device_address_t* address );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Scan settings */
static wiced_bt_smart_scan_settings_t scan_settings =
{
    .type              = BT_SMART_ACTIVE_SCAN,
    .filter_policy     = FILTER_POLICY_NONE,
    .filter_duplicates = DUPLICATES_FILTER_ENABLED,
    .interval          = 96,
    .window            = 48,
    .duration_second   = 5,
};

/* SmartBridge connection settings */
static const wiced_bt_smart_connection_settings_t connection_settings =
{
    .timeout_second                = 3,
    .filter_policy                 = FILTER_POLICY_NONE,
    .interval_min                  = 40,
    .interval_max                  = 56,
    .latency                       = 0,
    .supervision_timeout           = 100,
    .ce_length_min                 = 0,
    .ce_length_max                 = 0,
    .attribute_protocol_timeout_ms = 2000,
};

/* SmartBridge security settings */
static const wiced_bt_smart_security_settings_t security_settings =
{
    .timeout_second              = 15,
    .io_capabilities             = BT_SMART_IO_KEYBOARD_DISPLAY,
    .authentication_requirements = BT_SMART_AUTH_REQ_BONDING_AND_PASSKEY_ENTRY,
    .oob_authentication          = BT_SMART_OOB_AUTH_NONE,
    .max_encryption_key_size     = 16,
    .master_key_distribution     = BT_SMART_DISTRIBUTE_ALL_KEYS,
    .slave_key_distribution      = BT_SMART_DISTRIBUTE_ALL_KEYS,
};

static wiced_semaphore_t             scan_complete_semaphore;
static wiced_bt_smartbridge_socket_t smartbridge_socket      [MAX_BT_SMART_CONNECTIONS];
static uint16_t                      last_notification_handle[MAX_BT_SMART_CONNECTIONS];
static wiced_iso8601_time_string_t   last_notification_time  [MAX_BT_SMART_CONNECTIONS];
static wiced_mutex_t                 dct_mutex;
static wiced_bt_smart_device_t       directed_advertising_device;
static wiced_bool_t                  directed_advertisement_received = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/

/* Application entry point
 */
void application_start( )
{
    /* Initialise WICED */
    wiced_init( );

    /* Initialise DCT mutex */
    wiced_rtos_init_mutex( &dct_mutex );

    console_app_main( );
}

/* Scan complete handler. Scan complete event reported via this callback.
 * It runs on the WICED_NETWORKING_WORKER_THREAD context.
 */
static wiced_result_t scan_complete_handler( void )
{
    WPRINT_APP_INFO( ( "Scan complete\n") );
    wiced_rtos_set_semaphore( &scan_complete_semaphore );
    return WICED_SUCCESS;
}

/* Scan result handler. Scan result is reported via this callback.
 * It runs on the Bluetooth transport thread context.
 */
static wiced_result_t scan_advertising_report_handler( const wiced_bt_smart_advertising_report_t* advertising_report )
{
    /* When a Bluetooth Smart devices sends a directed advertisement, the device
     * expects a connection request before the advertising duration expires.
     * In here, stop scanning and set semaphore to notify app thread that a directed
     * advertisement has been received.
     */
    if ( advertising_report->event == BT_SMART_CONNECTABLE_DIRECTED_ADVERTISING_EVENT )
    {
        directed_advertisement_received = WICED_TRUE;

        memcpy( &directed_advertising_device, &advertising_report->remote_device, sizeof( directed_advertising_device ) );

        wiced_rtos_set_semaphore( &scan_complete_semaphore );
    }

    return WICED_SUCCESS;
}

/* Disconnection handler. Disconnection by remote device is reported via this callback.
 * It runs on the WICED_NETWORKING_WORKER_THREAD context.
 */
static wiced_result_t disconnection_handler( wiced_bt_smartbridge_socket_t* socket )
{
    uint8_t a;

    /* Find socket */
    for ( a = 0; a < MAX_BT_SMART_CONNECTIONS; a++ )
    {
        if ( &smartbridge_socket[a] == socket )
        {
            WPRINT_APP_INFO( ( "Socket %u disconnected by remote device\n", a ) );

            /* Socket found. a is the index */
            last_notification_handle[a] = 0;
            memset( &last_notification_time[a], 0, sizeof(wiced_iso8601_time_string_t) );
            break;
        }
    }

    return WICED_SUCCESS;
}

/* Bonding handler. Successful bonding is reported via this callback.
 * It runs on the WICED_NETWORKING_WORKER_THREAD context.
 */
static wiced_result_t pairing_handler( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_bond_info_t* bond_info )
{
    WPRINT_LIB_INFO( ( "Bonded to %02X:%02X:%02X:%02X:%02X:%02X\n",
                       bond_info->peer_address.address[5],
                       bond_info->peer_address.address[4],
                       bond_info->peer_address.address[3],
                       bond_info->peer_address.address[2],
                       bond_info->peer_address.address[1],
                       bond_info->peer_address.address[0] ) );

    store_bond_info( bond_info );
    return WICED_SUCCESS;
}

/* Notification handler. GATT notification by remote device is reported via this callback.
 * It runs on the WICED_NETWORKING_WORKER_THREAD context.
 */
static wiced_result_t notification_handler( wiced_bt_smartbridge_socket_t* socket, uint16_t attribute_handle )
{
    uint8_t a;

    WPRINT_LIB_INFO( ( "Notification[%lu,%lu]\n", (uint32_t)socket->connection_handle, (uint32_t)attribute_handle ) );

    /* Find socket */
    for ( a = 0; a < MAX_BT_SMART_CONNECTIONS; a++ )
    {
        if ( &smartbridge_socket[a] == socket )
        {
            /* Socket found. a is the index */
            break;
        }
    }

    /* Store attribute handle and timestamp */
    last_notification_handle[a] = attribute_handle;
    wiced_time_get_iso8601_time( &last_notification_time[a].time );

    return WICED_SUCCESS;
}

/* Find Bond Info of device with given address and address type in DCT
 */
static wiced_result_t find_bond_info( const wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type, wiced_bt_smart_bond_info_t* bond_info )
{
    bt_smartbridge_bond_info_dct_t* dct;
    uint32_t bond_info_count = sizeof( dct->bond_info ) / sizeof( wiced_bt_smart_bond_info_t );
    uint32_t a;

    /* DCT API isn't thread-safe. Lock mutex */
    wiced_rtos_lock_mutex( &dct_mutex );

    wiced_dct_read_lock( (void**) &dct, WICED_FALSE, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    for ( a = 0; a < bond_info_count; a++ )
    {
        if ( ( memcmp( &( dct->bond_info[a].peer_address ), address, sizeof( *address ) ) == 0 ) && ( dct->bond_info[a].address_type == type ) )
        {
            memcpy( bond_info, &dct->bond_info[a], sizeof( *bond_info ) );

            wiced_dct_read_unlock( (void*) dct, WICED_FALSE );

            wiced_rtos_unlock_mutex( &dct_mutex );

            return WICED_SUCCESS;
        }
    }

    wiced_dct_read_unlock( (void*) dct, WICED_FALSE );

    wiced_rtos_unlock_mutex( &dct_mutex );

    return WICED_NOT_FOUND;
}

/* Store new Bond Info into DCT
 */
static wiced_result_t store_bond_info( const wiced_bt_smart_bond_info_t* bond_info )
{
    bt_smartbridge_bond_info_dct_t* bond_info_dct = NULL;
    uint32_t bond_info_count = sizeof( bt_smartbridge_bond_info_dct_t ) / sizeof( wiced_bt_smart_bond_info_t );
    uint32_t a;

    /* DCT API isn't thread-safe. Lock mutex */
    wiced_rtos_lock_mutex( &dct_mutex );

    /* Read DCT to local copy so it can be modified */
    wiced_dct_read_lock( (void**) &bond_info_dct, WICED_TRUE, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    /* Search DCT if device with address and address type specified is found. If found, update bond info */
    for ( a = 0; a < bond_info_count; a++ )
    {
        if ( ( memcmp( &bond_info_dct->bond_info[a].peer_address, &bond_info->peer_address, sizeof( bond_info->peer_address ) ) == 0 ) && ( bond_info_dct->bond_info[a].address_type == bond_info->address_type ) )
        {
            /* Device is found in the DCT. Update bond info */
            memcpy( &bond_info_dct->bond_info[a], bond_info, sizeof( *bond_info ) );

            /* Write updated bond info to DCT */
            wiced_dct_write( (const void*) bond_info_dct, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );
            wiced_dct_read_unlock( (void*) bond_info_dct, WICED_TRUE );

            /* Unlock mutex */
            wiced_rtos_unlock_mutex( &dct_mutex );

            return WICED_SUCCESS;
        }
    }

    /* Bond info for the device isn't found. Store at the next index */
    memcpy( &bond_info_dct->bond_info[bond_info_dct->current_index], bond_info, sizeof( *bond_info ) );

    /* Update current index */
    bond_info_dct->current_index++;
    bond_info_dct->current_index %= bond_info_count;

    /* Write updated bond info to DCT */
    wiced_dct_write( (const void*) bond_info_dct, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );
    wiced_dct_read_unlock( (void*) bond_info_dct, WICED_TRUE );

    wiced_rtos_unlock_mutex( &dct_mutex );

    return WICED_SUCCESS;
}

/* Convert hex string to uint8_t array
 */
static wiced_result_t convert_hex_str_to_uint8_t_array( const char* hex_string, uint8_t array[], uint32_t size )
{
    char* iterator = (char*)hex_string;
    int count = size - 1;

    while ( iterator != 0 && count >= 0 )
    {
        char buffer[3] = { 0 };
        char* end;

        buffer[0] = ( iterator != NULL ) ? *iterator : 0;
        iterator++;
        buffer[1] = ( iterator != NULL ) ? *iterator : 0;
        iterator++;

        array[count--] = strtoul( buffer, &end, 16 );
    }

    return WICED_SUCCESS;
}

/* Convert string to device address
 */
static wiced_result_t convert_str_to_device_address( const char* addr_string, wiced_bt_device_address_t* address )
{
    uint32_t i;
    uint32_t j;

    /* BD_ADDR string format:
     * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
     * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * |  X |  X |  : |  X |  X |  : |  X |  X |  : |  X |  X |  : |  X |  X |  : |  X |  X |
     * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     */

    /* BD_ADDR is passed within the URL. Parse it here */
    for ( i = 6, j = 0; i > 0; i--, j += 3 )
    {
        char  buffer[3] = {0};
        char* end;

        buffer[0] = addr_string[j];
        buffer[1] = addr_string[j + 1];
        address->address[i - 1] = strtoul( buffer, &end, 16 );
    }

    return WICED_SUCCESS;
}

int start_smartbridge( int argc, char *argv[] )
{
    uint32_t a;

    memset( &smartbridge_socket, 0, sizeof( smartbridge_socket ) );
    memset( &last_notification_handle, 0, sizeof( last_notification_handle ) );
    memset( &last_notification_time, 0, sizeof( last_notification_time ) );

    /* Initialise WICED Bluetooth Framework */
    wiced_bt_init( WICED_BT_HCI_MODE, "SmartBridge Device" );

    /* Initialise WICED SmartBridge */
    wiced_bt_smartbridge_init();

    wiced_bt_smartbridge_set_max_concurrent_connections( MAX_BT_SMART_CONNECTIONS );

    /* Initialise scan complete semaphore */
    wiced_rtos_init_semaphore( &scan_complete_semaphore );

    /* Initialise all sockets */
    for ( a = 0; a < MAX_BT_SMART_CONNECTIONS; a++)
    {
        wiced_bt_smartbridge_create_socket( &smartbridge_socket[a] );
    }

    return ERR_CMD_OK;
}

int stop_smartbridge( int argc, char *argv[] )
{
    uint32_t a;

    wiced_rtos_deinit_semaphore( &scan_complete_semaphore );

    /* Deinitialise all sockets */
    for ( a = 0; a < MAX_BT_SMART_CONNECTIONS; a++)
    {
        wiced_bt_smartbridge_delete_socket( &smartbridge_socket[a] );
    }

    /* Deinitialise SmartBridge */
    wiced_bt_smartbridge_deinit( );

    /* Deinitialise Bluetooth */
    wiced_bt_deinit( );

    return ERR_CMD_OK;
}

int auto_test_smartbridge( int argc, char *argv[] )
{
    uint32_t a;
    uint32_t iteration = atoi( argv[1] );
    wiced_bt_smart_scan_result_t* result;
    wiced_bt_smart_scan_result_t* temp;
    uint32_t b;
    uint32_t count;
    wiced_bool_t cached_enabled = WICED_FALSE;
    wiced_result_t retval;

    memset( &smartbridge_socket, 0, sizeof( smartbridge_socket ) );
    memset( &last_notification_handle, 0, sizeof( last_notification_handle ) );
    memset( &last_notification_time, 0, sizeof( last_notification_time ) );

    /* Initialise WICED Bluetooth Framework */
    wiced_bt_init( WICED_BT_HCI_MODE, "SmartBridge Device" );

    /* Initialise WICED SmartBridge */
    wiced_bt_smartbridge_init();

    /* Enable attribute caching */
    wiced_bt_smartbridge_enable_attribute_cache( MAX_BT_SMART_CONNECTIONS );

    /* Set maximum concurrent connections */
    wiced_bt_smartbridge_set_max_concurrent_connections( MAX_BT_SMART_CONNECTIONS );

    /* Initialise scan complete semaphore */
    wiced_rtos_init_semaphore( &scan_complete_semaphore );

    for ( a = 0; a < MAX_BT_SMART_CONNECTIONS; a++)
    {
        wiced_bt_smartbridge_create_socket( &smartbridge_socket[a] );
    }

    for ( b = 0; b < iteration; b++ )
    {
        WPRINT_APP_INFO( ( "Iteration %lu\n", (uint32_t)(b + 1) ) );

        scan_settings.duration_second = 3;

        wiced_bt_smartbridge_start_scan( &scan_settings, scan_complete_handler, scan_advertising_report_handler );

        wiced_rtos_get_semaphore( &scan_complete_semaphore, WICED_NEVER_TIMEOUT );

        wiced_bt_smartbridge_get_scan_result_list( &result, &count );

        temp = result;

        WPRINT_APP_INFO( ( "\n---------------------------------------------------\n" ) );
        WPRINT_APP_INFO( ( " Scan Report"  ) );
        WPRINT_APP_INFO( ( "\n---------------------------------------------------\n" ) );

        if ( count > 0 )
        {
            while ( temp != NULL )
            {
                WPRINT_LIB_INFO( ( "\n Device Name    : %s\n", temp->remote_device.name ) );

                WPRINT_LIB_INFO( ( " Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
                                   temp->remote_device.address.address[5],
                                   temp->remote_device.address.address[4],
                                   temp->remote_device.address.address[3],
                                   temp->remote_device.address.address[2],
                                   temp->remote_device.address.address[1],
                                   temp->remote_device.address.address[0] ) );

                WPRINT_LIB_INFO( ( " Address Type   : %lu\n", (uint32_t)temp->remote_device.address_type ) );

                /* Move result to next */
                temp = temp->next;
            }
        }
        else
        {
            WPRINT_APP_INFO( ( "No device found\n" ) );
        }

        WPRINT_APP_INFO( ( "\n---------------------------------------------------\n\n" ) );

        temp = result;

        wiced_rtos_delay_milliseconds( 1000 );

        /* Decide here test variations */
        if ( b % 5 == 0 )
        {
            if ( cached_enabled == WICED_FALSE )
            {
                wiced_bt_smartbridge_enable_attribute_cache( MAX_BT_SMART_CONNECTIONS );
                cached_enabled = WICED_TRUE;
            }
            else
            {
                wiced_bt_smartbridge_enable_attribute_cache( MAX_BT_SMART_CONNECTIONS );
                cached_enabled = WICED_FALSE;
            }
        }

        for ( a = 0; a < count && a < MAX_BT_SMART_CONNECTIONS; a++ )
        {
            wiced_bt_smart_bond_info_t bond_info;

            if ( b % 10 == 0 )
            {
                wiced_bt_smartbridge_enable_pairing( &smartbridge_socket[a], &security_settings, NULL, pairing_handler );
            }
            else
            {
                if ( find_bond_info( &temp->remote_device.address, temp->remote_device.address_type, &bond_info ) == WICED_SUCCESS )
                {
                    wiced_bt_smartbridge_set_bond_info( &smartbridge_socket[a], &security_settings, &bond_info );
                }
                else
                {
                    wiced_bt_smartbridge_enable_pairing( &smartbridge_socket[a], &security_settings, NULL, pairing_handler );
                }
            }

            WPRINT_APP_INFO( ( "Connecting to socket %lu\n", (uint32_t)a ) );

            if ( wiced_bt_smartbridge_connect( &smartbridge_socket[a], &temp->remote_device, &connection_settings, disconnection_handler, notification_handler ) == WICED_SUCCESS )
            {
                wiced_bt_smart_attribute_t attribute;

                WPRINT_APP_INFO( ( "Connecting socket %lu successful. Connection handle: %lu\n", a, (uint32_t)smartbridge_socket[a].connection_handle ) );

                wiced_rtos_delay_milliseconds( 1000 );

                retval = wiced_bt_smartbridge_get_attribute_cache_by_handle( &smartbridge_socket[a], 1, &attribute, 512 );

                if ( retval == WICED_SUCCESS )
                {
                    wiced_bt_smart_attribute_print( &attribute );
                }
                else if ( retval == WICED_PENDING )
                {
                    WPRINT_LIB_INFO(( "Busy discovering. Try again later\n" ));
                }
                else
                {
                    WPRINT_LIB_INFO(( "Cache not enabled\n" ));
                }

                WPRINT_APP_INFO( ( "There should be no notification for socket %lu for the next few seconds\n", a ) );

                wiced_rtos_delay_milliseconds( 2000 );

                WPRINT_APP_INFO( ( "Now enabling notification for socket %lu\n", a ) );

                wiced_bt_smartbridge_enable_attribute_cache_notification( &smartbridge_socket[a] );

                wiced_rtos_delay_milliseconds( 2000 );
            }
            else
            {
                WPRINT_APP_INFO( ( "Connecting socket %lu failed!\n", a ) );
            }

            temp = temp->next;
        }

        wiced_rtos_delay_milliseconds( 3000 );

        for ( a = 0; a < count && a < MAX_BT_SMART_CONNECTIONS; a++ )
        {
            wiced_bt_smartbridge_socket_status_t status;
            wiced_bt_smartbridge_get_socket_status( &smartbridge_socket[a], &status );

            if ( status == SMARTBRIDGE_SOCKET_CONNECTED )
            {
                wiced_bt_smartbridge_disable_attribute_cache_notification( &smartbridge_socket[a] );

                if ( wiced_bt_smartbridge_disconnect( &smartbridge_socket[a] ) == WICED_SUCCESS )
                {
                    WPRINT_APP_INFO( ( "Disconnecting socket %lu successful.\n", a ) );
                }
                else
                {
                    WPRINT_APP_INFO( ( "Disconnecting socket %lu failed!\n", a ) );
                }
            }
        }

        if ( cached_enabled == WICED_FALSE )
        {
            wiced_bt_smartbridge_disable_attribute_cache();
        }

        wiced_rtos_delay_milliseconds( 2000 );
    }

    wiced_bt_smartbridge_disable_attribute_cache();
    wiced_bt_smartbridge_deinit();
    wiced_bt_deinit();

    WPRINT_APP_INFO( ( "TEST COMPLETE\n" ) );
    return ERR_CMD_OK;
}

int connect( int argc, char *argv[] )
{
    uint32_t socket_number     = atoi( argv[1] );
    char*    bd_addr_str       = argv[2];
    uint32_t enable_encryption = atoi( argv[4] );
    char*    passkey           = argv[5];
    wiced_bt_smart_device_t remote_device;

    remote_device.address_type = atoi( argv[3] ) ;

    convert_str_to_device_address( bd_addr_str, &remote_device.address );

    if ( enable_encryption != 0 )
    {
        wiced_bt_smart_bond_info_t bond_info;

        if ( find_bond_info( &remote_device.address, remote_device.address_type, &bond_info ) == WICED_SUCCESS )
        {
            wiced_bt_smartbridge_set_bond_info( &smartbridge_socket[socket_number], &security_settings, &bond_info );
        }
        else
        {
            wiced_bt_smartbridge_enable_pairing( &smartbridge_socket[socket_number], &security_settings, (const char*)passkey, pairing_handler );
        }
    }
    else
    {
        wiced_bt_smartbridge_disable_pairing( &smartbridge_socket[socket_number] );
    }

    if ( wiced_bt_smartbridge_connect( &smartbridge_socket[socket_number], &remote_device, &connection_settings, disconnection_handler, notification_handler ) == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "Connecting socket %lu successful. Connection handle: %lu\n", socket_number, (uint32_t)smartbridge_socket[socket_number].connection_handle ) );
    }
    else
    {
        WPRINT_APP_INFO( ( "Connecting socket %lu failed!\n", socket_number ) );
    }

    return ERR_CMD_OK;
}

int scan_connect_direct_adv( int argc, char *argv[] )
{
    uint32_t socket_number     = atoi( argv[1] );
    uint32_t enable_encryption = atoi( argv[2] );

    directed_advertisement_received = WICED_FALSE;
    scan_settings.duration_second   = (uint16_t)atoi( argv[3] );
    scan_settings.filter_policy     = FILTER_POLICY_NONE;

    wiced_bt_smartbridge_start_scan( &scan_settings, scan_complete_handler, scan_advertising_report_handler );

    wiced_rtos_get_semaphore( &scan_complete_semaphore, WICED_NEVER_TIMEOUT );

    wiced_bt_smartbridge_stop_scan();

    if ( directed_advertisement_received == WICED_TRUE )
    {
        if ( enable_encryption != 0 )
        {
            wiced_bt_smart_bond_info_t bond_info;

            if ( find_bond_info( &directed_advertising_device.address, directed_advertising_device.address_type, &bond_info ) == WICED_SUCCESS )
            {
                wiced_bt_smartbridge_set_bond_info( &smartbridge_socket[socket_number], &security_settings, &bond_info );
            }
            else
            {
                wiced_bt_smartbridge_enable_pairing( &smartbridge_socket[socket_number], &security_settings, NULL, pairing_handler );
            }
        }
        else
        {
            wiced_bt_smartbridge_disable_pairing( &smartbridge_socket[socket_number] );
        }

        if ( wiced_bt_smartbridge_connect( &smartbridge_socket[socket_number], &directed_advertising_device, &connection_settings, disconnection_handler, notification_handler ) == WICED_SUCCESS )
        {
            WPRINT_APP_INFO( ( "Connecting socket %lu successful. Connection handle: %lu\n", socket_number, (uint32_t)smartbridge_socket[socket_number].connection_handle ) );
        }
        else
        {
            WPRINT_APP_INFO( ( "Connecting socket %lu failed!\n", socket_number ) );
        }
    }
    else
    {
        WPRINT_APP_INFO( ( "No directed advertisement received\n" ) );
    }

    directed_advertisement_received = WICED_FALSE;

    return ERR_CMD_OK;
}

int disconnect( int argc, char *argv[] )
{
    uint32_t socket_number = atoi( argv[1] );
    wiced_bt_smartbridge_socket_status_t status;

    wiced_bt_smartbridge_get_socket_status( &smartbridge_socket[socket_number], &status );

    if ( status != SMARTBRIDGE_SOCKET_DISCONNECTED )
    {
        if ( status == SMARTBRIDGE_SOCKET_CONNECTED )
        {
            wiced_bt_smartbridge_disable_attribute_cache_notification( &smartbridge_socket[socket_number] );
        }

        if ( wiced_bt_smartbridge_disconnect( &smartbridge_socket[socket_number] ) == WICED_SUCCESS )
        {
            WPRINT_APP_INFO( ( "Disconnecting socket %lu successful.\n", socket_number ) );
        }
        else
        {
            WPRINT_APP_INFO( ( "Disconnecting socket %lu failed!\n", socket_number ) );
        }

        last_notification_handle[socket_number] = 0;
        memset( &last_notification_time[socket_number], 0, sizeof(wiced_iso8601_time_string_t) );
    }

    return ERR_CMD_OK;
}

int scan( int argc, char *argv[] )
{
    wiced_bt_smart_scan_result_t* result;
    uint32_t count;

    scan_settings.duration_second = atoi( argv[1] );
    scan_settings.filter_policy   = atoi( argv[2] );

    wiced_bt_smartbridge_start_scan( &scan_settings, scan_complete_handler, scan_advertising_report_handler );

    wiced_rtos_get_semaphore( &scan_complete_semaphore, WICED_NEVER_TIMEOUT );

    wiced_bt_smartbridge_get_scan_result_list( &result, &count );

    WPRINT_APP_INFO( ( "\n---------------------------------------------------\n" ) );
    WPRINT_APP_INFO( ( " Scan Report"  ) );
    WPRINT_APP_INFO( ( "\n---------------------------------------------------\n" ) );

    if ( count > 0 )
    {
        while ( result != NULL )
        {
            WPRINT_LIB_INFO( ( "\n Device Name    : %s\n", result->remote_device.name ) );

            WPRINT_LIB_INFO( ( " Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
                               result->remote_device.address.address[5],
                               result->remote_device.address.address[4],
                               result->remote_device.address.address[3],
                               result->remote_device.address.address[2],
                               result->remote_device.address.address[1],
                               result->remote_device.address.address[0] ) );

            WPRINT_LIB_INFO( ( " Address Type   : %lu\n", (uint32_t)result->remote_device.address_type ) );

            /* Move result to next */
            result = result->next;
        }
    }
    else
    {
        WPRINT_APP_INFO( ( "No device found\n" ) );
    }

    WPRINT_APP_INFO( ( "\n---------------------------------------------------\n\n" ) );
    return ERR_CMD_OK;
}

int status( int argc, char *argv[] )
{
    uint8_t a;

    WPRINT_APP_INFO( ( "\n---------------------------------------------------\n" ) );
    WPRINT_APP_INFO( ( " Status Report"  ) );
    WPRINT_APP_INFO( ( "\n---------------------------------------------------\n" ) );

    for ( a = 0; a < MAX_BT_SMART_CONNECTIONS; a++ )
    {
        wiced_bt_smartbridge_socket_status_t status;

        wiced_bt_smartbridge_get_socket_status( &smartbridge_socket[a], &status );

        WPRINT_APP_INFO( ( "\n Socket %lu\n", (uint32_t)a ) );

        WPRINT_APP_INFO( ( " Connection status        : " ) );

        if ( status == SMARTBRIDGE_SOCKET_CONNECTED )
        {
            WPRINT_APP_INFO( ( "connected to %02X:%02X:%02X:%02X:%02X:%02X\n",
                                smartbridge_socket[a].remote_device.address.address[5],
                                smartbridge_socket[a].remote_device.address.address[4],
                                smartbridge_socket[a].remote_device.address.address[3],
                                smartbridge_socket[a].remote_device.address.address[2],
                                smartbridge_socket[a].remote_device.address.address[1],
                                smartbridge_socket[a].remote_device.address.address[0] ) );

            WPRINT_APP_INFO( ( " Connection handle        : %lu\n", (uint32_t)smartbridge_socket[a].connection_handle ) );
        }
        else
        {
            WPRINT_APP_INFO( ( "not connected\n" ) );
        }


        if ( last_notification_handle[a] != 0 )
        {
            WPRINT_APP_INFO( ( " Last notification handle : %lu\n", (uint32_t)last_notification_handle[a] ) );
            WPRINT_APP_INFO( ( " Last notification time   : %s\n", (char*)&last_notification_time[a].time ) );
        }

        WPRINT_APP_INFO( ( "\n" ) );
    }

    WPRINT_APP_INFO( ( "\n---------------------------------------------------\n\n" ) );
    return ERR_CMD_OK;
}

int clear_bond_info_dct( int argc, char *argv[] )
{
    bt_smartbridge_bond_info_dct_t* bond_info_dct;
    wiced_result_t result;

    /* DCT API isn't thread-safe. Lock mutex */
    wiced_rtos_lock_mutex( &dct_mutex );

    /* Read DCT to local copy so it can be modified */
    wiced_dct_read_lock( (void**) &bond_info_dct, WICED_TRUE, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    /* Reset DCT local copy to zeroes */
    memset( bond_info_dct, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    /* Write updated bond info to DCT */
    result = wiced_dct_write( (void*) bond_info_dct, DCT_APP_SECTION, 0, sizeof(bt_smartbridge_bond_info_dct_t) );

    wiced_dct_read_unlock( (void*)bond_info_dct, WICED_TRUE );

    wiced_rtos_unlock_mutex( &dct_mutex );

    if ( result == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int enable_attribute_cache( int argc, char *argv[] )
{
    uint32_t number_of_cache = atoi( argv[1] );

    if ( wiced_bt_smartbridge_enable_attribute_cache( number_of_cache ) == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int disable_attribute_cache( int argc, char *argv[] )
{
    if ( wiced_bt_smartbridge_disable_attribute_cache( ) == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int print_attribute_cache( int argc, char *argv[] )
{
    uint32_t socket_number = atoi( argv[1] );
    wiced_bt_smart_attribute_list_t* att_cache_list;
    wiced_result_t result;

    result = wiced_bt_smartbridge_get_attribute_cache_list( &smartbridge_socket[socket_number], &att_cache_list );

    if ( result == WICED_SUCCESS )
    {
        wiced_bt_smart_attribute_print_list( att_cache_list );
    }
    else if ( result == WICED_PENDING )
    {
        WPRINT_LIB_INFO(( "Busy discovering. Try again later\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Not enabled\n" ));
    }

    return ERR_CMD_OK;
}

int write_attribute_cache( int argc, char *argv[] )
{
    uint32_t socket_number = atoi( argv[1] );
    uint32_t handle        = atoi( argv[2] );
    char*    value_hex_str = (char*)argv[3];
    uint8_t  value_size    = strlen( value_hex_str ) / 2;
    wiced_bt_smart_attribute_t attribute;

    if ( wiced_bt_smartbridge_get_attribute_cache_by_handle( &smartbridge_socket[socket_number], handle, &attribute, sizeof( attribute.value ) ) != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Attribute not found\n" ));
    }

    attribute.handle       = handle;
    attribute.value_length = value_size;

    convert_hex_str_to_uint8_t_array( value_hex_str, (uint8_t*)&attribute.value, value_size );

    if ( wiced_bt_smartbridge_write_attribute_cache_characteristic_value( &smartbridge_socket[socket_number], &attribute ) == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int enable_notification( int argc, char *argv[] )
{
    uint32_t socket_number = atoi( argv[1] );

    if ( wiced_bt_smartbridge_enable_attribute_cache_notification( &smartbridge_socket[socket_number] ) == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int disable_notification( int argc, char *argv[] )
{
    uint32_t socket_number = atoi( argv[1] );

    if ( wiced_bt_smartbridge_disable_attribute_cache_notification( &smartbridge_socket[socket_number] ) == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int add_whitelist( int argc, char *argv[] )
{
    char*                         bd_addr_str  = (char*)argv[1];
    wiced_bt_smart_address_type_t address_type = (wiced_bt_smart_address_type_t)atoi( argv[2] );
    wiced_bt_device_address_t     device_address;

    convert_str_to_device_address( bd_addr_str, &device_address );

    if ( wiced_bt_smartbridge_add_device_to_whitelist( &device_address, address_type ) == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int remove_whitelist( int argc, char *argv[] )
{
    char*                         bd_addr_str  = (char*)argv[1];
    wiced_bt_smart_address_type_t address_type = (wiced_bt_smart_address_type_t)atoi( argv[2] );
    wiced_bt_device_address_t     device_address;

    convert_str_to_device_address( bd_addr_str, &device_address );

    if ( wiced_bt_smartbridge_remove_device_from_whitelist( &device_address, address_type ) == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int get_whitelist_size( int argc, char *argv[] )
{
    uint32_t size;

    wiced_bt_smartbridge_get_whitelist_size( &size );

    WPRINT_LIB_INFO( ( "Whitelist size is %lu\n", size ) );

    return ERR_CMD_OK;
}

int clear_whitelist( int argc, char *argv[] )
{
    wiced_bt_smartbridge_clear_whitelist( );
    return ERR_CMD_OK;
}

int discover_all_services( int argc, char *argv[] )
{
    uint32_t       socket_number = (uint32_t)atoi( argv[1] );
    wiced_result_t result;
    wiced_bt_smart_attribute_list_t list;

    memset( &list, 0, sizeof( list ) );

    result = wiced_bt_smartbridge_gatt_discover_all_primary_services( &smartbridge_socket[socket_number], &list );

    if ( result == WICED_SUCCESS )
    {
        /* Print list */
        wiced_bt_smart_attribute_print_list( &list );

        /* List is no longer used. Delete */
        wiced_bt_smart_attribute_delete_list( &list );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int discover_services_by_uuid( int argc, char *argv[] )
{
    uint32_t        socket_number = (uint32_t)atoi( argv[1] );
    char*           uuid_hex_str  = (char*)argv[2];
    uint8_t         uuid_size     = strlen( uuid_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_uuid_t uuid;
    wiced_bt_smart_attribute_list_t list;

    memset( &list, 0, sizeof( list ) );
    memset( &uuid, 0, sizeof( uuid ) );

    uuid.size = uuid_size;

    convert_hex_str_to_uint8_t_array( uuid_hex_str, (uint8_t*)&uuid.value, uuid.size );

    result = wiced_bt_smartbridge_gatt_discover_primary_services_by_uuid( &smartbridge_socket[socket_number], &uuid, &list );

    if ( result == WICED_SUCCESS )
    {
        /* Print list */
        wiced_bt_smart_attribute_print_list( &list );

        /* List is no longer used. Delete */
        wiced_bt_smart_attribute_delete_list( &list );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int find_included_services( int argc, char *argv[] )
{
    uint32_t       socket_number = (uint32_t)atoi( argv[1] );
    uint16_t       start_handle  = (uint16_t)atoi( argv[2] );
    uint16_t       end_handle    = (uint16_t)atoi( argv[3] );
    wiced_result_t result;
    wiced_bt_smart_attribute_list_t list;

    memset( &list, 0, sizeof( list ) );

    result = wiced_bt_smartbridge_gatt_find_included_services( &smartbridge_socket[socket_number], start_handle, end_handle, &list );

    if ( result == WICED_SUCCESS )
    {
        /* Print list */
        wiced_bt_smart_attribute_print_list( &list );

        /* List is no longer used. Delete */
        wiced_bt_smart_attribute_delete_list( &list );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int discover_all_chars_in_a_service( int argc, char *argv[] )
{
    uint32_t       socket_number = (uint32_t)atoi( argv[1] );
    uint16_t       start_handle  = (uint16_t)atoi( argv[2] );
    uint16_t       end_handle    = (uint16_t)atoi( argv[3] );
    wiced_result_t result;
    wiced_bt_smart_attribute_list_t list;

    memset( &list, 0, sizeof( list ) );

    result = wiced_bt_smartbridge_gatt_discover_all_characteristics_in_a_service( &smartbridge_socket[socket_number], start_handle, end_handle, &list );

    if ( result == WICED_SUCCESS )
    {
        /* Print list */
        wiced_bt_smart_attribute_print_list( &list );

        /* List is no longer used. Delete */
        wiced_bt_smart_attribute_delete_list( &list );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int discover_chars_by_uuid( int argc, char *argv[] )
{
    uint32_t        socket_number = (uint32_t)atoi( argv[1] );
    char*           uuid_hex_str  = (char*)argv[2];
    uint8_t         uuid_size     = strlen( uuid_hex_str ) / 2;
    uint16_t        start_handle  = (uint16_t)atoi( argv[3] );
    uint16_t        end_handle    = (uint16_t)atoi( argv[4] );
    wiced_result_t  result;
    wiced_bt_uuid_t uuid;
    wiced_bt_smart_attribute_list_t list;

    memset( &list, 0, sizeof( list ) );
    memset( &uuid, 0, sizeof( uuid ) );

    uuid.size = uuid_size;

    convert_hex_str_to_uint8_t_array( uuid_hex_str, (uint8_t*)&uuid.value, uuid.size );

    result = wiced_bt_smartbridge_gatt_discover_characteristic_by_uuid( &smartbridge_socket[socket_number], &uuid, start_handle, end_handle, &list );

    if ( result == WICED_SUCCESS )
    {
        /* Print list */
        wiced_bt_smart_attribute_print_list( &list );

        /* List is no longer used. Delete */
        wiced_bt_smart_attribute_delete_list( &list );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int discover_all_char_descriptors( int argc, char *argv[] )
{
    uint32_t       socket_number = (uint32_t)atoi( argv[1] );
    uint16_t       start_handle  = (uint16_t)atoi( argv[2] );
    uint16_t       end_handle    = (uint16_t)atoi( argv[3] );
    wiced_result_t result;
    wiced_bt_smart_attribute_list_t list;

    memset( &list, 0, sizeof( list ) );

    result = wiced_bt_smartbridge_gatt_discover_handle_and_type_of_all_characteristic_descriptors( &smartbridge_socket[socket_number], start_handle, end_handle, &list );

    if ( result == WICED_SUCCESS )
    {
        /* Print list */
        wiced_bt_smart_attribute_print_list( &list );

        /* List is no longer used. Delete */
        wiced_bt_smart_attribute_delete_list( &list );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int read_char_descriptor( int argc, char *argv[] )
{
    uint32_t        socket_number    = (uint32_t)atoi( argv[1] );
    uint16_t        attribute_handle = (uint32_t)atoi( argv[2] );
    char*           uuid_hex_str     = (char*)argv[3];
    uint8_t         uuid_size        = strlen( uuid_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_uuid_t uuid;
    wiced_bt_smart_attribute_t* descriptor;

    memset( &uuid, 0, sizeof( uuid ) );

    uuid.size = uuid_size;

    convert_hex_str_to_uint8_t_array( uuid_hex_str, (uint8_t*)&uuid.value, uuid.size );

    result = wiced_bt_smartbridge_gatt_read_characteristic_descriptor( &smartbridge_socket[socket_number], attribute_handle, &uuid, &descriptor );

    if ( result == WICED_SUCCESS )
    {
        /* Print attribute */
        wiced_bt_smart_attribute_print( descriptor );

        /* attribute is no longer used. Delete */
        wiced_bt_smart_attribute_delete( descriptor );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int read_long_char_descriptor( int argc, char *argv[] )
{
    uint32_t        socket_number    = (uint32_t)atoi( argv[1] );
    uint16_t        attribute_handle = (uint32_t)atoi( argv[2] );
    char*           uuid_hex_str     = (char*)argv[3];
    uint8_t         uuid_size        = strlen( uuid_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_uuid_t uuid;
    wiced_bt_smart_attribute_t* descriptor;

    memset( &uuid, 0, sizeof( uuid ) );

    uuid.size = uuid_size;

    convert_hex_str_to_uint8_t_array( uuid_hex_str, (uint8_t*)&uuid.value, uuid.size );

    result = wiced_bt_smartbridge_gatt_read_long_characteristic_descriptor( &smartbridge_socket[socket_number], attribute_handle, &uuid, &descriptor );

    if ( result == WICED_SUCCESS )
    {
        /* Print attribute */
        wiced_bt_smart_attribute_print( descriptor );

        /* attribute is no longer used. Delete */
        wiced_bt_smart_attribute_delete( descriptor );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int write_char_descriptor( int argc, char *argv[] )
{
    uint32_t        socket_number    = (uint32_t)atoi( argv[1] );
    uint16_t        attribute_handle = (uint32_t)atoi( argv[2] );
    char*           value_hex_str    = (char*)argv[3];
    uint32_t        value_length     = strlen( value_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_smart_attribute_t descriptor;

    memset( &descriptor, 0, sizeof( descriptor ) );

    descriptor.handle       = attribute_handle;
    descriptor.value_length = value_length;
    convert_hex_str_to_uint8_t_array( value_hex_str, (uint8_t*)&descriptor.value, value_length );

    result = wiced_bt_smartbridge_gatt_write_characteristic_descriptor( &smartbridge_socket[socket_number], &descriptor );

    if ( result == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int write_long_char_descriptor( int argc, char *argv[] )
{
    uint32_t        socket_number    = (uint32_t)atoi( argv[1] );
    uint16_t        attribute_handle = (uint32_t)atoi( argv[2] );
    char*           value_hex_str    = (char*)argv[3];
    uint32_t        value_length     = strlen( value_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_smart_attribute_t descriptor;

    memset( &descriptor, 0, sizeof( descriptor ) );

    descriptor.handle       = attribute_handle;
    descriptor.value_length = value_length;
    convert_hex_str_to_uint8_t_array( value_hex_str, (uint8_t*)&descriptor.value, value_length );

    result = wiced_bt_smartbridge_gatt_write_long_characteristic_descriptor( &smartbridge_socket[socket_number], &descriptor );

    if ( result == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int read_char_value( int argc, char *argv[] )
{
    uint32_t        socket_number    = (uint32_t)atoi( argv[1] );
    uint16_t        attribute_handle = (uint32_t)atoi( argv[2] );
    char*           uuid_hex_str     = (char*)argv[3];
    uint8_t         uuid_size        = strlen( uuid_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_uuid_t uuid;
    wiced_bt_smart_attribute_t* characteristic_value;

    memset( &uuid, 0, sizeof( uuid ) );

    uuid.size = uuid_size;

    convert_hex_str_to_uint8_t_array( uuid_hex_str, (uint8_t*)&uuid.value, uuid.size );

    result = wiced_bt_smartbridge_gatt_read_characteristic_value( &smartbridge_socket[socket_number], attribute_handle, &uuid, &characteristic_value );

    if ( result == WICED_SUCCESS )
    {
        /* Print attribute */
        wiced_bt_smart_attribute_print( characteristic_value );

        /* attribute is no longer used. Delete */
        wiced_bt_smart_attribute_delete( characteristic_value );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int read_long_char_value( int argc, char *argv[] )
{
    uint32_t        socket_number    = (uint32_t)atoi( argv[1] );
    uint16_t        attribute_handle = (uint32_t)atoi( argv[2] );
    char*           uuid_hex_str     = (char*)argv[3];
    uint8_t         uuid_size        = strlen( uuid_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_uuid_t uuid;
    wiced_bt_smart_attribute_t* characteristic_value;

    memset( &uuid, 0, sizeof( uuid ) );

    uuid.size = uuid_size;

    convert_hex_str_to_uint8_t_array( uuid_hex_str, (uint8_t*)&uuid.value, uuid.size );

    result = wiced_bt_smartbridge_gatt_read_long_characteristic_value( &smartbridge_socket[socket_number], attribute_handle, &uuid, &characteristic_value );

    if ( result == WICED_SUCCESS )
    {
        /* Print attribute */
        wiced_bt_smart_attribute_print( characteristic_value );

        /* attribute is no longer used. Delete */
        wiced_bt_smart_attribute_delete( characteristic_value );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int read_char_values_by_uuid( int argc, char *argv[] )
{
    uint32_t        socket_number = (uint32_t)atoi( argv[1] );
    char*           uuid_hex_str  = (char*)argv[2];
    uint8_t         uuid_size     = strlen( uuid_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_uuid_t uuid;
    wiced_bt_smart_attribute_list_t list;

    memset( &list, 0, sizeof( list ) );
    memset( &uuid, 0, sizeof( uuid ) );

    uuid.size = uuid_size;

    convert_hex_str_to_uint8_t_array( uuid_hex_str, (uint8_t*)&uuid.value, uuid.size );

    result = wiced_bt_smartbridge_gatt_read_characteristic_values_using_uuid( &smartbridge_socket[socket_number], &uuid, &list );

    if ( result == WICED_SUCCESS )
    {
        /* Print list */
        wiced_bt_smart_attribute_print_list( &list );

        /* List is no longer used. Delete */
        wiced_bt_smart_attribute_delete_list( &list );
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int write_char_value( int argc, char *argv[] )
{
    uint32_t        socket_number    = (uint32_t)atoi( argv[1] );
    uint16_t        attribute_handle = (uint32_t)atoi( argv[2] );
    char*           value_hex_str    = (char*)argv[3];
    uint32_t        value_length     = strlen( value_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_smart_attribute_t characteristic_value;

    memset( &characteristic_value, 0, sizeof( characteristic_value ) );

    characteristic_value.handle       = attribute_handle;
    characteristic_value.value_length = value_length;
    convert_hex_str_to_uint8_t_array( value_hex_str, (uint8_t*)&characteristic_value.value, value_length );

    result = wiced_bt_smartbridge_gatt_write_characteristic_value( &smartbridge_socket[socket_number], &characteristic_value );

    if ( result == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int write_long_char_value( int argc, char *argv[] )
{
    uint32_t        socket_number    = (uint32_t)atoi( argv[1] );
    uint16_t        attribute_handle = (uint32_t)atoi( argv[2] );
    char*           value_hex_str    = (char*)argv[3];
    uint32_t        value_length     = strlen( value_hex_str ) / 2;
    wiced_result_t  result;
    wiced_bt_smart_attribute_t characteristic_value;

    memset( &characteristic_value, 0, sizeof( characteristic_value ) );

    characteristic_value.handle       = attribute_handle;
    characteristic_value.value_length = value_length;
    convert_hex_str_to_uint8_t_array( value_hex_str, (uint8_t*)&characteristic_value.value, value_length );

    result = wiced_bt_smartbridge_gatt_write_long_characteristic_value( &smartbridge_socket[socket_number], &characteristic_value );

    if ( result == WICED_SUCCESS )
    {
        WPRINT_LIB_INFO(( "Successful\n" ));
    }
    else
    {
        WPRINT_LIB_INFO(( "Error\n" ));
    }

    return ERR_CMD_OK;
}

int dump_packet( int argc, char *argv[] )
{
    uint32_t enable = atoi( argv[1] );
    wiced_result_t result;

    if ( enable == 0 )
    {
        result = bt_transport_thread_disable_packet_dump( );
    }
    else
    {
        result = bt_transport_thread_enable_packet_dump( );
    }

    if ( result == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "Successful\n" ) );
    }
    else if ( result == WICED_UNSUPPORTED )
    {
        WPRINT_APP_INFO( ( "Packet dump unsupported!\n" ) );
    }
    else
    {
        WPRINT_APP_INFO( ( "Error\n" ) );
    }

    return ERR_CMD_OK;
}

int stack_trace( int argc, char *argv[] )
{
    uint32_t enable = atoi( argv[1] );
    wiced_result_t result;

    if ( enable == 0 )
    {
        result = bt_stack_disable_debug_trace( );
    }
    else
    {
        result = bt_stack_enable_debug_trace( );
    }

    if ( result == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "Successful\n" ) );
    }
    else if ( result == WICED_UNSUPPORTED )
    {
        WPRINT_APP_INFO( ( "Packet dump unsupported!\n" ) );
    }
    else
    {
        WPRINT_APP_INFO( ( "Error\n" ) );
    }

    return ERR_CMD_OK;
}

int set_tx_power( int argc, char *argv[] )
{
    uint32_t       socket_number = atoi( argv[1] );
    int8_t         tx_power_dbm  = atoi(argv[2] );
    wiced_result_t result;

    result = wiced_bt_smartbridge_set_transmit_power( &smartbridge_socket[socket_number], tx_power_dbm );

    if ( result == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "Successful\n" ) );
    }
    else if ( result == WICED_BADARG )
    {
        WPRINT_APP_INFO( ( "TX power is beyond regulatory limit\n" ) );
    }
    else
    {
        WPRINT_APP_INFO( ( "Error\n" ) );
    }

    return ERR_CMD_OK;
}
