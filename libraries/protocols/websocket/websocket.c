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
#include "websocket.h"
#include "websocket_handshake.h"
#include "wwd_debug.h"
#include "wwd_crypto.h"
#include "wiced_utilities.h"
/******************************************************
 *                      Macros
 ******************************************************/

#define  IF_ERROR_RECORD_AND_EXIT( GENERIC_ERROR, WEBSOCKET_ERROR ) \
    {                                                               \
        if( GENERIC_ERROR!=WICED_SUCCESS )                          \
        {                                                           \
            websocket->error_type=WEBSOCKET_ERROR;                  \
            websocket->on_error(websocket);                         \
            return GENERIC_ERROR;                                   \
        }                                                           \
    }

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_FRAME_BYTES         1024
#define MASKING_KEY_BYTES       4
#define MAX_PAYLOAD_FIELD_BYTES 2
#define CONTROL_FIELD_BYTES     2
#define MAX_HEADER_SIZE         (MASKING_KEY_BYTES + MAX_PAYLOAD_FIELD_BYTES + CONTROL_FIELD_BYTES)
#define MAX_PAYLOAD             (MAX_FRAME_BYTES-MAX_HEADER_SIZE)
#define MAX_CONTROL_FRAME       125
#define MAX_SOCKETS             10

/*  (MSB)
*    0                   1                   2                   3
*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*   +-+-+-+-+-------+-+-------------+-------------------------------+
*   |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
*   |I|S|S|S|  (4)  |A|     (7)     |             (16/63)           |
*   |N|V|V|V|       |S|             |   (if payload len==126/127)   |
*   | |1|2|3|       |K|             |                               |
*   +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
*   |     Extended payload length continued, if payload len == 127  |
*   + - - - - - - - - - - - - - - - +-------------------------------+
*   |                               |         Masking-key           |
*   +-------------------------------+ - - - - - - - - - - - - - - - +<---------
*   :    Masking-key (cont.)        |         Payload Data          :
*   +---------------------------------------------------------------+ AREA TO
*   :                          Payload data                         : MASK
*   +---------------------------------------------------------------+<---------
*/
/*WEBSOCKET FRAME CONTROL BYTE MASK*/
#define FIN_MASK                         0x80
#define RSV1_MASK                        0x40
#define RSV2_MASK                        0x20
#define RSV3_MASK                        0x10
#define OPCODE_MASK                      0x0F
/*WEBSOCKET FRAME PAYLOAD BYTE MASK*/
#define PAYLOAD_MASK                     0x80
#define PAYLOAD_LENGTH_1_BYTE_MASK       0x7F

#define RSV_ERR_SHUTDOWN_MESSAGE              "rsv bits/extension negotiation is not supported"
#define CLOSE_REQUEST_SHUTDOWN_MESSAGE        "close requested from Server"
#define LENGTH_SHUTDOWN_MESSAGE               "max Frame length supported is 1024"
#define PAYLOAD_TYPE_ERROR_SHUTDOWN_MESSAGE   "unsupported payload type"

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef enum
{
    READ_CONTROL_BYTES,
    READ_LENGTH,
    READ_PAYLOAD,
    READ_COMPLETED_SUCCESSFULLY,
    READ_FRAME_ERROR
} wiced_websocket_read_frame_state_t;

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_websocket_t* application_websocket[MAX_SOCKETS]; /* the application websocket interface */
    int                index;
} wiced_websocket_manager_t;


/******************************************************
 *               Static Function Declarations
 ******************************************************/
static wiced_result_t wiced_websocket_common_connect   ( wiced_websocket_t* websocket, wiced_websocket_handshake_fields_t* websocket_header_fields, wiced_bool_t use_secure_connection);
static wiced_result_t wiced_websocket_connect          ( wiced_websocket_t* websocket,wiced_ip_address_t* address );
static wiced_result_t wiced_websocket_tls_connect      ( wiced_websocket_t* websocket,wiced_ip_address_t* address );
static wiced_result_t wiced_mask_unmask_frame_data     ( uint8_t* data_in, uint8_t* data_out, uint16_t data_length, uint8_t* mask );
static uint8_t        wiced_get_websocket_index        ( wiced_websocket_t* websocket );
static wiced_result_t wiced_update_socket_state        ( wiced_websocket_t* websocket, wiced_websocket_state_t state);
static wiced_result_t wiced_register_websocket         ( wiced_websocket_t* websocket );
static wiced_result_t wiced_deregister_websocket       ( wiced_websocket_t* websocket );

static wiced_result_t wiced_on_close_callback   ( void* websocket );
static wiced_result_t wiced_on_message_callback ( void* websocket );

