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

#define DMA_FLAG_TC(stream_id) dma_flag_tc(stream_id)

/******************************************************
 *                    Constants
 ******************************************************/

#define I2C_FLAG_CHECK_TIMEOUT      ( 1000 )
#define I2C_FLAG_CHECK_LONG_TIMEOUT ( 1000 )
#define I2C_MESSAGE_DMA_MASK_POSN   ( 0 )
#define I2C_MESSAGE_NO_DMA          ( 0 << I2C_MESSAGE_DMA_MASK_POSN ) /* No DMA is set to 0 because DMA should be enabled by */
#define I2C_MESSAGE_USE_DMA         ( 1 << I2C_MESSAGE_DMA_MASK_POSN ) /* default, and turned off as an exception */
#define DMA_TIMEOUT_LOOPS           ( 10000000 )

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

static DMA_InitTypeDef i2c_dma_init; /* Should investigate why this is global */

static const uint32_t dma_transfer_complete_flags[] =
{
    [0] = DMA_FLAG_TCIF0,
    [1] = DMA_FLAG_TCIF1,
    [2] = DMA_FLAG_TCIF2,
    [3] = DMA_FLAG_TCIF3,
    [4] = DMA_FLAG_TCIF4,
    [5] = DMA_FLAG_TCIF5,
    [6] = DMA_FLAG_TCIF6,
    [7] = DMA_FLAG_TCIF7,
};


