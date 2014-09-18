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
#include <stdint.h>
#include <string.h>
#include "platform_config.h"
#include "platform_peripheral.h"
#include "platform_sleep.h"
#include "platform_assert.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define RAM_START_ADDR                   (IRAM_ADDR)
#define RAM_END_ADDR    (RAM_START_ADDR + IRAM_SIZE)

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

void platform_uart_irq( platform_uart_driver_t* driver );

/******************************************************
 *               Variable Definitions
 ******************************************************/
int platform_uarts_irq_numbers[] =
{
    [UART_0]  = UART0_IRQn,
    [UART_1]  = UART1_IRQn,
    [USART_0] = USART0_IRQn,
    [USART_1] = USART1_IRQn,
};


/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_uart_init( platform_uart_driver_t* driver, const platform_uart_t* peripheral, const platform_uart_config_t* config, wiced_ring_buffer_t* optional_ring_buffer )
{
    sam_usart_opt_t    settings;

    UNUSED_PARAMETER(driver);
    UNUSED_PARAMETER(peripheral);
    UNUSED_PARAMETER(config);
    UNUSED_PARAMETER(optional_ring_buffer);

    pdc_packet_t dma_packet;

    if ( config->flow_control != FLOW_CONTROL_DISABLED )
    {
        return WICED_UNSUPPORTED;
    }

    memset( &settings, 0, sizeof( settings ) );

    switch ( config->data_width )
    {
        case DATA_WIDTH_5BIT:
            settings.char_length = US_MR_CHRL_5_BIT;
            break;
        case DATA_WIDTH_6BIT:
            settings.char_length = US_MR_CHRL_6_BIT;
            break;
        case DATA_WIDTH_7BIT:
            settings.char_length = US_MR_CHRL_7_BIT;
            break;
        case DATA_WIDTH_8BIT:
            settings.char_length = US_MR_CHRL_8_BIT;
            break;
        case DATA_WIDTH_9BIT:
        default:
            return WICED_UNSUPPORTED;
    }

    switch ( config->parity )
    {
        case ODD_PARITY:
            settings.parity_type = US_MR_PAR_ODD;
            break;
        case EVEN_PARITY:
            settings.parity_type = US_MR_PAR_EVEN;
            break;
        case NO_PARITY:
            settings.parity_type = US_MR_PAR_NO;
            break;
        default:
            break;
    }

    switch ( config->stop_bits )
    {
        case STOP_BITS_1:
            settings.stop_bits = US_MR_NBSTOP_1_BIT;
            break;
        case STOP_BITS_2:
            settings.stop_bits = US_MR_NBSTOP_2_BIT;
            break;
        default:
            break;
    }

    settings.baudrate     = config->baud_rate;
    settings.channel_mode = US_MR_CHMODE_NORMAL;


    driver->peripheral = peripheral;

//    /* Initialise TX and RX complete semaphores */
    host_rtos_init_semaphore( &driver->tx_dma_complete );
    host_rtos_init_semaphore( &driver->rx_dma_complete );

    /* Set Tx and Rx pin mode to UART peripheral */
    platform_gpio_peripheral_pin_init( peripheral->tx_pin, ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP ) );
    platform_gpio_peripheral_pin_init( peripheral->rx_pin, ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP ) );

    /* Init CTS and RTS pins (if available) */
    if ( peripheral->cts_pin != NULL )
    {
        platform_gpio_peripheral_pin_init( peripheral->cts_pin, ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP ) );
    }

    if ( peripheral->rts_pin != NULL )
    {
        platform_gpio_peripheral_pin_init( peripheral->rts_pin, ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP ) );
    }

    /* Enable the peripheral clock in the PMC. */
    sysclk_enable_peripheral_clock( peripheral->peripheral_id );

    /* Enable the receiver and transmitter. */
    usart_reset_tx( peripheral->peripheral );
    usart_reset_rx( peripheral->peripheral );

    /* Configure USART in serial mode. */
    usart_init_rs232( peripheral->peripheral, &settings, CPU_CLOCK_HZ );

    /* Disable all the interrupts. */
    usart_disable_interrupt( peripheral->peripheral, 0xffffffff );

    /* Enable uart interrupt */
    NVIC_SetPriority( platform_uarts_irq_numbers[peripheral->uart_id], 0x06 );
    NVIC_EnableIRQ( platform_uarts_irq_numbers[peripheral->uart_id] );

    /* Enable PDC transmit */
    pdc_enable_transfer( usart_get_pdc_base( peripheral->peripheral ), PERIPH_PTCR_TXTEN | PERIPH_PTCR_RXTEN );

    driver->rx_ring_buffer = optional_ring_buffer;

    dma_packet.ul_addr = (uint32_t)driver->rx_ring_buffer->buffer;
    dma_packet.ul_size = (uint32_t)driver->rx_ring_buffer->size;
    pdc_rx_init( usart_get_pdc_base( peripheral->peripheral ), &dma_packet, &dma_packet );


    usart_enable_interrupt( peripheral->peripheral, US_IER_ENDRX | US_IER_RXBUFF | US_IER_RXRDY | US_IER_ENDTX );

    /* Enable the receiver and transmitter. */
    usart_enable_tx( peripheral->peripheral );
    usart_enable_rx( peripheral->peripheral );

    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_deinit( platform_uart_driver_t* driver )
{

    usart_disable_interrupt( driver->peripheral->peripheral, 0xffffffff );

    NVIC_DisableIRQ( platform_uarts_irq_numbers[driver->peripheral->uart_id] );

    pdc_disable_transfer( usart_get_pdc_base( driver->peripheral->peripheral ), PERIPH_PTCR_TXTDIS | PERIPH_PTCR_RXTDIS );

    usart_disable_tx( driver->peripheral->peripheral );
    usart_disable_rx( driver->peripheral->peripheral );

    sysclk_disable_peripheral_clock( driver->peripheral->peripheral_id );

    platform_gpio_deinit( driver->peripheral->tx_pin );
    platform_gpio_deinit( driver->peripheral->rx_pin );

    if ( driver->peripheral->cts_pin != NULL )
    {
        platform_gpio_deinit( driver->peripheral->cts_pin );
    }

    if ( driver->peripheral->rts_pin != NULL )
    {
        platform_gpio_deinit( driver->peripheral->rts_pin );
    }

    host_rtos_deinit_semaphore( &driver->tx_dma_complete );
    host_rtos_deinit_semaphore( &driver->rx_dma_complete );

    driver->peripheral = NULL;
    memset( driver, 0, sizeof(platform_uart_driver_t) );

    return WICED_SUCCESS;
}

