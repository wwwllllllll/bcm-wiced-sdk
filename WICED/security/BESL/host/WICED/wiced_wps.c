/*
 * WICED WPS host implementation
 *
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 */

/******************************************************
 *            Includes
 ******************************************************/

#include "wps_host.h"
#include "network/wwd_buffer_interface.h"
#include "network/wwd_network_interface.h"
#include "network/wwd_network_constants.h"
#include "string.h"
#include "wps_host_interface.h"
#include "wwd_wifi.h"
#include "wwd_crypto.h"
#include "wwd_events.h"
#include "stdlib.h"
#include "wps_constants.h"
#include "wps_common.h"
#include "wiced_rtos.h"
#include "wiced_time.h"
#include "wps_p2p_interface.h"
#include "besl_host.h"
#include "besl_host_interface.h"
#include "internal/wwd_sdpcm.h"
#include "wiced_wps.h"

/******************************************************
 *             Constants
 ******************************************************/

#define EAPOL_HEADER_SPACE       (sizeof(ether_header_t) + sizeof(eapol_header_t))

#define WL_CHANSPEC_CHAN_MASK    0x00ff

#define WPS_THREAD_STACK_SIZE  (8*1024)
#define WPS_THREAD_PRIORITY    3

#define WPS_TOTAL_MAX_TIME   (120*1000) /* In milliseconds */

#define AUTHORIZED_MAC_LIST_LENGTH   (1)

#define DEFAULT_WPS_JOIN_TIMEOUT   (1500)

#define WPS_PBC_OVERLAP_WINDOW      (120*1000) /* In milliseconds */

#define ACTIVE_WPS_WORKSPACE_ARRAY_SIZE   (3)

/******************************************************
 *             Macros
 ******************************************************/

#define IF_TO_WORKSPACE( interface )   ( active_wps_workspaces[ (((interface)==WWD_STA_INTERFACE)?0:1) ] )     /* STA = 0,  AP = 1 */

/******************************************************
 *             Local Structures
 ******************************************************/

typedef void (*wps_scan_handler_t)(wl_escan_result_t* result, void* user_data);

typedef struct
{
    host_thread_type_t      wps_thread;
    void*                   wps_thread_stack;
    host_semaphore_type_t*  wps_thread_semaphore;
    host_semaphore_type_t   event_semaphore;
    host_queue_type_t       event_queue;
    wps_event_message_t     event_buffer[10];
    wiced_time_t            timer_reference;
    uint32_t                timer_timeout;

    wwd_interface_t         interface;

    union
    {
        struct
        {
            besl_wps_credential_t*  enrollee_output;
            uint16_t                enrollee_output_length;
            uint16_t                stored_credential_count;
            wps_ap_t                ap_list[AP_LIST_SIZE];
            uint8_t                 ap_list_counter;
            wps_scan_handler_t      scan_handler_ptr;
            wps_credential_t*       credential_list[CREDENTIAL_LIST_LENGTH];
        } enrollee;
        struct
        {
            const besl_wps_credential_t* ap_details;
            besl_mac_t                   authorized_mac_list[AUTHORIZED_MAC_LIST_LENGTH];
        } registrar;
    } stuff;
} wps_host_workspace_t;

typedef struct
{
    wiced_time_t   probe_request_rx_time;
    besl_mac_t     probe_request_mac;
} wps_pbc_overlap_record_t;

/******************************************************
 *             Static Variables
 ******************************************************/

/* Active WPS workspaces.
 * One for each interface */
static wps_agent_t* active_wps_workspaces[ACTIVE_WPS_WORKSPACE_ARRAY_SIZE] = {0,0,0};

static       wps_pbc_overlap_record_t pbc_overlap_array[2] = { { 0 } };
static       wps_pbc_overlap_record_t last_pbc_enrollee    = { 0 };
static const wwd_event_num_t        wps_events[]         = { WLC_E_PROBREQ_MSG, WLC_E_NONE };

/******************************************************
 *             Static Function Prototypes
 ******************************************************/

static void          wiced_wps_thread             ( uint32_t arg );
static void          scan_result_handler          ( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status );
static void*         wps_softap_event_handler     ( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data );
static void          wps_update_pbc_overlap_array ( const besl_mac_t* mac );
static besl_result_t besl_wps_pbc_overlap_check   ( const besl_mac_t* mac );

/******************************************************
 *             Function definitions
 ******************************************************/