/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_i2c_init( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
    I2C_InitTypeDef I2C_InitStructure;

    wiced_assert( "bad argument", ( i2c != NULL ) && ( config != NULL ) );

    platform_mcu_powersave_disable( );

    // Init I2C GPIO clocks
    RCC_APB1PeriphClockCmd( i2c->peripheral_clock_reg, ENABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG,     ENABLE );

    // Reset the I2C clock
    RCC_APB1PeriphResetCmd( i2c->peripheral_clock_reg, ENABLE );
    RCC_APB1PeriphResetCmd( i2c->peripheral_clock_reg, DISABLE );

    // GPIO Configuration
    platform_gpio_set_alternate_function( i2c->pin_scl->port, i2c->pin_scl->pin_number, GPIO_OType_OD, GPIO_PuPd_NOPULL, i2c->gpio_af );
    platform_gpio_set_alternate_function( i2c->pin_sda->port, i2c->pin_sda->pin_number, GPIO_OType_OD, GPIO_PuPd_NOPULL, i2c->gpio_af );

    if ( config->flags & I2C_DEVICE_USE_DMA )
    {
        // Enable the DMA clock
        RCC_AHB1PeriphClockCmd( i2c->tx_dma_peripheral_clock, ENABLE );

        // Configure the DMA streams for operation with the CP
        i2c_dma_init.DMA_Channel            = i2c->tx_dma_channel;
        i2c_dma_init.DMA_PeripheralBaseAddr = (uint32_t) &i2c->port->DR;
        i2c_dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
        i2c_dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
        i2c_dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        i2c_dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
        i2c_dma_init.DMA_Mode               = DMA_Mode_Normal;
        i2c_dma_init.DMA_Priority           = DMA_Priority_VeryHigh;
        //dma_init.DMA_FIFOMode             = DMA_FIFOMode_Enable;
        //dma_init.DMA_FIFOThreshold        = DMA_FIFOThreshold_Full;
        i2c_dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
        i2c_dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
        i2c_dma_init.DMA_Memory0BaseAddr    = (uint32_t) 0;               // This parameter will be configured during communication
        i2c_dma_init.DMA_DIR                = DMA_DIR_MemoryToPeripheral; // This parameter will be configured during communication
        i2c_dma_init.DMA_BufferSize         = 0xFFFF;                     // This parameter will be configured during communication

        DMA_DeInit( i2c->rx_dma_stream );
        DMA_DeInit( i2c->tx_dma_stream );

        // Clear any pending flags, disable, and clear the Tx DMA channel
        //DMA_ClearFlag( i2c_mapping[i2c->port].tx_dma_stream, CP_TX_DMA_FLAG_FEIF | CP_TX_DMA_FLAG_DMEIF | CP_TX_DMA_FLAG_TEIF | CP_TX_DMA_FLAG_HTIF | CP_TX_DMA_FLAG_TCIF );
        DMA_Cmd( i2c->tx_dma_stream, DISABLE );
        DMA_Cmd( i2c->rx_dma_stream, DISABLE );

        // Clear any pending flags, disable, and clear the Rx DMA channel
        //DMA_ClearFlag( i2c_mapping[i2c->port].rx_dma_stream, CP_RX_DMA_FLAG_FEIF | CP_RX_DMA_FLAG_DMEIF | CP_RX_DMA_FLAG_TEIF | CP_RX_DMA_FLAG_HTIF | CP_RX_DMA_FLAG_TCIF );
    }

    // Initialize the InitStruct for the CP
    I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1         = 0xA0;
    I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    if ( config->speed_mode == I2C_LOW_SPEED_MODE )
    {
        I2C_InitStructure.I2C_ClockSpeed = 10000;
    }
    else if ( config->speed_mode == I2C_STANDARD_SPEED_MODE )
    {
        I2C_InitStructure.I2C_ClockSpeed = 100000;
    }
    else if ( config->speed_mode == I2C_HIGH_SPEED_MODE )
    {
        I2C_InitStructure.I2C_ClockSpeed = 400000;
    }

    // Enable and initialize the I2C bus
    I2C_Cmd( i2c->port, ENABLE );
    I2C_Init( i2c->port, &I2C_InitStructure );

    if ( config->flags & I2C_DEVICE_USE_DMA )
    {
        // Enable DMA on the I2C bus
        I2C_DMACmd( i2c->port, ENABLE );
    }

    platform_mcu_powersave_enable( );

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_deinit( const platform_i2c_t* i2c, const platform_i2c_config_t* config )
{
    wiced_assert( "bad argument", ( i2c != NULL ) && ( config != NULL ) );

    platform_mcu_powersave_disable( );

    /* Disable I2C peripheral clocks */
    RCC_APB1PeriphClockCmd( i2c->peripheral_clock_reg, DISABLE );

    /* Disable DMA */
    if ( config->flags & I2C_DEVICE_USE_DMA )
    {
        DMA_DeInit( i2c->rx_dma_stream );
        DMA_DeInit( i2c->tx_dma_stream );
        RCC_AHB1PeriphClockCmd( i2c->tx_dma_peripheral_clock, DISABLE );
    }

    platform_mcu_powersave_enable( );

    return PLATFORM_SUCCESS;
}

static platform_result_t i2c_wait_for_event( I2C_TypeDef* i2c, uint32_t event_id, uint32_t number_of_waits )
{
    while ( I2C_CheckEvent( i2c, event_id ) != SUCCESS )
    {
        number_of_waits--;
        if ( number_of_waits == 0 )
        {
            return PLATFORM_TIMEOUT;
        }
    }

    return PLATFORM_SUCCESS;
}

wiced_bool_t platform_i2c_probe_device( const platform_i2c_t* i2c, const platform_i2c_config_t* config, int retries )
{
    platform_result_t result;
    int i;

    wiced_assert( "bad argument", ( i2c != NULL ) && ( config != NULL ) );

    platform_mcu_powersave_disable();

    for ( i = 0; i < retries; i++ )
    {
        /* generate a start condition and address a i2c in write mode */
        I2C_GenerateSTART( i2c->port, ENABLE );

        /* wait till start condition is generated and the bus becomes free */
        result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_MODE_SELECT, I2C_FLAG_CHECK_TIMEOUT );
        if ( result != PLATFORM_SUCCESS )
        {
            // FIXME platform_mcu_powersave_enable();
            return WICED_FALSE;
        }

        if ( config->address_width == I2C_ADDRESS_WIDTH_7BIT )
        {
            /* send the address and R/W bit set to write of the requested i2c, wait for an acknowledge */
            I2C_Send7bitAddress( i2c->port, (uint8_t) ( config->address << 1 ), I2C_Direction_Transmitter );

            /* wait till address gets sent and the direction bit is sent and */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                /* keep on pinging */
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            // TODO
            /* send 10 bits of the address and wait for an acknowledge */
        }
    }

    /* generate a stop condition */
    I2C_GenerateSTOP( i2c->port, ENABLE );

    platform_mcu_powersave_enable();

    /* Check if the i2c didn't respond */
    if ( i == retries )
    {
        return WICED_FALSE;
    }
    else
    {
        return WICED_TRUE;
    }
}

