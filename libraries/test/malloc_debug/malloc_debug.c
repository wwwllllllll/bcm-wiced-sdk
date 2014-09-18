/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <wiced_utilities.h>
#include <wwd_rtos.h>
#include <wwd_assert.h>


//#define MALLOC_FREE_STRICT_ORDER

#define MALLOC_PADDING_SIZE (8)
#define MALLOC_PADDING_VALUE (0xA5)
#define NUM_BACKTRACE_ITEMS (4)
#define MALLOC_MAGIC_NUM  (0xD94BC579)


#if 1
#define MALLOC_LOG(x)
#else
#define MALLOC_LOG(x)   printf x
#endif

typedef struct malloc_elem_struct
{
    uint32_t magic_num;
    uint32_t size;
    const char* name;
    uint32_t num;
    struct malloc_elem_struct* next;
    struct malloc_elem_struct* prev;
    void* caller;
    malloc_thread_handle thread;
    enum { LEAK_CHECK = 0, LEAK_BASE = 1, LEAK_NO_CHECK = 2 } no_leak_flag;
} malloc_elem_t;

extern void *      __real_malloc  ( size_t size );
extern void *      __wrap_malloc  ( size_t size );
extern void *      __wrap_realloc ( void *ptr, size_t size );
extern void *      __real_realloc ( void *ptr, size_t size );
extern void *      __wrap_calloc  ( size_t nelem, size_t elsize );
extern void *      __real_calloc  ( size_t nelem, size_t elsize );
extern void        __real_free    ( void *m );
extern void        __wrap_free    ( void* m );
extern void *      __builtin_return_address ( unsigned int level );
static void *      malloc_generic( const char* name, size_t size, void* caller );

void check_mallocs( void );
static void malloc_error( const char* error_message, malloc_elem_t* malloc_block_details );


static const char* curr_malloc_name = NULL;
static uint32_t max_allocated = 0;
static uint32_t curr_allocated = 0;
static malloc_elem_t* malloc_block_list = NULL;

static volatile uint32_t break_at_malloc_num = 0xffffffff;

#ifdef MALLOC_MOCKED
void * malloc_debug_malloc( size_t size )
#else /* ifdef MALLOC_MOCKED */
void * __wrap_malloc( size_t size )
#endif /* ifdef MALLOC_MOCKED */
{
    return malloc_generic( curr_malloc_name, size, __builtin_return_address( 0 ) );
}


void * malloc_named( const char* name, size_t size )
{
    return malloc_generic( name, size, __builtin_return_address( 0 ) );
}

void * malloc_named_hideleak( const char* name, size_t size )
{
    void* ret = malloc_generic( name, size, __builtin_return_address( 0 ) );
    if ( ret != NULL )
    {
        malloc_elem_t*elem = (malloc_elem_t*)  ((char*)ret-MALLOC_PADDING_SIZE-sizeof(malloc_elem_t));
        elem->no_leak_flag = LEAK_NO_CHECK;
    }
    return ret;
}

void *__wrap_calloc(size_t nelem, size_t elsize)
{
    void* ret = malloc_generic( curr_malloc_name, elsize*nelem, __builtin_return_address( 0 ) );
    memset(ret, 0, elsize*nelem);
    return ret;
}

void *calloc_named(const char* name, size_t nelem, size_t elsize)
{
    void* ret = malloc_generic( name, elsize*nelem, __builtin_return_address( 0 ) );
    memset(ret, 0, elsize*nelem);
    return ret;
}

void * calloc_named_hideleak( const char* name, size_t nelem, size_t elsize )
{
    void* ret = malloc_generic( name, elsize*nelem, __builtin_return_address( 0 ) );
    if ( ret != NULL )
    {
        malloc_elem_t*elem = (malloc_elem_t*)  ((char*)ret-MALLOC_PADDING_SIZE-sizeof(malloc_elem_t));
        elem->no_leak_flag = LEAK_NO_CHECK;
    }
    memset(ret, 0, elsize*nelem);
    return ret;
}

