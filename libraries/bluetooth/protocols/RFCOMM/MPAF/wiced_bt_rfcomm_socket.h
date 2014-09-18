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

#include "wiced.h"
#include "wiced_bt.h"
#include "bt_linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define RFCOMM_SOCKET_ID              0x1206030F
#define RFCOMM_SOCKET_INITIALISED           0x00
#define RFCOMM_SOCKET_LISTENING             0x01
#define RFCOMM_SOCKET_CONNECTING            0x02
#define RFCOMM_SOCKET_CONNECTED             0x04
#define RFCOMM_SOCKET_WAITING_FOR_PACKET    0x08
#define RFCOMM_SOCKET_DISCONNECTING         0x10
#define RFCOMM_SOCKET_TERMINATED            0x20
#define RFCOMM_MAX_ENDPOINT                   15

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct wiced_bt_rfcomm_socket wiced_bt_rfcomm_socket_t;

/******************************************************
 *                    Structures
 ******************************************************/

struct wiced_bt_rfcomm_socket
{

    uint32_t          id;
    wiced_semaphore_t semaphore;
    void*             handler;
    void*             arg;

    struct
    {
        void               (*lock)  ( wiced_bt_rfcomm_socket_t* socket );
        void               (*unlock)( wiced_bt_rfcomm_socket_t* socket );
        wiced_mutex_t      mutex;
        uint8_t            status;
        uint8_t            endpoint;
        uint8_t            channel;
        uint8_t            sdp_record_handle;
        bt_linked_list_t rx_packet_list;
    } shared;

};

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif
