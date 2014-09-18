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
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "wiced_utilities.h"
#include "wwd_crypto.h"

/* Message queue constants. */
#define archMESG_QUEUE_LENGTH     ( (unsigned long) 6 )
#define archPOST_BLOCK_TIME_MS    ( ( unsigned long ) 10000 )

/* The timeout code seems to be unused */
#if LWIP_SYS_ARCH_TIMEOUTS
struct timeout_list
{
    struct sys_timeouts timeouts;
    xTaskHandle pid;
};
static struct timeout_list timeout_list[SYS_THREAD_MAX];
static uint16_t next_thread = 0;
#endif /* if LWIP_SYS_ARCH_TIMEOUTS */

/* This is the number of threads that can be started with sys_thread_new() */
#define SYS_THREAD_MAX                 (4)

#define lwipTCP_STACK_SIZE             (600)
#define lwipBASIC_SERVER_STACK_SIZE    (250)

/*-----------------------------------------------------------------------------------
 * Creates an empty mailbox.
 */
err_t sys_mbox_new( /*@special@*/ /*@out@*/ sys_mbox_t *mbox, /*@unused@*/int size ) /*@allocates *mbox@*/  /*@defines **mbox@*/
{
    /*@-noeffect@*/
    (void) size; /* unused parameter */
    /*@+noeffect@*/

    *mbox = xQueueCreate( archMESG_QUEUE_LENGTH, (unsigned long) sizeof(void *) );

    /*@-compdef@*/ /* Lint doesnt realise allocation has occurred */
    return ERR_OK;
    /*@+compdef@*/
}

/*-----------------------------------------------------------------------------------
 * Deallocates a mailbox. If there are messages still present in the
 * mailbox when the mailbox is deallocated, it is an indication of a
 * programming error in lwIP and the developer should be notified.
 */
void sys_mbox_free( /*@special@*/ sys_mbox_t *mbox ) /*@releases *mbox@*/
{
    if ( uxQueueMessagesWaiting( *mbox ) != 0 )
    {
        /* Line for breakpoint.  Should never break here! */
#ifdef __GNUC__
        __asm volatile ( "NOP" );
#elif defined( __IAR_SYSTEMS_ICC__ )
        asm( "NOP" );
#endif
    }

    vQueueDelete( *mbox );
}

/*-----------------------------------------------------------------------------------
 * Posts the "msg" to the mailbox.
 */
void sys_mbox_post( sys_mbox_t *mbox, void *msg )
{
    signed portBASE_TYPE retval;
    retval = xQueueSend( *mbox, &msg, ( portTickType ) ( archPOST_BLOCK_TIME_MS / portTICK_RATE_MS ) );
    LWIP_ASSERT("Error posting to LwIP mailbox", retval == pdTRUE );
}

/*-----------------------------------------------------------------------------------
 * Blocks the thread until a message arrives in the mailbox, but does
 * not block the thread longer than "timeout" milliseconds (similar to
 * the sys_arch_sem_wait() function). The "msg" argument is a result
 * parameter that is set by the function (i.e., by doing "*msg =
 * ptr"). The "msg" parameter maybe NULL to indicate that the message
 * should be dropped.
 *
 * The return values are the same as for the sys_arch_sem_wait() function:
 * Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
 * timeout.
 *
 * Note that a function with a similar name, sys_mbox_fetch(), is
 * implemented by lwIP.
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, /*@null@*/ /*@out@*/ void **msg, u32_t timeout)
{
    void *dummyptr;
    void ** tmp_ptr;
    portTickType start_time, end_time, elapsed_time;

    start_time = xTaskGetTickCount( );

    if ( msg == NULL )
    {
        tmp_ptr = &dummyptr;
    }
    else
    {
        tmp_ptr = msg;
    }

    if ( timeout != 0 )
    {
        if ( pdTRUE == xQueueReceive( *mbox, tmp_ptr, timeout ) )
        {
            end_time = xTaskGetTickCount( );
            elapsed_time = end_time - start_time;
            if ( elapsed_time == 0 )
            {
                elapsed_time = (portTickType) 1;
            }
            return ( elapsed_time );
        }
        else /* timed out blocking for message */
        {
            if ( msg != NULL )
            {
                *msg = NULL;
            }
            return SYS_ARCH_TIMEOUT;
        }
    }
    else /* block forever for a message. */
    {
        if ( xQueueReceive( *mbox, &(*tmp_ptr), (portTickType) 0xFFFFFFFF ) == errQUEUE_EMPTY)
        {
            *tmp_ptr = NULL;
            return SYS_ARCH_TIMEOUT;
        }

        end_time = xTaskGetTickCount( );
        elapsed_time = end_time - start_time;
        if ( elapsed_time == 0 )
        {
            elapsed_time = (portTickType) 1;
        }
        return ( elapsed_time ); /* return time blocked TBD test */
    }
}

