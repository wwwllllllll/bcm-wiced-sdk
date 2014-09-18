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
#include "wwd_constants.h"
#include "wiced_tcpip.h"
#include "wiced_tls.h"
#include "tls_host_api.h"
#include "wwd_crypto.h"
#include "wwd_buffer_interface.h"
#include "crypto_constants.h"
#include "crypto_structures.h"
#include "wiced_time.h"
#include "wwd_assert.h"
#include "wwd_buffer_interface.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef WICED_TLS_MAX_RESUMABLE_SESSIONS
#define WICED_TLS_MAX_RESUMABLE_SESSIONS    4
#endif

/* Maximum number of session reconnects */
#define MAX_TLS_SESSION_AGE     32

#define SSL_IS_CLIENT            0
#define SSL_IS_SERVER            1
#define SESSION_CAN_BE_RESUMED   1
#define SESSION_NO_TIMEOUT       0

#define MAX_HANDSHAKE_WAIT  10000

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

static int32_t tls_get_session( ssl_context *ssl );
static int32_t tls_set_session( ssl_context *ssl );
static wiced_result_t tls_packetize_buffered_data( wiced_tls_context_t* context, wiced_packet_t** packet );

static besl_result_t wiced_tls_load_certificate( wiced_tls_certificate_t* certificate, const char* cert_string );
static besl_result_t wiced_tls_load_key( wiced_tls_key_t* key, const char* key_string );

extern int32_t x509parse_crt( x509_cert *chain, const unsigned char *buf, uint32_t buflen );
extern int32_t x509parse_crtfile( x509_cert *chain, const char *path );
extern int32_t x509parse_key( rsa_context *rsa, const unsigned char *buf, uint32_t buflen, const unsigned char *pwd, uint32_t pwdlen );
extern int32_t x509parse_keyfile( rsa_context *rsa, const char *path, const char *pwd );
extern int32_t x509parse_dn_gets( char *buf, char *end, const x509_name *dn );
extern char *x509parse_cert_info( const char *prefix, const x509_cert *crt );
extern int32_t x509parse_expired( const x509_cert *crt );
extern int32_t x509parse_verify( const x509_cert *crt, const x509_cert *trust_ca, const char *cn, int32_t *flags );
extern void x509_free( x509_cert *crt );

extern void rsa_free( rsa_context *ctx );

extern tls_result_t ssl_init( ssl_context *ssl );
extern void ssl_set_rng( ssl_context *ssl, int32_t (*f_rng)( void * ), void *p_rng );
extern void ssl_set_session( ssl_context *ssl, int32_t resume, int32_t timeout, ssl_session *session );
extern void ssl_free( ssl_context *ssl );
extern void ssl_set_endpoint( ssl_context *ssl, int32_t endpoint );
extern void ssl_set_ca_chain( ssl_context *ssl, x509_cert *ca_chain, const char *peer_cn );
extern void ssl_set_authmode( ssl_context *ssl, int32_t authmode );
extern void ssl_set_own_cert( ssl_context *ssl, x509_cert *own_cert, rsa_context *rsa_key );
extern tls_result_t ssl_handshake_server_async( ssl_context *ssl );
extern tls_result_t ssl_handshake_client_async( ssl_context *ssl );
extern tls_result_t ssl_set_dh_param( ssl_context *ssl, const unsigned char *dhm_P, int P_len, const unsigned char *dhm_G, int G_len );
extern int32_t ssl_close_notify( ssl_context *ssl );
extern tls_result_t tls_encrypt_record( ssl_context *ssl, tls_record_t* record, uint32_t message_length );
extern tls_result_t tls_decrypt_record( wiced_tls_context_t *ssl, tls_record_t* record );

extern void md5_update( md5_context *ctx, const unsigned char *input, int32_t ilen );
extern void sha1_update( sha1_context *ctx, const unsigned char *input, int32_t ilen );

extern void microrng_init( microrng_state *state);
extern int32_t microrng_rand( void *p_state);

/* Extern network functions */
extern wiced_result_t network_tcp_receive (wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout );
extern wiced_result_t network_tcp_send_packet   ( wiced_tcp_socket_t* socket, wiced_packet_t*  packet );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static ssl_session session_list[WICED_TLS_MAX_RESUMABLE_SESSIONS];
static int session_list_initialized = 0;

/**
 * This list of cipher suites should be in order of strength with the strongest first.
 * Do not enable cipher suites unless they meet your security requirements
 */
static const cipher_suite_t* my_ciphers[] =
{
        /* First: Ephemeral Diffie Hellman with 256 bit cipher, in order of MAC strength */
        &TLS_DHE_RSA_WITH_AES_256_CBC_SHA256,
        &TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256,
        &TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
        &TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA,

        /* Next: Ephemeral Diffie Hellman with 128 bit cipher, in order of MAC strength */
        &TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
        &TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256,
        &TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
        &TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA,

        /* Next: Regular Diffie Hellman with 256 bit cipher, in order of MAC strength */
        &TLS_RSA_WITH_AES_256_CBC_SHA256,
        &TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256,
        &TLS_RSA_WITH_AES_256_CBC_SHA,
        &TLS_RSA_WITH_CAMELLIA_256_CBC_SHA,

        /* Next: Regular Diffie Hellman with 128 bit cipher, in order of MAC strength */
        &TLS_RSA_WITH_AES_128_CBC_SHA256,
        &TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256,
        &TLS_RSA_WITH_AES_128_CBC_SHA,
        &TLS_RSA_WITH_CAMELLIA_128_CBC_SHA,

        /* Finally: Other ciphers */
        &TLS_RSA_WITH_SEED_CBC_SHA,

//        &TLS_RSA_WITH_RC4_128_MD5,         /* Insecure */
//        &TLS_RSA_WITH_RC4_128_SHA,         /* Insecure */
//        &TLS_RSA_WITH_3DES_EDE_CBC_SHA,    /* Insecure */
//        &TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA /* Insecure */

        0   /* List termination */
};

wiced_tls_certificate_t* root_ca_certificates = NULL;

/* DH prime */
static const unsigned char diffie_hellman_prime_P[] =
{
        0x9C, 0xE8, 0x56, 0x40, 0x90, 0x3B, 0xF1, 0x23,
        0x90, 0x69, 0x47, 0xFE, 0xDE, 0x76, 0x72, 0x61,
        0xD9, 0xB4, 0xA9, 0x73, 0xEB, 0x8F, 0x7D, 0x98,
        0x4A, 0x8C, 0x65, 0x6E, 0x2B, 0xCC, 0x16, 0x1C,
        0x18, 0x3D, 0x4C, 0xA4, 0x71, 0xBA, 0x78, 0x22,
        0x5F, 0x94, 0x0F, 0x16, 0xD1, 0xD9, 0x9C, 0xA3,
        0xE6, 0x61, 0x52, 0xCC, 0x68, 0xED, 0xCE, 0x13,
        0x11, 0xA3, 0x90, 0xF3, 0x07, 0x74, 0x18, 0x35,
        0x44, 0xFF, 0x6A, 0xB5, 0x53, 0xEC, 0x70, 0x73,
        0xAD, 0x0C, 0xB6, 0x08, 0xF2, 0xA3, 0xB4, 0x80,
        0x19, 0xE6, 0xC0, 0x2B, 0xCE, 0xD4, 0x0B, 0xD3,
        0x0E, 0x91, 0xBB, 0x24, 0x69, 0x08, 0x96, 0x70,
        0xDE, 0xF4, 0x09, 0xC0, 0x8E, 0x8A, 0xC2, 0x4D,
        0x17, 0x32, 0xA6, 0x12, 0x8D, 0x22, 0x20, 0xDC,
        0x53,
};
static const unsigned char diffie_hellman_prime_G[] = { 4 };

/******************************************************
 *               Function Definitions
 ******************************************************/

tls_result_t tls_host_create_buffer( ssl_context* ssl, uint8_t** buffer, uint16_t buffer_size )
{
    wiced_assert("", ssl->outgoing_packet == NULL);

    /* Round requested buffer size up to next 64 byte chunk (required if encryption is active) */
    buffer_size = (uint16_t) ROUND_UP(buffer_size, 64);

    /* Check if requested buffer fits within a single MTU */
    if (buffer_size < 1300) /* TODO: Fix this */
    {
        uint16_t actual_packet_size;
        if ( wiced_packet_create_tcp( ssl->send_context, buffer_size, (wiced_packet_t**) &ssl->outgoing_packet, buffer, &actual_packet_size ) != WICED_SUCCESS )
        {
            *buffer = NULL;
            return 1;
        }
    }
    else
    {
        /* Malloc space */
        *buffer = tls_host_malloc("tls", buffer_size);
        ssl->out_buffer_size = buffer_size;
    }

    return 0;
}


