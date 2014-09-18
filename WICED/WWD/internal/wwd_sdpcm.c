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
 *  Provides an implementation of the Broadcom SDPCM protocol.
 *  The Broadcom SDPCM protocol provides multiplexing of Wireless Data frames,
 *  I/O Control functions (IOCTL), and Asynchronous Event signalling.
 *  It is required when communicating with Broadcom 802.11 devices.
 *
 */

#include <stdint.h>
#include <string.h> /* For strlen, memcpy */
#include "wwd_rtos.h"
#include "wwd_buffer.h"
#include "wwd_constants.h"
#include "wwd_wifi.h"
#include "wwd_assert.h"
#include "wwd_logging.h"
#include "wwd_events.h"
#include "RTOS/wwd_rtos_interface.h"
#include "network/wwd_buffer_interface.h"
#include "network/wwd_network_interface.h"
#include "network/wwd_network_constants.h"
#include "internal/wwd_sdpcm.h"
#include "internal/wwd_ap.h"
#include "internal/wwd_thread.h"
#include "internal/wwd_bcmendian.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"
#include "internal/wwd_internal.h"
#include "wwd_management.h"

/******************************************************
 * @cond       Constants
 ******************************************************/

#ifndef WWD_EVENT_HANDLER_LIST_SIZE
#define WWD_EVENT_HANDLER_LIST_SIZE    (5)      /** Maximum number of simultaneously registered event handlers */
#endif

#define WWD_IOCTL_TIMEOUT_MS         (400)


#define BDC_PROTO_VER                  (2)      /** Version number of BDC header */
#define BDC_FLAG_VER_SHIFT             (4)      /** Number of bits to shift BDC version number in the flags field */
#define ETHER_TYPE_BRCM           (0x886C)      /** Broadcom Ethertype for identifying event packets - Copied from DHD include/proto/ethernet.h */
#define BRCM_OUI            "\x00\x10\x18"      /** Broadcom OUI (Organizationally Unique Identifier): Used in the proprietary(221) IE (Information Element) in all Broadcom devices */
#define DOT11_OUI_LEN                  (3)      /** Length in bytes of 802.11 OUI*/
#define BCM_MSG_IFNAME_MAX            (16)      /** Maximum length of an interface name in a wl_event_msg_t structure*/

/* QoS related definitions (type of service) */
#define IPV4_DSCP_OFFSET              (15)      /** Offset for finding the DSCP field in an IPv4 header */

/* CDC flag definitions taken from bcmcdc.h */
#define CDCF_IOC_ERROR              (0x01)      /** 0=success, 1=ioctl cmd failed */
#define CDCF_IOC_IF_MASK          (0xF000)      /** I/F index */
#define CDCF_IOC_IF_SHIFT             (12)      /** # of bits of shift for I/F Mask */
#define CDCF_IOC_ID_MASK      (0xFFFF0000)      /** used to uniquely id an ioctl req/resp pairing */
#define CDCF_IOC_ID_SHIFT             (16)      /** # of bits of shift for ID Mask */

#define BDC_FLAG2_IF_MASK           (0x0f)

#define SDPCM_HEADER_LEN              (12)
#define BDC_HEADER_LEN                 (4)

/* Event flags */
#define WLC_EVENT_MSG_LINK      (0x01)    /** link is up */
#define WLC_EVENT_MSG_FLUSHTXQ  (0x02)    /** flush tx queue on MIC error */
#define WLC_EVENT_MSG_GROUP     (0x04)    /** group MIC error */
#define WLC_EVENT_MSG_UNKBSS    (0x08)    /** unknown source bsscfg */
#define WLC_EVENT_MSG_UNKIF     (0x10)    /** unknown source OS i/f */


/******************************************************
 *             Macros
 ******************************************************/

/* bit map related macros */
#ifndef setbit
#ifndef NBBY      /* the BSD family defines NBBY */
#define  NBBY  8  /* 8 bits per byte */
#endif /* #ifndef NBBY */
#define  setbit(a, i)   (((uint8_t*)       a)[(int)(i)/(int)(NBBY)] |= (uint8_t)(1<<((i)%NBBY)))
#define  clrbit(a, i)   (((uint8_t*)       a)[(int)(i)/(int)(NBBY)] &= (uint8_t)~(1<<((i)%NBBY)))
#define  isset(a, i)    (((const uint8_t*) a)[(int)(i)/(int)(NBBY)] & (1<<((i)%NBBY)))
#define  isclr(a, i)    ((((const uint8_t*)a)[(int)(i)/(int)(NBBY)] & (1<<((i)%NBBY))) == 0)
#endif /* setbit */

#define DATA_AFTER_HEADER( x )   ((void*)(&x[1]))

/******************************************************
 *             Local Structures
 ******************************************************/

typedef enum
{
    DATA_HEADER       = 2,
    ASYNCEVENT_HEADER = 1,
    CONTROL_HEADER    = 0
} sdpcm_header_type_t;

#pragma pack(1)

/*TODO: Keep this typedef? (in preference to the defines above */
#if 0
typedef struct
{
    uint16_t control_id;
    uint8_t  interface_index :4;
    uint16_t reserved        :10;
    uint8_t  set             :1;
    uint8_t  error           :1;
}sdpcm_cdc_flags_t;
#endif /* if 0 */
typedef struct
{
    uint8_t sequence;
    uint8_t channel_and_flags;
    uint8_t next_length;
    uint8_t header_length;
    uint8_t wireless_flow_control;
    uint8_t bus_data_credit;
    uint8_t _reserved[2];
} sdpcm_sw_header_t;

typedef struct
{
    uint32_t cmd;    /* ioctl command value */
    uint32_t len;    /* lower 16: output buflen; upper 16: input buflen (excludes header) */
    uint32_t flags;  /* flag defns given in bcmcdc.h */
    uint32_t status; /* status code returned from the device */
} sdpcm_cdc_header_t;

typedef struct
{
    uint8_t flags;      /* Flags */
    uint8_t priority;   /* 802.1d Priority (low 3 bits) */
    uint8_t flags2;
    uint8_t data_offset; /* Offset from end of BDC header to packet data, in 4-uint8_t words.  Leaves room for optional headers.*/
} sdpcm_bdc_header_t;

typedef struct
{
    wiced_mac_t   destination_address;
    wiced_mac_t   source_address;
    uint16_t    ethertype;
} sdpcm_ethernet_header_t;


/*
 * SDPCM header definitions
 */
typedef struct
{
    uint16_t           frametag[2];
    sdpcm_sw_header_t  sw_header;
} sdpcm_header_t;

/*
 * SDPCM Packet structure definitions
 */
typedef struct
{
    wwd_buffer_header_t    buffer_header;
    sdpcm_header_t         sdpcm_header;
} sdpcm_common_header_t;

typedef struct
{
    sdpcm_common_header_t  common;
    sdpcm_cdc_header_t     cdc_header;
} sdpcm_control_header_t;

typedef struct
{
    sdpcm_common_header_t  common;
    uint8_t                _padding[2];
    sdpcm_bdc_header_t     bdc_header;
} sdpcm_data_header_t;

typedef struct bcmeth_hdr
{
    uint16_t subtype;      /** Vendor specific..32769 */
    uint16_t length;
    uint8_t  version;      /** Version is 0 */
    uint8_t  oui[3];       /** Broadcom OUI */
    uint16_t usr_subtype;  /** user specific Data */
} sdpcm_bcmeth_header_t;

