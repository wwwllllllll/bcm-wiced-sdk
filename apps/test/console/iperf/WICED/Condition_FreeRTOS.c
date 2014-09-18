/**
 * The implementation of Condition Variables using semaphores.
 *
 * Adapted from http://research.microsoft.com/pubs/64242/ImplementingCVs.pdf
 *
 */

#if defined(WICED) && defined(HAVE_THREADS)

#include "headers.h"
#include "Condition.h"

#include <FreeRTOS.h>
#include <semphr.r>
#include <portmacro.h>

void Lock_Initialize( Lock *m )
{
    /*
     * sm = new Semaphore( );
     * sm.count = 1;
     * sm.limit = 1;
     */
    m->sm = xSemaphoreCreateCounting( 1, 1 );
}


void Lock_Destroy( Lock *m )
{
    if ( m->sm != NULL)
    {
        vQueueDelete( m->sm );
        m->sm = NULL;
    }
}


void Lock_Acquire( Lock *m )
{
    /*
     * sm.P( );
     */
    xSemaphoreTake( m->sm, portMAX_DELAY );
}


void Lock_Release( Lock *m )
{
    /*
     * sm.V( );
     */
    xSemaphoreGive( m->sm );
}


void Condition_Initialize( Condition *c )
{
    if ( c->m == NULL )
    {
        /* this.m = m; */
        Lock *m = (Lock*) malloc ( sizeof( Lock ) );
        FAIL( m == NULL, ( "Unable to allocate memory for Lock m.\r\n" ), NULL );
        IPERF_MEMALLOC_DEBUG_MSG( m, sizeof( Lock ) );
        Lock_Initialize( m );
        c->m = m;

        /* int waiters = 0; */
        c->waiters = 0;

        /*
         * s = new Semaphore( );
         * s.count = 0;
         * s.limit = 999999;
         */
        c->s = xSemaphoreCreateCounting( portMAX_DELAY, 0 );

        /*
         * x = new Semaphore( );
         * x.count = 1;
         * x.limit = 1;
         */
        c->x = xSemaphoreCreateCounting( 1, 1 );

        /*
         * h = new Semaphore( );
         * h.count = 0;
         * h.limit = 999999;
         */
        c->h = xSemaphoreCreateCounting( portMAX_DELAY, 0 );
    }
    else
    {
        fprintf( stderr, "Condition already has a lock. Unable to initialize a new lock.\r\n" );
    }
}


void Condition_Destroy( Condition *c )
{
    Lock_Destroy( c->m );
    c->m = NULL;

    host_rtos_deinit_semaphore( c->s );
    c->s = NULL;

    host_rtos_deinit_semaphore( c->x );
    c->x = NULL;

    host_rtos_deinit_semaphore( c->h );
    c->h = NULL;
}


void Condition_Wait( Condition *c )
{
     /* x.P( ); */
    xSemaphoreTake( c->x, portMAX_DELAY );

    /* waiters++; */
    {
        c->waiters++;
    }

    /* x.V( ); */
    xSemaphoreGive( c->x );

    /* m.Release() */
    Lock_Release( c->m );

    /* s.P( ); */
    xSemaphoreTake( c->s, portMAX_DELAY );

    /* h.V( ); */
    xSemaphoreGive( c->h );

    /* m.Acquire( ); */
     Lock_Acquire( c->m );
}


void Condition_Signal( Condition *c )
{
    /* x.P( ); */
    xSemaphoreTake( c->x, portMAX_DELAY );

    {
        /* if ( waiters > 0 ) */
        if ( c->waiters > 0 )
        {
            /* waiters--; */
            c->waiters--;

            /* s.V( ); */
            xSemaphoreGive( c->s );

            /* h.P( ); */
            xSemaphoreTake( c->s, portMAX_DELAY );
        }
    }

    /* x.V( ); */
    xSemaphoreGive( c->x );
}


void Condition_Broadcast( Condition *c )
{
    /* x.P( ); */
    xSemaphoreTake( c->x, portMAX_DELAY );

    {
        /* for ( int i = 0; i < waiters; i++ ) */
        unsigned int i;
        for ( i = 0; c->waiters >= 0  &&  i < (unsigned) c->waiters; i++ )
        {
            /* s.V( ); */
            xSemaphoreGive( c->s );
        }

        /* while ( waiters > 0 ) */
        while ( c->waiters > 0 )
        {
            /* waiters--; */
            c->waiters--;

            /* h.P( ); */
            xSemaphoreTake( c->h, portMAX_DELAY );
        }
    }

    /* x.V( ); */
    xSemaphoreGive( c->x );
}


void Condition_Lock( Condition *c )
{
    Lock_Acquire( c->m );
}


void Condition_Unlock( Condition *c )
{
    Lock_Release( c-> m );
}

#endif
