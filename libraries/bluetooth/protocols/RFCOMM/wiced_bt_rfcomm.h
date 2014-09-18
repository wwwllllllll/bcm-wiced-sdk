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
 *  Defines functions for communications over Bluetooth RFCOMM protocol
 */

#pragma once

#include "wiced.h"
#include "wiced_bt.h"
#include "wiced_bt_constants.h"
#include "wiced_bt_rfcomm_socket.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/** RFCOMM Event
 */
typedef enum
{
    RFCOMM_EVENT_CONNECTION_ERROR,        /**< RFCOMM connection error has occurred                                             */
    RFCOMM_EVENT_CONNECTION_ACCEPTED,     /**< RFCOMM connection request from a remote device has been accepted                 */
    RFCOMM_EVENT_CONNECTION_DISCONNECTED, /**< RFCOMM connection has been disconnected by the remote device                     */
    RFCOMM_EVENT_INCOMING_PACKET_READY,   /**< Incoming packet is ready. Call wiced_bt_rfcomm_receive_packet to retrieve packet */
    RFCOMM_EVENT_CLIENT_CONNECTED,        /**< RFCOMM client is connected to a remote device                                    */
} wiced_bt_rfcomm_event_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/** RFCOMM event callback function
 */
typedef void (*wiced_bt_rfcomm_event_handler_t)( wiced_bt_rfcomm_socket_t* socket, wiced_bt_rfcomm_event_t event, void* arg );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/*****************************************************************************/
/** @addtogroup btprot  Protocols
 *  @ingroup wicedbt
 *
 *  Bluetooth Protocols Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/*****************************************************************************/
/** @addtogroup btrfcomm      RFCOMM
 *  @ingroup btprot
 *
 * Bluetooth RFCOMM Protocol Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/*****************************************************************************/
/** @addtogroup btrfcommprot RFCOMM Protocol
 *  @ingroup btrfcomm
 *
 *  RFCOMM Protocol Management Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialise the Bluetooth RFCOMM protocol
 *
 * This function initialises the Bluetooth RFCOMM protocol and its supporting
 * software components.
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred.
 */
wiced_result_t wiced_bt_rfcomm_init( void );

/** Deinitialise the Bluetooth RFCOMM protocol
 *
 * This function tears down the Bluetooth RFCOMM protocol and its supporting
 * software components.
 *
 * @return    WICED_SUCCESS : on success.
 *            WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_bt_rfcomm_deinit( void );

/** @} */

/*****************************************************************************/
/** @addtogroup btrfcommconn RFCOMM Connection
 *  @ingroup btrfcomm
 *
 *  RFCOMM Connection Management Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Initialise a WICED Bluetooth RFCOMM socket
 *
 * This function initialises an RFCOMM socket
 *
 * @param socket  : A pointer to the socket to initialise
 * @param handler : A callback function provided by the user that is invoked
 *                  when any of the @ref wiced_bt_rfcomm_event_t events occurs.
 *                  A valid callback function must be provided.
 * @param arg     : An argument provided by the user which will be passed to
 *                  the callback function
 *
 * @warning Only @ref btpktmgmt, @ref btrfcommpkt and @ref btrfcommbuf function calls are
 *               are allowed from the callback function.
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if any other error occurred.
 */
wiced_result_t wiced_bt_rfcomm_init_socket( wiced_bt_rfcomm_socket_t* socket, wiced_bt_rfcomm_event_handler_t handler, void* arg );

/** Deinitialise a WICED Bluetooth RFCOMM socket
 *
 * This function deinitialises the socket
 *
 * @param socket  : A pointer to the socket to deinitialise
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if any other error occurred.
 */
wiced_result_t wiced_bt_rfcomm_deinit_socket( wiced_bt_rfcomm_socket_t* socket );

/** Listen for an RFCOMM connection from a remote Bluetooth device
 *
 * This function tells the local Bluetooth device to listen for an incoming
 * RFCOMM connection from a remote device.
 *
 * @warning: Only auto-accept is supported currently. Once the connection is
 * accepted, @ref RFCOMM_EVENT_CONNECTION_ACCEPTED will be reported via the
 * callback function provided.
 *
 * @param socket       : A socket which will listen for an incoming connection
 * @param auto_accept  : Accept the connection automatically if equals WICED_TRUE
 * @param service_name : Service name of the RFCOMM connection's SDP record
 * @param uuid         : UUID for of RFCOMM connection's SDP record
 *
 * @return    WICED_SUCCESS     : on success;
 *            WICED_BADARG      : if bad argument(s) are inserted;
 *            WICED_PENDING        : if socket already in listening state;
 *            WICED_UNSUPPORTED : if mode of operation is not supported;
 *            WICED_ERROR       : if an error occurred.
 */
