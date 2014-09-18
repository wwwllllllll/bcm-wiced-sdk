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
#include "wiced.h"
#include "dhcp_server.h"
#include "wwd_network_constants.h"
#include "wiced_network.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define DHCP_IP_ADDRESS_CACHE_MAX       (5)

#define DHCP_THREAD_PRIORITY (WICED_DEFAULT_LIBRARY_PRIORITY)
#define DHCP_SERVER_RECEIVE_TIMEOUT (500)
/* BOOTP operations */
#define BOOTP_OP_REQUEST                (1)
#define BOOTP_OP_REPLY                  (2)

/* DHCP commands */
#define DHCPDISCOVER                    (1)
#define DHCPOFFER                       (2)
#define DHCPREQUEST                     (3)
#define DHCPDECLINE                     (4)
#define DHCPACK                         (5)
#define DHCPNAK                         (6)
#define DHCPRELEASE                     (7)
#define DHCPINFORM                      (8)

/* UDP port numbers for DHCP server and client */
#define IPPORT_DHCPS                   (67)
#define IPPORT_DHCPC                   (68)

static const uint8_t mtu_option_buff[]             = { 26, 2, WICED_PAYLOAD_MTU>>8, WICED_PAYLOAD_MTU&0xff };
static const uint8_t dhcp_offer_option_buff[]      = { 53, 1, DHCPOFFER };
static const uint8_t dhcp_ack_option_buff[]        = { 53, 1, DHCPACK };
static const uint8_t dhcp_nak_option_buff[]        = { 53, 1, DHCPNAK };
static const uint8_t lease_time_option_buff[]      = { 51, 4, 0x00, 0x01, 0x51, 0x80 }; /* 1 day lease */
static const uint8_t dhcp_magic_cookie[]           = { 0x63, 0x82, 0x53, 0x63 };

#define WPAD_SAMPLE_URL "http://xxx.xxx.xxx.xxx/wpad.dat"

static const wiced_mac_t const empty_cache = { .octet = {0} };
static wiced_mac_t             cached_mac_addresses[DHCP_IP_ADDRESS_CACHE_MAX];
static wiced_ip_address_t      cached_ip_addresses [DHCP_IP_ADDRESS_CACHE_MAX];

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/* DHCP data structure */
typedef struct
{
    uint8_t  opcode;                     /* packet opcode type */
    uint8_t  hardware_type;              /* hardware addr type */
    uint8_t  hardware_addr_len;          /* hardware addr length */
    uint8_t  hops;                       /* gateway hops */
    uint32_t transaction_id;             /* transaction ID */
    uint16_t second_elapsed;             /* seconds since boot began */
    uint16_t flags;
    uint8_t  client_ip_addr[4];          /* client IP address */
    uint8_t  your_ip_addr[4];            /* 'your' IP address */
    uint8_t  server_ip_addr[4];          /* server IP address */
    uint8_t  gateway_ip_addr[4];         /* gateway IP address */
    uint8_t  client_hardware_addr[16];   /* client hardware address */
    uint8_t  legacy[192];
    uint8_t  magic[4];
    uint8_t  options[275];               /* options area */
    /* as of RFC2131 it is variable length */
} dhcp_header_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void           dhcp_thread                      ( uint32_t thread_input );
static uint8_t*       find_option                      ( dhcp_header_t* request, uint8_t option_num );
static wiced_result_t get_client_ip_address_from_cache ( const wiced_mac_t* client_mac_address, wiced_ip_address_t* client_ip_address );
static wiced_result_t add_client_to_cache              ( const wiced_mac_t* client_mac_address, const wiced_ip_address_t* client_ip_address );
static void           ipv4_to_string                   ( char* buffer, uint32_t ipv4_address );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_start_dhcp_server(wiced_dhcp_server_t* server, wiced_interface_t interface)
{
    server->interface = interface;

    /* Clear cache */
    memset( cached_mac_addresses, 0, sizeof( cached_mac_addresses ) );
    memset( cached_ip_addresses,  0, sizeof( cached_ip_addresses  ) );

    /* Create DHCP socket */
    WICED_VERIFY(wiced_udp_create_socket(&server->socket, IPPORT_DHCPS, interface));

    /* Start DHCP server */
    wiced_rtos_create_thread(&server->thread, DHCP_THREAD_PRIORITY, "DHCPserver", dhcp_thread, DHCP_STACK_SIZE, server);
    return WICED_SUCCESS;
}

