/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#ifdef WICED

#include "headers.h"
#include "Thread.h"
#include "Condition.h"
#include "Settings.hpp"
#include "Locale.h"

/******************************************************
 *             Static Variables
 ******************************************************/
int         thread_sNum = 0;                /* Number of currently running threads */
int         nonterminating_num = 0;         /* Number of non-terminating running
                                               threads (ie listener thread) */
Condition   thread_sNum_cond;               /* Condition to protect updating the above
                                               and alerting on changes to above */


void thread_init( void )
{
    IPERF_DEBUGF( CONDITION_DEBUG | IPERF_DBG_TRACE, ( "Initializing thread number condition.\r\n" ) );
    Condition_Initialize( &thread_sNum_cond );
} /* thread_init */


void thread_destroy( void )
{
    IPERF_DEBUGF( CONDITION_DEBUG | IPERF_DBG_TRACE, ( "Destroying thread number condition.\r\n" ) );
    Condition_Destroy( &thread_sNum_cond );
} /* thread_destroy */


void thread_start( struct thread_Settings *thread )
{
    /* Make sure this object has not been started already */
    if ( thread_equalid( thread->mTID, thread_zeroid( ) ) )
    {
        /* Check if we need to start another thread before this one */
        if ( thread->runNow != NULL )
        {
            thread_start( thread->runNow );
        }

        /* Increment thread count */
        Condition_Lock( &thread_sNum_cond );
        IPERF_DEBUGF( THREAD_DEBUG | IPERF_DBG_TRACE, ( "Incrementing thread count from %d to %d.\r\n", thread_sNum, thread_sNum + 1 ) );
        thread_sNum++;
        Condition_Unlock( &thread_sNum_cond );

#ifdef HAVE_THREADS
        xTaskCreate( thread_run_wrapper, (signed char*) thread_names[thread->mThreadMode], IPERF_THREAD_STACKSIZE, (void*) thread, IPERF_THREAD_PRIORITY, &(thread->mTID) );

        if ( thread->mTID == NULL )
        {
            IPERF_DEBUGF( THREAD_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_LEVEL_SERIOUS, ( "xTaskCreate failed.\r\n" ) );

            /* Decrement thread count */
            Condition_Lock( &thread_sNum_cond );
            IPERF_DEBUGF( THREAD_DEBUG | IPERF_DBG_TRACE, ( "Decrementing thread count from %d to %d.\r\n", thread_sNum, thread_sNum - 1 ) )
            thread_sNum--;
            Condition_Unlock( &thread_sNum_cond );

        }
#else /* single threaded */
        /* Call Run_Wrapper in this thread */
        thread_run_wrapper( thread );
#endif
    }
} /* thread_start */


void thread_stop( struct thread_Settings *thread )
{
#ifdef HAVE_THREADS
    /* Make sure we have been started */
    if ( !thread_equalid( thread->mTID, thread_zeroid( ) ) )
    {
        /* Decrement thread count */
        Condition_Lock( &thread_sNum_cond );
        IPERF_DEBUGF( THREAD_DEBUG | IPERF_DBG_TRACE, ( "Decrementing thread count from %d to %d.\r\n", thread_sNum, thread_sNum - 1 ) );
        thread_sNum--;
        IPERF_DEBUGF( CONDITION_DEBUG | IPERF_DBG_TRACE, ( "Signaling on thread number condition.\r\n" ) );
        Condition_Signal( &thread_sNum_cond );
        Condition_Unlock( &thread_sNum_cond );

        /* Use exit()   if called from within this thread */
        /* Use cancel() if called from a different thread */
        if ( thread_equalid( thread_getid( ), thread->mTID ) )
        {
            /* Destroy the object */
            Settings_Destroy( thread );
        }
        else
        {
            /* Destroy the object only after killing the thread */
            Settings_Destroy( thread );
        }
    }
#endif // HAVE_THREADS
} /* thread_stop */


