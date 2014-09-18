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

#define MAX_NUM_SPI_PRESCALERS (8)
#define SPI_DMA_TIMEOUT_LOOPS  (10000)

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
    uint16_t factor;
    uint16_t prescaler_value;
} spi_baudrate_division_mapping_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static platform_result_t calculate_prescaler( uint32_t speed, uint16_t* prescaler );
static uint16_t          spi_transfer       ( const platform_spi_t* spi, uint16_t data );
static platform_result_t spi_dma_transfer   ( const platform_spi_t* spi, const platform_spi_config_t* config );
static void              spi_dma_config     ( const platform_spi_t* spi, const platform_spi_message_segment_t* message );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const spi_baudrate_division_mapping_t spi_baudrate_prescalers[MAX_NUM_SPI_PRESCALERS] =
{
    { 2,   SPI_BaudRatePrescaler_2   },
    { 4,   SPI_BaudRatePrescaler_4   },
    { 8,   SPI_BaudRatePrescaler_8   },
    { 16,  SPI_BaudRatePrescaler_16  },
    { 32,  SPI_BaudRatePrescaler_32  },
    { 64,  SPI_BaudRatePrescaler_64  },
    { 128, SPI_BaudRatePrescaler_128 },
    { 256, SPI_BaudRatePrescaler_256 },
};

/* SPI peripheral clock functions */
static const platform_peripheral_clock_function_t spi_peripheral_clock_functions[NUMBER_OF_SPI_PORTS] =
{
    [0] = RCC_APB2PeriphClockCmd,
    [1] = RCC_APB1PeriphClockCmd,
    [2] = RCC_APB1PeriphClockCmd,
};

