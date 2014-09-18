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
#include "stdint.h"
#include "string.h"
#include "platform_peripheral.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos.h"
#include "wwd_assert.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define I2C_TRANSACTION_TIMEOUT        (1000)

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
    host_semaphore_type_t    transfer_complete;
    uint32_t                 transfer_size;
    wiced_bool_t             i2c_inited;
    const platform_i2c_t*    peripheral;
    platform_i2c_message_t*  current_message;
    uint16_t                 data_count;
    volatile int             transfer_status;
} sam4s_i2c_runtime_data_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

void i2c0_irq( void );
void i2c1_irq( void );
void twi_irq ( sam4s_i2c_runtime_data_t* runtime_data );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Note: Every Atmel SAM4S family chip has no more than 2 TWI peripheral blocks */
static sam4s_i2c_runtime_data_t i2c_runtime_data[2];
static wiced_bool_t             i2c_runtime_data_inited = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_i2c_init( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
    platform_result_t result;
    twi_options_t     twi_slave_config;

    if ( i2c_runtime_data_inited == WICED_FALSE )
    {
        /* Initialise when this function is first called */
        memset( &i2c_runtime_data, 0, sizeof( i2c_runtime_data ) );
        i2c_runtime_data_inited = WICED_TRUE;
    }

    /* Initialise transfer complete semaphore */
    result = host_rtos_init_semaphore( &i2c_runtime_data[ i2c->i2c_block_id ].transfer_complete );
    if ( result != PLATFORM_SUCCESS )
    {
        wiced_assert("Semaphore initialization has failed", result == PLATFORM_SUCCESS);
        return result;
    }

    /* Setup SAM4S pins */
    platform_gpio_init( i2c->sda_pin, OUTPUT_OPEN_DRAIN_PULL_UP );
    platform_gpio_init( i2c->scl_pin, OUTPUT_OPEN_DRAIN_PULL_UP );
    platform_gpio_peripheral_pin_init( i2c->sda_pin, ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP ) );
    platform_gpio_peripheral_pin_init( i2c->scl_pin, ( IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP ) );

    /* Enable the peripheral clock in the PMC. */
    sysclk_enable_peripheral_clock( i2c->peripheral_id );

    /* Configure and enable interrupt for TWI peripheral on the cpu core */
    /* Priority must be set in platform.c file function platform_init_peripheral_irq_priorities */
    if ( i2c->peripheral_id == ID_TWI0 )
    {
        NVIC_EnableIRQ( TWI0_IRQn );
    }
    else if ( i2c->peripheral_id == ID_TWI1 )
    {
        NVIC_EnableIRQ( TWI1_IRQn );
    }

    twi_enable_master_mode( i2c->peripheral );

    wiced_assert( "Device address other than 7 bits is not supported currenly", config->address_width == I2C_ADDRESS_WIDTH_7BIT );

    twi_slave_config.chip = (uint8_t) config->address;
    switch ( config->speed_mode )
    {
        case I2C_LOW_SPEED_MODE     : twi_slave_config.speed = 10000;  break;
        case I2C_STANDARD_SPEED_MODE: twi_slave_config.speed = 100000; break;
        case I2C_HIGH_SPEED_MODE    : twi_slave_config.speed = 400000; break;
        default                     : wiced_assert("Speed mode is not supported", 0 != 0); break;
    }

    twi_slave_config.master_clk = CPU_CLOCK_HZ;
    twi_slave_config.smbus      = 0;
    twi_master_init(i2c->peripheral, &twi_slave_config);

    i2c_runtime_data->peripheral = i2c;
    i2c_runtime_data->i2c_inited = WICED_TRUE;

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_deinit( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
    memset( &i2c_runtime_data[i2c->i2c_block_id], 0x00, sizeof( sam4s_i2c_runtime_data_t ) );
    i2c_runtime_data->i2c_inited = WICED_FALSE;
    return PLATFORM_SUCCESS;
}

wiced_bool_t platform_i2c_probe_device( const platform_i2c_t* i2c, const platform_i2c_config_t* config, int retries )
{
    UNUSED_PARAMETER( i2c );
    UNUSED_PARAMETER( config );
    UNUSED_PARAMETER( retries );

    /* This function is unimplemented because sam4s cant transmit address byte by itself.
     * It will be trailed by one dummy byte because transmission can not be kicked off until TX register is written.
     * As soon as the TX register is written, the I2C peripheral will send address byte followed by TX register contents.
     * Some chips may interpret it as a part of the chip protocol which is unacceptable for a probing functionality */

    return WICED_TRUE;
}