void *thread_run_wrapper( void *parameter )
{
    struct thread_Settings *thread = (struct thread_Settings *) parameter;

    /* Which type of object are we? */
    switch ( thread->mThreadMode )
    {
        case kMode_Server:
            /* Spawn a Server thread with these settings */
            server_spawn( thread );
            break;

        case kMode_Client:
            /* Spawn a Client thread with these settings */
            client_spawn( thread );
            break;

        case kMode_Reporter:
            /* Spawn a Reporter thread with these settings */
            reporter_spawn( thread );
            break;

        case kMode_Listener:
            /* Increment the non-terminating thread count */
            thread_register_nonterm( );

            /* Spawn a Listener thread with these settings */
            listener_spawn( thread );

            /* Decrement the non-terminating thread count */
            thread_unregister_nonterm( );

            break;

        default:
            FAIL( 1, ( "Unknown thread type!\r\n" ), thread );
            break;
    }

    /* Decrement thread count and send condition signal */
    Condition_Lock( &thread_sNum_cond );
    IPERF_DEBUGF( CONDITION_DEBUG | IPERF_DBG_TRACE, ( "Decrementing thread count from %d to %d.\r\n", thread_sNum, thread_sNum - 1 ) );
    thread_sNum--;
    IPERF_DEBUGF( CONDITION_DEBUG | IPERF_DBG_TRACE,( "Signaling thread number condition.\r\n" ) );
    Condition_Signal( &thread_sNum_cond );
    Condition_Unlock( &thread_sNum_cond );

    /* Check if we need to start up a thread after executing this one */
    if ( thread->runNext != NULL )
    {
        thread_start( thread->runNext );
    }

    /* Destroy this thread object */
    Settings_Destroy( thread );

    return thread;
} /* thread_run_wrapper */


void thread_joinall( void )
{
    Condition_Lock( &thread_sNum_cond );

    while ( thread_sNum > 0 )
    {
        Condition_Wait( &thread_sNum_cond );
    }

    Condition_Unlock( &thread_sNum_cond );
} /* thread_joinall */


int thread_equalid( nthread_t inLeft, nthread_t inRight )
{
    return ( memcmp( &inLeft, &inRight, sizeof( inLeft ) ) == 0 );
}


nthread_t thread_zeroid( void )
{
    nthread_t a;
    memset( &a, 0, sizeof( a ) );
    return a;
}


void thread_setignore( void )
{
    Condition_Lock( &thread_sNum_cond );
    thread_sNum--;
    Condition_Signal( &thread_sNum_cond );
    Condition_Unlock( &thread_sNum_cond );
}


void thread_unsetignore( void )
{
    Condition_Lock( &thread_sNum_cond );
    thread_sNum++;
    Condition_Signal( &thread_sNum_cond );
    Condition_Unlock( &thread_sNum_cond );
}


void thread_register_nonterm( void )
{
    Condition_Lock( &thread_sNum_cond );
    nonterminating_num++;
    Condition_Unlock( &thread_sNum_cond );
}


void thread_unregister_nonterm( void )
{
    Condition_Lock( &thread_sNum_cond );
    if ( nonterminating_num == 0 )
    {
        /*
         * Nonterminating has been released with release_nonterm. Add back to
         * the threads to wait on.
         */
        thread_sNum++;
    }
    else
    {
        nonterminating_num--;
    }
    Condition_Unlock( &thread_sNum_cond );
}


int thread_release_nonterm( int interrupt )
{
    Condition_Lock( &thread_sNum_cond );
    thread_sNum -= nonterminating_num;
    if ( thread_sNum > 1  &&  nonterminating_num > 0  &&  interrupt != 0 )
    {
        fprintf( stderr, "%s", wait_server_threads );
    }
    nonterminating_num = 0;
    Condition_Signal( &thread_sNum_cond );
    Condition_Unlock( &thread_sNum_cond );

    return thread_sNum;
}


int thread_numuserthreads( void )
{
    return thread_sNum;
}


void thread_rest ( void )
{

}

#endif // WICED