wiced_result_t wiced_stop_dhcp_server(wiced_dhcp_server_t* server)
{
    server->quit = WICED_TRUE;
    if ( wiced_rtos_is_current_thread( &server->thread ) != WICED_SUCCESS )
    {
        wiced_rtos_thread_force_awake( &server->thread );
        wiced_rtos_thread_join( &server->thread );
        wiced_rtos_delete_thread( &server->thread );
    }
    return WICED_SUCCESS;
}

/**
 *  Implements a very simple DHCP server.
 *
 *  Server will always offer next available address to a DISCOVER command
 *  Server will NAK any REQUEST command which is not requesting the next available address
 *  Server will ACK any REQUEST command which is for the next available address, and then increment the next available address
 *
 * @param my_addr : local IP address for binding of server port.
 */
static void dhcp_thread( uint32_t thread_input )
{
    wiced_packet_t*      received_packet;
    wiced_packet_t*      transmit_packet;
    wiced_ip_address_t   local_ip_address;
    wiced_ip_address_t   netmask;
    uint32_t             next_available_ip_addr;
    uint32_t             ip_mask;
    uint32_t             subnet;
    uint32_t             netmask_htobe;
    char*                option_ptr;
    wiced_dhcp_server_t* server                       = (wiced_dhcp_server_t*)thread_input;
    uint8_t              subnet_option_buff[]         = { 1, 4, 0, 0, 0, 0 };
    uint8_t              server_ip_addr_option_buff[] = { 54, 4, 0, 0, 0, 0 };
    uint32_t*            server_ip_addr_ptr           = (uint32_t*)&server_ip_addr_option_buff[2];
    uint8_t              wpad_option_buff[ 2 + sizeof(WPAD_SAMPLE_URL)-1 ] = { 252, sizeof(WPAD_SAMPLE_URL)-1 };

    /* Save local IP address to be sent in DHCP packets */
    wiced_ip_get_ipv4_address(server->interface, &local_ip_address);
    *server_ip_addr_ptr = htobe32( GET_IPV4_ADDRESS( local_ip_address ) );

    wiced_ip_get_netmask(server->interface, &netmask);
    ip_mask = ~(GET_IPV4_ADDRESS(netmask));

    netmask_htobe = htobe32( GET_IPV4_ADDRESS(netmask) );
    memcpy(&subnet_option_buff[2], &netmask_htobe, 4);

    subnet = GET_IPV4_ADDRESS(local_ip_address) & GET_IPV4_ADDRESS(netmask);
    next_available_ip_addr = subnet | ((GET_IPV4_ADDRESS(local_ip_address) + 1) & ip_mask);

    /* Prepare wpad_option_buff */
    memcpy(&wpad_option_buff[2], WPAD_SAMPLE_URL, sizeof(WPAD_SAMPLE_URL)-1);
    ipv4_to_string( (char*)&wpad_option_buff[2 + 7], *server_ip_addr_ptr);

    server->quit = WICED_FALSE;

    /* Loop endlessly */
    while ( server->quit == WICED_FALSE )
    {
        uint16_t       data_length;
        uint16_t       available_data_length;
        dhcp_header_t* request_header;

        /* Sleep until data is received from socket. */
        if ( wiced_udp_receive( &server->socket, &received_packet, WICED_WAIT_FOREVER ) != WICED_SUCCESS )
        {
            continue;
        }

        wiced_packet_get_data( received_packet, 0, (uint8_t**) &request_header, &data_length, &available_data_length );

        /* Check DHCP command */
        switch ( request_header->options[2] )
        {
            case DHCPDISCOVER:
            {
                dhcp_header_t*     reply_header;
                uint16_t           available_space;
                wiced_mac_t        client_mac_address;
                wiced_ip_address_t client_ip_address;
                uint32_t           temp;

                /* Create reply and free received packet */
                if ( wiced_packet_create_udp( &server->socket, sizeof(dhcp_header_t), &transmit_packet, (uint8_t**) &reply_header, &available_space ) != WICED_SUCCESS )
                {
                    wiced_packet_delete( received_packet );
                    break;
                }

                memcpy(reply_header, request_header, sizeof(dhcp_header_t) - sizeof(reply_header->options));
                wiced_packet_delete( received_packet );

                /* Discover command - send back OFFER response */
                reply_header->opcode = BOOTP_OP_REPLY;

                /* Clear the DHCP options list */
                memset( &reply_header->options, 0, sizeof( reply_header->options ) );

                /* Record client MAC address */
                memcpy( &client_mac_address, request_header->client_hardware_addr, sizeof( client_mac_address ) );

                /* Check whether device is already cached */
                if ( get_client_ip_address_from_cache( &client_mac_address, &client_ip_address ) != WICED_SUCCESS )
                {
                    /* Address not found in cache. Use next available IP address */
                    client_ip_address.version = WICED_IPV4;
                    client_ip_address.ip.v4   = next_available_ip_addr;
                }

                /* Create the IP address for the Offer */
                temp = htonl(client_ip_address.ip.v4);
                memcpy( reply_header->your_ip_addr, &temp, sizeof( temp ) );

                /* Copy the magic DHCP number */
                memcpy( reply_header->magic, dhcp_magic_cookie, 4 );

                /* Add options */
                option_ptr     = (char *) &reply_header->options;
                option_ptr     = MEMCAT( option_ptr, dhcp_offer_option_buff, 3 );                                   /* DHCP message type            */
                option_ptr     = MEMCAT( option_ptr, server_ip_addr_option_buff, 6 );                               /* Server identifier            */
                option_ptr     = MEMCAT( option_ptr, lease_time_option_buff, 6 );                                   /* Lease Time                   */
                option_ptr     = MEMCAT( option_ptr, subnet_option_buff, 6 );                                       /* Subnet Mask                  */
                option_ptr     = (char*)MEMCAT( option_ptr, wpad_option_buff, sizeof(wpad_option_buff) );           /* Web proxy auto discovery URL */
                memcpy( option_ptr, server_ip_addr_option_buff, 6 );                                                /* Router (gateway)             */
                option_ptr[0]  = 3;                                                                                 /* Router id                    */
                option_ptr    += 6;
                memcpy( option_ptr, server_ip_addr_option_buff, 6 );                                                /* DNS server                   */
                option_ptr[0]  = 6;                                                                                 /* DNS server id                */
                option_ptr    += 6;
                option_ptr     = MEMCAT( option_ptr, mtu_option_buff, 4 );                                          /* Interface MTU                */
                option_ptr[0]  = (char) 0xff;                                                                       /* end options                  */
                option_ptr++;

                /* Send packet */
                wiced_packet_set_data_end( transmit_packet, (uint8_t*) option_ptr );
                if ( wiced_udp_send( &server->socket, WICED_IP_BROADCAST, IPPORT_DHCPC, transmit_packet ) != WICED_SUCCESS )
                {
                    wiced_packet_delete( transmit_packet );
                }
            }
                break;

            case DHCPREQUEST:
            {
                /* REQUEST command - send back ACK or NAK */
                uint32_t           temp;
                uint32_t*          server_id_req;
                dhcp_header_t*     reply_header;
                uint16_t           available_space;
                wiced_mac_t        client_mac_address;
                wiced_ip_address_t given_ip_address;
                wiced_ip_address_t requested_ip_address;
                wiced_bool_t       next_avail_ip_address_used = WICED_FALSE;

                /* Check that the REQUEST is for this server */
                server_id_req = (uint32_t*) find_option( request_header, 54 );
                if ( ( server_id_req != NULL ) && ( GET_IPV4_ADDRESS( local_ip_address ) != htobe32(*server_id_req) ) )
                {
                    break; /* Server ID does not match local IP address */
                }

                /* Create reply and free received packet */
                if ( wiced_packet_create_udp( &server->socket, sizeof(dhcp_header_t), &transmit_packet, (uint8_t**) &reply_header, &available_space ) != WICED_SUCCESS )
                {
                    wiced_packet_delete( received_packet );
                    break;
                }

                memcpy(reply_header, request_header, sizeof(dhcp_header_t) - sizeof(reply_header->options));

                /* Record client MAC address */
                memcpy( &client_mac_address, request_header->client_hardware_addr, sizeof( client_mac_address ) );

                /* Locate the requested address in the options and keep requested address */
                requested_ip_address.version = WICED_IPV4;
                requested_ip_address.ip.v4   = ntohl(*(uint32_t*)find_option( request_header, 50 ));

                /* Delete received packet. We don't need it anymore */
                wiced_packet_delete( received_packet );

                reply_header->opcode = BOOTP_OP_REPLY;

                /* Blank options list */
                memset( &reply_header->options, 0, sizeof( reply_header->options ) );

                /* Copy DHCP magic number into packet */
                memcpy( reply_header->magic, dhcp_magic_cookie, 4 );

                option_ptr = (char *) &reply_header->options;

                /* Check if device is cache. If it is, give the previous IP address. Otherwise give the next available IP address */
                if ( get_client_ip_address_from_cache( &client_mac_address, &given_ip_address ) != WICED_SUCCESS )
                {
                    /* Address not found in cache. Use next available IP address */
                    next_avail_ip_address_used = WICED_TRUE;
                    given_ip_address.version   = WICED_IPV4;
                    given_ip_address.ip.v4     = next_available_ip_addr;
                }

                if ( memcmp( &requested_ip_address.ip.v4, &given_ip_address.ip.v4, sizeof( requested_ip_address.ip.v4 ) ) != 0 )
                {
                    /* Request is not for next available IP - force client to take next available IP by sending NAK */
                    /* Add appropriate options */
                    option_ptr = (char*)MEMCAT( option_ptr, dhcp_nak_option_buff, 3 );             /* DHCP message type */
                    option_ptr = (char*)MEMCAT( option_ptr, server_ip_addr_option_buff, 6 );       /* Server identifier */
                    memset( reply_header->your_ip_addr, 0, sizeof( reply_header->your_ip_addr ) ); /* Clear IP addr     */
                }
                else
                {
                    /* Request is for next available IP */
                    /* Add appropriate options */
                    option_ptr     = (char*)MEMCAT( option_ptr, dhcp_ack_option_buff, 3 );                              /* DHCP message type            */
                    option_ptr     = (char*)MEMCAT( option_ptr, server_ip_addr_option_buff, 6 );                        /* Server identifier            */
                    option_ptr     = (char*)MEMCAT( option_ptr, lease_time_option_buff, 6 );                            /* Lease Time                   */
                    option_ptr     = (char*)MEMCAT( option_ptr, subnet_option_buff, 6 );                                /* Subnet Mask                  */
                    option_ptr     = (char*)MEMCAT( option_ptr, wpad_option_buff, sizeof(wpad_option_buff) );           /* Web proxy auto discovery URL */
                    memcpy( option_ptr, server_ip_addr_option_buff, 6 );                                                /* Router (gateway)             */
                    option_ptr[0]  = 3;                                                                                 /* Router id                    */
                    option_ptr    += 6;
                    memcpy( option_ptr, server_ip_addr_option_buff, 6 );                                                /* DNS server                   */
                    option_ptr[0]  = 6;                                                                                 /* DNS server id                */
                    option_ptr    += 6;
                    option_ptr     = (char*)MEMCAT( option_ptr, mtu_option_buff, 4 );                                   /* Interface MTU                */

                    /* Create the IP address for the Offer */
                    temp = htonl(given_ip_address.ip.v4);
                    memcpy( reply_header->your_ip_addr, &temp, sizeof( temp ) );

                    /* Increment next available IP address only if not found in cache */
                    if ( next_avail_ip_address_used == WICED_TRUE )
                    {
                        do
                        {
                            next_available_ip_addr = subnet | ( ( next_available_ip_addr + 1 ) & ip_mask );
                        } while ( next_available_ip_addr == GET_IPV4_ADDRESS(local_ip_address) );
                    }

                    /* Cache client */
                    add_client_to_cache( &client_mac_address, &given_ip_address );

                }

                option_ptr[0] = (char) 0xff; /* end options */
                option_ptr++;

                /* Send packet */
                wiced_packet_set_data_end( transmit_packet, (uint8_t*) option_ptr );
                if ( wiced_udp_send( &server->socket, WICED_IP_BROADCAST, IPPORT_DHCPC, transmit_packet ) != WICED_SUCCESS )
                {
                    wiced_packet_delete( transmit_packet );
                }

            }
                break;

            default:
                wiced_packet_delete(received_packet);
                break;
        }
    }

    /* Delete DHCP socket */
    wiced_udp_delete_socket( &server->socket );
    WICED_END_OF_CURRENT_THREAD( );
}