/*-----------------------------------------------------------------------------------
 * Creates and returns a new semaphore. The "count" argument specifies
 * the initial state of the semaphore. TBD finish and test
 */
err_t sys_sem_new( /*@special@*/ /*@out@*/ sys_sem_t *sem, u8_t count) /*@allocates *sem@*/
{
    if ( sem == NULL )
    {
        /*@-mustdefine@*/ /*@-compdef@*/ /* do not need to allocate or set *sem */
        return ERR_VAL;
        /*@+mustdefine@*/ /*@+compdef@*/
    }

    portENTER_CRITICAL();
    vSemaphoreCreateBinary( *sem )
    if ( *sem == NULL )
    {
        portEXIT_CRITICAL();
        /*@-mustdefine@*/ /*@-compdef@*/ /* allocation failed - dont allocate or set *sem */
        return ERR_MEM;
        /*@+mustdefine@*/ /*@+compdef@*/
    }

    if ( count == (u8_t) 0 ) /* Means it can't be taken */
    {
        if ( pdTRUE != xSemaphoreTake( *sem, (portTickType) 1 ) )
        {
            vQueueDelete( *sem );
            portEXIT_CRITICAL();
            /*@-compdef@*/ /* take failed - dont allocate or set *sem */
            return ERR_VAL;
            /*@+compdef@*/
        }
    }

    portEXIT_CRITICAL();

    /*@-compdef@*/ /* Lint doesnt realise allocation has occurred */
    return ERR_OK;
    /*@+compdef@*/
}

/*-----------------------------------------------------------------------------------
 * Blocks the thread while waiting for the semaphore to be
 * signaled. If the "timeout" argument is non-zero, the thread should
 * only be blocked for the specified time (measured in
 * milliseconds).
 *
 * If the timeout argument is non-zero, the return value is the number of
 * milliseconds spent waiting for the semaphore to be signaled. If the
 * semaphore wasn't signaled within the specified time, the return value is
 * SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
 * (i.e., it was already signaled), the function may return zero.
 *
 * Notice that lwIP implements a function with a similar name,
 * sys_sem_wait(), that uses the sys_arch_sem_wait() function.
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    portTickType start_time, end_time, elapsed_time;

    start_time = xTaskGetTickCount( );

    if ( timeout != 0 )
    {
        if ( xSemaphoreTake( *sem, timeout ) == (long) pdTRUE )
        {
            end_time = xTaskGetTickCount( );
            elapsed_time = end_time - start_time;
            if ( elapsed_time == 0 )
            {
                elapsed_time = (portTickType) 1;
            }
            return ( elapsed_time ); /* return time blocked TBD test */
        }
        else
        {
            return SYS_ARCH_TIMEOUT;
        }
    }
    else /* must block without a timeout */
    {
        while ( xSemaphoreTake( *sem, (portTickType) 10000 ) != (long) pdTRUE )
        {
            /* Do nothing */
        }
        end_time = xTaskGetTickCount( );
        elapsed_time = end_time - start_time;
        if ( elapsed_time == 0 )
        {
            elapsed_time = (portTickType) 1;
        }

        return ( elapsed_time ); /* return time blocked */

    }
}

/*-----------------------------------------------------------------------------------
 * Signals a semaphore
 */
void sys_sem_signal( sys_sem_t *sem )
{
    signed portBASE_TYPE result;

    result = xSemaphoreGive( *sem );
    /*@-noeffect@*/
    (void) result;  /* unused in release build */
    /*@+noeffect@*/

    LWIP_ASSERT( "FreeRTOS failed to set semaphore for LwIP", result == pdTRUE );
}

/*-----------------------------------------------------------------------------------
 * Deallocates a semaphore
 */
void sys_sem_free( /*@special@*/ sys_sem_t *sem ) /*@releases *sem@*/
{
    vQueueDelete( *sem );
}

/*-----------------------------------------------------------------------------------
 * Initialize sys arch
 */
void sys_init( void )
{
#if LWIP_SYS_ARCH_TIMEOUTS
    int i;
    /* Initialize the the per-thread sys_timeouts structures
     * make sure there are no valid pids in the list
     */
    for(i = 0; i < SYS_THREAD_MAX; i++)
    {
        timeout_list[i].pid = 0;
    }

    /* keep track of how many threads have been created */
    next_thread = 0;
#endif /* if LWIP_SYS_ARCH_TIMEOUTS */
}

void sys_deinit( void )
{
}

#if LWIP_SYS_ARCH_TIMEOUTS
/*-----------------------------------------------------------------------------------
 * Returns a pointer to the per-thread sys_timeouts structure. In lwIP,
 * each thread has a list of timeouts which is represented as a linked
 * list of sys_timeout structures. The sys_timeouts structure holds a
 * pointer to a linked list of timeouts. This function is called by
 * the lwIP timeout scheduler and must not return a NULL value.
 *
 * In a single threaded sys_arch implementation, this function will
 * simply return a pointer to a global sys_timeouts variable stored in
 * the sys_arch module.
 */

