/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "RTOS/wiced_rtos_interface.h"
#include "gpio_trace.h"
#include "../trace.h"
#include "wiced_constants.h"
#include <limits.h>

/** TASK_LIST_BUFFER_SIZE should be about 40 bytes per task for FreeRTOS. */
#define TASK_LIST_BUFFER_SIZE    (256)

#if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED
/**
 * A list of GPIO pins used to represent bit order. The bit order is stored
 * implicitly by the position in the list.
 *
 * The least significant bit is at the head of the list. The next most
 * significant bit immediately follows. Etc...
 *
 * This will wrap around if there are not enough bits to represent the TCB
 * number. For example, if there are only 2 bits available to encode the TCB
 * number then tasks with TCB numbers 1 and 5 will have the same encoding.
 */
#elif TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ONEBIT
/**
 *
 * A list of GPIO pins used to represent tasks.
 *
 * The head of the list represents the task with TCB 0. The next element of the
 * list represents the task with TCB 1. Etc.
 *
 * If there are more tasks than GPIO bins available, then any extra tasks (above
 * the GPIO pin count) will not be output.
 */
#endif /* #if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED */
struct Bit_List;
struct breakout_pins_setting_t;
typedef struct Bit_List
{
    struct breakout_pins_setting_t *settings;
    struct Bit_List *next;
} Bit_List;

/** The list of GPIO pins to form the bit representation. */
static Bit_List *least_significant_bit = NULL;


/******************************************************
 *        External variables
 ******************************************************/
extern const unsigned int breakout_settings_size;
extern const unsigned int sizeof_breakout_pins_setting_t;
extern struct breakout_pins_setting_t *breakout_gpio_settings;

/******************************************************
 *        Forward declarations
 ******************************************************/
static void gpio_trace_init( void );
static void gpio_encode_value( int );
static void print_tasks( void );

/******************************************************
 *        Macros
 ******************************************************/
#define TRACE_START_SEQUENCE()              \
    do {                                    \
        gpio_encode_value( all_on );        \
        host_rtos_delay_milliseconds( 1 );  \
        gpio_encode_value( all_off );       \
        host_rtos_delay_milliseconds( 2 );  \
        gpio_encode_value( all_on );        \
        host_rtos_delay_milliseconds( 1 );  \
        gpio_encode_value( all_off );       \
        host_rtos_delay_milliseconds( 2 );  \
        gpio_encode_value( all_on );        \
        host_rtos_delay_milliseconds( 1 );  \
        gpio_encode_value( all_off );       \
    } while ( 0 )
#define TRACE_END_SEQUENCE()                \
    do {                                    \
        gpio_encode_value( all_on );        \
        host_rtos_delay_milliseconds( 1 );  \
        gpio_encode_value( all_off );       \
        host_rtos_delay_milliseconds( 2 );  \
        gpio_encode_value( all_on );        \
        host_rtos_delay_milliseconds( 1 );  \
        gpio_encode_value( all_off );       \
        host_rtos_delay_milliseconds( 2 );  \
    } while ( 0 )
#define MANUAL_ARRAY_ACCESS(cast, array, elem, elem_size) \
    (cast) ((void *) &array + ( elem * elem_size ))
/******************************************************
 *        Special values
 ******************************************************/
static const int all_on = -1;
static const int all_off = -2;


/**
 * Start tracing scheduler activity.
 */
void gpio_trace_start_trace( trace_flush_function_t flush_f )
{
    /* We don't need a flush function because GPIO tracing does not use "any" memory */
    UNUSED_PARAMETER( flush_f );

    /* Setup the GPIO pins and construct the bit list. */
    gpio_trace_init( );

    /* Print out the GPIO->Bit mapping. */
    Bit_List *curr = least_significant_bit;
    unsigned int i = 0;
    while ( curr != NULL )
    {
#if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED
    #define _GPIO_TRACE_START_TRACE_TEXT    "Bit"
#elif TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ONEBIT
    #define _GPIO_TRACE_START_TRACE_TEXT    "Task"
#endif /* #if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED */
        printf( _GPIO_TRACE_START_TRACE_TEXT " %d => Pin %d\r\n", i, breakout_pins_get_pin_number( curr->settings ) );
        curr = curr->next;
        i++;
    }

    printf( "\r\n" );
    print_tasks( );

#if 0 /* TEST SEQUENCE */
    /* Count the number of GPIO pins available */
    int num_pins = 0;
    curr = least_significant_bit;
    while ( curr != NULL )
    {
        num_pins++;
        curr = curr->next;
    }

    printf( "\r\n" );
    printf( "Testing in 3 seconds..." );

    host_rtos_delay_milliseconds( 1000 );
    printf( "\rTesting in 2 seconds..." );

    host_rtos_delay_milliseconds( 1000 );
    printf( "\rTesting in 1 second... " );

    host_rtos_delay_milliseconds( 1000 );
    printf( "\rTesting...             " );

#if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED
#include <math.h>
    const int max_value = (int) pow( (double) 2, (double) num_pins );
#elif TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ONEBIT
    const int max_value = num_pins;
#endif /* #if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED */
    for ( i = 0; i < max_value; i++ )
    {
        gpio_encode_value( i );
        host_rtos_delay_milliseconds( 1 );
    }

    gpio_encode_value( all_off );
    printf( "\rTesting... DONE!       \r\n" );
#endif /* 0 */

    /* Flash GPIO pins to indicate start of trace */
    TRACE_START_SEQUENCE( );
} /* gpio_trace_start_trace */