void *__wrap_realloc(void *ptr, size_t size)
{
    malloc_elem_t* elem;
    malloc_elem_t* tmp;
    malloc_elem_t* ret;

    if ( size == 0 )
    {
        __wrap_free( ptr );
        return NULL;
    }

    if ( ptr == NULL )
    {
        return __wrap_malloc( size );
    }

    check_mallocs( );

    MALLOC_LOG(( "realloc %d bytes at 0x%x\n", size, ptr ));

    WICED_DISABLE_INTERRUPTS( );
    elem = (malloc_elem_t*)  ((char*)ptr-MALLOC_PADDING_SIZE-sizeof(malloc_elem_t));

    tmp = malloc_block_list;
    while ( ( tmp != NULL ) && ( tmp != elem ) )
    {
        tmp = tmp->next;
    }
    if ( tmp == NULL )
    {
        malloc_error( "realloc arg not in malloc list", elem );
    }

    ret = (malloc_elem_t*) __real_realloc( elem, size + 2*MALLOC_PADDING_SIZE + sizeof( malloc_elem_t ) );

    if ( ret == NULL )
    {
        return NULL;
    }

    if ( ret != elem )
    {
        if ( malloc_block_list == elem )
        {
            malloc_block_list = ret;
        }

        if ( ret->next != NULL )
        {
            ret->next->prev = ret;
        }
        if ( ret->prev != NULL )
        {
            ret->prev->next = ret;
        }
    }
    ret->size = size;

    WICED_ENABLE_INTERRUPTS();

    return ((char*)ret) + MALLOC_PADDING_SIZE + sizeof( malloc_elem_t );
}

static void * malloc_generic( const char* name, size_t size, void* caller )
{
    malloc_elem_t* item_ptr;
    char* padding_ptr;
    static uint32_t malloc_counter = 0;

    check_mallocs( );

    WICED_DISABLE_INTERRUPTS( );

    if ( break_at_malloc_num == malloc_counter )
    {
        malloc_error( "Stopped at requested malloc number", NULL );
    }

    item_ptr = __real_malloc ( size + 2*MALLOC_PADDING_SIZE + sizeof( malloc_elem_t ) );

    if ( item_ptr == NULL )
    {
        return NULL;
    }
    item_ptr->magic_num = MALLOC_MAGIC_NUM;
    item_ptr->name = name;
    item_ptr->size = size;
    item_ptr->num  = malloc_counter;
    malloc_counter++;
    item_ptr->caller = caller;


    if ( malloc_block_list == NULL )
    {
        malloc_block_list = item_ptr;
        item_ptr->next = NULL;
        item_ptr->prev = NULL;
    }
    else
    {
        malloc_block_list->prev = item_ptr;
        item_ptr->next = malloc_block_list;
        item_ptr->prev = NULL;
        malloc_block_list = item_ptr;
    }

    padding_ptr = (char*)item_ptr + sizeof( malloc_elem_t );
    memset( padding_ptr, MALLOC_PADDING_VALUE, 2*MALLOC_PADDING_SIZE + size  );

    curr_allocated += size;
    if ( curr_allocated > max_allocated )
    {
        max_allocated = curr_allocated;
    }

    item_ptr->no_leak_flag = LEAK_CHECK;
    item_ptr->thread = malloc_get_current_thread( );

    WICED_ENABLE_INTERRUPTS( );

    MALLOC_LOG(( "malloc %d bytes at 0x%x (#%d %s 0x%x)\n", size, padding_ptr + MALLOC_PADDING_SIZE, item_ptr->num, name? name: "", item_ptr ));

    return padding_ptr + MALLOC_PADDING_SIZE;
}

void malloc_set_name( const char* name )
{
    curr_malloc_name = name;
}

void __wrap_free (void* m)
{
    malloc_elem_t* elem;

    check_mallocs( );

    WICED_DISABLE_INTERRUPTS( );

    elem = (malloc_elem_t*)  ((char*)m-MALLOC_PADDING_SIZE-sizeof(malloc_elem_t));

    if ( break_at_malloc_num == elem->num )
    {
        malloc_error( "Stopped at requested free number", NULL );
    }

    MALLOC_LOG(( "free %d bytes at 0x%x (#%d name=%s wrapper=0x%x)\n", elem->size, m, elem->num, elem->name? elem->name : "", elem ));

#ifdef MALLOC_FREE_STRICT_ORDER
    if ( elem != malloc_block_list )
    {
        malloc_error( "Free not in reverse order of malloc", elem );
    }
#else
    {
        malloc_elem_t* tmp = malloc_block_list;
        while ( ( tmp != NULL ) && ( tmp != elem ) )
        {
            tmp = tmp->next;
        }
        if ( tmp == NULL )
        {
            malloc_error( "Free arg not in malloc list", m );
        }
    }
#endif


    if ( elem->no_leak_flag == LEAK_BASE )
    {
        malloc_error( "Freeing block marked as base", elem );
    }

    if ( elem->thread != malloc_get_current_thread( ) )
    {
        /* Disabled due to Massively complex set of mallocs from TLS - hard to transfer to correct thread */
        /* malloc_error( "Freeing block from wrong thread", elem ); */
    }

    if ( elem->next != NULL )
    {
        elem->next->prev = elem->prev;
    }
    if ( elem->prev != NULL )
    {
        elem->prev->next = elem->next;
    }
    if ( elem == malloc_block_list )
    {
        malloc_block_list = elem->next;
    }

    curr_allocated -= elem->size;

    __real_free (elem);
    WICED_ENABLE_INTERRUPTS( );
}

