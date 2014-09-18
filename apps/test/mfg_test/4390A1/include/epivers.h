/*
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: epivers.h.in,v 13.33 2010-09-08 22:08:53 csm Exp $
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	6

#define	EPI_MINOR_VERSION	30

#define	EPI_RC_NUMBER		0

#define	EPI_INCREMENTAL_NUMBER	0

#define EPI_BUILD_NUMBER	1

#define	EPI_VERSION		6, 30, 0, 0

#define	EPI_VERSION_NUM		0x061e0000

#define EPI_VERSION_DEV		6.30.0

/* Driver Version String, ASCII, 32 chars max */
#ifdef BCMINTERNAL
#define	EPI_VERSION_STR		"6.30 (TOB) (r BCMINT)"
#else
#ifdef WLTEST
#define	EPI_VERSION_STR		"6.30 (TOB) (r WLTEST)"
#else
#define	EPI_VERSION_STR		"6.30 (TOB) (r)"
#endif
#endif /* BCMINTERNAL */

#endif /* _epivers_h_ */
