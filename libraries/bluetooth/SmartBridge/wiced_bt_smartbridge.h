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
#include "wiced_bt_smart_interface.h"
#include "bt_linked_list.h"

/** @file
 *  Defines functions for bridging Bluetooth Smart with Wi-Fi
 */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/**
 * SmartBridge socket status
 */
typedef enum
{
    SMARTBRIDGE_SOCKET_DISCONNECTED, /**< Socket is disconnected                   */
    SMARTBRIDGE_SOCKET_CONNECTING,   /**< Socket is in connecting state            */
    SMARTBRIDGE_SOCKET_CONNECTED,    /**< Socket is connected with a remote device */
} wiced_bt_smartbridge_socket_status_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/**
 * Socket to create a SmartBridge connection
 */
typedef struct wiced_bt_smartbridge_socket wiced_bt_smartbridge_socket_t;

/**
 * Socket disconnection callback
 */
typedef wiced_result_t (*wiced_bt_smartbridge_disconnection_callback_t)( wiced_bt_smartbridge_socket_t* socket );

/**
 * Socket GATT notification callback
 */
typedef wiced_result_t (*wiced_bt_smartbridge_notification_callback_t) ( wiced_bt_smartbridge_socket_t* socket, uint16_t attribute_handle );

/**
 * Socket Pairing/Bonding callback
 */
typedef wiced_result_t (*wiced_bt_smartbridge_bonding_callback_t)      ( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_bond_info_t* bond_info );

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)
/**
 * Socket to create a SmartBridge connection
 * @warning The content of the socket structure is for INTERNAL USE only. Modifying
 * the content of this structure is prohibited. Please use the Bluetooth SmartBridge
 * API to retrieve socket information.
 */
struct wiced_bt_smartbridge_socket
{
    bt_list_node_t                                node;                           /**< Socket list node                                              */
    wiced_bt_smart_device_t                       remote_device;                  /**< Remote Bluetooth device WICED is connected with (bridging)    */
    uint16_t                                      connection_handle;              /**< Connection handle                                             */
    uint16_t                                      last_notified_attribute_handle; /**< Last notified attribute handle                                */
    uint8_t                                       state;                          /**< Internal state                                                */
    uint8_t                                       actions;                        /**< Internal socket actions                                       */
    wiced_bt_smartbridge_disconnection_callback_t disconnection_callback;         /**< Callback for handling disconnection event by remote device    */
    wiced_bt_smartbridge_bonding_callback_t       bonding_callback;               /**< Callback for handling pairing/bonding successful event        */
    wiced_bt_smartbridge_notification_callback_t  notification_callback;          /**< Callback for handling GATT notification from remote device    */
    wiced_bt_smart_connection_settings_t          connection_settings;            /**< Connection settings                                           */
    wiced_bt_smart_security_settings_t            security_settings;              /**< Security settings                                             */
    char                                          passkey[7];                     /**< NUL-terminated numeric passkey                                */
    wiced_bt_smart_bond_info_t                    bond_info;                      /**< Bond Info                                                     */
    void*                                         att_cache;                      /**< Pointer to Attribute Cache                                    */
    wiced_semaphore_t                             semaphore;                      /**< Semaphore                                                     */
};
#pragma pack()

/******************************************************
 *             Function declarations
 ******************************************************/

/*****************************************************************************/
/** @addtogroup smartbridge  SmartBridge
 *  @ingroup wicedbt
 *
 *  Bluetooth SmartBridge Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/*****************************************************************************/
