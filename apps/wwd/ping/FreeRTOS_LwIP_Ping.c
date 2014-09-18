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
 *
 * ICMP Ping Application using FreeRTOS/LwIP
 *
 * This is an example of a simple 'ping' (ICMP request-reply)
 * application for LwIP / FreeRTOS
 *
 * By default, the app connects using DHCP and pings the gateway
 *
 * The source is based on the Ping example app which is part
 * of the LwIP contrib package: contrib-1.4.0.rc2
 *
 */


#include "lwip/opt.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "ipv4/lwip/ip.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "wwd_network.h"
#include "wwd_management.h"
#include "wwd_wifi.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"
#include "RTOS/wwd_rtos_interface.h"

/******************************************************
 *                      Macros
 ******************************************************/
/** @cond */
#define MAKE_IPV4_ADDRESS(a, b, c, d)          ((((uint32_t) (a)) << 24) | (((uint32_t) (b)) << 16) | (((uint32_t) (c)) << 8) | ((uint32_t) (d)))

/* In release builds all UART printing is suppressed to remove remove printf and malloc dependency which reduces memory usage dramatically */
#ifndef DEBUG
#undef  WPRINT_APP_INFO
#define WPRINT_APP_INFO(args) printf args
#undef  WPRINT_APP_ERROR
#define WPRINT_APP_ERROR(args) printf args
#endif

/** @endcond */

/******************************************************
 *                    Constants
 ******************************************************/

#define AP_SSID              "YOUR_AP_SSID"
#define AP_PASS              "YOUR_AP_PASSPHRASE"
#define AP_SEC               WICED_SECURITY_WPA2_AES_PSK

/* Constants for WEP usage. NOTE: The WEP key index is a value from 0 - 3 and not 1 - 4 */
#define WEP_KEY              WEP_KEY_104   /* Demonstrates usage of WEP if AP_SEC = WICED_SECURITY_WEP_PSK                                   */
#define WEP_KEY_40           {{0,  5, {0x01,0x23,0x45,0x67,0x89}}                                           /* WEP-40  has a 10 hex byte key */
#define WEP_KEY_104          {{0, 13, {0x01,0x23,0x45,0x67,0x89,0x01,0x23,0x45,0x67,0x89,0x01,0x23,0x45}}}  /* WEP-104 has a 26 hex byte key */

#define COUNTRY              WICED_COUNTRY_AUSTRALIA
#define USE_DHCP             WICED_TRUE
#define IP_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   1,  95 )  /* Not required if USE_DHCP is WICED_TRUE */
#define GW_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   1,   1 )  /* Not required if USE_DHCP is WICED_TRUE */
#define NETMASK              MAKE_IPV4_ADDRESS( 255, 255, 255,   0 )  /* Not required if USE_DHCP is WICED_TRUE */
/* #define PING_TARGET          MAKE_IPV4_ADDRESS( 192, 168,   1, 2 ) */  /* Uncomment if you want to ping a specific IP instead of the gateway*/

