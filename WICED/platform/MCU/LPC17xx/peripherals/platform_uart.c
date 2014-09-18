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
 * LPC43xx UART implementation
 */
#include <stdint.h>
#include <string.h>
#include "platform_config.h"
#include "platform_peripheral.h"
#include "platform_isr_interface.h"
#include "platform_sleep.h"
#include "platform_assert.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    IP_UART_DATABIT_T databits;
    IP_UART_PARITY_T  parity;
    IP_UART_STOPBIT_T stopbits;
}config_uart_data_t;

/******************************************************
 *                    Variables
 ******************************************************/

/* UART interrupt vectors */
static const IRQn_Type uart_irq_vectors[NUMBER_OF_UART_PORTS] =
{
    [0] = UART0_IRQn,
    [1] = UART1_IRQn,
    [2] = UART2_IRQn,
    [3] = UART3_IRQn,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_uart_init( platform_uart_driver_t* driver, const platform_uart_t* interface, const platform_uart_config_t* config, wiced_ring_buffer_t* optional_ring_buffer )
{
    uint32_t           uart_number;
    UART_FIFO_CFG_T    UARTFIFOConfigStruct;
    config_uart_data_t config_uart_data;

    wiced_assert( "bad argument", ( driver != NULL ) && ( interface != NULL ) && ( config != NULL ) );
    Chip_Clock_EnablePeriphClock( SYSCTL_CLOCK_UART0 );

    uart_number = platform_uart_get_port_number(interface->uart_base);
    driver->rx_size              = 0;
    driver->tx_size              = 0;
    driver->last_transmit_result = PLATFORM_SUCCESS;
    driver->last_receive_result  = PLATFORM_SUCCESS;
    driver->interface            = (platform_uart_t*)interface;
    host_rtos_init_semaphore( &driver->tx_complete );
    host_rtos_init_semaphore( &driver->rx_complete );

    platform_gpio_set_alternate_function( &driver->interface->tx_pin);
    platform_gpio_set_alternate_function( &driver->interface->rx_pin);

    /* Initialise USART peripheral */
    Chip_UART_FIFOConfigStructInit( driver->interface->uart_base, &UARTFIFOConfigStruct );
    Chip_UART_Init( driver->interface->uart_base );
    Chip_UART_SetBaud( driver->interface->uart_base, config->baud_rate );
    config_uart_data.databits = ( ( config->data_width == DATA_WIDTH_8BIT ) || ( ( config->data_width == DATA_WIDTH_7BIT ) && ( config->parity != NO_PARITY ) ) ) ? UART_DATABIT_8 : UART_DATABIT_7;
    config_uart_data.stopbits = ( config->stop_bits == STOP_BITS_1 ) ? UART_STOPBIT_1 : UART_STOPBIT_2;
    switch ( config->parity )
    {
        case NO_PARITY:
            config_uart_data.parity = UART_PARITY_NONE;
            break;

        case EVEN_PARITY:
            config_uart_data.parity = UART_PARITY_EVEN;
            break;

        case ODD_PARITY:
            config_uart_data.parity = UART_PARITY_ODD;
            break;

        default:
            return WICED_BADARG;
    }

    Chip_UART_ConfigData( driver->interface->uart_base, config_uart_data.databits, config_uart_data.parity, config_uart_data.stopbits );
    Chip_UART_TxCmd( driver->interface->uart_base, ENABLE );
    /* Enable receive data and line status interrupt */
    /* Initialize FIFO for UART0 peripheral */
    Chip_UART_FIFOConfig( driver->interface->uart_base, &UARTFIFOConfigStruct );
    /* Enable UART Rx interrupt */
    Chip_UART_IntConfig( driver->interface->uart_base, UART_INTCFG_RBR, ENABLE );
    /* Enable UART line status interrupt */
    Chip_UART_IntConfig( driver->interface->uart_base, UART_INTCFG_RLS, ENABLE );

    /* Enable Interrupt for UART channel */
    NVIC_DisableIRQ( uart_irq_vectors[ uart_number ] );
    NVIC_ClearPendingIRQ( uart_irq_vectors[ uart_number ] );
    /*Note the LPC uses 5 bits for interrupt priority levels*/
    NVIC_EnableIRQ( uart_irq_vectors[ uart_number ] );
    if ( optional_ring_buffer != NULL )
    {
        /* Note that the ring_buffer should've been initialised first */
        driver->rx_buffer = optional_ring_buffer;
        driver->rx_size = 0;
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_transmit_bytes( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
    Chip_UART_Send(driver->interface->uart_base, (uint8_t *)data_out,(int)size, BLOCKING);

    return WICED_SUCCESS;
}

platform_result_t platform_uart_receive_bytes( platform_uart_driver_t* driver, uint8_t* data_in, uint32_t expected_data_size, uint32_t timeout_ms )
{
    /*The following is a temporary implemenration of the UART*/
    platform_result_t result = PLATFORM_SUCCESS;
    wiced_assert( "bad argument", ( driver != NULL ) && ( data_in != NULL ) && ( expected_data_size != 0 ) );

    if ( driver->rx_buffer != NULL )
    {
        while ( expected_data_size != 0 )
        {
            uint32_t transfer_size = MIN( driver->rx_buffer->size / 2, expected_data_size );

            /* Check if ring buffer already contains the required amount of data. */
            if ( transfer_size > ring_buffer_used_space( driver->rx_buffer ) )
            {
                wwd_result_t wwd_result;

                /* Set rx_size and wait in rx_complete semaphore until data reaches rx_size or timeout occurs */
                WICED_DISABLE_INTERRUPTS( );
                driver->last_receive_result = PLATFORM_SUCCESS;
                driver->rx_size = transfer_size;
                WICED_ENABLE_INTERRUPTS( );

                wwd_result = host_rtos_get_semaphore( &driver->rx_complete, timeout_ms, WICED_TRUE );

                /* Reset rx_size to prevent semaphore being set while nothing waits for the data */
                WICED_DISABLE_INTERRUPTS( );
                driver->rx_size = 0;
                WICED_ENABLE_INTERRUPTS( );

                if ( wwd_result == WWD_TIMEOUT )
                {
                    /* Semaphore timeout. breaks from the while loop */
                    result = PLATFORM_TIMEOUT;
                    break;
                }
                else
                {
                    /* No timeout. retrieve result */
                    result = driver->last_receive_result;
                }
            }

            expected_data_size -= transfer_size;

            // Grab data from the buffer
            do
            {
                uint8_t* available_data;
                uint32_t bytes_available;

                ring_buffer_get_data( driver->rx_buffer, &available_data, &bytes_available );
                bytes_available = MIN( bytes_available, transfer_size );
                memcpy( data_in, available_data, bytes_available );
                transfer_size -= bytes_available;
                data_in = ( (uint8_t*) data_in + bytes_available );
                ring_buffer_consume( driver->rx_buffer, bytes_available );
            } while ( transfer_size != 0 );
        }

        return result;
    }
    else
    {
        return result;
    }
}

uint8_t platform_uart_get_port_number( platform_uart_port_t* uart )
{
    if ( uart == LPC_UART0 )
    {
        return 0;
    }
    else if ( uart == LPC_UART1 )
    {
        return 1;
    }
    else if ( uart == LPC_UART2 )
    {
        return 2;
    }
    else if ( uart == LPC_UART3 )
    {
        return 3;
    }
    else
    {
        return 0xff;
    }
}

/******************************************************
 *            IRQ Handlers Definition
 ******************************************************/

void platform_uart_irq( platform_uart_driver_t* driver )
{
    platform_uart_port_t* uart = (platform_uart_port_t*) driver->interface->uart_base;
    uint8_t data=0;

    while (Chip_UART_GetLineStatus(uart) & UART_LSR_RDR) {
        Chip_UART_ReceiveByte(uart,&data);
        ring_buffer_write( driver->rx_buffer,&data, 1 );
    }

    // Notify thread if sufficient data are available
    if ( ( driver->rx_size > 0 ) && ( ring_buffer_used_space( driver->rx_buffer ) >= driver->rx_size ) )
    {
        host_rtos_set_semaphore( &driver->rx_complete, WICED_TRUE );
        driver->rx_size = 0;
    }
}