tls_result_t tls_host_get_packet_data( tls_packet_t* packet, uint32_t offset, uint8_t** data, uint16_t* data_length, uint16_t* available_data_length )
{
    uint16_t temp_length;
    uint16_t temp_available_length;
    if ( wiced_packet_get_data((wiced_packet_t*)packet, (uint16_t)offset, data, &temp_length, &temp_available_length) == WICED_SUCCESS)
    {
        *data_length = temp_length;
        *available_data_length = temp_available_length;
        return WICED_SUCCESS;
    }
    else
    {
        return TLS_ERROR_BAD_INPUT_DATA;
    }
}

void* tls_host_malloc( const char* name, uint32_t size)
{
    (void) name;
    return malloc_named( name, size );
}

void tls_host_free(void* p)
{
    free( p );
}

void* tls_host_get_defragmentation_buffer ( uint16_t size )
{
    return malloc_named( "tls", size );
}

void  tls_host_free_defragmentation_buffer( void* buffer )
{
    free( buffer );
}

/*
 * Flush any data not yet written
 */
tls_result_t ssl_flush_output( ssl_context *ssl, uint8_t* buffer, uint32_t length )
{
    if (ssl->outgoing_packet != NULL)
    {
        wiced_packet_set_data_end((wiced_packet_t*)ssl->outgoing_packet, buffer + length);
        tls_host_send_packet(ssl->send_context, ssl->outgoing_packet);
        ssl->outgoing_packet = NULL;
        ssl->out_buffer_size = 0;
    }
    else
    {
        uint16_t      actual_packet_size;
        tls_packet_t* temp_packet;
        uint8_t*      packet_buffer;
        uint8_t*      data = buffer;

        while (length != 0)
        {
            uint16_t amount_to_copy;
            if ( wiced_packet_create_tcp( ssl->send_context, (uint16_t)length, (wiced_packet_t**) &temp_packet, &packet_buffer, &actual_packet_size ) != WICED_SUCCESS )
            {
                free(buffer);
                return 1;
            }
            amount_to_copy = (uint16_t) MIN(length, actual_packet_size);
            packet_buffer = MEMCAT(packet_buffer, data, amount_to_copy);
            data   += amount_to_copy;
            length -= amount_to_copy;
            wiced_packet_set_data_end((wiced_packet_t*)temp_packet, packet_buffer );
            tls_host_send_packet(ssl->send_context, temp_packet);
        }

        free(buffer);
    }

    return( 0 );
}

uint64_t tls_host_get_time_ms( void )
{
    uint64_t time_ms;
    wiced_time_get_utc_time_ms( (wiced_utc_time_ms_t*) &time_ms );
    return time_ms;
}

static int32_t tls_get_session( ssl_context *ssl )
{
    int idx;
    ssl_session *cur;
    uint64_t t = tls_host_get_time_ms( );

    if ( ssl->resume == 0 )
        return ( 1 );

    /* self-initializing */
    if ( !session_list_initialized )
    {
        memset( session_list, 0, sizeof( session_list ) );
        session_list_initialized = 1;
    }

    for ( idx = 0; idx < WICED_TLS_MAX_RESUMABLE_SESSIONS; idx++ )
    {
        cur = &( session_list[idx] );

        if ( ( !cur->length ) || ( ssl->session->cipher != cur->cipher ) || ( ssl->session->length != cur->length ) )
            continue;

        if ( memcmp( ssl->session->id, cur->id, (size_t)cur->length ) != 0 )
            continue;

        /* if timeout is non-zero, timeout frees matched session slot after t=timeout.
         * zero timeout has no effect (never expires) */
        if ( (ssl->timeout != 0) && ((time_t)t - cur->start > ssl->timeout ))
        {
            goto done;
        }

        /* if age is non-zero,
         * age frees matched session slot after it has been
         * resumed age number of times, regardless
         * of whether timeout is enabled and working.*/
        if ( cur->age )
        {
            cur->age--;
            /* if aged out then reuse this slot */
            if ( !cur->age )
            {
                goto done;
            }
        }

        /* session matched, restore master key */
        memcpy( ssl->session->master, cur->master, 48 );

        /* refresh the expiration timeout */
        cur->start = (time_t)t;

        /* resuming session is true */
        return ( 0 );
    }

    done:

    /* resuming session is false */
    return ( 1 );
}

