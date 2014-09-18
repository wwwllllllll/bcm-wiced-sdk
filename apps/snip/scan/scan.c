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
 *
 * Scan Application
 *
 * Features demonstrated
 *  - WICED scan API
 *
 * This application snippet regularly scans for nearby Wi-Fi access points
 *
 * Application Instructions
 *   Connect a PC terminal to the serial port of the WICED Eval board,
 *   then build and download the application as described in the WICED
 *   Quick Start Guide
 *
 *   Each time the application scans, a list of Wi-Fi access points in
 *   range is printed to the UART
 *
 */

#include <stdlib.h>
#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define DELAY_BETWEEN_SCANS       (5000)

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

wiced_result_t scan_result_handler( wiced_scan_handler_result_t* malloced_scan_result );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static int record_count;
static wiced_time_t scan_start_time;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    wiced_init( );

    while(1)
    {
        record_count = 0;
        WPRINT_APP_INFO( ( "Waiting for scan results...\n" ) );
        WPRINT_APP_INFO( ("  # Type  BSSID             RSSI  Rate Chan  Security    SSID\n" ) );
        WPRINT_APP_INFO( ("----------------------------------------------------------------------------------------------\n" ) );
        wiced_time_get_time(&scan_start_time);
        wiced_wifi_scan_networks(scan_result_handler, NULL );
        wiced_rtos_delay_milliseconds(DELAY_BETWEEN_SCANS);
    }
}

/*
 * Callback function to handle scan results
 */
wiced_result_t scan_result_handler( wiced_scan_handler_result_t* malloced_scan_result )
{
    malloc_transfer_to_curr_thread( malloced_scan_result );

    if ( malloced_scan_result->status == WICED_SCAN_INCOMPLETE )
    {
        wiced_scan_result_t* record = &malloced_scan_result->ap_details;
        record->SSID.value[record->SSID.length] = 0; /* Ensure the SSID is null terminated */

        WPRINT_APP_INFO( ( "%3d ", record_count ) );
        print_scan_result(record);
        ++record_count;
    }
    else
    {
        wiced_time_t scan_end_time;
        wiced_time_get_time(&scan_end_time);
        WPRINT_APP_INFO( ("\nScan complete in %lu milliseconds\n", scan_end_time - scan_start_time) );
    }

    free( malloced_scan_result );

    return WICED_SUCCESS;
}
