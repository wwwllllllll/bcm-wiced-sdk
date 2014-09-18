/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "wwd_wifi.h"
#include "wiced_management.h"
#include "wiced_network.h"
#include "wwd_network.h"
#include "network/wwd_buffer_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "lwip/opt.h"
#include "lwip/mem.h"
#include <string.h>
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/sockets.h"  /* equivalent of <sys/socket.h> */
#include "lwip/inet.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
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
 *  Initial thread function - Starts LwIP and calls console_app_main
 *
 *  This function starts up LwIP using the tcpip_init function, then waits on a semaphore
 *  until LwIP indicates that it has started by calling the callback @ref tcpip_init_done.
 *  Once that has been done, the @ref console_app_main function of the app is called.
 *
 * @param arg : Unused - required for conformance to thread function prototype
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
    struct ip_addr ipaddr, netmask, gw;
    if ( argc < 4 )
    {
        return ERR_UNKNOWN;
    }
    ipaddr.addr  = str_to_ip(argv[1]);
    netmask.addr = str_to_ip(argv[2]);
    gw.addr      = str_to_ip(argv[3]);
    netif_set_addr( &wiced_ip_handle[WICED_STA_INTERFACE], &ipaddr, &netmask, &gw );
    return ERR_CMD_OK;
}

void network_print_status( char* sta_ssid, char* ap_ssid )
{
    uint8_t interface;
    for ( interface = 0; interface <= 1; interface++ )
    {
        if ( wwd_wifi_is_ready_to_transceive( (wwd_interface_t) interface ) == WWD_SUCCESS )
        {
            if ( interface == WWD_STA_INTERFACE )
            {
                WPRINT_APP_INFO( ( "STA Interface\r\n"));
                WPRINT_APP_INFO( ( "   SSID       : %s\r\n", sta_ssid ) );
            }
            else
            {
                WPRINT_APP_INFO( ( "AP Interface\r\n"));
                WPRINT_APP_INFO( ( "   SSID       : %s\r\n", ap_ssid ) );

            }
            if ( netif_is_up(&wiced_ip_handle[interface]) )
            {
                WPRINT_APP_INFO( ( "   IP Addr    : %u.%u.%u.%u\r\n", (unsigned char) ( ( htonl( wiced_ip_handle[interface].ip_addr.addr ) >> 24 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].ip_addr.addr ) >> 16 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].ip_addr.addr ) >> 8 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].ip_addr.addr ) >> 0 ) & 0xff ) ) );
                WPRINT_APP_INFO( ( "   Gateway    : %u.%u.%u.%u\r\n", (unsigned char) ( ( htonl( wiced_ip_handle[interface].gw.addr ) >> 24 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].gw.addr ) >> 16 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].gw.addr ) >> 8 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].gw.addr ) >> 0 ) & 0xff ) ) );
                WPRINT_APP_INFO( ( "   Netmask    : %u.%u.%u.%u\r\n", (unsigned char) ( ( htonl( wiced_ip_handle[interface].netmask.addr ) >> 24 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].netmask.addr ) >> 16 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].netmask.addr ) >> 8 ) & 0xff ), (unsigned char) ( ( htonl( wiced_ip_handle[interface].netmask.addr ) >> 0 ) & 0xff ) ) );
            }
        }
        else
        {
            if ( interface == WWD_STA_INTERFACE )
                WPRINT_APP_INFO( ( "STA Interface : Down\r\n") );
            else
                WPRINT_APP_INFO( ( "AP Interface  : Down\r\n") );
        }
    }
}

uint32_t host_get_time( void )
{
    return host_rtos_get_time();
}
