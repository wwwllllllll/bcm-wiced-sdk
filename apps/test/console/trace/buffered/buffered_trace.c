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
#include "console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "RTOS/wiced_rtos_interface.h"
#include "portmacro.h"
#include "wiced_assert.h"
#include "buffered_trace.h"
#include "../clocktimer/clocktimer.h"
#include "../trace.h"

/******************************************************
 *        Forward declarations
 ******************************************************/
static void clean_buffers( int is_flush );
static void add_action( TICK_T,
#if BUFFERED_TRACE_USE_CLOCKTIME
        CLOCKTIME_T,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        trace_log_action_t,
        int );
static void buffer_switchout( TICK_T,
#if BUFFERED_TRACE_USE_CLOCKTIME
        CLOCKTIME_T,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        trace_log_action_t );
static void write_buffered_switchout( void );
static int matches_buffered_switchout( TICK_T,
#if BUFFERED_TRACE_USE_CLOCKTIME
        CLOCKTIME_T,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        trace_log_action_t );

static int write_tick_to_buffer( trace_log_tick_t * );
static int write_action_to_buffer( trace_log_action_t *
#if BUFFERED_TRACE_USE_CLOCKTIME
        , CLOCKTIME_T
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        );


/******************************************************
 *        Static variables
 ******************************************************/
/** Current tick count. Updated by the trace tick hook function. */
static TICK_T ticks = 0;

/*----------------------------------------------------------------------------*/
/** @name Task information */
/** @{ */

/** Stores information about all tasks */
static task_info_t task_info[TRACE_LOG_TASK_COUNT];

/** Number of tasks in the task info array */
static unsigned int task_count = 0;

/** @} */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/** @name Trace buffer */
/** @{ */

/**
 * @brief Buffer to store trace information, in the forms of trace_logn_tick_t
 * and trace_log_action_t structures.
 *
 * @remarks This buffer is of type `signed char' because this ensures that the
 * compiler doesn't do anything to attempt to align each element. We will be
 * carefully managing the contents of this array.
 *
 * @details The structure of the array is as follows (not to scale):
 * - T = trace_log_tick_t
 * - A = trace_log_action_t
 * - X = trace_log_clocktime_t
 * |---------------------------------------------------------------------------|
 * |  T  | A | X | A | X |  T  | A |  T  | A | A | X |  T  | A | X | T | A | A |
 * |---------------------------------------------------------------------------|
 *
 * @remarks The values of field trace_log_tick_t->action_count is equal to the
 * number of trace_log_action_t structures that follow the trace_log_tick_t
 * structure in the buffer.
 *
 * @remarks A trace_log_clocktime_t will follow a trace_log_action_t structure
 * if and only if the action of the trace_log_action_t is Trace_SwitchOut. This
 * must be carefully checked when processing trace data.
 */
static signed char tracebuffer[TRACE_LOG_BUFFER_SIZE];
static int tracebuffer_flags = 0; /**< flags to describe the current state of the
                                       trace buffer */

/** A pointer to the last trace_log_tick_t stored in the buffer */
static trace_log_tick_t *tracebuffer_currtick = NULL;

/**
 * A pointer to the next free position in the trace.
 *
 * @remarks We must ensure that this pointer is always within the memory bounds
 * of the trace buffer.
 */
static void *tracebuffer_currpos = &tracebuffer;

/** @} */

static trace_flush_function_t flush_function = NULL;
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * We buffer switch out actions in case they are "canceled out" by an identical
 * switch in action during the same tick.
 *
 * If this occurs, then the scheduler didn't do anything interesting and so,
 * to save memory resources, we discard both the SwitchOut and SwitchIn actions.
 *
 * @remarks a portTickType value of 0 indicates an invalid/unset
 * trace_log_action_t.
 */
static int buffered_switchout_valid = WICED_FALSE;
static TICK_T buffered_switchout_ticks = 0;
#if BUFFERED_TRACE_USE_CLOCKTIME
static CLOCKTIME_T buffered_switchout_clocktime = 0;
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
static trace_log_action_t buffered_switchout;
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * We buffer actions in case they "canceled out" but we want to now print the
 * log. In this case the last "canceled out" action should be written to the
 * log, because it makes more sense on the output.
 *
 * @remarks a portTickType value of 0 indicates an invalid/unset
 * trace_log_action_t.
 */