static int32_t tls_set_session( ssl_context *ssl )
{
    int idx;
    ssl_session *cur;
    uint64_t t = tls_host_get_time_ms( );
    int age = MAX_TLS_SESSION_AGE; /* oldest age */
    int old = 0; /* oldest slot */

    /* self-initializing */
    if ( !session_list_initialized )
    {
        memset( session_list, 0, sizeof( session_list ) );
        session_list_initialized = 1;
    }

    /* search static list */
    for ( idx = 0; idx < WICED_TLS_MAX_RESUMABLE_SESSIONS; idx++ )
    {
        cur = &( session_list[idx] );

        /* unused? */
        if ( !cur->age )
            break;

        /* keeps track of oldest entry if a slot needs to be taken away. */
        if ( cur->age < age )
        {
            age = cur->age;
            old = idx;
        }

    } /* for session_list */

    /* check if all slots in use, then take away oldest slot */
    if ( WICED_TLS_MAX_RESUMABLE_SESSIONS == idx )
    {
        idx = old;
        cur = &( session_list[idx] );
    }

    /* save session */
    memcpy( cur, ssl->session, sizeof(ssl_session) );
    /* set expiration timer */
    cur->start = (time_t)t;
    cur->age   = MAX_TLS_SESSION_AGE;

    /* session was saved */
    return ( 0 );
}

static besl_result_t wiced_tls_load_certificate( wiced_tls_certificate_t* certificate, const char* cert_string )
{
    if ( x509parse_crt( certificate, (unsigned char *) cert_string, (uint32_t) strlen( cert_string ) ) != 0 )
    {
        wiced_assert("Certificate parse error", 0 != 0 );
        return BESL_CERT_PARSE_FAIL;
    }
    else
    {
        return BESL_SUCCESS;
    }
}

static besl_result_t wiced_tls_load_key( wiced_tls_key_t* key, const char* key_string )
{
    if ( x509parse_key( key, (unsigned char *) key_string, (uint32_t) strlen( key_string ), NULL, 0 ) != 0 )
    {
        wiced_assert("Key parse error", 0 != 0 );
        return BESL_KEY_PARSE_FAIL;
    }
    else
    {
        return BESL_SUCCESS;
    }
}

wiced_result_t wiced_tls_init_simple_context( wiced_tls_simple_context_t* context, const char* peer_cn )
{
    memset(context, 0, sizeof(wiced_tls_simple_context_t));
    context->context_type    = WICED_TLS_SIMPLE_CONTEXT;
    context->context.peer_cn = peer_cn;
    return WICED_SUCCESS;
}

