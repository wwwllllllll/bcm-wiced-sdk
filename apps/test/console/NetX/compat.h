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

#include "tx_api.h"
#include "nx_api.h"

#include "inet.h"
#include "netdb.h"
//#include "netx_bsd_layer/nx_bsd.h"
#include "netx_applications/dns/nx_dns.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THREADXNETX_COMPAT_H_ */