/* these fields are stored in network order */
typedef struct
{

    uint16_t     version;                     /** Version 1 has fields up to ifname.  Version 2 has all fields including ifidx and bss_cfg_idx */
    uint16_t     flags;                       /** see flags */
    uint32_t     event_type;                  /** Message */
    uint32_t     status;                      /** Status code */
    uint32_t     reason;                      /** Reason code (if applicable) */
    uint32_t     auth_type;                   /** WLC_E_AUTH */
    uint32_t     datalen;                     /** data buf */
    wiced_mac_t  addr;                        /** Station address (if applicable) */
    char         ifname[BCM_MSG_IFNAME_MAX];  /** name of the packet incoming interface */
    uint8_t      ifidx;                       /** destination OS i/f index */
    uint8_t      bss_cfg_idx;                 /** source bsscfg index */
} sdpcm_raw_event_header_t;

/* used by driver msgs */
typedef struct bcm_event
{
    sdpcm_ethernet_header_t      ether;
    sdpcm_bcmeth_header_t        bcmeth;
    union
    {
        wwd_event_header_t       wwd;
        sdpcm_raw_event_header_t raw;
    } event;
} sdpcm_bcm_event_t;

#pragma pack()

/** Event list element structure
 *
 * events : A pointer to a wwd_event_num_t array that is terminated with a WLC_E_NONE event
 * handler: A pointer to the wwd_event_handler_t function that will receive the event
 * handler_user_data : User provided data that will be passed to the handler when a matching event occurs
 */
typedef struct
{
    const /*@null@*/ wwd_event_num_t* events;
    /*@null@*/ wwd_event_handler_t    handler;
    /*@null@*/ void*                  handler_user_data;
} sdpcm_event_list_elem_t;

/** @endcond */

/******************************************************
 *             Static Variables
 ******************************************************/

/* Event list variables */
static sdpcm_event_list_elem_t  wwd_sdpcm_event_list[WWD_EVENT_HANDLER_LIST_SIZE];

/* IOCTL variables*/
static uint16_t                  wwd_sdpcm_requested_ioctl_id;
static host_semaphore_type_t     wwd_sdpcm_ioctl_mutex;
static /*@only@*/ wiced_buffer_t wwd_sdpcm_ioctl_response;
static host_semaphore_type_t     wwd_sdpcm_ioctl_sleep;
static uint32_t                  wwd_sdpcm_ioctl_abort_count = 0;

/* Bus data credit variables */
static uint8_t wwd_sdpcm_packet_transmit_sequence_number;
static uint8_t wwd_sdpcm_last_bus_data_credit    = 0;
static uint8_t wwd_sdpcm_credit_diff             = 0;
static uint8_t wwd_sdpcm_largest_credit_diff     = 0;

/* QoS related variables */
static uint8_t wwd_sdpcm_highest_rx_tos = 0;

/* Packet send queue variables */
static host_semaphore_type_t                 wwd_sdpcm_send_queue_mutex;
static wiced_buffer_t /*@owned@*/ /*@null@*/ wwd_sdpcm_send_queue_head   = (wiced_buffer_t) NULL;
static wiced_buffer_t /*@owned@*/ /*@null@*/ wwd_sdpcm_send_queue_tail   = (wiced_buffer_t) NULL;

static wwd_wifi_raw_packet_processor_t       wwd_sdpcm_raw_packet_processor = NULL;

/******************************************************
 *             SDPCM Logging
 *
 * Enable this section to allow logging of SDPCM packets
 * into a buffer for later perusal
 *
 * See sdpcm_log  and  next_sdpcm_log_pos
 *
 ******************************************************/
/** @cond */

#if 0

#define SDPCM_LOG_SIZE 30
#define SDPCM_LOG_HEADER_SIZE (0x60)

typedef enum { UNUSED, LOG_TX, LOG_RX } sdpcm_log_direction_t;
typedef enum { IOCTL, DATA, EVENT } sdpcm_log_type_t;

typedef struct SDPCM_log_entry_struct
{
    sdpcm_log_direction_t direction;
    sdpcm_log_type_t      type;
    unsigned long         time;
    unsigned long         length;
    unsigned char         header[SDPCM_LOG_HEADER_SIZE];
}sdpcm_log_entry_t;

static int next_sdpcm_log_pos = 0;
static sdpcm_log_entry_t sdpcm_log[SDPCM_LOG_SIZE];

static void add_sdpcm_log_entry( sdpcm_log_direction_t dir, sdpcm_log_type_t type, unsigned long length, char* eth_data )
{

    sdpcm_log[next_sdpcm_log_pos].direction = dir;
    sdpcm_log[next_sdpcm_log_pos].type = type;
    sdpcm_log[next_sdpcm_log_pos].time = host_rtos_get_time();
    sdpcm_log[next_sdpcm_log_pos].length = length;
    memcpy( sdpcm_log[next_sdpcm_log_pos].header, eth_data, SDPCM_LOG_HEADER_SIZE );
    next_sdpcm_log_pos++;
    if (next_sdpcm_log_pos >= SDPCM_LOG_SIZE)
    {
        next_sdpcm_log_pos = 0;
    }
}
#else
#define add_sdpcm_log_entry( dir, type, length, eth_data )
#endif

/** @endcond */

/******************************************************
 *             Static Function Prototypes
 ******************************************************/

static wiced_buffer_t wwd_sdpcm_get_next_buffer_in_queue ( wiced_buffer_t buffer );
static void           wwd_sdpcm_set_next_buffer_in_queue ( wiced_buffer_t buffer, wiced_buffer_t prev_buffer );
static void           wwd_sdpcm_send_common              ( /*@only@*/ wiced_buffer_t buffer, sdpcm_header_type_t header_type );
static uint8_t        wwd_map_dscp_to_priority           ( uint8_t dscp_val );

/******************************************************
 *             Function definitions
 ******************************************************/

/** Initialises the SDPCM protocol handler
 *
 *  Initialises mutex and semaphore flags needed by the SDPCM handler.
 *  Also initialises the list of event handlers. This function is called
 *  from the @ref wwd_thread_init function.
 *
 * @return    WWD result code
 */