static platform_result_t i2c_dma_config_and_execute( const platform_i2c_t* i2c, platform_i2c_message_t* message, wiced_bool_t tx_dma )
{
    uint32_t counter;

    /* Initialize the DMA with the new parameters */
    if ( tx_dma == WICED_TRUE )
    {
        /* Enable DMA channel for I2C */
        I2C_DMACmd( i2c->port, ENABLE );

        /* TX DMA configuration */
        DMA_DeInit( i2c->tx_dma_stream );

        /* Configure the DMA TX Stream with the buffer address and the buffer size */
        i2c_dma_init.DMA_Memory0BaseAddr = (uint32_t) message->tx_buffer;
        i2c_dma_init.DMA_DIR             = DMA_DIR_MemoryToPeripheral;
        i2c_dma_init.DMA_BufferSize      = (uint32_t) message->tx_length;
        DMA_Init( i2c->tx_dma_stream, &i2c_dma_init );

        /* Enable DMA channel */
        DMA_Cmd( i2c->tx_dma_stream, ENABLE );

        /* wait until transfer is completed */
        /* TODO: change flag!!!!,wait on a semaphore */
        counter = DMA_TIMEOUT_LOOPS;
        while ( DMA_GetFlagStatus( i2c->tx_dma_stream, dma_transfer_complete_flags[ i2c->tx_dma_stream_id ] ) == RESET )
        {
            --counter;
            if ( counter == 0 )
            {
                return PLATFORM_ERROR;
            }
        }

        /* Disable DMA and channel */
        I2C_DMACmd( i2c->port, DISABLE );
        DMA_Cmd( i2c->tx_dma_stream, DISABLE );
    }
    else
    {
        /* Enable dma channel for I2C */
        I2C_DMACmd( i2c->port, ENABLE );

        /* RX DMA configuration */
        DMA_DeInit( i2c->rx_dma_stream );

        /* Configure the DMA Rx Stream with the buffer address and the buffer size */
        i2c_dma_init.DMA_Memory0BaseAddr = (uint32_t) message->rx_buffer;
        i2c_dma_init.DMA_DIR             = DMA_DIR_PeripheralToMemory;
        i2c_dma_init.DMA_BufferSize      = (uint32_t) message->rx_length;
        DMA_Init( i2c->rx_dma_stream, &i2c_dma_init );

        /* Enable DMA channel */
        DMA_Cmd( i2c->rx_dma_stream, ENABLE );

        /* wait until transfer is completed */
        counter = DMA_TIMEOUT_LOOPS;
        while ( DMA_GetFlagStatus( i2c->rx_dma_stream, dma_transfer_complete_flags[ i2c->rx_dma_stream_id ] ) == RESET )
        {
            --counter;
            if ( counter == 0 )
            {
                return PLATFORM_ERROR;
            }
        }

        /* disable DMA and channel */
        I2C_DMACmd( i2c->port, DISABLE );
        DMA_Cmd( i2c->rx_dma_stream, DISABLE );
    }

    return PLATFORM_SUCCESS;
}

