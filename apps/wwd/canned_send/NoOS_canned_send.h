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
 * Provides structures and macros for Canned Send example application
 */

#ifndef INCLUDED_NOOS_CANNED_SEND_H_
#define INCLUDED_NOOS_CANNED_SEND_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @cond */
/******************************************************
 *                     Macros
 ******************************************************/
#define MAKE_IPV4_ADDRESS(a, b, c, d)          ((((uint32_t) (d)) << 24) | (((uint32_t) (c)) << 16) | (((uint32_t) (b)) << 8) | ((uint32_t) (a)))
#define SWAP16( a )                            ((uint16_t)( ((((uint16_t)(a))&0xff)<<8) + ((((uint16_t)(a))&0xff00)>>8) ))

/******************************************************
 *                    Constants
 ******************************************************/
#define MIN_IOCTL_BUFFER_SIZE                  (120)
#define PACKET_SIZE                            (sizeof(udp_packet_t) + MAX_PAYLOAD)
#define ARP_OPERATION_REQUEST                  (1)
#define ARP_OPERATION_REPLY                    (2)
#define ARP_PROTOCOL_TYPE_IPV4                 (0x0800)
#define ARP_HARDWARE_TYPE_ETHERNET             (1)

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum
{
    ETHER_TYPE_IPv4  = 0x0800,
/*    ETHER_TYPE_ARP   = 0x0806, */
    ETHER_TYPE_IPv6  = 0x86DD,
} ether_type_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
#pragma pack(1)

typedef struct
{
    wiced_mac_t destination;
    wiced_mac_t source;
    uint16_t  ether_type;
} ethernet_header_t;


typedef struct
{
    uint16_t    hardware_type;
    uint16_t    protocol_type;
    uint8_t     hardware_address_length;
    uint8_t     protocol_address_length;
    uint16_t    operation;
    wiced_mac_t   sender_hardware_address;
    uint32_t    sender_protocol_address;
    wiced_mac_t   target_hardware_address;
    uint32_t    target_protocol_address;
} arp_message_t;

typedef struct
{
    uint8_t  header_length : 4;
    uint8_t  version      : 4;
    uint8_t  differentiated_services;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment_offset;
    uint8_t  time_to_live;
    uint8_t  protocol;
    uint16_t header_checksum;
    uint32_t source_address;
    uint32_t destination_address;
} ipv4_header_t;

typedef struct
{
    uint16_t  source_port;
    uint16_t  dest_port;
    uint16_t  udp_length;
    uint16_t  checksum;
} udp_header_t;


typedef struct
{
    char              reserved[WICED_LINK_OVERHEAD_BELOW_ETHERNET_FRAME];
    ethernet_header_t ethernet_header;
    arp_message_t     arp_message;
} arp_packet_t;


typedef struct
{
    char              reserved[WICED_LINK_OVERHEAD_BELOW_ETHERNET_FRAME];
    ethernet_header_t ethernet_header;
    ipv4_header_t     ip_header;
    udp_header_t      udp_header;
    char              data[1];
} udp_packet_t;

#pragma pack()

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *        Local Enumerations and Structures
 ******************************************************/


/** @endcond */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef INCLUDED_CANNED_SEND_H_ */
