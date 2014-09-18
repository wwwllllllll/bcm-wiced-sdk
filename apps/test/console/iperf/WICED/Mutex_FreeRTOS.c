/*==============================================================================
 * Copyright (c) 2007-2009, Isaac Marino Bavaresco
 * All rights reserved
 * isaacbavaresco@yahoo.com.br
 *
 * Retrieved from http://www.piclist.com/techref/member/IMB-yahoo-J86/mutex.c.htm
 *
 * Requires #define configUSE_MUTEX 1 to enable mutexes in FreeRTOS.
 *==============================================================================*/

#if defined(WICED) && defined(HAVE_THREADS)

#include "headers.h"
#include "Mutex.h"

#include <FreeRTOS.h>
#include <list.h>
#include <task.h>
#include <portmacro.h>

xMutexHandle xMutexCreate( void )
{
    xMUTEX *pxNewMutex;

    pxNewMutex = (xMUTEX*) pvPortMalloc( sizeof( xMUTEX ) );
    if ( pxNewMutex != NULL )
    {
        pxNewMutex->pxOwner = NULL;
        pxNewMutex->uxCount = 0;
        vListInitialise( &( pxNewMutex->xTasksWaitingToTake ) );
    }

    return pxNewMutex;
}


void xMutexDestroy( xMutexHandle pxMutex )
{
    (void) pxMutex;
}


signed portBASE_TYPE xMutexTake( xMutexHandle pxMutex, portTickType xTicksToWait )
{
    portENTER_CRITICAL( );
    if ( pxMutex->pxOwner == xTaskGetCurrentTaskHandle( ) )
    {
        pxMutex->uxCount++;
        portEXIT_CRITICAL( );
        return pdTRUE;
    }

    if ( ( xTicksToWait > ( portTickType ) 0 ) && ( pxMutex->pxOwner != NULL ) )
    {
        vTaskPlaceOnEventList( &( pxMutex->xTasksWaitingToTake ), xTicksToWait );
        taskYIELD( );

        if ( pxMutex->pxOwner == xTaskGetCurrentTaskHandle( ) )
        {
            pxMutex->uxCount = 1;
            portEXIT_CRITICAL( );
            return pdTRUE;
        }
        else
        {
            portEXIT_CRITICAL( );
            return pdFALSE;
        }
    }

    if ( pxMutex->pxOwner == NULL )
    {
        pxMutex->pxOwner = xTaskGetCurrentTaskHandle( );
        pxMutex->uxCount = 1;
        portEXIT_CRITICAL( );
        return pdTRUE;
    }

    portEXIT_CRITICAL( );
    return pdFALSE;
}


signed portBASE_TYPE xMutexGive( xMutexHandle pxMutex, portBASE_TYPE Release )
{
    portENTER_CRITICAL( );
    if ( pxMutex->pxOwner != xTaskGetCurrentTaskHandle( ) )
    {
        portEXIT_CRITICAL( );
        return pdFALSE;
    }

    if ( Release )
        pxMutex->uxCount = 0;
    else
    {
        if ( --pxMutex->uxCount != 0 )
        {
            portEXIT_CRITICAL( );
            return pdFALSE;
        }
    }

    if( !listLIST_IS_EMPTY( &pxMutex->xTasksWaitingToTake ) )
    {
        pxMutex->pxOwner = (xTaskHandle) listGET_OWNER_OF_HEAD_ENTRY( ( &pxMutex->xTasksWaitingToTake ) );
        pxMutex->uxCount = 1;

        if( xTaskRemoveFromEventList( &pxMutex->xTasksWaitingToTake ) == pdTRUE )
            taskYIELD( );
    }
    else
    {
        pxMutex->pxOwner = NULL;
    }

    portEXIT_CRITICAL( );
    return pdTRUE;
}


signed portBASE_TYPE xDoIOwnTheMutex( xMutexHandle pxMutex )
{
    portBASE_TYPE c;

    portENTER_CRITICAL( );
    c = pxMutex->pxOwner == xTaskGetCurrentTaskHandle( );
    portEXIT_CRITICAL( );

    return c;
}

#endif
