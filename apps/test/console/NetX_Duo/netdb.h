/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef NETDB_H_
#define NETDB_H_

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
struct hostent {
    CHAR  *h_name;      /* Official name of the host. */
    CHAR **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    INT    h_addrtype;  /* Address type. */
    INT    h_length;    /* The length, in bytes, of the address. */
    CHAR **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
};

/*define_style_exception_start*/
#define h_addr h_addr_list[0] /* for backward compatibility */
/*define_style_exception_end*/

#include "netx_applications/dns/nxd_dns.h"

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
struct hostent* netx_gethostbyname(NX_DNS *dns_ptr, const CHAR *name);
struct hostent* gethostbyname(const CHAR *name);

#ifdef __cplusplus
}
#endif

#endif /* NETDB_H_ */
