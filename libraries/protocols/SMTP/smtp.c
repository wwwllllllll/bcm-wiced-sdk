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
#include "time.h"
#include "smtp.h"
#include "wiced_rtos.h"
#include "wwd_debug.h"
#include <string.h>
#include <wiced_utilities.h>
#include "base64.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define VERIFY( expected, function ) \
    do \
    { \
        uint32_t retval; \
        retval = (uint32_t)(function); \
        if ( (uint32_t)(expected) != retval ) \
        { \
            WPRINT_LIB_ERROR(("Unexpected return %d in email.c on line %d\n\r", (int)retval, __LINE__));  \
            result = WICED_ERROR; \
            goto exit; \
        } \
    } while(0)

#define INIT_BUFFER_AND_LENGTH( buf, len ) \
    do \
    { \
        (len) = sizeof( buf ); \
        memset( buf, 0, (size_t) (len) ); \
    } while(0)

#define GET_CONST_BUF_LENGTH(buf)      (sizeof(buf) - 1)

/******************************************************
 *                    Constants
 ******************************************************/

#define CONTENT_TABLE_SIZE             (27)

/* SMTP constants */
#define STANDARD_SMTP_PORT             (25)
#define STANDARD_SECURE_SMTP_PORT      (465)

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    /* Successful outcomes */
    SMTP_SERVICE_READY                      = 220,
    SMTP_AUTH_SUCCESSFUL                    = 235,
    SMTP_ACTION_COMPLETED                   = 250,
    SMTP_MORE_INFO                          = 334,
    SMTP_START_MAIL_INPUT                   = 354,
    SMTP_OCTET_OFFSET_IS_TRANSACTION_OFFSET = 355,
    SMTP_DOMAIN_SERVICE_UNAVAILABLE         = 421,
    SMTP_PASSWORD_TRANSITION_NEEDED         = 432,
    SMTP_ATRN_REQUEST_REFUSED               = 450,
    SMTP_UNABLE_TO_PROCESS_ATRN_REQUEST     = 451,
    SMTP_INSUFFICIENT_SYSTEM_STORAGE        = 452,
    SMTP_NO_MAIL                            = 453,
    SMTP_TLS_TEMPORARILY_UNAVAILABLE        = 454,
    SMTP_UNABLE_TO_QUEUE_MESSAGES           = 458,
    SMTP_NODE_NOT_ALLOWED                   = 459,
    SMTP_COMMAND_NOT_RECOGNIZED             = 500,
    SMTP_PARAMETERS_NOT_ALLOWED             = 501,
    SMTP_COMMAND_NOT_IMPLEMENTED            = 502,
    SMTP_BAD_COMMAND_SEQUENCE               = 503,
    SMTP_PARAMETER_NOT_IMPLEMENTED          = 504,
    SMTP_MACHINE_DOESNT_ACCEPT_MAIL         = 521,
    SMTP_AUTHENTICATION_TOO_WEAK            = 534,
    SMTP_ENCRYPTION_REQUIRED                = 538,
    SMTP_MAILBOX_UNAVAILABLE                = 550,
    SMTP_USER_NOT_LOCAL                     = 551,
    SMTP_STORAGE_ALLOCATION_EXCEEDED        = 552,
    SMTP_MAILBOX_NAME_NOT_ALLOWED           = 553,
    SMTP_TRANSACTION_FAILED                 = 554,
} smtp_reply_code_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    uint8_t* content;
    uint32_t length;
} email_content_table_field_t;

struct wiced_email_account_internal
{
    wiced_tcp_socket_t smtp_socket;    /* Socket for SMTP        */
    wiced_ip_address_t smtp_server_ip; /* SMTP server IP address */
};

typedef struct wiced_email_account_internal wiced_email_account_internal_t;


/******************************************************
 *               Static Function Declarations
 ******************************************************/

