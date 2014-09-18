/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/**
 * @file compat.h
 * @brief An attempt to make ThreadX/NetX more compatible with POSIX functions.
 */

#ifndef THREADXNETX_COMPAT_H_
#define THREADXNETX_COMPAT_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "inet.h"
#include "netdb.h"

//#define ULONG unsigned long
//#define UINT  unsigned int

#if defined(NETWORK_NetX)
#include "NetX/sockets.h"
#include "tx_api.h"
#include "nx_api.h"
#include "netx_applications/dns/nx_dns.h"
#elif defined(NETWORK_NetX_Duo)
#include "NetX_Duo/sockets.h"
#include "tx_api.h"
#include "nx_api.h"
#include "netx_applications/dns/nxd_dns.h"
#elif defined(NETWORK_LwIP)
#include "LwIP/sockets.h"
#endif

#ifdef __cplusplus
}
#endif

#ifndef socklen_t
#define socklen_t INT
#endif /* socklen_t */

#endif /* THREADXNETX_COMPAT_H_ */