wwd_result_t wwd_sdpcm_init( void )
{
    uint16_t i;
    wwd_sdpcm_ioctl_abort_count = 0;

    /* Create the mutex protecting the packet send queue */
    if ( host_rtos_init_semaphore( &wwd_sdpcm_ioctl_mutex ) != WWD_SUCCESS )
    {
        return WWD_SEMAPHORE_ERROR;
    }
    if ( host_rtos_set_semaphore( &wwd_sdpcm_ioctl_mutex, WICED_FALSE ) != WWD_SUCCESS )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    /* Create the event flag which signals the wwd thread needs to wake up */
    if ( host_rtos_init_semaphore( &wwd_sdpcm_ioctl_sleep ) != WWD_SUCCESS )
    {
        host_rtos_deinit_semaphore( &wwd_sdpcm_ioctl_mutex );
        return WWD_SEMAPHORE_ERROR;
    }

    /* Create the sdpcm packet queue semaphore */
    if ( host_rtos_init_semaphore( &wwd_sdpcm_send_queue_mutex ) != WWD_SUCCESS )
    {
        host_rtos_deinit_semaphore( &wwd_sdpcm_ioctl_sleep );
        host_rtos_deinit_semaphore( &wwd_sdpcm_ioctl_mutex );
        return WWD_SEMAPHORE_ERROR;
    }
    if ( host_rtos_set_semaphore( &wwd_sdpcm_send_queue_mutex, WICED_FALSE ) != WWD_SUCCESS )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    wwd_sdpcm_send_queue_head = NULL;
    wwd_sdpcm_send_queue_tail = NULL;

    /* Initialise the list of event handler functions */
    for ( i = 0; i < (uint16_t) WWD_EVENT_HANDLER_LIST_SIZE; i++ )
    {
        wwd_sdpcm_event_list[i].events = NULL;
        wwd_sdpcm_event_list[i].handler = NULL;
        wwd_sdpcm_event_list[i].handler_user_data = NULL;
    }

    wwd_sdpcm_highest_rx_tos = 0;
    wwd_sdpcm_packet_transmit_sequence_number = 0;
    wwd_sdpcm_last_bus_data_credit = (uint8_t) 1;

    return WWD_SUCCESS;
}


/** Initialises the SDPCM protocol handler
 *
 *  De-initialises mutex and semaphore flags needed by the SDPCM handler.
 *  This function is called from the @ref wwd_thread_func function when it is exiting.
 */

void wwd_sdpcm_quit( void ) /*@globals killed wwd_sdpcm_ioctl_sleep, killed wwd_sdpcm_ioctl_mutex, killed wwd_sdpcm_send_queue_mutex@*/ /*@modifies wwd_sdpcm_send_queue_head@*/
{
    /* Delete the sleep mutex */
    (void) host_rtos_deinit_semaphore( &wwd_sdpcm_ioctl_sleep ); /* Ignore return - not much can be done about failure */

    /* Delete the queue mutex.  */
    (void) host_rtos_deinit_semaphore( &wwd_sdpcm_ioctl_mutex ); /* Ignore return - not much can be done about failure */

    /* Delete the SDPCM queue mutex */
    (void) host_rtos_deinit_semaphore( &wwd_sdpcm_send_queue_mutex ); /* Ignore return - not much can be done about failure */

    /* Free any left over packets in the queue */
    while (wwd_sdpcm_send_queue_head != NULL)
    {
        wiced_buffer_t buf = wwd_sdpcm_get_next_buffer_in_queue( wwd_sdpcm_send_queue_head );
        host_buffer_release(wwd_sdpcm_send_queue_head, WWD_NETWORK_TX);
        wwd_sdpcm_send_queue_head = buf;
    }
}


/** Sets/Gets an I/O Variable (IOVar)
 *
 *  This function either sets or retrieves the value of an I/O variable from the Broadcom 802.11 device.
 *  The data which is set or retrieved must be in a format structure which is appropriate for the particular
 *  I/O variable being accessed. These structures can only be found in the DHD source code such as wl/exe/wlu.c.
 *
 *  @Note: The function blocks until the I/O variable read/write has completed
 *
 * @param type       : SDPCM_SET or SDPCM_GET - indicating whether to set or get the I/O variable value
 * @param send_buffer_hnd : A handle for a packet buffer containing the data value to be sent.
 * @param response_buffer_hnd : A pointer which will receive the handle for the packet buffer containing the response data value received..
 * @param interface : Which interface to send the iovar to (AP or STA)
 *
 * @return    WWD result code
 */
wwd_result_t wwd_sdpcm_send_iovar( sdpcm_command_type_t type, /*@only@*/ wiced_buffer_t send_buffer_hnd, /*@special@*/ /*@out@*/ /*@null@*/ wiced_buffer_t* response_buffer_hnd, wwd_interface_t interface )  /*@allocates *response_buffer_hnd@*/  /*@defines **response_buffer_hnd@*/
{
    if ( type == SDPCM_SET )
    {
        return wwd_sdpcm_send_ioctl( SDPCM_SET, (uint32_t) WLC_SET_VAR, send_buffer_hnd, response_buffer_hnd, interface );
    }
    else
    {
        return wwd_sdpcm_send_ioctl( SDPCM_GET, (uint32_t) WLC_GET_VAR, send_buffer_hnd, response_buffer_hnd, interface );
    }
}

/** Sends a data packet.
 *
 *  This function should be called by the bottom of the network stack in order for it
 *  to send an ethernet frame.
 *  The function prepends a BDC header, before sending to @ref wwd_sdpcm_send_common where
 *  the SDPCM header will be added
 *
 * @param buffer  : The ethernet packet buffer to be sent
 * @param interface : the interface over which to send the packet (AP or STA)
 *
 */
void wwd_network_send_ethernet_data( /*@only@*/ wiced_buffer_t buffer, wwd_interface_t interface ) /* Returns immediately - Wiced_buffer_tx_completed will be called once the transmission has finished */
{
    sdpcm_data_header_t* packet;
    wwd_result_t         result;
    uint8_t* dscp = NULL;
    uint8_t priority = 0;
    static const int32_t packet_overhead = (int32_t) ( sizeof( sdpcm_common_header_t ) + sizeof ( uint8_t ) * 2 + sizeof ( sdpcm_bdc_header_t ) );
    sdpcm_ethernet_header_t* ethernet_header = (sdpcm_ethernet_header_t*)host_buffer_get_current_piece_data_pointer( buffer );
    uint16_t ether_type;

    ether_type = NTOH16( ethernet_header->ethertype );
    if ( ether_type == WICED_ETHERTYPE_IPv4 )
    {
        dscp = (uint8_t*)host_buffer_get_current_piece_data_pointer( buffer ) + IPV4_DSCP_OFFSET;
    }

    add_sdpcm_log_entry( LOG_TX, DATA, host_buffer_get_current_piece_size( buffer ), (char*) host_buffer_get_current_piece_data_pointer( buffer ) );

    WWD_LOG( ( "Wcd:> DATA pkt 0x%08X len %d\n", (unsigned int)buffer, (int)host_buffer_get_current_piece_size( buffer ) ) );


    /* Add link space at front of packet */
    result = host_buffer_add_remove_at_front( &buffer, -packet_overhead );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_WWD_DEBUG(("Unable to adjust header space\n"));
        host_buffer_release( buffer, WWD_NETWORK_TX );
        return;
    }

    packet = (sdpcm_data_header_t*) host_buffer_get_current_piece_data_pointer( buffer );

    if ( ( interface != WWD_STA_INTERFACE ) &&
         ( interface != WWD_AP_INTERFACE  ) )
    {
        WPRINT_WWD_DEBUG(("No interface for packet send\n"));
        host_buffer_release( buffer, WWD_NETWORK_TX );
        return;
    }

    /* Prepare the BDC header */
    packet->bdc_header.flags = 0;
    packet->bdc_header.flags = (uint8_t) ( BDC_PROTO_VER << BDC_FLAG_VER_SHIFT );
    packet->bdc_header.flags2 = (uint8_t) ( (interface == WWD_STA_INTERFACE)?CHIP_STA_INTERFACE:CHIP_AP_INTERFACE );
    packet->bdc_header.data_offset = 0;

    /* If it's an IPv4 packet set the BDC header priority based on the DSCP field */
    if ( ( ether_type == WICED_ETHERTYPE_IPv4 ) && ( dscp != NULL ) )
    {
        if ( *dscp != 0 ) /* If it's equal 0 then it's best effort traffic and nothing needs to be done */
        {
            priority = wwd_map_dscp_to_priority( *dscp );
        }
    }

    /* If it's for the STA interface then re-map priority to the priority allowed by the AP, regardless of whether it's an IPv4 packet */
    if ( interface == WWD_STA_INTERFACE )
    {
        packet->bdc_header.priority = wwd_tos_map[ priority ];
    }
    else
    {
        packet->bdc_header.priority = priority;
    }

    /* Add the length of the BDC header and pass "down" */
    wwd_sdpcm_send_common( buffer, DATA_HEADER );
}