platform_result_t platform_uart_transmit_bytes( platform_uart_driver_t* driver, const uint8_t* data_out, uint32_t size )
{
    UNUSED_PARAMETER(driver);
    UNUSED_PARAMETER(data_out);
    UNUSED_PARAMETER(size);
    pdc_packet_t dma_packet;

    /* Limitation: SAM4S doesn't support DMA transfer from embedded flash.
     * If data_out address is not within RAM range, use normal write to THR.
     */
    if ( data_out >= (const uint8_t*)RAM_START_ADDR && data_out < (const uint8_t*)RAM_END_ADDR )
    {
        /* Initialise TPR and TCR register values. TNPR and TNCR are unused */
        dma_packet.ul_addr = (uint32_t)data_out;
        dma_packet.ul_size = (uint32_t)size;
        pdc_tx_init( usart_get_pdc_base( driver->peripheral->peripheral ), &dma_packet, NULL );

        /* Enable Tx DMA transmission */
        pdc_enable_transfer( usart_get_pdc_base( driver->peripheral->peripheral ), PERIPH_PTCR_TXTEN );

        host_rtos_get_semaphore( &driver->tx_dma_complete, NEVER_TIMEOUT, WICED_FALSE );
    }
    else
    {
        while ( size > 0 )
        {
            usart_putchar( driver->peripheral->peripheral, (uint32_t)*data_out++ );
            size--;
        }
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_uart_receive_bytes( platform_uart_driver_t* driver, uint8_t* data_in, uint32_t expected_data_size, uint32_t timeout_ms )
{
    UNUSED_PARAMETER(driver);
    UNUSED_PARAMETER(data_in);
    UNUSED_PARAMETER(expected_data_size);
    UNUSED_PARAMETER(timeout_ms);

    if ( driver->rx_ring_buffer != NULL )
    {
        while ( expected_data_size != 0 )
        {
            uint32_t transfer_size = MIN(driver->rx_ring_buffer->size / 2, expected_data_size);

            /* Check if ring buffer already contains the required amount of data. */
            if ( transfer_size > ring_buffer_used_space( driver->rx_ring_buffer ) )
            {
                /* Set rx_size and wait in rx_complete semaphore until data reaches rx_size or timeout occurs */
                driver->rx_transfer_size = transfer_size;

                if ( host_rtos_get_semaphore( &driver->rx_dma_complete, timeout_ms, WICED_FALSE ) != WWD_SUCCESS )
                {
                    driver->rx_transfer_size = 0;
                    return PLATFORM_TIMEOUT;
                }

                /* Reset rx_size to prevent semaphore being set while nothing waits for the data */
                driver->rx_transfer_size = 0;
            }

            expected_data_size -= transfer_size;

            // Grab data from the buffer
            do
            {
                uint8_t* available_data;
                uint32_t bytes_available;

                ring_buffer_get_data( driver->rx_ring_buffer, &available_data, &bytes_available );
                bytes_available = MIN( bytes_available, transfer_size );
                memcpy( data_in, available_data, bytes_available );
                transfer_size -= bytes_available;
                data_in = ( (uint8_t*)data_in + bytes_available );
                ring_buffer_consume( driver->rx_ring_buffer, bytes_available );
            }
            while ( transfer_size != 0 );
        }

        if ( expected_data_size != 0 )
        {
            return PLATFORM_ERROR;
        }
        else
        {
            return PLATFORM_SUCCESS;
        }
    }
    else
    {
        /* TODO: need to implement this */
        return PLATFORM_UNSUPPORTED;
    }
}

void platform_uart_irq( platform_uart_driver_t* driver )
{
    uint32_t status = usart_get_status( driver->peripheral->peripheral );
    uint32_t mask = usart_get_interrupt_mask( driver->peripheral->peripheral );
    Pdc* pdc_register = usart_get_pdc_base( driver->peripheral->peripheral );

    /* ENDTX flag is set when Tx DMA transfer is done
     */
    if ( ( mask & US_IMR_ENDTX ) && ( status & US_CSR_ENDTX ) )
    {
        pdc_packet_t dma_packet;

        /* ENDTX is cleared when TCR or TNCR is set to a non-zero value, which effectively
         * starts another Tx DMA transaction. To work around this, disable Tx before
         * performing a dummy Tx init.
         */
        pdc_disable_transfer( usart_get_pdc_base( driver->peripheral->peripheral ), PERIPH_PTCR_TXTDIS );

        dma_packet.ul_addr = (uint32_t)0;
        dma_packet.ul_size = (uint32_t)1;

        pdc_tx_init( usart_get_pdc_base( USART1 ), &dma_packet, NULL );

        /* Notifies waiting thread that Tx DMA transfer is complete */
        host_rtos_set_semaphore( &driver->tx_dma_complete, WICED_TRUE );
    }

    /* ENDRX flag is set when RCR is 0. RNPR and RNCR values are then copied into
     * RPR and RCR, respectively, while the Tx tranfer continues. We now need to
     * prepare RNPR and RNCR for the next iteration.
     */
    if ( ( mask & US_IMR_ENDRX ) && ( status & US_CSR_ENDRX ) )
    {
        pdc_register->PERIPH_RNPR = (uint32_t)driver->rx_ring_buffer->buffer;
        pdc_register->PERIPH_RNCR = (uint32_t)driver->rx_ring_buffer->size;
    }

    /* RXRDY interrupt is triggered and flag is set when a new character has been
     * received but not yet read from the US_RHR. When this interrupt executes,
     * the DMA engine already read the character out from the US_RHR and RXRDY flag
     * is no longer asserted. The code below updates the ring buffer parameters
     * to keep them current
     */
    if ( mask & US_CSR_RXRDY )
    {
        driver->rx_ring_buffer->tail = driver->rx_ring_buffer->size - pdc_register->PERIPH_RCR;

        // Notify thread if sufficient data are available
        if ( ( driver->rx_transfer_size > 0 ) && ( ring_buffer_used_space( driver->rx_ring_buffer ) >= driver->rx_transfer_size ) )
        {
            host_rtos_set_semaphore( &driver->rx_dma_complete, WICED_TRUE );
            driver->rx_transfer_size = 0;
        }
    }
}

