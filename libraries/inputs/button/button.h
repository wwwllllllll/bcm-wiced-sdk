/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "wiced_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************
 *                      Macros
 ******************************************************/
/* Map these to corresponding GPIOs */
/*
#define WICED_BUTTON3
#define WICED_BUTTON4
#define WICED_BUTTON5
*/
/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum wiced_button_state
{
    BUTTON_RELEASED,
    BUTTON_PRESSED,
    BUTTON_HELD,

}wiced_button_state_t;

/* Each button should have specific type so that we can identify which should
 * allow the valid state it can be in. For example, if we have a button which should
 * handle only single click events, should reject other states like DOUBLE_CLICK etc */

typedef enum wiced_button_click_type
{
    SINGLE_CLICK = (1<<0),
    DOUBLE_CLICK = (1<<1),

}wiced_button_click_type_t;

typedef enum wiced_button_type
{
    BUTTON_SINGLE_FUNCTION = (1<<0),
    BUTTON_MULTI_FUNCTION = (1<<1),
}wiced_button_type_t;

typedef enum wiced_button_event
{
    /* Denotes the events for multifunctional buttons. Differntiates on
     * the time-gap between a press-release cycle */
    EVT_SHORT,
    EVT_LONG,
    EVT_VERYLONG,
    EVT_EXTENDED,
    EVT_MAX,
    EVT_INVALID = EVT_MAX,

}wiced_button_event_t;

typedef enum wiced_button_position
{
    BUTTON_POSITION_PRESSED,
    BUTTON_POSITION_RELEASED,

}wiced_button_position_t;

typedef enum wiced_button_id
{
    BUTTON_0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_MAX,
    BUTTON_UNKNOWN = 0xFF,

}wiced_button_id_t;

typedef enum wiced_button_gpio_polarity
{
    BUTTON_POLARITY_LOW,
    BUTTON_POLARITY_HIGH,
}wiced_button_gpio_polarity_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
struct wiced_button_list;

typedef struct wiced_button
{
    wiced_gpio_t                    gpio_number; /* GPIO number corresponding to the button */
    wiced_button_type_t             type;
    wiced_button_state_t            current_state;
    wiced_button_position_t         position;
    wiced_button_gpio_polarity_t    polarity;
    uint8_t                         id; /* Button ID */
    void (*callback) (void *arg); /* Pointer to callback function */
    uint32_t                        last_irq_timestamp;
    wiced_button_position_t         last_position;
    struct wiced_button_list        *owner;
    wiced_bool_t                    is_pressed;
    uint32_t                        last_pressed;

}wiced_button_t;

typedef struct wiced_button_list
{
    wiced_button_t          *buttons;
    uint8_t                 nr_buttons;
    void (*callback) (wiced_button_id_t id, wiced_button_event_t arg); /* Pointer to callback function */
    uint8_t                 active; /* Mask for currently active buttons */
    wiced_timer_t           check_state_timer;
    wiced_worker_thread_t   *thread;

}wiced_button_list_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
extern wiced_result_t wiced_button_init( wiced_button_list_t *arg );
wiced_result_t wiced_button_enable( wiced_button_list_t *arg,  uint8_t num_buttons );
wiced_result_t wiced_button_register_callback( void (*cb)(wiced_button_id_t id, wiced_button_event_t evt) );
void wiced_button_disable( void *arg );

#ifdef __cplusplus
} /* extern "C" */
#endif
