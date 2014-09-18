/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#if !defined(HAVE_USLEEP) && defined(WICED)

#include <RTOS/wwd_rtos_interface.h>
#include <sys/types.h>

int usleep(useconds_t usec) {
    return host_rtos_delay_milliseconds( usec / 1000 );
}

#endif /* !defined(HAVE_USLEEP) && defined(WICED) */