platform_result_t platform_i2c_init_tx_message( platform_i2c_message_t* message, const void* tx_buffer, uint16_t tx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    UNUSED_PARAMETER( disable_dma );

    wiced_assert( "DMA is not supported by SAM4S I2C driver", disable_dma == WICED_TRUE );
    wiced_assert( "Bad args", ( message != 0 ) && ( tx_buffer != 0 ) && ( tx_buffer_length != 0 ) );

    memset( message, 0x00, sizeof(platform_i2c_message_t) );
    message->tx_buffer = tx_buffer;
    message->tx_length = tx_buffer_length;
    message->combined  = WICED_FALSE;
    message->retries   = retries;

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_init_rx_message( platform_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    UNUSED_PARAMETER(disable_dma);

    wiced_assert( "DMA is not supported by SAM4S I2C driver", disable_dma == WICED_TRUE );
    wiced_assert( "Bad args", ( message != 0 ) && ( rx_buffer != 0 ) && ( rx_buffer_length != 0 ) );

    memset(message, 0x00, sizeof(platform_i2c_message_t));
    message->rx_buffer = rx_buffer;
    message->combined  = WICED_FALSE;
    message->retries   = retries;
    message->rx_length = rx_buffer_length;

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_init_combined_message( platform_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    UNUSED_PARAMETER( disable_dma );

    wiced_assert( "DMA is not supported by SAM4S I2C driver", disable_dma == WICED_TRUE );
    wiced_assert( "Bad args", ( message != 0 ) && ( tx_buffer != 0 ) && ( tx_buffer_length != 0 ) && ( rx_buffer != 0 ) && ( rx_buffer_length != 0 ) );

    memset(message, 0x00, sizeof(platform_i2c_message_t));
    message->rx_buffer = rx_buffer;
    message->tx_buffer = tx_buffer;
    message->combined  = WICED_TRUE;
    message->retries   = retries;
    message->tx_length = tx_buffer_length;
    message->rx_length = rx_buffer_length;

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_transfer( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* messages, uint16_t number_of_messages )
{
    platform_result_t result =  PLATFORM_SUCCESS;
    Twi*              twi    = ( Twi* )( i2c_runtime_data[i2c->i2c_block_id].peripheral->peripheral );
    int               i;
    int               retry_count;

    /* Check input arguments */
    wiced_assert("Bad argument", (i2c != NULL) && ( config != NULL ) &&( messages != NULL ) && ( number_of_messages != 0 ));

    for ( i = 0; i < number_of_messages && result == PLATFORM_SUCCESS; i++ )
    {
        platform_i2c_message_t* message_pointer = &messages[i];
        for ( retry_count = 0; retry_count < message_pointer->retries; retry_count++ )
        {
            wiced_assert("Message pointer shouldn't be null", message_pointer != NULL);

            i2c_runtime_data[i2c->i2c_block_id].current_message = message_pointer;
            i2c_runtime_data[i2c->i2c_block_id].data_count      = 0;
            i2c_runtime_data[i2c->i2c_block_id].transfer_status = 0;

            if ( message_pointer->rx_buffer != NULL )
            {
                /* Initiate an RX transfer */

                /* Set read mode and slave address */
                twi->TWI_MMR = 0;
                twi->TWI_MMR = TWI_MMR_MREAD | TWI_MMR_DADR(config->address);

                /* Dumb status and rx registers reading */
                twi->TWI_SR;
                twi->TWI_RHR;

                /* Enable NACK, RXRDY interrupts */
                twi->TWI_IER = TWI_IER_NACK | TWI_IER_RXRDY;

                if ( message_pointer->rx_length == 1 )
                {
                    /* Extract from sam4s user manual.
                     * When a single data byte read is performed, with or without internal address (IADR),
                     * the START and STOP bits must be set at the same time */
                    twi->TWI_CR = TWI_CR_STOP;
                }

                /* Generate a START Condition */
                twi->TWI_CR = TWI_CR_START;

            }
            else if ( message_pointer->tx_buffer != NULL )
            {
                /* Initiate a TX transfer */
                /* Set read mode, slave address */
                twi->TWI_MMR = 0;
                twi->TWI_MMR = TWI_MMR_DADR(config->address);

                /* Dumb status register reading */
                twi->TWI_SR;

                /* Enable NACK, TXRDY interrupts */
                twi->TWI_IER = TWI_IER_NACK | TWI_IER_TXRDY;

            }
            else
            {
                /* Combined transaction is not supported currently */
                wiced_assert("Combined transactions are not supported on sam4s twi", 0!=0);
                return WICED_ERROR;
            }
            /* Wait on a semaphore until a message is received or transmitted */
            result = host_rtos_get_semaphore( &i2c_runtime_data[i2c->i2c_block_id].transfer_complete, I2C_TRANSACTION_TIMEOUT, WICED_TRUE );
            if ( result != PLATFORM_SUCCESS )
            {
                return result;
            }

            /* Check for errors */
            if ( i2c_runtime_data[i2c->i2c_block_id].transfer_status < 0 )
            {
                result = PLATFORM_ERROR;
            }
            else
            {
                /* Generate a stop condition at the of the transaction */
                twi->TWI_CR = TWI_CR_STOP;
                result = PLATFORM_SUCCESS;
                break;
            }
        }
    }

    return result;
}

void twi_irq( sam4s_i2c_runtime_data_t* runtime_data )
{
    Twi*     twi_regs = (Twi*)( runtime_data->peripheral->peripheral );
    uint32_t status   = twi_regs->TWI_SR;
    uint32_t mask     = twi_get_interrupt_mask( runtime_data->peripheral->peripheral );

    /* Check if this is a NACK interrupt */
    if ( ( status & mask & TWI_SR_NACK ) != 0 )
    {
        runtime_data->transfer_status = -1;
        twi_regs->TWI_IDR = TWI_IDR_NACK;
    }
    else if ( (status & mask & TWI_SR_RXRDY) != 0)
    {
        uint8_t* rx_buffer_pointer = ( uint8_t* )runtime_data->current_message->rx_buffer;

        /* Read data increment the number of received bytes */
        /* Get data from Receive Holding Register */
        rx_buffer_pointer[runtime_data->data_count] = (uint8_t)twi_regs->TWI_RHR;

        /* Increase transmitted bytes count */
        runtime_data->data_count++;

        if ( runtime_data->data_count == runtime_data->current_message->rx_length )
        {
            /* Disable RX interrupt */
            twi_regs->TWI_IDR = TWI_IDR_RXRDY;

            /* Set the semaphore that transaction is complete */
            host_rtos_set_semaphore( &runtime_data->transfer_complete, WICED_TRUE );
        }
    }
    else if ( (status & mask & TWI_SR_TXRDY) != 0 )
    {
        if ( runtime_data->data_count == runtime_data->current_message->tx_length )
        {
            /* Finish the transmit operation */
            twi_regs->TWI_IDR = TWI_IDR_TXRDY;

            /* Check if this is a combined transaction */
            if ( ( runtime_data->current_message->tx_buffer != NULL ) && ( runtime_data->current_message->rx_buffer != NULL ) )
            {
                /* Generate a START Condition */
                twi_regs->TWI_CR = TWI_CR_START;
            }
            else
            {
                /* Set the semaphore that transaction is complete */
                host_rtos_set_semaphore( &runtime_data->transfer_complete, WICED_TRUE );
            }
        }
        else
        {
            uint8_t* tx_buffer_pointer = (uint8_t*) runtime_data->current_message->tx_buffer;

            /* Put the byte in the Transmit Holding Register */
            twi_regs->TWI_THR = tx_buffer_pointer[runtime_data->data_count];

            /* Increase transmitted bytes number */
            runtime_data->data_count++;
        }
    }
    else
    {
        wiced_assert("Unhandled I2C0 interrupt", 0 != 0);
    }
}

WWD_RTOS_DEFINE_ISR(i2c0_irq)
{
    twi_irq( &i2c_runtime_data[TWI_0] );
}

WWD_RTOS_DEFINE_ISR(i2c1_irq)
{
    twi_irq( &i2c_runtime_data[TWI_1] );
}

/******************************************************
 *             IRQ Handler Mapping
 ******************************************************/

WWD_RTOS_MAP_ISR( i2c0_irq , TWI0_irq )
WWD_RTOS_MAP_ISR( i2c1_irq , TWI1_irq )