besl_result_t besl_wps_init(wps_agent_t* workspace, const besl_wps_device_detail_t* details, wps_agent_type_t type, wwd_interface_t interface )
{
    wps_host_workspace_t* host_workspace;

    memset(workspace, 0, sizeof(wps_agent_t));
    host_workspace = besl_host_malloc("wps", sizeof(wps_host_workspace_t));
    if (host_workspace == NULL)
    {
        return BESL_OUT_OF_HEAP_SPACE;
    }
    memset(host_workspace, 0, sizeof(wps_host_workspace_t));
    workspace->wps_host_workspace = host_workspace;

#ifdef RTOS_USE_STATIC_THREAD_STACK
    host_workspace->wps_thread_stack = besl_host_malloc("wps stack", WPS_THREAD_STACK_SIZE);
    if (host_workspace->wps_thread_stack == NULL)
    {
        besl_host_free(workspace->wps_host_workspace);
        workspace->wps_host_workspace = NULL;
        return BESL_ERROR_WPS_STACK_MALLOC_FAIL;
    }
#else
    host_workspace->wps_thread_stack = NULL;
#endif

    host_workspace->interface = interface;
    workspace->interface      = interface;
    workspace->agent_type     = type;
    workspace->device_details = details;
    workspace->wps_result     = WPS_NOT_STARTED;
    workspace->wps_mode       = WPS_PIN_MODE;

    host_rtos_init_queue( &host_workspace->event_queue, host_workspace->event_buffer, sizeof( host_workspace->event_buffer ), sizeof(wps_event_message_t) );

    wps_init_workspace( workspace );

    if ( workspace->agent_type == WPS_REGISTRAR_AGENT )
    {
        wps_advertise_registrar( workspace, 0 );

        /* Init the authorized mac list with the broadcast address */
        memset(&host_workspace->stuff.registrar.authorized_mac_list[0], 0xFF, sizeof(besl_mac_t));
    }

    return WICED_SUCCESS;
}

besl_result_t besl_wps_management_set_event_handler( wps_agent_t* workspace, wiced_bool_t enable )
{
    wwd_result_t        result;

    /* Add WPS event handler */
    if ( enable == WICED_TRUE )
    {
        result = wwd_management_set_event_handler( wps_events, wps_softap_event_handler, workspace, WWD_AP_INTERFACE );
    }
    else
    {
        result = wwd_management_set_event_handler( wps_events, NULL, workspace, WWD_AP_INTERFACE );
    }

    if ( result != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Error setting event %u\n", (unsigned int)result));
    }
    return result;
}

besl_result_t besl_wps_get_result( wps_agent_t* workspace )
{
    return workspace->wps_result;
}

besl_result_t besl_wps_deinit( wps_agent_t* workspace )
{
    wps_host_workspace_t* host_workspace = (wps_host_workspace_t*)workspace->wps_host_workspace;

    besl_wps_wait_till_complete( workspace );

    wps_deinit_workspace(workspace);

    if ( host_workspace != NULL )
    {
        /* Delete the WPS thread */
        if ( workspace->wps_result != WPS_NOT_STARTED )
        {
            host_rtos_delete_terminated_thread( &host_workspace->wps_thread );
        }

        if ( host_workspace->wps_thread_stack != NULL )
        {
            besl_host_free( host_workspace->wps_thread_stack );
            host_workspace->wps_thread_stack = NULL;
        }

        host_rtos_deinit_queue( &host_workspace->event_queue );
        besl_host_free( host_workspace );
        workspace->wps_host_workspace = NULL;
    }
    return BESL_SUCCESS;
}

besl_result_t besl_wps_start( wps_agent_t* workspace, besl_wps_mode_t mode, const char* password, besl_wps_credential_t* credentials, uint16_t credential_length )
{
    besl_result_t result;
    wps_host_workspace_t* host_workspace = (wps_host_workspace_t*) workspace->wps_host_workspace;

    if ( ( workspace->agent_type == WPS_REGISTRAR_AGENT ) &&
         ( besl_wps_pbc_overlap_check( NULL ) == BESL_PBC_OVERLAP ) && ( mode == BESL_WPS_PBC_MODE ) )
    {
        return BESL_PBC_OVERLAP;
    }

    result = wps_internal_init( workspace, workspace->interface, mode, password, credentials, credential_length );
    if ( result == BESL_SUCCESS )
    {
        result = host_rtos_create_thread_with_arg( &host_workspace->wps_thread, wiced_wps_thread, "wps", host_workspace->wps_thread_stack, WPS_THREAD_STACK_SIZE, RTOS_HIGHER_PRIORTIY_THAN(RTOS_DEFAULT_THREAD_PRIORITY), (uint32_t) workspace );
    }

    return result;
}

besl_result_t besl_wps_restart( wps_agent_t* workspace )
{
    workspace->start_time = host_rtos_get_time( );
    return BESL_SUCCESS;
}

besl_result_t wps_internal_init( wps_agent_t* workspace, uint32_t interface, besl_wps_mode_t mode, const char* password, besl_wps_credential_t* credentials, uint16_t credential_length )
{
    wps_host_workspace_t* host_workspace = (wps_host_workspace_t*)workspace->wps_host_workspace;

    workspace->password = password;
    workspace->wps_mode = mode;

    if (workspace->agent_type == WPS_ENROLLEE_AGENT)
    {
        host_workspace->stuff.enrollee.enrollee_output        = credentials;
        host_workspace->stuff.enrollee.enrollee_output_length = credential_length;
    }
    else
    {
        if (credentials == NULL)
        {
            return WICED_BADARG;
        }
        host_workspace->stuff.registrar.ap_details = credentials;
    }

    return BESL_SUCCESS;
}

