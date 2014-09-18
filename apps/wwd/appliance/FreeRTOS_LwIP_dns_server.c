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
 * A DNS server which responds to all DNS queries with the local address of the server.
 */


#include "lwip/sockets.h"  /* equivalent of <sys/socket.h> */
#include <string.h>
#include <stdint.h>
#include "appliance.h"
#include "network/wwd_network_constants.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define DNS_STACK_SIZE                     (600)

#define DNS_FLAG_QR_POS                     (15)
#define DNS_FLAG_QR_SIZE                  (0x01)
#define DNS_FLAG_QR_QUERY                    (0)
#define DNS_FLAG_QR_RESPONSE                 (1)


/* DNS opcodes */
#define DNS_FLAG_OPCODE_POS                 (11)
#define DNS_FLAG_OPCODE_OPCODE_SIZE       (0x0f)
#define DNS_FLAG_OPCODE_SQUERY               (0) /* RFC 1035 */
#define DNS_FLAG_OPCODE_IQUERY               (1) /* RFC 1035, 3425 */
#define DNS_FLAG_OPCODE_STATUS               (2) /* RFC 1035 */
#define DNS_FLAG_OPCODE_NOTIFY               (4) /* RFC 1996 */
#define DNS_FLAG_OPCODE_UPDATE               (5) /* RFC 2136 */

#define DNS_FLAG_AUTHORITIVE_POS            (10)
#define DNS_FLAG_AUTHORITIVE_YES             (1)
#define DNS_FLAG_AUTHORITIVE_NO              (0)

#define DNS_FLAG_TRUNCATED_POS               (9)
#define DNS_FLAG_TRUNCATED_YES               (1)
#define DNS_FLAG_TRUNCATED_NO                (0)

#define DNS_FLAG_DESIRE_RECURS_POS           (8)
#define DNS_FLAG_DESIRE_RECURS_YES           (1)
#define DNS_FLAG_DESIRE_RECURS_NO            (0)

#define DNS_FLAG_RECURS_AVAIL_POS            (7)
#define DNS_FLAG_RECURS_AVAIL_YES            (1)
#define DNS_FLAG_RECURS_AVAIL_NO             (0)

#define DNS_FLAG_AUTHENTICATED_POS           (5)
#define DNS_FLAG_AUTHENTICATED_YES           (1)
#define DNS_FLAG_AUTHENTICATED_NO            (0)

#define DNS_FLAG_ACCEPT_NON_AUTHENT_POS      (4)
#define DNS_FLAG_ACCEPT_NON_AUTHENT_YES      (1)
#define DNS_FLAG_ACCEPT_NON_AUTHENT_NO       (0)

#define DNS_FLAG_REPLY_CODE_POS              (0)
#define DNS_FLAG_REPLY_CODE_NO_ERROR         (0)


#define DNS_UDP_PORT                        (53)


/* DNS socket timeout value in milliseconds. Modify this to make thread exiting more responsive */
#define DNS_SOCKET_TIMEOUT                 (500)

#define DNS_MAX_DOMAIN_LEN                 (255)
#define DNS_QUESTION_TYPE_CLASS_SIZE         (4)
#define DNS_IPV4_ADDRESS_SIZE                (4)

#define DNS_MAX_PACKET_SIZE   ( sizeof(dns_header_t) + DNS_MAX_DOMAIN_LEN + DNS_QUESTION_TYPE_CLASS_SIZE + sizeof(response_answer_array) + DNS_IPV4_ADDRESS_SIZE )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
/* DNS data structure */
typedef struct
{
    uint16_t  transaction_id;
    uint16_t  flags;
    uint16_t  num_questions;
    uint16_t  num_answer_records;
    uint16_t  num_authority_records;
    uint16_t  num_additional_records;

} dns_header_t;



/******************************************************
 *               Static Function Declarations
 ******************************************************/
static void dns_thread( void * thread_input );

/******************************************************
 *               Variable Definitions
 ******************************************************/
static const char response_answer_array[] =
{
        0xC0,                   /* Indicates next value is a offset to a name, rather than the length of an immediately following segment */
        0x0C,                   /* Offset of first question name */
        0x00, 0x01,             /* Type A - host address */
        0x00, 0x01,             /* Internet Class */
        0x00, 0x00, 0x00, 0x3C, /* Time to Live - 60 seconds - to avoid messing up proper internet DNS cache */
        0x00, 0x04,             /* Data Length - 4 bytes for IPv4 address */
};

static volatile char    dns_quit_flag = 0;
static int              dns_socket_handle            = -1;
static xTaskHandle      dns_thread_handle;
static char             dns_packet_buff[ DNS_MAX_PACKET_SIZE ];

/******************************************************
 *               Function Definitions
 ******************************************************/

void start_dns_server( uint32_t local_addr )
{
    xTaskCreate( dns_thread, (signed char*)"DNS thread", DNS_STACK_SIZE/sizeof( portSTACK_TYPE ), (void*)local_addr, DEFAULT_THREAD_PRIO, &dns_thread_handle);
}

void quit_dns_server( void )
{
    dns_quit_flag = 1;
}

/**
 *  Implements a very simple DNS server.
 *
 * @param my_addr : local IP address for binding of server port.
 */