static int last_canceled_out_valid = WICED_FALSE;
static TICK_T last_canceled_out_ticks = 0;
#if BUFFERED_TRACE_USE_CLOCKTIME
static CLOCKTIME_T last_canceled_out_clocktime = 0;
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
static trace_log_action_t last_canceled_out;
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

static int switchin_valid = WICED_FALSE;
#if BUFFERED_TRACE_USE_CLOCKTIME
static CLOCKTIME_T switchin_clocktime = 0;
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
static TCB_NUMBER_T switchin_tcb_number;
/*----------------------------------------------------------------------------*/


/**
 * Start tracing scheduler activity.
 */
void buffered_trace_start_trace( trace_flush_function_t flush_f )
{
#if BUFFERED_TRACE_USE_CLOCKTIME
    /**
     * Start the processor timer to allow greater time resolution than the RTOS
     * can provide.
     */
    start_clocktimer( );
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
    flush_function = flush_f;
} /* buffered_trace_start_trace */


/**
 * Pause the tracing of scheduler activity, without clearing any data from the
 * buffer.
 */
void buffered_trace_stop_trace( void )
{
#if BUFFERED_TRACE_USE_CLOCKTIME
    end_clocktimer( );
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
} /* buffered_trace_stop_trace */


/**
 * Stop tracing scheduler activity and clear all stored data.
 */
void buffered_trace_cleanup_trace( void )
{
#if BUFFERED_TRACE_USE_CLOCKTIME
    end_clocktimer( );
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
    clean_buffers( WICED_FALSE );
} /* buffered_trace_cleanup_trace */


/**
 * Perform any house-keeping tasks that must occur before a trace can be
 * processed.
 */
void buffered_trace_preprocess_trace( void )
{
    /** Write any buffered switch out actions to the trace log now */
    write_buffered_switchout( );

    if ( last_canceled_out_valid && tracebuffer_currtick->ticks <= last_canceled_out_ticks )
    {
        /**
         * Write the last canceled out action to the trace log.
         *
         * @remarks It makes sense to write the last canceled out action (which
         * will be a switch out action). For example, if a single thread was the
         * only thread active for the whole trace then the only other action in
         * the log will be a switch in for the active task. It makes sense to
         * also include a switch out at the end of the trace period.
         */
        add_action( last_canceled_out_ticks,
#if BUFFERED_TRACE_USE_CLOCKTIME
                last_canceled_out_clocktime,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
                last_canceled_out,
                WICED_TRUE );
        last_canceled_out_valid = WICED_FALSE;
    }

    flush_function = NULL;
} /* buffered_trace_preprocess_trace */


/**
 * Clean up from a trace and free all associated memory.
 */
static void clean_buffers( int is_flush )
{
    tracebuffer_currtick = NULL;
#if 0
    memset( tracebuffer, 0, TRACE_LOG_BUFFER_SIZE ); /* not really necessary */
#endif /* 0 */
    tracebuffer_currpos = &tracebuffer; /* now data will be overwritten! */
    tracebuffer_flags = 0;

    /* If we aren't flushing then we can free task information */
    if ( !is_flush )
    {
        task_count = 0;
#if 0
        memset( task_info, 0, sizeof( task_info ) ); /* not really necessary */
#endif /* 0 */
    }
} /* clean_buffers */


/**
 * Stores information about a task in the task info list. If the task is already
 * in the array, then it doesn't really do anything.
 *
 * This function should be called instead of directly inserting a task into the
 * array, because this function ensures that there are no duplicates.
 */
TCB_NUMBER_T task_storeinfo( void *p, char TaskName[], TCB_NUMBER_T TCBNumber )
{
    /* Check if task info already exists. If it does, return it. */
    unsigned int i;
    for ( i = 0; i < task_count; i++ )
    {
        if ( strcmp( task_info[i].task_name, (const char *) TaskName ) == 0 )
        {
            /* Task info already exists... just return it */
            return task_info[i].task_tcb_number;
        }
    }

    wiced_assert( "Task info buffer is full.\r\n", task_count < TRACE_LOG_TASK_COUNT );
    /**
     * Note that if the task info buffer is full, we can continue writing into
     * the trace buffer, but won't be able to translate TCB numbers into task
     * names.
     */

    /* Generate new task info */
    task_info_t new_info;
    strcpy( new_info.task_name, (const char *) TaskName );
    new_info.p               = p;
    new_info.task_tcb_number = TCBNumber;
    task_info[task_count++]  = new_info;
    return new_info.task_tcb_number;
} /* task_storeinfo */


/**
 * Add a scheduler action to the trace log. This function may buffer the value
 * and attempt to discard it later (if it is unnecessary).
 *
 * @param force If nonzero, then this function will not attempt to buffer the
 * action and instead will write it to the log immediately. You should force
 * the action to the buffer if you are attempting to add an already buffered
 * action to the log.
 *
 * @remarks This function will set the trace buffer "Incomplete" flag if it is
 * unable to write data to the trace buffer because it is full.
 */
void add_action( TICK_T action_ticks,
#if BUFFERED_TRACE_USE_CLOCKTIME
        CLOCKTIME_T action_clocktime,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        trace_log_action_t action,
        int force )
{
    /**
     * If the action is a switch out, then we should buffer the action and try
     * to cancel it out later. If we can't cancel it out later, then we will
     * write it to the log then.
     */
    if ( !force && ( action.action == Trace_SwitchOut ) )
    {
        /* Buffer the action */
        buffer_switchout( action_ticks,
#if BUFFERED_TRACE_USE_CLOCKTIME
                action_clocktime,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
                action );
        return;
    }
    /**
     * If this action matches with the already buffered switch out action, then
     * we can cancel the two actions out.
     */
    else if ( !force && matches_buffered_switchout( action_ticks,
#if BUFFERED_TRACE_USE_CLOCKTIME
            action_clocktime,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
            action ) )
    {
        return;
    }

    /* Check if we need to make a new trace log tick */
    if ( ( ( tracebuffer_currtick == NULL ) ||
           ( action_ticks != tracebuffer_currtick->ticks ) ) )
    {
        /* Create a new tick */
        trace_log_tick_t new_tick;
        new_tick.ticks        = action_ticks;
        new_tick.action_count = 0;

        /**
         * Attempt to copy data into buffer.
         *
         * @remarks If the buffer is full, then we will flush the existing data
         * from the buffer. If, after flushing, the trace buffer still doesn't
         * have enough space, then we will mark the buffer as full using a flag,
         * and thus we will no longer attempt to write data to it.
         */
        if ( !write_tick_to_buffer( &new_tick ) )
        {
            /**
             * Flushing the trace should, but isn't required to, free up the
             * trace buffer.
             */
            if ( flush_function )
            {
                flush_function( );
            }

            /**
             * Attempt to write to the buffer again. If we fail again, then
             * give up! */
            if ( !write_tick_to_buffer( &new_tick ) )
            {
                /** We are now missing data... */
                TraceBuffer_SetIncomplete( tracebuffer_flags );
                return;
            }
        }
    }

    /**
     * Copy trace action into buffer.
     *
     * @remarks If the buffer is full, then we are going to discard it. For this
     * reason, TRACE_FLUSH_BUFFER should be large enough to accommodate all
     * actions that could be added to that tick.
     */
    if ( !write_action_to_buffer( &action
#if BUFFERED_TRACE_USE_CLOCKTIME
            , action_clocktime
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
            ) )
    {
        /** We are now missing data... */
        TraceBuffer_SetIncomplete( tracebuffer_flags );
    }
} /* add_action */


/**
 * Store a switch out action in the buffer. This function shouldn't be called if
 * there is already an action in the buffer, but if it is then we will first
 * write this buffered action to the log.
 */
static void buffer_switchout( TICK_T action_ticks,
#if BUFFERED_TRACE_USE_CLOCKTIME
        CLOCKTIME_T action_clocktime,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        trace_log_action_t action )
{
    if ( action.action == Trace_SwitchOut )
    {
        /**
         * There shouldn't be a buffered action at the moment, but let's just
         * check.
         */
        if ( buffered_switchout_valid )
        {
            write_buffered_switchout( );
        }
        buffered_switchout_ticks     = action_ticks;
#if BUFFERED_TRACE_USE_CLOCKTIME
        buffered_switchout_clocktime = action_clocktime;
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        buffered_switchout           = action;
        buffered_switchout_valid     = WICED_TRUE;
    }
    else
    {
        /* we shouldn't be here */
    }
} /* buffer_switchout */


/**
 * Write the buffered switch out action to the trace log.
 */
static void write_buffered_switchout( void )
{
    if ( buffered_switchout_valid )
    {
        /**
         * We set the force argument to WICED_TRUE to prevent from checking if
         * we can buffer or cancel out this action (as we have already done
         * this).
         */
        add_action( buffered_switchout_ticks,
#if BUFFERED_TRACE_USE_CLOCKTIME
                buffered_switchout_clocktime,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
                buffered_switchout,
                WICED_TRUE );
    }

    /** Reset buffered action */
    buffered_switchout_valid = WICED_FALSE;
} /* write_buffered_switchout */


/**
 * Checks if an action "cancels out" with the buffered switch out action.
 *
 * Returns true if the actions canceled out. This signals that no further
 * processing is required and both actions have been deleted.
 *
 * Returns false if the actions did not match, and the new action should be
 * written to the trace log.
 */
static int matches_buffered_switchout( TICK_T action_ticks,
#if BUFFERED_TRACE_USE_CLOCKTIME
        CLOCKTIME_T action_clocktime,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        trace_log_action_t action )
{
    if ( ( action.action == Trace_SwitchIn ) && buffered_switchout_valid )
    {
        if ( ( action_ticks == buffered_switchout_ticks ) &&
             ( action.task_tcb == buffered_switchout.task_tcb ) )
        {
            /**
             * If the trace buffer is empty, then we don't want to write the
             * first switch out action to the log (if it is matched). So now
             * let's remove the buffered switch out action but keep the switch in
             * action.
             */
            if ( tracebuffer_currpos == (void *) &tracebuffer )
            { /* trace buffer is empty */
                buffered_switchout_valid = WICED_FALSE;

                /** Write the new action to the trace log */
                return WICED_FALSE;
            }
            else
            {
                /**
                 * We have a match - let's discard these actions because they
                 * "cancel out".
                 *
                 * However, we don't discard the action yet in case this is the
                 * last action in the trace, in which case it makes more sense
                 * to print it out.
                 *
                 * Therefore, we buffer the newer action and discard the older
                 * action.
                 */
                last_canceled_out_ticks      = buffered_switchout_ticks;
#if BUFFERED_TRACE_USE_CLOCKTIME
                last_canceled_out_clocktime  = buffered_switchout_clocktime;
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
                last_canceled_out            = buffered_switchout;
                last_canceled_out_valid      = WICED_TRUE;
                buffered_switchout_valid     = WICED_FALSE;

                /** Do NOT write the new action to the trace log. */
                return WICED_TRUE;
            }
        }
        else
        {
            /**
             * No match... now we have to write the buffered action to the
             * trace log.
             */
            write_buffered_switchout( );

            /** Write the new action to the trace log */
            return WICED_FALSE;
        }
    }
    else
    {
        return WICED_FALSE;
    }
} /* matches_buffered_switchout */


/**
 * Hook function to be called when the scheduler performs some interesting
 * action.
 */
void buffered_trace_task_hook( TRACE_TASK_HOOK_SIGNATURE )
{
#if BUFFERED_TRACE_USE_CLOCKTIME
    CLOCKTIME_T clocktime = get_clocktime( );
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */

    /* Create a new trace activity */
    trace_log_action_t new_action;
    new_action.task_tcb = task_storeinfo( p, (char *) TaskName, TCBNumber );
    new_action.action   = action;

    /* Add this action to the trace log */
    add_action( ticks,
#if BUFFERED_TRACE_USE_CLOCKTIME
            clocktime,
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
            new_action,
            WICED_FALSE );
} /* buffered_trace_task_hook */


/**
 * Hook function to be called when the scheduler ticks.
 */
void buffered_trace_tick_hook( TRACE_TICK_HOOK_SIGNATURE )
{
    /** Update the tick count */
    TICK_T old_ticks = ticks;
    ticks = TickCount;

    /**
     * If we are incrementing the tick counting and we still have a buffered
     * trace action, then we can now write this trace action to the trace
     * log.
     */
    if ( old_ticks != ticks )
    {
        if ( buffered_switchout_valid )
        {
            write_buffered_switchout( );
        }
    }
} /* buffered_trace_tick_hook */