besl_result_t besl_wps_wait_till_complete( wps_agent_t* workspace )
{
    if ( workspace->wps_result != WPS_NOT_STARTED )
    {
        wps_host_workspace_t* host_workspace = (wps_host_workspace_t*) workspace->wps_host_workspace;
        host_rtos_join_thread( &host_workspace->wps_thread );
    }
    return BESL_SUCCESS;
}

besl_result_t besl_wps_abort( wps_agent_t* workspace )
{
    wps_host_workspace_t* host      = (wps_host_workspace_t*)workspace->wps_host_workspace;
    wps_event_message_t   message;

    message.event_type = WPS_EVENT_ABORT_REQUESTED;
    return host_rtos_push_to_queue(&host->event_queue, &message, WICED_NEVER_TIMEOUT);
}

besl_result_t besl_wps_scan( wps_agent_t* workspace, wps_ap_t** ap_array, uint16_t* ap_array_size )
{
    wwd_result_t          result;
    wps_event_message_t   message;
    wps_host_workspace_t* host = (wps_host_workspace_t*) workspace->wps_host_workspace;

    /* Verify that WPS hasn't started */
    if ( workspace->wps_result != WPS_NOT_STARTED )
    {
        return BESL_ERROR_ALREADY_STARTED;
    }

    IF_TO_WORKSPACE( WWD_STA_INTERFACE ) = workspace;
    memset(host->stuff.enrollee.ap_list, 0, sizeof(host->stuff.enrollee.ap_list));
    wps_host_scan( workspace, wps_scan_result_handler );

    do
    {
        result = host_rtos_pop_from_queue( &host->event_queue, &message, 5000 );
    } while ( result == WWD_SUCCESS && message.event_type != WPS_EVENT_DISCOVER_COMPLETE );

    IF_TO_WORKSPACE( WWD_STA_INTERFACE ) = NULL;

    if ( result == WWD_SUCCESS )
    {
        *ap_array = host->stuff.enrollee.ap_list;
        *ap_array_size = host->stuff.enrollee.ap_list_counter;
    }

    return (besl_result_t) result;
}

besl_result_t besl_wps_set_directed_wps_target( wps_agent_t* workspace, wps_ap_t* ap, uint32_t maximum_join_attempts )
{
    workspace->directed_wps_max_attempts = maximum_join_attempts;
    workspace->ap = ap;

    return BESL_SUCCESS;
}

static besl_result_t besl_wps_pbc_overlap_check( const besl_mac_t* mac )
{
    if ( wps_pbc_overlap_check( mac ) != WPS_SUCCESS )
    {
        return BESL_PBC_OVERLAP;
    }
    return BESL_SUCCESS;
}

static void wiced_wps_thread( uint32_t arg )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)((wps_agent_t*)arg)->wps_host_workspace;

    wiced_wps_thread_main( arg );

    host_rtos_finish_thread( &host->wps_thread );
    WICED_END_OF_CURRENT_THREAD( );
}

