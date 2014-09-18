/*
 * BCM common config options
 *
 *
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: bcm_cfg.h 241182 2011-02-17 21:50:03Z gmo $
 */


#if defined(__NetBSD__) || defined(__FreeBSD__)
#if defined(_KERNEL)
#include <opt_bcm.h>
#endif /* defined(_KERNEL) */
#endif /* defined(__NetBSD__) || defined(__FreeBSD__) */
