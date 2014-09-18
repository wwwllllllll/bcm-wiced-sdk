/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

static void *memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen )
{
    unsigned char* needle_char = (unsigned char*) needle;
    unsigned char* haystack_char = (unsigned char*) haystack;
    int needle_pos = 0;

    if ( ( haystack == NULL ) ||
         ( needle == NULL ) )
    {
        return NULL;
    }

    while ( ( haystacklen > 0 ) &&
            ( needle_pos < needlelen ) )
    {
        if ( *haystack_char == needle_char[needle_pos] )
        {
            needle_pos++;
        }
        else if ( needle_pos != 0 )
        {
            /* go back to start of section */
            haystacklen += needle_pos;
            haystack_char -= needle_pos;
            needle_pos = 0;
        }
        haystack_char++;
        haystacklen--;
    }

    if ( needle_pos == needlelen )
    {
       return ((unsigned char*)haystack_char) - needlelen;
    }

    return NULL;
}

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
