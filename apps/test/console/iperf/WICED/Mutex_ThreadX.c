/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#if defined(WICED) && defined(HAVE_THREADS)

#include "headers.h"
#include "Mutex.h"

xMutexHandle xMutexCreate( void )
{

}


void xMutexDestroy( xMutexHandle pxMutex )
{

}


signed portBASE_TYPE xMutexTake( xMutexHandle pxMutex, portTickType xTicksToWait )
{

}


signed portBASE_TYPE xMutexGive( xMutexHandle pxMutex, portBASE_TYPE Release )
{

}


signed portBASE_TYPE xDoIOwnTheMutex( xMutexHandle pxMutex )
{

}

#endif
