/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
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

#define PINS_PER_PORT (32) /* Px00 to Px31 */
#define TOTAL_PORTS   ( 3) /* PIOA to C    */

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)
typedef struct
{
    wiced_bool_t                 wakeup_pin;
    platform_gpio_irq_callback_t callback;
    void*                        arg;
} sam4s_gpio_irq_data_t;
#pragma pack()

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Flag to indicate whether active GPIO IRQ data is initialised */
static wiced_bool_t active_gpio_irq_data_initted = WICED_FALSE;

/* Pointer to active GPIO IRQs */
static sam4s_gpio_irq_data_t active_gpio_irq_data[TOTAL_PORTS * PINS_PER_PORT];

///* GPIO IRQ interrupt vectors */
static const IRQn_Type const irq_vectors[] =
{
    [0] = PIOA_IRQn,
    [1] = PIOB_IRQn,
    [2] = PIOC_IRQn,
};

/******************************************************
 *             SAM4S Function Definitions
 ******************************************************/

platform_result_t platform_gpio_init( const platform_gpio_t* gpio, platform_pin_config_t config )
{
    uint8_t       direction; /* direction: IOPORT_DIR_INPUT or OUTPUT */
    ioport_mode_t mode;      /* pin mode:  see ioport_pio.h           */

    UNUSED_PARAMETER(gpio);
    UNUSED_PARAMETER(config);

    platform_mcu_powersave_disable();

    switch ( config )
    {
        case INPUT_PULL_UP:
        {
            direction = IOPORT_DIR_INPUT;
            mode      = IOPORT_MODE_PULLUP;
            break;
        }
        case INPUT_PULL_DOWN:
        {
            direction = IOPORT_DIR_INPUT;
            mode      = IOPORT_MODE_PULLDOWN;
            break;
        }
        case INPUT_HIGH_IMPEDANCE:
        {
            direction = IOPORT_DIR_INPUT;
            mode      = 0;
            break;
        }
        case OUTPUT_PUSH_PULL:
        {
            direction = IOPORT_DIR_OUTPUT;
            mode      = 0;
            break;
        }
        case OUTPUT_OPEN_DRAIN_NO_PULL:
        {
            direction = IOPORT_DIR_OUTPUT;
            mode      = IOPORT_MODE_OPEN_DRAIN;
            break;
        }
        case OUTPUT_OPEN_DRAIN_PULL_UP:
        {
            direction = IOPORT_DIR_OUTPUT;
            mode      = IOPORT_MODE_OPEN_DRAIN | IOPORT_MODE_PULLUP;
            break;
        }
        default:
        {
            platform_mcu_powersave_enable();
            return PLATFORM_BADARG;
        }
    }

    ioport_enable_pin  ( gpio->pin );
    ioport_set_pin_mode( gpio->pin, mode );
    ioport_set_pin_dir ( gpio->pin, direction );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_peripheral_pin_init( const platform_gpio_t* gpio, ioport_mode_t pin_mode )
{
    UNUSED_PARAMETER(gpio);
    UNUSED_PARAMETER(pin_mode);

    platform_mcu_powersave_disable( );

    /* Set pin mode and disable GPIO peripheral */
    ioport_set_pin_mode( gpio->pin, pin_mode );
    ioport_disable_pin( gpio->pin );

    platform_mcu_powersave_enable( );
    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_deinit( const platform_gpio_t* gpio )
{
    UNUSED_PARAMETER(gpio);
    platform_mcu_powersave_disable( );
    ioport_disable_pin( gpio->pin );
    platform_mcu_powersave_enable( );
    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_output_high( const platform_gpio_t* gpio )
{
    UNUSED_PARAMETER(gpio);
    wiced_assert( "bad argument", ( gpio != NULL ) );

    platform_mcu_powersave_disable( );

    ioport_set_pin_level( gpio->pin, IOPORT_PIN_LEVEL_HIGH );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_output_low ( const platform_gpio_t* gpio )
{
    UNUSED_PARAMETER(gpio);
    wiced_assert( "bad argument", ( gpio != NULL ) );

    platform_mcu_powersave_disable( );

    ioport_set_pin_level( gpio->pin, IOPORT_PIN_LEVEL_LOW );
    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

wiced_bool_t platform_gpio_input_get( const platform_gpio_t* gpio )
{
    UNUSED_PARAMETER(gpio);
    wiced_bool_t result;

    wiced_assert( "bad argument", ( gpio != NULL ) );

    platform_mcu_powersave_disable();
    result = ( ioport_get_pin_level( gpio->pin ) == false ) ? WICED_FALSE : WICED_TRUE;
    platform_mcu_powersave_enable();

    return result;
}

platform_result_t platform_gpio_irq_enable ( const platform_gpio_t* gpio, platform_gpio_irq_trigger_t trigger, platform_gpio_irq_callback_t handler, void* arg )
{
    ioport_port_mask_t           mask               = ioport_pin_to_mask( gpio->pin );
    ioport_port_t                port               = ioport_pin_to_port_id( gpio->pin );
    volatile Pio*                port_register      = arch_ioport_port_to_base( port );
    uint32_t                     temp;
    uint8_t                      sam4s_irq_trigger;



    UNUSED_PARAMETER(gpio);
    UNUSED_PARAMETER(temp);
    UNUSED_PARAMETER(trigger);
    UNUSED_PARAMETER(handler);
    UNUSED_PARAMETER(arg);

    platform_mcu_powersave_disable();

    if ( active_gpio_irq_data_initted == WICED_FALSE )
    {
        /* Initialise the first time this function is called */
        memset( &active_gpio_irq_data_initted, 0, sizeof( active_gpio_irq_data_initted ) );
        active_gpio_irq_data_initted = WICED_TRUE;
    }

//    /* Reset and enable associated CPU interrupt vector */
    NVIC_DisableIRQ( irq_vectors[port] );
    NVIC_ClearPendingIRQ( irq_vectors[port] );

    /* Set priority in platform.c file !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    /* Set priority in platform.c file !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    /* Set priority in platform.c file !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
//    NVIC_SetPriority( irq_vectors[port], SAM4S_GPIO_IRQ_PRIO );

    active_gpio_irq_data[gpio->pin].wakeup_pin = gpio->wakeup_pin_config->is_wakeup_pin;
    active_gpio_irq_data[gpio->pin].arg        = arg;
    active_gpio_irq_data[gpio->pin].callback   = handler;


    switch ( trigger )
    {
        case IRQ_TRIGGER_RISING_EDGE:  sam4s_irq_trigger = IOPORT_SENSE_RISING;    break;
        case IRQ_TRIGGER_FALLING_EDGE: sam4s_irq_trigger = IOPORT_SENSE_FALLING;   break;
        case IRQ_TRIGGER_BOTH_EDGES:   sam4s_irq_trigger = IOPORT_SENSE_BOTHEDGES; break;
        default:
            return PLATFORM_BADARG;
    }

    if( gpio->wakeup_pin_config->is_wakeup_pin == WICED_TRUE )
    {
        platform_powersave_enable_wakeup_pin( gpio->wakeup_pin_config );
    }

    if ( sam4s_irq_trigger == IOPORT_SENSE_RISING || sam4s_irq_trigger == IOPORT_SENSE_BOTHEDGES )
    {
       port_register->PIO_AIMER  |= mask;
       port_register->PIO_ESR    |= mask;
       port_register->PIO_REHLSR |= mask;
    }

    if ( sam4s_irq_trigger == IOPORT_SENSE_FALLING || sam4s_irq_trigger == IOPORT_SENSE_BOTHEDGES )
    {
        port_register->PIO_AIMER  |= mask;
        port_register->PIO_ESR    |= mask;
        port_register->PIO_FELLSR |= mask;
    }

    /* Read ISR to clear residual interrupt status */
    temp = port_register->PIO_ISR;

    /* Enable interrupt source */
    port_register->PIO_IER |= mask;

    NVIC_EnableIRQ( irq_vectors[port] );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_gpio_irq_disable( const platform_gpio_t* gpio )
{
    ioport_port_mask_t mask          = ioport_pin_to_mask   ( gpio->pin );
    ioport_port_t      port          = ioport_pin_to_port_id( gpio->pin );
    volatile Pio*      port_register = arch_ioport_port_to_base( port );

    /* Disable interrupt on pin */
    port_register->PIO_IDR = mask;

    /* Disable Cortex-M interrupt vector as well if no pin interrupt is enabled */
    if ( port_register->PIO_IMR == 0 )
    {
        NVIC_DisableIRQ( irq_vectors[port] );
    }

    return PLATFORM_SUCCESS;
}

/******************************************************
 *                  ISR Definitions
 ******************************************************/

WWD_RTOS_DEFINE_ISR(gpio_irq)
{
    ioport_port_t port;

    for ( port = 0; port < TOTAL_PORTS; port++ )
    {
        volatile Pio* port_register = arch_ioport_port_to_base( port );
        uint32_t      status        = port_register->PIO_ISR; /* Get interrupt status. Read clears the interrupt */
        uint32_t      mask          = port_register->PIO_IMR;
        uint32_t      iter          = 0;

        if ( ( status != 0 ) && ( mask != 0 ) )
        {
            /* Call the respective GPIO interrupt handler/callback */
            for ( iter = 0; iter < PINS_PER_PORT; iter++, status >>= 1, mask >>= 1 )
            {
                uint32_t gpio_irq_data_index = (port * PINS_PER_PORT) + iter;

                if ( ( ( mask & 0x1 ) != 0 ) && ( ( status & 0x1 ) != 0 ) && ( active_gpio_irq_data[gpio_irq_data_index].callback != NULL ) )
                {
                    if ( active_gpio_irq_data[gpio_irq_data_index].wakeup_pin == WICED_TRUE )
                    {
                        platform_mcu_powersave_exit_notify();
                    }
                    active_gpio_irq_data[gpio_irq_data_index].callback( active_gpio_irq_data[gpio_irq_data_index].arg );
                }
            }
        }
    }
}

/******************************************************
 *             IRQ Handler Mapping
 ******************************************************/

WWD_RTOS_MAP_ISR( gpio_irq , PIO_CTRL_A_irq )
WWD_RTOS_MAP_ISR( gpio_irq , PIO_CTRL_B_irq )
WWD_RTOS_MAP_ISR( gpio_irq , PIO_CTRL_C_irq )