static wiced_result_t send_smtp_command           ( wiced_tcp_socket_t *socket, const char *command, uint8_t command_length, const char* params, uint16_t params_length );
static wiced_result_t receive_smtp_reply          ( wiced_tcp_socket_t *socket, wiced_packet_t **reply_packet, char **reply_string, uint16_t *reply_length );
static uint16_t       get_smtp_reply_code         ( wiced_tcp_socket_t *socket, wiced_packet_t *reply_packet );
static wiced_result_t send_smtp_sender            ( wiced_tcp_socket_t *socket, const char *address, uint8_t length );
static wiced_result_t send_smtp_recipient         ( wiced_tcp_socket_t *socket, const char *address, uint8_t length );
static uint8_t        get_first_address_from_list ( char **address_list, char **first_address, uint8_t *first_address_length );
static void           init_content_table          ( const wiced_email_account_t *account, const wiced_email_t *email, const char date[32], email_content_table_field_t *content_table );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const char const localhost[]                             = "WICED";
static const char const smtp_ehlo[]                             = "EHLO";
static const char const smtp_starttls[]                         = "STARTTLS";
static const char const smtp_auth_login[]                       = "AUTH LOGIN";
static const char const smtp_username_prompt[]                  = "Username:";
static const char const smtp_password_prompt[]                  = "Password:";
static const char const smtp_data[]                             = "DATA";
static const char const smtp_quit[]                             = "QUIT";
static const char const smtp_mail_from[]                        = "MAIL FROM: <";
static const char const smtp_rcpt_to[]                          = "RCPT TO: <";
static const char const smtp_return_newline[]                   = "\r\n";
static const char const smtp_end_of_rcpt_to[]                   = ">\r\n";
static const char const smtp_end_of_data[]                      = "\r\n.\r\n";
static const char const smtp_space[]                            = " ";
static const char const email_header_msg_id[]                   = "Message-ID: <>";
static const char const email_header_date[]                     = "Date: ";
static const char const email_header_from[]                     = "From: ";
static const char const email_header_mime_version[]             = "MIME-Version: 1.0";
static const char const email_header_to[]                       = "To: ";
static const char const email_header_cc[]                       = "Cc: ";
static const char const email_header_subject[]                  = "Subject: ";
static const char const email_header_content_type[]             = "Content-Type: text/plain; charset=\"utf-8\"";
static const char const email_header_content_tranfer_encoding[] = "Content-Transfer-Encoding: 8bit";

