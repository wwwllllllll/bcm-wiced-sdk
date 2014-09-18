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
 * Wi-Fi Scan Application using FreeRTOS/LwIP
 *
 * This application scans for Wi-Fi Networks in range and prints
 * scan results to a console. A new scan is initiated at 5 second
 * intervals.
 *
 * The application works with FreeRTOS / LwIP
 *
 */

#include <stdio.h>
#include <string.h>
#include "wwd_management.h"
#include "wwd_wifi.h"
#include "wwd_debug.h"
#include "wwd_events.h"
#include "wwd_assert.h"
#include "RTOS/wwd_rtos_interface.h"
#include "platform/wwd_platform_interface.h"
#include "lwip/tcpip.h"

/******************************************************
 *                      Macros
 ******************************************************/
/** @cond */
/* Macros for comparing MAC addresses */
#define CMP_MAC( a, b )  (((a[0])==(b[0]))&& \
                          ((a[1])==(b[1]))&& \
                          ((a[2])==(b[2]))&& \
                          ((a[3])==(b[3]))&& \
                          ((a[4])==(b[4]))&& \
                          ((a[5])==(b[5])))

#define NULL_MAC( a )  (((a[0])==0)&& \
                        ((a[1])==0)&& \
                        ((a[2])==0)&& \
                        ((a[3])==0)&& \
                        ((a[4])==0)&& \
                        ((a[5])==0))
/** @endcond */

/******************************************************
 *                    Constants
 ******************************************************/

#define COUNTRY                       WICED_COUNTRY_AUSTRALIA
#define CIRCULAR_RESULT_BUFF_SIZE     (40)
#define APP_THREAD_STACKSIZE          (4096)

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
static void tcpip_init_done( void* arg );
static void startup_thread( void* arg );
static void scan_results_handler( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status );

/******************************************************
 *               Variable Definitions
 ******************************************************/
static xTaskHandle             startup_thread_handle;
static wiced_mac_t             bssid_list[200]; /* List of BSSID (AP MAC addresses) that have been scanned */
static host_semaphore_type_t   num_scan_results_semaphore;
static wiced_scan_result_t     result_buff[CIRCULAR_RESULT_BUFF_SIZE];
static uint16_t                result_buff_write_pos = 0;
static uint16_t                result_buff_read_pos  = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * Main Scan app
 *
 * Initialises Wiced, starts a scan, waits till scan is finished,
 * fetches scan results and prints them.
 */
static void app_main( void )   /*@globals killed num_scan_results_semaphore, undef num_scan_results_semaphore, result_buff@*/ /*@modifies bssid_list, result_buff_read_pos, result_buff_write_pos @*/
{
    wiced_scan_result_t * result_ptr = (wiced_scan_result_t *) &result_buff;
    wiced_scan_result_t * record;
    int                   record_number;
    wwd_result_t          result;

    /* Initialise Wiced */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));
    while ( WWD_SUCCESS != ( result = wwd_management_init( COUNTRY, NULL ) ) )
    {
        WPRINT_APP_INFO(("Error %d while starting WICED!\n", (int) result));
    }

    host_rtos_init_semaphore( &num_scan_results_semaphore );

    /* Repeatedly Scan forever */ /*@-infloops@*/
    while ( 1 == 1 )
    {
        record_number = 1;

        /* Clear list of BSSID addresses already parsed */
        memset( &bssid_list, 0, sizeof( bssid_list ) );

        /* Start Scan */
        WPRINT_APP_INFO(("Starting Scan\n"));

        /* Examples of scan parameter values */
#if 0
        wiced_ssid_t ssid = { 12,  "YOUR_AP_SSID" };                     /* Scan for networks named YOUR_AP_SSID only */
        wiced_mac_t mac = {{0x00,0x1A,0x30,0xB9,0x4E,0x90}};             /* Scan for Access Point with the specified BSSID */
        uint16_t chlist[] = { 6, 0 };                                    /* Scan channel 6 only */
        wiced_scan_extended_params_t extparam = { 5, 1000, 1000, 100 };  /* Spend much longer scanning (1 second and 5 probe requests per channel) */
#endif /* if 0 */

        result_buff_read_pos = 0;
        result_buff_write_pos = 0;

        if ( WWD_SUCCESS != wwd_wifi_scan( WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, NULL, NULL, NULL, NULL, scan_results_handler, (wiced_scan_result_t **) &result_ptr, NULL, WWD_STA_INTERFACE ) )
        {
            WPRINT_APP_ERROR(("Error starting scan\n"));
            host_rtos_deinit_semaphore( &num_scan_results_semaphore );
            return;
        }
        WPRINT_APP_INFO(("Waiting for scan results...\n"));

        while ( host_rtos_get_semaphore( &num_scan_results_semaphore, NEVER_TIMEOUT, WICED_FALSE ) == WWD_SUCCESS )
        {
            int k;

            record = &result_buff[result_buff_read_pos];

            /*TODO: change 0xff to a defined flag */
            if ( record->channel == (uint8_t) 0xff )
            {
                /* Scan completed */
                break;
            }

            /* Print SSID */
            WPRINT_APP_INFO(("\n#%03d SSID          : ", record_number ));
            for ( k = 0; k < (int)record->SSID.length; k++ )
            {
                WPRINT_APP_INFO(("%c",(char)record->SSID.value[k]));
            }
            WPRINT_APP_INFO(("\n" ));

            wiced_assert( "error", (record->bss_type==WICED_BSS_TYPE_INFRASTRUCTURE)||(record->bss_type==WICED_BSS_TYPE_ADHOC));

            /* Print other network characteristics */
            WPRINT_APP_INFO(("     BSSID         : %02X:%02X:%02X:%02X:%02X:%02X\n", (unsigned int) record->BSSID.octet[0],
                                                                                       (unsigned int) record->BSSID.octet[1],
                                                                                       (unsigned int) record->BSSID.octet[2],
                                                                                       (unsigned int) record->BSSID.octet[3],
                                                                                       (unsigned int) record->BSSID.octet[4],
                                                                                       (unsigned int) record->BSSID.octet[5] ));
            WPRINT_APP_INFO(("     RSSI          : %ddBm", (int)record->signal_strength ));
            if (record->on_channel == WICED_FALSE)
            {
                WPRINT_APP_INFO((" (off-channel)\n"));
            }
            else
            {
                WPRINT_APP_INFO(("\n"));
            }
            WPRINT_APP_INFO(("     Max Data Rate : %.1f Mbits/s\n", (float)record->max_data_rate /1000.0));
            WPRINT_APP_INFO(("     Network Type  : %s\n", (record->bss_type==WICED_BSS_TYPE_INFRASTRUCTURE)?"Infrastructure":(record->bss_type==WICED_BSS_TYPE_ADHOC)?"Ad hoc":"Unknown" ));
            WPRINT_APP_INFO(("     Security      : %s\n", (record->security==WICED_SECURITY_OPEN)?"Open":
                                                      (record->security==WICED_SECURITY_WEP_PSK)?"WEP":
                                                      (record->security==WICED_SECURITY_WPA_TKIP_PSK)?"WPA":
                                                      (record->security==WICED_SECURITY_WPA2_AES_PSK)?"WPA2 AES":
                                                      (record->security==WICED_SECURITY_WPA2_MIXED_PSK)?"WPA2 Mixed":"Unknown" ));
            WPRINT_APP_INFO(("     Radio Band    : %s\n", (record->band==WICED_802_11_BAND_5GHZ)?"5GHz":"2.4GHz" ));
            WPRINT_APP_INFO(("     Channel       : %d\n", (int) record->channel ));
            result_buff_read_pos++;
            if ( result_buff_read_pos >= (uint16_t) CIRCULAR_RESULT_BUFF_SIZE )
            {
                result_buff_read_pos = 0;
            }
            record_number++;

        }

        /* Done! */
        WPRINT_APP_INFO(("\nEnd of scan results - next scan in 5 seconds\n"));

        /* Wait 5 seconds till next scan */
        (void) host_rtos_delay_milliseconds( (uint32_t) 5000 );
    }
    /*@+infloops@*/

