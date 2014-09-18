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
 * Defines common WICED Platform API
 */
#include <stdint.h>
#include <string.h>
#include "wwd_assert.h"
#include "platform_peripheral.h"
#include "wiced_result.h"
#include "wiced_platform.h"

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

extern wiced_result_t wiced_platform_init( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Externed from platforms/<Platform>/platform.c */
extern const platform_gpio_t       platform_gpio_pins[];
extern const platform_adc_t        platform_adc_peripherals[];
extern const platform_i2c_t        platform_i2c_peripherals[];
extern const platform_pwm_t        platform_pwm_peripherals[];
extern const platform_spi_t        platform_spi_peripherals[];
extern const platform_uart_t       platform_uart_peripherals[];
extern platform_uart_driver_t      platform_uart_drivers[];
extern platform_spi_slave_driver_t platform_spi_slave_drivers[];

wiced_bool_t i2c_initialized[WICED_I2C_MAX];

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_platform_init( void )
{
    WPRINT_PLATFORM_INFO( ("Platform " PLATFORM " initialised\n") );

    if ( WICED_TRUE == platform_watchdog_check_last_reset( ) )
    {
        WPRINT_PLATFORM_ERROR( ("WARNING: Watchdog reset occured previously. Please see platform_watchdog.c for debugging instructions.\n") );
    }

    memset(i2c_initialized, 0, sizeof(i2c_initialized));

    return WICED_SUCCESS;
}

wiced_result_t wiced_adc_init( wiced_adc_t adc, uint32_t sample_cycle )
{
    return platform_adc_init( &platform_adc_peripherals[adc], sample_cycle );
}

wiced_result_t wiced_adc_deinit( wiced_adc_t adc )
{
    return platform_adc_deinit( &platform_adc_peripherals[adc] );
}

wiced_result_t wiced_adc_take_sample( wiced_adc_t adc, uint16_t* output )
{
    return platform_adc_take_sample( &platform_adc_peripherals[adc], output );
}

wiced_result_t wiced_adc_take_sample_stream( wiced_adc_t adc, void* buffer, uint16_t buffer_length )
{
    return platform_adc_take_sample_stream( &platform_adc_peripherals[adc], buffer, buffer_length );
}

wiced_result_t wiced_gpio_init( wiced_gpio_t gpio, wiced_gpio_config_t configuration )
{
    return platform_gpio_init( &platform_gpio_pins[gpio], configuration );
}

wiced_result_t wiced_gpio_output_high( wiced_gpio_t gpio )
{
    return platform_gpio_output_high( &platform_gpio_pins[gpio] );
}

wiced_result_t wiced_gpio_output_low( wiced_gpio_t gpio )
{
    return platform_gpio_output_low( &platform_gpio_pins[gpio] );
}

wiced_bool_t wiced_gpio_input_get( wiced_gpio_t gpio )
{
    return platform_gpio_input_get( &platform_gpio_pins[gpio] );
}

wiced_result_t wiced_gpio_input_irq_enable( wiced_gpio_t gpio, wiced_gpio_irq_trigger_t trigger, wiced_gpio_irq_handler_t handler, void* arg )
{
    return platform_gpio_irq_enable( &platform_gpio_pins[gpio], trigger, handler, arg );
}

wiced_result_t wiced_gpio_input_irq_disable( wiced_gpio_t gpio )
{
    return platform_gpio_irq_disable( &platform_gpio_pins[gpio] );
}

wiced_result_t wiced_i2c_init( wiced_i2c_device_t* device )
{
    platform_i2c_config_t config;
    wiced_result_t result;

    if (i2c_initialized[device->port] == WICED_TRUE)
    {
        return WICED_SUCCESS;
    }


    config.address       = device->address;
    config.address_width = device->address_width;
    config.flags         = device->flags;
    config.speed_mode    = device->speed_mode;

    result = platform_i2c_init( &platform_i2c_peripherals[device->port], &config );
    if (result == WICED_SUCCESS)
    {
        i2c_initialized[device->port] = WICED_TRUE;
    }
    return result;
}

wiced_result_t wiced_i2c_deinit( wiced_i2c_device_t* device )
{
    platform_i2c_config_t config;

    config.address       = device->address;
    config.address_width = device->address_width;
    config.flags         = device->flags;
    config.speed_mode    = device->speed_mode;

    i2c_initialized[device->port] = WICED_FALSE;

    return platform_i2c_deinit( &platform_i2c_peripherals[device->port], &config );
}

wiced_bool_t wiced_i2c_probe_device( wiced_i2c_device_t* device, int retries )
{
    platform_i2c_config_t config;

    config.address       = device->address;
    config.address_width = device->address_width;
    config.flags         = device->flags;
    config.speed_mode    = device->speed_mode;

    return platform_i2c_probe_device( &platform_i2c_peripherals[device->port], &config, retries );
}

wiced_result_t wiced_i2c_init_tx_message( wiced_i2c_message_t* message, const void* tx_buffer, uint16_t tx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    return platform_i2c_init_tx_message( message, tx_buffer, tx_buffer_length, retries, disable_dma );
}

wiced_result_t wiced_i2c_init_rx_message( wiced_i2c_message_t* message, void* rx_buffer, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    return platform_i2c_init_rx_message( message, rx_buffer, rx_buffer_length, retries, disable_dma );
}

wiced_result_t wiced_i2c_init_combined_message( wiced_i2c_message_t* message, const void* tx_buffer, void* rx_buffer, uint16_t tx_buffer_length, uint16_t rx_buffer_length, uint16_t retries, wiced_bool_t disable_dma )
{
    return platform_i2c_init_combined_message( message, tx_buffer, rx_buffer, tx_buffer_length, rx_buffer_length, retries, disable_dma );
}

wiced_result_t wiced_i2c_transfer( wiced_i2c_device_t* device, wiced_i2c_message_t* messages, uint16_t number_of_messages )
{
    platform_i2c_config_t config;

    config.address       = device->address;
    config.address_width = device->address_width;
    config.flags         = device->flags;
    config.speed_mode    = device->speed_mode;

    return platform_i2c_transfer( &platform_i2c_peripherals[device->port], &config, messages, number_of_messages );
}

void wiced_platform_mcu_enable_powersave( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    platform_mcu_powersave_enable( );
#endif
}

void wiced_platform_mcu_disable_powersave( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    platform_mcu_powersave_disable( );
#endif
}

wiced_result_t wiced_pwm_init( wiced_pwm_t pwm_peripheral, uint32_t frequency, float duty_cycle )
{
    return platform_pwm_init( &platform_pwm_peripherals[pwm_peripheral], frequency, duty_cycle );
}

wiced_result_t wiced_pwm_start( wiced_pwm_t pwm )
{
    return platform_pwm_start( &platform_pwm_peripherals[pwm] );
}

wiced_result_t wiced_pwm_stop( wiced_pwm_t pwm )
{
    return platform_pwm_stop( &platform_pwm_peripherals[pwm] );
}

wiced_result_t wiced_platform_get_rtc_time( wiced_rtc_time_t* time )
{
    return platform_rtc_get_time( time );
}

wiced_result_t wiced_platform_set_rtc_time( const wiced_rtc_time_t* time )
{
    return platform_rtc_set_time( time );
}

wiced_result_t wiced_spi_init( const wiced_spi_device_t* spi )
{
    platform_spi_config_t config;

    config.chip_select = &platform_gpio_pins[spi->chip_select];
    config.speed       = spi->speed;
    config.mode        = spi->mode;
    config.bits        = spi->bits;

    return platform_spi_init( &platform_spi_peripherals[spi->port], &config );
}

wiced_result_t wiced_spi_deinit( const wiced_spi_device_t* spi )
{
    platform_spi_config_t config;

    config.chip_select = &platform_gpio_pins[spi->chip_select];
    config.speed       = spi->speed;
    config.mode        = spi->mode;
    config.bits        = spi->bits;

    return platform_spi_init( &platform_spi_peripherals[spi->port], &config );
}

wiced_result_t wiced_spi_transfer( const wiced_spi_device_t* spi, const wiced_spi_message_segment_t* segments, uint16_t number_of_segments )
{
    platform_spi_config_t config;

    config.chip_select = &platform_gpio_pins[spi->chip_select];
    config.speed       = spi->speed;
    config.mode        = spi->mode;
    config.bits        = spi->bits;

    return platform_spi_transfer( &platform_spi_peripherals[spi->port], &config, segments, number_of_segments );
}

wiced_result_t wiced_spi_slave_init( wiced_spi_t spi, const wiced_spi_slave_config_t* config )
{
    return platform_spi_slave_init( &platform_spi_slave_drivers[spi], &platform_spi_peripherals[spi], config );
}

wiced_result_t wiced_spi_slave_deinit( wiced_spi_t spi )
{
    return platform_spi_slave_deinit( &platform_spi_slave_drivers[spi] );
}

wiced_result_t wiced_spi_slave_send_error_status( wiced_spi_t spi, wiced_spi_slave_transfer_status_t error_status )
{
    return platform_spi_slave_send_error_status( &platform_spi_slave_drivers[spi], error_status );
}

wiced_result_t wiced_spi_slave_receive_command( wiced_spi_t spi, wiced_spi_slave_command_t* command, uint32_t timeout_ms )
{
    return platform_spi_slave_receive_command( &platform_spi_slave_drivers[spi], command, timeout_ms );
}

wiced_result_t wiced_spi_slave_transfer_data( wiced_spi_t spi, wiced_spi_slave_transfer_direction_t direction, wiced_spi_slave_data_buffer_t* buffer, uint32_t timeout_ms )
{
    return platform_spi_slave_transfer_data( &platform_spi_slave_drivers[spi], direction, buffer, timeout_ms );
}

wiced_result_t wiced_spi_slave_generate_interrupt( wiced_spi_t spi, uint32_t pulse_duration_ms )
{
    return platform_spi_slave_generate_interrupt( &platform_spi_slave_drivers[spi], pulse_duration_ms );
}

wiced_result_t wiced_uart_init( wiced_uart_t uart, const wiced_uart_config_t* config, wiced_ring_buffer_t* optional_rx_buffer )
{
#ifndef WICED_DISABLE_STDIO
    /* Interface is used by STDIO. Uncomment WICED_DISABLE_STDIO to overcome this */
    if ( uart == STDIO_UART )
    {
        return WICED_ERROR;
    }
#endif

    return platform_uart_init( &platform_uart_drivers[uart], &platform_uart_peripherals[uart], config, optional_rx_buffer );
}

wiced_result_t wiced_uart_deinit( wiced_uart_t uart )
{
    return platform_uart_deinit( &platform_uart_drivers[uart] );
}

wiced_result_t wiced_uart_transmit_bytes( wiced_uart_t uart, const void* data, uint32_t size )
{
    return platform_uart_transmit_bytes( &platform_uart_drivers[uart], (const uint8_t*) data, size );
}

wiced_result_t wiced_uart_receive_bytes( wiced_uart_t uart, void* data, uint32_t size, uint32_t timeout )
{
    return platform_uart_receive_bytes( &platform_uart_drivers[uart], (uint8_t*)data, size, timeout );
}

wiced_result_t wiced_watchdog_kick( void )
{
    return platform_watchdog_kick( );
}

uint64_t wiced_get_nanosecond_clock_value( void )
{
    return platform_get_nanosecond_clock_value( );
}

void wiced_deinit_nanosecond_clock( void )
{
    platform_deinit_nanosecond_clock( );
}

void wiced_reset_nanosecond_clock( void )
{
    platform_reset_nanosecond_clock( );
}

void wiced_init_nanosecond_clock( void )
{
    platform_init_nanosecond_clock( );
}
