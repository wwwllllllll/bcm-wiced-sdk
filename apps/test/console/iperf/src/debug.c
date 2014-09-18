/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "headers.h"
#include "debug.h"
#include "Mutex.h"

#if IPERF_DEBUG
static Mutex debugprint_mutex;
static int   debugprint_mutex_init = 0;

void debug_get_mutex() {
    if (debugprint_mutex_init != 0)
        Mutex_Lock(&debugprint_mutex);
}

void debug_release_mutex() {
    if (debugprint_mutex_init != 0)
        Mutex_Unlock(&debugprint_mutex);
}
#endif /* IPERF_DEBUG */

void debug_init() {
#if IPERF_DEBUG
    Mutex_Initialize(&debugprint_mutex);
    debugprint_mutex_init = 1;
#endif /* IPERF_DEBUG */
}

void debug_destroy() {
#if IPERF_DEBUG
    Mutex_Destroy(&debugprint_mutex);
    debugprint_mutex_init = 0;
#endif /* IPERF_DEBUG */
}