void wiced_wps_thread_main( uint32_t arg )
{
    wiced_time_t          current_time;
    wps_result_t          result;
    wps_event_message_t   message;
    wps_agent_t*          workspace = (wps_agent_t*)arg;
    wps_host_workspace_t* host      = (wps_host_workspace_t*)workspace->wps_host_workspace;

    workspace->wps_result = WPS_IN_PROGRESS;

    /* Now that our queue is initialized we can flag the workspace as active */
    IF_TO_WORKSPACE( workspace->interface ) = workspace;

    wps_prepare_workspace_crypto( workspace );

    /* Start 120 second timer */
    workspace->start_time = host_rtos_get_time( );

    if ( workspace->agent_type == WPS_REGISTRAR_AGENT )
    {
        WPS_INFO(("Starting WPS Registrar\n"));
        wps_registrar_start( workspace );

        /* Allow clients to connect to AP using Open authentication for WPS handshake */
        wiced_buffer_t buffer;
        uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSSCFG_WSEC );
        BESL_WRITE_32(&data[0], CHIP_AP_INTERFACE);
        BESL_WRITE_32(&data[1], ( host->stuff.registrar.ap_details->security | SES_OW_ENABLED ));
        wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );
    }
    else
    {
        WPS_INFO(("Starting WPS Enrollee\n"));
        wps_enrollee_start( workspace );
    }

    while ( workspace->wps_result == WPS_IN_PROGRESS )
    {
        uint32_t     time_to_wait;
        wiced_bool_t waiting_for_event = WICED_FALSE;

        current_time = host_rtos_get_time( );

        if (( current_time - workspace->start_time ) >= 2 * MINUTES )
        {
            workspace->wps_result = WPS_TIMEOUT;
            continue;
        }

        time_to_wait = ( 2 * MINUTES ) - ( current_time - workspace->start_time );
        if ( host->timer_timeout != 0 )
        {
            waiting_for_event = WICED_TRUE;
            time_to_wait = MIN( time_to_wait, host->timer_timeout - (current_time - host->timer_reference));
        }

        if ( host_rtos_pop_from_queue( &host->event_queue, &message, time_to_wait ) != WWD_SUCCESS )
        {
            /* Create a timeout message */
            message.event_type = WPS_EVENT_TIMER_TIMEOUT;
            message.data.value = 0;
        }

        /* Process the message */
        result = wps_process_event( workspace, &message );
        if ( result != WPS_SUCCESS )
        {
            if ( result == WPS_ATTEMPTED_EXTERNAL_REGISTRAR_DISCOVERY )
            {
                WPS_INFO(("Client attempted external registrar discovery\n"));
            }
            else
            {
                if ( waiting_for_event == WICED_TRUE )
                {
                    int32_t time_left;
                    current_time = host_rtos_get_time( );
                    time_left = MAX( ( ( 2 * MINUTES ) - ( current_time - workspace->start_time ) )/1000, 0);
                    WPS_INFO(( "WPS Procedure failed. Restarting with %li seconds left\n", (long)time_left));
                    REFERENCE_DEBUG_ONLY_VARIABLE( time_left );
                }
            }

            /* Reset the agent type if we were in reverse registrar mode */
            if (workspace->in_reverse_registrar_mode != 0)
            {
                workspace->agent_type = WPS_REGISTRAR_AGENT;
            }

            if ( workspace->agent_type == WPS_ENROLLEE_AGENT )
            {
                wps_host_leave();
            }

            wps_reset_workspace( workspace );
        }
    }

    /* Remove workspace from list of active workspaces */
    IF_TO_WORKSPACE( workspace->interface ) = NULL;

    /* Print result (if enabled) */
    if ( workspace->wps_result == WPS_COMPLETE )
    {
        WPS_INFO(( "WPS completed successfully\n" ));
        workspace->wps_result = WPS_SUCCESS;
    }
    else if ( workspace->wps_result == WPS_PBC_OVERLAP )
    {
        WPS_INFO(( "PBC overlap detected - wait and try again\n" ));
    }
    else if ( workspace->wps_result == WPS_ABORTED )
    {
        WPS_INFO(( "WPS aborted\r\n" ));
    }
    else
    {
        WPS_INFO(( "WPS timed out\n" ));
    }

    /* De-init the workspace */
    wps_deinit_workspace( workspace );

    if (workspace->agent_type == WPS_REGISTRAR_AGENT)
    {
        /* Remove ability for clients to connect to AP using Open authentication */
        wiced_buffer_t buffer;
        uint32_t* data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSSCFG_WSEC );
        BESL_WRITE_32(&data[0], CHIP_AP_INTERFACE);
        BESL_WRITE_32(&data[1], ( host->stuff.registrar.ap_details->security ));
        wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );

        /* Re-advertise WPS. The IEs are removed in wps_deinit_workspace() */
        wps_advertise_registrar( workspace, 0 );

        /* Init the authorized mac list with the broadcast address */
        memset(&((wps_host_workspace_t*)workspace->wps_host_workspace)->stuff.registrar.authorized_mac_list[0], 0xFF, sizeof(besl_mac_t));
    }
    else
    {
        wps_host_leave( );
    }

    /* Clean up left over messages in the event queue */
    while ( host_rtos_pop_from_queue( &host->event_queue, &message, 0 ) == WWD_SUCCESS )
    {
        if (message.event_type == WPS_EVENT_EAPOL_PACKET_RECEIVED)
        {
            wps_host_free_eapol_packet(message.data.packet);
        }
    }

    /* Clean up the WPS thread */
    host_rtos_finish_thread( &host->wps_thread );

    WICED_END_OF_CURRENT_THREAD( );
}

void host_network_process_eapol_data( /*@only@*/ wiced_buffer_t buffer, wwd_interface_t interface )
{
    wps_agent_t* workspace = IF_TO_WORKSPACE( interface );
    if ( workspace != NULL )
    {
        wps_event_message_t message;
        message.event_type = WPS_EVENT_EAPOL_PACKET_RECEIVED;
        message.data.packet = buffer;
        if ( host_rtos_push_to_queue( &( (wps_host_workspace_t*) workspace->wps_host_workspace )->event_queue, &message, WICED_NEVER_TIMEOUT ) != WWD_SUCCESS )
        {
            host_buffer_release( buffer, WWD_NETWORK_RX );
        }
    }
    else
    {
        host_buffer_release( buffer, WWD_NETWORK_RX );
    }
}

/******************************************************
 *               WPS Host API Definitions
 ******************************************************/

void wps_host_create_eapol_packet(wps_eapol_packet_t* packet, uint16_t size)
{
    if (host_buffer_get((wiced_buffer_t*)packet, WWD_NETWORK_TX, size + WICED_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX, WICED_TRUE) != WWD_SUCCESS)
    {
        *packet = 0;
        return;
    }
    host_buffer_add_remove_at_front((wiced_buffer_t*)packet, WICED_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX);
}

uint8_t* wps_host_get_eapol_data(wps_eapol_packet_t packet)
{
    return host_buffer_get_current_piece_data_pointer(packet);
}

uint16_t wps_host_get_eapol_packet_size(wps_eapol_packet_t packet)
{
    return host_buffer_get_current_piece_size(packet);
}

void wps_host_free_eapol_packet( wps_eapol_packet_t packet )
{
    host_buffer_release( (wiced_buffer_t) packet, WWD_NETWORK_RX );
}

void wps_host_send_packet(void* workspace, wps_eapol_packet_t packet, uint16_t size)
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)workspace;
    host_buffer_set_size( (wiced_buffer_t) packet, size );
    wwd_network_send_ethernet_data( packet, host->interface );
}

wps_result_t wps_host_leave( void )
{
    wwd_wifi_leave( WWD_STA_INTERFACE );
    return WPS_SUCCESS;
}

wps_result_t wps_host_join( void* workspace, wps_ap_t* ap )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*) workspace;

    WPS_INFO( ("Joining '%.*s'\n", ap->SSID.length, ap->SSID.value) );

    uint8_t attempts = 0;
    besl_result_t ret;
    wiced_semaphore_t semaphore;
    wiced_rtos_init_semaphore(&semaphore);
    do
    {
        ++attempts;
        ret = wwd_wifi_join_specific( ap, NULL, 0, &semaphore, WWD_STA_INTERFACE );
        if (ret != BESL_SUCCESS)
        {
            continue;
        }
        ret = wiced_rtos_get_semaphore(&semaphore, DEFAULT_WPS_JOIN_TIMEOUT);
    } while ( ret != BESL_SUCCESS && attempts < 2 );

    wiced_rtos_deinit_semaphore(&semaphore);

    if ( ret != BESL_SUCCESS )
    {
        WPS_ERROR( ("WPS join failed\n") );
        wps_host_start_timer( host, 100 );
        return WPS_ERROR_JOIN_FAILED;
    }

    wps_event_message_t message;
    message.event_type = WPS_EVENT_ENROLLEE_ASSOCIATED;
    message.data.value = 0;
    host_rtos_push_to_queue(&host->event_queue, &message, 0);

    return WPS_SUCCESS;
}

void wps_host_add_vendor_ie( uint32_t interface, void* data, uint16_t data_length, uint32_t packet_mask )
{
    wwd_wifi_manage_custom_ie( interface, WICED_ADD_CUSTOM_IE, (uint8_t*) WPS_OUI, WPS_OUI_TYPE, data, data_length, packet_mask );
}

void wps_host_remove_vendor_ie( uint32_t interface, void* data, uint16_t data_length, uint32_t packet_mask )
{
    wwd_wifi_manage_custom_ie( interface, WICED_REMOVE_CUSTOM_IE, (uint8_t*) WPS_OUI, WPS_OUI_TYPE, data, data_length, packet_mask );
}

/*
 * NOTE: This function is called from the context of the WICED thread and so should not consume
 *       much stack space and must not printf().
 */
wps_ap_t* wps_host_store_ap( void* workspace, wl_escan_result_t* scan_result )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)workspace;

    if (host->stuff.enrollee.ap_list_counter < AP_LIST_SIZE)
    {
        int a;
        wps_ap_t* ap;

        /* Check if this AP has already been added */
        for (a = 0; a < host->stuff.enrollee.ap_list_counter; ++a)
        {
            ap = &host->stuff.enrollee.ap_list[a];
            if (memcmp(&ap->BSSID, &scan_result->bss_info[0].BSSID, sizeof(wiced_mac_t)) == 0)
            {
                return NULL;
            }
        }

        /* Add to AP list */
        ap = &host->stuff.enrollee.ap_list[host->stuff.enrollee.ap_list_counter++];

        /* Save SSID, BSSID, channel, security and band */
        ap->SSID.length = scan_result->bss_info[0].SSID_len;
        memcpy( ap->SSID.value, scan_result->bss_info[0].SSID, scan_result->bss_info[0].SSID_len );
        memcpy( &ap->BSSID, &scan_result->bss_info[0].BSSID, sizeof(wiced_mac_t) );
        ap->channel  = scan_result->bss_info[0].chanspec & WL_CHANSPEC_CHAN_MASK;
        ap->security = WICED_SECURITY_WPS_SECURE;
        ap->band     = ( ( scan_result->bss_info[0].chanspec & WL_CHANSPEC_BAND_MASK ) == (uint16_t) WL_CHANSPEC_BAND_2G ) ? WICED_802_11_BAND_2_4GHZ : WICED_802_11_BAND_5GHZ;

        return ap;
    }

    return NULL;
}

wps_ap_t* wps_host_retrieve_ap( void* workspace )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)workspace;
    if ( host->stuff.enrollee.ap_list_counter != 0 )
    {
        return &host->stuff.enrollee.ap_list[--host->stuff.enrollee.ap_list_counter];
    }

    return NULL;
}

uint16_t wps_host_get_ap_list_size( void* workspace )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)workspace;
    return host->stuff.enrollee.ap_list_counter;
}

void wps_host_store_credential( void* workspace, wps_credential_t* credential )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)workspace;
    besl_wps_credential_t* temp;

    /* Store credentials if we have room */
    if ( host->stuff.enrollee.stored_credential_count < host->stuff.enrollee.enrollee_output_length)
    {
        temp = &host->stuff.enrollee.enrollee_output[host->stuff.enrollee.stored_credential_count];
        credential->ssid[credential->ssid_length] = 0;
        WPS_INFO( ("Storing credentials for %s\n", credential->ssid) );

        memset( temp, 0, sizeof(besl_wps_credential_t) );
        temp->passphrase_length = credential->network_key_length;
        memcpy( temp->passphrase, credential->network_key, credential->network_key_length );
        temp->ssid.length = credential->ssid_length;
        memcpy( temp->ssid.value, credential->ssid, credential->ssid_length );

        switch ( credential->encryption_type )
        {
            case WPS_MIXED_ENCRYPTION:
            case WPS_AES_ENCRYPTION:
                if ( ( credential->authentication_type == WPS_WPA2_PSK_AUTHENTICATION ) ||
                     ( credential->authentication_type == WPS_WPA2_WPA_PSK_MIXED_AUTHENTICATION ) )
                {
                    temp->security = WICED_SECURITY_WPA2_MIXED_PSK;
                }
                else if ( credential->authentication_type == WPS_WPA_PSK_AUTHENTICATION )
                {
                    temp->security = WICED_SECURITY_WPA_AES_PSK;
                }
                break;
            case WPS_TKIP_ENCRYPTION:
                if ( ( credential->authentication_type == WPS_WPA2_PSK_AUTHENTICATION ) ||
                     ( credential->authentication_type == WPS_WPA2_WPA_PSK_MIXED_AUTHENTICATION ) )
                {
                    temp->security = WICED_SECURITY_WPA2_TKIP_PSK;
                }
                else if ( credential->authentication_type == WPS_WPA_PSK_AUTHENTICATION )
                {
                    temp->security = WICED_SECURITY_WPA_TKIP_PSK;
                }
                break;
            case WPS_WEP_ENCRYPTION:
                temp->security = WICED_SECURITY_WEP_PSK;
                break;
            case WPS_NO_ENCRYPTION:
            default:
                temp->security = WICED_SECURITY_OPEN;
                break;
        }

        ++host->stuff.enrollee.stored_credential_count;
    }
}

void wps_host_retrieve_credential( void* workspace, wps_credential_t* credential )
{
    wps_host_workspace_t*  host       = (wps_host_workspace_t*) workspace;
    const besl_wps_credential_t* ap_details = host->stuff.registrar.ap_details;

    credential->ssid_length = ap_details->ssid.length;
    memcpy( credential->ssid, ap_details->ssid.value, sizeof( credential->ssid ) );
    credential->network_key_length = ap_details->passphrase_length;
    memcpy( credential->network_key, ap_details->passphrase, sizeof( credential->network_key ) );

    switch ( ap_details->security )
    {
        default:
        case WICED_SECURITY_OPEN:
            credential->encryption_type     = WPS_NO_ENCRYPTION;
            credential->authentication_type = WPS_OPEN_AUTHENTICATION;
            break;
        case WICED_SECURITY_WEP_PSK:
            credential->encryption_type     = WPS_WEP_ENCRYPTION;
            credential->authentication_type = WPS_OPEN_AUTHENTICATION;
            break;
        case WICED_SECURITY_WPA_TKIP_PSK:
            credential->encryption_type     = WPS_TKIP_ENCRYPTION;
            credential->authentication_type = WPS_WPA_PSK_AUTHENTICATION;
            break;
        case WICED_SECURITY_WPA_AES_PSK:
            credential->encryption_type     = WPS_AES_ENCRYPTION;
            credential->authentication_type = WPS_WPA_PSK_AUTHENTICATION;
            break;
        case WICED_SECURITY_WPA2_AES_PSK:
            credential->encryption_type     = WPS_AES_ENCRYPTION;
            credential->authentication_type = WPS_WPA2_PSK_AUTHENTICATION;
            break;
        case WICED_SECURITY_WPA2_TKIP_PSK:
            credential->encryption_type     = WPS_TKIP_ENCRYPTION;
            credential->authentication_type = WPS_WPA2_PSK_AUTHENTICATION;
            break;
        case WICED_SECURITY_WPA2_MIXED_PSK:
            credential->encryption_type     = WPS_MIXED_ENCRYPTION;
            credential->authentication_type = WPS_WPA2_PSK_AUTHENTICATION;
            break;
    }
}

void wps_host_start_timer( void* workspace, uint32_t timeout )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)workspace;
    host->timer_reference = host_rtos_get_time( );
    host->timer_timeout   = timeout;
}

void wps_host_stop_timer( void* workspace )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)workspace;
    host->timer_timeout = 0;
}

static void scan_result_handler( wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status )
{
    uint8_t a;

    /* Verify the workspace is still valid */
    for ( a = 0; a < ACTIVE_WPS_WORKSPACE_ARRAY_SIZE; ++a )
    {
        if (active_wps_workspaces[a] == user_data)
        {
            /* Get the host workspace now that we know the workspace is still valid */
            wps_host_workspace_t* host = (wps_host_workspace_t*) ( (wps_agent_t*) ( user_data ) )->wps_host_workspace;

            /* Check if scan is complete */
            if ( result_ptr == NULL )
            {
                wps_event_message_t message;
                message.event_type = WPS_EVENT_DISCOVER_COMPLETE;
                message.data.value = 0;
                host_rtos_push_to_queue( &host->event_queue, &message, 0 );
            }
            else
            {
                host->stuff.enrollee.scan_handler_ptr( (wl_escan_result_t*) result_ptr, user_data );
            }

            break;
        }
    }
}

void wps_host_scan( wps_agent_t* workspace, wps_scan_handler_t result_handler )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*) (workspace->wps_host_workspace);
    host->stuff.enrollee.ap_list_counter  = 0;
    host->stuff.enrollee.scan_handler_ptr = result_handler;
    uint8_t attempts = 0;
    besl_result_t ret;
    uint16_t chlist[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,0 };
    wiced_scan_extended_params_t extparam = { 2, 40, 110, 50 };

    do
    {
        ++attempts;
        ret = wwd_wifi_scan( WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_INFRASTRUCTURE, 0, 0, chlist, &extparam, scan_result_handler, 0, workspace, WWD_STA_INTERFACE );
    } while ( ret != BESL_SUCCESS && attempts < 5 );

    if (ret != BESL_SUCCESS)
    {
        WPS_ERROR( ("WPS scan failure\n") );
        wps_host_start_timer( host, 100 );
    }
    else
    {
        wps_host_start_timer( host, 2000);
    }
}

void wps_host_get_authorized_macs( void* workspace, besl_mac_t** mac_list, uint8_t* mac_list_length )
{
    wps_host_workspace_t* host = (wps_host_workspace_t*)workspace;
    *mac_list = &host->stuff.registrar.authorized_mac_list[0];
    *mac_list_length = 1;
}


void wps_host_deauthenticate_client( const besl_mac_t* mac, uint32_t reason )
{
    wwd_wifi_deauth_sta( (wiced_mac_t*) mac, reason );
}

static void* wps_softap_event_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data )
{
    const uint8_t*      data;
    uint32_t      length = event_header->datalen;
    tlv8_data_t*  tlv8;

    if (length <= 24 )
    {
        return handler_user_data;
    }
    length -= 24; /* XXX length of management frame MAC header */
    data = event_data + 24;

    switch ( event_header->event_type )
    {
        case WLC_E_PROBREQ_MSG:
            if ( event_header->status == WLC_E_STATUS_SUCCESS )
            {
                do
                {
                    /* Scan result for vendor specific IE */
                    tlv8 = tlv_find_tlv8( data, length, DOT11_MNG_VS_ID );
                    if ( tlv8 == NULL )
                    {
                        break;
                    }
                    /* Check if the TLV we've found is outside the bounds of the scan result length. i.e. Something is bad */
                    if ((( (uint8_t*) tlv8 - data ) + tlv8->length + sizeof(tlv8_header_t)) > length)
                    {
                        break;
                    }

                    length -= ( (uint8_t*) tlv8 - data ) + tlv8->length + sizeof(tlv8_header_t);
                    data = (uint8_t*) tlv8 + tlv8->length + sizeof(tlv8_header_t);

                    /* Verify extension is WPS extension */
                    if ( memcmp( tlv8->data, WPS_OUI, 3 ) == 0 && tlv8->data[3] == WPS_OUI_TYPE )
                    {
                        /* Look for pwd ID, check if it is the same mode that we are */
                        tlv16_uint16_t* pwd_id = (tlv16_uint16_t*) tlv_find_tlv16( &tlv8->data[4], tlv8->length - 4, WPS_ID_DEVICE_PWD_ID );
                        if ( pwd_id != NULL && besl_host_hton16( pwd_id->data ) == WPS_PUSH_BTN_DEVICEPWDID )
                        {
                            wps_update_pbc_overlap_array( (besl_mac_t*)(event_data + 10) );
                        }
                    }
                } while ( tlv8 != NULL );
            }
            break;

        default:
            break;
    }

    return handler_user_data;
}