/**
 * Access the trace buffer.
 *
 * @return A pointer to the start of the trace buffer.
 */
void *buffered_trace_get_tracebuffer_start( void )
{
    return &tracebuffer;
} /* buffered_trace_get_tracebuffer_start */


/**
 * Locate the end of the trace buffer.
 *
 * @return A pointer to the next last written location in the trace buffer.
 */
void *buffered_trace_get_tracebuffer_end( void )
{
    return tracebuffer_currpos;
} /* buffered_trace_get_tracebuffer_end */


/**
 * Access the task info.
 *
 * @return A pointer to the task info array.
 */
void *buffered_trace_get_taskinfo( void )
{
    return &task_info;
} /* buffered_trace_get_taskinfo */


/**
 * Query the number of tasks stored in the task info array.
 *
 * @return The number of tasks stored in the task info array.
 */
unsigned int buffered_trace_get_taskcount( void )
{
    return task_count;
} /* buffered_trace_get_taskcount */


/**
 * Get the trace buffer flags.
 */
int buffered_trace_get_tracebuffer_flags( void )
{
    return tracebuffer_flags;
} /* buffered_trace_get_tracebuffer_flags */


/**
 * Get the next trace_log_tick_t structure from the buffer, if one exists.
 *
 * @param pos A pointer to a pointer to the current position in the buffer.
 * @return A pointer to the next trace_log_tick_t, or NULL if none exists.
 *
 * @remarks This function will advance the position pointer (`pos') if data is
 * retrieved.
 *
 * @warning This function performs no checks as to whether the pointer points
 * to a valid tick.
 */
trace_log_tick_t *buffered_trace_get_tick_from_buffer( void **pos )
{
    trace_log_tick_t *data = NULL;

    if ( ( ((signed char *) *pos) + sizeof( trace_log_tick_t ) )
             <= (signed char *) tracebuffer_currpos )
    {
        data = *pos;
        (*pos) += sizeof( trace_log_tick_t );
    }

    return data;
} /* buffered_trace_get_tick_from_buffer */


/**
 * Get a trace_log_action_t structure from the buffer, if one exists.
 *
 * @param pos A pointer to a pointer to the current position in the buffer.
 * @return A pointer to the next trace_log_action_t, or NULL if none exists.
 *
 * @remarks This function will advance the position pointer (`pos') if data is
 * retrieved.
 *
 * @warning This function performs no checks as to whether the pointer points
 * to a valid action.
 */
trace_log_action_t *buffered_trace_get_action_from_buffer( void **pos )
{
    trace_log_action_t *data = NULL;

    if ( ( ((signed char *) *pos) + sizeof( trace_log_action_t ) )
                 <= (signed char *) tracebuffer_currpos )
    {
        data = *pos;
        (*pos) += sizeof( trace_log_action_t );
    }

    return data;
} /* buffered_trace_get_action_from_buffer */


#if BUFFERED_TRACE_USE_CLOCKTIME
/**
 * Get a trace_log_clocktime_t structure from the buffer, if one exists.
 *
 * @param pos A pointer to a pointer to the current position in the buffer.
 * @return A pointer to the next trace_log_clocktime_t, or NULL if none exists.
 *
 * @remarks This function will advance the position pointer (`pos') if data is
 * returned.
 *
 * @warning This function performs no checks as to whether the pointer points
 * to a valid clocktime.
 */
trace_log_clocktime_t *buffered_trace_get_clocktime_from_buffer( void **pos )
{
    trace_log_clocktime_t *data = NULL;

    if ( ( ((signed char *) *pos) + sizeof( trace_log_clocktime_t ) )
                 <= (signed char *) tracebuffer_currpos )
    {
        data = *pos;
        (*pos) += sizeof( trace_log_clocktime_t );
    }

    return data;
} /* buffered_trace_get_clocktime_from_buffer */
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */


/**
 * Write a trace_log_tick_t structure to the trace buffer.
 *
 * @param data A pointer to the trace_log_tick_t to be written to the buffer.
 * @return True if the write was successful. False if the buffer is full.
 *
 * @remarks This function will advance the tracebuffer_currpos pointer if data
 * is written to the buffer.
 * @remarks This function will set the trace buffer "full" flag if there is
 * not enough space in the buffer for new data.
 */
static int write_tick_to_buffer( trace_log_tick_t *data )
{
    /** Make sure we have enough space in the trace buffer. */
    if ( ( ((signed char *) tracebuffer_currpos) + sizeof( trace_log_tick_t ) )
         <= ( tracebuffer + sizeof( tracebuffer ) ) )
    {
        //memcpy(tracebuffer_currpos, data, sizeof( trace_log_tick_t ) );
        *((trace_log_tick_t*) tracebuffer_currpos) = *data;
        tracebuffer_currtick = (trace_log_tick_t*) tracebuffer_currpos;
        tracebuffer_currpos += sizeof( trace_log_tick_t );
        return WICED_TRUE;
    }
    else
    {
        /* Buffer is full */
        TraceBuffer_SetFull( tracebuffer_flags );
        return WICED_FALSE;
    }
} /* write_tick_to_buffer */


/**
 * Write a trace_log_action_t structure to the trace buffer.
 *
 * @param data A pointer to the trace_log_action_t to be written to the buffer.
 * @return True if the write was successful. False if the buffer is full.
 *
 * @remarks This function will advance the tracebuffer_currpos pointer if data is
 * written to the buffer.
 * @remarks If necessary, this function will also write a clocktime to the
 * buffer.
 * @remarks This function will set the trace buffer "full" flag if there is
 * not enough space in the buffer for new data.
 */
static int write_action_to_buffer( trace_log_action_t *data
#if BUFFERED_TRACE_USE_CLOCKTIME
        , CLOCKTIME_T clocktime
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        )
{
    /**
     * If this is a switch in action, let's record the time so that we can
     * calculate the total running time when we find the corresponding switch
     * out.
     */
    if ( data->action == Trace_SwitchIn )
    {
#if BUFFERED_TRACE_USE_CLOCKTIME
        switchin_clocktime  = clocktime;
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */
        switchin_tcb_number = data->task_tcb;
        switchin_valid      = WICED_TRUE;
    }

    /** Make sure we have enough space in the trace buffer. */
    if ( ( ((signed char *) tracebuffer_currpos) + sizeof( trace_log_action_t ) )
         <= ( tracebuffer + sizeof( tracebuffer ) ) )
    {
        //memcpy( tracebuffer_currpos, data, sizeof( trace_log_action_t ) );
        *((trace_log_action_t*) tracebuffer_currpos) = *data;
        tracebuffer_currpos += sizeof( trace_log_action_t );
        tracebuffer_currtick->action_count++;
    }
    else
    {
        /* Buffer is full */
        TraceBuffer_SetFull( tracebuffer_flags );
        return WICED_FALSE;
    }

#if BUFFERED_TRACE_USE_CLOCKTIME
    /**
     * If this is a switch out action, let's check if we can calculate the
     * task running time using the previously record switch in time.
     */
    if ( data->action == Trace_SwitchOut )
    {
        if ( ( ((signed char *) tracebuffer_currpos) + sizeof( trace_log_clocktime_t ) )
           <= ( tracebuffer + sizeof( tracebuffer ) ) )
        {
            /** Store clocktime */
            trace_log_clocktime_t duration;
            if ( switchin_valid && data->task_tcb == switchin_tcb_number )
            {
                duration.clocktime = clocktime - switchin_clocktime;
            }
            else
            {
                /** The recorded switch in time is for a different task... oops! */
                duration.clocktime = 0;
            }

            *((trace_log_clocktime_t*) tracebuffer_currpos) = duration;
            tracebuffer_currpos += sizeof( trace_log_clocktime_t );
            switchin_valid = WICED_FALSE;
            return WICED_TRUE;
        }
        else
        {
        /* Buffer is full */
        TraceBuffer_SetFull( tracebuffer_flags );
        return WICED_FALSE;
        }
    }
#endif /* BUFFERED_TRACE_USE_CLOCKTIME */

    /** Success */
    return WICED_TRUE;
} /* write_action_to_buffer */