void wwd_sdpcm_update_credit(uint8_t* data)
{
    sdpcm_sw_header_t* header = (sdpcm_sw_header_t*)(data + 4);

    if ( ( header->channel_and_flags & 0x0f ) < (uint8_t) 3 )
    {
        wwd_sdpcm_credit_diff = (uint8_t)(header->bus_data_credit - wwd_sdpcm_last_bus_data_credit);
        if ( wwd_sdpcm_credit_diff <= (uint8_t) CHIP_MAX_BUS_DATA_CREDIT_DIFF )
        {
            wwd_sdpcm_last_bus_data_credit = header->bus_data_credit;
        }
        else
        {
            if (wwd_sdpcm_credit_diff > wwd_sdpcm_largest_credit_diff)
            {
                wwd_sdpcm_largest_credit_diff = wwd_sdpcm_credit_diff;
            }
        }
    }

    wwd_bus_set_flow_control(header->wireless_flow_control);
}

/** Processes and directs incoming SDPCM packets
 *
 *  This function receives SDPCM packets from the Broadcom 802.11 device and decodes the SDPCM header
 *  to determine where the packet should be directed.
 *
 *  - Control packets (IOCTL/IOVAR) cause the IOCTL flag to be set to allow the resumption of the thread
 *    which sent the IOCTL
 *  - Data Packets are sent to the bottom layer of the network stack via the @ref host_network_process_ethernet_data function
 *  - Event Packets are decoded to determine which event occurred, and the event handler list is consulted
 *    and the appropriate event handler is called.
 *
 * @param buffer  : The SDPCM packet buffer received from the Broadcom 802.11 device
 *
 */