#define PING_RCV_TIMEOUT     (1000)    /** ping receive timeout - in milliseconds */
#define PING_DELAY           (1000)    /** Delay between ping response/timeout and the next ping send - in milliseconds */
#define PING_ID              (0xAFAF)
#define PING_DATA_SIZE       (32)      /** ping additional data size to include in the packet */
#define JOIN_TIMEOUT         (10000)   /** timeout for joining the wireless network in milliseconds  = 10 seconds */
#define APP_THREAD_STACKSIZE (5120)


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
static void ping_prepare_echo( struct icmp_echo_hdr *iecho, uint16_t len );
static err_t ping_recv( int socket_hnd );
static err_t ping_send( int s, struct ip_addr *addr );
static void tcpip_init_done( void * arg );
static void startup_thread( void *arg );
static void app_main( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static uint16_t     ping_seq_num;
static struct netif wiced_if;
static xTaskHandle  startup_thread_handle;

static const wiced_ssid_t ap_ssid =
{
    .length = sizeof(AP_SSID)-1,
    .value  = AP_SSID,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * Main Ping app
 *
 * Initialises Wiced, joins a wireless network, then periodically pings the specified IP address forever.
 */

static void app_main( void )
{
    struct ip_addr ipaddr, netmask, gw;
    struct ip_addr target;
    int socket_hnd;
    wwd_time_t send_time;
    int recv_timeout = PING_RCV_TIMEOUT;
    wwd_result_t result;

    /* Initialise Wiced */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));
    while ( WWD_SUCCESS != ( result = wwd_management_init( COUNTRY, NULL ) ) )
    {
        WPRINT_APP_INFO(("Error %d while starting WICED!\n", result));
    }

    /* Attempt to join the Wi-Fi network */
    WPRINT_APP_INFO(("Joining : " AP_SSID "\n"));
    if (AP_SEC == WICED_SECURITY_WEP_PSK)  /* Demonstrates usage of WEP */
    {
        wiced_wep_key_t key[] = WEP_KEY;
        while ( wwd_wifi_join( &ap_ssid, AP_SEC, (uint8_t*) key, sizeof( key ), NULL ) != WWD_SUCCESS )
        {
            WPRINT_APP_INFO(("Failed to join  : " AP_SSID "   .. retrying\n"));
        }
    }
    else
    {
        while ( wwd_wifi_join( &ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, NULL ) != WWD_SUCCESS )
        {
            WPRINT_APP_INFO(("Failed to join  : " AP_SSID "   .. retrying\n"));
        }
    }
    WPRINT_APP_INFO(("Successfully joined : " AP_SSID "\n"));

    /* Setup IP configuration */
    if ( USE_DHCP == WICED_TRUE )
    {
        ip_addr_set_zero( &gw );
        ip_addr_set_zero( &ipaddr );
        ip_addr_set_zero( &netmask );
    }
    else
    {
        ipaddr.addr  = htonl( IP_ADDR );
        gw.addr      = htonl( GW_ADDR );
        netmask.addr = htonl( NETMASK );
    }

    if ( NULL == netif_add( &wiced_if, &ipaddr, &netmask, &gw, (void*) WWD_STA_INTERFACE, ethernetif_init, ethernet_input ) )
    {
        WPRINT_APP_ERROR(( "Could not add network interface\n" ));
        return;
    }

    if ( USE_DHCP == WICED_TRUE )
    {
        struct dhcp netif_dhcp;
        WPRINT_APP_INFO(("Obtaining IP address via DHCP\n"));
        dhcp_set_struct( &wiced_if, &netif_dhcp );
        dhcp_start( &wiced_if );
        while ( netif_dhcp.state != DHCP_BOUND )
        {
            /* wait */
            sys_msleep( 10 );
        }
    }
    else
    {
        netif_set_up( &wiced_if );
    }

    WPRINT_APP_INFO( ( "Network ready IP: %u.%u.%u.%u\n", (unsigned char) ( ( htonl( wiced_if.ip_addr.addr ) >> 24 ) & 0xff ),
                                                      (unsigned char) ( ( htonl( wiced_if.ip_addr.addr ) >> 16 ) & 0xff ),
                                                      (unsigned char) ( ( htonl( wiced_if.ip_addr.addr ) >>  8 ) & 0xff ),
                                                      (unsigned char) ( ( htonl( wiced_if.ip_addr.addr ) >>  0 ) & 0xff ) ) );


    /* Open a local socket for pinging */
    if ( ( socket_hnd = lwip_socket( AF_INET, SOCK_RAW, IP_PROTO_ICMP ) ) < 0 )
    {
        WPRINT_APP_ERROR(( "unable to create socket for Ping\n" ));
        return;
    }

    /* Set the receive timeout on local socket so pings will time out. */
    lwip_setsockopt( socket_hnd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof( recv_timeout ) );

#ifdef PING_TARGET
        target.addr = htonl( PING_TARGET );
#else /* ifdef PING_TARGET */
        target.addr = wiced_if.gw.addr;
#endif /* ifdef PING_TARGET */

    WPRINT_APP_INFO(("Pinging: %u.%u.%u.%u\n", (unsigned char) ( ( htonl( target.addr ) >> 24 ) & 0xff ),
                                           (unsigned char) ( ( htonl( target.addr ) >> 16 ) & 0xff ),
                                           (unsigned char) ( ( htonl( target.addr ) >>  8 ) & 0xff ),
                                           (unsigned char) ( ( htonl( target.addr ) >>  0 ) & 0xff ) ));

    /* Loop forever */
    while ( 1 )
    {
        err_t result;

        /* Send a ping */
        if ( ping_send( socket_hnd, (struct ip_addr*)&target.addr ) != ERR_OK )
        {
            WPRINT_APP_ERROR(( "Unable to send Ping\n" ));
            return;
        }

        /* Record time ping was sent */
        send_time = host_rtos_get_time( );

        /* Wait for ping reply */
        result = ping_recv( socket_hnd );
        if ( ERR_OK == result )
        {
            WPRINT_APP_INFO(("Ping Reply %dms\n", (int)( host_rtos_get_time( ) - send_time ) ));
        }
        else
        {
            WPRINT_APP_INFO(("Ping timeout\n"));
        }

        /* Sleep until time for next ping */
        sys_msleep( PING_DELAY );
    }

    /* Shutdown code - not used due to infinite loop */
#if 0
    WPRINT_APP_INFO(("Closing down\n"));
    lwip_close( socket_hnd );
    if ( USE_DHCP == WICED_TRUE )
    {
        dhcp_stop( &wiced_if );
    }
    netif_set_down( &wiced_if );
    netif_remove( &wiced_if );
    wwd_wifi_leave( );
    if ( WWD_SUCCESS != wwd_management_deinit( ) )
    {
        WPRINT_APP_ERROR(("WICED de-initialization failed\n"));
    }
#endif /* if 0 */
}



