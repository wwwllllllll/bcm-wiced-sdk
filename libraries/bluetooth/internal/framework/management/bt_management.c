/**
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
#include "wiced_bt.h"
#include "wiced_bt_constants.h"
#include "bt_bus.h"
#include "bt_packet_internal.h"
#include "bt_hci_interface.h"
#include "bt_firmware_image.h"
#ifdef BT_MPAF_MODE
#include "bt_mpaf.h"
#include "bt_management_mpaf.h"
#endif
#ifdef BT_HCI_MODE
#include "bt_management_hci.h"
#include "bt_stack.h"
#endif
#ifdef BT_MFGTEST_MODE
#include "bt_mfgtest.h"
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define BT_DEVICE_NAME_MAX_LENGTH 21

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static wiced_result_t bt_management_read_device_address    ( void );
static wiced_result_t bt_management_override_device_address( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static char                              bt_device_name[BT_DEVICE_NAME_MAX_LENGTH + 1] = { 0 };
static wiced_bt_mode_t                   bt_mode                  = WICED_BT_HCI_MODE;
static wiced_bool_t                      bt_initialised           = WICED_FALSE;
static wiced_bt_device_address_t         bt_address               = { .address = { 0 } };
static wiced_bt_device_address_t         bt_override_address      = { .address = { 0 } };
static wiced_bt_device_address_t         bt_override_address_mask = { .address = { 0 } };
static wiced_bool_t                      bt_is_address_overriden  = WICED_FALSE;
static const hci_command_header_t        command_read_bd_addr     = { .packet_type = 0x1, .op_code = HCI_CMD_OPCODE_READ_BD_ADDR,  .content_length = 0x0 };
static const hci_command_header_t        command_write_bd_addr    = { .packet_type = 0x1, .op_code = HCI_CMD_OPCODE_WRITE_BD_ADDR, .content_length = 0x6 };
static const hci_event_extended_header_t event_read_bd_addr       = { .header = {.packet_type = 0x4, .event_code = 0xE, .content_length = 0xA }, .total_packets = 0x1, .op_code = HCI_CMD_OPCODE_READ_BD_ADDR,  .status = 0x0 };
static const hci_event_extended_header_t event_write_bd_addr      = { .header = {.packet_type = 0x4, .event_code = 0xE, .content_length = 0x4 }, .total_packets = 0x1, .op_code = HCI_CMD_OPCODE_WRITE_BD_ADDR, .status = 0x0 };

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_bt_init( wiced_bt_mode_t mode, const char* device_name )
{
    const char *   firmware_version = NULL;
    wiced_result_t result;

    if ( bt_initialised == WICED_TRUE )
    {
        return WICED_BT_SUCCESS;
    }

    WPRINT_LIB_INFO( ("Starting WICED Bluetooth\n") );

    result = bt_bus_init( );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error initialising Bluetooth transport bus\n") );
        return result;
    }

    /* Read device address from the controller. This should be placed here before any other HCI commands */
    bt_management_read_device_address();

    /* This function is to override BD_ADDR at runtime. This should be placed here before any other HCI commands */
    bt_management_override_device_address();

    if ( mode == WICED_BT_MPAF_MODE )
    {
        #ifdef BT_MPAF_MODE
        firmware_version = bt_mpaf_firmware_version;

        result = bt_management_mpaf_init();
        if ( result != WICED_BT_SUCCESS )
        {
            WPRINT_LIB_ERROR( ("Error initialising MPAF\n") );
            return result;
        }
        #else
        return WICED_BT_UNSUPPORTED;
        #endif
    }
    else if ( mode == WICED_BT_HCI_MODE )
    {
        #ifdef BT_HCI_MODE
        firmware_version = bt_hci_firmware_version;

        /* Initialise Host-Controller Interface */
        result = bt_management_hci_init();
        if ( result != WICED_BT_SUCCESS )
        {
            WPRINT_LIB_ERROR( ("Error initialising HCI\n") );
            return result;
        }
        /* Initialise Bluetooth Stack */
        result = bt_stack_init();
        if ( result != WICED_BT_SUCCESS )
        {
            WPRINT_LIB_ERROR( ("Error initialising Bluetooth stack\n") );
            return result;
        }
        #else
        return WICED_BT_UNSUPPORTED;
        #endif
    }

    bt_mode = mode;
    memset( bt_device_name, 0, sizeof( bt_device_name ) );
    memcpy( bt_device_name, device_name, strnlen( device_name, BT_DEVICE_NAME_MAX_LENGTH ) );
    bt_initialised = WICED_TRUE;