/**
 *  Finds a specified DHCP option
 *
 *  Searches the given DHCP request and returns a pointer to the
 *  specified DHCP option data, or NULL if not found
 *
 * @param request :    The DHCP request structure
 * @param option_num : Which DHCP option number to find
 *
 * @return Pointer to the DHCP option data, or NULL if not found
 */

static uint8_t* find_option( dhcp_header_t* request, uint8_t option_num )
{
    uint8_t* option_ptr = request->options;
    while ( ( option_ptr[0] != 0xff ) &&
            ( option_ptr[0] != option_num ) &&
            ( option_ptr < ( (unsigned char*) request ) + sizeof( dhcp_header_t ) ) )
    {
        option_ptr += option_ptr[1] + 2;
    }
    if ( option_ptr[0] == option_num )
    {
        return &option_ptr[2];
    }
    return NULL;

}

static wiced_result_t get_client_ip_address_from_cache( const wiced_mac_t* client_mac_address, wiced_ip_address_t* client_ip_address )
{
    uint32_t a;

    /* Check whether device is already cached */
    for ( a = 0; a < DHCP_IP_ADDRESS_CACHE_MAX; a++ )
    {
        if ( memcmp( &cached_mac_addresses[ a ], client_mac_address, sizeof( *client_mac_address ) ) == 0 )
        {
            *client_ip_address = cached_ip_addresses[ a ];
            return WICED_SUCCESS;
        }
    }

    return WICED_NOT_FOUND;
}

