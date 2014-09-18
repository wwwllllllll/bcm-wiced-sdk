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
#include "sntp.h"
#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define NTP_EPOCH            (86400U * (365U * 70U + 17U))
#define NTP_PORT             123

#ifndef WICED_NTP_REPLY_TIMEOUT
#define WICED_NTP_REPLY_TIMEOUT 300
#endif

#define MAX_NTP_ATTEMPTS     3


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/*
 * Taken from RFC 5905
 * http://www.ietf.org/rfc/rfc5905.txt
 */
typedef struct
{
    unsigned int mode : 3;
    unsigned int vn   : 3;
    unsigned int li   : 2;
    uint8_t      stratum;
    int8_t       poll;
    uint32_t     root_delay;
    uint32_t     root_dispersion;
    uint32_t     reference_identifier;
    uint32_t     reference_timestamp_seconds;
    uint32_t     reference_timestamp_fraction;
    uint32_t     origin_timestamp_seconds;
    uint32_t     origin_timestamp_fraction;
    uint32_t     receive_timestamp_seconds;
    uint32_t     receive_timestamp_fraction;
    uint32_t     transmit_timestamp_seconds;
    uint32_t     transmit_timestamp_fraction;
} ntp_packet_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static wiced_result_t sync_ntp_time( void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_timed_event_t sync_ntp_time_event;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t sntp_start_auto_time_sync( uint32_t interval_ms )
{
    wiced_result_t result;

    /* Synchronise time with NTP server and schedule for re-sync every one day */
    result = sync_ntp_time( NULL );
    wiced_rtos_register_timed_event( &sync_ntp_time_event, WICED_NETWORKING_WORKER_THREAD, sync_ntp_time, interval_ms, 0 );
    return result;
}

wiced_result_t sntp_stop_auto_time_sync( void )
{
    return wiced_rtos_deregister_timed_event( &sync_ntp_time_event );
}

wiced_result_t sntp_get_time( const wiced_ip_address_t* address, ntp_timestamp_t* timestamp)
{
    wiced_udp_socket_t socket;
    wiced_packet_t*    packet;
    ntp_packet_t*      data;
    uint16_t           data_length;
    uint16_t           available_data_length;
    wiced_utc_time_t   utc_time;
    wiced_result_t     result;

    /* Create the query packet */
    if ( wiced_packet_create_udp( &socket, sizeof(ntp_packet_t), &packet, (uint8_t**) &data, &available_data_length ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }

    /* Fill packet contents */
    wiced_time_get_utc_time( &utc_time );
    memset( data, 0, sizeof(ntp_packet_t) );
    data->li                         = 3;
    data->vn                         = 3;
    data->mode                       = 3;
    data->poll                       = 17;
    data->root_dispersion            = 0xfe030100;
    data->transmit_timestamp_seconds = htobe32( utc_time + NTP_EPOCH );
    wiced_packet_set_data_end( packet, (uint8_t*) data + sizeof(ntp_packet_t) );

    /* Create the UDP socket and send request */
    if ( wiced_udp_create_socket( &socket, NTP_PORT, WICED_STA_INTERFACE ) != WICED_SUCCESS )
    {
        wiced_packet_delete( packet );
        return WICED_ERROR;
    }
    if ( wiced_udp_send( &socket, address, NTP_PORT, packet ) != WICED_SUCCESS )
    {
        wiced_packet_delete( packet );
        wiced_udp_delete_socket( &socket );
        return WICED_ERROR;
    }

    /* Wait for reply */
    result = wiced_udp_receive(&socket, &packet, WICED_NTP_REPLY_TIMEOUT);

    /* Close the socket */
    wiced_udp_delete_socket( &socket );

    /* Process received time stamp */
    if (result != WICED_SUCCESS)
    {
        return result;
    }
    wiced_packet_get_data( packet, 0, (uint8_t**) &data, &data_length, &available_data_length );
    timestamp->seconds      = htobe32( data->transmit_timestamp_seconds ) - NTP_EPOCH;
    timestamp->microseconds = htobe32( data->transmit_timestamp_fraction ) / 4295; /* 4295 = 2^32 / 10^6 */
    wiced_packet_delete( packet );
    return WICED_SUCCESS;
}

static wiced_result_t sync_ntp_time( void* arg )
{
    ntp_timestamp_t      current_time;
    wiced_ip_address_t   ntp_server_ip;
    wiced_iso8601_time_t iso8601_time;
    uint32_t             a;

    UNUSED_PARAMETER( arg );

    /* Get the time */
    WPRINT_LIB_INFO( ( "Getting NTP time... ") );

    for ( a = 0; a < MAX_NTP_ATTEMPTS; ++a )
    {
        wiced_result_t result = WICED_ERROR;

        if ( wiced_hostname_lookup( "pool.ntp.org", &ntp_server_ip, 3 * SECONDS ) == WICED_SUCCESS )
        {
            result = sntp_get_time( &ntp_server_ip, &current_time );
        }

        if ( result == WICED_SUCCESS )
        {
            WPRINT_LIB_INFO( ( "success\n" ) );
            break;
        }
        else
        {
            WPRINT_LIB_INFO( ( "\nfailed, trying again...\n" ) );
        }
    }

    if ( a >= MAX_NTP_ATTEMPTS )
    {
        WPRINT_LIB_INFO( ("Give up getting NTP time\n") );
        memset( &current_time, 0, sizeof( current_time ) );
        return WICED_TIMEOUT;
    }
    else
    {
        wiced_utc_time_ms_t utc_time_ms = (uint64_t)current_time.seconds * (uint64_t)1000 + ( current_time.microseconds / 1000 );

        /* Set & Print the time */
        wiced_time_set_utc_time_ms( &utc_time_ms );
    }

    wiced_time_get_iso8601_time( &iso8601_time );
    WPRINT_LIB_INFO( ("Current time is: %.26s\n", (char*)&iso8601_time) );
    return WICED_SUCCESS;
}
