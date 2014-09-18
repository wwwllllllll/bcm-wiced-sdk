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
 *  Allows thread safe access to the WICED WiFi Driver (WWD) hardware bus
 *
 *  This file provides functions which allow multiple threads to use the WWD hardware bus (SDIO or SPI)
 *  This is achieved by having a single thread (the "WWD Thread") which queues messages to be sent, sending
 *  them sequentially, as well as receiving messages as they arrive.
 *
 *  Messages to be sent come from the @ref wwd_sdpcm_send_common function in wwd_sdpcm.c .  The messages already
 *  contain SDPCM headers, but not any bus headers (GSPI), and are passed to the wwd_thread_send_data function.
 *  This function can be called from any thread.
 *
 *  Messages are received by way of a callback supplied by in SDPCM.c - wwd_sdpcm_process_rx_packet
 *  Received messages are delivered in the context of the WWD Thread, so the callback function needs to avoid blocking.
 *
 *  It is also possible to use these functions without any operating system, by periodically calling the wwd_thread_send_one_packet,
 *  @ref wwd_thread_receive_one_packet or @ref wwd_thread_poll_all functions
 *
 */

#include <string.h>
#include "wwd_assert.h"
#include "wwd_logging.h"
#include "wwd_poll.h"
#include "wwd_rtos.h"
#include "RTOS/wwd_rtos_interface.h"
#include "network/wwd_buffer_interface.h"
#include "network/wwd_network_interface.h"
#include "platform/wwd_bus_interface.h"
#include "internal/wwd_thread.h"
#include "internal/wwd_sdpcm.h"
#include "internal/wwd_internal.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"

#define WWD_THREAD_POLL_TIMEOUT      (NEVER_TIMEOUT)

#ifdef RTOS_USE_STATIC_THREAD_STACK
static uint8_t wwd_thread_stack[WWD_THREAD_STACK_SIZE];
#define WWD_THREAD_STACK     wwd_thread_stack
#else
#ifdef RTOS_USE_DYNAMIC_THREAD_STACK
#define WWD_THREAD_STACK     NULL
#else
#error RTOS_USE_STATIC_THREAD_STACK or RTOS_USE_DYNAMIC_THREAD_STACK must be defined
#endif
#endif


/******************************************************
 *             Static Variables
 ******************************************************/

static wiced_bool_t          wwd_thread_quit_flag = WICED_FALSE;
static wiced_bool_t          wwd_inited           = WICED_FALSE;
static host_thread_type_t    wwd_thread;
static host_semaphore_type_t wwd_transceive_semaphore;
static wiced_bool_t          wwd_bus_interrupt = WICED_FALSE;

/******************************************************
 *             Static Function Prototypes
 ******************************************************/

static void wwd_thread_func( uint32_t /*@unused@*/thread_input ) /*@globals killed wwd_transceive_semaphore@*/ /*@modifies wwd_wlan_status, wwd_bus_interrupt, wwd_thread_quit_flag@*/;

/******************************************************
 *             Global Functions
 ******************************************************/


/** Initialises the WWD Thread
 *
 * Initialises the WWD thread, and its flags/semaphores,
 * then starts it running
 *
 * @return    WWD_SUCCESS : if initialisation succeeds
 *            otherwise, a result code
 */
wwd_result_t wwd_thread_init( void ) /*@globals undef wwd_thread, undef wwd_transceive_semaphore@*/ /*@modifies wwd_inited@*/
{
    wwd_result_t retval;

    retval = wwd_sdpcm_init( );

    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not initialize SDPCM codec\n"));
        /*@-unreachable@*/ /*@-globstate@*/ /* Lint: Reachable after hitting assert & globals not defined due to error */
        return retval;
        /*@+unreachable@*/ /*@+globstate@*/
    }

    /* Create the event flag which signals the WWD thread needs to wake up */
    retval = host_rtos_init_semaphore( &wwd_transceive_semaphore );
    if ( retval != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not initialize WWD thread semaphore\n"));
        /*@-unreachable@*/ /*@-globstate@*/ /* Lint: Reachable after hitting assert & globals not defined due to error */
        return retval;
        /*@+unreachable@*/ /*@+globstate@*/
    }

    retval = host_rtos_create_thread( &wwd_thread, wwd_thread_func, "WWD", WWD_THREAD_STACK, (uint32_t) WWD_THREAD_STACK_SIZE, (uint32_t) WWD_THREAD_PRIORITY );
    if ( retval != WWD_SUCCESS )
    {
        /* Could not start WWD main thread */
        WPRINT_WWD_ERROR(("Could not start WWD thread\n"));
        /*@-unreachable@*/ /* Reachable after hitting assert */
        return retval;
        /*@+unreachable@*/
    }

    wwd_inited = WICED_TRUE;
    return WWD_SUCCESS;
}