static void dns_thread( void * thread_input )
{
    struct sockaddr_in    my_addr_store;
    struct sockaddr_in*   my_addr = &my_addr_store;
    struct sockaddr_in    source_addr;
    struct sockaddr_in    destination_addr;
    char * loc_ptr;
    int                   status;
    socklen_t             source_addr_len;
    static dns_header_t* dns_header_ptr = (dns_header_t*) &dns_packet_buff;
    int                   recv_timeout = DNS_SOCKET_TIMEOUT;

    my_addr->sin_family = AF_INET;
    my_addr->sin_addr.s_addr = (u32_t) thread_input;

    /* Add port number to IP address */
    my_addr->sin_port = htons( DNS_UDP_PORT );

    /* Create DNS socket */
    dns_socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt( dns_socket_handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&recv_timeout, sizeof( recv_timeout ) );

    /* Bind the socket to the local IP address */
    status = bind(dns_socket_handle, (struct sockaddr*)my_addr, sizeof(struct sockaddr_in));

    /* Set up the destination IP address and port number */
    destination_addr.sin_port   = htons( DNS_UDP_PORT );
    destination_addr.sin_family = AF_INET;
    memset( &destination_addr.sin_addr.s_addr, 0xff, 4 ); /* Broadcast */

    /* Loop endlessly */
    while ( dns_quit_flag == 0 )
    {
        /* Sleep until data is received from socket. */
        status = recvfrom( dns_socket_handle, dns_packet_buff, sizeof( dns_packet_buff ), 0 , (struct sockaddr *) &source_addr, &source_addr_len);
        if ( status <= 0 )
        {
            continue;
        }

        /* Fix endianness of header */
        dns_header_ptr->flags                  = ntohs( dns_header_ptr->flags );
        dns_header_ptr->num_questions          = ntohs( dns_header_ptr->num_questions );

        /* Only respond to queries */
        if ( DNS_FLAG_QR_QUERY != ( ( dns_header_ptr->flags >> DNS_FLAG_QR_POS ) & DNS_FLAG_QR_SIZE ) )
        {
            continue;
        }

        if ( DNS_FLAG_OPCODE_SQUERY != ( ( dns_header_ptr->flags >> DNS_FLAG_OPCODE_POS ) & DNS_FLAG_OPCODE_OPCODE_SIZE ) )
        {
            continue;
        }

        if ( dns_header_ptr->num_questions < 1 )
        {
            continue;
        }


        dns_header_ptr->flags = ( DNS_FLAG_QR_RESPONSE           << DNS_FLAG_QR_POS                 ) |
                                ( DNS_FLAG_OPCODE_SQUERY         << DNS_FLAG_OPCODE_POS             ) |
                                ( DNS_FLAG_AUTHORITIVE_YES       << DNS_FLAG_AUTHORITIVE_POS        ) |
                                ( DNS_FLAG_TRUNCATED_NO          << DNS_FLAG_TRUNCATED_POS          ) |
                                ( DNS_FLAG_DESIRE_RECURS_NO      << DNS_FLAG_DESIRE_RECURS_POS      ) |
                                ( DNS_FLAG_RECURS_AVAIL_NO       << DNS_FLAG_RECURS_AVAIL_POS       ) |
                                ( DNS_FLAG_AUTHENTICATED_YES     << DNS_FLAG_AUTHENTICATED_POS      ) |
                                ( DNS_FLAG_ACCEPT_NON_AUTHENT_NO << DNS_FLAG_ACCEPT_NON_AUTHENT_POS ) |
                                ( DNS_FLAG_REPLY_CODE_NO_ERROR   << DNS_FLAG_REPLY_CODE_POS         );


        dns_header_ptr->num_questions          = 1;
        dns_header_ptr->num_answer_records     = 1;
        dns_header_ptr->num_authority_records  = 0;
        dns_header_ptr->num_additional_records = 0;


        /* Find end of question */
        loc_ptr = &dns_packet_buff[ sizeof(dns_header_t) ];

        /* Skip over domain name - made up of sections with one byte leading size values */
        while ( ( *loc_ptr != 0 ) &&
                ( *loc_ptr <= 64 ) )
        {
            loc_ptr += *loc_ptr + 1;
        }
        loc_ptr++; /* skip terminating null */

        /* Check that domain name was not too large for packet - probably from an attack */
        if ( loc_ptr >  &dns_packet_buff[ sizeof(dns_header_t) + DNS_MAX_DOMAIN_LEN ] )
        {
            continue;
        }

        loc_ptr += DNS_QUESTION_TYPE_CLASS_SIZE;

        /* Copy the response data in */
        memcpy( loc_ptr, response_answer_array, sizeof(response_answer_array) );
        loc_ptr += sizeof(response_answer_array);

        /* Add our IP address */
        memcpy( loc_ptr, &my_addr->sin_addr.s_addr, DNS_IPV4_ADDRESS_SIZE );
        loc_ptr += DNS_IPV4_ADDRESS_SIZE;

        dns_header_ptr->flags                  = htons( dns_header_ptr->flags );
        dns_header_ptr->num_questions          = htons( dns_header_ptr->num_questions );
        dns_header_ptr->num_answer_records     = htons( dns_header_ptr->num_answer_records );
        dns_header_ptr->num_authority_records  = htons( dns_header_ptr->num_authority_records );
        dns_header_ptr->num_additional_records = htons( dns_header_ptr->num_additional_records );


        /* Send packet */
        sendto( dns_socket_handle, (char *)dns_header_ptr, (int)(loc_ptr - (char*)&dns_packet_buff), 0 , (struct sockaddr *) &source_addr, source_addr_len);

    }

    /* Delete DNS socket */
    lwip_close( dns_socket_handle );

    /* Clean up this startup thread */
    vTaskDelete( dns_thread_handle );
}