/******************************************************
 *               Variable Definitions
 ******************************************************/
static wiced_tls_simple_context_t              websocket_tls_context;
static wiced_mutex_t                           handshake_lock;
static wiced_bool_t                            mutex_initialised  = WICED_FALSE;
static wiced_websocket_manager_t               websocket_manager  = { .index = 0 };
/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_ws_connect(wiced_websocket_t* websocket, wiced_websocket_handshake_fields_t* websocket_header)
{

    if ( wiced_websocket_common_connect( websocket, websocket_header, WICED_FALSE ) != WICED_SUCCESS )
    {
        wiced_websocket_close( websocket, NULL );
        return WICED_ERROR;
    }

    return WICED_SUCCESS;

}

wiced_result_t wiced_wss_connect(wiced_websocket_t* websocket, wiced_websocket_handshake_fields_t* websocket_header)
{

    if( wiced_websocket_common_connect( websocket, websocket_header, WICED_TRUE ) != WICED_SUCCESS )
    {
        wiced_websocket_close( websocket, NULL );
        return WICED_ERROR;
    }

    return WICED_SUCCESS;

}

static wiced_result_t wiced_websocket_common_connect(wiced_websocket_t* websocket, wiced_websocket_handshake_fields_t* websocket_header_fields, wiced_bool_t use_secure_connection)
{
    wiced_result_t      result = WICED_ERROR;
    wiced_ip_address_t  address;


    if( websocket != NULL )
    {

        if ( !mutex_initialised )
        {
            /*create handhsake mutex. There can only be one connection in a connecting state*/
            wiced_rtos_init_mutex( &handshake_lock );
            mutex_initialised = WICED_TRUE;
        }

        /*as per rfc 6455 specification, there can not be more then one socket in a connecting state*/
        wiced_rtos_lock_mutex( &handshake_lock );

        /*perform DNS lookup*/
        result = wiced_hostname_lookup( websocket_header_fields->host, &address, 10000 );
        IF_ERROR_RECORD_AND_EXIT( result, WEBSOCKET_DNS_RESOLVE_ERROR );

        /*register websocket*/
        result = wiced_register_websocket( websocket );
        IF_ERROR_RECORD_AND_EXIT( result, WEBSOCKET_NO_AVAILABLE_SOCKET );

        wiced_update_socket_state( websocket,WEBSOCKET_CONNECTING );

        /*establish secure/non secure tcp connection to server*/
        if (use_secure_connection)
        {
            result = wiced_websocket_tls_connect( websocket, &address );
        }
        else
        {
            result = wiced_websocket_connect( websocket, &address );
        }
        IF_ERROR_RECORD_AND_EXIT( result, WEBSOCKET_CLIENT_CONNECT_ERROR );

        result = wiced_establish_websocket_handshake(websocket, websocket_header_fields);
        IF_ERROR_RECORD_AND_EXIT( result, WEBSOCKET_SERVER_HANDSHAKE_RESPONSE_INVALID );

        /* If a sub protocol was requested, extract it from the handshake */
        if( websocket_header_fields->sec_websocket_protocol != NULL )
        {
            result = wiced_get_websocket_subprotocol( websocket->subprotocol );
            IF_ERROR_RECORD_AND_EXIT( result, WEBSOCKET_SUBPROTOCOL_NOT_SUPPORTED );
        }

        wiced_tcp_register_callbacks( &websocket->socket, NULL, wiced_on_message_callback, wiced_on_close_callback );

        wiced_update_socket_state( websocket,WEBSOCKET_OPEN );
        websocket->on_open( websocket );

        /*release handhsake mutex. There can only be one connection in a connecting state*/
        wiced_rtos_unlock_mutex(&handshake_lock);

    }

    return result;

}


static wiced_result_t wiced_websocket_tls_connect( wiced_websocket_t* websocket,wiced_ip_address_t* address )
{
    wiced_result_t result = WICED_ERROR;

    wiced_tls_init_simple_context( &websocket_tls_context, NULL );

    WICED_VERIFY( wiced_tcp_create_socket( &websocket->socket, WICED_STA_INTERFACE ) );

    wiced_tcp_register_callbacks( &websocket->socket, websocket->on_open, NULL, websocket->on_close );

    wiced_tcp_enable_tls( &websocket->socket, &websocket_tls_context );

    wiced_tcp_bind( &websocket->socket, 443 );

    result = wiced_tcp_connect( &websocket->socket, address, 443, 10000 );
    if ( result != WICED_SUCCESS )
    {
        websocket->error_type=WEBSOCKET_CLIENT_CONNECT_ERROR;
        websocket->on_error( websocket );
        wiced_tcp_delete_socket( &websocket->socket );
        return ( result );
    }

    return ( result );
}