wiced_result_t wiced_tls_init_advanced_context( wiced_tls_advanced_context_t* context, const char* certificate, const char* key)
{
    wiced_result_t result;

    wiced_assert("Bad args", (certificate != NULL) && (key != NULL));

    memset( context, 0, sizeof(wiced_tls_advanced_context_t) );
    context->context_type = WICED_TLS_ADVANCED_CONTEXT;

    result = wiced_tls_load_certificate( &context->certificate, certificate );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    result = wiced_tls_load_key( &context->key, key );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_tls_reset_context( wiced_tls_simple_context_t* tls_context )
{
    ssl_free( &tls_context->context);

    return WICED_SUCCESS;
}

wiced_result_t wiced_tls_deinit_context( wiced_tls_simple_context_t* tls_context )
{
    /* Check if context is of an advanced variety. Note that the server and advanced client context are exactly the same */
    if (tls_context->context_type == WICED_TLS_ADVANCED_CONTEXT)
    {
        x509_free(&((wiced_tls_advanced_context_t*)tls_context)->certificate);
        rsa_free(&((wiced_tls_advanced_context_t*)tls_context)->key);
    }

    ssl_free( &tls_context->context);

    return WICED_SUCCESS;
}

wiced_result_t wiced_tls_init_root_ca_certificates( const char* trusted_ca_certificates )
{
    wiced_result_t result;
    wiced_tls_deinit_root_ca_certificates();

    root_ca_certificates = tls_host_malloc("tls", sizeof(wiced_tls_certificate_t));
    if ( root_ca_certificates == NULL )
    {
        return WICED_OUT_OF_HEAP_SPACE;
    }

    memset(root_ca_certificates, 0, sizeof(wiced_tls_certificate_t));

    result = wiced_tls_load_certificate( root_ca_certificates, trusted_ca_certificates );
    if ( result != WICED_SUCCESS )
    {
        free( root_ca_certificates );
        root_ca_certificates = NULL;
        return result;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_tls_deinit_root_ca_certificates( void )
{
    if ( root_ca_certificates != NULL )
    {
        x509_free(root_ca_certificates);
        free( root_ca_certificates );
        root_ca_certificates = NULL;
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_enable_tls( wiced_tcp_socket_t* socket, void* context )
{
    socket->tls_context = context;
    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_start_tls( wiced_tcp_socket_t* socket, wiced_tls_endpoint_type_t type, wiced_tls_certificate_verification_t verification )
{
    return wiced_tcp_start_tls_with_ciphers( socket, type, verification, my_ciphers );
}

wiced_result_t wiced_tcp_start_tls_with_ciphers( wiced_tcp_socket_t* socket, wiced_tls_endpoint_type_t type, wiced_tls_certificate_verification_t verification, const cipher_suite_t* cipher_list[] )
{
    microrng_state              rngstate;
    wiced_tls_simple_context_t* tls_context = (wiced_tls_simple_context_t*)socket->tls_context;
    int                         prev_state;
    uint64_t                    start_time;
    tls_result_t                result;

    /* Initialize the session data */
    memset( &tls_context->session, 0, sizeof(wiced_tls_session_t) );
    memset( &tls_context->context, 0, sizeof(wiced_tls_context_t) );

    /* Prepare session and entropy */
    tls_context->session.age = MAX_TLS_SESSION_AGE;
    wwd_wifi_get_random( &rngstate.entropy, 4 );

    /* Initialize session context */ /* TODO: Ideally this should be done once for a socket */
    if ( ssl_init( &tls_context->context ) != 0 )
    {
        wiced_assert("Error initialising SSL", 0!=0 );
        return TLS_INIT_FAIL;
    }

    microrng_init( &rngstate );

    ssl_set_endpoint( &tls_context->context, type );
    ssl_set_rng     ( &tls_context->context, microrng_rand, &rngstate );
    tls_context->context.receive_context = socket;
    tls_context->context.send_context    = socket;
    tls_context->context.get_session     = tls_get_session;
    tls_context->context.set_session     = tls_set_session;
    tls_context->context.ciphers         = cipher_list;
    ssl_set_session ( &tls_context->context, SESSION_CAN_BE_RESUMED, SESSION_NO_TIMEOUT, &tls_context->session );

    /* Assert if user has not created correct TLS context for the TLS endpoint type */
    wiced_assert("TLS servers must have an advanced TLS context", !((type == WICED_TLS_AS_SERVER) && (socket->tls_context->context_type != WICED_TLS_ADVANCED_CONTEXT)));

    if ( root_ca_certificates != NULL )
    {
        ssl_set_ca_chain( &tls_context->context, root_ca_certificates, tls_context->context.peer_cn );
        ssl_set_authmode( &tls_context->context, verification );
    }
    else
    {
        ssl_set_authmode( &tls_context->context, SSL_VERIFY_NONE );
    }

    if ( socket->tls_context->context_type == WICED_TLS_ADVANCED_CONTEXT )
    {
        wiced_tls_advanced_context_t* advanced_context = (wiced_tls_advanced_context_t*)tls_context;
        ssl_set_own_cert( &advanced_context->context, &advanced_context->certificate, &advanced_context->key );

        ssl_set_dh_param( &socket->tls_context->context, diffie_hellman_prime_P, sizeof( diffie_hellman_prime_P ), diffie_hellman_prime_G, sizeof( diffie_hellman_prime_G ) );
    }

    prev_state = 0;
    start_time = tls_host_get_time_ms();
    do
    {
        uint64_t curr_time;
        if (type == WICED_TLS_AS_SERVER)
        {
            result = ssl_handshake_server_async( &tls_context->context );
            if ( result != TLS_SUCCESS )
            {
                WPRINT_SECURITY_INFO(( "Error with TLS handshake\n" ));
                goto exit_with_inited_context;
            }
        }
        else
        {
            result = ssl_handshake_client_async( &tls_context->context );
            if ( result != TLS_SUCCESS )
            {
                WPRINT_SECURITY_INFO(( "Error with TLS handshake\n" ));
                goto exit_with_inited_context;
            }
        }

        /* break out if stuck */
        curr_time = tls_host_get_time_ms();
        if ( curr_time - start_time > MAX_HANDSHAKE_WAIT )
        {
            WPRINT_SECURITY_INFO(( "Timeout in SSL handshake\n" ));
            result = TLS_HANDSHAKE_TIMEOUT;
            goto exit_with_inited_context;
        }

        /* if no state change then wait on client */
        if ( prev_state == tls_context->context.state )
        {
            host_rtos_delay_milliseconds( 10 );
        }
        else /* otherwise process next state with no delay */
        {
            prev_state = tls_context->context.state;
        }
    } while ( tls_context->context.state != SSL_HANDSHAKE_OVER );

    return WICED_SUCCESS;

exit_with_inited_context:
    ssl_close_notify( &tls_context->context );
    ssl_free(&tls_context->context);
    return (wiced_result_t) result;
}

wiced_result_t wiced_tls_calculate_overhead( wiced_tls_context_t* context, uint16_t content_length, uint16_t* header, uint16_t* footer )
{
    *header = 0;
    *footer = 0;
    if ( context != NULL && context->state == SSL_HANDSHAKE_OVER )
    {
        int padlen;

        *header = sizeof(tls_record_header_t);

        if ( context->ivlen != 0 )
        {
            /* Padding */
            padlen = context->ivlen - ( content_length +context->maclen + 1 ) % context->ivlen;
            if ( padlen == context->ivlen )
            {
                padlen = 0;
            }

            *footer += padlen + 1;
        }


        /* MAC */
        *footer += context->maclen;

        /* TLS 1.1+ uses explicit IV in footer */
        if( context->minor_ver > 1 )
        {
            *footer += context->ivlen;
        }
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_tls_encrypt_packet( wiced_tls_context_t* context, wiced_packet_t* packet )
{
    uint8_t* data;
    uint16_t length;
    uint16_t available;
    wiced_result_t result;
    tls_record_t* record;

    wiced_packet_get_data(packet, 0, &data, &length, &available);
    data -= sizeof(tls_record_header_t);
    result = tls_host_set_packet_start((tls_packet_t*)packet, data);
    if ( result != WICED_SUCCESS)
    {
        return result;
    }

    record                = (tls_record_t*) data;
    record->type          = SSL_MSG_APPLICATION_DATA;
    record->major_version = (uint8_t)context->major_ver;
    record->minor_version = (uint8_t)context->minor_ver;
    record->length        = htobe16( length );

    result = tls_encrypt_record( context, record, length );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    wiced_packet_set_data_end(packet, data + htobe16(record->length) + sizeof(tls_record_header_t));
    return WICED_SUCCESS;
}

#if 0
wiced_result_t wiced_tls_send_buffer(wiced_tcp_socket_t* socket, void* buffer, uint16_t buffer_length)
{
    UNUSED_PARAMETER(socket);
    UNUSED_PARAMETER(buffer);
    UNUSED_PARAMETER(buffer_length);
    return WICED_UNSUPPORTED;
}
#endif

wiced_result_t wiced_tls_receive_packet( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    wiced_result_t result;
    wiced_tls_context_t* context = &socket->tls_context->context;


    /* Check if we already have a record which should only happen if it was larger than a packet which means it's stored in the defragmentation buffer */
    if ( context->current_record != NULL )
    {
        wiced_assert( "Something wrong", (void*)context->current_record == context->defragmentation_buffer );
        return tls_packetize_buffered_data( context, packet );
    }
    else
    {
        tls_record_t* record;
        result = tls_get_next_record( context, &record, timeout, TLS_RECEIVE_PACKET_IF_NEEDED );
        if ( result != WICED_SUCCESS )
        {
            return result;
        }
        /* Check if this record has been defragmented */
        if ( (void*)record == context->defragmentation_buffer )
        {
            return tls_packetize_buffered_data( context, packet );
        }
        else
        {
            tls_record_t* temp_record;
            uint8_t* packet_data;
            uint16_t length;
            uint16_t available;
            uint8_t* end_of_data;

            /* We have a pointer to the current record so we can move on */
            tls_skip_current_record(context);

            /* Make sure we process every record in this packet */
            end_of_data = record->message + htobe16( record->length );
            while ( tls_get_next_record( context, &temp_record, timeout, TLS_AVOID_NEW_RECORD_PACKET_RECEIVE ) == TLS_SUCCESS )
            {
                /* Make the record data contiguous with the previous record */
                uint16_t temp_record_length = htobe16( temp_record->length );
                end_of_data = MEMCAT( end_of_data, temp_record->message, temp_record_length );
                record->length = htobe16( htobe16(record->length) + temp_record_length );
                tls_skip_current_record( context );
            }

            /* Set the packet start and end */
            wiced_packet_get_data( (wiced_packet_t*)context->received_packet, 0, &packet_data, &length, &available );
            tls_host_set_packet_start( context->received_packet, record->message );
            wiced_packet_set_data_end( (wiced_packet_t*)context->received_packet, end_of_data );

            *packet = (wiced_packet_t*)context->received_packet;
            context->received_packet        = NULL;
            context->received_packet_length = 0;
        }
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_tls_close_notify( wiced_tcp_socket_t* socket )
{
    if ( socket->tls_context != NULL )
    {
        ssl_close_notify( &socket->tls_context->context );
    }
    return WICED_SUCCESS;
}


static wiced_result_t tls_packetize_buffered_data( wiced_tls_context_t* context, wiced_packet_t** packet )
{
    uint8_t* data;
    uint16_t length;
    uint16_t available_length;
    uint16_t record_length;
    wiced_result_t result;
    uint32_t amount_to_copy;

    (void)available_length;

    record_length = (uint16_t)( htobe16(context->current_record->length) + sizeof(tls_record_header_t) );

    /* Get a buffer and fill with decrypted content */
    result = wiced_packet_create_tcp( context->send_context, (uint16_t) MIN(1024, record_length - context->defragmentation_buffer_bytes_processed), (wiced_packet_t**) packet, &data, &length );
    if ( result  != WICED_SUCCESS )
    {
        *packet = NULL;
        return result;
    }

    amount_to_copy = (uint32_t) MIN( length, record_length - context->defragmentation_buffer_bytes_processed );
    memcpy( data, &context->defragmentation_buffer[context->defragmentation_buffer_bytes_processed], amount_to_copy );
    wiced_packet_set_data_end( *packet, data + amount_to_copy );

    context->defragmentation_buffer_bytes_processed = (uint16_t) ( context->defragmentation_buffer_bytes_processed + amount_to_copy );

    /* Check if we've returned all the data to the user */
    if ( context->defragmentation_buffer_bytes_processed >= record_length )
    {
        tls_host_free_defragmentation_buffer(context->defragmentation_buffer);
        context->defragmentation_buffer = 0;
        context->current_record = NULL;
    }

    return WICED_SUCCESS;
}

wiced_bool_t wiced_tls_is_encryption_enabled( wiced_tcp_socket_t* socket )
{
    if ( socket->tls_context != NULL && socket->tls_context->context.state == SSL_HANDSHAKE_OVER )
    {
        return WICED_TRUE;
    }
    else
    {
        return WICED_FALSE;
    }
}

/*
 * TLS support functions
 */
tls_result_t tls_host_free_packet( tls_packet_t* packet )
{
    wiced_packet_delete((wiced_packet_t*)packet);
    return WICED_SUCCESS;
}

tls_result_t tls_host_send_packet(void* context, tls_packet_t* packet)
{
    if ( network_tcp_send_packet( (wiced_tcp_socket_t*)context, (wiced_packet_t*)packet) != WICED_SUCCESS )
    {
        wiced_packet_delete((wiced_packet_t*)packet);
    }
    return WICED_SUCCESS;
}

tls_result_t tls_host_receive_packet(void* context, tls_packet_t** packet, uint32_t timeout)
{
    return network_tcp_receive( (wiced_tcp_socket_t*) context, (wiced_packet_t**) packet, timeout );
}

tls_result_t tls_host_set_packet_start( tls_packet_t* packet, uint8_t* start )
{
    wiced_packet_set_data_start((wiced_packet_t*)packet, start);
    return WICED_SUCCESS;
}
