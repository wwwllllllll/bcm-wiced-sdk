/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INET_H_
#define INET_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NETWORK_LwIP
#define INADDR_NONE         IP_ADDRESS( 255, 255, 255, 255 )    /* 255.255.255.255 */
#define INADDR_LOOPBACK     IP_ADDRESS( 127,   0,   0,   1 )    /* 127.0.0.1 */
#define INADDR_ANY          IP_ADDRESS(   0,   0,   0,   0 )    /* 0.0.0.0 */
#define INADDR_BROADCAST    IP_ADDRESS( 255, 255, 255, 255 )    /* 255.255.255.255 */
#endif

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* INET_H_ */