static wiced_result_t wiced_websocket_connect( wiced_websocket_t* websocket,wiced_ip_address_t* address )
{
    wiced_result_t     result = WICED_ERROR;

    WICED_VERIFY( wiced_tcp_create_socket( &websocket->socket, WICED_STA_INTERFACE ) );

    wiced_tcp_register_callbacks( &websocket->socket, websocket->on_open, NULL, websocket->on_close );

    wiced_tcp_bind( &websocket->socket, 80 );

    result = wiced_tcp_connect( &websocket->socket, address, 80, 10000 );
    if ( result != WICED_SUCCESS )
    {
        websocket->error_type=WEBSOCKET_CLIENT_CONNECT_ERROR;
        websocket->on_error(websocket);
        wiced_tcp_delete_socket( &websocket->socket );
        return ( result );
    }

    return ( result );
}

wiced_result_t wiced_websocket_send( wiced_websocket_t* websocket, wiced_websocket_frame_t* tx_frame)
{
    wiced_result_t result = WICED_ERROR;
    uint16_t       random_number;
    uint8_t        websocket_frame[MAX_FRAME_BYTES];
    uint8_t        additional_bytes_to_represent_length;
    uint8_t        masking_key_byte_offset;

    /* Initialise frame */
    memset( websocket_frame, 0x00, MAX_FRAME_BYTES );

    /*Set FIN, RSV and opcode fields of Byte 0*/
    websocket_frame[0] = (uint8_t)( FIN_MASK | ( (int)tx_frame->payload_type & OPCODE_MASK ) );

    /*Before setting payload, need to find out if extended bits of frame are required*/
    if ( tx_frame->payload_length < 126 )
    {
        /*In this case, payload length is represented with 1 byte, and bytes extended payload is not used*/
        websocket_frame[1] = (uint8_t)(PAYLOAD_MASK | tx_frame->payload_length);

        additional_bytes_to_represent_length = 0;
    }
    else
    {   /*******************************************************************************************************
         * Check we can buffer the data for sending (needed due to masking). Also ensure its not a control frame,
         * as they have a maximum size limit of 125
         * ******************************************************************************************************/
        if ( ( tx_frame->payload_length <= MAX_PAYLOAD )|( tx_frame->payload_type < WEBSOCKET_CONNECTION_CLOSE ) )
        {
            websocket_frame[1] = (uint8_t)(PAYLOAD_MASK | 126);
            websocket_frame[2] = (uint8_t)(tx_frame->payload_length >> 8);
            websocket_frame[3] = (uint8_t)(tx_frame->payload_length);
            additional_bytes_to_represent_length = 2;
        }
        else /* payload > MAX_PAYLOAD */
        {
            /*unsupported frame size*/
            return WICED_UNSUPPORTED;
        }
    }
    /*Note: masking key starts after the payload. Two bytes of the control info of frame also need to be considered/*/
    masking_key_byte_offset  = (uint8_t)(additional_bytes_to_represent_length + CONTROL_FIELD_BYTES);

    /* Generate random mask to be used in masking the frame data*/
    wwd_wifi_get_random( &websocket_frame[ masking_key_byte_offset ], 4 );

    /* use algorithm of RFC6455, 5.3 to mask the payload only, not the header*/
    wiced_mask_unmask_frame_data(tx_frame->payload,&websocket_frame[masking_key_byte_offset+MASKING_KEY_BYTES],tx_frame->payload_length,&websocket_frame[masking_key_byte_offset]);

    /* Send frame to server */
    result = wiced_tcp_send_buffer( &websocket->socket, websocket_frame, (uint16_t) ( tx_frame->payload_length + masking_key_byte_offset + MASKING_KEY_BYTES ) );
    IF_ERROR_RECORD_AND_EXIT( result, WEBSOCKET_FRAME_SEND_ERROR );

    return ( result );
}