/** @addtogroup sbmgmt SmartBridge Management
 *  @ingroup smartbridge
 *
 *  SmartBridge Management Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Initialise the WICED SmartBridge
 *
 * @note
 * This function initialises:
 * \li Generic Attribute Profile (GATT) Client
 * \li Generic Access Profile (GAP) Central Role
 * \li SmartBridge Socket Manager
 *
 * After calling @ref wiced_bt_smartbridge_init, you may call:
 * \li  @ref wiced_bt_smartbridge_set_max_concurrent_connections() to set the maximum
 *      number of concurrent connections
 * \li  @ref wiced_bt_smartbridge_enable_attribute_cache() to enable Attribute Cache
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_init( void );


/** Deinitialise the WICED SmartBridge
 *
 * @note
 * This function deinitialises:
 * \li GATT Client
 * \li GAP Central Role
 * \li SmartBridge Socket Manager
 * \li SmartBridge Attribute Cache Manager (if enabled)
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_deinit( void );


/** Set the number of concurrent connections supported by the WICED SmartBridge
 *
 * @note
 * This function sets the maximum number of concurrent connections supported by
 * the WICED SmartBridge and allocates a workspace for the connections.
 *
 * @warning
 * To enable support for concurrent connections, this function shall be called
 * immediately after @ref wiced_bt_smartbridge_init(). Otherwise, only a single
 * connection is supported. The number cannot change unless the WICED SmartBridge
 * is reinitialised.
 *
 * @param[in]  count : connection count
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_set_max_concurrent_connections( uint8_t count );


/** Enable Attribute Cache
 *
 * @note
 * This function enables the attribute caching feature as specified in the Bluetooth
 * Specification version 4.0 Volume 3 Part G Section 2.5.2. Attribute Cache allows
 * SmartBridge to discover server's Attribute information once and use cached information
 * across reconnections without rediscovery, thus saving time and reducing the amount
 * of packet exhange required between WICED SmartBridge and the server.
 *
 * On a reconnection, SmartBridge searches for matching Attribute information in
 * the cache. If not found, SmartBridge  discovers server's Attribute
 * information and adds the information to the cache when completed. If SmartBridge
 * runs out of cache space, the first non-active cache in the list is replaced.
 *
 * @param[in]  cache_count : the number of caches that will be supported by WICED SmartBridge
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_enable_attribute_cache( uint32_t cache_count );


/** Disable Attribute Cache
 *
 * @note
 * This function disables the attribute caching feature
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_disable_attribute_cache( void );


/** @} */

/*****************************************************************************/
/** @addtogroup sbscan SmartBridge Scan
 *  @ingroup smartbridge
 *
 *  SmartBridge Scan Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Check if the WICED SmartBridge is currently scanning for Bluetooth Smart devices
 *
 * @note
 * This function checks if the Bluetooth Controller is currently scanning for Bluetooth
 * Smart devices
 *
 * @return @ref wiced_bool_t
 */
wiced_bool_t   wiced_bt_smartbridge_is_scanning( void );


/** Start scanning for remote Bluetooth Smart devices
 *
 * @note
 * This function instructs the Bluetooth controller to start scanning for remote
 * Bluetooth Smart devices within proximity with the settings specified.
 * Scan results are reported via the given callback handlers.
 *
 * @warning
 * \li result_callback is an intermediate report callback. The complete scan results
 *     are retrieved using @ref wiced_bt_smartbridge_get_scan_result_list once scan
 *     is complete
 * \li advertising_report_callback runs on the context of Bluetooth transport thread. Please refrain
 *     from executing a long task in the callback
 * \li complete_callback runs on the context of WICED_NETWORKING_WORKER_THREAD
 * \li If the whitelist filter is enabled in the scan settings, only devices
 *     in the whitelist appear in the scan results. Call @ref wiced_bt_smartbridge_add_device_to_whitelist()
 *     to add a device to the whitelist
 *
 * @param[in]  settings                     : scan settings
 * @param[in]  complete_callback            : callback function which is called when scan is
 *                                            complete
 * @param[in]  advertising_report_callback  : callback function which is called when an advertising
 *                                            report is received
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_start_scan( const wiced_bt_smart_scan_settings_t*        settings,
                                                wiced_bt_smart_scan_complete_callback_t      complete_callback,
                                                wiced_bt_smart_advertising_report_callback_t advertising_report_callback );


/** Stop the ongoing scan process
 *
 * This function instructs the Bluetooth controller to stop scanning for remote
 * Bluetooth Smart devices.
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_stop_scan( void );


/** Retrieve the most recent scan results
 *
 * @note
 * This function returns a linked-list of the most recent scan results
 *
 * @warning:
 * Application must not modify the list
 *
 * @param[out]  result_list : pointer that will receive the result linked-list
 * @param[out]  count       : variable that will receive the result count
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_get_scan_result_list( wiced_bt_smart_scan_result_t** result_list, uint32_t* count );

/** @} */