/* SPI peripheral clocks */
static const uint32_t spi_peripheral_clocks[NUMBER_OF_SPI_PORTS] =
{
    [0] = RCC_APB2Periph_SPI1,
    [1] = RCC_APB1Periph_SPI2,
    [2] = RCC_APB1Periph_SPI3,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

uint8_t platform_spi_get_port_number( platform_spi_port_t* spi )
{
    if ( spi == SPI1 )
    {
        return 0;
    }
    else if ( spi == SPI2 )
    {
        return 1;
    }
    else if ( spi == SPI3 )
    {
        return 2;
    }
    else
    {
        return INVALID_UART_PORT_NUMBER;
    }
}

platform_result_t platform_spi_init( const platform_spi_t* spi, const platform_spi_config_t* config )
{
    SPI_InitTypeDef   spi_init;
    platform_result_t result;
    uint8_t           spi_number;

    wiced_assert( "bad argument", ( spi != NULL ) && ( config != NULL ) );

    platform_mcu_powersave_disable();

    spi_number = platform_spi_get_port_number( spi->port );

    /* Init SPI GPIOs */
    platform_gpio_set_alternate_function( spi->pin_clock->port, spi->pin_clock->pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SPI1 );
    platform_gpio_set_alternate_function( spi->pin_mosi->port,  spi->pin_mosi->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SPI1 );
    platform_gpio_set_alternate_function( spi->pin_miso->port,  spi->pin_miso->pin_number,  GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SPI1 );

    /* Init the chip select GPIO */
    platform_gpio_init( config->chip_select, OUTPUT_PUSH_PULL );
    platform_gpio_output_high( config->chip_select );

    /* Calculate prescaler */
    result = calculate_prescaler( config->speed, &spi_init.SPI_BaudRatePrescaler );
    if ( result != PLATFORM_SUCCESS )
    {
        platform_mcu_powersave_enable();
        return result;
    }

    /* Configure data-width */
    if ( config->bits == 8 )
    {
        spi_init.SPI_DataSize = SPI_DataSize_8b;
    }
    else if ( config->bits == 16 )
    {
        if ( config->mode & SPI_USE_DMA )
        {
            platform_mcu_powersave_enable();

            /* 16 bit mode is not supported for a DMA */
            return PLATFORM_UNSUPPORTED;
        }

        spi_init.SPI_DataSize = SPI_DataSize_16b;
    }
    else
    {
        platform_mcu_powersave_enable();

        /* Requested mode is not supported */
        return PLATFORM_UNSUPPORTED;
    }

    /* Configure MSB or LSB */
    if ( config->mode & SPI_MSB_FIRST )
    {
        spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
    }
    else
    {
        spi_init.SPI_FirstBit = SPI_FirstBit_LSB;
    }

    /* Configure mode CPHA and CPOL */
    if ( config->mode & SPI_CLOCK_IDLE_HIGH )
    {
        spi_init.SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        spi_init.SPI_CPOL = SPI_CPOL_Low;
    }

    if ( config->mode & SPI_CLOCK_RISING_EDGE )
    {
        spi_init.SPI_CPHA = ( config->mode & SPI_CLOCK_IDLE_HIGH ) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
    }
    else
    {
        spi_init.SPI_CPHA = ( config->mode & SPI_CLOCK_IDLE_HIGH ) ? SPI_CPHA_1Edge : SPI_CPHA_2Edge;
    }

    /* Enable SPI peripheral clock */
    spi_peripheral_clock_functions[ spi_number ]( spi_peripheral_clocks[ spi_number ], ENABLE );

    spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode      = SPI_Mode_Master;
    spi_init.SPI_NSS       = SPI_NSS_Soft;
    spi_init.SPI_CRCPolynomial = 0x7; /* reset value */
    SPI_CalculateCRC( spi->port, DISABLE );

    /* Init and enable SPI */
    SPI_Init( spi->port, &spi_init );
    SPI_Cmd ( spi->port, ENABLE );

    platform_mcu_powersave_enable();

    return WICED_SUCCESS;
}

platform_result_t platform_spi_deinit( const platform_spi_t* spi )
{
    UNUSED_PARAMETER( spi );
    /* TODO: unimplemented */
    return PLATFORM_UNSUPPORTED;
}

platform_result_t platform_spi_transfer( const platform_spi_t* spi, const platform_spi_config_t* config, const platform_spi_message_segment_t* segments, uint16_t number_of_segments )
{
    platform_result_t result = PLATFORM_SUCCESS;
    uint32_t          count  = 0;
    uint16_t          i;

    wiced_assert( "bad argument", ( spi != NULL ) && ( config != NULL ) && ( segments != NULL ) && ( number_of_segments != 0 ) );

    platform_mcu_powersave_disable();

    /* Activate chip select */
    platform_gpio_output_low( config->chip_select );

    for ( i = 0; i < number_of_segments; i++ )
    {
        /* Check if we are using DMA */
        if ( config->mode & SPI_USE_DMA )
        {
            spi_dma_config( spi, &segments[ i ] );

            result = spi_dma_transfer( spi, config );
            if ( result != PLATFORM_SUCCESS )
            {
                goto cleanup_transfer;
            }
        }
        else
        {
            count = segments[i].length;

            /* in interrupt-less mode */
            if ( config->bits == 8 )
            {
                const uint8_t* send_ptr = ( const uint8_t* )segments[i].tx_buffer;
                uint8_t*       rcv_ptr  = ( uint8_t* )segments[i].rx_buffer;

                while ( count-- )
                {
                    uint16_t data = 0xFF;

                    if ( send_ptr != NULL )
                    {
                        data = *send_ptr++;
                    }

                    data = spi_transfer( spi, data );

                    if ( rcv_ptr != NULL )
                    {
                        *rcv_ptr++ = (uint8_t)data;
                    }
                }
            }
            else if ( config->bits == 16 )
            {
                const uint16_t* send_ptr = (const uint16_t *) segments[i].tx_buffer;
                uint16_t*       rcv_ptr  = (uint16_t *) segments[i].rx_buffer;

                /* Check that the message length is a multiple of 2 */
                if ( ( count % 2 ) != 0 )
                {
                    result = WICED_ERROR;
                    goto cleanup_transfer;
                }

                /* Transmit/receive data stream, 16-bit at time */
                while ( count != 0 )
                {
                    uint16_t data = 0xFFFF;

                    if ( send_ptr != NULL )
                    {
                        data = *send_ptr++;
                    }

                    data = spi_transfer( spi, data );

                    if ( rcv_ptr != NULL )
                    {
                        *rcv_ptr++ = data;
                    }

                    count -= 2;
                }
            }
        }
    }

cleanup_transfer:
    /* Deassert chip select */
    platform_gpio_output_high( config->chip_select );

    platform_mcu_powersave_enable( );

    return result;
}

static uint16_t spi_transfer( const platform_spi_t* spi, uint16_t data )
{
    /* Wait until the transmit buffer is empty */
    while ( SPI_I2S_GetFlagStatus( spi->port, SPI_I2S_FLAG_TXE ) == RESET )
    {
    }

    /* Send the byte */
    SPI_I2S_SendData( spi->port, data );

    /* Wait until a data is received */
    while ( SPI_I2S_GetFlagStatus( spi->port, SPI_I2S_FLAG_RXNE ) == RESET )
    {
    }

    /* Get the received data */
    return SPI_I2S_ReceiveData( spi->port );
}

static platform_result_t calculate_prescaler( uint32_t speed, uint16_t* prescaler )
{
    uint8_t i;

    wiced_assert("Bad args", prescaler != NULL);

    for( i = 0 ; i < MAX_NUM_SPI_PRESCALERS ; i++ )
    {
        if( ( 60000000 / spi_baudrate_prescalers[i].factor ) <= speed )
        {
            *prescaler = spi_baudrate_prescalers[i].prescaler_value;
            return PLATFORM_SUCCESS;
        }
    }

    return PLATFORM_ERROR;
}

static platform_result_t spi_dma_transfer( const platform_spi_t* spi, const platform_spi_config_t* config )
{
    uint32_t loop_count;

    /* Enable dma channels that have just been configured */
    DMA_Cmd( spi->tx_dma.stream, ENABLE );
    DMA_Cmd( spi->rx_dma.stream, ENABLE );

    /* Wait for DMA to complete */
    /* TODO: This should wait on a semaphore that is triggered from an IRQ */
    loop_count = 0;
    while ( ( DMA_GetFlagStatus( spi->tx_dma.stream, spi->tx_dma.complete_flags ) == RESET ) )
    {
        loop_count++;
        /* Check if we've run out of time */
        if ( loop_count >= (uint32_t) SPI_DMA_TIMEOUT_LOOPS )
        {
            platform_gpio_output_high( config->chip_select );
            return WICED_TIMEOUT;
        }
    }

    platform_gpio_output_high( config->chip_select );
    return WICED_SUCCESS;
}

static void spi_dma_config( const platform_spi_t* spi, const platform_spi_message_segment_t* message )
{
    DMA_InitTypeDef dma_init;
    uint8_t         dummy = 0xFF;

    /* Setup DMA for SPI TX if it is enabled */
    DMA_DeInit( spi->tx_dma.stream );

    /* Setup DMA stream for TX */
    dma_init.DMA_Channel            = spi->tx_dma.channel;
    dma_init.DMA_PeripheralBaseAddr = ( uint32_t )spi->port->DR;
    dma_init.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_BufferSize         = message->length;
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
    dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;

    if ( message->tx_buffer != NULL )
    {
       dma_init.DMA_Memory0BaseAddr = ( uint32_t )message->tx_buffer;
       dma_init.DMA_MemoryInc       = DMA_MemoryInc_Enable;
    }
    else
    {
       dma_init.DMA_Memory0BaseAddr = ( uint32_t )(&dummy);
       dma_init.DMA_MemoryInc       = DMA_MemoryInc_Disable;
    }

    DMA_Init( spi->tx_dma.stream, &dma_init );

    /* Activate SPI DMA mode for transmission */
    SPI_I2S_DMACmd( spi->port, SPI_I2S_DMAReq_Tx, ENABLE );

    /* TODO: Init TX DMA finished semaphore  */

    /* Setup DMA for SPI RX stream */
    DMA_DeInit( spi->rx_dma.stream );
    dma_init.DMA_Channel            = spi->rx_dma.channel;
    dma_init.DMA_PeripheralBaseAddr = ( uint32_t )&spi->port->DR;
    dma_init.DMA_DIR                = DMA_DIR_PeripheralToMemory;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_BufferSize         = message->length;
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
    dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    if ( message->rx_buffer != NULL )
    {
        dma_init.DMA_Memory0BaseAddr = (uint32_t)message->rx_buffer;
        dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    }
    else
    {
        dma_init.DMA_Memory0BaseAddr = (uint32_t)&dummy;
        dma_init.DMA_MemoryInc = DMA_MemoryInc_Disable;
    }

    /* Init and activate RX DMA channel */
    DMA_Init( spi->tx_dma.stream, &dma_init );
    SPI_I2S_DMACmd( spi->port, SPI_I2S_DMAReq_Rx, ENABLE );

    /* TODO: Init RX DMA finish semaphore */
}
