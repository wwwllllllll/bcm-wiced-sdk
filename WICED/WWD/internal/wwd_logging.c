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
#include <stdarg.h>

#ifdef WWD_LOGGING_BUFFER_ENABLE

#define LOGGING_BUFFER_SIZE ((unsigned long)4096)


static unsigned long logging_buffer_position = 0;
static char          logging_buffer[LOGGING_BUFFER_SIZE];


int wwd_logging_printf(const char *format, ...)
{
    int potential_num_written = 0;
    va_list args;
    va_start (args, format);
    potential_num_written = vsnprintf (&logging_buffer[logging_buffer_position], (size_t)(LOGGING_BUFFER_SIZE - logging_buffer_position), format, args);
    if ( potential_num_written > (int)(LOGGING_BUFFER_SIZE - logging_buffer_position) )
    {
        /* full print did not fit in buffer - wipe what was just written and reprint at start of buffer */
        memset( &logging_buffer[logging_buffer_position], (int)'\n', (size_t)(LOGGING_BUFFER_SIZE - logging_buffer_position ) );
        logging_buffer_position = 0;
        potential_num_written = vsnprintf (&logging_buffer[logging_buffer_position], (size_t)(LOGGING_BUFFER_SIZE - logging_buffer_position), format, args);
    }
    logging_buffer_position += potential_num_written;
    if ( logging_buffer_position >= LOGGING_BUFFER_SIZE )
    {
        logging_buffer_position = 0;
    }

    va_end (args);
    return potential_num_written;
}

#endif /* ifdef WWD_LOGGING_BUFFER_ENABLE */