/*****************************************************************************/
/** @addtogroup sbwhitelist SmartBridge Whitelist Filter
 *  @ingroup smartbridge
 *
 *  SmartBridge Whitelist Filter Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Add a Bluetooth Smart device to the whitelist
 *
 * @note
 * This function instructs the Bluetooth Controller to add a Bluetooth Smart device
 * with the address and address type provided to the whitelist
 *
 * @warning
 * User application shall maintain a copy of the whitelist. The Bluetooth
 * Smart Host Controller Interface (HCI) does not provide a command to retrieve
 * the whitelist
 *
 * @param[in]  device_address : Bluetooth address of the device to add to the whitelist
 * @param[in]  address_type   : Bluetooth Smart address type of the device
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_add_device_to_whitelist( const wiced_bt_device_address_t* device_address, wiced_bt_smart_address_type_t address_type );


/** Remove a Bluetooth Smart device from the whitelist
 *
 * @note
 * This function instructs the Bluetooth Controller to remove a Bluetooth Smart device
 * with the address and address type provided from whitelist
 *
 * @param[in]  device_address : Bluetooth address of the device to remove from the whitelist
 * @param[in]  address_type   : Bluetooth Smart address type of the device
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_remove_device_from_whitelist( const wiced_bt_device_address_t* device_address, wiced_bt_smart_address_type_t address_type );


/** Get the maximum number of devices supported by the whitelist
 *
 * @note
 * This function retrieves the maximum number of Bluetooth Smart devices which can
 * be added to the whitelist.
 *
 * @param[out]  size : device count
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_get_whitelist_size( uint32_t* size );


/** Clear the whitelist
 *
 * @note
 * This function instructs the Bluetooth Controller to remove all devices from the
 * whitelist
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_clear_whitelist( void );


/** @} */


/*****************************************************************************/
/** @addtogroup sbsock SmartBridge Socket and Connection Management
 *  @ingroup smartbridge
 *
 *  SmartBridge Socket and Connection Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Check if the Bluetooth SmartBridge is ready to connect
 *
 * @note
 * This function checks if the Bluetooth SmartBridge is ready to make a connection
 * to a Bluetooth Smart device. The Bluetooth Controller does not support concurrent
 * connection requests.
 *
 * @return @ref wiced_bool_t
 */
wiced_bool_t   wiced_bt_smartbridge_is_ready_to_connect( void );