struct sys_timeouts *
sys_arch_timeouts(void)
{
    int i;
    xTaskHandle pid;
    struct timeout_list *tl;

    pid = xTaskGetCurrentTaskHandle( );

    for(i = 0; i < next_thread; i++)
    {
        tl = &timeout_list[i];
        if(tl->pid == pid)
        {
            return &(tl->timeouts);
        }
    }

    /* Error */
    return NULL;
}
#endif /* if LWIP_SYS_ARCH_TIMEOUTS */

/*-----------------------------------------------------------------------------------
 * Starts a new thread with priority "prio" that will begin its execution in the
 * function "thread()". The "arg" argument will be passed as an argument to the
 * thread() function. The id of the new thread is returned. Both the id and
 * the priority are system dependent.
 */
/*@null@*/ sys_thread_t sys_thread_new( const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio )
{
    xTaskHandle created_task;
    signed portBASE_TYPE result;
    /* The first time this is called we are creating the lwIP handler. */
    result = xTaskCreate( thread, ( signed char * ) name, (unsigned short) stacksize, arg, (unsigned portBASE_TYPE) prio, &created_task );

#if LWIP_SYS_ARCH_TIMEOUTS
    /* For each task created, store the task handle (pid) in the timers array.
     * This scheme doesn't allow for threads to be deleted
     */
    timeout_list[next_thread++].pid = created_task;
#endif /* if LWIP_SYS_ARCH_TIMEOUTS */

    if ( result == pdPASS )
    {
        return created_task;
    }
    else
    {
        return NULL;
    }
}

void sys_thread_exit( void )
{
    malloc_leak_check(NULL, LEAK_CHECK_THREAD);
    vTaskDelete(NULL);
}

void sys_thread_free( sys_thread_t task )
{
#if ( configFREE_TASKS_IN_IDLE == 0 )
    vTaskFreeTerminated( task );
#endif /* if ( configFREE_TASKS_IN_IDLE == 0 ) */
}

/*-----------------------------------------------------------------------------------
 * This optional function does a "fast" critical region protection and returns
 * the previous protection level. This function is only called during very short
 * critical regions. An embedded system which supports ISR-based drivers might
 * want to implement this function by disabling interrupts. Task-based systems
 * might want to implement this by using a mutex or disabling tasking. This
 * function should support recursive calls from the same task or interrupt. In
 * other words, sys_arch_protect() could be called while already protected. In
 * that case the return value indicates that it is already protected.
 *
 * sys_arch_protect() is only required if your port is supporting an operating
 * system.
 */
sys_prot_t sys_arch_protect( void )
{
    vPortEnterCritical( );
    return 1;
}

/*-----------------------------------------------------------------------------------
 * This optional function does a "fast" set of critical region protection to the
 * value specified by pval. See the documentation for sys_arch_protect() for
 * more information. This function is only required if your port is supporting
 * an operating system.
 */
void sys_arch_unprotect( sys_prot_t pval )
{
    /*@-noeffect@*/
    (void) pval;  /* Parameter is unused */
    /*@+noeffect@*/
    vPortExitCritical( );
}

/*-----------------------------------------------------------------------------------
 * Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
 * return with SYS_MBOX_EMPTY.  On success, 0 is returned.
 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    void *dummy_ptr;
    void ** tmp_ptr = msg;

    if ( msg == NULL )
    {
        tmp_ptr = &dummy_ptr;
    }

    if ( pdTRUE == xQueueReceive( *mbox, tmp_ptr, 0 ) )
    {
        return ERR_OK;
    }
    else
    {
        return SYS_MBOX_EMPTY;
    }
}


/*-----------------------------------------------------------------------------------
 * Try to post the "msg" to the mailbox.
 */
err_t sys_mbox_trypost( sys_mbox_t *mbox, void *msg )
{
    err_t result;

    if ( xQueueSend( *mbox, &msg, 0 ) == pdPASS )
    {
        result = ERR_OK;
    }
    else
    {
        /* could not post, queue must be full */
        result = ERR_MEM;

#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */

    }

    return result;
}

int sys_mbox_valid( sys_mbox_t *mbox )
{
    return (int)( *mbox != 0 );
}

int sys_sem_valid( sys_sem_t *sem )
{
    return (int)( *sem != 0 );
}

void sys_mbox_set_invalid( sys_mbox_t *mbox )
{
    ( *(int*) mbox ) = 0;
}

void sys_sem_set_invalid( sys_sem_t *sem )
{
    ( *(int*) sem ) = 0;
}



uint16_t sys_rand16( void )
{
    uint16_t output;
    wwd_wifi_get_random( &output, 2 );
    return output;
}