void wwd_sdpcm_process_rx_packet( /*@only@*/ wiced_buffer_t buffer )
{
    sdpcm_common_header_t* packet;
    uint16_t i;
    uint16_t j;
    uint16_t size;
    uint16_t size_inv;

    packet = (sdpcm_common_header_t*) host_buffer_get_current_piece_data_pointer( buffer );

    /* Extract the total SDPCM packet size from the first two frametag bytes */
    size = packet->sdpcm_header.frametag[0];

    /* Check that the second two frametag bytes are the binary inverse of the size */
    size_inv = (uint16_t) ~size;  /* Separate variable due to GCC Bug 38341 */
    if ( packet->sdpcm_header.frametag[1] != size_inv )
    {
        WPRINT_WWD_DEBUG(("Received a packet with a frametag which is wrong\n"));
        host_buffer_release( buffer, WWD_NETWORK_RX );
        return;
    }

    /* Check whether the size is big enough to contain the SDPCM header */
    if ( size < (uint16_t) 12 )
    {
        WPRINT_WWD_DEBUG(("Received a packet that is too small to contain anything useful\n"));
        host_buffer_release( buffer, WWD_NETWORK_RX );
        return;
    }

    wwd_sdpcm_update_credit((uint8_t*)&packet->sdpcm_header.frametag);

    if ( size == (uint16_t) 12 )
    {
        /* This is a flow control update packet with no data - release it. */
        host_buffer_release( buffer, WWD_NETWORK_RX );
        return;
    }

    wiced_assert( "Packet too big", size <= (uint16_t) WICED_LINK_MTU );
    /* Check the SDPCM channel to decide what to do with packet. */
    switch ( packet->sdpcm_header.sw_header.channel_and_flags & 0x0f )
    {
        case CONTROL_HEADER:  /* IOCTL/IOVAR reply packet */
            add_sdpcm_log_entry( LOG_RX, IOCTL, host_buffer_get_current_piece_size( buffer ), (char*) host_buffer_get_current_piece_data_pointer( buffer ) );

            /* Check that packet size is big enough to contain the CDC header as well as the SDPCM header */
            if ( packet->sdpcm_header.frametag[0] < ( sizeof( packet->sdpcm_header.frametag ) + sizeof(sdpcm_sw_header_t) + sizeof(sdpcm_cdc_header_t) ) )
            {
                /* Received a too-short SDPCM packet! */
                WPRINT_WWD_DEBUG(("Received a too-short SDPCM packet!\n"));
                host_buffer_release( buffer, WWD_NETWORK_RX );
                break;
            }

            /* Ignore a reply for each aborted ioctl attempt to ensure wwd_sdpcm_ioctl_sleep semaphore does not get out of sync */
            if (wwd_sdpcm_ioctl_abort_count != 0)
            {
                --wwd_sdpcm_ioctl_abort_count;
                host_buffer_release( buffer, WWD_NETWORK_RX );
                break;
            }

            /* Save the response packet in a global variable */
            wwd_sdpcm_ioctl_response = buffer;

            WWD_LOG( ( "Wcd:< Procd pkt 0x%08X: IOCTL Response (%d bytes)\n", (unsigned int)buffer, size ) );

            /* Wake the thread which sent the IOCTL/IOVAR so that it will resume */
            host_rtos_set_semaphore( &wwd_sdpcm_ioctl_sleep, WICED_FALSE );
            break;

        case DATA_HEADER:
            {
                sdpcm_bdc_header_t* bdc_header;
                int32_t headers_len_below_payload;

                /* Check that the packet is big enough to contain SDPCM & BDC headers */
                if ( packet->sdpcm_header.frametag[0] <= (uint16_t)( SDPCM_HEADER_LEN + BDC_HEADER_LEN ) )
                {
                    WPRINT_WWD_DEBUG(("Packet too small to contain SDPCM + BDC headers\n"));
                    host_buffer_release( buffer, WWD_NETWORK_RX );
                    break;
                }

                /* Calculate where the BDC header is - this is dependent on the data offset field of the SDPCM SW header */
                bdc_header = (sdpcm_bdc_header_t*) &((char*)&packet->sdpcm_header)[ packet->sdpcm_header.sw_header.header_length ];

                /* Calculate where the payload is - this is dependent on the data offset fields of the SDPCM SW header and the BDC header */
                headers_len_below_payload = (int32_t) ( (int32_t)sizeof(wwd_buffer_header_t) + (int32_t) packet->sdpcm_header.sw_header.header_length + (int32_t)BDC_HEADER_LEN + (int32_t)(bdc_header->data_offset<<2) );

                /* Move buffer pointer past gSPI, SDPCM, BCD headers and padding, so that the network stack or 802.11 monitor sees only the payload */
                if ( WWD_SUCCESS != host_buffer_add_remove_at_front( &buffer, headers_len_below_payload ) )
                {
                    WPRINT_WWD_DEBUG(("No space for headers without chaining. this should never happen\n"));
                    host_buffer_release( buffer, WWD_NETWORK_RX );
                    break;
                }

                add_sdpcm_log_entry( LOG_RX, DATA, host_buffer_get_current_piece_size( buffer ), (char*) host_buffer_get_current_piece_data_pointer( buffer ) );
                WWD_LOG( ( "Wcd:< Procd pkt 0x%08X: Data (%d bytes)\n", (unsigned int)buffer, size ) );


                /* Check if we are in monitor mode */
                if ( wwd_wifi_monitor_mode_is_enabled() == WICED_TRUE )
                {
                    if ( wwd_sdpcm_raw_packet_processor != NULL )
                    {
                        wwd_sdpcm_raw_packet_processor( buffer, WWD_STA_INTERFACE );
                    }
                    else
                    {
                        host_buffer_release( buffer, WWD_NETWORK_RX );
                    }
                }
                else
                {
                    /* Otherwise an Ethernet data frame received */

                    if ( bdc_header->priority > wwd_sdpcm_highest_rx_tos )
                    {
                        wwd_sdpcm_highest_rx_tos = bdc_header->priority;
                    }

                    /* Send packet to bottom of network stack */
                    host_network_process_ethernet_data( buffer, ( wwd_wifi_is_packet_from_ap( bdc_header->flags2 ) == WICED_TRUE ) ? WWD_AP_INTERFACE : WWD_STA_INTERFACE );
                    }
            }
            break;

        case ASYNCEVENT_HEADER:
            {
                sdpcm_bdc_header_t* bdc_header;
                uint16_t ether_type;
                wwd_event_header_t* wwd_event;
                sdpcm_bcm_event_t* event;

                bdc_header = (sdpcm_bdc_header_t*) &((char*)&packet->sdpcm_header)[ packet->sdpcm_header.sw_header.header_length ];

                event = (sdpcm_bcm_event_t*) &bdc_header[ bdc_header->data_offset + 1 ];

                ether_type = NTOH16( event->ether.ethertype );

                /* If frame is truly an event, it should have EtherType equal to the Broadcom type. */
                if ( ether_type != (uint16_t)ETHER_TYPE_BRCM )
                {
                    WPRINT_WWD_DEBUG(("Error - received a channel 1 packet which was not BRCM ethertype\n"));
                    host_buffer_release( buffer, WWD_NETWORK_RX );
                    break;
                }

                /* If ethertype is correct, the contents of the ethernet packet
                 * are a structure of type bcm_event_t
                 */

                /* Check that the OUI matches the Broadcom OUI */
                if ( 0 != memcmp( BRCM_OUI, &event->bcmeth.oui[0], (size_t)DOT11_OUI_LEN ) )
                {
                    WPRINT_WWD_DEBUG(("Event OUI mismatch\n"));
                    host_buffer_release( buffer, WWD_NETWORK_RX );
                    break;
                }

                wwd_event = &event->event.wwd;

                /* Search for the event type in the list of event handler functions
                 * event data is stored in network endianness
                 */
                wwd_event->flags      =                        NTOH16( wwd_event->flags      );
                wwd_event->event_type = (wwd_event_num_t)      NTOH32( wwd_event->event_type );
                wwd_event->status     = (wwd_event_status_t)   NTOH32( wwd_event->status     );
                wwd_event->reason     = (wwd_event_reason_t)   NTOH32( wwd_event->reason     );
                wwd_event->auth_type  =                        NTOH32( wwd_event->auth_type  );
                wwd_event->datalen    =                        NTOH32( wwd_event->datalen    );
                wwd_event->interface  = ( event->event.raw.ifidx == CHIP_STA_INTERFACE )? WWD_STA_INTERFACE: WWD_AP_INTERFACE;


                /* This is necessary because people who defined event statuses and reasons overlapped values. */
                if ( wwd_event->event_type == WLC_E_PSK_SUP )
                {
                    wwd_event->status = (wwd_event_status_t) ( (int)wwd_event->status + WLC_SUP_STATUS_OFFSET   );
                    wwd_event->reason = (wwd_event_reason_t) ( (int)wwd_event->reason + WLC_E_SUP_REASON_OFFSET );
                }
                else if ( wwd_event->event_type == WLC_E_PRUNE )
                {
                    wwd_event->reason = (wwd_event_reason_t) ( (int)wwd_event->reason + WLC_E_PRUNE_REASON_OFFSET );
                }
                else if ( ( wwd_event->event_type == WLC_E_DISASSOC ) || ( wwd_event->event_type == WLC_E_DEAUTH ) )
                {
                    wwd_event->status = (wwd_event_status_t) ( (int)wwd_event->status + WLC_DOT11_SC_STATUS_OFFSET   );
                    wwd_event->reason = (wwd_event_reason_t) ( (int)wwd_event->reason + WLC_E_DOT11_RC_REASON_OFFSET );
                }

                for ( i = 0; i < (uint16_t) WWD_EVENT_HANDLER_LIST_SIZE; i++ )
                {
                    if ( wwd_sdpcm_event_list[i].events != NULL )
                    {
                        for ( j = 0; wwd_sdpcm_event_list[i].events[j] != WLC_E_NONE; ++j )
                        {
                            if ( wwd_sdpcm_event_list[i].events[j] == wwd_event->event_type )
                            {
                                /* Correct event type has been found - call the handler function and exit loop */
                                wwd_sdpcm_event_list[i].handler_user_data = wwd_sdpcm_event_list[i].handler( wwd_event, (uint8_t*) DATA_AFTER_HEADER( event ), wwd_sdpcm_event_list[i].handler_user_data );
                                /*@innerbreak@*/
                                break;
                            }
                        }
                    }
                }

                add_sdpcm_log_entry( LOG_RX, EVENT, host_buffer_get_current_piece_size( buffer ), (char*) host_buffer_get_current_piece_data_pointer( buffer ) );
                WWD_LOG( ( "Wcd:< Procd pkt 0x%08X: Evnt %d (%d bytes)\n", (unsigned int)buffer, (int)event->event.event_type, size ) );

                /* Release the event packet buffer */
                host_buffer_release( buffer, WWD_NETWORK_RX );
            }
            break;

        default:
            wiced_assert("SDPCM packet of unknown channel received - dropping packet", 0 != 0);
            host_buffer_release( buffer, WWD_NETWORK_RX );
            break;
    }
}


/** Sends an IOCTL command
 *
 *  Sends a I/O Control command to the Broadcom 802.11 device.
 *  The data which is set or retrieved must be in a format structure which is appropriate for the particular
 *  I/O control being sent. These structures can only be found in the DHD source code such as wl/exe/wlu.c.
 *  The I/O control will always respond with a packet buffer which may contain data in a format specific to
 *  the I/O control being used.
 *
 *  @Note: The caller is responsible for releasing the response buffer.
 *  @Note: The function blocks until the IOCTL has completed
 *  @Note: Only one IOCTL may happen simultaneously.
 *
 *  @param type       : SDPCM_SET or SDPCM_GET - indicating whether to set or get the I/O control
 *  @param send_buffer_hnd : A handle for a packet buffer containing the data value to be sent.
 *  @param response_buffer_hnd : A pointer which will receive the handle for the packet buffer containing the response data value received..
 *  @param interface : Which interface to send the iovar to (SDPCM_STA_INTERFACE or SDPCM_AP_INTERFACE)
 *
 *  @return    WWD result code
 */