/**
 *  Prepare the contents of an echo ICMP request packet
 *
 *  @param iecho  : Pointer to an icmp_echo_hdr structure in which the ICMP packet will be constructed
 *  @param len    : The length in bytes of the packet buffer passed to the iecho parameter
 *
 */

static void ping_prepare_echo( struct icmp_echo_hdr *iecho, uint16_t len )
{
    int i;

    ICMPH_TYPE_SET( iecho, ICMP_ECHO );
    ICMPH_CODE_SET( iecho, 0 );
    iecho->chksum = 0;
    iecho->id = PING_ID;
    iecho->seqno = htons( ++ping_seq_num );

    /* fill the additional data buffer with some data */
    for ( i = 0; i < PING_DATA_SIZE; i++ )
    {
        ( (char*) iecho )[sizeof(struct icmp_echo_hdr) + i] = i;
    }

    iecho->chksum = inet_chksum( iecho, len );
}



/**
 *  Send a Ping
 *
 *  Sends a ICMP echo request (Ping) to the specified IP address, using the specified socket.
 *
 *  @param socket_hnd : The handle for the local socket through which the ping request will be sent
 *  @param addr       : The IP address to which the ping request will be sent
 *
 *  @return  ERR_OK if successfully sent, ERR_MEM if out of memory or ERR_VAL otherwise
 */

static err_t ping_send( int socket_hnd, struct ip_addr *addr )
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_in to;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

    /* Allocate memory for packet */
    if ( !( iecho = (struct icmp_echo_hdr*) mem_malloc( ping_size ) ) )
    {
        return ERR_MEM;
    }

    /* Construct ping request */
    ping_prepare_echo( iecho, ping_size );

    /* Send the ping request */
    to.sin_len = sizeof( to );
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = addr->addr;

    err = lwip_sendto( socket_hnd, iecho, ping_size, 0, (struct sockaddr*) &to, sizeof( to ) );

    /* free packet */
    mem_free( iecho );

    return ( err ? ERR_OK : ERR_VAL );
}


