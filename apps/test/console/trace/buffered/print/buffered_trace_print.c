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
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "RTOS/wiced_rtos_interface.h"
#include "wiced_assert.h"

#include "../buffered_trace.h"
#include "buffered_trace_print.h"


/******************************************************
 *        Forward declarations
 ******************************************************/

static void print_trace_graph( trace_output_format_t format );
static void print_trace_graph_CSV( trace_output_format_t format );
static void print_trace_graph_Table( trace_output_format_t format );
static void traceaction_printlegend( void );
static int get_task_index( unsigned long tcb_number );
static portTickType format_time( trace_output_format_t format, portTickType time );
static char traceaction_to_char( unsigned int a );
static trace_action_t char_to_traceaction( char c );
static trace_action_t subsequent_traceaction( trace_action_t a );
#if 0
static int is_active( trace_action_t a );
#endif
static unsigned int num_digits( unsigned int num );


/******************************************************
 *        Static variables
 ******************************************************/

/** How do we wish to format the output data? */
static trace_output_format_t output_format =
{
    Output_Style_Table,
    Output_TimeFormat_RelativeMilliseconds,
    /* FLAG_FILL_IN_BLANKS | */ FLAG_SHOW_LEGEND | FLAG_SHOW_TASK_NAMES
};

/** For relative time calculations */
static TICK_T tick_offset = 0;


/**
 * API function to print the graph.
 */
void buffered_trace_print_process_trace( void )
{
    print_trace_graph( output_format );
} /* process_trace */


/**
 * Force an early processing of trace data so that we can delete this data from
 * the buffer.
 */
void buffered_trace_print_flush_trace( void )
{
    /**
     * Flushing the buffer by printing to the console doesn't seem to work.
     */
} /* flush_trace */


/**
 * Display a graphical representation of the thread trace.
 */

void print_trace_graph( trace_output_format_t format )
{
    switch ( format.style )
    {
        case Output_Style_CSV:
            print_trace_graph_CSV( format );
            break;

        case Output_Style_Table:
            print_trace_graph_Table( format );
            break;

        default:
            printf( "No output method for the format specified.\r\n" );
    }
} /* print_trace_graph */


static void print_trace_graph_CSV( trace_output_format_t format )
{

} /* print_trace_graph_CSV */