wiced_result_t wiced_bt_rfcomm_listen( wiced_bt_rfcomm_socket_t* socket, wiced_bool_t auto_accept, const char* service_name, const wiced_bt_uuid_t* uuid );

/** Connect to an RFCOMM server running on a remote Bluetooth device
 *
 * This function tells the local Bluetooth device to initiate an RFCOMM connection
 * with a remote device. Once the connection is accepted, @ref RFCOMM_EVENT_CLIENT_CONNECTED
 * will be reported via the socket callback function provided.
 *
 * @param socket       : A socket which will listen for an incoming connection
 * @param service_name : Service name of the RFCOMM server to connect to
 * @param uuid         : UUID of RFCOMM server to connect to
 *
 * @return    WICED_SUCCESS     : on success;
 *            WICED_BADARG      : if bad argument(s) are inserted;
 *            WICED_PENDING        : if socket already in connecting state;
 *            WICED_UNSUPPORTED : if mode of operation is not supported;
 *            WICED_ERROR       : if an error occurred.
 */
wiced_result_t wiced_bt_rfcomm_connect( wiced_bt_rfcomm_socket_t* socket, const char* service_name, const wiced_bt_uuid_t* uuid );

/** Close an RFCOMM connection
 *
 * If in connected state, this function tells the local Bluetooth adapter to
 * disconnect the RFCOMM connection and de-initialise the socket. If the socket
 * is still in listening state, it will stop listening before deinitialising the
 * socket.
 *
 * @param socket : The socket which will have the connection closed
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_ERROR   : if an error occurred.
 */
wiced_result_t wiced_bt_rfcomm_disconnect( wiced_bt_rfcomm_socket_t* socket );

/** @} */

/*****************************************************************************/
/** @addtogroup btrfcommpkt RFCOMM Packet Comms
 *  @ingroup btrfcomm
 *
 *  Functions for communications over RFCOMM in packet mode
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Allocate a WICED Bluetooth RFCOMM packet
 *
 * This function dynamically allocates memory space for the specified packet
 * type and size and returns a pointer to it. It also returns a pointer to the
 * start of the data section.
 *
 * @note: User need not worry about allocating space for the header of the
 *        specified packet type as it is handled internally. If the size of the
 *        data is not known, user shall provide a large-enough size. The size
 *        can later be updated using @ref wiced_bt_packet_set_data_end.
 *
 * @param packet      : A pointer that will receive the pointer to the packet
 *                      created
 * @param data_size   : The size of the data to allocate
 * @param data        : A pointer that will receive the pointer to the data
 *                      section in the packet. This is where data shall be
 *                      written.
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_OUT_OF_HEAP_SPACE   : if no memory left;
 *            WICED_ERROR   : if any other error occurred.
 */
wiced_result_t wiced_bt_rfcomm_create_packet( wiced_bt_packet_t** packet, uint32_t data_size, uint8_t** data );

/** Send a packet containing RFCOMM data
 *
 * The packet is created and manipulated using API defined in wiced_bt_packet.h.
 *
 * @param socket : A pointer to an open socket handle
 * @param packet : A pointer to packet to send
 *
 * @return    WICED_SUCCESS : on success.
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred
 */
wiced_result_t wiced_bt_rfcomm_send_packet( wiced_bt_rfcomm_socket_t* socket, wiced_bt_packet_t* packet );

/** Receive a packet containing RFCOMM data
 *
 * @param socket     : A pointer to an open socket handle
 * @param packet     : A pointer that will receive the pointer to the incoming
 *                     packet
 * @param data       : A pointer that will receive the pointer to the data
 *                     section in the packet
 * @param data_size  : A pointer that will receive the data size in the packet
 * @param timeout_ms : The time in milliseconds the caller will wait before a
 *                     timeout occurs
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred.
 */
wiced_result_t wiced_bt_rfcomm_receive_packet( wiced_bt_rfcomm_socket_t* socket, wiced_bt_packet_t** packet, uint8_t** data, uint32_t* data_size, uint32_t timeout_ms );

/** @} */

/*****************************************************************************/
/** @addtogroup btrfcommbuf RFCOMM Buffer Comms
 *  @ingroup btrfcomm
 *
 *  Functions for communications over RFCOMM in buffer mode
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Send a C array of RFCOMM data
 *
 * @param socket      : A pointer to an open socket handle
 * @param buffer      : A pointer to the C array containing the data to send
 * @param buffer_size : The size of the array in bytes
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred.
 */
wiced_result_t wiced_bt_rfcomm_send_buffer( wiced_bt_rfcomm_socket_t* socket, const uint8_t* buffer, uint32_t buffer_size );

/** @} */
/** @} */
/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif
