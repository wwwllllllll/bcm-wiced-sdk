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
 */
#include "wiced.h"
#include "wiced_p2p.h"
#include "p2p_host_interface.h"
#include "internal/wwd_sdpcm.h"
#include "network/wwd_buffer_interface.h"
#include "wwd_events.h"
#include "rtos.h"
#include "wps_p2p_interface.h"
#include "wiced_wps.h"
#include "wps_host_interface.h"
#include "internal/wiced_internal_api.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_NUMBER_OF_P2P_MESSAGES     (5)
#define P2P_THREAD_STACK_SIZE          (6*1024)

#define P2P_OUI             "\x50\x6f\x9a"
#define P2P_OUI_TYPE        9

#define P2P_MAX_DISCOVERABLE_INTERVAL    (3)
#define P2P_BEACON_INTERVAL_MS           (100)


/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    P2P_CAPABILITY_SERVICE_DISCOVERY       = (1 << 0),
    P2P_CAPABILITY_CLIENT_DISCOVERABLITY   = (1 << 1),
    P2P_CAPABILITY_CONCURRENT_OPERATION    = (1 << 2),
    P2P_CAPABILITY_INFRASTRUCTURE_MANAGER  = (1 << 3),
    P2P_CAPABILITY_DEVICE_LIMIT            = (1 << 4),
    P2P_CAPABILITY_INVITATION_PROCEDURE    = (1 << 5),
} p2p_capabilities_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void           p2p_thread_main( uint32_t arg );
static void*          p2p_event_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data );
static besl_result_t  p2p_scan( void );
static besl_result_t  p2p_set_discovery_state( p2p_discovery_state_t state );
static void           p2p_discover( p2p_workspace_t* workspace );

static besl_result_t p2p_run_as_client( p2p_workspace_t* workspace, wps_agent_t* wps_enrollee );
static besl_result_t p2p_run_as_go    ( p2p_workspace_t* workspace, wps_agent_t* wps_registrar );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static host_thread_type_t p2p_thread;
static host_queue_type_t  p2p_message_queue;
static host_queue_type_t  p2p_outgoing_packet_queue;
static p2p_message_t      p2p_message_queue_buffer        [ MAX_NUMBER_OF_P2P_MESSAGES ];
static p2p_message_t      p2p_outgoing_packet_queue_buffer[ MAX_NUMBER_OF_P2P_MESSAGES ];
static uint8_t            p2p_thread_stack                [ P2P_THREAD_STACK_SIZE ];

static const wwd_event_num_t p2p_events[]  = { WLC_E_ESCAN_RESULT, WLC_E_P2P_DISC_LISTEN_COMPLETE, WLC_E_P2P_PROBREQ_MSG, WLC_E_ACTION_FRAME, WLC_E_NONE };

static const wiced_ip_setting_t p2p_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192, 168, 10,  1) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255, 255, 255, 0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192, 168, 10,  1) ),
};

/******************************************************
 *               Function Definitions
 ******************************************************/

besl_result_t besl_p2p_init( p2p_workspace_t* workspace, const besl_p2p_device_detail_t* device_details )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint32_t*      data;
    wwd_result_t result;
    REFERENCE_DEBUG_ONLY_VARIABLE(result);

    memset(workspace, 0, sizeof(p2p_workspace_t));

    workspace->p2p_capability = 0x0000;
    workspace->p2p_name       = device_details->device_name;
    workspace->group_owner_intent = 1;

    /* Turn off all the other Wi-Fi interfaces */
    wiced_network_down(WICED_STA_INTERFACE);
    wiced_network_down(WICED_AP_INTERFACE);

    /* Query the AP interface to ensure that it is up */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 36, IOVAR_STR_BSSCFG_SSID );
    memset(data, 0, 36);
    data[0] = (uint32_t) CHIP_AP_INTERFACE;
    wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WWD_STA_INTERFACE );

    /*  Enable discovery */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, 4, IOVAR_STR_P2P_DISC );
    *data = 1;
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    /*  Find what interface is the P2P device */
    wwd_sdpcm_get_iovar_buffer(&buffer, 4, IOVAR_STR_P2P_DEV );
    result = wwd_sdpcm_send_iovar(SDPCM_GET, buffer, &response, WICED_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);
    workspace->p2p_interface = BESL_READ_32(host_buffer_get_current_piece_data_pointer(response));
    host_buffer_release(response, WWD_NETWORK_RX);

    /* Get the P2P interface MAC address */
    besl_host_get_mac_address(&workspace->device_info.mac_address, workspace->p2p_interface);

    /* Set the standard interface MAC address to be the same as the P2P interface */
    besl_host_set_mac_address(&workspace->device_info.mac_address, WICED_STA_INTERFACE);
    besl_host_set_mac_address(&workspace->device_info.mac_address, WICED_AP_INTERFACE);

    /* Get the standard MAC address to confirm */
    besl_host_get_mac_address(&workspace->intended_mac_address, WICED_STA_INTERFACE);

    BESL_INFO( ("STA MAC: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", workspace->intended_mac_address.octet[0],
        workspace->intended_mac_address.octet[1],
        workspace->intended_mac_address.octet[2],
        workspace->intended_mac_address.octet[3],
        workspace->intended_mac_address.octet[4],
        workspace->intended_mac_address.octet[5]) );


    /* Set the device details */
    workspace->wps_device_details = &device_details->wps_device_details;

    /* Allow the P2P library to initialize */
    p2p_init(workspace, workspace->p2p_name);

    /* Bring up P2P interface */
    wwd_sdpcm_get_ioctl_buffer( &buffer, 0 );
    result = wwd_sdpcm_send_ioctl( SDPCM_SET, WLC_UP, buffer, NULL, workspace->p2p_interface);
    wiced_assert("", result == WWD_SUCCESS);

    /* Set wsec to any non-zero value in the discovery bsscfg to ensure our P2P probe responses have the privacy bit set in the 802.11 WPA IE.
     * Some peer devices may not initiate WPS with us if this bit is not set. */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, 8, IOVAR_STR_BSSCFG_WSEC );
    data[0] = workspace->p2p_interface;
    data[1] = WICED_SECURITY_WPA2_AES_PSK;
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    workspace->p2p_current_state = P2P_STATE_DISCOVERING;

    /*  Add P2P event handler */
    result = wwd_management_set_event_handler( p2p_events, p2p_event_handler, workspace, workspace->p2p_interface );
    wiced_assert("", result == WWD_SUCCESS);

    /* Create the message queue */
    host_rtos_init_queue(&p2p_message_queue, p2p_message_queue_buffer, sizeof(p2p_message_queue_buffer), sizeof(p2p_message_t));

    /* Create the pending outgoing packet queue */
    host_rtos_init_queue(&p2p_outgoing_packet_queue, p2p_outgoing_packet_queue_buffer, sizeof(p2p_outgoing_packet_queue_buffer), sizeof(p2p_message_t));

    /* Create the P2P thread */
    host_rtos_create_thread_with_arg( &p2p_thread, p2p_thread_main, "p2p", p2p_thread_stack, sizeof(p2p_thread_stack), RTOS_HIGHER_PRIORTIY_THAN(RTOS_DEFAULT_THREAD_PRIORITY), (uint32_t)workspace );

    return BESL_SUCCESS;
}

besl_result_t besl_p2p_deinit( p2p_workspace_t* workspace )
{
    /* Stop and delete the P2P thread */
    besl_p2p_stop(workspace);
    host_rtos_join_thread(&p2p_thread);
    host_rtos_delete_terminated_thread(&p2p_thread);
    memset(&p2p_thread, 0, sizeof(p2p_thread));

    /* Delete the message queue */
    host_rtos_deinit_queue( &p2p_message_queue );

    /* Delete the pending outgoing packet queue */
    host_rtos_deinit_queue( &p2p_outgoing_packet_queue );

    return BESL_SUCCESS;
}

besl_result_t besl_p2p_start( p2p_workspace_t* workspace )
{
    p2p_message_t message;
    message.type = P2P_EVENT_START_REQUESTED;
    host_rtos_push_to_queue(&p2p_message_queue, &message, WICED_NEVER_TIMEOUT);

    return BESL_SUCCESS;
}

besl_result_t besl_p2p_stop( p2p_workspace_t* workspace )
{
    p2p_message_t message;
    message.type = P2P_EVENT_STOP_REQUESTED;
    host_rtos_push_to_queue( &p2p_message_queue, &message, WICED_NEVER_TIMEOUT );

    return BESL_SUCCESS;
}

p2p_discovered_device_t* p2p_host_find_device( p2p_workspace_t* workspace, const besl_mac_t* mac )
{
    int a = 0;
    while ( workspace->discovered_devices[a].status != P2P_DEVICE_INVALID )
    {
        if ( memcmp( &workspace->discovered_devices[a].mac, mac, sizeof(besl_mac_t) ) == 0 )
        {
            return &workspace->discovered_devices[a];
        }
        if ( ++a >= P2P_MAX_NUMBER_OF_DEVICES )
        {
            return NULL;
        }
    }

    return &workspace->discovered_devices[a];
}

static void p2p_thread_main( uint32_t arg )
{
    p2p_workspace_t* workspace = (p2p_workspace_t*)arg;
    wwd_result_t   result;
    p2p_message_t    message;
    uint8_t          last_printed_discovered_device = 0;

    BESL_INFO(("P2P discovery enabled. Advertised as '%s'\n", workspace->p2p_name));

    workspace->p2p_result = BESL_IN_PROGRESS;

    while ( workspace->p2p_current_state != P2P_STATE_COMPLETE &&
            workspace->p2p_current_state != P2P_STATE_ABORTED )
    {
        host_rtos_pop_from_queue(&p2p_message_queue, &message, WICED_NEVER_TIMEOUT);

        switch(message.type)
        {
            case P2P_EVENT_SCAN_COMPLETE:
                p2p_discover(workspace);
                break;

            case P2P_EVENT_DISCOVERY_COMPLETE:
                if ( workspace->p2p_current_state == P2P_STATE_NEGOTIATING )
                {
                    p2p_discover(workspace);
                    break;
                }
                /* Otherwise fall through */

            case P2P_EVENT_START_REQUESTED:
                if (workspace->p2p_current_state != P2P_STATE_NEGOTIATING)
                {
                    workspace->p2p_current_state = P2P_STATE_SCANNING;
                }
                result = p2p_set_discovery_state( P2P_DISCOVERY_STATE_SEARCH );
                result = p2p_scan( );
                break;

            case P2P_EVENT_PACKET_TO_BE_SENT:
                result = wwd_sdpcm_send_iovar( SDPCM_SET, (wiced_buffer_t) message.data, NULL, WICED_STA_INTERFACE );
                if (result != WWD_SUCCESS)
                {
                    /*  Packet has been lost.. Maybe. Don't think we can recover it though */
                }
                break;

            case P2P_EVENT_NEGOTIATION_COMPLETE:
                BESL_INFO( ("P2P negotiation complete...\n") );
                workspace->p2p_current_state = P2P_STATE_COMPLETE;
                break;

            case P2P_EVENT_STOP_REQUESTED:
                workspace->p2p_current_state = P2P_STATE_ABORTED;
                break;

            case P2P_EVENT_NEW_DEVICE_DISCOVERED:
                printf( "Found P2P device: %s\r\n", workspace->discovered_devices[last_printed_discovered_device].device_name);
                ++last_printed_discovered_device;
                break;

            default:
                break;
        }
    }

    /*  Remove P2P event handler */
    result = wwd_management_set_event_handler( p2p_events, NULL, workspace, workspace->p2p_interface );
    wiced_assert("", result == WWD_SUCCESS);

    /* Remove the unused WPS and P2P IEs */
    wps_host_remove_vendor_ie( workspace->p2p_interface, workspace->wps_probe_ie,  workspace->wps_probe_ie_length,  VENDOR_IE_PROBE_REQUEST );
    wps_host_remove_vendor_ie( workspace->p2p_interface, workspace->wps_probe_ie, workspace->wps_probe_ie_length, VENDOR_IE_PROBE_RESPONSE );
    p2p_host_remove_vendor_ie( workspace->p2p_interface, workspace->p2p_probe_request_ie,  workspace->p2p_probe_request_ie_length,  VENDOR_IE_PROBE_REQUEST);
    p2p_host_remove_vendor_ie( workspace->p2p_interface, workspace->p2p_probe_response_ie, workspace->p2p_probe_response_ie_length, VENDOR_IE_PROBE_RESPONSE);

    wiced_buffer_t buffer;
    uint32_t* data;

    /* Bring down the P2P interface */
    data = wwd_sdpcm_get_iovar_buffer(&buffer, sizeof(wiced_mac_t), IOVAR_STR_P2P_IFDEL );
    memcpy(data, &workspace->device_info.mac_address, sizeof(wiced_mac_t));
    result = wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE);
    wiced_assert("", result == WWD_SUCCESS);

    if (workspace->p2p_current_state == P2P_STATE_COMPLETE)
    {
        wps_agent_t* wps_agent = besl_host_malloc( "p2p", sizeof(wps_agent_t) );
        if ( wps_agent == NULL )
        {
            goto return_with_error;
        }
        memset( wps_agent, 0, sizeof(wps_agent_t) );

        if ( workspace->i_am_group_owner == 0 )
        {
            p2p_run_as_client( workspace, wps_agent );
        }
        else
        {
            p2p_run_as_go( workspace, wps_agent );
        }

        besl_host_free(wps_agent);
    }

return_with_error:
    WICED_END_OF_CURRENT_THREAD( );
}

static besl_result_t p2p_run_as_client( p2p_workspace_t* workspace, wps_agent_t* wps_enrollee )
{
    wwd_result_t        result;
    wl_escan_result_t     p2p_group_ap;
    besl_wps_credential_t credential;
    uint8_t               a;

    /* Start WPS */
    memset( &p2p_group_ap, 0, sizeof(p2p_group_ap) );

    /* Add the P2P association request IE to the STA interface */
    workspace->p2p_interface = WICED_STA_INTERFACE;
    memcpy(&workspace->device_info.mac_address, &workspace->intended_mac_address, sizeof(besl_mac_t));
    p2p_write_association_request_ie( workspace );

    host_rtos_delay_milliseconds(1500);

    besl_wps_init( wps_enrollee, workspace->wps_device_details, WPS_ENROLLEE_AGENT, WICED_STA_INTERFACE);

    /* NOTE: For some reason packet being received on the P2P interface are sent up with an interface of 0 (WICED_STA_INTERFACE) */
    wps_internal_init(wps_enrollee, WICED_STA_INTERFACE, WPS_PBC_MODE, "00000000", &credential, 1);

    /*  Create the AP details */
    p2p_group_ap.bss_info[0].chanspec = WL_CHANSPEC_BAND_2G | workspace->group_candidate.channel;
    p2p_group_ap.bss_info[0].SSID_len = (uint8_t)workspace->group_candidate.ssid.length;
    memcpy(&p2p_group_ap.bss_info[0].BSSID, &workspace->group_candidate.bssid, sizeof(besl_mac_t));
    memcpy(p2p_group_ap.bss_info[0].SSID,    workspace->group_candidate.ssid.value,  p2p_group_ap.bss_info[0].SSID_len);

    /* Add a few copies to the WPS workspace so it will automatically join the designated AP */
    wps_host_store_ap(wps_enrollee->wps_host_workspace, &p2p_group_ap );
    wps_host_store_ap(wps_enrollee->wps_host_workspace, &p2p_group_ap );
    wps_host_store_ap(wps_enrollee->wps_host_workspace, &p2p_group_ap );
    wps_host_store_ap(wps_enrollee->wps_host_workspace, &p2p_group_ap );

    /* Run the WPS state machine in the P2P thread */
    wiced_wps_thread_main((uint32_t)wps_enrollee);

    /* Clean up WPS objects*/
    BESL_INFO( ("WPS complete\n") );
    besl_wps_deinit( wps_enrollee );

    wiced_scan_result_t wps_ap;
    wps_ap.channel  = workspace->group_candidate.channel;
    wps_ap.SSID.length = (uint8_t)workspace->group_candidate.ssid.length;
    wps_ap.security = credential.security;
    wps_ap.band     = WICED_802_11_BAND_2_4GHZ;
    wps_ap.bss_type = WICED_BSS_TYPE_INFRASTRUCTURE;
    memcpy(&wps_ap.BSSID,  &workspace->group_candidate.bssid, sizeof(besl_mac_t));
    memcpy(wps_ap.SSID.value, workspace->group_candidate.ssid.value,  wps_ap.SSID.length);

    /* Try a few times to join the AP with the credentials we've just received */
    result = WWD_PENDING;
    for ( a = 0; a < 3 && result != WWD_SUCCESS; ++a )
    {
        result = wwd_wifi_join_specific( &wps_ap, credential.passphrase, credential.passphrase_length, NULL, WWD_STA_INTERFACE );
    }

    if ( result == WWD_SUCCESS )
    {
        BESL_INFO( ("P2P complete\n") );
        wiced_ip_up(workspace->p2p_interface, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
        workspace->p2p_result = BESL_SUCCESS;
        return BESL_SUCCESS;
    }
    else
    {
        BESL_INFO( ("P2P couldn't join AP\n") );
        workspace->p2p_result = BESL_ERROR_JOIN_FAILED;
        return BESL_ERROR_JOIN_FAILED;
    }
}

static besl_result_t p2p_run_as_go( p2p_workspace_t* workspace, wps_agent_t* wps_registrar )
{
    uint32_t*             data;
    wwd_result_t          result;
    besl_wps_credential_t credential;
    wiced_buffer_t        buffer;
    uint8_t               passphrase_buffer[32];
    uint8_t               a;

    REFERENCE_DEBUG_ONLY_VARIABLE(result);

    host_rtos_delay_milliseconds( 500 );

    /* Limit the rates used on the P2P soft AP */
    data    = wwd_sdpcm_get_iovar_buffer( &buffer, 4, IOVAR_STR_BSS_RATESET );
    data[0] = 1;
    result  = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WICED_AP_INTERFACE );
    wiced_assert("", result == WWD_SUCCESS);

    /* Prepare the AP credentials */
    credential.security = WICED_SECURITY_WPA2_AES_PSK;
    credential.ssid.length = workspace->group_candidate.ssid.length;
    memcpy( credential.ssid.value, workspace->group_candidate.ssid.value, credential.ssid.length );
    credential.passphrase_length = 64;
    besl_host_random_bytes(passphrase_buffer, 32);
    for ( a = 0; a < 32; ++a )
    {
        credential.passphrase[2*a]     = nibble_to_hexchar(passphrase_buffer[a] >> 4);
        credential.passphrase[2*a + 1] = nibble_to_hexchar(passphrase_buffer[a] & 0x0F);
    }

    /* Start the AP */
    wwd_wifi_start_ap( &workspace->group_candidate.ssid, WICED_SECURITY_WPA2_AES_PSK | WPS_ENABLED, credential.passphrase, credential.passphrase_length, 1 );

    workspace->p2p_interface = WICED_AP_INTERFACE;
    memcpy( &workspace->device_info.mac_address, &workspace->intended_mac_address, sizeof(besl_mac_t) );

    wl_p2p_if_t* p2p_if = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_p2p_if_t), IOVAR_STR_P2P_IFUPD );
    p2p_if->interface_type = P2P_GROUP_OWNER_MODE;
    memcpy( &p2p_if->mac_address, &workspace->intended_mac_address, sizeof(besl_mac_t) );
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE );
    wiced_assert("", result == WWD_SUCCESS);

    p2p_write_probe_response_ie( workspace );
    p2p_write_beacon_ie( workspace );

    /* Bring up IP layer on AP interface */
    wiced_ip_up( WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &p2p_ip_settings );

    besl_wps_init( wps_registrar, workspace->wps_device_details, WPS_REGISTRAR_AGENT, WICED_AP_INTERFACE );

    /* Init WPS */
    wps_internal_init( wps_registrar, WICED_AP_INTERFACE, WPS_PBC_MODE, "00000000", &credential, 1 );

    /* Run WPS state machine in P2P thread */
    wiced_wps_thread_main( (uint32_t) wps_registrar );

    if ( wps_registrar->wps_result == WPS_COMPLETE )
    {
        workspace->p2p_result = BESL_SUCCESS;
    }
    else
    {
        workspace->p2p_result = wps_registrar->wps_result;
    }

    return BESL_SUCCESS;
}

static void p2p_discover( p2p_workspace_t* workspace )
{
    wwd_result_t result;
    p2p_message_t  message;
    REFERENCE_DEBUG_ONLY_VARIABLE(result);

    if ( workspace->p2p_current_state == P2P_STATE_SCANNING )
    {
        workspace->p2p_current_state = P2P_STATE_DISCOVERING;
    }

    result = p2p_set_discovery_state( P2P_DISCOVERY_STATE_LISTEN );
    wiced_assert("", result == WWD_SUCCESS);

    while ( host_rtos_pop_from_queue( &p2p_outgoing_packet_queue, &message, 0 ) == WWD_SUCCESS )
    {
        result = wwd_sdpcm_send_iovar( SDPCM_SET, (wiced_buffer_t) message.data, NULL, WICED_STA_INTERFACE );
        wiced_assert("", result == WWD_SUCCESS);
    }
}

void p2p_host_add_vendor_ie( uint32_t interface, void* data, uint16_t data_length, uint32_t packet_mask )
{
    wwd_wifi_manage_custom_ie( interface, WICED_ADD_CUSTOM_IE, (uint8_t*) P2P_OUI, P2P_OUI_TYPE, data, data_length, packet_mask );
}

void p2p_host_remove_vendor_ie( uint32_t interface, void* data, uint16_t data_length, uint32_t packet_mask )
{
    wwd_wifi_manage_custom_ie( interface, WICED_REMOVE_CUSTOM_IE, (uint8_t*) P2P_OUI, P2P_OUI_TYPE, data, data_length, packet_mask );
}

static void* p2p_event_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, /*@returned@*/ void* handler_user_data )
{
    p2p_workspace_t* workspace = handler_user_data;
    p2p_message_t    message;

    switch ( event_header->event_type )
    {
        case WLC_E_ESCAN_RESULT:
            if ( event_header->status == WLC_E_STATUS_SUCCESS )
            {
                /*  Scan complete */
                message.type = P2P_EVENT_SCAN_COMPLETE;
                host_rtos_push_to_queue(&p2p_message_queue, &message, 0);
            }
            break;

        case WLC_E_P2P_DISC_LISTEN_COMPLETE:
            message.type = P2P_EVENT_DISCOVERY_COMPLETE;
            host_rtos_push_to_queue(&p2p_message_queue, &message, 0);
            break;

        case WLC_E_P2P_PROBREQ_MSG:
            if ( event_data != NULL ) /* TODO: Check if this is needed */
            {
                if ( p2p_process_probe_request( workspace, event_data, event_header->datalen ) == BESL_SUCCESS )
                {
                    message.type = P2P_EVENT_NEW_DEVICE_DISCOVERED;
                    host_rtos_push_to_queue( &p2p_message_queue, &message, 0 );
                }
            }
            break;

        case WLC_E_ACTION_FRAME:
        {
            /* Find matching discovered_device */
            p2p_discovered_device_t* device = p2p_host_find_device(workspace, (besl_mac_t*)&event_header->addr);

            /* Check if device hasn't been seen before */
            if (device->status == P2P_DEVICE_INVALID)
            {
                memcpy( &device->mac, &event_header->addr, sizeof(besl_mac_t) );
                if ( p2p_process_new_device_data( workspace, device, event_data + 16 + sizeof(ieee80211_header_t), event_header->datalen - 16 - sizeof(ieee80211_header_t) ) == BESL_SUCCESS )
                {
                    message.type = P2P_EVENT_NEW_DEVICE_DISCOVERED;
                    host_rtos_push_to_queue( &p2p_message_queue, &message, 0 );
                }
            }
            p2p_process_action_frame( workspace, device, event_data, event_header->datalen );
            break;
        }

        default:
            break;
    }

    return handler_user_data;
}

besl_result_t p2p_send_action_frame( p2p_workspace_t* workspace, p2p_discovered_device_t* device, p2p_action_frame_writer_t writer, uint32_t channel )
{
    wiced_buffer_t  buffer;
    wl_af_params_t* frame;
    p2p_message_t   message;
    wwd_result_t  result;

    uint32_t* a = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_af_params_t) + 4, IOVAR_STR_BSSCFG_ACTFRAME );
    *a = workspace->p2p_interface;
    frame = (wl_af_params_t*) ( a + 1 );
    frame->channel    = channel;
    frame->dwell_time = 100;

    memcpy( &frame->action_frame.da, &device->mac, sizeof(besl_mac_t) );
    memcpy( &frame->BSSID, &frame->action_frame.da, 6 );

    uint8_t* end_of_data = writer( workspace, device, frame->action_frame.data );

    frame->action_frame.len      = end_of_data - frame->action_frame.data;
    frame->action_frame.packetId = 1;

    message.type = P2P_EVENT_PACKET_TO_BE_SENT;
    message.data = buffer;
    if ( workspace->p2p_current_state != P2P_STATE_SCANNING && workspace->p2p_current_state != P2P_STATE_NEGOTIATING)
    {
        result = host_rtos_push_to_queue(&p2p_message_queue, &message, WICED_NEVER_TIMEOUT);
    }
    else
    {
        result = host_rtos_push_to_queue(&p2p_outgoing_packet_queue, &message, WICED_NEVER_TIMEOUT);
    }

    if ( result != WWD_SUCCESS)
    {
        return BESL_ERROR_QUEUE_PUSH;
    }

    return BESL_SUCCESS;
}

static besl_result_t p2p_scan( void )
{
    wiced_buffer_t buffer;
    wl_p2p_scan_t* p2p_scan;

    /*  Begin p2p scan of the "escan" variety */
    p2p_scan = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_p2p_scan_t), IOVAR_STR_P2P_SCAN );
    memset( p2p_scan, 0, sizeof(wl_p2p_scan_t) );

    /* Fill in the appropriate details of the scan parameters structure */
    p2p_scan->type                      = 'E';
    besl_host_random_bytes((uint8_t*)&p2p_scan->escan.sync_id, sizeof(p2p_scan->escan.sync_id));
    p2p_scan->escan.version             = htod32(ESCAN_REQ_VERSION);
    p2p_scan->escan.action              = htod16(WL_SCAN_ACTION_START);
    p2p_scan->escan.params.scan_type    = (int8_t) WICED_SCAN_TYPE_ACTIVE;
    p2p_scan->escan.params.bss_type     = (int8_t) WICED_BSS_TYPE_INFRASTRUCTURE;
    p2p_scan->escan.params.nprobes      = (int32_t) -1;
    p2p_scan->escan.params.active_time  = (int32_t) -1;
    p2p_scan->escan.params.passive_time = (int32_t) -1;
    p2p_scan->escan.params.home_time    = (int32_t) -1;
    p2p_scan->escan.params.channel_num  = 0;
    p2p_scan->escan.params.ssid.SSID_len = sizeof( P2P_WILDCARD_SSID ) - 1;
    memcpy( p2p_scan->escan.params.ssid.SSID, P2P_WILDCARD_SSID, sizeof( P2P_WILDCARD_SSID ) - 1 );

    if ( wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE ) != WWD_SUCCESS )
    {
        return BESL_ERROR_SCAN_START_FAIL;
    }

    return BESL_SUCCESS;
}

static besl_result_t p2p_set_discovery_state( p2p_discovery_state_t state )
{
    wiced_buffer_t buffer;
    wl_p2p_disc_st_t* discovery_mode = wwd_sdpcm_get_iovar_buffer(&buffer, sizeof(wl_p2p_disc_st_t), IOVAR_STR_P2P_STATE );

    discovery_mode->state = state;
    if (state == P2P_DISCOVERY_STATE_LISTEN)
    {
        uint16_t listen_ms;
        besl_host_random_bytes( (uint8_t*)&listen_ms, 2 );
        listen_ms = ( 1 + (listen_ms % P2P_MAX_DISCOVERABLE_INTERVAL ) ) * P2P_BEACON_INTERVAL_MS;

        discovery_mode->chanspec      = 0;
        discovery_mode->dwell_time_ms = listen_ms;
    }
    else
    {
        discovery_mode->chanspec      = 0;
        discovery_mode->dwell_time_ms = 0;
    }
    return wwd_sdpcm_send_iovar(SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE);
}

void p2p_host_negotiation_complete( p2p_workspace_t* workspace )
{
    p2p_message_t message;
    message.type = P2P_EVENT_NEGOTIATION_COMPLETE;
    host_rtos_push_to_queue(&p2p_message_queue, &message, WICED_NEVER_TIMEOUT);
}

besl_result_t besl_p2p_set_ap_suffix( p2p_workspace_t* workspace, const char* suffix )
{
    memcpy(workspace->p2p_ap_suffix, suffix, MIN(P2P_MAX_SUFFIX_LENGTH, strlen(suffix)));
    return BESL_SUCCESS;
}

besl_result_t besl_p2p_get_result( p2p_workspace_t* workspace )
{
    return workspace->p2p_result;
}

besl_result_t besl_p2p_invite( p2p_workspace_t* workspace, p2p_discovered_device_t* device)
{
    besl_result_t result = p2p_send_action_frame( workspace, device, p2p_write_go_request, device->channel );
    if ( result == BESL_SUCCESS )
    {
        device->status = P2P_DEVICE_REQUESTED;
    }

    return result;
}

besl_result_t besl_p2p_get_discovered_peers( p2p_workspace_t* workspace, p2p_discovered_device_t** devices, uint8_t* device_count )
{
    *devices = workspace->discovered_devices;
    *device_count = workspace->discovered_device_count;

    return BESL_SUCCESS;
}

#ifdef WICED_TESTING
void besl_p2p_test( p2p_workspace_t* workspace )
{
    wiced_buffer_t buffer;
    wiced_result_t result;
    uint32_t* data;
    wiced_mac_t my_mac;

    REFERENCE_DEBUG_ONLY_VARIABLE(result);

    wiced_wifi_get_mac_address( &my_mac );
    my_mac.octet[0] |= 0x2;

    wl_p2p_if_t* p2p_if = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_p2p_if_t), IOVAR_STR_P2P_IFADD );
    p2p_if->interface_type = P2P_GROUP_OWNER_MODE;
    memcpy( &p2p_if->mac_address, &my_mac, sizeof(besl_mac_t) );
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE );
    wiced_assert("", result == WICED_SUCCESS);

    /*  Enable discovery */
    data = wwd_sdpcm_get_iovar_buffer( &buffer, 4, IOVAR_STR_P2P_DISC );
    *data = 1;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE );
    wiced_assert("", result == WICED_SUCCESS);

    wiced_buffer_t response;
    wwd_sdpcm_get_iovar_buffer( &buffer, 4, IOVAR_STR_P2P_DEV );
    result = wwd_sdpcm_send_iovar( SDPCM_GET, buffer, &response, WICED_STA_INTERFACE );
    wiced_assert("", result == WICED_SUCCESS);
    workspace->p2p_interface = BESL_READ_32(host_buffer_get_current_piece_data_pointer(response));
    host_buffer_release( response, WICED_NETWORK_RX );
    BESL_DEBUG( ("interface = %lu\n", workspace->p2p_interface) );

    data = wiced_get_ioctl_buffer( &buffer, 4 );
    *data = 1;
    result = wiced_send_ioctl( SDPCM_SET, WLC_SET_CHANNEL, buffer, NULL, workspace->p2p_interface );
    wiced_assert("", result == WICED_SUCCESS);

    /*  Set WSEC */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSSCFG_WSEC );
    data[0] = (uint32_t) workspace->p2p_interface;
    data[1] = WICED_SECURITY_WPA2_AES_PSK;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, SDPCM_STA_INTERFACE );
    wiced_assert("", result == WICED_SUCCESS);

    wsec_pmk_t* psk;

    /* Set the wpa auth */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) 8, IOVAR_STR_BSSCFG_WPA_AUTH );
    data[0] = (uint32_t) workspace->p2p_interface;
    data[1] = (uint32_t) WPA2_AUTH_PSK;
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, SDPCM_STA_INTERFACE );
    wiced_assert("", result == WICED_SUCCESS );

    /* Set the passphrase */
    psk = (wsec_pmk_t*) wiced_get_ioctl_buffer( &buffer, sizeof(wsec_pmk_t) );
    memcpy( psk->key, "YOUR_AP_PASSPHRASE", 18 );
    psk->key_len = 18;
    psk->flags = (uint16_t) WSEC_PASSPHRASE;
    host_rtos_delay_milliseconds( 1 ); /*  Delay required to allow radio firmware to be ready to receive PMK and avoid intermittent failure */
    result = wiced_send_ioctl( SDPCM_SET, WLC_SET_WSEC_PMK, buffer, 0, workspace->p2p_interface );
    wiced_assert("", result == WICED_SUCCESS );

    wlc_ssid_t* p2p_ssid = wwd_sdpcm_get_iovar_buffer( &buffer, sizeof(wl_p2p_if_t), IOVAR_STR_P2P_SSID );
    p2p_ssid->SSID_len = 9;
    memcpy( p2p_ssid->SSID, "DIRECT-ww", 9 );
    result = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, NULL, WICED_STA_INTERFACE );
    wiced_assert("", result == WICED_SUCCESS);

    /* Bring up P2P interface */
    wiced_get_ioctl_buffer( &buffer, 0 );
    result = wiced_send_ioctl( SDPCM_SET, WLC_UP, buffer, NULL, workspace->p2p_interface );
    wiced_assert("", result == WICED_SUCCESS);

    p2p_set_discovery_state( P2P_DISCOVERY_STATE_LISTEN );
}
#endif