static platform_result_t i2c_dma_transfer( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message )
{
    platform_result_t result;
    uint32_t counter;
    int i = 0;

    if ( message->combined == WICED_TRUE )
    {
        /* combined transaction case, please refer to Philips I2C document to have an understanding of a combined fragment */

        /* some chips( authentication and security related chips ) has to be addressed several times before they acknowledge their address */
        for ( i = 0; i < message->retries; i++ )
        {
            /* generate a start condition and address a i2c in write mode */
            I2C_GenerateSTART( i2c->port, ENABLE );

            /* wait till start condition is generated and the bus becomes free */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_MODE_SELECT, I2C_FLAG_CHECK_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                return PLATFORM_TIMEOUT;
            }

            if ( config->address_width == I2C_ADDRESS_WIDTH_7BIT )
            {
                /* send the address and R/W bit set to write of the requested i2c, wait for an acknowledge */
                I2C_Send7bitAddress( i2c->port, (uint8_t) ( config->address << 1 ), I2C_Direction_Transmitter );

                /* wait till address gets sent and the direction bit is sent and */
                result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
                if ( result == PLATFORM_SUCCESS )
                {
                    break;
                }
            }
            else
            {
                //TODO
                /* send 10 bits of the address and wait for an acknowledge */
            }
        }

        if ( i == message->retries )
        {
            return PLATFORM_TIMEOUT;
        }

        /* configure dma tx channel for i2c */
        i2c_dma_config_and_execute( i2c, message, WICED_TRUE );

        /* wait till the byte is actually sent from the i2c peripheral */
        counter = 1000;
        while ( I2C_GetFlagStatus( i2c->port, I2C_FLAG_BTF ) == RESET )
        {
            --counter;
            if ( counter == 0 )
            {
                return PLATFORM_ERROR;
            }
        }

        /* generate start condition again and address a i2c in read mode */
        /* some chips( authentication and security related chips ) has to be addressed several times before they acknowledge their address */
        for ( i = 0; i < message->retries; i++ )
        {
            /* generate a start condition */
            I2C_GenerateSTART( i2c->port, ENABLE );

            /* wait till start condition is generated and the bus becomes free */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_MODE_SELECT, I2C_FLAG_CHECK_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                return PLATFORM_TIMEOUT;
            }

            if ( config->address_width == I2C_ADDRESS_WIDTH_7BIT )
            {
                /* send the address and R/W bit set to write of the requested i2c, wait for an acknowledge */
                I2C_Send7bitAddress( i2c->port, (uint8_t) ( config->address << 1 ), I2C_Direction_Receiver );

                /* wait till address gets sent and the direction bit is sent and */
                result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
                if ( result == PLATFORM_SUCCESS )
                {
                    break;
                }
            }
            else
            {
                //TODO
                /* send 10 bits of the address and wait for an acknowledge */
            }
        }

        if ( i == message->retries )
        {
            return PLATFORM_TIMEOUT;
        }

        /* receive data from the slave i2c */
        if ( message->rx_length == 1 )
        {
            /* disable acknowledgement before we start receiving bytes, this is a single byte transmission */
            I2C_AcknowledgeConfig( i2c->port, DISABLE );
        }
        else
        {
            /* enable acknowledgement before we start receiving bytes, this is a single byte transmission */
            I2C_AcknowledgeConfig( i2c->port, ENABLE );
        }

        /* start dma which will read bytes */
        i2c_dma_config_and_execute( i2c, message, WICED_FALSE );
        /* maybe we will have to wait on the BTF flag!!! */
    }
    else
    {

        /* read or write transaction */

        /* some chips( authentication and security related chips ) has to be addressed several times before they acknowledge their address */
        for ( i = 0; i < message->retries; i++ )
        {
            /* generate a start condition */
            I2C_GenerateSTART( i2c->port, ENABLE );

            /* wait till start condition is generated and the bus becomes free */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_MODE_SELECT, I2C_FLAG_CHECK_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                return PLATFORM_TIMEOUT;
            }

            if ( config->address_width == I2C_ADDRESS_WIDTH_7BIT )
            {
                /* send the address of the requested i2c, wait for an acknowledge */
                I2C_Send7bitAddress( i2c->port, (uint8_t) ( config->address << 1 ), ( ( message->tx_buffer ) ? I2C_Direction_Transmitter : I2C_Direction_Receiver ) );

                /* wait till address gets sent and the direction bit is sent */
                if ( message->tx_buffer )
                {
                    result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
                }
                else
                {
                    result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
                }

                if ( result == PLATFORM_SUCCESS )
                {
                    break;
                }
            }
            else
            {
                //TODO
                /* send 10 bits of the address and wait for an acknowledge */
            }
        }
        if ( i == message->retries )
        {
            return PLATFORM_TIMEOUT;
        }

        if ( message->tx_buffer )
        {
            /* write transaction */
            /* configure dma tx channel for i2c */
            i2c_dma_config_and_execute( i2c, message, WICED_TRUE );

            /* wait till the byte is actually sent from the i2c peripheral */
            counter = 1000;
            while ( I2C_GetFlagStatus( i2c->port, I2C_FLAG_BTF ) == RESET )
            {
                --counter;
                if ( counter == 0 )
                {
                    return PLATFORM_ERROR;
                }
            }
        }
        else
        {
            /* read transaction */
            if ( message->rx_length == 1 )
            {
                /* disable acknowledgement before we are going to receive a single byte */
                I2C_AcknowledgeConfig( i2c->port, DISABLE );
            }
            else
            {
                /* enable acknowledgement before we start receiving multiple bytes */
                I2C_AcknowledgeConfig( i2c->port, ENABLE );
            }

            /* start dma which will read bytes */
            i2c_dma_config_and_execute( i2c, message, WICED_FALSE );

            /* wait til the last byte is received */
            counter = 1000;
            while ( I2C_GetFlagStatus( i2c->port, I2C_FLAG_BTF ) == RESET )
            {
                --counter;
                if ( counter == 0 )
                {
                    return PLATFORM_ERROR;
                }
            }
        }
    }

    /* generate a stop condition */
    I2C_GenerateSTOP( i2c->port, ENABLE );
    return PLATFORM_SUCCESS;
}

