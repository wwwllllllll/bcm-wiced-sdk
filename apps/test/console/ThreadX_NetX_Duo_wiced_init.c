/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "NetX_Duo/compat.h"
#include "tx_api.h"
#include "tx_thread.h"
#include "nx_api.h"
#include "wwd_management.h"
#include "wwd_network.h"
#include "wwd_wifi.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "network/wwd_buffer_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_network.h"
#include "console.h"
#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

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

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 *  Application start called byfunction in wiced_rtos.c
 */
void application_start( void )
{
    /* Initialise the device */
    wiced_init( );

    /* Run the main application function */
    console_app_main( );

}

int set_ip( int argc, char* argv[] )
{
    ULONG ipaddr, netmask, gw;
    if ( argc < 4 )
    {
        return ERR_UNKNOWN;
    }
    ipaddr  = htonl(str_to_ip(argv[1]));
    netmask = htonl(str_to_ip(argv[2]));
    gw      = htonl(str_to_ip(argv[3]));
    nx_ip_interface_address_set( &wiced_ip_handle[WICED_STA_INTERFACE], 0, ipaddr, netmask );
    nx_ip_gateway_address_set( &wiced_ip_handle[WICED_STA_INTERFACE], gw );
    return ERR_CMD_OK;
}

void network_print_status( char* sta_ssid, char* ap_ssid )
{
    uint8_t interface;
    for ( interface = 0; interface <= 1; interface++ )
    {
        if ( wwd_wifi_is_ready_to_transceive( (wiced_interface_t) interface ) == WWD_SUCCESS )
        {
            if ( interface == WICED_STA_INTERFACE )
            {
                WPRINT_APP_INFO( ( "STA Interface\r\n"));
                WPRINT_APP_INFO( ( "   SSID       : %s\r\n", sta_ssid ) );
            }
            else
            {
                WPRINT_APP_INFO( ( "AP Interface\r\n"));
                WPRINT_APP_INFO( ( "   SSID       : %s\r\n", ap_ssid ) );

            }

            if ( wiced_ip_handle[interface].nx_ip_driver_link_up )
            {
                uint32_t ip       = wiced_ip_handle[interface].nx_ip_address;
                uint32_t gateway  = wiced_ip_handle[interface].nx_ip_gateway_address;
                uint32_t net_mask = wiced_ip_handle[interface].nx_ip_network_mask;
                WPRINT_APP_INFO( ( "   IP Addr    : %u.%u.%u.%u\r\n", (unsigned char) ( ( ip >> 24 ) & 0xff ), (unsigned char) ( ( ip >> 16 ) & 0xff ), (unsigned char) ( ( ip >> 8 ) & 0xff ), (unsigned char) ( ( ip >> 0 ) & 0xff ) ) );
                WPRINT_APP_INFO( ( "   Gateway    : %u.%u.%u.%u\r\n", (unsigned char) ( ( gateway >> 24 ) & 0xff ), (unsigned char) ( ( gateway >> 16 ) & 0xff ), (unsigned char) ( ( gateway >> 8 ) & 0xff ), (unsigned char) ( ( gateway >> 0 ) & 0xff ) ) );
                WPRINT_APP_INFO( ( "   Netmask    : %u.%u.%u.%u\r\n", (unsigned char) ( ( net_mask >> 24 ) & 0xff ), (unsigned char) ( ( net_mask >> 16 ) & 0xff ), (unsigned char) ( ( net_mask >> 8 ) & 0xff ), (unsigned char) ( ( net_mask >> 0 ) & 0xff ) ) );
            }
        }
        else
        {
            if ( interface == WICED_STA_INTERFACE )
                WPRINT_APP_INFO( ( "STA Interface : Down\r\n" ) );
            else
                WPRINT_APP_INFO( ( "AP Interface  : Down\r\n" ) );
        }
    }
}

uint32_t host_get_time( void )
{
    return host_rtos_get_time();
}