/** Get SmartBridge socket status
 *
 * @param[in]  socket : pointer to the socket to get the status
 * @param[out] status : socket status
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_get_socket_status( wiced_bt_smartbridge_socket_t* socket, wiced_bt_smartbridge_socket_status_t* status );


/** Create a SmartBridge socket
 *
 * @note
 * \li This function initialises the socket internals to make it ready to connect to
 *     a Bluetooth Smart device
 * \li Socket reconnections are allowed without re-creations
 *
 * @param[out]  socket : pointer to the socket to initialise
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_create_socket( wiced_bt_smartbridge_socket_t* socket );


/** Delete a SmartBridge socket
 *
 * @note
 * This function deinitialises the socket internals.
 *
 * @param[in,out]  socket : pointer to the socket to deinitialise
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_delete_socket( wiced_bt_smartbridge_socket_t* socket );


/** Initiate a SmartBridge connection with a remote Bluetooth Smart device
 *
 * @note
 * This function creates a connection with the specified remote device.
 *
 * \li If Bond Information is loaded, the socket uses the Long-Term Key (LTK), Random
 *     Number (RAND), and Encryption Diversifier (EDIV) of the Bond Information to
 *     encrypt the connection. Call @ref wiced_bt_smartbridge_set_bond_info() before
 *     connecting to load Bond information.
 * \li If Pairing is enforced, the socket initiates Pairing Request and a new Bond
 *     is created. Call @ref wiced_bt_smartbridge_enable_pairing() before connecting
 *     to initiate Pairing Request.
 * \li If Attribute Cache is enabled, the socket starts a discovery of the server's
 *     Attribute information.
 *
 * @warning
 * \li Callback functions run on the context of WICED_NETWORKING_WORKER_THREAD.
 * \li If Pairing and Attribute Cache are enabled, this function may block for a
 *     few seconds.
 *
 * @param[in,out]  socket                 : pointer to the socket to create the
 *                                          connection
 * @param[in]      remote_device          : remote device to connect
 * @param[in]      settings               : connection settings
 * @param[in]      disconnection_callback : callback function that is called when
 *                                          the connection is disconnected by
 *                                          remote device
 * @param[in]      notification_callback  : callback function that is called when
 *                                          a GATT notification or indication is
 *                                          received from the remote device
 *

 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_connect( wiced_bt_smartbridge_socket_t*                socket,
                                             const wiced_bt_smart_device_t*                remote_device,
                                             const wiced_bt_smart_connection_settings_t*   settings,
                                             wiced_bt_smartbridge_disconnection_callback_t disconnection_callback,
                                             wiced_bt_smartbridge_notification_callback_t  notification_callback );


/** Disconnect a SmartBridge connection
 *
 * @note
 * This function disconnects a connection with a remote device.
 *
 * @param[in,out]  socket : pointer to the socket of the connection to disconnect
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_disconnect( wiced_bt_smartbridge_socket_t* socket );


/** Set transmit power during connection
 *
 * @note
 * This function set the transmit power of the connection
 *
 * @param[in]  socket             : pointer to the socket of the connection
 * @param[in]  transmit_power_dbm : transmit power in dBm
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_set_transmit_power( wiced_bt_smartbridge_socket_t* socket, int8_t transmit_power_dbm );

/** @} */

/*****************************************************************************/
/** @addtogroup sbsec SmartBridge Security
 *  @ingroup smartbridge
 *
 *  SmartBridge Security Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Set a peer device's bond information
 *
 * @note
 * This function loads Bond (security) information to the socket which will be used
 * to reestablish an encrypted link with the peer device without having to perform
 * the complete handshake procedure (pairing)
 *
 * @warning
 * This function must be called before calling @ref wiced_bt_smartbridge_connect
 *
 * @param[in,out]  socket    : socket to load the Bond information into
 * @param[in]      settings  : security settings
 * @param[in]      bond_info : Bond information
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_set_bond_info( wiced_bt_smartbridge_socket_t*            socket,
                                                   const wiced_bt_smart_security_settings_t* settings,
                                                   const wiced_bt_smart_bond_info_t*         bond_info );


/** Reset the bond information stored in the socket
 *
 * @warning
 * This function must be called before calling @ref wiced_bt_smartbridge_connect
 *
 * @param[in,out]  socket : pointer to the socket of the connection to disconnect
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_clear_bond_info( wiced_bt_smartbridge_socket_t* socket );


/** Force a SmartBridge socket to initiate Pairing Request with a Bluetooth Smart device
 *
 * @note
 * This function prepares the socket to initiate Pairing Request with the device.
 * Previously loaded bond information is deleted and is replaced with a new bond
 * if the pairing process succeeds.
 *
 * @warning
 * \li This function must be called before calling @ref wiced_bt_smartbridge_connect
 * \li Callback functions run on the context of WICED_NETWORKING_WORKER_THREAD
 *
 * @param[in,out]  socket           : pointer to the socket to enable pairing
 * @param[in]      settings         : security settings
 * @param[in]      numeric_passkey  : NUL-terminated string of numeric passkey. Maximum is 6 digits.
 * @param[in]      bonding callback : callback that is called when bonding with remote device occurs
 *
 * @note:
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_enable_pairing( wiced_bt_smartbridge_socket_t*            socket,
                                                    const wiced_bt_smart_security_settings_t* settings,
                                                    const char*                               numeric_passkey,
                                                    wiced_bt_smartbridge_bonding_callback_t   bonding_callback );


/** Set a SmartBridge socket to disable pairing with a remote device
 *
 * @warning
 * This function must be called before calling @ref wiced_bt_smartbridge_connect
 *
 * @param[in,out]  socket : pointer to the socket to disable pairing
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_disable_pairing( wiced_bt_smartbridge_socket_t* socket );

/** @} */

