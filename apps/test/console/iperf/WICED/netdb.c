/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "compat.h"
#include "netdb.h"
#include "stdio.h"

#if defined(NETWORK_NetX) || defined(Network_NetX_Duo)
#include "tx_api.h"
#include "nx_api.h"
#endif
//#include "netx_applications/dns/nx_dns.h"

/**
 * Returns an entry containing addresses of address family AF_INET
 * for the host with name name.
 * Due to dns_gethostbyname limitations, only one address is returned.
 *
 * @param name the hostname to resolve
 * @return an entry containing addresses of address family AF_INET
 *         for the host with name name
 */
struct hostent* gethostbyname(const char *name)
{
    unsigned long addr;
    static struct hostent s_hostent;
    static char *s_aliases;
    static unsigned long s_hostent_addr;
    static unsigned long *s_phostent_addr[2];

    int temp[4];
    sscanf( name, "%d.%d.%d.%d", &temp[0], &temp[1], &temp[2], &temp[3] );
    addr = temp[3] << 24 | temp[3] << 16 | temp[1] << 8 | temp[0];
//    if (TX_SUCCESS != nx_dns_host_by_name_get(NULL, (UCHAR*) name, &addr, 250))
//    {
//        return NULL;
//    }

    /* fill hostent */
    s_hostent_addr          = addr;
    s_phostent_addr[0]      = &s_hostent_addr;
    s_phostent_addr[1]      = NULL;
    s_hostent.h_name        = (char*) name;
    s_hostent.h_aliases     = &s_aliases;
    s_hostent.h_addrtype    = AF_INET;
    s_hostent.h_length      = sizeof(ulong);
    s_hostent.h_addr_list   = (char**) &s_phostent_addr;

    return &s_hostent;
}