static wiced_result_t add_client_to_cache( const wiced_mac_t* client_mac_address, const wiced_ip_address_t* client_ip_address )
{
    uint32_t a;
    uint32_t first_empty_slot;
    uint32_t cached_slot;

    /* Search for empty slot in cache */
    for ( a = 0, first_empty_slot = DHCP_IP_ADDRESS_CACHE_MAX, cached_slot = DHCP_IP_ADDRESS_CACHE_MAX; a < DHCP_IP_ADDRESS_CACHE_MAX; a++ )
    {
        /* Check for matching MAC address */
        if ( memcmp( &cached_mac_addresses[ a ], client_mac_address, sizeof( *client_mac_address ) ) == 0 )
        {
            /* Cached device found */
            cached_slot = a;
            break;
        }
        else if ( first_empty_slot == DHCP_IP_ADDRESS_CACHE_MAX && memcmp( &cached_mac_addresses[ a ], &empty_cache, sizeof(wiced_mac_t) ) == 0 )
        {
            /* Device not found in cache. Return the first empty slot */
            first_empty_slot = a;
        }
    }

    if ( cached_slot != DHCP_IP_ADDRESS_CACHE_MAX )
    {
        /* Update IP address of cached device */
        cached_ip_addresses[cached_slot] = *client_ip_address;
    }
    else if ( first_empty_slot != DHCP_IP_ADDRESS_CACHE_MAX )
    {
        /* Add device to the first empty slot */
        cached_mac_addresses[ first_empty_slot ] = *client_mac_address;
        cached_ip_addresses [ first_empty_slot ] = *client_ip_address;
    }
    else
    {
        /* Cache is full. Add device to slot 0 */
        cached_mac_addresses[ 0 ] = *client_mac_address;
        cached_ip_addresses [ 0 ] = *client_ip_address;
    }

    return WICED_SUCCESS;
}

static void ipv4_to_string( char* buffer, uint32_t ipv4_address )
{
    uint8_t* ip = (uint8_t*)&ipv4_address;
    utoa(ip[0], &buffer[0], 3, 3);
    buffer[3] = '.';
    utoa(ip[1], &buffer[4], 3, 3);
    buffer[7] = '.';
    utoa(ip[2], &buffer[8], 3, 3);
    buffer[11] = '.';
    utoa(ip[3], &buffer[12], 3, 3);
}
