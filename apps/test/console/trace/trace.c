/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <stdio.h>
#include <string.h>
#include "wiced_debug.h"
#include "trace.h"
#include "trace_hook.h"
#include "../console.h"
#include "buffered/buffered_trace.h"
#include "gpio/gpio_trace.h"

extern const command_t *console_command_table;

/** A table containing all possible trace types. */
static const trace_t trace_types[] = {
    TRACE_T_BUFFER
    TRACE_T_GPIO
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, (trace_process_t []) { { NULL, NULL, NULL } } }
};

/** The currently active trace type. */
static const trace_t *active_trace = NULL;
/** The currently active trace process type. */
static const trace_process_t *active_trace_process = NULL;


int console_trace( int argc, char *argv[] )
{
    if ( active_trace != NULL )
    {
        WINFO_APP( ( "Already tracing...\r\n" ) );
        return ERR_CMD_OK;
    }

    const command_t *cmd_ptr = NULL;
    int cmd_argc;
    char **cmd_argv;

    /* Make sure a command and trace type was specified */
    if ( argc < 3 )
    {
        return ERR_INSUFFICENT_ARGS;
    }

    cmd_argc = argc - 1;
    cmd_argv = &argv[1];

    /* Find the trace type */
    for ( active_trace = trace_types; active_trace->name != NULL; active_trace++ )
    {
        if ( strcmp( cmd_argv[0], active_trace->name ) == 0 )
        {
            cmd_argc--;
            cmd_argv = &cmd_argv[1];
            break;
        }
    }

    /**
     * If a valid trace type was specified, then cmd_ptr now points to the
     * specified command.
     */

    if ( active_trace->name == NULL )
    {
        WINFO_APP( ( "Invalid trace type. Valid trace types are:\r\n" ) );
        const trace_t *p;
        int printed_anything = 0;
        for ( p = trace_types; p->name != NULL; p++ )
        {
            printed_anything = 1;
            WINFO_APP( ( "\t%s\r\n", p->name ) );
        }
        if ( !printed_anything )
        {
            WINFO_APP( ( "\t(none)\r\n" ) );
        }
        active_trace = NULL;
        active_trace_process = NULL;
        return ERR_UNKNOWN;
    }

    /* Find the trace process type */
    for ( active_trace_process = active_trace->process_types; active_trace_process->name != NULL; active_trace_process++ )
    {
        if ( strcmp( cmd_argv[0], active_trace_process->name ) == 0 )
        {
            cmd_argc--;
            cmd_argv = &cmd_argv[1];
            break;
        }
    }

    if ( ( active_trace_process->name == NULL ) && ( active_trace->process_types->name != NULL ) )
    {
        WINFO_APP( ( "Invalid trace process type. Valid trace process types are:\r\n" ) );
        const trace_process_t *p;
        int printed_anything = 0;
        for ( p = active_trace->process_types; p->name != NULL; p++ )
        {
            printed_anything = 1;
            WINFO_APP( ( "\t%s\r\n", p->name ) );
        }
        if ( !printed_anything )
        {
            WINFO_APP( ( "\t(none)\r\n" ) );
        }
        active_trace = NULL;
        active_trace_process = NULL;
        return ERR_UNKNOWN;
    }

    /* Find the command */
    for ( cmd_ptr = console_command_table; cmd_ptr->name != NULL; cmd_ptr++ )
    {
        if ( strcmp( cmd_argv[0], cmd_ptr->name ) == 0 )
        {
            break;
        }
    }

    /**
     * If a valid command was specified, then cmd_ptr now points to the
     * specified command.
     */

    if ( cmd_ptr->command == NULL )
    {
        active_trace = NULL;
        active_trace_process = NULL;
        return ERR_UNKNOWN_CMD;
    }

    /* Start tracing */
    WINFO_APP( ( "Starting trace...\r\n" ) );
    if ( active_trace->start_command )
    {
        active_trace->start_command( active_trace->process_types->flush_command );
    }
    set_trace_hooks( active_trace->task_hook, active_trace->tick_hook );


    /* Execute the command and fetch the return value */
    int return_value = cmd_ptr->command( cmd_argc, cmd_argv );
    UNUSED_PARAMETER( return_value );

    /* Finish tracing */
    unset_trace_hooks( );
    if ( active_trace->stop_command )
    {
        active_trace->stop_command( );
    }
    WINFO_APP( ( "Ended trace...\r\n" ) );
    if ( active_trace->preprocess_command )
    {
        active_trace->preprocess_command( );
    }
    if ( active_trace_process->process_command )
    {
        active_trace_process->process_command( );
    }
    if ( active_trace->cleanup_command )
    {
        active_trace->cleanup_command( );
    }

    active_trace = NULL;
    active_trace_process = NULL;

    return ERR_CMD_OK;
} /* trace */


int console_start_trace( int argc, char *argv[] )
{
    if ( active_trace != NULL )
    {
        WINFO_APP( ( "Already tracing...\r\n" ) );
        return ERR_CMD_OK;
    }

    int cmd_argc;
    char **cmd_argv;

    /* Make sure a command and trace type was specified */
    if ( argc < 2 )
    {
        return ERR_INSUFFICENT_ARGS;
    }

    cmd_argc = argc - 1;
    cmd_argv = &argv[1];

    /* Find the trace type */
    for ( active_trace = trace_types; active_trace->name != NULL; active_trace++ )
    {
        if ( strcmp( cmd_argv[0], active_trace->name ) == 0 )
        {
            cmd_argc--;
            cmd_argv = &cmd_argv[1];
            break;
        }
    }

    /**
     * If a valid trace type was specified, then cmd_ptr now points to the
     * specified command.
     */

    if ( active_trace->name == NULL )
    {
        WINFO_APP( ( "Invalid trace type. Valid trace types are:\r\n" ) );
        const trace_t *p;
        int printed_anything = 0;
        for ( p = trace_types; p->name != NULL; p++ )
        {
            printed_anything = 1;
            WINFO_APP( ( "\t%s\r\n", p->name ) );
        }
        if ( !printed_anything )
        {
            WINFO_APP( ( "\t(none)\r\n" ) );
        }
        active_trace = NULL;
        active_trace_process = NULL;
        return ERR_UNKNOWN;
    }

    /* Start tracing */
    WINFO_APP( ( "Starting trace...\r\n" ) );
    if ( active_trace->start_command )
    {
        active_trace->start_command( NULL );
    }
    set_trace_hooks( active_trace->task_hook, active_trace->tick_hook );

    return ERR_CMD_OK;
} /* console_start_trace */


int console_end_trace( int argc, char *argv[] )
{
    int cmd_argc = argc - 1;
    char **cmd_argv = &argv[1];

    if ( active_trace == NULL )
    {
        WINFO_APP( ( "Trace not active!\r\n" ) );
        return ERR_CMD_OK;
    }

    unset_trace_hooks( );
    if ( active_trace->stop_command )
    {
        active_trace->stop_command( );
    }
    WINFO_APP( ( "Ended trace...\r\n" ) );

    /* Find the trace process type */
    for ( active_trace_process = active_trace->process_types; active_trace_process->name != NULL; active_trace_process++ )
    {
        if ( strcmp( cmd_argv[0], active_trace_process->name ) == 0 )
        {
            cmd_argc--;
            cmd_argv = &cmd_argv[1];
            break;
        }
    }

    if ( ( active_trace_process->name == NULL ) && ( active_trace->process_types->name != NULL ) )
    {
        WINFO_APP( ( "Invalid trace process type. Valid trace process types are:\r\n" ) );
        const trace_process_t *p;
        int printed_anything = 0;
        for ( p = active_trace->process_types; p->name != NULL; p++ )
        {
            printed_anything = 1;
            WINFO_APP( ( "\t%s\r\n", p->name ) );
        }
        if ( !printed_anything )
        {
            WINFO_APP( ( "\t(none)\r\n" ) );
        }
        active_trace = NULL;
        active_trace_process = NULL;
        return ERR_UNKNOWN;
    }

    if ( active_trace_process && active_trace_process->process_command )
    {
        active_trace_process->process_command( );
    }
    if ( active_trace->cleanup_command )
    {
        active_trace->cleanup_command( );
    }

    active_trace = NULL;
    active_trace_process = NULL;

    return ERR_CMD_OK;
} /* console_end_trace */