/** Sends the first queued packet
 *
 * Checks the queue to determine if there is any packets waiting
 * to be sent. If there are, then it sends the first one.
 *
 * This function is normally used by the WWD Thread, but can be
 * called periodically by systems which have no RTOS to ensure
 * packets get sent.
 *
 * @return    1 : packet was sent
 *            0 : no packet sent
 */
int8_t wwd_thread_send_one_packet( void ) /*@modifies internalState @*/
{
    wiced_buffer_t tmp_buf_hnd = NULL;

    if ( wwd_sdpcm_get_packet_to_send( &tmp_buf_hnd ) != WWD_SUCCESS )
    {
        /*@-mustfreeonly@*/ /* Failed to get a packet */
        return 0;
        /*@+mustfreeonly@*/
    }

    /* Ensure the wlan backplane bus is up */
    if ( wwd_bus_ensure_is_up() != WWD_SUCCESS )
    {
        wiced_assert("Could not bring bus back up", 0 != 0 );
        host_buffer_release( tmp_buf_hnd, WWD_NETWORK_TX );
        return 0;
    }

    WPRINT_WWD_DEBUG(("Wcd:> Sending pkt 0x%08X\n\r", (unsigned int)tmp_buf_hnd ));
    if ( wwd_bus_send_buffer( tmp_buf_hnd ) != WWD_SUCCESS )
    {
        return 0;
    }
    return (int8_t) 1;
}

/** Receives a packet if one is waiting
 *
 * Checks the wifi chip fifo to determine if there is any packets waiting
 * to be received. If there are, then it receives the first one, and calls
 * the callback @ref wwd_sdpcm_process_rx_packet (in wwd_sdpcm.c).
 *
 * This function is normally used by the WWD Thread, but can be
 * called periodically by systems which have no RTOS to ensure
 * packets get received properly.
 *
 * @return    1 : packet was received
 *            0 : no packet waiting
 */
int8_t wwd_thread_receive_one_packet( void )
{
    /* Check if there is a packet ready to be received */
    wiced_buffer_t recv_buffer;
    if ( wwd_bus_read_frame( &recv_buffer ) != WWD_SUCCESS)
    {
        /*@-mustfreeonly@*/ /* Failed to read a packet */
        return 0;
        /*@+mustfreeonly@*/
    }

    if ( recv_buffer != NULL )  /* Could be null if it was only a credit update */
    {

        WWD_LOG(("Wcd:< Rcvd pkt 0x%08X\n", (unsigned int)recv_buffer ));

        /* Send received buffer up to SDPCM layer */
        wwd_sdpcm_process_rx_packet( recv_buffer );
    }
    return (int8_t) 1;
}

/** Sends and Receives all waiting packets
 *
 * Repeatedly calls wwd_thread_send_one_packet and wwd_thread_receive_one_packet
 * to send and receive packets, until there are no more packets waiting to
 * be transferred.
 *
 * This function is normally used by the WWD Thread, but can be
 * called periodically by systems which have no RTOS to ensure
 * packets get send and received properly.
 *
 */
int8_t wwd_thread_poll_all( void ) /*@modifies internalState@*/
{
    int8_t result = 0;
    result |= wwd_thread_send_one_packet( ) ;
    result |= wwd_thread_receive_one_packet( );
    return result;
}

/** Terminates the WWD Thread
 *
 * Sets a flag then wakes the WWD Thread to force it to terminate.
 *
 */
void wwd_thread_quit( void )
{
    wwd_result_t result;

    /* signal main thread and wake it */
    wwd_thread_quit_flag = WICED_TRUE;
    result = host_rtos_set_semaphore( &wwd_transceive_semaphore, WICED_FALSE );

    if ( result == WWD_SUCCESS )
    {
        /* Wait for the WWD thread to end */
        host_rtos_join_thread( &wwd_thread );

        (void) host_rtos_delete_terminated_thread( &wwd_thread ); /* Ignore return - not much can be done about failure */
    }
}

