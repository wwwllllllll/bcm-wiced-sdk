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
 * A DNS redirector which responds to all DNS queries with the local address of the server.
 */

#include <string.h>
#include <stdint.h>
#include "wiced.h"
#include "dns_redirect.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define DNS_QUERY_TABLE_ENTRY(s)      {s, sizeof(s)}

/******************************************************
 *                    Constants
 ******************************************************/

/* DNS redirect thread constants*/
#define DNS_THREAD_PRIORITY                 (WICED_DEFAULT_LIBRARY_PRIORITY)
#define DNS_STACK_SIZE                      (1024)

#define DNS_FLAG_QR_MASK                    (0x01 << 15)

/* DNS opcodes */
#define DNS_FLAG_OPCODE_MASK         (0x0F << 11)

/* Generic DNS flag constants */
#define DNS_FLAG_YES            (1)
#define DNS_FLAG_NO             (0)

/* DNS flags */
#define DNS_FLAG_QR_QUERY                 (0 << 15)
#define DNS_FLAG_QR_RESPONSE              (1 << 15)
#define DNS_FLAG_OPCODE_SQUERY            (0 << 11) /* RFC 1035 */
#define DNS_FLAG_OPCODE_IQUERY            (1 << 11) /* RFC 1035, 3425 */
#define DNS_FLAG_OPCODE_STATUS            (2 << 11) /* RFC 1035 */
#define DNS_FLAG_OPCODE_NOTIFY            (4 << 11) /* RFC 1996 */
#define DNS_FLAG_OPCODE_UPDATE            (5 << 11) /* RFC 2136 */
#define DNS_FLAG_AUTHORITIVE_RECORD       (1 << 10)
#define DNS_FLAG_TRUNCATED                (1 << 9)
#define DNS_FLAG_DESIRE_RECURSION         (1 << 8)
#define DNS_FLAG_RECURSION_AVAILABLE      (1 << 7)
#define DNS_FLAG_AUTHENTICATED            (1 << 5)
#define DNS_FLAG_ACCEPT_NON_AUTHENTICATED (1 << 4)
#define DNS_FLAG_REPLY_CODE_NO_ERROR      (0 << 0)

/* DNS socket constants*/
#define DNS_UDP_PORT                        (53)

/* DNS socket timeout value in milliseconds. Modify this to make thread exiting more responsive */
#define DNS_SOCKET_TIMEOUT                  (500)

#define DNS_MAX_DOMAIN_LEN                  (255)
#define DNS_QUESTION_TYPE_CLASS_SIZE        (4)
#define DNS_IPV4_ADDRESS_SIZE               (4)

#define DNS_MAX_PACKET_SIZE      ( sizeof(dns_header_t) + DNS_MAX_DOMAIN_LEN + DNS_QUESTION_TYPE_CLASS_SIZE + sizeof(response_answer_array) + DNS_IPV4_ADDRESS_SIZE )
#define DNS_QUERY_TABLE_SIZE     ( sizeof(valid_dns_query_table)/sizeof(dns_query_table_entry_t) )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)

typedef struct
{
    uint16_t  transaction_id;
    uint16_t  flags;
    uint16_t  num_questions;
    uint16_t  num_answer_records;
    uint16_t  num_authority_records;
    uint16_t  num_additional_records;
} dns_header_t;

typedef struct
{
    uint8_t  name_offset_indicator;
    uint8_t  name_offset_value;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t data_length;
    /* Note: Actual IPv4 address needs to be inserted after this */
} dns_a_record_t;

typedef struct
{
    const char* query;
    uint8_t     length;
} dns_query_table_entry_t;

#pragma pack()


/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void dns_thread_main( uint32_t thread_input );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const dns_a_record_t a_record_template =
{
    .name_offset_indicator = 0xC0,    /* Indicates next value is a offset to a name, rather than the length of an immediately following segment */
    .name_offset_value     = 0x00,    /* Offset of cname name. To be calculated at runtime */
    .type                  = 0x0100,  /* Type A - host address */
    .class                 = 0x0100,  /* Internet Class */
    .ttl                   = 0x0,     /* Time to Live - 0 seconds - to avoid messing up proper internet DNS cache */
    .data_length           = 0x0400,  /* Data Length - 4 bytes for IPv4 address */
};

static const dns_query_table_entry_t valid_dns_query_table[] =
{
    DNS_QUERY_TABLE_ENTRY("\x0A""securedemo""\x05""wiced""\x08""broadcom""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x05""apple""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x03""www""\x05""apple""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x06""google""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x03""www""\x06""google""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x04""bing""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x03""www""\x04""bing""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x08""facebook""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x03""www""\x08""facebook""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x08""broadcom""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x03""www""\x08""broadcom""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x05""wiced""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x03""www""\x05""wiced""\x03""com"),
    DNS_QUERY_TABLE_ENTRY("\x08""clients3""\x06""google""\x03""com"),
};

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_dns_redirector_start( dns_redirector_t* dns_server, wiced_interface_t interface )
{
    /* Prepare network */
    if ( wiced_udp_create_socket(&dns_server->socket, DNS_UDP_PORT, interface) != WICED_SUCCESS)
    {
        return WICED_ERROR;
    }

    /* Create the DNS responder thread */
    dns_server->dns_quit_flag = 0;
    dns_server->interface     = interface;
    wiced_rtos_create_thread(&dns_server->dns_thread, DNS_THREAD_PRIORITY, "DNS thread", dns_thread_main, DNS_STACK_SIZE, dns_server);

    return WICED_SUCCESS;
}

