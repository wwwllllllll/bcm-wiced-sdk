/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <stdlib.h>
#include <yfuns.h>

int errno;
extern void platform_stdio_write( const char* str, uint32_t len );

size_t __write( int handle, const unsigned char * buffer, size_t size )
{

    if ( buffer == 0 )
    {
        return 0;
    }

    platform_stdio_write( (const char*)buffer, size );

    return size;
}
