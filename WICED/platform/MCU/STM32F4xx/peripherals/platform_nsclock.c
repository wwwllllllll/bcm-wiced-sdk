/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */
#include "stdint.h"
#include "platform_cmsis.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define CYCLE_COUNTING_INIT() \
    do \
    { \
        /* enable DWT hardware and cycle counting */ \
        CoreDebug->DEMCR = CoreDebug->DEMCR | CoreDebug_DEMCR_TRCENA_Msk; \
        /* reset a counter */ \
        DWT->CYCCNT = 0;  \
        /* enable the counter */ \
        DWT->CTRL = (DWT->CTRL | DWT_CTRL_CYCCNTENA_Msk) ; \
    } \
    while(0)

#define CYCLE_COUNTING_RESULT() \
        ({ \
                            uint32_t count; \
                            count = DWT->CYCCNT; \
                            count; \
        });

#define absolute_value(a) ( a < 0 ) ? -(a) : (a)

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

uint32_t nsclock_nsec =0;
uint32_t nsclock_sec =0;
uint32_t prev_cycles = 0;


/******************************************************
 *               Function Definitions
 ******************************************************/

uint64_t platform_get_nanosecond_clock_value(void)
{
    uint64_t nanos;
    uint32_t cycles;
    uint32_t diff;
    cycles = CYCLE_COUNTING_RESULT();

    /* add values to the ns part of the time which can be divided by 3 */
    /* every time such value is added we will increment our clock by 25ns = 1/40 000000( cycle counter is running on the CPU frequency ),  */
    /* values will be 3, 6, 9, etc */
    diff = absolute_value(cycles - prev_cycles);
    nsclock_nsec += ( diff / 3 ) * 25;

    /* when ns counter rolls over, add one second */
    if( nsclock_nsec >= 1000000000 )
    {
        nsclock_sec++;
        nsclock_nsec = nsclock_nsec - 1000000000;
    }
    prev_cycles = cycles - (diff % 3);

    nanos = nsclock_sec;
    nanos *= 1000000000;
    nanos += nsclock_nsec;
    return nanos;
}


void platform_deinit_nanosecond_clock(void)
{
    nsclock_nsec = 0;
    nsclock_sec = 0;
}

void platform_reset_nanosecond_clock(void)
{
    nsclock_nsec = 0;
    nsclock_sec = 0;
}


void platform_init_nanosecond_clock(void)
{
    CYCLE_COUNTING_INIT();
    nsclock_nsec = 0;
    nsclock_sec = 0;
}