#ifdef WPRINT_ENABLE_LIB_INFO
    WPRINT_LIB_INFO( ("Bluetooth MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\r\n", bt_address.address[5], bt_address.address[4], bt_address.address[3], bt_address.address[2], bt_address.address[1], bt_address.address[0] ) );
    WPRINT_LIB_INFO( ("Bluetooth Firmware    : %s\r\n", firmware_version ) );
#endif

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_deinit( void )
{
    if ( bt_initialised == WICED_FALSE )
    {
        return WICED_BT_SUCCESS;
    }

    if ( bt_mode == WICED_BT_MPAF_MODE )
    {
#ifdef BT_MPAF_MODE
        bt_management_mpaf_deinit();
#else
        return WICED_BT_UNSUPPORTED;
#endif
    }
    else if ( bt_mode == WICED_BT_HCI_MODE )
    {
#ifdef BT_HCI_MODE
        bt_stack_deinit();
        bt_management_hci_deinit();
#else
        return WICED_BT_UNSUPPORTED;
#endif
    }

    bt_bus_deinit();
    memset( bt_device_name, 0, sizeof( bt_device_name ) );
    bt_initialised = WICED_FALSE;
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_init_address( const wiced_bt_device_address_t* address, const wiced_bt_device_address_t* mask )
{
    bt_override_address      = *address;
    bt_override_address_mask = *mask;
    bt_is_address_overriden  = WICED_TRUE;
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_start_mfgtest_mode( const wiced_uart_config_t* config )
{
#ifdef BT_MFGTEST_MODE
    wiced_result_t result = bt_bus_init();
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error initialising Bluetooth transport bus\n") );
        return result;
    }

    return bt_mfgtest_start( config );
#else
    return WICED_BT_UNSUPPORTED;
#endif
}

wiced_result_t wiced_bt_device_get_address( wiced_bt_device_address_t* address )
{
    memcpy( address, &bt_address, sizeof( *address ) );
    return WICED_BT_SUCCESS;
}

const char* wiced_bt_device_get_name( void )
{
    return (const char*)bt_device_name;
}

wiced_bool_t wiced_bt_device_is_on( void )
{
    return ( bt_initialised == WICED_FALSE ) ? WICED_FALSE : bt_bus_is_on( );
}

wiced_bool_t wiced_bt_device_is_connectable( void )
{
    if ( bt_initialised == WICED_FALSE )
    {
        return WICED_FALSE;
    }

    if ( bt_mode == WICED_BT_MPAF_MODE )
    {
#ifdef BT_MPAF_MODE
        return bt_management_mpaf_is_device_connectable( );
#else
        return WICED_FALSE;
#endif
    }
    else if ( bt_mode == WICED_BT_HCI_MODE )
    {
#ifdef BT_HCI_MODE
        return bt_management_hci_is_device_connectable( );
#else
        return WICED_FALSE;
#endif
    }

    return WICED_FALSE;
}

wiced_bool_t wiced_bt_device_is_discoverable( void )
{
    if ( bt_initialised == WICED_FALSE )
    {
        return WICED_FALSE;
    }

    if ( bt_mode == WICED_BT_MPAF_MODE )
    {
#ifdef BT_MPAF_MODE
        return bt_management_mpaf_is_device_connectable( );
#else
        return WICED_FALSE;
#endif
    }
    else if ( bt_mode == WICED_BT_HCI_MODE )
    {
#ifdef BT_HCI_MODE
        return bt_management_hci_is_device_connectable( );
#else
        return WICED_FALSE;
#endif
    }

    return WICED_FALSE;
}

/******************************************************
 *            Internal Function Definitions
 ******************************************************/

static wiced_result_t bt_management_read_device_address( void )
{
    uint8_t                      read_bd_addr_event_data[sizeof(hci_event_extended_header_t) + sizeof(wiced_bt_device_address_t)] = { 0 };
    hci_event_extended_header_t* read_bd_addr_event = (hci_event_extended_header_t*)read_bd_addr_event_data;
    wiced_bt_device_address_t*   address_read       = (wiced_bt_device_address_t*)( read_bd_addr_event + 1 );
    wiced_result_t               result;

    result = bt_bus_transmit( (const uint8_t*)&command_read_bd_addr, sizeof( command_read_bd_addr ) );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error sending HCI_READ_BD_ADDR command\n") );
        return result;
    }

    result = bt_bus_receive( read_bd_addr_event_data, sizeof( read_bd_addr_event_data ), 3000 );
    if ( result  != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error receiving HCI_READ_BD_ADDR response\n") );
        return result;
    }

    if ( memcmp( read_bd_addr_event, &event_read_bd_addr, sizeof( event_read_bd_addr ) ) != 0 )
    {
        WPRINT_LIB_ERROR( ("Unexpected HCI_READ_BD_ADDR response\n") );
        return WICED_BT_UNKNOWN_PACKET;
    }

    memcpy( &bt_address, address_read, sizeof( bt_address ) );
    return WICED_BT_SUCCESS;
}

