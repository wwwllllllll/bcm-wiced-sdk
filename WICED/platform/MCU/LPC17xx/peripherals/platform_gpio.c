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
 * common GPIO implementation
 */
#include "stdint.h"
#include "string.h"
#include "platform_peripheral.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos.h"
#include "wwd_assert.h"
#include "platform_config.h"
#include "wwd_platform_common.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define P0_INT_MASK (0x0001)
#define P2_INT_MASK (0x0004)
#define MAX_P0_IRQ_PIN 30
#define MAX_P2_IRQ_PIN 13
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

/* Structure of runtime GPIO IRQ data */
typedef struct
{
    platform_gpio_irq_handler_t     handler; // User callback
    void*                           arg;     // User argument to be passed to the callbackA
    platform_gpio_interrupt_trigger trigger;
} platform_gpio_irq_data_t;


/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Runtime GPIO IRQ data */
static volatile platform_gpio_irq_data_t p0_gpio_irq_data[MAX_P0_IRQ_PIN+1];
static volatile platform_gpio_irq_data_t p2_gpio_irq_data[MAX_P2_IRQ_PIN+1];

/******************************************************
 *               Function Definitions
 ******************************************************/
platform_result_t platform_gpio_irq_manager_init( void )
{
    memset( (void*)p0_gpio_irq_data, 0, sizeof( p0_gpio_irq_data ) );
    memset( (void*)p2_gpio_irq_data, 0, sizeof( p2_gpio_irq_data ) );

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_init( const platform_gpio_t* gpio, platform_pin_config_t config )
{
    uint16_t mode;
    uint8_t  direction;

    switch(config)
    {
        case INPUT_PULL_UP:
        {
            mode = IOCON_MODE_PULLUP;
            direction =LPC_PIN_INPUT;
            break;
        }
        case INPUT_PULL_DOWN:
        {
            mode = IOCON_MODE_PULLDOWN;
            direction =LPC_PIN_INPUT;
            break;
        }
        case INPUT_HIGH_IMPEDANCE:
        {
            mode = IOCON_MODE_INACT;
            direction =LPC_PIN_INPUT;
            break;
        }
        case OUTPUT_PUSH_PULL:
        {
            mode = IOCON_MODE_REPEATER;
            direction =LPC_PIN_OUTPUT;
            break;
        }
        case OUTPUT_OPEN_DRAIN_NO_PULL:
        {
            mode = IOCON_MODE_INACT;
            direction =LPC_PIN_OUTPUT;
            break;
        }
        case OUTPUT_OPEN_DRAIN_PULL_UP:
        {
            mode = IOCON_MODE_PULLUP;
            direction =LPC_PIN_OUTPUT;
            break;
        }
        default:
        {
            return PLATFORM_UNSUPPORTED;
        }

    }
    Chip_IOCON_PinMux(LPC_IOCON, gpio->port, gpio->pin, mode, IOCON_FUNC0);
    Chip_GPIO_WriteDirBit(LPC_GPIO, gpio->port, gpio->pin, direction );
    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_set_alternate_function( const platform_gpio_config_t* gpio)
{
    Chip_IOCON_PinMux(LPC_IOCON, gpio->config.port, gpio->config.pin, gpio->mode, gpio->function);
    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_output_high( const platform_gpio_t* gpio)
{
    Chip_GPIO_WritePortBit(LPC_GPIO, gpio->port, gpio->pin,true);
    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_output_low( const platform_gpio_t* gpio)
{
    Chip_GPIO_WritePortBit(LPC_GPIO, gpio->port, gpio->pin,false);
    return PLATFORM_SUCCESS;
}

wiced_bool_t platform_gpio_input_get( const platform_gpio_t* gpio )
{
    wiced_bool_t result;
    result = Chip_GPIO_ReadPortBit(LPC_GPIO,gpio->port, gpio->pin) ? WICED_TRUE : WICED_FALSE;
    return result;
}

platform_result_t platform_gpio_irq_enable( const platform_gpio_t* gpio, platform_gpio_irq_trigger_t trigger, platform_gpio_irq_handler_t handler, void* arg )
{
    volatile platform_gpio_irq_data_t* gpio_irq_data;

    if ( gpio->port == LPC_17xx_P0 )
    {
        /*Pins 12,13,14,31 can be used as interrupts as per data sheet*/
        if ( ( gpio->pin == 14 ) || ( gpio->pin == 13 ) || ( gpio->pin == 12 ) || ( gpio->pin == 31 ) )
        {
            return PLATFORM_UNSUPPORTED;
        }
        gpio_irq_data = p0_gpio_irq_data;
    }
    else if ( gpio->port == LPC_17xx_P2 )
    {
        if ( gpio->pin > MAX_P2_IRQ_PIN )
        {
            return PLATFORM_UNSUPPORTED;
        }
        gpio_irq_data = p2_gpio_irq_data;
    }
    else
    {
        return PLATFORM_BADARG;
    }
    gpio_irq_data[ gpio->pin ].handler = handler;
    gpio_irq_data[ gpio->pin ].arg     = arg;

    switch ( trigger )
    {
        case IRQ_TRIGGER_RISING_EDGE:
            gpio_irq_data[ gpio->pin ].trigger = GPIOPININT_RISING_EDGE;
            break;
        case IRQ_TRIGGER_FALLING_EDGE:
            gpio_irq_data[ gpio->pin ].trigger = GPIOPININT_FALLING_EDGE;
            break;
        case IRQ_TRIGGER_BOTH_EDGES:
            gpio_irq_data[ gpio->pin ].trigger = GPIOPININT_RISING_EDGE | GPIOPININT_FALLING_EDGE;
            break;
        default:
            return PLATFORM_BADARG;
            break;
    }
    NVIC_DisableIRQ( EINT3_IRQn );
    NVIC_ClearPendingIRQ( EINT3_IRQn );
    /*Note the LPC uses 5 bits for interrupt priority levels*/
    NVIC_EnableIRQ( EINT3_IRQn );

    IP_GPIOINT_IntCmd( LPC_GPIOINT, gpio->port, ( 1 << gpio->pin ), gpio_irq_data[ gpio->pin ].trigger );

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_irq_disable( const platform_gpio_t* gpio )
{
    volatile platform_gpio_irq_data_t* gpio_irq_data;

    if ( gpio->port == LPC_17xx_P0 )
    {
        gpio_irq_data = p0_gpio_irq_data;
    }
    else if ( gpio->port == LPC_17xx_P2 )
    {
        gpio_irq_data = p2_gpio_irq_data;
    }
    else
    {
        return PLATFORM_BADARG;
    }
    gpio_irq_data[ gpio->pin ].handler = NULL;
    gpio_irq_data[ gpio->pin ].arg = 0;
    gpio_irq_data[ gpio->pin ].trigger = 0;

    IP_GPIOINT_IntClear( LPC_GPIOINT, gpio->port, gpio->pin );
    return PLATFORM_SUCCESS;
}

/******************************************************
 *               IRQ Handler Definitions
 ******************************************************/

/* ***************************************************
 * Common IRQ handler for all GPIOs.
 * Add checks for interrupt GPIO here as required.
 * Currently only check for SPI irq is performed
 *****************************************************/
WWD_RTOS_DEFINE_ISR( gpio_irq )
{

    /*Add other interrupt GPIO as required*/
    if ( IP_GPIOINT_IntGetStatus( LPC_GPIOINT, wifi_spi_pins[WWD_PIN_SPI_IRQ].port, wifi_spi_pins[WWD_PIN_SPI_IRQ].pin, p2_gpio_irq_data[ wifi_spi_pins[WWD_PIN_SPI_IRQ].pin ].trigger ) )
    {
        if ( p2_gpio_irq_data[ wifi_spi_pins[WWD_PIN_SPI_IRQ].pin ].handler != NULL )
        {
            IP_GPIOINT_IntClear( LPC_GPIOINT, wifi_spi_pins[WWD_PIN_SPI_IRQ].port, ( 1 << wifi_spi_pins[WWD_PIN_SPI_IRQ].pin ) );
            void* arg = p2_gpio_irq_data[ wifi_spi_pins[WWD_PIN_SPI_IRQ].pin ].arg; /* Avoids undefined order of access to volatiles */
            p2_gpio_irq_data[ wifi_spi_pins[WWD_PIN_SPI_IRQ].pin ].handler( arg );
        }
    }
}

/******************************************************
 *               IRQ Handler Mapping
 ******************************************************/

WWD_RTOS_MAP_ISR( gpio_irq , EINT3_irq )