wwd_result_t wwd_sdpcm_send_ioctl( sdpcm_command_type_t type, uint32_t command, /*@only@*/ wiced_buffer_t send_buffer_hnd, /*@special@*/ /*@out@*/ /*@null@*/  wiced_buffer_t* response_buffer_hnd, wwd_interface_t interface )  /*@allocates *response_buffer_hnd@*/  /*@defines **response_buffer_hnd@*/
{
    uint32_t data_length;
    uint32_t flags;
    uint16_t id;
    wwd_result_t retval;
    sdpcm_control_header_t* send_packet;
    sdpcm_common_header_t*  recv_packet;
    sdpcm_cdc_header_t* cdc_header;
    uint8_t interface_val = (uint8_t) ( (interface == WWD_STA_INTERFACE)?CHIP_STA_INTERFACE:CHIP_AP_INTERFACE );

    /* Acquire mutex which prevents multiple simultaneous IOCTLs */
    retval = host_rtos_get_semaphore( &wwd_sdpcm_ioctl_mutex, NEVER_TIMEOUT, WICED_FALSE );
    if ( retval != WWD_SUCCESS )
    {
        host_buffer_release( send_buffer_hnd, WWD_NETWORK_TX );
        return retval;
    }

    /* Get the data length and cast packet to a CDC SDPCM header */
    data_length = host_buffer_get_current_piece_size( send_buffer_hnd ) - sizeof(sdpcm_common_header_t) - sizeof(sdpcm_cdc_header_t);
    send_packet = (sdpcm_control_header_t*) host_buffer_get_current_piece_data_pointer( send_buffer_hnd );

    /* Check if IOCTL is actually IOVAR */
    if ( command == WLC_SET_VAR || command == WLC_GET_VAR )
    {
        uint8_t* data = DATA_AFTER_HEADER( send_packet );
        uint8_t* ptr = data;

        /* Calculate the offset added to compensate for IOVAR string creating unaligned data section */
        while ( *ptr == 0 )
        {
            ptr++;
        }
        if ( data != ptr )
        {
            data_length -= (uint32_t)( ptr - data );
            memmove( data, ptr, data_length );
            host_buffer_set_size( send_buffer_hnd, (uint16_t) ( data_length + sizeof(sdpcm_common_header_t) + sizeof(sdpcm_cdc_header_t) ) );
        }

    }

    /* Prepare the CDC header */
    send_packet->cdc_header.cmd    = command;
    send_packet->cdc_header.len    = data_length;
    send_packet->cdc_header.flags  = ( ( (uint32_t) ++wwd_sdpcm_requested_ioctl_id << CDCF_IOC_ID_SHIFT ) & CDCF_IOC_ID_MASK ) | type | (uint32_t) interface_val << 12;
    send_packet->cdc_header.status = 0;

    /* Manufacturing test can receive big buffers, but sending big buffers causes a wlan firmware error */
    if ( host_buffer_get_current_piece_size( send_buffer_hnd ) > 1500 )
    {
        host_buffer_set_size( send_buffer_hnd, 1500 );
        data_length = host_buffer_get_current_piece_size( send_buffer_hnd ) - sizeof(sdpcm_common_header_t) - sizeof(sdpcm_cdc_header_t);
    }

    WWD_LOG( ( "Wcd:> IOCTL pkt 0x%08X: cmd %d, len %d\n", (unsigned int)send_buffer_hnd, (int)command, data_length ) );

    /* Store the length of the data and the IO control header and pass "down" */
    wwd_sdpcm_send_common( send_buffer_hnd, CONTROL_HEADER );

    do
    {
        /* Wait till response has been received  */
        retval = host_rtos_get_semaphore( &wwd_sdpcm_ioctl_sleep, (uint32_t) WWD_IOCTL_TIMEOUT_MS, WICED_FALSE );
        if ( retval != WWD_SUCCESS )
        {
/*            wiced_assert("IOCTL Timed out\n", 0 != 0 ); */
            ++wwd_sdpcm_ioctl_abort_count;
            /* Release the mutex since wwd_sdpcm_ioctl_response will no longer be referenced. */
            host_rtos_set_semaphore( &wwd_sdpcm_ioctl_mutex, WICED_FALSE );
            return retval;
        }

        /* Cast the response to a CDC + SDPCM header */
        recv_packet = (sdpcm_common_header_t*) host_buffer_get_current_piece_data_pointer( wwd_sdpcm_ioctl_response );
        cdc_header = (sdpcm_cdc_header_t*) &((char*)&recv_packet->sdpcm_header)[ recv_packet->sdpcm_header.sw_header.header_length ];
        flags = ltoh32( cdc_header->flags );
        id = (uint16_t) ( ( flags & CDCF_IOC_ID_MASK ) >> CDCF_IOC_ID_SHIFT );

        /* Check that the IOCTL identifier matches the identifier that was sent */
        if ( id != wwd_sdpcm_requested_ioctl_id )
        {
            host_buffer_release( wwd_sdpcm_ioctl_response, WWD_NETWORK_RX );
            WPRINT_WWD_DEBUG(("Received a response for a different IOCTL - retry\n"));
        }
    } while ( id != wwd_sdpcm_requested_ioctl_id );


    retval = (wwd_result_t) ( WLAN_ENUM_OFFSET -  ltoh32( cdc_header->status ) );

    /* Check if the caller wants the response */
    if ( response_buffer_hnd != NULL )
    {
        *response_buffer_hnd = wwd_sdpcm_ioctl_response;
        host_buffer_add_remove_at_front( response_buffer_hnd, (int32_t) IOCTL_OFFSET );
    }
    else
    {
        host_buffer_release( wwd_sdpcm_ioctl_response, WWD_NETWORK_RX );
        recv_packet = 0; /* Note: packet will no longer be valid after freeing buffer */
    }

    wwd_sdpcm_ioctl_response = NULL;

    /* Release the mutex since wwd_sdpcm_ioctl_response will no longer be referenced. */
    host_rtos_set_semaphore( &wwd_sdpcm_ioctl_mutex, WICED_FALSE );


    /* Check whether the IOCTL response indicates it failed. */
    if ( ( flags & CDCF_IOC_ERROR ) != 0)
    {
        if ( response_buffer_hnd != NULL )
        {
            host_buffer_release( *response_buffer_hnd, WWD_NETWORK_RX );
            *response_buffer_hnd = NULL;
        }
/*        wiced_assert("IOCTL failed\n", 0 != 0 ); */
        return retval;
    }

    return WWD_SUCCESS;
}

/**
 * Registers a handler to receive event callbacks.
 *
 * This function registers a callback handler to be notified when
 * a particular event is received.
 *
 * Alternately the function clears callbacks for given event type.
 *
 * @note : Currently each event may only be registered to one handler
 *         and there is a limit to the number of simultaneously registered
 *         events
 *
 * @param  event_nums     An array of event types that is to trigger the handler. The array must be terminated with a WLC_E_NONE event
 *                        See @ref wwd_event_num_t for available events
 * @param handler_func   A function pointer to the new handler callback,
 *                        or NULL if callbacks are to be disabled for the given event type
 * @param handler_user_data  A pointer value which will be passed to the event handler function
 *                            at the time an event is triggered (NULL is allowed)
 * @param interface      The interface to set the handler for.
 *
 * @return WWD result code
 */