static void print_trace_graph_Table( trace_output_format_t format )
{
    unsigned int task_count = buffered_trace_get_taskcount( );
    task_info_t *task_info = buffered_trace_get_taskinfo( );
    unsigned int i;
    char data_output_buffer[100];
    int flags = buffered_trace_get_tracebuffer_flags( );

    unsigned int max_tcb = 0;
    for ( i = 0; i < task_count; i++ )
    {
        if ( task_info[i].task_tcb_number > max_tcb )
        {
            max_tcb = task_info[i].task_tcb_number;
        }
    }
    unsigned int max_digits = num_digits( max_tcb );
    unsigned int width_per_task = max_digits + 1 /* for a space separator */;

    /**
     * Number of spaces occupied by trace output (excluding separators and tick
     * values).
     */
    unsigned int data_width = width_per_task * buffered_trace_get_taskcount( );

    /** Total width of graphical output */
    unsigned int total_width = data_width
                               + strlen( TABLE_TIME_HEADER )
                               + strlen( TABLE_ENDLINE )
                               - strlen( "\r\n" );
    data_output_buffer[data_width] = '\0';

    if ( do_Show_Task_Names( format ) )
    {
        /** Print task names */
        TASKTRACE_PRINT( ( "TASKS:\r\n" ) );

        for ( i = 0; i < task_count; i++ )
        {
            printf( "%lu: %s\r\n", task_info[i].task_tcb_number, task_info[i].task_name );
        }
        printf( "\r\n" );
    }

    if ( do_Show_Legend( format ) )
    {
        /** Print legend */
        printf( "LEGEND:\r\n" );
        traceaction_printlegend( );
        printf( "\r\n" );
    }

    /** Print a separator */
    TABLE_PRINT_SEPERATOR( total_width );

    /** Print graph header */
    printf( TABLE_TIME_HEADER );
    for ( i = 0; i < task_count; i++ )
    {
        printf( " %.*lu", max_digits, task_info[i].task_tcb_number );
    }
    printf( TABLE_ENDLINE );

    /** Print a separator */
    TABLE_PRINT_SEPERATOR( total_width );

    /** Blank the data line (set each character to whitespace) */
    TABLE_RESET_LINE( data_output_buffer, data_width );

    /** Print the graph */
    void *curr_pos = buffered_trace_get_tracebuffer_start( );
    trace_log_tick_t *curr_tick = buffered_trace_get_tick_from_buffer( &curr_pos );

    while ( curr_tick != NULL )
    {
        /** Set the time offset if it isn't already set */
        if ( tick_offset == 0 )
        {
            tick_offset = curr_tick->ticks;
        }

        /**
         * @remarks There should be at least one trace_log_action_t, else the
         * trace_log_tick_t should never have been written to the buffer.
         */
        trace_log_action_t *curr_action = buffered_trace_get_action_from_buffer( &curr_pos );
        int action_index = 1;

        while ( curr_action != NULL )
        {
#if BUFFERED_TRACE_USE_CLOCKTIME
            trace_log_clocktime_t *clocktime = 0;
            CLOCKTIME_T ct;
            if ( curr_action->action == Trace_SwitchOut )
            {
                clocktime = buffered_trace_get_clocktime_from_buffer( &curr_pos );
                ct = clocktime->clocktime;
                ct /= ( CLOCKS_PER_SECOND / 1000000 );
            }
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */

            /* Write the corresponding character to the graph output buffer */
            int task_index = get_task_index( curr_action->task_tcb );
            data_output_buffer[ width_per_task * ( task_index + 1 ) - 1 ] = traceaction_to_char( curr_action->action );

            /* Print output for current tick count */
            if ( action_index <= 1 )
            {
#if BUFFERED_TRACE_USE_CLOCKTIME
                if ( clocktime == 0 )
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
                {
                    printf( TABLE_LINE_FORMAT,
                            format_time( format, curr_tick->ticks ),
                            data_output_buffer );
                }
#if BUFFERED_TRACE_USE_CLOCKTIME
                else
                {
                    printf( TABLE_LINE_FORMAT_CLOCKTIME,
                            format_time( format, curr_tick->ticks ),
                            data_output_buffer,
                            ct );
                }
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
            }
            else
            {
#if BUFFERED_TRACE_USE_CLOCKTIME
                if ( clocktime == 0 )
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
                {
                    printf( TABLE_LINE_FORMAT_NOTIME, data_output_buffer );
                }
#if BUFFERED_TRACE_USE_CLOCKTIME
                else
                {
                    printf( TABLE_LINE_FORMAT_NOTIME_CLOCKTIME, data_output_buffer, ct );
                }
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
            }

            /** Update the data output buffer */
            TABLE_TRANSFORM_LINE( data_output_buffer, data_width );

            /* Get next action in list (or set to NULL if none exists) */
            if ( action_index < curr_tick->action_count )
            {
                curr_action = buffered_trace_get_action_from_buffer( &curr_pos );
                action_index++;
            }
            else
            {
                curr_action = NULL;
            }
        }

        /* Go to next tick */
        TICK_T last_tick = curr_tick->ticks;
        curr_tick = buffered_trace_get_tick_from_buffer( &curr_pos );

        if ( do_Fill_In_Blanks( format ) && curr_tick != NULL )
        {
            /**
             * Fill in any gaps in the timeline. This may greatly increase
             * the amount of text that is output, but it will give the graph a
             * more natural scaling.
             */
            while ( ++last_tick < curr_tick->ticks )
            {
                /** Update the data output buffer */
                TABLE_TRANSFORM_LINE( data_output_buffer, data_width );

                printf( TABLE_LINE_FORMAT,
                        format_time( format, last_tick ),
                        data_output_buffer );
            }
        }

        /* Reset the data output buffer to whitespace */
        TABLE_RESET_LINE( data_output_buffer, data_width );
    }

    /* Print separator */
    TABLE_PRINT_SEPERATOR( total_width );

    if ( TraceBuffer_IsIncomplete( flags ) )
    {
        printf( "NOTE: There is data missing from the trace buffer because the buffer is full.\r\n" );
    }

    /* Clean up */
    tick_offset = 0;
} /* print_trace_graph_Table */


/**
 * Searches through the task list for a task with the specified TCB number.
 * Returns the index of the task in the array, if found.
 *
 * @param tcb_number The TCB number of the task to search for.
 * @return The index of this task within the task info array.
 */
static int get_task_index( unsigned long tcb_number )
{
    task_info_t *task_info = buffered_trace_get_taskinfo( );
    unsigned int i = 0;
    task_info_t t;

    for( i = 0; i < buffered_trace_get_taskcount( ); i++ )
    {
        t = task_info[i];

        if ( t.task_tcb_number == tcb_number )
        {
            return i;
        }
    }

    /* Task was not found in the task list. */
    return -1;
} /* get_task_index */


/**
 * Format the time value as required by the trace_output_format_type.
 *
 * @param format The desired output format.
 * @param time The time value.
 * @return The formatted time value.
 */
static portTickType format_time( trace_output_format_t format, TICK_T ticks )
{
    switch ( format.time )
    {
        case Output_TimeFormat_AbsoluteMilliseconds:
            return ticks / ( TICKS_PER_SEC / 1000 );

        case Output_TimeFormat_AbsoluteTicks:
            return ticks;

        case Output_TimeFormat_RelativeMilliseconds:
            return ( ticks - tick_offset ) / ( TICKS_PER_SEC / 1000 );

        case Output_TimeFormat_RelativeTicks:
            return ( ticks - tick_offset );

        default:
            return 0;
    }
} /* format_time */


/**
 * This function is used to map the trace_action_t enumerator into an output
 * character.
 *
 * @param a The trace_action_t to be mapped to an output character. Note that
 * this is stored in the buffer as a 4-bit unsigned int.
 * @return The output character corresponding to the trace_action_t enumerator.
 */
static char traceaction_to_char( unsigned int a )
{
    switch ( a & TRACE_ACTION_T_MASK )
    {
        case Trace_Create:          return 'O';
        case Trace_Delete:          return '-';
        case Trace_Suspend:         return 'Z';
        case Trace_Resume:          return 'R';
        case Trace_ResumeFromISR:   return 'I';
        case Trace_Delay:           return '.';
        case Trace_Die:             return 'X';
        case Trace_PrioritySet:     return 'P';
        case Trace_SwitchOut:       return '\\';
        case Trace_SwitchIn:        return '/';
        case Trace_Executing:       return '*';

        case Trace_Invalid:
        default:                    return ' ';
    }
} /* traceaction_to_char */


/**
 *
 * This function is used to map the output character back into a trace_action_t
 * enumerator.
 *
 * @param c The character to be mapped to a trace_action_t.
 * @return The trace_action_t corresponding to the input character.
 */

static trace_action_t char_to_traceaction( char c )
{
    switch ( c )
    {
        case 'O':   return Trace_Create;
        case '-':   return Trace_Delete;
        case 'Z':   return Trace_Suspend;
        case 'R':   return Trace_Resume;
        case 'I':   return Trace_ResumeFromISR;
        case '.':   return Trace_Delay;
        case 'X':   return Trace_Die;
        case 'P':   return Trace_PrioritySet;
        case '\\':  return Trace_SwitchOut;
        case '/':   return Trace_SwitchIn;
        case '*':   return Trace_Executing;

        default:    return Trace_Invalid;
    }
} /* char_to_traceaction */


