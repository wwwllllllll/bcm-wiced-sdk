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
#include "wiced.h"
#include "gpio_keypad.h"
#include "wiced_time.h"
#include "wiced_rtos.h"
#include "wiced_utilities.h"
#include "wwd_constants.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define DEBOUNCE_TIME_MS         (150)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct gpio_key_internal
{
    gpio_key_t*                  key;
    uint32_t                     last_irq_timestamp;
    struct gpio_keypad_internal* owner;
}gpio_key_internal_t;

typedef struct gpio_keypad_internal
{
    uint32_t               total_keys;
    gpio_key_t*            key_list;
    gpio_keypad_handler_t  function;
    wiced_worker_thread_t* thread;
    gpio_key_internal_t*   current_key_pressed;
    wiced_timer_t          check_state_timer;
}gpio_keypad_internal_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void           gpio_interrupt_key_handler( void* arg );
static void           check_state_timer_handler ( void* arg );
static wiced_result_t key_pressed_event_handler ( void* arg );
static wiced_result_t key_held_event_handler    ( void* arg );
static wiced_result_t key_released_event_handler( void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t gpio_keypad_enable( gpio_keypad_t* keypad, wiced_worker_thread_t* thread, gpio_keypad_handler_t function, uint32_t held_event_interval_ms, uint32_t total_keys, const gpio_key_t* key_list )
{
    gpio_key_internal_t* key_internal;
    uint32_t malloc_size;
    uint32_t i;

    if ( !keypad || !thread || !function || !total_keys || !key_list )
    {
        return WICED_BADARG;
    }

    malloc_size      = sizeof(gpio_keypad_internal_t) + total_keys * sizeof(gpio_key_internal_t);
    keypad->internal = (gpio_keypad_internal_t*) malloc_named("key internal", malloc_size);
    if ( !keypad->internal )
    {
        return WICED_ERROR;
    }

    memset( keypad->internal, 0, sizeof( *( keypad->internal ) ) );

    key_internal = (gpio_key_internal_t*)((uint8_t*)keypad->internal + sizeof(gpio_keypad_internal_t));

    keypad->internal->function   = function;
    keypad->internal->thread     = thread;
    keypad->internal->total_keys = total_keys;
    keypad->internal->key_list   = (gpio_key_t*)key_list;

    wiced_rtos_init_timer( &keypad->internal->check_state_timer, held_event_interval_ms, check_state_timer_handler, (void*) keypad->internal );

    for ( i = 0; i < total_keys; i++ )
    {
        wiced_gpio_irq_trigger_t trigger = ( key_list[i].polarity == KEY_POLARITY_HIGH ) ? IRQ_TRIGGER_FALLING_EDGE : IRQ_TRIGGER_RISING_EDGE;

        key_internal[i].key   = (gpio_key_t*)&key_list[i];
        key_internal[i].owner = keypad->internal;
        key_internal[i].last_irq_timestamp = 0;

        wiced_gpio_init( key_list[i].gpio, ( ( key_list[i].polarity == KEY_POLARITY_HIGH ) ? INPUT_PULL_UP : INPUT_PULL_DOWN ) );
        wiced_gpio_input_irq_enable( key_list[i].gpio, trigger, gpio_interrupt_key_handler, (void*)&key_internal[i] );
    }

    return WICED_SUCCESS;
}

wiced_result_t gpio_keypad_disable( gpio_keypad_t *keypad )
{
    uint32_t i;

    malloc_transfer_to_curr_thread(keypad->internal);

    wiced_rtos_deinit_timer( &keypad->internal->check_state_timer );

    for ( i = 0; i < keypad->internal->total_keys; i++ )
    {
        wiced_gpio_input_irq_disable( keypad->internal->key_list[i].gpio );
    }

    free( keypad->internal );
    keypad->internal = 0;

    malloc_leak_check( NULL, LEAK_CHECK_THREAD );
    return WICED_SUCCESS;
}

static void gpio_interrupt_key_handler( void* arg )
{
    gpio_key_internal_t* key = (gpio_key_internal_t*)arg;

    if ( key != NULL )
    {
        gpio_keypad_internal_t* keypad = key->owner;
        uint32_t current_time;

        wiced_watchdog_kick( );
        wiced_time_get_time( &current_time );

        if ( ( current_time - key->last_irq_timestamp > DEBOUNCE_TIME_MS ) && ( keypad->current_key_pressed == 0 ) )
        {
            wiced_time_get_time( &key->last_irq_timestamp );
            wiced_rtos_send_asynchronous_event( keypad->thread, key_pressed_event_handler, (void*)key );
        }
    }
}

static void check_state_timer_handler( void* arg )
{
    gpio_keypad_internal_t* keypad = (gpio_keypad_internal_t*)arg;

    if ( keypad != NULL )
    {
        gpio_key_internal_t* key = keypad->current_key_pressed;

        if ( key != NULL )
        {
            if ( key->key->polarity == KEY_POLARITY_HIGH )
            {
                if ( wiced_gpio_input_get( key->key->gpio ) == 0 )
                {
                    wiced_rtos_send_asynchronous_event( keypad->thread, key_held_event_handler, (void*)key );
                }
                else
                {
                    wiced_rtos_send_asynchronous_event( keypad->thread, key_released_event_handler, (void*)key );
                }
            }
            else
            {
                if ( wiced_gpio_input_get( key->key->gpio ) == 1 )
                {
                    wiced_rtos_send_asynchronous_event( keypad->thread, key_held_event_handler, (void*)key );
                }
                else
                {
                    wiced_rtos_send_asynchronous_event( keypad->thread, key_released_event_handler, (void*)key );
                }
            }
        }
    }
}

static wiced_result_t key_pressed_event_handler( void* arg )
{
    gpio_key_internal_t* key = (gpio_key_internal_t*)arg;

    if ( key != NULL )
    {
        gpio_keypad_internal_t* keypad = key->owner;

        if ( keypad->current_key_pressed == NULL )
        {
            keypad->current_key_pressed = key;

            wiced_rtos_start_timer( &keypad->check_state_timer );

            keypad->function( key->key->code, KEY_EVENT_PRESSED );

            return WICED_SUCCESS;
        }
    }

    return WICED_ERROR;
}

static wiced_result_t key_held_event_handler( void* arg )
{
    gpio_key_internal_t* key = (gpio_key_internal_t*)arg;

    if ( key != NULL )
    {
        gpio_keypad_internal_t* keypad = key->owner;

        if ( keypad->current_key_pressed == key )
        {
            keypad->function( key->key->code, KEY_EVENT_HELD );

            return WICED_SUCCESS;
        }
    }

    return WICED_ERROR;
}

static wiced_result_t key_released_event_handler( void* arg )
{
    gpio_key_internal_t* key = (gpio_key_internal_t*)arg;

    if ( key != NULL )
    {
        gpio_keypad_internal_t* keypad = key->owner;

        if ( keypad->current_key_pressed == key )
        {
            keypad->current_key_pressed = 0;

            wiced_rtos_stop_timer( &keypad->check_state_timer );

            keypad->function( key->key->code, KEY_EVENT_RELEASED );

            return WICED_SUCCESS;
        }
    }

    return WICED_ERROR;
}
