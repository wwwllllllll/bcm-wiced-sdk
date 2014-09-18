/*
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: epivers.h.in 277737 2011-08-16 17:54:59Z hharte $
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#define	EPI_MAJOR_VERSION	5

#define	EPI_MINOR_VERSION	90

#define	EPI_RC_NUMBER		230

#define	EPI_INCREMENTAL_NUMBER	255

#define	EPI_BUILD_NUMBER	0

#define	EPI_VERSION		5, 90, 230, 255

#define	EPI_VERSION_NUM		0x055ae6ff

#define EPI_VERSION_DEV		5.90.230

/* Driver Version String, ASCII, 32 chars max */
#ifdef BCMINTERNAL
#define	EPI_VERSION_STR		"5.90.230 (DEV) (BCM INTERNAL)"
#else
#ifdef WLTEST
#define	EPI_VERSION_STR		"5.90.230 (DEV-MFGTEST)"
#else
#define	EPI_VERSION_STR		"5.90.230 (DEV)"
#endif
#endif /* BCMINTERNAL */

#endif /* _epivers_h_ */