void malloc_error( const char* error_message, malloc_elem_t* malloc_block_details )
{
    /* Dynamic memory error
     * Please see variables below for details
     *
     * You can additionally re-run the debugger and set variable break_at_malloc_num
     * to match the value in malloc_block_details->num which will catch the point of allocation
     */
    (void) error_message;
    (void) malloc_block_details;
    (void) malloc_block_list;
    wiced_assert("Dynamic memory error", 0 != 0 );
}

void check_mallocs( void )
{
    malloc_elem_t* tmp;
    uint32_t alloc_count = 0;

    WICED_DISABLE_INTERRUPTS( );
    tmp = malloc_block_list;
    while ( tmp != NULL )
    {
        unsigned char* padding1 = (unsigned char*)tmp + sizeof(malloc_elem_t);
        unsigned char* padding2 = padding1 + MALLOC_PADDING_SIZE + tmp->size;
        int i = 0;
        while ( ( i < MALLOC_PADDING_SIZE ) &&
                ( padding1[i] == MALLOC_PADDING_VALUE ) &&
                ( padding2[i] == MALLOC_PADDING_VALUE ) )
        {
            i++;
        }
        if ( i != MALLOC_PADDING_SIZE )
        {
            malloc_error( "Padding bytes overwritten", tmp );
        }
        if ( tmp->magic_num != MALLOC_MAGIC_NUM )
        {
            malloc_error( "Magic Number corrupted", tmp );
        }

        alloc_count += tmp->size;
        tmp = tmp->next;
    }

    if ( alloc_count != curr_allocated )
    {
        malloc_error( "Allocation count missmatch", NULL );
    }
    WICED_ENABLE_INTERRUPTS( );
}


void  malloc_leak_set_ignored( leak_check_scope_t global_flag )
{
    malloc_elem_t* tmp;

    check_mallocs( );

    tmp = malloc_block_list;
    while ( tmp != NULL )
    {
        if ( ( global_flag != 0 ) || ( tmp->thread == malloc_get_current_thread( ) ) )
        {
            tmp->no_leak_flag = LEAK_NO_CHECK;
        }
        tmp = tmp->next;
    }
}



void  malloc_leak_set_base( leak_check_scope_t global_flag )
{
    malloc_elem_t* tmp;

    check_mallocs( );

    tmp = malloc_block_list;
    while ( tmp != NULL )
    {
        if ( ( global_flag != 0 ) || ( tmp->thread == malloc_get_current_thread( ) ) )
        {
            tmp->no_leak_flag = LEAK_BASE;
        }
        tmp = tmp->next;
    }
}
void malloc_leak_check( malloc_thread_handle thread, leak_check_scope_t global_flag )
{
    malloc_elem_t* tmp;

    check_mallocs( );

    if ( thread == NULL )
    {
        thread = malloc_get_current_thread( );
    }

    tmp = malloc_block_list;
    while ( tmp != NULL )
    {
        if ( ( ( global_flag == LEAK_CHECK_GLOBAL ) || ( tmp->thread == thread ) ) && ( tmp->no_leak_flag == LEAK_CHECK ) )
        {
            malloc_error( "Memory leak", tmp );
        }
        tmp = tmp->next;
    }
}

void malloc_transfer_to_thread( void* block, malloc_thread_handle thread )
{
    malloc_elem_t* elem;
    malloc_elem_t* tmp;

    check_mallocs( );

    elem = (malloc_elem_t*)  ((char*)block-MALLOC_PADDING_SIZE-sizeof(malloc_elem_t));

    tmp = malloc_block_list;
    while ( ( tmp != NULL ) && ( tmp != elem ) )
    {
        tmp = tmp->next;
    }
    if ( tmp == NULL )
    {
        malloc_error( "Transfer arg not in malloc list", elem );
    }

    elem->thread = thread;
}


void malloc_transfer_to_curr_thread( void* block )
{
    malloc_transfer_to_thread( block, malloc_get_current_thread( ) );
}