/**
 *  Not required due to while (1)
 *
 *  host_rtos_deinit_semaphore( &num_scan_results_semaphore );
 */
}

/**
 *  Main function - creates an initial thread then starts FreeRTOS
 *  Called from the crt0 _start function
 *
 */
int main( void )
{
    signed portBASE_TYPE create_result;

    /* Create an initial thread */
    create_result = xTaskCreate(startup_thread, (signed char*)"app_thread", (unsigned short) (APP_THREAD_STACKSIZE / sizeof( portSTACK_TYPE )), NULL, DEFAULT_THREAD_PRIO, &startup_thread_handle);

    wiced_assert("Failed to create main thread", create_result == pdPASS );

    /* Start the FreeRTOS scheduler - this call should never return */
    vTaskStartScheduler( );

    /* Should never get here, unless there is an error in vTaskStartScheduler */
    WPRINT_APP_ERROR(("Main() function returned - error" ));
    return 0;
}

/**
 *  Initial thread function - Starts LwIP and calls app_main
 *
 *  Although this app does not use a network stack, LwIP is still required, since
 *  it provides the packet buffers which Wiced uses for IOCTL commands.
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
    WPRINT_APP_INFO( ( "Started FreeRTOS " FreeRTOS_VERSION "\n" ) );
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

/**
 *  Scan result callback
 *  Called whenever a scan result is available
 *
 *  @param result_ptr : pointer to pointer for location where result is stored. The inner pointer
 *                      can be updated to cause the next result to be put in a new location.
 *  @param user_data : unused
 */
static void scan_results_handler( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status )
{
    if ( result_ptr == NULL )
    {
        /* finished */
        result_buff[result_buff_write_pos].channel = 0xff;
        host_rtos_set_semaphore( &num_scan_results_semaphore, WICED_FALSE );
        return;
    }

    wiced_scan_result_t* record = ( *result_ptr );

    /* Check the list of BSSID values which have already been printed */
    wiced_mac_t * tmp_mac = bssid_list;
    while ( NULL_MAC( tmp_mac->octet ) == WICED_FALSE )
    {
        if ( CMP_MAC( tmp_mac->octet, record->BSSID.octet ) == WICED_TRUE )
        {
            /* already seen this BSSID */
            return;
        }
        tmp_mac++;
    }

    /* New BSSID - add it to the list */
    memcpy( &tmp_mac->octet, record->BSSID.octet, sizeof(wiced_mac_t) );

    /* Add the result to the list and set the pointer for the next result */
    result_buff_write_pos++;
    if ( result_buff_write_pos >= CIRCULAR_RESULT_BUFF_SIZE )
    {
        result_buff_write_pos = 0;
    }
    *result_ptr = &result_buff[result_buff_write_pos];
    host_rtos_set_semaphore( &num_scan_results_semaphore, WICED_FALSE );

    wiced_assert( "Circular result buffer overflow", result_buff_write_pos != result_buff_read_pos );
}