wiced_result_t wiced_websocket_receive( wiced_websocket_t* websocket, wiced_websocket_frame_t* rx_frame)
{
    wiced_result_t                      result                  = WICED_SUCCESS;
    uint16_t                            payload_length          = 0;
    wiced_websocket_read_frame_state_t  read_state              = READ_CONTROL_BYTES;
    uint32_t                            bytes_read              = 0;
    const char*                         connection_close_string = NULL;
    uint16_t                            total_received_bytes    = 0;
    uint8_t                             frame_control_bytes[2];
    uint8_t                             frame_length_bytes[2];
    wiced_websocket_frame_t             respond_frame;
    wiced_packet_t*                     tcp_reply_packet;
    uint16_t                            tcp_data_available;
    uint8_t*                            received_data;


    memset( frame_control_bytes, 0x0, sizeof( frame_control_bytes ) );
    memset( frame_length_bytes,  0x0, sizeof( frame_length_bytes ) );


    /************************************************
     * Unpack websocket frame
     ************************************************/
    while ( read_state != ( READ_COMPLETED_SUCCESSFULLY ) || ( read_state == READ_FRAME_ERROR ) )
    {
        result = wiced_tcp_receive( &websocket->socket, &tcp_reply_packet, WICED_WAIT_FOREVER );
        wiced_packet_get_data( tcp_reply_packet, 0, (uint8_t**)&received_data, &total_received_bytes, &tcp_data_available );

        if (result != WICED_SUCCESS)
        {
            read_state = READ_FRAME_ERROR;
            break;
        }

        switch(read_state)
        {
            case READ_CONTROL_BYTES:
                rx_frame->final_frame = ( received_data[0] & FIN_MASK )?WICED_TRUE:WICED_FALSE;

                /*read RSV bits. Note RSV and extension negotiations are not supported*/
                if( received_data[0] & (RSV1_MASK | RSV2_MASK | RSV3_MASK) )
                {
                    connection_close_string = RSV_ERR_SHUTDOWN_MESSAGE;
                    read_state = READ_FRAME_ERROR;
                    break;
                }

                /*Read the payload type and pass to the application. Note any ping/pong or close action if required*/
                rx_frame->payload_type = (wiced_websocket_payload_type_t)(received_data[0] & OPCODE_MASK);
                if( ( (rx_frame->payload_type >= WEBSOCKET_RESERVED_3) && (rx_frame->payload_type <= WEBSOCKET_RESERVED_7) ) || \
                    ( (rx_frame->payload_type >= WEBSOCKET_RESERVED_B) ) )
                {
                    connection_close_string = PAYLOAD_TYPE_ERROR_SHUTDOWN_MESSAGE;
                    read_state = READ_FRAME_ERROR;
                    break;
                }
                read_state++;
                bytes_read++;
            case READ_LENGTH:
                /*Extract the length of the payload*/
                payload_length = received_data[1] & PAYLOAD_LENGTH_1_BYTE_MASK;
                /*This indicates bytes 2 and 3 of frame are used to store actual payload length*/
                if( payload_length == 126 )
                {
                    /*Extract Frame bytes 2 and 3*/
                    if (total_received_bytes < ( CONTROL_FIELD_BYTES  + MAX_PAYLOAD_FIELD_BYTES ) )
                    {
                        /*break out of switch and wait for get more data*/
                        break;
                    }
                    payload_length = (uint16_t)( received_data[3] | (received_data[2] << 8) );
                    if ( payload_length > MAX_PAYLOAD )
                    {
                        connection_close_string = LENGTH_SHUTDOWN_MESSAGE;
                        read_state = READ_FRAME_ERROR;
                        break;
                    }
                    bytes_read+=3;
                }
                else if( payload_length == 127 )
                {
                    /*unsupported payload length*/
                    connection_close_string = LENGTH_SHUTDOWN_MESSAGE;
                    read_state = READ_FRAME_ERROR;
                    break;
                }
                else
                {
                    /*payload length already extracted in second byte*/
                    bytes_read++;
                }
                read_state++;
            case READ_PAYLOAD:
                /*Check if payload needs to be unmasked, and maksing key extracted along with data*/
                if ( received_data[1] & PAYLOAD_MASK )
                {
                    if ( ( total_received_bytes-bytes_read ) < (uint16_t)(MASKING_KEY_BYTES + payload_length) )
                    {
                        /*break out of switch and wait for get more data*/
                        break;
                    }
                    wiced_mask_unmask_frame_data(&received_data[bytes_read + MASKING_KEY_BYTES], rx_frame->payload, payload_length, &received_data[bytes_read]);
                }
                else if (total_received_bytes-bytes_read < (uint16_t)payload_length )
                {
                    /*break out of switch and wait for get more data*/
                    break;
                }
                else
                {
                    memcpy( rx_frame->payload, &received_data[bytes_read], payload_length );
                }
                read_state++;
            case READ_COMPLETED_SUCCESSFULLY:
                 break;
            case READ_FRAME_ERROR:
                /* fall through */
            default:
                read_state = READ_FRAME_ERROR;
                break;
        }
    }

    /*Check if unpack failed, close connection*/
    if( read_state == READ_FRAME_ERROR )
    {
        wiced_websocket_close(websocket,connection_close_string);
        result =  WICED_ERROR;
    }
    else if( rx_frame->payload_type == WEBSOCKET_PING )
    {
        /*pong back with the data received*/
        respond_frame.final_frame    = WICED_TRUE;
        respond_frame.payload_type   = WEBSOCKET_PONG;
        respond_frame.payload_length = payload_length;
        respond_frame.payload        = rx_frame->payload;
        wiced_websocket_send(websocket, &respond_frame);
    }
    else if ( rx_frame->payload_type == WEBSOCKET_CONNECTION_CLOSE)
    {
        /*Did I send a connection close string*/
        respond_frame.final_frame    = WICED_TRUE;
        respond_frame.payload_type   = WEBSOCKET_CONNECTION_CLOSE;
        respond_frame.payload_length = payload_length;
        respond_frame.payload        = (char*)CLOSE_REQUEST_SHUTDOWN_MESSAGE;
        wiced_websocket_send(websocket, &respond_frame);
        wiced_websocket_close(websocket,NULL);
    }
    else
    {
        /*closing else*/
    }

    return result;
}