/**
 * Informs WWD of an interrupt
 *
 * This function should be called from the SDIO/SPI interrupt function
 * and usually indicates newly received data is available.
 * It wakes the WWD Thread, forcing it to check the send/receive
 *
 */
void wwd_thread_notify_irq( void )
{
    wwd_bus_interrupt = WICED_TRUE;

    /* just wake up the main thread and let it deal with the data */
    if ( wwd_inited == WICED_TRUE )
    {
        (void) host_rtos_set_semaphore( &wwd_transceive_semaphore, WICED_TRUE ); /* ignore failure since there is nothing that can be done about it in a ISR */
    }
}

void wwd_thread_notify( void )
{
    /* just wake up the main thread and let it deal with the data */
    if ( wwd_inited == WICED_TRUE )
    {
        (void) host_rtos_set_semaphore( &wwd_transceive_semaphore, WICED_FALSE ); /* Ignore return - not much can be done about failure */
    }
}

/******************************************************
 *             Static Functions
 ******************************************************/

/** The WWD Thread function
 *
 *  This is the main loop of the WWD Thread.
 *  It simply calls @ref wwd_thread_poll_all to send/receive all waiting packets, then goes
 *  to sleep.  The sleep has a 100ms timeout, causing the send/receive queues to be
 *  checked 10 times per second in case an interrupt is missed.
 *  Once the quit flag has been set, flags/mutexes are cleaned up, and the function exits.
 *
 * @param thread_input  : unused parameter needed to match thread prototype.
 *
 */
static void wwd_thread_func( uint32_t /*@unused@*/thread_input ) /*@globals killed wwd_transceive_semaphore@*/ /*@modifies wwd_wlan_status, wwd_bus_interrupt, wwd_thread_quit_flag, wwd_inited, wwd_thread@*/
{
    int8_t       rx_status;
    int8_t       tx_status;
    wwd_result_t result;

    UNUSED_PARAMETER(thread_input);

    while ( wwd_thread_quit_flag != WICED_TRUE )
    {
        /* Check if we were woken by interrupt */
        if ( ( wwd_bus_interrupt == WICED_TRUE ) ||
             ( WWD_BUS_USE_STATUS_REPORT_SCHEME ) )
        {
            wwd_bus_interrupt = WICED_FALSE;

            /* Check if the interrupt indicated there is a packet to read */
            if ( wwd_bus_packet_available_to_read( ) != 0)
            {
                /* Receive all available packets */
                do
                {
                    rx_status = wwd_thread_receive_one_packet( );
                } while ( rx_status != 0 );
            }
        }

        /* Send all queued packets */
        do
        {
            tx_status = wwd_thread_send_one_packet( );
        }
        while (tx_status != 0);

        /* Check if we have run out of bus credits */
        if ( wWd_sdpcm_get_available_credits( ) == 0 )
        {
            /* Keep poking the WLAN until it gives us more credits */
            result = wwd_bus_poke_wlan( );
            wiced_assert( "Poking failed!", result == WWD_SUCCESS );
            result = host_rtos_get_semaphore( &wwd_transceive_semaphore, (uint32_t) 100, WICED_FALSE );
        }
        else
        {
            /* Put the bus to sleep and wait for something else to do */
            if ( wwd_wlan_status.keep_wlan_awake == 0 )
            {
                result = wwd_bus_allow_wlan_bus_to_sleep( );
                wiced_assert( "Error setting wlan sleep", result == WWD_SUCCESS );
            }

            result = host_rtos_get_semaphore( &wwd_transceive_semaphore, (uint32_t) WWD_THREAD_POLL_TIMEOUT, WICED_FALSE );
        }
        REFERENCE_DEBUG_ONLY_VARIABLE(result);
        wiced_assert("Could not get wwd sleep semaphore\n", (result == WWD_SUCCESS)||(result == WWD_TIMEOUT) );
    }

    /* Reset the quit flag */
    wwd_thread_quit_flag = WICED_FALSE;

    /* Delete the semaphore */
    (void) host_rtos_deinit_semaphore( &wwd_transceive_semaphore );  /* Ignore return - not much can be done about failure */

    wwd_sdpcm_quit( );
    wwd_inited = WICED_FALSE;

    if ( WWD_SUCCESS != host_rtos_finish_thread( &wwd_thread ) )
    {
        WPRINT_WWD_DEBUG(("Could not close WWD thread\n"));
    }
}