static wiced_result_t bt_management_override_device_address( void )
{
    uint8_t                      write_bd_addr_cmd_data[sizeof(hci_command_header_t) + sizeof(wiced_bt_device_address_t)] = { 0 };
    hci_command_header_t*        write_bd_addr_cmd  = (hci_command_header_t*)write_bd_addr_cmd_data;
    wiced_bt_device_address_t*   new_address        = (wiced_bt_device_address_t*)( write_bd_addr_cmd + 1 );
    hci_event_extended_header_t  write_bd_addr_event;
    wiced_result_t               result;

    /* If application decides to override the address, read the default address and override */
    if ( bt_is_address_overriden == WICED_FALSE )
    {
        /* No override */
        return WICED_BT_SUCCESS;
    }

    /* Override address */
    new_address->address[0] = ( bt_address.address[0] & ~( bt_override_address_mask.address[0] ) ) | ( bt_override_address.address[0] & bt_override_address_mask.address[0] );
    new_address->address[1] = ( bt_address.address[1] & ~( bt_override_address_mask.address[1] ) ) | ( bt_override_address.address[1] & bt_override_address_mask.address[1] );
    new_address->address[2] = ( bt_address.address[2] & ~( bt_override_address_mask.address[2] ) ) | ( bt_override_address.address[2] & bt_override_address_mask.address[2] );
    new_address->address[3] = ( bt_address.address[3] & ~( bt_override_address_mask.address[3] ) ) | ( bt_override_address.address[3] & bt_override_address_mask.address[3] );
    new_address->address[4] = ( bt_address.address[4] & ~( bt_override_address_mask.address[4] ) ) | ( bt_override_address.address[4] & bt_override_address_mask.address[4] );
    new_address->address[5] = ( bt_address.address[5] & ~( bt_override_address_mask.address[5] ) ) | ( bt_override_address.address[5] & bt_override_address_mask.address[5] );

    memcpy( write_bd_addr_cmd, &command_write_bd_addr, sizeof( *write_bd_addr_cmd ) );

    result = bt_bus_transmit( (const uint8_t*)write_bd_addr_cmd_data, sizeof( write_bd_addr_cmd_data ) );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error sending HCI_WRITE_BD_ADDR command\n") );
        return result;
    }

    result = bt_bus_receive( (uint8_t*)&write_bd_addr_event, sizeof( write_bd_addr_event ), 1000 );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error receiving HCI_WRITE_BD_ADDR response\n") );
        return result;
    }

    if ( memcmp( &write_bd_addr_event, &event_write_bd_addr, sizeof( write_bd_addr_event ) ) != 0 )
    {
        WPRINT_LIB_ERROR( ("Unexpected HCI_WRITE_BD_ADDR response\n") );
        return WICED_BT_UNKNOWN_PACKET;
    }

    /* Update local device address */
    memcpy( &bt_address, new_address, sizeof( bt_address ) );
    return WICED_BT_SUCCESS;
}