wiced_result_t wiced_websocket_close( wiced_websocket_t* websocket, const char* optional_close_message )
{
    wiced_result_t          result = WICED_SUCCESS;
    wiced_websocket_frame_t frame;


    if ( websocket->state != WEBSOCKET_CLOSED )
    {

        wiced_update_socket_state( websocket, WEBSOCKET_CLOSING );

        if ( optional_close_message != NULL )
        {
            frame.payload        = (char*)optional_close_message;
            frame.payload_length = (uint16_t)strlen(optional_close_message);
            frame.payload_type   = WEBSOCKET_CONNECTION_CLOSE;
            wiced_websocket_send( websocket, &frame );
        }

        wiced_rtos_unlock_mutex(&handshake_lock);

        wiced_tcp_disconnect( &websocket->socket );
        if(&websocket->socket!=NULL)
        {
            result = wiced_tcp_delete_socket( &websocket->socket );
        }

        wiced_update_socket_state( websocket, WEBSOCKET_CLOSED );

        wiced_deregister_websocket ( websocket );
    }

    return result;
}

static wiced_result_t wiced_mask_unmask_frame_data(uint8_t* data_in, uint8_t* data_out, uint16_t data_length, uint8_t* mask)
{
    wiced_result_t     result = WICED_SUCCESS;
    uint32_t i;

    for(i=0; i<data_length;i++)
    {
        data_out[i]=data_in[i] ^ mask[i%4];
    }

    return result;
}

static wiced_result_t wiced_register_websocket( wiced_websocket_t* websocket )
{
    wiced_result_t result    = WICED_SUCCESS;

    if ( ( websocket_manager.application_websocket[websocket_manager.index] == NULL ) && (websocket_manager.index < MAX_SOCKETS) )
    {
        websocket_manager.application_websocket[websocket_manager.index] = websocket;
        websocket_manager.index++;
    }
    else
    {
        result = WICED_ERROR;
    }

    return result;
}

static wiced_result_t wiced_deregister_websocket( wiced_websocket_t* websocket )
{
    uint8_t index;
    index= wiced_get_websocket_index( websocket );

    websocket_manager.application_websocket[index] = NULL;
    if ( websocket_manager.index > 0 )
    {
        websocket_manager.index--;
    }

    return WICED_SUCCESS;
}

static wiced_result_t wiced_update_socket_state( wiced_websocket_t* websocket, wiced_websocket_state_t state)
{

    uint8_t websocket_index = wiced_get_websocket_index( websocket );
    if ( websocket_index >= MAX_SOCKETS)
    {
        return WICED_DOES_NOT_EXIST;
    }
    else
    {
        websocket->state = state;
        return WICED_SUCCESS;
    }

}

static uint8_t wiced_get_websocket_index( wiced_websocket_t* websocket )
{
    uint8_t websocket_index = MAX_SOCKETS;
    uint8_t i;

    for ( i=0; i< MAX_SOCKETS; i++ )
    {
        if ( websocket_manager.application_websocket[i] == websocket )
        {
            websocket_index = i;
        }
    }
    return websocket_index;
}



static wiced_result_t wiced_on_close_callback( void* websocket)
{
    wiced_websocket_t* websocket_ptr = websocket;

    websocket_ptr->on_close( websocket );

    wiced_deregister_websocket( websocket );

    return WICED_SUCCESS;
}

static wiced_result_t wiced_on_message_callback( void* websocket)
{
    wiced_websocket_t* websocket_ptr = websocket;

    websocket_ptr->on_message( websocket );

    return WICED_SUCCESS;
}
