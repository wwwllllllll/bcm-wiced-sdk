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
#include "platform_cmsis.h"
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_isr_interface.h"
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

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *            WICED Function Definitions
 ******************************************************/

platform_result_t platform_spi_init( const platform_spi_t* spi, const platform_spi_config_t* config )
{
    UNUSED_PARAMETER(spi);
    UNUSED_PARAMETER(config);

    platform_mcu_powersave_disable( );

    Pdc* spi_pdc = spi_get_pdc_base( spi->peripheral );

    /* Setup chip select pin */
    platform_gpio_init( config->chip_select, OUTPUT_PUSH_PULL );
    platform_gpio_output_high( config->chip_select );

    /* Setup other pins */
    platform_gpio_peripheral_pin_init( spi->mosi_pin, ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP ) );
    platform_gpio_peripheral_pin_init( spi->miso_pin, ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP ) );
    platform_gpio_peripheral_pin_init( spi->clk_pin,  ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP )  );

    /* Configure an SPI peripheral. */
    pmc_enable_periph_clk( spi->peripheral_id );
    spi_disable( spi->peripheral );
    spi_reset( spi->peripheral );
    spi_set_lastxfer( spi->peripheral );
    spi_set_master_mode( spi->peripheral );
    spi_disable_mode_fault_detect( spi->peripheral );
    spi_set_peripheral_chip_select_value( spi->peripheral, 0 );

    spi_set_clock_polarity( spi->peripheral, 0, ( ( config->mode && SPI_CLOCK_IDLE_HIGH )   ? (1) : (0) ) );
    spi_set_clock_phase( spi->peripheral, 0,    ( ( config->mode && SPI_CLOCK_RISING_EDGE ) ? (1) : (0) ) );

    spi_set_bits_per_transfer( spi->peripheral, 0, SPI_CSR_BITS_8_BIT );
    spi_set_baudrate_div( spi->peripheral, 0, (uint8_t)( CPU_CLOCK_HZ / config->speed ) );
    spi_set_transfer_delay( spi->peripheral, 0, 0, 0 );
    spi_enable( spi->peripheral );
    pdc_disable_transfer( spi_pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS );

    platform_mcu_powersave_enable( );

    return PLATFORM_SUCCESS;
}

platform_result_t platform_spi_deinit( const platform_spi_t* spi )
{
    UNUSED_PARAMETER(spi);

    /* Disable the RX and TX PDC transfer requests */
    spi_disable( spi->peripheral );
    spi_reset( spi->peripheral );
    return PLATFORM_SUCCESS;
}

platform_result_t sam4s_spi_transfer_internal( const platform_spi_t* spi, const uint8_t* data_out, uint8_t* data_in, uint32_t data_length )
{
    Pdc*         spi_pdc  = spi_get_pdc_base( spi->peripheral );
    pdc_packet_t pdc_spi_packet;

    pdc_spi_packet.ul_addr = (uint32_t)data_in;
    pdc_spi_packet.ul_size = (uint32_t)data_length;
    pdc_rx_init( spi_pdc, &pdc_spi_packet, NULL );

    pdc_spi_packet.ul_addr = (uint32_t)data_out;
    pdc_spi_packet.ul_size = (uint32_t)data_length;
    pdc_tx_init( spi_pdc, &pdc_spi_packet, NULL );

    /* Enable the RX and TX PDC transfer requests */
    pdc_enable_transfer( spi_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN );

    /* Waiting transfer done*/
    while ( ( spi_read_status( spi->peripheral ) & SPI_SR_RXBUFF ) == 0 )
    {
    }

    /* Disable the RX and TX PDC transfer requests */
    pdc_disable_transfer(spi_pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS);

    return PLATFORM_SUCCESS;
}


platform_result_t platform_spi_transfer( const platform_spi_t* spi, const platform_spi_config_t* config, const platform_spi_message_segment_t* segments, uint16_t number_of_segments )
{
    int i = 0;
    platform_result_t result;
    platform_mcu_powersave_disable( );

    platform_gpio_output_low( config->chip_select );

    for ( i = 0; i < number_of_segments; i++ )
    {
        result = sam4s_spi_transfer_internal( spi, segments[i].tx_buffer, segments[i].rx_buffer, segments[i].length );

        if ( result != PLATFORM_SUCCESS )
        {
            return result;
        }
    }

    platform_gpio_output_high( config->chip_select );

    platform_mcu_powersave_enable( );

    return PLATFORM_SUCCESS;
}
