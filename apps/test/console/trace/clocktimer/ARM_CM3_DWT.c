/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "ARM_CM3_DWT.h"
#include "clocktimer.h"


/**
 * Get the clock time from the DWT cycle counter.
 */
CLOCKTIME_T get_clocktime( void )
{
    return *((unsigned long *) DWT_CYCCNT);
}


/**
 * Start the DWT cycle counter.
 */
void start_clocktimer( void )
{
    *((unsigned long *) DWT_CTRL) |= 1;
}


/**
 * Reset the DWT cycle counter.
 */
void reset_clocktimer( void )
{
    *((unsigned long *) DWT_CYCCNT) = 0;
}


/**
 * Stop the DWT cycle counter.
 */
void end_clocktimer( void )
{
    *((unsigned long *) DWT_CTRL) &= 0;
}