static void wps_update_pbc_overlap_array(const besl_mac_t* mac )
{
    int i;
    wiced_time_t rx_time;

    rx_time = host_rtos_get_time( );

    /* If the MAC address is the same as the last enrollee give it 5 seconds to stop asserting PBC mode */
    if ( ( memcmp( mac, (char*) &last_pbc_enrollee.probe_request_mac, sizeof(besl_mac_t) ) == 0 ) &&
         ( ( rx_time - last_pbc_enrollee.probe_request_rx_time ) <= 5000 ) )
    {
        WPS_DEBUG( ("Received probe request asserting PBC mode from last enrollee\r\n") );
        return;
    }

    /* If the MAC address is already in the array update the rx time */
    for ( i = 0; i < 2; i++ )
    {
        if (memcmp(mac, (char*)&pbc_overlap_array[i].probe_request_mac, sizeof(besl_mac_t)) == 0)
        {
            pbc_overlap_array[i].probe_request_rx_time = rx_time;
            return;
        }
    }

    /* If this is a new MAC address replace oldest existing entry with this MAC address */
    if ( pbc_overlap_array[0].probe_request_rx_time == 0 ) /* Initial condition for array record 0 */
    {
        memcpy((char*)&pbc_overlap_array[0].probe_request_mac, mac, sizeof(besl_mac_t));
        pbc_overlap_array[0].probe_request_rx_time = rx_time;
    }
    else if ( pbc_overlap_array[1].probe_request_rx_time == 0 ) /* Initial condition for array record 1 */
    {
        memcpy((char*)&pbc_overlap_array[1].probe_request_mac, mac, sizeof(besl_mac_t));
        pbc_overlap_array[1].probe_request_rx_time = rx_time;
    }
    else if ( pbc_overlap_array[0].probe_request_rx_time <= pbc_overlap_array[1].probe_request_rx_time )
    {
        memcpy((char*)&pbc_overlap_array[0].probe_request_mac, mac, sizeof(besl_mac_t));
        pbc_overlap_array[0].probe_request_rx_time = rx_time;
    }
    else
    {
        memcpy((char*)&pbc_overlap_array[1].probe_request_mac, mac, sizeof(besl_mac_t));
    }
    return;
}

void wps_clear_pbc_overlap_array( void )
{
    memset( (char*)pbc_overlap_array, 0, sizeof(pbc_overlap_array) );
}


wps_result_t wps_pbc_overlap_check(const besl_mac_t* mac )
{
    wiced_time_t detection_window_start;
    wiced_time_t time_now;

    /* Detection window starts at 0, or 120 seconds prior to now if the host has been up for more than 120 seconds */
    detection_window_start = 0;
    time_now = host_rtos_get_time( );
    if ( time_now > WPS_PBC_OVERLAP_WINDOW )
    {
        detection_window_start = time_now - WPS_PBC_OVERLAP_WINDOW;
    }

    WPS_DEBUG( ("PBC overlap detection window start %u\n", (unsigned int)detection_window_start) );

    /* This tests the case where M1 has arrived and there may or may not be a probe request from the same enrollee
     * in the detection array, but there is a probe request from another enrollee.
     */
    if ( mac != NULL )
    {
        if ( ( memcmp( mac, (char*) &pbc_overlap_array[0].probe_request_mac, sizeof(besl_mac_t) ) != 0 ) &&
             ( pbc_overlap_array[0].probe_request_rx_time > detection_window_start ) )
        {
            return WPS_PBC_OVERLAP;
        }

        if ( ( memcmp( mac, (char*) &pbc_overlap_array[1].probe_request_mac, sizeof(besl_mac_t) ) != 0 ) &&
             ( pbc_overlap_array[1].probe_request_rx_time > detection_window_start ) )
        {
            return WPS_PBC_OVERLAP;
        }
        else
        {
            return WPS_SUCCESS;
        }
    }
    else
    {
        /* This tests the simple case where two enrollees have been probing during the detection window */
        if ( ( pbc_overlap_array[0].probe_request_rx_time > detection_window_start ) &&
             ( pbc_overlap_array[1].probe_request_rx_time > detection_window_start ) )
        {
            WPS_DEBUG(("PBC overlap array entry 0 %u\n", (unsigned int)pbc_overlap_array[0].probe_request_rx_time));
            WPS_DEBUG(("PBC overlap array entry 1 %u\n", (unsigned int)pbc_overlap_array[1].probe_request_rx_time));
            return WPS_PBC_OVERLAP;
        }
    }

    return WPS_SUCCESS;
}

void wps_record_last_pbc_enrollee( const besl_mac_t* mac )
{
    memcpy((char*)&last_pbc_enrollee.probe_request_mac, mac, sizeof(besl_mac_t));
    last_pbc_enrollee.probe_request_rx_time = host_rtos_get_time( );
}