wwd_result_t wwd_management_set_event_handler( /*@keep@*/ const wwd_event_num_t* event_nums, /*@null@*/ wwd_event_handler_t handler_func, /*@null@*/ /*@keep@*/ void* handler_user_data, wwd_interface_t interface )
{
    wiced_buffer_t buffer;
    uint8_t* event_mask;
    uint16_t entry = (uint16_t) 0xFF;
    uint16_t i;
    uint16_t j;
    wwd_result_t res;
    uint32_t* data;

    /* Find an existing matching entry OR the next empty entry */
    for ( i = 0; i < (uint16_t) WWD_EVENT_HANDLER_LIST_SIZE; i++ )
    {
        /* Find a matching event list OR the first empty event entry */
        if ( wwd_sdpcm_event_list[i].events == event_nums )
        {
            /* Check if all the data already matches */
            if ( wwd_sdpcm_event_list[i].handler           == handler_func &&
                 wwd_sdpcm_event_list[i].handler_user_data == handler_user_data )
            {
                return WWD_SUCCESS;
            }

            /* Delete the entry */
            wwd_sdpcm_event_list[i].events = NULL;
            wwd_sdpcm_event_list[i].handler = NULL;
            wwd_sdpcm_event_list[i].handler_user_data = NULL;

            entry = i;
            break;
        }
        else if ( ( entry == (uint16_t) 0xFF ) && ( wwd_sdpcm_event_list[i].events == NULL ) )
        {
            entry = i;
        }
    }

    /* Check if handler function was provided */
    if ( handler_func != NULL )
    {
        /* Check if an empty entry was not found */
        if ( entry == (uint16_t) 0xFF )
        {
            WPRINT_WWD_DEBUG(("Out of space in event handlers table - try increasing WWD_EVENT_HANDLER_LIST_SIZE\n"));
            return WWD_OUT_OF_EVENT_HANDLER_SPACE;
        }

        /* Add the new handler in at the free space */
        wwd_sdpcm_event_list[entry].handler           = handler_func;
        wwd_sdpcm_event_list[entry].handler_user_data = handler_user_data;
        wwd_sdpcm_event_list[entry].events            = event_nums;
    }

    /* Send the new event mask value to the wifi chip */
    data = (uint32_t*) wwd_sdpcm_get_iovar_buffer( &buffer, (uint16_t) WL_EVENTING_MASK_LEN + 4, IOVAR_STR_BSSCFG_EVENT_MSGS );
    if ( data == NULL )
    {
        return WWD_BUFFER_UNAVAILABLE_PERMANENT;
    }

    data[0] = interface;
    event_mask = (uint8_t*)&data[1];

    /* Keep the wlan awake while we set the event_msgs */
    WWD_WLAN_KEEP_AWAKE( );

    /* Set the event bits for each event from every handler */
    memset( event_mask, 0, (size_t) WL_EVENTING_MASK_LEN );
    for ( i = 0; i < (uint16_t) WWD_EVENT_HANDLER_LIST_SIZE; i++ )
    {
        if ( wwd_sdpcm_event_list[i].events != NULL )
        {
            for ( j = 0; wwd_sdpcm_event_list[i].events[j] != WLC_E_NONE; j++ )
            {
                setbit(event_mask, wwd_sdpcm_event_list[i].events[j]);
            }
        }
    }
    res = wwd_sdpcm_send_iovar( SDPCM_SET, buffer, 0, WWD_STA_INTERFACE );

    /* The wlan chip can sleep from now on */
    WWD_WLAN_LET_SLEEP( );

    return res;
}

/** A helper function to easily acquire and initialise a buffer destined for use as an iovar
 *
 * @param  buffer      : A pointer to a wiced_buffer_t object where the created buffer will be stored
 * @param  data_length : The length of space reserved for user data
 * @param  name        : The name of the iovar
 *
 * @return A pointer to the start of user data with data_length space available
 */
/*@null@*/ /*@exposed@*/ void* wwd_sdpcm_get_iovar_buffer( /*@special@*/ /*@out@*/ wiced_buffer_t* buffer, uint16_t data_length, const char* name )  /*@allocates *buffer@*/ /*@defines **buffer@*/
{
    uint32_t name_length = strlen( name ) + 1; /* + 1 for terminating null */
    uint32_t name_length_alignment_offset = (64 - name_length) % sizeof(uint32_t);
    if ( host_buffer_get( buffer, WWD_NETWORK_TX, (unsigned short) ( IOCTL_OFFSET + data_length + name_length + name_length_alignment_offset ), WICED_TRUE ) == WWD_SUCCESS )
    {
        uint8_t* data = ( host_buffer_get_current_piece_data_pointer( *buffer ) + IOCTL_OFFSET );
        memset( data, 0, name_length_alignment_offset );
        memcpy( data + name_length_alignment_offset, name, name_length );
        return ( data + name_length + name_length_alignment_offset );
    }
    else
    {
        WPRINT_WWD_DEBUG(("Error - failed to allocate a packet buffer for IOVAR\n"));
        return NULL;
    }
}

/** A helper function to easily acquire and initialise a buffer destined for use as an ioctl
 *
 * @param  buffer      : A pointer to a wiced_buffer_t object where the created buffer will be stored
 * @param  data_length : The length of space reserved for user data
 *
 * @return A pointer to the start of user data with data_length space available
 */
/*@null@*/ /*@exposed@*/ void* wwd_sdpcm_get_ioctl_buffer( /*@special@*/ /*@out@*/ wiced_buffer_t* buffer, uint16_t data_length ) /*@allocates *buffer@*/  /*@defines **buffer@*/
{
    if ( host_buffer_get( buffer, WWD_NETWORK_TX, (unsigned short) ( IOCTL_OFFSET + data_length ), WICED_TRUE ) == WWD_SUCCESS )
    {
        return ( host_buffer_get_current_piece_data_pointer( *buffer ) + IOCTL_OFFSET );
    }
    else
    {
        WPRINT_WWD_DEBUG(("Error - failed to allocate a packet buffer for IOCTL\n"));
        return NULL;
    }
}


