/**
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
#include "bt_bus.h"
#include "platform_config.h"
#include "wwd_assert.h"
#include "wiced.h"
#include "wiced_rtos.h"
#include "wiced_utilities.h"
#include "wiced_bt_platform.h"

/******************************************************
 *                      Macros
 ******************************************************/

/* Verify if WICED Platform API returns success.
 * Otherwise, returns the error code immediately.
 * Assert in DEBUG build.
 */
#define VERIFY_RETVAL( x ) \
do \
{ \
    wiced_result_t verify_result = (x); \
    if ( verify_result != WICED_SUCCESS ) \
    { \
        wiced_assert( "bus error", 0!=0 ); \
        return verify_result; \
    } \
} while( 0 )

/* Macro for checking of bus is initialised */
#define IS_BUS_INITIALISED( ) \
do \
{ \
    if ( bus_initialised == WICED_FALSE ) \
    { \
        wiced_assert( "bus uninitialised", 0!=0 ); \
        return WICED_ERROR; \
    } \
}while ( 0 )

/******************************************************
 *                    Constants
 ******************************************************/

/* Should be overridden by application. If undefined, set to 512 bytes. */
#ifndef BT_BUS_RX_FIFO_SIZE
#define BT_BUS_RX_FIFO_SIZE (512)
#endif

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

static volatile wiced_bool_t bus_initialised = WICED_FALSE;
static volatile wiced_bool_t device_powered  = WICED_FALSE;

/* RX ring buffer. Bluetooth chip UART receive can be asynchronous, therefore a ring buffer is required */
static volatile wiced_ring_buffer_t rx_ring_buffer;
static volatile uint8_t             rx_data[BT_BUS_RX_FIFO_SIZE];

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_bus_init( void )
{
    if ( bus_initialised == WICED_TRUE )
    {
       return WICED_BT_SUCCESS;
    }

#ifdef  WICED_USE_BT_HOST_WAKE_PIN
    VERIFY_RETVAL( platform_gpio_init( wiced_bt_control_pins[WICED_BT_PIN_HOST_WAKE], INPUT_HIGH_IMPEDANCE ) );
#endif

#ifdef  WICED_USE_BT_HOST_DEVICE_WAKE_PIN
    VERIFY_RETVAL( platform_gpio_init( wiced_bt_control_pins[WICED_BT_PIN_DEVICE_WAKE], OUTPUT_OPEN_DRAIN_PULL_UP ) );
    VERIFY_RETVAL( platform_gpio_output_high( wiced_bt_control_pins[WICED_BT_PIN_DEVICE_WAKE] ) );
    wiced_rtos_delay_milliseconds( 100 );
#endif

    /* Configure Reg Enable pin to output. Set to HIGH */
    VERIFY_RETVAL( platform_gpio_init( wiced_bt_control_pins[WICED_BT_PIN_POWER], OUTPUT_OPEN_DRAIN_PULL_UP ) );
    VERIFY_RETVAL( platform_gpio_output_high( wiced_bt_control_pins[WICED_BT_PIN_POWER] ) );
    device_powered = WICED_TRUE;

#ifdef  WICED_BT_USE_RESET_PIN
    /* Configure Reset pin to output. Set to HIGH */
    VERIFY_RETVAL( platform_gpio_init( wiced_bt_control_pins[WICED_BT_PIN_RESET], OUTPUT_OPEN_DRAIN_PULL_UP ) );
    VERIFY_RETVAL( platform_gpio_output_high( wiced_bt_control_pins[WICED_BT_PIN_RESET] ) );
#endif

    /* Configure RTS pin to output. Set to HIGH */
    VERIFY_RETVAL( platform_gpio_init( wiced_bt_uart_pins[WICED_BT_PIN_UART_RTS], OUTPUT_OPEN_DRAIN_PULL_UP ) );
    VERIFY_RETVAL( platform_gpio_output_high( wiced_bt_uart_pins[WICED_BT_PIN_UART_RTS] ) );

    /* Configure CTS pin to input pull-up */
    VERIFY_RETVAL( platform_gpio_init( wiced_bt_uart_pins[WICED_BT_PIN_UART_CTS], INPUT_PULL_UP ) );

    /* Initialise RX ring buffer */
    ring_buffer_init( (wiced_ring_buffer_t*) &rx_ring_buffer, (uint8_t*) rx_data, sizeof( rx_data ) );

    /* Configure USART comms */
    VERIFY_RETVAL( platform_uart_init( wiced_bt_uart_driver, wiced_bt_uart_peripheral, &wiced_bt_uart_config, (wiced_ring_buffer_t*) &rx_ring_buffer ) );

#ifdef  WICED_BT_USE_RESET_PIN
    /* Reset bluetooth chip. Delay momentarily. */
    VERIFY_RETVAL( platform_gpio_output_low( &wiced_bt_control_pins[WICED_BT_PIN_RESET] ) );
    wiced_rtos_delay_milliseconds( 10 );
    VERIFY_RETVAL( platform_gpio_output_high( &wiced_bt_control_pins[WICED_BT_PIN_RESET] ) );
#endif

    /* Wait until the Bluetooth chip stabilises.  */
    wiced_rtos_delay_milliseconds( 500 );

    /* Bluetooth chip is ready. Pull host's RTS low */
    VERIFY_RETVAL( platform_gpio_output_low( wiced_bt_uart_pins[WICED_BT_PIN_UART_RTS] ) );

    bus_initialised = WICED_TRUE;

    /* Wait for bluetooth chip to pull its RTS (host's CTS) low. From observation using CRO, it takes the bluetooth chip > 170ms to pull its RTS low after CTS low */
    BT_BUS_WAIT_UNTIL_READY();

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_bus_deinit( void )
{
    if ( bus_initialised == WICED_FALSE )
    {
        return WICED_BT_SUCCESS;
    }

#ifdef  WICED_BT_USE_RESET_PIN
    VERIFY_RETVAL( platform_gpio_output_low( wiced_bt_control_pins[WICED_BT_PIN_RESET] ) );
#endif
    VERIFY_RETVAL( platform_gpio_output_high( wiced_bt_uart_pins[WICED_BT_PIN_UART_RTS] ) ); // RTS deasserted
    VERIFY_RETVAL( platform_gpio_output_low ( wiced_bt_control_pins[WICED_BT_PIN_POWER] ) ); // Bluetooth chip regulator off
    device_powered = WICED_FALSE;

    /* Deinitialise UART */
    VERIFY_RETVAL( platform_uart_deinit( wiced_bt_uart_driver ) );
    bus_initialised = WICED_FALSE;

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_bus_transmit( const uint8_t* data_out, uint32_t size )
{
    IS_BUS_INITIALISED();

    BT_BUS_WAIT_UNTIL_READY();

    VERIFY_RETVAL( platform_uart_transmit_bytes( wiced_bt_uart_driver, data_out, size ) );

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_bus_receive( uint8_t* data_in, uint32_t size, uint32_t timeout_ms )
{
    IS_BUS_INITIALISED();

    return platform_uart_receive_bytes( wiced_bt_uart_driver, (void*)data_in, size, timeout_ms );
}

wiced_bool_t bt_bus_is_ready( void )
{
    return ( bus_initialised == WICED_FALSE ) ? WICED_FALSE : ( ( platform_gpio_input_get( wiced_bt_uart_pins[WICED_BT_PIN_UART_CTS] ) == WICED_TRUE ) ? WICED_FALSE : WICED_TRUE );
}

wiced_bool_t bt_bus_is_on( void )
{
    return device_powered;
}