static platform_result_t i2c_transfer_message_no_dma( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* message )
{
    platform_result_t result;
    int i = 0;

    if ( message->combined == WICED_TRUE )
    {
        const char* tmp_ptr;
        uint8_t* tmp_rd_ptr;

        /* combined transaction case, please refer to Philips I2C document to have an understanding of a combined fragment */

        /* some chips( authentication and security related chips ) has to be addressed several times before they acknowledge their address */
        for ( i = 0; i < message->retries; i++ )
        {
            /* generate a start condition and address a i2c in write mode */
            I2C_GenerateSTART( i2c->port, ENABLE );

            /* wait till start condition is generated and the bus becomes free */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_MODE_SELECT, I2C_FLAG_CHECK_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                return PLATFORM_TIMEOUT;
            }

            if ( config->address_width == I2C_ADDRESS_WIDTH_7BIT )
            {
                /* send the address and R/W bit set to write of the requested i2c, wait for an acknowledge */
                I2C_Send7bitAddress( i2c->port, (uint8_t) ( config->address << 1 ), I2C_Direction_Transmitter );

                /* wait till address gets sent and the direction bit is sent and */
                result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
                if ( result == PLATFORM_SUCCESS )
                {
                    break;
                }
            }
            else
            {
                //TODO
                /* send 10 bits of the address and wait for an acknowledge */
            }
        }

        if ( i == message->retries )
        {
            return PLATFORM_TIMEOUT;
        }

        tmp_ptr = (const char*) message->tx_buffer;

        /* send data to the i2c i2c */
        for ( i = 0; i < message->tx_length; i++ )
        {
            I2C_SendData( i2c->port, (uint8_t) tmp_ptr[ i ] );

            /* wait till it actually gets transferred and acknowledged */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_FLAG_CHECK_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                return result;
            }
        }

        /* generate start condition again and address a i2c in read mode */
        /* some chips( authentication and security related chips ) has to be addressed several times before they acknowledge their address */
        for ( i = 0; i < message->retries; i++ )
        {
            /* generate a start condition */
            I2C_GenerateSTART( i2c->port, ENABLE );

            /* wait till start condition is generated and the bus becomes free */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_MODE_SELECT, I2C_FLAG_CHECK_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                return PLATFORM_TIMEOUT;
            }
            if ( config->address_width == I2C_ADDRESS_WIDTH_7BIT )
            {
                /* send the address and R/W bit set to write of the requested i2c, wait for an acknowledge */
                I2C_Send7bitAddress( i2c->port, (uint8_t) ( config->address << 1 ), I2C_Direction_Receiver );

                /* wait till address gets sent and the direction bit is sent and */
                result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
                if ( result != PLATFORM_SUCCESS )
                {
                    /* keep on pinging, if a i2c doesnt respond */
                    continue;
                }
                else
                {
                    break;
                }
            }
            else
            {
                //TODO
                /* send 10 bits of the address and wait for an acknowledge */
            }
        }
        if ( i == message->retries )
        {
            return PLATFORM_TIMEOUT;
        }

        /* receive data from the slave i2c */
        if ( message->rx_length == 1 )
        {
            /* disable acknowledgement before we start receiving bytes, this is a single byte transmission */
            I2C_AcknowledgeConfig( i2c->port, DISABLE );
        }
        else
        {
            /* enable acknowledgement before we start receiving bytes, this is a single byte transmission */
            I2C_AcknowledgeConfig( i2c->port, ENABLE );
        }
        tmp_rd_ptr = (uint8_t*) message->rx_buffer;
        /* start reading bytes */
        for ( i = 0; i < message->rx_length; i++ )
        {
            /* wait till something is in the i2c data register */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_BYTE_RECEIVED, I2C_FLAG_CHECK_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                return result;
            }

            /* get data */
            tmp_rd_ptr[ i ] = I2C_ReceiveData( i2c->port );
            if ( i == ( message->rx_length - 1 ) )
            {
                /* setup NACK for the last byte to be received */
                I2C_AcknowledgeConfig( i2c->port, DISABLE );
            }
            else
            {
                /* setup an acknowledgement beforehand for every byte that is to be received */
                I2C_AcknowledgeConfig( i2c->port, ENABLE );
            }
        }
    }
    else
    {

        /* read or write transaction */

        /* some chips( authentication and security related chips ) has to be addressed several times before they acknowledge their address */
        for ( i = 0; i < message->retries; i++ )
        {
            /* generate a start condition */
            I2C_GenerateSTART( i2c->port, ENABLE );

            /* wait till start condition is generated and the bus becomes free */
            result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_MODE_SELECT, I2C_FLAG_CHECK_TIMEOUT );
            if ( result != PLATFORM_SUCCESS )
            {
                return PLATFORM_TIMEOUT;
            }
            if ( config->address_width == I2C_ADDRESS_WIDTH_7BIT )
            {
                /* send the address of the requested i2c, wait for an acknowledge */
                I2C_Send7bitAddress( i2c->port, (uint8_t) ( config->address << 1 ), ( ( message->tx_buffer ) ? I2C_Direction_Transmitter : I2C_Direction_Receiver ) );

                /* wait till address gets sent and the direction bit is sent */
                if ( message->tx_buffer )
                {
                    result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
                }
                else
                {
                    result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, I2C_FLAG_CHECK_LONG_TIMEOUT );
                }
                if ( result != PLATFORM_SUCCESS )
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            else
            {
                //TODO
                /* send 10 bits of the address and wait for an acknowledge */
            }
        }
        if ( i == message->retries )
        {
            return PLATFORM_TIMEOUT;
        }

        if ( message->tx_buffer )
        {
            /* write transaction */

            const char* temp_ptr = (const char*) message->tx_buffer;
            /* send data to the i2c i2c */
            for ( i = 0; i < message->tx_length; i++ )
            {
                I2C_SendData( i2c->port, (uint8_t) temp_ptr[ i ] );

                /* wait till it actually gets transferred and acknowledged */
                result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_BYTE_TRANSMITTED, I2C_FLAG_CHECK_TIMEOUT );
                if ( result != PLATFORM_SUCCESS )
                {
                    return result;
                }
            }
        }
        else
        {
            /* read transaction */

            uint8_t* tmp_ptr = (uint8_t*) message->rx_buffer;
            if ( message->rx_length == 1 )
            {
                /* disable acknowledgement before we are going to receive a single byte */
                I2C_AcknowledgeConfig( i2c->port, DISABLE );
            }
            else
            {
                /* enable acknowledgement before we start receiving multiple bytes */
                I2C_AcknowledgeConfig( i2c->port, ENABLE );
            }
            /* receive data from the i2c i2c */
            for ( i = 0; i < message->rx_length; i++ )
            {
                /* wait till something is in the i2c data register */
                result = i2c_wait_for_event( i2c->port, I2C_EVENT_MASTER_BYTE_RECEIVED, I2C_FLAG_CHECK_TIMEOUT );
                if ( result != PLATFORM_SUCCESS )
                {
                    return result;
                }

                /* get data */
                tmp_ptr[ i ] = I2C_ReceiveData( i2c->port );
                if ( i == ( message->rx_length - 1 ) )
                {
                    /* setup NACK for the last byte to be received */
                    I2C_AcknowledgeConfig( i2c->port, DISABLE );
                }
                else
                {
                    /* setup an acknowledgement beforehand for every byte that is to be received */
                    I2C_AcknowledgeConfig( i2c->port, ENABLE );
                }
            }
        }
    }

    /* generate a stop condition */
    I2C_GenerateSTOP( i2c->port, ENABLE );
    return PLATFORM_SUCCESS;

}