wwd_result_t wwd_sdpcm_get_packet_to_send( /*@special@*/ /*@out@*/  wiced_buffer_t* buffer) /*@allocates *buffer@*/  /*@defines **buffer@*/
{
    sdpcm_common_header_t* packet;
    if ( wwd_sdpcm_send_queue_head != NULL )
    {
        /* Check if we're being flow controlled */
        if ( wwd_bus_is_flow_controlled() == WICED_TRUE )
        {
            return WWD_FLOW_CONTROLLED;
        }

        /* Check if we have enough bus data credits spare */
        if ( wwd_sdpcm_packet_transmit_sequence_number == wwd_sdpcm_last_bus_data_credit )
        {
            return WWD_NO_CREDITS;
        }

        /* There is a packet waiting to be sent - send it then fix up queue and release packet */
        if ( host_rtos_get_semaphore( &wwd_sdpcm_send_queue_mutex, NEVER_TIMEOUT, WICED_FALSE ) != WWD_SUCCESS )
        {
            /* Could not obtain mutex, push back the flow control semaphore */
            return WWD_SEMAPHORE_ERROR;
        }

        /* Pop the head off and set the new send_queue head */
        *buffer = wwd_sdpcm_send_queue_head;
        wwd_sdpcm_send_queue_head = wwd_sdpcm_get_next_buffer_in_queue( *buffer );
        if ( wwd_sdpcm_send_queue_head == NULL )
        {
            wwd_sdpcm_send_queue_tail = NULL;
        }
        host_rtos_set_semaphore( &wwd_sdpcm_send_queue_mutex, WICED_FALSE );

        /* Set the sequence number */
        packet = (sdpcm_common_header_t*) host_buffer_get_current_piece_data_pointer( *buffer );
        packet->sdpcm_header.sw_header.sequence = wwd_sdpcm_packet_transmit_sequence_number;
        wwd_sdpcm_packet_transmit_sequence_number++;

        return WWD_SUCCESS;
    }
    else
    {
        return WWD_NO_PACKET_TO_SEND;
    }
}


/** Returns the number of bus credits available
 *
 * @return The number of bus credits available
 */
uint8_t wWd_sdpcm_get_available_credits( void )
{
    return (uint8_t)( wwd_sdpcm_last_bus_data_credit - wwd_sdpcm_packet_transmit_sequence_number );
}


/** Sets a handler functions for monitor mode
 *
 * Packets received in monitor mode have raw 802.11 headers
 * and cannot be processed by the normal stack.
 * Use this function to set a handler function which will
 * process and free the raw packets received in monitor mode.
 *
 * @param func : function pointer to handler. Set to NULL to clear handler.
 *
 * @return result code
 */
wwd_result_t wwd_wifi_set_raw_packet_processor( wwd_wifi_raw_packet_processor_t function )
{
    wwd_sdpcm_raw_packet_processor = function;
    return WWD_SUCCESS;
}


/******************************************************
 *             Static Functions
 ******************************************************/

/** Writes SDPCM headers and sends packet to WWD Thread
 *
 *  Prepends the given packet with a new SDPCM header,
 *  then passes the packet to the WWD thread via a queue
 *
 *  This function is called by @ref wwd_network_send_ethernet_data and @ref wwd_sdpcm_send_ioctl
 *
 *  @param buffer     : The handle of the packet buffer to send
 *  @param header_type  : DATA_HEADER, ASYNCEVENT_HEADER or CONTROL_HEADER - indicating what type of SDPCM packet this is.
 */

static void wwd_sdpcm_send_common( /*@only@*/ wiced_buffer_t buffer, sdpcm_header_type_t header_type )
{
    uint16_t size;
    sdpcm_common_header_t* packet = (sdpcm_common_header_t *) host_buffer_get_current_piece_data_pointer( buffer );

    size = host_buffer_get_current_piece_size( buffer );

#ifdef SUPPORT_BUFFER_CHAINING
    wiced_buffer_t tmp_buff;
    while ( NULL != (tmp_buff = host_buffer_get_next_piece( tmp_buff )
    {
        size += host_buffer_get_current_piece_size( tmp_buff )
    }
#endif /* ifdef SUPPORT_BUFFER_CHAINING */

    size = (uint16_t) ( size - (uint16_t) sizeof(wwd_buffer_header_t) );

    /* Prepare the SDPCM header */
    memset( (uint8_t*) &packet->sdpcm_header, 0, sizeof(sdpcm_header_t) );
    packet->sdpcm_header.sw_header.channel_and_flags = (uint8_t) header_type;
    packet->sdpcm_header.sw_header.header_length = ( header_type == DATA_HEADER ) ? sizeof(sdpcm_header_t) + 2 : sizeof(sdpcm_header_t);
    packet->sdpcm_header.sw_header.sequence = 0; /* Note: The real sequence will be written later */
    packet->sdpcm_header.frametag[0] = size;
    packet->sdpcm_header.frametag[1] = (uint16_t) ~size;

    add_sdpcm_log_entry( LOG_TX, ( header_type == DATA_HEADER )? DATA : ( header_type == CONTROL_HEADER)? IOCTL : EVENT, host_buffer_get_current_piece_size( buffer ), (char*) host_buffer_get_current_piece_data_pointer( buffer ) );

    /* Add the length of the SDPCM header and pass "down" */
    if ( host_rtos_get_semaphore( &wwd_sdpcm_send_queue_mutex, NEVER_TIMEOUT, WICED_FALSE ) != WWD_SUCCESS )
    {
        /* Could not obtain mutex */
        /* Fatal error */
        host_buffer_release(buffer, WWD_NETWORK_TX);
        return;
    }

    wwd_sdpcm_set_next_buffer_in_queue( NULL, buffer );
    if ( wwd_sdpcm_send_queue_tail != NULL )
    {
        wwd_sdpcm_set_next_buffer_in_queue( buffer, wwd_sdpcm_send_queue_tail );
    }
    wwd_sdpcm_send_queue_tail = buffer;
    if ( wwd_sdpcm_send_queue_head == NULL )
    {
        wwd_sdpcm_send_queue_head = buffer;
    }
    host_rtos_set_semaphore( &wwd_sdpcm_send_queue_mutex, WICED_FALSE );

    wwd_thread_notify();
}


static wiced_buffer_t wwd_sdpcm_get_next_buffer_in_queue( wiced_buffer_t buffer )
{
    wwd_buffer_header_t* packet = (wwd_buffer_header_t*) host_buffer_get_current_piece_data_pointer( buffer );
    return packet->queue_next;
}


/** Sets the next buffer in the send queue
 *
 *  The send queue is a linked list of packet buffers where the 'next' pointer
 *  is stored in the first 4 bytes of the buffer content.
 *  This function sets that pointer.
 *
 * @param buffer       : handle of packet in the send queue
 *        prev_buffer  : handle of new packet whose 'next' pointer will point to 'buffer'
 */
static void wwd_sdpcm_set_next_buffer_in_queue( wiced_buffer_t buffer, wiced_buffer_t prev_buffer )
{
    wwd_buffer_header_t* packet = (wwd_buffer_header_t*) host_buffer_get_current_piece_data_pointer( prev_buffer );
    packet->queue_next = buffer;
}



/** Map a DSCP value from an IP header to a WMM QoS priority
 *
 * @param dscp_val : DSCP value from IP header
 *
 * @return wmm_qos : WMM priority
 *
 */
static uint8_t wwd_map_dscp_to_priority( uint8_t val )
{
    uint8_t wmm_qos = 0;
    uint8_t dscp_val = (uint8_t)(val >> 2); /* DSCP field is the first 6 bits of the second byte of an IPv4 header */

    if ( dscp_val < 8 )        /* Best Effort traffic */
    {
        wmm_qos = 0;
    }
    else if ( dscp_val < 24 ) /* Background traffic */
    {
        wmm_qos = 1;
    }
    else if ( dscp_val < 32 ) /* Best Effort traffic again */
    {
        wmm_qos = 0;
    }
    else if ( dscp_val < 48 ) /* Video traffic */
    {
        wmm_qos = 5;
    }
    else if ( dscp_val < 64 ) /* Voice traffic */
    {
        wmm_qos = 7;
    }

    return wmm_qos;
}