static const char const* month_abbreviations[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

static const char const* weekday_abbreviations[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_smtp_account_init( wiced_email_account_t* account )
{
    uint32_t smtp_server_ipv4;
    uint8_t count = 0;

    account->internal = (wiced_email_account_internal_t*) MALLOC_OBJECT("email_acc_int", wiced_email_account_internal_t);

    if ( account->internal == NULL )
    {
        return WICED_ERROR;
    }

    memset( account->internal, 0, sizeof( *( account->internal ) ) );

    WPRINT_LIB_INFO(( "Querying DNS server for %s\n\r", account->smtp_server ));

    while ( wiced_hostname_lookup( account->smtp_server, &account->internal->smtp_server_ip, 5000 ) != WICED_SUCCESS && count < 10 )
    {
        count++;
        WPRINT_LIB_INFO(( "Failed to get IP for %s ... retrying \n\r", account->smtp_server ));
    }

    if ( count >= 10 )
    {
        WPRINT_LIB_DEBUG(( "Reached maximum number of retries\n\r" ));
        return WICED_ERROR;
    }

    smtp_server_ipv4 = GET_IPV4_ADDRESS( account->internal->smtp_server_ip );
    WPRINT_LIB_INFO( ( "Successfully retrieved SMTP server IP : %u.%u.%u.%u\n",
                 (unsigned char) ( ( smtp_server_ipv4 >> 24 ) & 0xff ),
                 (unsigned char) ( ( smtp_server_ipv4 >> 16 ) & 0xff ),
                 (unsigned char) ( ( smtp_server_ipv4 >> 8  ) & 0xff ),
                 (unsigned char) ( ( smtp_server_ipv4 >> 0  ) & 0xff ) ) );
    return WICED_SUCCESS;
}

wiced_result_t wiced_smtp_account_deinit( wiced_email_account_t* account )
{
    free( account->internal );
    account->internal = 0;
    return WICED_SUCCESS;
}

wiced_result_t wiced_smtp_send( wiced_email_account_t* account, const wiced_email_t* email )
{
    wiced_result_t              result = WICED_SUCCESS;
    wiced_packet_t*             packet = NULL;
    email_content_table_field_t content_table[CONTENT_TABLE_SIZE];
    wiced_tls_simple_context_t* tls_context;
    struct tm*                  timeinfo;
    wiced_utc_time_t            utc_time;
    char*                       reply;
    char*                       address_list;
    char*                       address;
    uint8_t                     address_length;
    uint8_t*                    data_ptr;
    uint16_t                    length;
    uint16_t                    available_space;
    uint32_t                    field_index;
    uint32_t                    content_index;
    char                        date[32]; /* Date string in email header format */

    memset( content_table, 0, sizeof( content_table ) );
    memset( &account->internal->smtp_socket, 0, sizeof( account->internal->smtp_socket ) );


    /* allocate memory for internal structure */
    memset( date, 0, sizeof( date ) );

    wiced_time_get_utc_time( &utc_time );

    /* Set email's date. UTC time is  */
    timeinfo = gmtime( (const time_t*)&utc_time );

    snprintf( date,
              sizeof( date ),
              "%s, %.02d %s %.04d %.02d:%.02d:%.02d +0000",
              weekday_abbreviations[timeinfo->tm_wday],
              timeinfo->tm_mday,
              month_abbreviations[timeinfo->tm_mon],
              timeinfo->tm_year + 1900,
              timeinfo->tm_hour,
              timeinfo->tm_min,
              timeinfo->tm_sec );

    /* Connect to SMTP server */
    result = wiced_tcp_create_socket( &account->internal->smtp_socket, WICED_STA_INTERFACE );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    result = wiced_tcp_connect( &account->internal->smtp_socket, &account->internal->smtp_server_ip, account->smtp_server_port, 20000 );
    if ( result != WICED_SUCCESS )
    {
        goto exit_without_connection;
    }
    VERIFY( WICED_SUCCESS, receive_smtp_reply( &account->internal->smtp_socket, &packet, &reply, &length ) );
    VERIFY( SMTP_SERVICE_READY, get_smtp_reply_code(&account->internal->smtp_socket, packet ) );
    wiced_packet_delete( packet );

    /* Send EHLO */
    VERIFY(WICED_SUCCESS, send_smtp_command(&account->internal->smtp_socket, smtp_ehlo, GET_CONST_BUF_LENGTH(smtp_ehlo), localhost, GET_CONST_BUF_LENGTH(localhost)));
    VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
    VERIFY(SMTP_ACTION_COMPLETED, get_smtp_reply_code(&account->internal->smtp_socket, packet));

    /* Check if encryption required */
    if ( account->smtp_encryption == WICED_EMAIL_ENCRYPTION_TLS && strstr( reply, smtp_starttls )  )
    {
        wiced_packet_delete( packet );

        /* Send STARTTLS */
        VERIFY(WICED_SUCCESS, send_smtp_command(&account->internal->smtp_socket, smtp_starttls, GET_CONST_BUF_LENGTH(smtp_starttls), NULL, 0));
        VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
        VERIFY(SMTP_SERVICE_READY, get_smtp_reply_code(&account->internal->smtp_socket, packet));
        wiced_packet_delete( packet );

        /* Establish TLS handshake here. Every packet from here onwards is encrypted */
        tls_context = MALLOC_OBJECT("email tls ctx", wiced_tls_simple_context_t);
        memset( tls_context, 0, sizeof( *tls_context ) );
        account->internal->smtp_socket.context_malloced = WICED_TRUE;

        if ( wiced_tcp_enable_tls( &account->internal->smtp_socket, tls_context ) != WICED_SUCCESS )
        {
            result = WICED_ERROR;
            WPRINT_LIB_ERROR(( "Failed to enable TLS\n" ));
            goto exit_without_sending_quit_command;
        }

        result = wiced_tcp_start_tls( &account->internal->smtp_socket, WICED_TLS_AS_CLIENT, WICED_TLS_DEFAULT_VERIFICATION );
        if ( result != WICED_SUCCESS )
        {
            WPRINT_LIB_ERROR(( "Failed to start TLS\n" ));
            goto exit_without_sending_quit_command;
        }

        /* Re-send EHLO, but already encrypted */
        VERIFY(WICED_SUCCESS, send_smtp_command(&account->internal->smtp_socket, smtp_ehlo, GET_CONST_BUF_LENGTH(smtp_ehlo), localhost, GET_CONST_BUF_LENGTH(localhost)));
        VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
        VERIFY(SMTP_ACTION_COMPLETED, get_smtp_reply_code(&account->internal->smtp_socket, packet));
    }

    /* Check if authentication is required */
    if ( strstr( reply, smtp_auth_login ) )
    {
        uint32_t buffer_length;
        char buffer[50];

        wiced_packet_delete( packet );

        /* Send AUTH LOGIN */
        VERIFY(WICED_SUCCESS, send_smtp_command(&account->internal->smtp_socket, smtp_auth_login, GET_CONST_BUF_LENGTH(smtp_auth_login), NULL, 0));
        VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
        VERIFY(SMTP_MORE_INFO, get_smtp_reply_code(&account->internal->smtp_socket, packet));

        /* Decode SMTP server reply using base64 algorithm */
        INIT_BUFFER_AND_LENGTH(buffer, buffer_length);
        base64_decode( (unsigned char*)reply, length, (unsigned char*)buffer, buffer_length, BASE64_DEC_STANDARD );
        wiced_packet_delete( packet );

        /* Check if decoded string is username challenge */
        if ( strstr( buffer, smtp_username_prompt ) )
        {
            /* Encode username */
            INIT_BUFFER_AND_LENGTH(buffer, buffer_length);
            base64_encode( (unsigned char*)account->user_name, (uint32_t) strlen( account->user_name ), (unsigned char*)buffer, buffer_length, BASE64_DEC_STANDARD );

            VERIFY(WICED_SUCCESS, send_smtp_command(&account->internal->smtp_socket, buffer, (uint8_t) buffer_length, NULL, 0));
            VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
            VERIFY(SMTP_MORE_INFO, get_smtp_reply_code(&account->internal->smtp_socket, packet));

            /* Decode SMTP server reply using base64 algorithm */
            INIT_BUFFER_AND_LENGTH(buffer, buffer_length);
            base64_decode( (unsigned char*)reply, length, (unsigned char*)buffer, buffer_length, BASE64_DEC_STANDARD );
            wiced_packet_delete( packet );

            /* Check if decoded string is password challenge */
            if ( strstr( buffer, smtp_password_prompt ) )
            {
                /* Encode and send password back to SMTP server */
                INIT_BUFFER_AND_LENGTH(buffer, buffer_length);
                base64_encode( (unsigned char*)account->password, (uint32_t) strlen( account->password ), (unsigned char*)buffer, buffer_length, BASE64_DEC_STANDARD );

                VERIFY(WICED_SUCCESS, send_smtp_command(&account->internal->smtp_socket, buffer, (uint8_t) buffer_length, NULL, 0));
                VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
                VERIFY(SMTP_AUTH_SUCCESSFUL, get_smtp_reply_code(&account->internal->smtp_socket, packet));
                wiced_packet_delete( packet );
            }
        }
    }
    else
    {
        wiced_packet_delete( packet );
    }

    /* Send sender information to the server (MAIL FROM command) */
    VERIFY(WICED_SUCCESS, send_smtp_sender(&account->internal->smtp_socket, account->email_address, (uint8_t) strlen(account->email_address)));
    VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
    VERIFY(SMTP_ACTION_COMPLETED, get_smtp_reply_code(&account->internal->smtp_socket, packet));;
    wiced_packet_delete( packet );

    /* Send recipient information to the server (RCPT TO command) */
    address_list = email->to_addresses;
    while ( get_first_address_from_list( &address_list, &address, &address_length ) )
    {
        send_smtp_recipient( &account->internal->smtp_socket, address, address_length );
        VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
        VERIFY(SMTP_ACTION_COMPLETED, get_smtp_reply_code(&account->internal->smtp_socket, packet));
        wiced_packet_delete( packet );
    }

    /* Send recipient information from the cc list to the server (RCPT TO command) */
    if ( email->cc_addresses )
    {
        address_list = email->cc_addresses;
        while ( get_first_address_from_list( &address_list, &address, &address_length ) )
        {
            send_smtp_recipient( &account->internal->smtp_socket, address, address_length );
            VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
            VERIFY(SMTP_ACTION_COMPLETED, get_smtp_reply_code(&account->internal->smtp_socket, packet));
            wiced_packet_delete( packet );
        }
    }

    /* Send recipient information from the bcc list to the server (RCPT TO command) */
    if ( email->bcc_addresses )
    {
        address_list = email->bcc_addresses;
        while ( get_first_address_from_list( &address_list, &address, &address_length ) )
        {
            send_smtp_recipient( &account->internal->smtp_socket, address, address_length );
            VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
            VERIFY(SMTP_ACTION_COMPLETED, get_smtp_reply_code(&account->internal->smtp_socket, packet));
            wiced_packet_delete( packet );
        }
    }

    /* Send DATA */
    VERIFY(WICED_SUCCESS, send_smtp_command(&account->internal->smtp_socket, smtp_data, GET_CONST_BUF_LENGTH(smtp_data), NULL, 0));
    VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
    VERIFY(SMTP_START_MAIL_INPUT, get_smtp_reply_code(&account->internal->smtp_socket, packet));
    wiced_packet_delete( packet );

    /* Fill packets with email content and send */
    init_content_table(account, email, date, content_table);

    VERIFY(WICED_SUCCESS, wiced_packet_create_tcp(&account->internal->smtp_socket, 0, &packet, &data_ptr, &available_space));

    for ( field_index = 0; field_index < CONTENT_TABLE_SIZE; field_index++ )
    {
        for ( content_index = 0; content_index < content_table[field_index].length; content_index++ )
        {
            /* Fill content */
            *data_ptr = content_table[field_index].content[content_index];
            available_space--;
            data_ptr++;

            /* Check if last bytes */
            if ( ( field_index   == (unsigned)( CONTENT_TABLE_SIZE - 1 ) ) &&
                 ( content_index == (unsigned)( content_table[field_index].length - 1 ) ) )
            {
                /* Send packet. */
                wiced_packet_set_data_end( (wiced_packet_t*) packet, (uint8_t*) data_ptr );
                VERIFY(WICED_SUCCESS, wiced_tcp_send_packet(&account->internal->smtp_socket, packet));
            }
            /* Check if payload if full */
            else if ( available_space == 0 )
            {
                /* Payload full. Send packet. */
                wiced_packet_set_data_end( (wiced_packet_t*) packet, (uint8_t*) data_ptr );
                VERIFY(WICED_SUCCESS, wiced_tcp_send_packet(&account->internal->smtp_socket, packet));

                /* Allocate another packet and continue filling payload with email content. */
                VERIFY(WICED_SUCCESS, wiced_packet_create_tcp(&account->internal->smtp_socket, 0, &packet, &data_ptr, &available_space));
            }
        }
    }

    VERIFY(WICED_SUCCESS, receive_smtp_reply(&account->internal->smtp_socket, &packet, &reply, &length));
    VERIFY(SMTP_ACTION_COMPLETED, get_smtp_reply_code(&account->internal->smtp_socket, packet));

    exit:
    wiced_packet_delete( packet );

    /* Send QUIT command and disconnect */
    send_smtp_command( &account->internal->smtp_socket, smtp_quit, GET_CONST_BUF_LENGTH(smtp_quit), NULL, 0 );
    receive_smtp_reply( &account->internal->smtp_socket, &packet, &reply, &length );

    exit_without_sending_quit_command:
    wiced_packet_delete( packet );
    wiced_tcp_disconnect( &account->internal->smtp_socket );

    exit_without_connection:
    wiced_tcp_delete_socket( &account->internal->smtp_socket );

    return result;
}

static wiced_result_t send_smtp_command( wiced_tcp_socket_t *socket, const char *command, uint8_t command_length, const char* params, uint16_t params_length )
{
    wiced_packet_t *packet;
    uint8_t *data;
    uint16_t available_space;

    if (params_length > 0)
    {
        WPRINT_LIB_DEBUG(("%s %s\n\r", command, params));
    }
    else
    {
        WPRINT_LIB_DEBUG(("%s\n\r", command));
    }

    WICED_VERIFY( wiced_packet_create_tcp( socket, (uint16_t) ( command_length + params_length ), &packet, &data, &available_space ) );

    /* Fill payload with command and parameters */
    memcpy( (void*) data, (void*) command, command_length );
    data += command_length;

    memcpy( (void*) data, (void*) smtp_space, GET_CONST_BUF_LENGTH(smtp_space) );
    data += GET_CONST_BUF_LENGTH(smtp_space);

    if ( params )
    {
        memcpy( (void*) data, (void*) params, params_length );
        data += params_length;
    }

    memcpy( (void*) data, (void*) smtp_return_newline, GET_CONST_BUF_LENGTH( smtp_return_newline ) );
    data += GET_CONST_BUF_LENGTH( smtp_return_newline );

    wiced_packet_set_data_end( packet, data );

    if ( wiced_tcp_send_packet( socket, packet ) != WICED_SUCCESS )
    {
        wiced_packet_delete( packet );
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

static wiced_result_t receive_smtp_reply( wiced_tcp_socket_t *socket, wiced_packet_t **reply_packet, char **reply_string, uint16_t *reply_length )
{
    uint16_t available_space;
    WICED_VERIFY( wiced_tcp_receive( socket, reply_packet, 5000 ) );
    WICED_VERIFY( wiced_packet_get_data( *reply_packet, 0, (uint8_t**)reply_string, reply_length, &available_space ) );

#ifdef WPRINT_ENABLE_DEBUG
    {
        char reply_buffer[200] = { 0 };
        memcpy( reply_buffer, *reply_string, ( *reply_length < GET_CONST_BUF_LENGTH(reply_buffer) ) ? *reply_length : GET_CONST_BUF_LENGTH(reply_buffer) );
        WPRINT_LIB_DEBUG(( "S:%s\n\r", reply_buffer ));
    }
#endif

    *reply_string += 4;
    *reply_length = (uint16_t) ( *reply_length - 4 );
    return WICED_SUCCESS;
}

static uint16_t get_smtp_reply_code( wiced_tcp_socket_t *socket, wiced_packet_t *reply_packet )
{
    char error_code_str[4];
    uint16_t length, available_space;
    uint8_t *data;

    UNUSED_PARAMETER( socket );

    WICED_VERIFY( wiced_packet_get_data( reply_packet, 0, &data, &length, &available_space ) );

    memcpy( (void*) error_code_str, (void*) data, 3 );
    error_code_str[3] = '\0';
    return (uint16_t) atoi( error_code_str );
}

static wiced_result_t send_smtp_sender( wiced_tcp_socket_t *socket, const char *address, uint8_t length )
{
    wiced_packet_t *packet;
    uint8_t *data;
    uint16_t available_space;

    WICED_VERIFY( wiced_packet_create_tcp( socket, (uint16_t)length, &packet, &data, &available_space ) );

    memcpy( (void*) data, (void*) smtp_mail_from, GET_CONST_BUF_LENGTH( smtp_mail_from ) );
    data += GET_CONST_BUF_LENGTH( smtp_mail_from );
    memcpy( (void*) data, (void*) address, length );
    data += length;
    memcpy( (void*) data, (void*) smtp_end_of_rcpt_to, GET_CONST_BUF_LENGTH( smtp_end_of_rcpt_to ) );
    data += GET_CONST_BUF_LENGTH( smtp_end_of_rcpt_to );

    wiced_packet_set_data_end( packet, data );

    if ( wiced_tcp_send_packet( socket, packet ) != WICED_SUCCESS )
    {
        wiced_packet_delete( packet );
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

static wiced_result_t send_smtp_recipient( wiced_tcp_socket_t *socket, const char *address, uint8_t length )
{
    wiced_packet_t *packet;
    uint8_t *data;
    uint16_t available_space;

    WICED_VERIFY( wiced_packet_create_tcp( socket, (uint16_t)length, &packet, &data, &available_space ) );

    memcpy( (void*) data, (void*) smtp_rcpt_to, GET_CONST_BUF_LENGTH( smtp_rcpt_to ) );
    data += GET_CONST_BUF_LENGTH( smtp_rcpt_to );
    memcpy( (void*) data, (void*) address, length );
    data += length;
    memcpy( (void*) data, (void*) smtp_end_of_rcpt_to, GET_CONST_BUF_LENGTH( smtp_end_of_rcpt_to ) );
    data += GET_CONST_BUF_LENGTH( smtp_end_of_rcpt_to );

    wiced_packet_set_data_end( packet, data );

    if ( wiced_tcp_send_packet( socket, packet ) != WICED_SUCCESS )
    {
        wiced_packet_delete( packet );
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

static uint8_t get_first_address_from_list(char **address_list, char **first_address, uint8_t *first_address_length)
{
    char *current = (char*)*address_list;
    uint8_t length = 0;

    /* Clear leading spaces, if any */
    while ( *current == ' ' || *current == ';' || *current == ',' )
    {
        current++;
    }

    if ( *current == '\0' )
    {
        return 0;
    }

    *first_address = current;

    /* Iterate until the end of first address */
    while ( *current != ' ' && *current != ';' && *current != ',' && *current != '\0' )
    {
        current++;
        length++;
    }

    *address_list = current;
    *first_address_length = length;
    return 1;
}

static void init_content_table(const wiced_email_account_t *account, const wiced_email_t *email, const char date[32], email_content_table_field_t *content_table )
{
    uint32_t index = 0;

    /* Message-ID */
    content_table[index].content   = (uint8_t*)email_header_msg_id;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_msg_id);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* Date */
    content_table[index].content   = (uint8_t*)email_header_date;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_date);
    content_table[index].content   = (uint8_t*)date;
    content_table[index++].length  = strlen(date);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* From */
    content_table[index].content   = (uint8_t*)email_header_from;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_from);
    content_table[index].content   = (uint8_t*)account->email_address;
    content_table[index++].length  = strlen( account->email_address );
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* MIME-Version */
    content_table[index].content   = (uint8_t*)email_header_mime_version;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_mime_version);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* To */
    content_table[index].content   = (uint8_t*)email_header_to;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_to);
    content_table[index].content   = (uint8_t*)email->to_addresses;
    content_table[index++].length  = strlen( email->to_addresses );
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* Cc */
    content_table[index].content   = (uint8_t*)email_header_cc;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_cc);
    content_table[index].content   = (uint8_t*)email->cc_addresses;
    content_table[index++].length  = strlen( email->cc_addresses );
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* Subject */
    content_table[index].content   = (uint8_t*)email_header_subject;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_subject);
    content_table[index].content   = (uint8_t*)email->subject;
    content_table[index++].length  = strlen( email->subject );
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* Content Type */
    content_table[index].content   = (uint8_t*)email_header_content_type;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_content_type);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* Content Transfer Encoding */
    content_table[index].content   = (uint8_t*)email_header_content_tranfer_encoding;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(email_header_content_tranfer_encoding);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_return_newline);

    /* Email body */
    content_table[index].content   = (uint8_t*)email->content;
    content_table[index++].length  = email->content_length;

    /* Email signature */
    content_table[index].content   = (uint8_t*)email->signature;
    content_table[index++].length  = email->signature_length;

    /* End of data */
    content_table[index].content   = (uint8_t*)smtp_end_of_data;
    content_table[index++].length  = GET_CONST_BUF_LENGTH(smtp_end_of_data);
}
