/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "wiced_utilities.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef BT_HCI_COMMAND_PACKET_COUNT
#define BT_HCI_COMMAND_PACKET_COUNT (4)
#endif /* BT_HCI_COMMAND_PACKET_COUNT */

#ifndef BT_HCI_EVENT_PACKET_COUNT
#define BT_HCI_EVENT_PACKET_COUNT   (4)
#endif /* HCI_EVENT_PACKET_COUNT */

#ifndef BT_HCI_ACL_PACKET_COUNT
#define BT_HCI_ACL_PACKET_COUNT     (8)
#endif /* BT_HCI_ACL_PACKET_COUNT */

#define BT_HCI_COMMAND_DATA_SIZE    (256)    /* Maximum HCI command parameters total length */
#define BT_HCI_EVENT_DATA_SIZE      (256)    /* Maximum HCI event parameters total length   */

#ifndef BT_HCI_ACL_DATA_SIZE
#define BT_HCI_ACL_DATA_SIZE        (23 + 4) /* ATT MTU size + L2CAP header size            */
#endif /* BT_HCI_ACL_DATA_SIZE */

#define BT_HCI_COMMAND_HEADER_SIZE  ( sizeof( hci_command_header_t ) )
#define BT_HCI_EVENT_HEADER_SIZE    ( sizeof( hci_event_header_t) )
#define BT_HCI_ACL_HEADER_SIZE      ( sizeof( hci_acl_packet_header_t ) )

/******************************************************
 *                   Enumerations
 ******************************************************/

/* HCI Transport Layer Packet Type */
typedef enum
{
    HCI_COMMAND_PACKET  = 0x01, // HCI Command packet from Host to Controller
    HCI_ACL_DATA_PACKET = 0x02, // Bidirectional Asynchronous Connection-Less Link (ACL) data packet
    HCI_SCO_DATA_PACKET = 0x03, // Bidirectional Synchronous Connection-Oriented (SCO) link data packet
    HCI_EVENT_PACKET    = 0x04, // HCI Event packet from Controller to Host
} hci_packet_type_t;

typedef enum
{
    HCI_CMD_RESET,
    HCI_CMD_DOWNLOAD_MINIDRIVER,
    HCI_CMD_WRITE_RAM,
    HCI_CMD_LAUNCH_RAM,
    HCI_CMD_READ_BD_ADDR,
    HCI_CMD_WRITE_BD_ADDR,
} hci_command_type_t;

typedef enum
{
    HCI_CMD_OPCODE_RESET               = 0x0C03,
    HCI_CMD_OPCODE_DOWNLOAD_MINIDRIVER = 0xFC2E,
    HCI_CMD_OPCODE_WRITE_RAM           = 0xFC4C,
    HCI_CMD_OPCODE_LAUNCH_RAM          = 0xFC4E,
    HCI_CMD_OPCODE_READ_BD_ADDR        = 0x1009,
    HCI_CMD_OPCODE_WRITE_BD_ADDR       = 0xFC01,
} hci_command_opcode_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)

typedef struct
{
    hci_packet_type_t packet_type; /* This is transport layer packet type. Not transmitted if transport bus is USB */
    uint16_t          op_code;
    uint8_t           content_length;
} hci_command_header_t;

typedef struct
{
    hci_packet_type_t packet_type; /* This is transport layer packet type. Not transmitted if transport bus is USB */
    uint8_t           event_code;
    uint8_t           content_length;
} hci_event_header_t;

typedef struct
{
    hci_packet_type_t packet_type; /* This is transport layer packet type. Not transmitted if transport bus is USB */
    uint16_t          hci_handle;
    uint16_t          content_length;
} hci_acl_packet_header_t;

typedef struct
{
    hci_packet_type_t packet_type; /* This is transport layer packet type. Not transmitted if transport bus is USB */
    uint16_t          hci_handle;
    uint8_t           content_length;
} hci_sco_packet_header_t;

typedef struct
{
    hci_event_header_t header;
    uint8_t            total_packets;
    uint16_t           op_code;
    uint8_t            status;
} hci_event_extended_header_t;

#pragma pack()

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif
