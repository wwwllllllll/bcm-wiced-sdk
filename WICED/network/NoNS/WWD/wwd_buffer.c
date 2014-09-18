/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "network/wwd_buffer_interface.h"
#include <string.h>
#include <stdint.h>
#include "wwd_assert.h"

static /*@null@*/ wiced_buffer_t internal_buffer           = 0;
static uint16_t                      internal_buffer_max_size  = 0;
static uint16_t                      internal_buffer_curr_size = 0;

wwd_result_t host_buffer_init( void * native_arg )
{
    wiced_assert("Error: Invalid native_arg\n", native_arg != NULL);

    internal_buffer = (wiced_buffer_t) ( (nons_buffer_init_t*) native_arg )->internal_buffer;
    internal_buffer_max_size = ( (nons_buffer_init_t*) native_arg )->buff_size;
    return WWD_SUCCESS;
}

wwd_result_t host_buffer_check_leaked( void )
{
    /* Nothing to do */
    return WWD_SUCCESS;
}

wwd_result_t host_buffer_get( /*@out@*/ wiced_buffer_t * buffer, wwd_buffer_dir_t direction, unsigned short size, wiced_bool_t wait )
{
    /*@-noeffect@*/
    UNUSED_PARAMETER( direction );
    UNUSED_PARAMETER( wait );
    /*@+noeffect@*/
    wiced_assert("Error: Invalid buffer size\n", size != 0);

    if ( (uint16_t) size > internal_buffer_max_size )
    {
        *buffer = NULL;
        return WWD_BUFFER_UNAVAILABLE_PERMANENT;
    }
    internal_buffer_curr_size = (uint16_t) size;
    *buffer = internal_buffer;
    return WWD_SUCCESS;
}

void host_buffer_release( /*@only@*/ wiced_buffer_t buffer, wwd_buffer_dir_t direction )
{
    /*@-noeffect@*/
    UNUSED_PARAMETER( buffer );
    UNUSED_PARAMETER( direction );
    /*@+noeffect@*/

    /*@-mustfree@*/ /* Buffers are not actually allocated in NoNS, hence are not freed */
    return;
    /*@+mustfree@*/
}

/*@exposed@*/ uint8_t* host_buffer_get_current_piece_data_pointer( /*@dependent@*/ wiced_buffer_t buffer )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    return (uint8_t *) buffer;
}

uint16_t host_buffer_get_current_piece_size( wiced_buffer_t buffer )
{
    /*@-noeffect@*/
    UNUSED_PARAMETER( buffer );
    /*@+noeffect@*/
    return internal_buffer_curr_size;
}

/*@exposed@*/ /*@null@*/ wiced_buffer_t host_buffer_get_next_piece( wiced_buffer_t buffer )
{
    /*@-noeffect@*/
    UNUSED_PARAMETER( buffer );
    /*@+noeffect@*/
    return NULL;
}

wwd_result_t host_buffer_add_remove_at_front( wiced_buffer_t * buffer, int32_t add_remove_amount )
{
    *buffer += add_remove_amount;
    internal_buffer_curr_size = (uint16_t) ( internal_buffer_curr_size - add_remove_amount );

    return WWD_SUCCESS;
}

wwd_result_t host_buffer_set_size( wiced_buffer_t buffer, unsigned short size )
{
    UNUSED_PARAMETER( buffer );
    internal_buffer_curr_size = size;
    return WWD_SUCCESS;
}