/**
 * This function transforms a trace_action_t enumerators for subsequent ticks.
 * What is meant by this is, if the scheduler performs an action on a clock
 * tick, but then no further actions on the next clock ticks, then this function
 * determines the subsequent output for each specified trace_action_t.
 *
 * @param a The input trace_action_t.
 * @return The subsequent trace_action_t.
 */
static trace_action_t subsequent_traceaction( trace_action_t a )
{
    switch ( a )
    {
        case Trace_Create:          return Trace_Invalid;
        case Trace_Delete:          return Trace_Invalid;
        case Trace_Suspend:         return Trace_Suspend;
        case Trace_Resume:          return Trace_Invalid;
        case Trace_ResumeFromISR:   return Trace_Invalid;
        case Trace_Delay:           return Trace_Delay;
        case Trace_Die:             return Trace_Invalid;
        case Trace_PrioritySet:     return Trace_Invalid;
        case Trace_SwitchOut:       return Trace_Invalid;
        case Trace_SwitchIn:        return Trace_Executing;
        case Trace_Executing:       return Trace_Executing;

        case Trace_Invalid:
        default:                    return Trace_Invalid;
    }
} /* subsequent_traceaction */

#if 0
/**
 * This function translates a trace action into a boolean value indicating if
 * a trace action represents the currently active task.
 *
 * @param a The input trace_action_t.
 * @return 1 if the trace is active, else 0.
 */
static int is_active( trace_action_t a )
{
    switch ( a )
    {
        case Trace_Create:          return 0;
        case Trace_Delete:          return 0;
        case Trace_Suspend:         return 0;
        case Trace_Resume:          return 0;
        case Trace_ResumeFromISR:   return 0;
        case Trace_Delay:           return 0;
        case Trace_Die:             return 0;
        case Trace_PrioritySet:     return 0;
        case Trace_SwitchOut:       return 0;
        case Trace_SwitchIn:        return 1;
        case Trace_Executing:       return 0;

        case Trace_Invalid:
        default:                    return Trace_Invalid;
    }
} /* subsequent_traceaction */
#endif

/**
 * This function prints a legend for the trace_action_t enumerator.
 */
static void traceaction_printlegend( void )
{
    int i;
    for ( i = TRACE_ACTION_T_START; i <= TRACE_ACTION_T_END; i++ )
    {
        char c = traceaction_to_char( i );
        printf( "'%c' => ", c );

        switch ( (trace_action_t) i )
        {
            case Trace_Create:          printf( "Create" ); break;
            case Trace_Delete:          printf( "Delete" ); break;
            case Trace_Suspend:         printf( "Suspend" ); break;
            case Trace_Resume:          printf( "Resume" ); break;
            case Trace_ResumeFromISR:   printf( "Resume from ISR" ); break;
            case Trace_Delay:           printf( "Delay" ); break;
            case Trace_Die:             printf( "Die" ); break;
            case Trace_PrioritySet:     printf( "Priority Set" ); break;
            case Trace_SwitchOut:       printf( "Switch Out" ); break;
            case Trace_SwitchIn:        printf( "Switch In" ); break;
            case Trace_Executing:       printf( "Executing" ); break;
            case Trace_Invalid:         printf( "No Action" ); break;
            default:                    break;
        }
        printf( "\r\n" );
    }
} /* traceaction_printlegend */


/**
 * Number of digits that a number contains.
 *
 * @param num The input number.
 * @return The number of digits required to represent the input number in
 * base 10 notation.
 */
static unsigned int num_digits( unsigned int num )
{
    if ( num < 10 )
    {
        return 1;
    }
    else
    {
        return ( num_digits( num / 10 ) + 1 );
    }
} /* num_digits */
