/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "trace_hook.h"
#include <stdio.h>

static trace_task_hook_f trace_task_hook_func = NULL;
static trace_tick_hook_f trace_tick_hook_func = NULL;


/**
 * Turn task tracing on by setting the hook functions.
 */
void set_trace_hooks( trace_task_hook_f task, trace_tick_hook_f tick )
{
    trace_task_hook_func = task;
    trace_tick_hook_func = tick;
} /* set_trace_hooks */


/**
 * Turn task tracing off.
 */
void unset_trace_hooks( void )
{
    trace_task_hook_func = NULL;
    trace_tick_hook_func = NULL;
} /* unset_trace_hooks */


/**
 * A hook to be called whenever a traceTASK_XXXX macro is called.
 */
inline void trace_task_hook( TRACE_TASK_HOOK_SIGNATURE )
{
    /** If a hook function has been set, then execute it */
    if ( trace_task_hook_func != NULL )
    {
        trace_task_hook_func( TRACE_TASK_HOOK_VARIABLES );
    }
    else
    {
        /* trace is not active... ignore */
    }
} /* trace_task_hook */


/**
 * A hook to be called whenever the traceTASK_INCREMENT_TICK macro is called.
 */
inline void trace_tick_hook( TRACE_TICK_HOOK_SIGNATURE )
{
    /** If a hook function has been set, then execute it */
    if ( trace_tick_hook_func != NULL )
    {
        trace_tick_hook_func( TRACE_TICK_HOOK_VARIABLES );
    }
    else
    {
        /* trace is not active... ignore */
    }
} /* trace_tick_hook */
