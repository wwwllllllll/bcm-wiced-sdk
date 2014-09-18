/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include "wiced.h"
#include "wiced_rtos.h"
#include "wiced_time.h"
#include "wiced_utilities.h"
#include "wwd_constants.h"
#include "button.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define SPURIOUS_INT_MS             (50)
#define DEBOUNCE_TIME_MS            (150)
#define HELD_EVENT_TIME_MS          (250)
#define SHORT_EVT_DURATION_MIN      (150)
#define SHORT_EVT_DURATION_MAX      (1000)
#define LONG_EVT_DURATION_MIN       (1000)
#define LONG_EVT_DURATION_MAX       (3000)
#define VERY_LONG_EVT_DURATION_MIN  (3000)
#define VERY_LONG_EVT_DURATION_MAX  (5000)
#define EXTENDED_EVT_DURATION_MIN   (5000)
#define EXTENDED_EVT_DURATION_MAX   (8000)
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
 *                 Global Variables
 ******************************************************/
wiced_button_list_t *buttons_list;
wiced_button_t    *buttons;

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static void           button_interrupt_handler( void* arg );
static wiced_result_t button_event_handler( void* arg );

wiced_result_t wiced_button_init( wiced_button_list_t *button_list )
{
    wiced_result_t ret = WICED_SUCCESS;
    uint8_t i;

    if ( !button_list || !button_list->buttons || (button_list->nr_buttons == 0) )
    {
        return WICED_BADARG;
    }

    buttons_list = button_list;
    buttons = button_list->buttons;
    buttons_list->active = 0;
    buttons_list->thread = WICED_HARDWARE_IO_WORKER_THREAD;
    WPRINT_LIB_INFO( ( "Initializng WICED buttons...\n" ) );
    //wiced_rtos_init_timer( &buttons_list->check_state_timer, HELD_EVENT_TIME_MS, check_state_timer_handler, NULL );
    for ( i = 0 ; i < buttons_list->nr_buttons; i++ )
    {
        //wiced_gpio_irq_trigger_t trigger = ( buttons[i].polarity == BUTTON_POLARITY_HIGH ) ? IRQ_TRIGGER_FALLING_EDGE : IRQ_TRIGGER_RISING_EDGE;
        wiced_gpio_irq_trigger_t trigger = IRQ_TRIGGER_BOTH_EDGES;
        wiced_gpio_init( buttons[i].gpio_number, ( ( buttons[i].polarity == BUTTON_POLARITY_HIGH ) ? INPUT_PULL_UP : INPUT_PULL_DOWN ) );

        buttons[i].last_irq_timestamp = 0;
        buttons[i].owner = buttons_list;
        buttons[i].current_state = BUTTON_RELEASED;
        buttons[i].last_position = BUTTON_POSITION_RELEASED;
        buttons[i].position = BUTTON_POSITION_RELEASED;
        buttons[i].is_pressed = 0;
        wiced_gpio_input_irq_enable( buttons[i].gpio_number, trigger, button_interrupt_handler, (void*)&buttons[i] );
    }

    return ret;

}

static void button_interrupt_handler( void* arg )
{
    wiced_button_t* button = (wiced_button_t*)arg;
    wiced_button_list_t *blist = button->owner;
    wiced_bool_t gpio_value = 0;
    wiced_button_position_t new_position;
    uint32_t current_time;

    if ( !button || !blist )
        return;

    wiced_watchdog_kick( );
    wiced_time_get_time( &current_time );

    gpio_value  = wiced_gpio_input_get( button->gpio_number );
    new_position = (gpio_value & 0x1) ? BUTTON_POSITION_RELEASED : BUTTON_POSITION_PRESSED;

    if ( ( (current_time - button->last_irq_timestamp) < SPURIOUS_INT_MS) && button->last_position == new_position )
        return ;

    if ( ( current_time - button->last_irq_timestamp > DEBOUNCE_TIME_MS ) || button->last_position != new_position )
    {
        if ( gpio_value & 0x1 )
        {
            WPRINT_LIB_INFO ( ( "Button:%u Released @time:%u\n", (unsigned int)button->id,\
                                    (unsigned int)current_time ) );
            button->is_pressed = 0 ;
            button->position = BUTTON_POSITION_RELEASED;
            button->last_position = new_position;
            wiced_time_get_time( &button->last_irq_timestamp );
            wiced_rtos_send_asynchronous_event( blist->thread, button_event_handler, (void*)button );
        }

        else
        {
            if ( button->is_pressed )
                return;

            WPRINT_LIB_INFO ( ( "Button:%u Pressed @time:%u\n", (unsigned int) button->id,
                                    (unsigned int)current_time ) );
            button->is_pressed = 1;
            button->position = BUTTON_POSITION_PRESSED;
            button->last_position = new_position;
            wiced_time_get_time( &button->last_irq_timestamp );
            button->last_pressed = button->last_irq_timestamp;
            wiced_rtos_send_asynchronous_event( blist->thread, button_event_handler, (void*)button );
        }

    }
}

static wiced_result_t button_event_handler( void* arg )
{
    wiced_button_t *button = (wiced_button_t *)arg;
    wiced_button_list_t *blist = button->owner;
    wiced_button_event_t evt = EVT_INVALID;

    if ( !button || !blist )
        return WICED_BADARG;

    switch ( button->current_state )
    {
        case BUTTON_RELEASED:

            if ( button->position == BUTTON_POSITION_RELEASED );
                // Do Nothing for now.
            if ( button->position == BUTTON_POSITION_PRESSED )
            {
               button->current_state = BUTTON_PRESSED;
            }
            break;

        case BUTTON_PRESSED:

            if ( button->position == BUTTON_POSITION_PRESSED );
                // Do nothing for now.
            if ( button->position == BUTTON_POSITION_RELEASED )
            {
                uint32_t time_diff = (button->last_irq_timestamp - button->last_pressed);

                if ( time_diff < SHORT_EVT_DURATION_MAX && time_diff >= SHORT_EVT_DURATION_MIN )
                    evt = EVT_SHORT;

                else if ( time_diff < LONG_EVT_DURATION_MAX && time_diff >= LONG_EVT_DURATION_MIN )
                    evt = EVT_LONG;

                else if ( time_diff < VERY_LONG_EVT_DURATION_MAX && time_diff >= VERY_LONG_EVT_DURATION_MIN )
                    evt = EVT_VERYLONG;

                else if ( time_diff < EXTENDED_EVT_DURATION_MAX && time_diff >= EXTENDED_EVT_DURATION_MIN )
                    evt = EVT_EXTENDED;

                else
                    evt = EVT_INVALID;

               button->current_state = BUTTON_RELEASED;
            }

            if ( blist->callback && evt != EVT_INVALID )
                blist->callback( button->id, evt );

            break;

        default:
            WPRINT_LIB_INFO ( ("Invalid Button State\n" ) );
            break;
    }
    //wiced_rtos_start_timer( &blist->check_state_timer );

    return WICED_SUCCESS;

}

wiced_result_t wiced_register_button_callback( void (*wiced_main_thread_cb)(wiced_button_id_t id, wiced_button_event_t evt) )
{
    wiced_result_t ret = WICED_SUCCESS;

    if ( !buttons_list )
        return WICED_BADARG;

    buttons_list->callback = wiced_main_thread_cb;

    return ret;

}
