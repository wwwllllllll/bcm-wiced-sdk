/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/*
 * @file
 * Interface functions for Newlib libC implementation
 */


#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <malloc.h>
#include <wwd_assert.h>
#include <wwd_constants.h>

#ifdef LINT  /* Some defines to keep lint happy in the absense of newlib */
typedef char *  caddr_t;
struct mallinfo { int x; };
/*@-declundef@*/ /*@-exportheader@*/ extern struct mallinfo mallinfo(void); /*@+declundef@*/ /*@+exportheader@*/
#endif /* ifdef LINT */

/*@-declundef@*/ /*@-exportheadervar@*/ /* Lint: These variables are defined by the linker or by newlib */

#undef errno
extern int errno;


/* sbrk
 * Increase program data space.
 * Malloc and related functions depend on this
 */
extern unsigned char _heap[];
extern unsigned char _eheap[];
/*@+declundef@*/ /*@+exportheadervar@*/

static /*@shared@*/ unsigned char *sbrk_heap_top = _heap;

/*@-exportheader@*/ /* Lint: These functions are used by newlib, but it does not provide a header */

/*@shared@*/ caddr_t _sbrk( int incr )
{
    unsigned char *prev_heap;

    if ( sbrk_heap_top + incr > _eheap )
    {
        /* Out of dynamic memory heap space */

        volatile struct mallinfo mi = mallinfo();

        // See variable mi for malloc information:
        // Total allocated :  mi.uordblks
        // Total free :       mi.fordblks

        wiced_assert("Out of dynamic memory heap space", 0 != 0 );

        UNUSED_VARIABLE( mi );

        errno = ENOMEM;
        return (caddr_t) -1;
    }
    prev_heap = sbrk_heap_top;

    sbrk_heap_top += incr;

    return (caddr_t) prev_heap;
}

/* Override the default Newlib assert, since it tries to do printf stuff */

void __assert_func( const char * file, int line, const char * func, const char * failedexpr )
{
    /* Assertion failed!
     *
     * To find out where this assert was triggered, either look up the call stack,
     * or inspect the file, line and function parameters
     */
    wiced_assert("newlib assert", 0 != 0 );

    UNUSED_PARAMETER( file );
    UNUSED_PARAMETER( line );
    UNUSED_PARAMETER( func );
    UNUSED_PARAMETER( failedexpr );
}

/*
 * These are needed for C++ programs. They shouldn't really be here, so let's just
 * hit a breakpoint when these functions are called.
 */

#if 1

int _kill( int pid, int sig )
{
    wiced_assert("", 0 != 0 );

    UNUSED_PARAMETER( pid );
    UNUSED_PARAMETER( sig );
    return 0;
}

int _getpid( void )
{
    wiced_assert("", 0 != 0 );
    return 0;
}

/*@+exportheader@*/

#endif

/* Search memory in reverse for the last instance of a character in a buffer*/
void *memrchr( const void *source_buffer, int search_character, size_t buffer_length )
{
    unsigned char * read_pos = ((unsigned char *)source_buffer + buffer_length);
    while ( ( *read_pos != (unsigned char) search_character ) &&
            ( read_pos >= (unsigned char*) source_buffer ) )
    {
        read_pos--;
    }
    return ( read_pos >= (unsigned char*) source_buffer )?read_pos:NULL;
}