platform_result_t platform_i2c_init_tx_message( platform_i2c_message_t* message, const void* tx_buffer, uint16_t tx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    wiced_assert( "bad argument", ( message != NULL ) && ( tx_buffer != NULL ) && ( tx_buffer_length != 0 ) );

    memset( message, 0x00, sizeof( *message ) );
    message->tx_buffer = tx_buffer;
    message->combined  = WICED_FALSE;
    message->retries   = retries;
    message->tx_length = tx_buffer_length;

    if ( disable_dma )
    {
        message->flags = I2C_MESSAGE_NO_DMA;
    }
    else
    {
        message->flags = I2C_MESSAGE_USE_DMA;
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_init_rx_message( platform_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    wiced_assert( "bad argument", ( message != NULL ) && ( rx_buffer != NULL ) && ( rx_buffer_length != 0 ) );

    memset( message, 0x00, sizeof( *message ) );

    message->rx_buffer = rx_buffer;
    message->combined  = WICED_FALSE;
    message->retries   = retries;
    message->rx_length = rx_buffer_length;

    if ( disable_dma )
    {
        message->flags = I2C_MESSAGE_NO_DMA;
    }
    else
    {
        message->flags = I2C_MESSAGE_USE_DMA;
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_init_combined_message( platform_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    wiced_assert( "bad argument", ( message != NULL ) && ( tx_buffer != NULL ) && ( tx_buffer_length != 0 ) && ( rx_buffer != NULL ) && ( rx_buffer_length != 0 ) );

    memset( message, 0x00, sizeof( *message ) );

    message->rx_buffer = rx_buffer;
    message->tx_buffer = tx_buffer;
    message->combined  = WICED_TRUE;
    message->retries   = retries;
    message->tx_length = tx_buffer_length;
    message->rx_length = rx_buffer_length;

    if ( disable_dma )
    {
        message->flags = I2C_MESSAGE_NO_DMA;
    }
    else
    {
        message->flags = I2C_MESSAGE_USE_DMA;
    }

    return PLATFORM_SUCCESS;
}

platform_result_t platform_i2c_transfer( const platform_i2c_t* i2c, const platform_i2c_config_t* config, platform_i2c_message_t* messages, uint16_t number_of_messages )
{
    platform_result_t result;
    int i = 0;

    wiced_assert( "bad argument", ( i2c != NULL ) && ( config != NULL ) && ( messages != 0 ) && ( number_of_messages != 0 ) );

    platform_mcu_powersave_disable();

    for ( i = 0; i < number_of_messages; i++ )
    {
        if ( ( config->flags & I2C_DEVICE_USE_DMA ) && ( ( messages[ i ].flags & I2C_MESSAGE_USE_DMA ) == 1 ) )
        {
            result = i2c_dma_transfer( i2c, config, &messages[ i ] );
            if ( result != PLATFORM_SUCCESS )
            {
                platform_mcu_powersave_enable();
                return PLATFORM_ERROR;
            }
        }
        else
        {
            result = i2c_transfer_message_no_dma( i2c, config, &messages[ i ] );
            if ( result != PLATFORM_SUCCESS )
            {
                platform_mcu_powersave_enable();
                return PLATFORM_ERROR;
            }
        }
    }

    platform_mcu_powersave_enable();
    return PLATFORM_SUCCESS;
}

