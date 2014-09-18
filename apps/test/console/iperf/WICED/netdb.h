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

struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};

#if defined(NETWORK_NetX)
#include "nx_api.h"
#include "netx_applications/dns/nx_dns.h"
extern struct hostent* netx_gethostbyname(NX_DNS *dns_ptr, const CHAR *name);
#elif defined(NETWORK_NetX_Duo)
#include "nx_api.h"
#include "netx_applications/dns/nxd_dns.h"
extern struct hostent* netx_gethostbyname(NX_DNS *dns_ptr, const CHAR *name);
#endif

struct hostent* gethostbyname(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* NETDB_H_ */