/**
 * Pause the tracing of scheduler activity.
 */
void gpio_trace_stop_trace( void )
{
    /* Output a special sequence to visually indicate that tracing has ceased. */
    TRACE_END_SEQUENCE( );
} /* gpio_trace_stop_trace */


/**
 * Stop tracing scheduler activity and clear all stored data.
 */
void gpio_trace_cleanup_trace( void )
{
    while ( least_significant_bit )
    {
        Bit_List *tmp = least_significant_bit->next;
        free( least_significant_bit );
        least_significant_bit = tmp;
    }
} /* gpio_trace_cleanup_trace */

/**
 * Hook function to be called when the scheduler performs some interesting
 * action.
 */
void gpio_trace_task_hook( TRACE_TASK_HOOK_SIGNATURE )
{
    /* We only care about switch in events */
    if ( action == Trace_SwitchIn )
    {
        gpio_encode_value( TCBNumber );
    }
} /* gpio_trace_task_hook */

/**
 * Hook function to be called when the scheduler ticks.
 */
void gpio_trace_tick_hook( TRACE_TICK_HOOK_SIGNATURE )
{
    /* we could do something here... if we wanted to */
} /* gpio_trace_tick_hook */

/**
 * Setup the GPIO pins for trace output.
 */
static void gpio_trace_init( void )
{
    struct breakout_pins_setting_t *breakout_pin;

    unsigned int i;
    for ( i = 0; i < breakout_settings_size; i++ )
    {
        breakout_pin = MANUAL_ARRAY_ACCESS( struct breakout_pins_setting_t *, breakout_gpio_settings, i, sizeof_breakout_pins_setting_t );
        if ( breakout_pins_gpio_init( breakout_pin ) )
        {
            /* Add this pin to the list of bit representations */
            Bit_List *new_bit_list = (Bit_List *) malloc( sizeof( Bit_List ) );
            new_bit_list->settings = breakout_pin;
            new_bit_list->next     = NULL;

            /* Find the insertion point */
            Bit_List *insertion_point = NULL;
            Bit_List *curr = least_significant_bit;
            while ( curr )
            {
                insertion_point = curr;
                curr = curr->next;
            }

            /* Set the next point */
            if ( insertion_point != NULL )
            {
                insertion_point->next = new_bit_list;
            }

            /* Set head of list if not yet set */
            if ( least_significant_bit == NULL )
            {
                least_significant_bit = new_bit_list;
            }
        }
    }
}

/**
 * Encode a decimal value using the GPIO bits.
 * @param value_to_encode To value to encode.
 */
static void gpio_encode_value( int value_to_encode )
{
#if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED
    int i = value_to_encode;
#elif TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ONEBIT
    int i = 0;
#endif /* #if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED */
    Bit_List *bit = least_significant_bit;

    while ( bit )
    {
#if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED
    #define _GPIO_ENCODE_VALUE_IF_COND  ( i % 2 == 1 )
#elif TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ONEBIT
    #define _GPIO_ENCODE_VALUE_IF_COND  ( i == value_to_encode )
#endif /* #if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED */
        if ( ( _GPIO_ENCODE_VALUE_IF_COND && ( value_to_encode != all_off ) ) || ( value_to_encode == all_on ) )
        {
            breakout_pins_gpio_setbits( bit->settings );
        }
        else
        {
            breakout_pins_gpio_resetbits( bit->settings );
        }

#if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED
        i /= 2;
#elif TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ONEBIT
        i++;
#endif /* #if TRACE_OUTPUT_TYPE == TRACE_OUTPUT_TYPE_ENCODED */
        bit = bit->next;
    }
}

/**
 * @todo This only works for FreeRTOS at the moment.
 */
static void print_tasks( void )
{
    signed char tasklist_buffer[TASK_LIST_BUFFER_SIZE];

    /* Get task information */
    vTaskList( (signed char *) &tasklist_buffer );

    /* Get task names */
    char *line;
    line = strtok( (char *) tasklist_buffer, "\n" );
    while ( line != NULL )
    {
        char task_name[MAX_TASKNAME_LEN];
        unsigned int task_tcb;

        if ( 2 == sscanf( line, "%s\t\t%*c\t%*u\t%*u\t%u\r\n", task_name, &task_tcb ) )
        {
            /* Print TCB number and task name */
            printf( "Task %d: '%s'\r\n", task_tcb, task_name );
        }
        line = strtok( NULL, "\n" );
    }
}
