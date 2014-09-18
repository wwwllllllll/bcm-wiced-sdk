/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "dns.h"
#include "string.h"
#include "wiced_network.h"
#include "wiced_tcpip.h"
#include "wwd_debug.h"
#include <wiced_utilities.h>
#include "wiced_time.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define DNS_PORT    (53)

#ifndef WICED_MAXIMUM_STORED_DNS_SERVERS
#define WICED_MAXIMUM_STORED_DNS_SERVERS (2)
#endif

/* Change to 1 to turn debug trace */
#define WICED_DNS_DEBUG   (0)

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

static wiced_ip_address_t dns_server_address_array[WICED_MAXIMUM_STORED_DNS_SERVERS];
static uint32_t           dns_server_address_count = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t dns_client_add_server_address( wiced_ip_address_t address )
{
    if ( dns_server_address_count >= WICED_MAXIMUM_STORED_DNS_SERVERS )
    {
        /* Server address array is full */
        return WICED_ERROR;
    }

    dns_server_address_array[dns_server_address_count++] = address;
    return WICED_SUCCESS;
}

wiced_result_t dns_client_remove_all_server_addresses( void )
{
    memset( dns_server_address_array, 0, sizeof( dns_server_address_array ) );
    dns_server_address_count = 0;
    return WICED_SUCCESS;
}

