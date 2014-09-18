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

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    KEY_POLARITY_LOW,   /* GPIO logic value 0 when key/button is released */
    KEY_POLARITY_HIGH,  /* GPIO logic value 1 when key/button is released */
} gpio_key_polarity_t;

enum
{
    KEY_EVENT_NONE     = 0,   /* No event              */
    KEY_EVENT_PRESSED  = 0x1, /* Key has been pressed  */
    KEY_EVENT_RELEASED = 0x2, /* Key has been released */
    KEY_EVENT_HELD     = 0x4, /* Key has been released */
};
typedef uint32_t gpio_key_event_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef uint32_t gpio_key_code_t;
typedef void (*gpio_keypad_handler_t)(gpio_key_code_t code, gpio_key_event_t event);

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct gpio_key
{
    wiced_gpio_t        gpio;
    gpio_key_code_t     code;
    gpio_key_polarity_t polarity;
}gpio_key_t;

typedef struct gpio_keypad
{
    /* Private members. Internal use only */
    struct gpio_keypad_internal* internal;
}gpio_keypad_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t gpio_keypad_enable ( gpio_keypad_t* keypad, wiced_worker_thread_t* thread, gpio_keypad_handler_t function, uint32_t held_key_interval_ms, uint32_t total_keys, const gpio_key_t* key_list );
wiced_result_t gpio_keypad_disable( gpio_keypad_t *keypad );

#ifdef __cplusplus
} /* extern "C" */
#endif