wiced_result_t wiced_dns_redirector_stop( dns_redirector_t* server )
{
    server->dns_quit_flag = 1;
    if ( wiced_rtos_is_current_thread( &server->dns_thread ) != WICED_SUCCESS )
    {
        wiced_rtos_thread_force_awake( &server->dns_thread );
        wiced_rtos_thread_join       ( &server->dns_thread );
        wiced_rtos_delete_thread     ( &server->dns_thread );
    }
    return WICED_SUCCESS;
}

/**
 *  Implements a very simple DNS redirection server
 */
static void dns_thread_main( uint32_t thread_input )
{
    dns_redirector_t* server = (dns_redirector_t*)thread_input;
    uint16_t          data_length;
    uint16_t          available_length;
    wiced_packet_t*   packet;
    dns_header_t*     dns_header_ptr;

    /* Loop endlessly */
    while ( server->dns_quit_flag == 0 )
    {
        wiced_ip_address_t source_address;
        uint16_t           source_port;
        char*              loc_ptr;
        dns_a_record_t*    a_record;
        wiced_ip_address_t ipv4_address;

        /* Sleep until data is received from socket. */
        if ( wiced_udp_receive( &server->socket, &packet, WICED_WAIT_FOREVER ) != WICED_SUCCESS )
        {
            continue;
        }

        /* Extract out where it came from, and the actual data */
        wiced_udp_packet_get_info( packet, &source_address, &source_port );
        wiced_packet_get_data( packet, 0, (uint8_t**) &dns_header_ptr, &data_length, &available_length );

        /* Swap endian-ness of header */
        dns_header_ptr->flags         = htobe16( dns_header_ptr->flags );
        dns_header_ptr->num_questions = htobe16( dns_header_ptr->num_questions );

        /* Only respond to valid queries */
        if ( ( ( dns_header_ptr->flags & DNS_FLAG_QR_MASK )    != DNS_FLAG_QR_QUERY ) ||
             ( ( dns_header_ptr->flags & DNS_FLAG_OPCODE_MASK) != DNS_FLAG_OPCODE_SQUERY ) ||
               ( dns_header_ptr->num_questions < 1 ) )
        {
            wiced_packet_delete( packet );
            continue;
        }

        /* Create our reply using the same packet */
        /* Set the header values */
        dns_header_ptr->flags = DNS_FLAG_QR_RESPONSE        |
                                DNS_FLAG_OPCODE_SQUERY      |
                                DNS_FLAG_AUTHORITIVE_RECORD |
                                DNS_FLAG_AUTHENTICATED      |
                                DNS_FLAG_REPLY_CODE_NO_ERROR;

        dns_header_ptr->num_questions         = 1;
        dns_header_ptr->num_answer_records    = 1;
        dns_header_ptr->num_authority_records = 0;

        /* Find end of question */
        loc_ptr = (char *) &dns_header_ptr[ 1 ];

#ifdef DNS_REDIRECT_ALL_QUERIES
        /* Skip over domain name - made up of sections with one byte leading size values */
        while ( ( *loc_ptr != 0 ) &&
                ( *loc_ptr <= 64 ) )
        {
            loc_ptr += *loc_ptr + 1;
        }
        loc_ptr++; /* skip terminating null */
#else
        {
            uint8_t valid_query;
            for ( valid_query = 0; valid_query < DNS_QUERY_TABLE_SIZE; ++valid_query )
            {
                if ( memcmp( loc_ptr, valid_dns_query_table[valid_query].query, valid_dns_query_table[valid_query].length ) == 0)
                {
                    loc_ptr += valid_dns_query_table[valid_query].length;
                    break;
                }
            }
            if ( valid_query >= DNS_QUERY_TABLE_SIZE )
            {
                wiced_packet_delete( packet );
                continue;
            }
        }
#endif

        /* Check that domain name was not too large for packet - probably from an attack */
        if ( ( loc_ptr - (char*) dns_header_ptr ) > data_length )
        {
            wiced_packet_delete( packet );
            continue;
        }

        /* Move pointer to end of question */
        loc_ptr += DNS_QUESTION_TYPE_CLASS_SIZE;

        /* Query for this devices domain - reply with only local IP */
        dns_header_ptr->num_additional_records = 0;

        /* Copy the A record in */
        a_record = (dns_a_record_t*)loc_ptr;
        loc_ptr = MEMCAT(loc_ptr, &a_record_template, sizeof(a_record_template) );
        a_record->name_offset_value = 0x0C;

        /* Add our IP address */
        wiced_ip_get_ipv4_address(server->interface, &ipv4_address);
        *((uint32_t*)loc_ptr) = htobe32( GET_IPV4_ADDRESS(ipv4_address) );
        loc_ptr += DNS_IPV4_ADDRESS_SIZE;

        /* Swap the endian-ness of the header variables */
        dns_header_ptr->flags                  = htobe16( dns_header_ptr->flags );
        dns_header_ptr->num_questions          = htobe16( dns_header_ptr->num_questions );
        dns_header_ptr->num_answer_records     = htobe16( dns_header_ptr->num_answer_records );
        dns_header_ptr->num_authority_records  = htobe16( dns_header_ptr->num_authority_records );
        dns_header_ptr->num_additional_records = htobe16( dns_header_ptr->num_additional_records );

        /* Send packet */
        wiced_packet_set_data_end( packet, (uint8_t*) loc_ptr );
        wiced_udp_send( &server->socket, &source_address, source_port, packet );
    }

    /* Delete DNS socket */
    wiced_udp_delete_socket( &server->socket );
    WICED_END_OF_CURRENT_THREAD( );
}