wiced_result_t dns_client_hostname_lookup( const char* hostname, wiced_ip_address_t* address, uint32_t timeout_ms )
{
    uint32_t               a;
    wiced_packet_t*        packet;
    dns_message_iterator_t iter;
    wiced_udp_socket_t     socket;
    uint32_t               remaining_time         = timeout_ms;
    wiced_ip_address_t     resolved_ipv4_address  = { WICED_INVALID_IP, .ip.v4 = 0 };
    wiced_ip_address_t     resolved_ipv6_address  = { WICED_INVALID_IP, .ip.v4 = 0 };
    wiced_bool_t           ipv4_address_found     = WICED_FALSE;
    wiced_bool_t           ipv6_address_found     = WICED_FALSE;
    char*                  temp_hostname          = (char*)hostname;
    uint16_t               hostname_length        = (uint16_t) strlen( hostname );

#if WICED_DNS_DEBUG
    wiced_time_t time;
    wiced_time_get_time( &time );
    WPRINT_LIB_INFO(("DNS lookup start @ %d\n\r", (int)time ));
#endif

    /* Create socket to send packets */
    if ( wiced_udp_create_socket( &socket, WICED_ANY_PORT, WICED_STA_INTERFACE ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }

    while ( ipv4_address_found == WICED_FALSE && remaining_time > 0)
    {
        /* Send DNS query messages */
        for ( a = 0; a < dns_server_address_count; a++ )
        {
            wiced_ip_address_t host_ipv6_address;
            uint16_t           available_space;

            /* Create IPv4 query packet */
            if ( wiced_packet_create_udp( &socket, (uint16_t) ( sizeof(dns_message_header_t) + sizeof(dns_question_t) + hostname_length ), &packet, (uint8_t**) &iter.header, &available_space ) != WICED_SUCCESS )
            {
                goto exit;
            }

            iter.iter = (uint8_t*) ( iter.header ) + sizeof(dns_message_header_t);
            dns_write_header( &iter, 0, 0x100, 1, 0, 0, 0 );
            dns_write_question( &iter, temp_hostname, RR_CLASS_IN, RR_TYPE_A );
            wiced_packet_set_data_end( packet, iter.iter );

            /* Send IPv4 query packet */
            if ( wiced_udp_send( &socket, &dns_server_address_array[a], DNS_PORT, packet ) != WICED_SUCCESS )
            {
                wiced_packet_delete( packet );
                goto exit;
            }

            /* Check if host has global IPv6 address. If it does, send IPv6 query too */
            if ( wiced_ip_get_ipv6_address( WICED_STA_INTERFACE, &host_ipv6_address, IPv6_GLOBAL_ADDRESS ) == WICED_SUCCESS )
            {
                /* Create IPv6 query packet */
                if ( wiced_packet_create_udp( &socket, (uint16_t) ( sizeof(dns_message_header_t) + sizeof(dns_question_t) + hostname_length ), &packet, (uint8_t**) &iter.header, &available_space ) != WICED_SUCCESS )
                {
                    goto exit;
                }

                iter.iter = (uint8_t*) ( iter.header ) + sizeof(dns_message_header_t);
                dns_write_header( &iter, 0, 0x100, 1, 0, 0, 0 );
                dns_write_question( &iter, temp_hostname, RR_CLASS_IN, RR_TYPE_AAAA );
                wiced_packet_set_data_end( packet, iter.iter );

                /* Send IPv6 query packet */
                if ( wiced_udp_send( &socket, &dns_server_address_array[a], DNS_PORT, packet ) != WICED_SUCCESS )
                {
                    wiced_packet_delete( packet );
                    goto exit;
                }
            }
        }

        /* Attempt to receive response packets */
        while ( ipv4_address_found == WICED_FALSE && remaining_time > 0 )
        {
            wiced_utc_time_ms_t current_time;
            wiced_utc_time_ms_t last_time;

            wiced_time_get_utc_time_ms( &last_time );

            if ( wiced_udp_receive( &socket, &packet, remaining_time ) == WICED_SUCCESS )
            {
                uint16_t     data_length;
                uint16_t     available_data_length;
                wiced_bool_t answer_found = WICED_FALSE;

                /* Extract the data */
                wiced_packet_get_data( packet, 0, (uint8_t**) &iter.header, &data_length, &available_data_length );
                iter.iter = (uint8_t*) ( iter.header ) + sizeof(dns_message_header_t);

                /* Check if the message is a response (otherwise its a query) */
                if ( htobe16( iter.header->flags ) & DNS_MESSAGE_IS_A_RESPONSE )
                {
                    /* Skip all the questions */
                    for ( a = 0; a < htobe16( iter.header->question_count ); ++a )
                    {
                        /* Skip the name */
                        dns_skip_name( &iter );

                        /* Skip the type and class */
                        iter.iter += 4;
                    }

                    /* Process the answer */
                    for ( a = 0; answer_found == WICED_FALSE && a < htobe16( iter.header->answer_count ); ++a )
                    {
                        dns_name_t   name;
                        dns_record_t record;

                        dns_get_next_record( &iter, &record, &name );

                        switch ( record.type )
                        {
                            case RR_TYPE_CNAME:
                                /* Received an alias for our queried name. Restart DNS query for the new name */
                                if ( temp_hostname != hostname )
                                {
                                    /* Only free if we're certain we've malloc'd it*/
                                    free( (char*) temp_hostname );
                                }
                                temp_hostname = dns_read_name( record.rdata, (dns_message_header_t*) name.start_of_packet );
                                hostname_length = (uint16_t) strlen( temp_hostname );

                                #if WICED_DNS_DEBUG
                                WPRINT_LIB_INFO( ("Found alias: %s\n", temp_hostname) );
                                #endif


                                break;

                            case RR_TYPE_A:
                            {
                                /* IP address found! */
                                answer_found       = WICED_TRUE;
                                ipv4_address_found = WICED_TRUE;

                                SET_IPV4_ADDRESS( resolved_ipv4_address, htonl(*(uint32_t*)record.rdata) );

                                #if WICED_DNS_DEBUG
                                {
                                    uint8_t* ip = record.rdata;

                                    WPRINT_LIB_INFO( ( "Found IPv4 IP: %u.%u.%u.%u\n", ( ( ip[0] ) & 0xff ), ( ( ip[1] ) & 0xff ), ( ( ip[2] ) & 0xff ), ( ( ip[3] ) & 0xff ) ) );
                                }
                                #endif

                                /* IPv4 is priority. Once found, immediately returned */
                                break;
                            }

                            case RR_TYPE_AAAA:
                            {
                                uint16_t* ip = (uint16_t*) ( &record.rdata[0] );

                                UNUSED_PARAMETER( ip );

                                answer_found = WICED_TRUE;

                                /* IPv6 address found! */
                                if ( ipv6_address_found == WICED_FALSE )
                                {
                                    ipv6_address_found = WICED_TRUE;
                                    SET_IPV6_ADDRESS( resolved_ipv6_address, MAKE_IPV6_ADDRESS(htons(ip[0]), htons(ip[1]), htons(ip[2]), htons(ip[3]), htons(ip[4]), htons(ip[5]), htons(ip[6]), htons(ip[7])));
                                }

                                #if WICED_DNS_DEBUG
                                WPRINT_LIB_INFO( ( "Found IPv6 IP: %.4X:%.4X:%.4X:%.4X:%.4X:%.4X:%.4X:%.4X\n",
                                                   (unsigned int) ( ( htons(ip[0]) ) ),
                                                   (unsigned int) ( ( htons(ip[1]) ) ),
                                                   (unsigned int) ( ( htons(ip[2]) ) ),
                                                   (unsigned int) ( ( htons(ip[3]) ) ),
                                                   (unsigned int) ( ( htons(ip[4]) ) ),
                                                   (unsigned int) ( ( htons(ip[5]) ) ),
                                                   (unsigned int) ( ( htons(ip[6]) ) ),
                                                   (unsigned int) ( ( htons(ip[7]) ) ) ) );
                                #endif

                                /* IPv6 isn't priority. Once found, still wait for IPv4 address or timeout */
                                break;
                            }

                            default:
                                break;
                        }
                    }

                    /* Skip any nameservers */
                    for ( a = 0; a < htobe16( iter.header->authoritative_answer_count ); ++a )
                    {
                        /* Skip the name */
                        dns_skip_name( &iter );

                        /* Skip the type, class and TTL */
                        iter.iter += 8;

                        /* Skip the data */
                        iter.iter += htobe16( *(uint16_t*) iter.iter ) + 2;
                    }

                    /* If no answer found yet they provided some additional A records we'll assume they are nameservers and try again */
                    if ( answer_found == WICED_FALSE && htobe16( iter.header->additional_record_count ) != 0 && htobe16( iter.header->authoritative_answer_count ) != 0 )
                    {
                        dns_name_t   name;
                        dns_record_t record;

                        /* Skip additional record */
                        dns_get_next_record( &iter, &record, &name );
                    }
                }

                wiced_packet_delete( packet );
            }

            wiced_time_get_utc_time_ms( &current_time );
            if ( ( current_time - last_time ) > remaining_time )
            {
                remaining_time = 0;
            }
            else
            {
                remaining_time = remaining_time - (uint32_t) ( current_time - last_time );
            }
        }
    }

    exit:
    wiced_udp_delete_socket( &socket );

    if ( temp_hostname != hostname )
    {
        /* Only free if we're certain we've malloc'd it*/
        free( (char*) temp_hostname );
    }

#if WICED_DNS_DEBUG
    wiced_time_get_time( &time );
    WPRINT_LIB_INFO(("DNS ends @ %d\n\r", (int)time ));
#endif

    if ( ipv4_address_found == WICED_TRUE )
    {
        *address = resolved_ipv4_address;
        return WICED_SUCCESS;
    }
    else if ( ipv6_address_found == WICED_TRUE )
    {
        *address = resolved_ipv6_address;
        return WICED_SUCCESS;
    }

    return WICED_ERROR;
}

/*********************************
 * DNS packet creation functions
 *********************************/
/*
 * Writes the question header using the provided iterator
 * @param iter    Pointer to the iterator where to write the data
 * @param target  The name of the object or service being queried for, e.g. "device_name.local"
 * @param class   The class of the query, e.g. RR_CLASS_IN
 * @param type    The type of the query, e.g. RR_QTYPE_ANY
 */
void dns_write_question(dns_message_iterator_t* iter, const char* target, uint16_t class, uint16_t type)
{
    /* Write target name, type and class */
    iter->iter = dns_write_name(iter->iter, target);
    dns_write_uint16(iter, type);
    dns_write_uint16(iter, class);
}

void dns_write_uint16(dns_message_iterator_t* iter, uint16_t data)
{
    /* We cannot assume the uint8_t alignment of iter->iter so we can't just type cast and assign */
    iter->iter[0] = (uint8_t) ( data >> 8 );
    iter->iter[1] = (uint8_t) ( data & 0xFF );
    iter->iter += 2;
}

void dns_write_uint32(dns_message_iterator_t* iter, uint32_t data)
{
    iter->iter[0] = (uint8_t) ( data >> 24 );
    iter->iter[1] = (uint8_t) ( data >> 16 );
    iter->iter[2] = (uint8_t) ( data >>  8 );
    iter->iter[3] = (uint8_t) ( data & 0xFF);
    iter->iter += 4;
}

void dns_write_bytes(dns_message_iterator_t* iter, const uint8_t* data, uint16_t length)
{
    int a;
    for (a = 0; a < length; ++a)
    {
        iter->iter[a] = data[a];
    }
    iter->iter += length;
}

/*
 * DNS packet processing functions
 */
void dns_get_next_question(dns_message_iterator_t* iter, dns_question_t* q, dns_name_t* name)
{
    /* Set the name pointers and then skip it */
    name->start_of_name   = (uint8_t*) iter->iter;
    name->start_of_packet = (uint8_t*) iter->header;
    dns_skip_name(iter);

    /* Read the type and class */
    q->type  = dns_read_uint16(&iter->iter[0]);
    q->class = dns_read_uint16(&iter->iter[2]);
    iter->iter += 4;
}

void dns_get_next_record(dns_message_iterator_t* iter, dns_record_t* r, dns_name_t* name)
{
    /* Set the name pointers and then skip it */
    name->start_of_name   = (uint8_t*) iter->iter;
    name->start_of_packet = (uint8_t*) iter->header;
    dns_skip_name(iter);

    /* Set the record values and the rdata pointer */
    r->type      = dns_read_uint16(&iter->iter[0]);
    r->class     = dns_read_uint16(&iter->iter[2]);
    r->ttl       = dns_read_uint32(&iter->iter[4]);
    r->rd_length = dns_read_uint16(&iter->iter[8]);
    iter->iter += 10;
    r->rdata     = iter->iter;

    /* Skip the rdata */
    iter->iter += r->rd_length;
}

#if 0 /* unreferenced */
void dns_read_bytes(dns_message_iterator_t* iter, uint8_t* dest, uint16_t length)
{
    memcpy(iter->iter, dest, length);
    iter->iter += length;
}
#endif /* if 0 unreferenced */


wiced_bool_t dns_compare_name_to_string( const dns_name_t* name, const char* string )
{
    wiced_bool_t finished = WICED_FALSE;
    wiced_bool_t result = WICED_TRUE;
    uint8_t* buffer = name->start_of_name;

    while (!finished)
    {
        uint8_t sectionLength;
        int a;

        /* Check if the name is compressed. If so, find the uncompressed version */
        while (*buffer & 0xC0)
        {
            uint16_t offset = (uint16_t) ( (*buffer++) << 8 );
            offset = (uint16_t) ( offset + *buffer );
            offset &= 0x3FFF;
            buffer = name->start_of_packet + offset;
        }

        /* Compare section */
        sectionLength = *(buffer++);
        for (a=sectionLength; a != 0; --a)
        {
            uint8_t b = *buffer++;
            char c = *string++;
            if (b >= 'a' && b <= 'z')
            {
                b = (uint8_t) ( b - ('a' - 'A') );
            }
            if (c >= 'a' && c <= 'z')
            {
                c = (char) ( c - ('a' - 'A') );
            }
            if (b != c)
            {
                result   = WICED_FALSE;
                finished = WICED_TRUE;
                break;
            }
        }

        /* Check if we've finished comparing */
        if (finished == WICED_FALSE && (*buffer == 0 || *string == 0))
        {
            finished = WICED_TRUE;
            /* Check if one of the strings has more data */
            if (*buffer != 0 || *string != 0)
            {
                result = WICED_FALSE;
            }
        }
        else
        {
            /* Skip '.' */
            ++string;
        }
    }

    return result;
}

void dns_skip_name(dns_message_iterator_t* iter)
{
    while (*iter->iter != 0)
    {
        /* Check if the name is compressed */
        if (*iter->iter & 0xC0)
        {
            iter->iter += 1; /* Normally this should be 2, but we have a ++ outside the while loop */
            break;
        }
        else
        {
            iter->iter += (uint32_t) *iter->iter + 1;
        }
    }
    /* Skip the null byte */
    ++iter->iter;
}

char* dns_read_name( const uint8_t* data, const dns_message_header_t* start_of_packet )
{
    uint16_t       length = 0;
    const uint8_t* buffer = data;
    char*          string;
    char*          stringIter;

    /* Find out how long the string is */
    while (*buffer != 0)
    {
        /* Check if the name is compressed */
        if (*buffer & 0xC0)
        {
            uint16_t offset = (uint16_t) ( (*buffer++) << 8 );
            offset = (uint16_t) ( offset + *buffer );
            offset &= 0x3FFF;
            buffer = (uint8_t*) start_of_packet + offset;
        }
        else
        {
            length = (uint16_t) ( length + *buffer + 1 ); /* +1 for the '.', unless its the last section in which case its the ending null character */
            buffer += *buffer + 1;
        }
    }
    if ( length == 0 )
    {
        return NULL;
    }

    /* Allocate memory for the string */
    string = malloc_named("dns", length);
    stringIter = string;

    buffer = data;
    while (*buffer != 0)
    {
        uint8_t sectionLength;

        /* Check if the name is compressed. If so, find the uncompressed version */
        if (*buffer & 0xC0)
        {
            /* Follow the offsets to an uncompressed name */
            while (*buffer & 0xC0)
            {
                uint16_t offset = (uint16_t) ( (*buffer++) << 8 );
                offset = (uint16_t) ( offset + *buffer );
                offset &= 0x3FFF;
                buffer = (uint8_t*) start_of_packet + offset;
            }
        }

        /* Copy the section of the name */
        sectionLength = *(buffer++);
        memcpy(stringIter, buffer, sectionLength);
        stringIter += sectionLength;
        buffer += sectionLength;

        /* Add a '.' if the next section is valid, otherwise terminate the string */
        if (*buffer != 0)
        {
            *stringIter++ = '.';
        }
        else
        {
            *stringIter = 0;
        }
    }

    return string;
}

void dns_write_header(dns_message_iterator_t* iter, uint16_t id, uint16_t flags, uint16_t question_count, uint16_t answer_count, uint16_t authoritative_count, uint16_t additional_count)
{
    memset(iter->header, 0, sizeof(dns_message_header_t));
    iter->header->id                         = htons(id);
    iter->header->flags                      = htons(flags);
    iter->header->question_count             = htons(question_count);
    iter->header->authoritative_answer_count = htons(authoritative_count);
    iter->header->answer_count               = htons(answer_count);
    iter->header->additional_record_count    = htons(additional_count);
}

void dns_write_record(dns_message_iterator_t* iter, const char* name, uint16_t class, uint16_t type, uint32_t TTL, const void* rdata)
{
    uint8_t* rd_length;
    uint8_t* tempPtr;

    /* Write the name, type, class, TTL */
    iter->iter = dns_write_name(iter->iter, name);
    dns_write_uint16(iter, type);
    dns_write_uint16(iter, class);
    dns_write_uint32(iter, TTL);

    /* Keep track of where we store the rdata length */
    rd_length   = iter->iter;
    iter->iter += 2;
    tempPtr    = iter->iter;

    switch (type)
    {
        case RR_TYPE_A:
            dns_write_bytes(iter, rdata, 4);
            break;

        case RR_TYPE_PTR:
            iter->iter = dns_write_name(iter->iter, (const char*) rdata);
            break;

        case RR_TYPE_TXT:
            if (rdata != NULL)
            {
                iter->iter = dns_write_string(iter->iter, rdata);
            }
            else
            {
                *iter->iter++ = 0;
            }
            break;

        case RR_TYPE_SRV:
        {
            /* Copy priority, weight, and port */
            memcpy(iter->iter, rdata, 6);
            iter->iter += 6;

            /* Write the hostname */
            iter->iter = dns_write_name( iter->iter, ((dns_srv_data_t*)rdata)->target );
            break;
        }

        case RR_TYPE_HINFO:
            iter->iter = dns_write_string(iter->iter, ((dns_hinfo_t*) rdata)->CPU);
            iter->iter = dns_write_string(iter->iter, ((dns_hinfo_t*) rdata)->OS);
            break;

        case RR_TYPE_NSEC:
        {
            dns_nsec_data_t* nsec_data = (dns_nsec_data_t*)rdata;
            iter->iter = dns_write_name(iter->iter, name);
            dns_write_bytes(iter, &nsec_data->block_number,  (uint16_t) ( 2 + nsec_data->bitmap_size ) );
        }
            break;

        default:
            break;
    }

    /* Write the rdata length */
    rd_length[0] = (uint8_t) ( (iter->iter - tempPtr) >> 8 );
    rd_length[1] = (uint8_t) ( (iter->iter - tempPtr) & 0xFF );
}

uint8_t* dns_write_name(uint8_t* dest, const char* src)
{
    dest = dns_write_string(dest, src);

    /* Add the ending null */
    *dest++ = 0;

    return dest;
}

uint8_t* dns_write_string(uint8_t* dest, const char* src)
{
    uint8_t* segmentLengthPointer;
    uint8_t segmentLength;

    while (*src != 0)
    {
        /* Remember where we need to store the segment length and reset the counter */
        segmentLengthPointer = dest++;
        segmentLength = 0;

        /* Copy bytes until '.' or end of string */
        while (*src != '.' && *src != 0)
        {
            *dest++ = (uint8_t) *src++;
            ++segmentLength;
        }

        /* Store the length of the segment*/
        *segmentLengthPointer = segmentLength;

        /* Check if we stopped because of a '.', if so, skip it */
        if (*src == '.')
        {
            ++src;
        }
    }

    return dest;
}

uint16_t dns_read_uint16( const uint8_t* data )
{
    return htons(*((uint16_t*)data));
}

uint32_t dns_read_uint32( const uint8_t* data )
{
    return htonl(*((uint32_t*)data));
}