/**
 *  Receive a Ping reply
 *
 *  Waits for a ICMP echo reply (Ping reply) to be received using the specified socket. Compares the
 *  sequence number, and ID number to the last ping sent, and if they match, returns ERR_OK, indicating
 *  a valid ping response.
 *
 *  @param socket_hnd : The handle for the local socket through which the ping reply will be received
 *
 *  @return  ERR_OK if valid reply received, ERR_TIMEOUT otherwise
 */

static err_t ping_recv( int socket_hnd )
{
    char buf[64];
    int fromlen, len;
    struct sockaddr_in from;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;

    while ( ( len = lwip_recvfrom( socket_hnd, buf, sizeof( buf ), 0, (struct sockaddr*) &from, (socklen_t*) &fromlen ) ) > 0 )
    {
        if ( len >= (int) ( sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr) ) )
        {
            iphdr = (struct ip_hdr *) buf;
            iecho = (struct icmp_echo_hdr *) ( buf + ( IPH_HL( iphdr ) * 4 ) );

            if ( ( iecho->id == PING_ID ) &&
                 ( iecho->seqno == htons( ping_seq_num ) ) &&
                 ( ICMPH_TYPE( iecho ) == ICMP_ER ) )
            {
                return ERR_OK; /* Echo reply received - return success */
            }
        }
    }

    return ERR_TIMEOUT; /* No valid echo reply received before timeout */
}





/**
 *  Main function - creates an initial thread then starts FreeRTOS
 *  Called from the crt0 _start function
 *
 */

int main( void )
{
    /* Create an initial thread */
    xTaskCreate(startup_thread, (const signed char*)"app_thread", APP_THREAD_STACKSIZE/sizeof( portSTACK_TYPE ), NULL, DEFAULT_THREAD_PRIO, &startup_thread_handle);

    /* Start the FreeRTOS scheduler - this call should never return */
    vTaskStartScheduler( );

    /* Should never get here, unless there is an error in vTaskStartScheduler */
    WPRINT_APP_ERROR(("Main() function returned - error" ));
    return 0;
}



/**
 *  Initial thread function - Starts LwIP and calls app_main
 *
 *  This function starts up LwIP using the tcpip_init function, then waits on a semaphore
 *  until LwIP indicates that it has started by calling the callback @ref tcpip_init_done.
 *  Once that has been done, the @ref app_main function of the app is called.
 *
 * @param arg : Unused - required for conformance to thread function prototype
 */

static void startup_thread( void *arg )
{
    xSemaphoreHandle lwip_done_sema;
    UNUSED_PARAMETER( arg);

    WPRINT_APP_INFO( ( "\nPlatform " PLATFORM " initialised\n" ) );
    WPRINT_APP_INFO( ( "Started FreeRTOS" FreeRTOS_VERSION "\n" ) );
    WPRINT_APP_INFO( ( "Starting LwIP " LwIP_VERSION "\n" ) );

    /* Create a semaphore to signal when LwIP has finished initialising */
    lwip_done_sema = xSemaphoreCreateCounting( 1, 0 );
    if ( lwip_done_sema == NULL )
    {
        /* could not create semaphore */
        WPRINT_APP_ERROR(("Could not create LwIP init semaphore"));
        return;
    }

    /* Initialise LwIP, providing the callback function and callback semaphore */
    tcpip_init( tcpip_init_done, (void*) &lwip_done_sema );
    xSemaphoreTake( lwip_done_sema, portMAX_DELAY );
    vQueueDelete( lwip_done_sema );

    /* Run the main application function */
    app_main( );

    /* Clean up this startup thread */
    vTaskDelete( startup_thread_handle );
}



/**
 *  LwIP init complete callback
 *
 *  This function is called by LwIP when initialisation has finished.
 *  A semaphore is posted to allow the startup thread to resume, and to run the app_main function
 *
 * @param arg : the handle for the semaphore to post (cast to a void pointer)
 */

static void tcpip_init_done( void * arg )
{
    xSemaphoreHandle * lwip_done_sema = (xSemaphoreHandle *) arg;
    xSemaphoreGive( *lwip_done_sema );
}