/*****************************************************************************/
/** @addtogroup sbattr SmartBridge Attribute Cache
 *  @ingroup smartbridge
 *
 *  SmartBridge Attribute Cache Functions
 *
 *
 *  @{
 */
/*****************************************************************************/


/** Enable all GATT notifications supported by the server
 *
 * @note
 * This function searches for client configuration Attribute in the cache and enables
 * all notifications from the server.
 *
 * @warning This function returns error if Attribute Cache is not enabled
 *
 * @param[in] socket : socket to enable notifications
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_enable_attribute_cache_notification( wiced_bt_smartbridge_socket_t* socket );


/** Disable all GATT notifications supported by the server
 *
 * @note
 * This function searches for client configuration Attribute in the cache and disables
 * all notifications from the server.
 *
 * @warning This function returns error if Attribute Cache is not enabled
 *
 * @param[in] socket : socket to disable notifications
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_disable_attribute_cache_notification( wiced_bt_smartbridge_socket_t* socket );


/** Retrieve the list of cached Attributes
 *
 * @note
 * This function retrieves the list of cached Attributes of the socket
 *
 * @warning This function returns error if Attribute Cache is not enabled
 *
 * @param[in] socket          : socket to get the Attribute list from
 * @param[out] att_cache_list : pointer that will receive the Attribute list
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_get_attribute_cache_list( wiced_bt_smartbridge_socket_t* socket, wiced_bt_smart_attribute_list_t** att_cache_list );


/** Find and read attribute with the handle provided from the Attribute Cache
 *
 * @note
 * This function searches for an attribute with the given handle in the cache and
 * copies the content to the attribute buffer provided.
 *
 * @warning This function returns error if Attribute Cache is not enabled
 *
 * @param[in]      socket    : pointer to the socket to search the attribute
 * @param[in]      handle    : handle of the attribute to find
 * @param[in,out]  attribute : pointer to the buffer which will contain the attribute
 * @param[in]      size      : buffer size in bytes
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_get_attribute_cache_by_handle( wiced_bt_smartbridge_socket_t* socket, uint16_t handle, wiced_bt_smart_attribute_t* attribute, uint16_t size );


/** Find and read attribute with the UUID provided from the local attribute database
 *
 * @note
 * This function searches for an attribute with the given UUID in the local
 * attribute database and copies the content to the attribute buffer provided.
 *
 * @warning This function returns error if Attribute Cache is not enabled
 *
 * @param[in]      socket          : pointer to the socket to search the attribute from
 * @param[in]      uuid            : UUID of the attribute to find
 * @param[in]      starting_handle : the search starting handle
 * @param[in]      ending_handle   : the search ending handle
 * @param[in,out]  attribute       : pointer to the buffer which will contain the attribute content
 * @param[in]      size            : buffer size in bytes
 *
 * @return @ref wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_get_attribute_cache_by_uuid( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_uuid_t* uuid, uint16_t starting_handle, uint16_t ending_handle, wiced_bt_smart_attribute_t* attribute, uint32_t size );


/** Find and refresh Characteristic Value with the given handle in the Attribute Cache
 *
 * @note
 * This function reads the Characteristic Value from the server and updates the
 * cache.
 *
 * @warning This function returns error if Attribute Cache is not enabled
 *
 * @param[in] socket : socket with the Characteristic Value to refresh
 * @param[in] handle : handle of the Characteritic Value to refresh
 *
 * @return wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_refresh_attribute_cache_characteristic_value( wiced_bt_smartbridge_socket_t* socket, uint16_t handle );


/** Write Characteristic Value in the Attribute Cache to the server
 *
 * @note
 * This function writes the given Characteristic Value to the server and updates the
 * cache.
 *
 * @warning This function returns error if Attribute Cache is not enabled
 *
 * @param[in] socket               : socket with the Characteristic Value to write to the server
 * @param[in] characteristic_value : Characteritic Value to write to the server
 *
 * @return wiced_result_t
 */
wiced_result_t wiced_bt_smartbridge_write_attribute_cache_characteristic_value( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value );

/** @} */

/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif

