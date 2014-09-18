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
#include "tx_api.h"
#include "nx_api.h"
#include "netdb.h"
#include "netx_applications/dns/nxd_dns.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define AF_INET         2

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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * Returns an entry containing addresses of address family AF_INET
 * for the host with name name.
 * Due to dns_gethostbyname limitations, only one address is returned.
 *
 * @param name the hostname to resolve
 * @return an entry containing addresses of address family AF_INET
 *         for the host with name name
 */
struct hostent* gethostbyname(const CHAR *name)
{
    ULONG addr;
    static struct hostent s_hostent;
    static CHAR *s_aliases;
    static ULONG s_hostent_addr;
    static ULONG *s_phostent_addr[2];

    if (TX_SUCCESS != nx_dns_host_by_name_get(NULL, (UCHAR*) name, &addr, 250))
    {
        return NULL;
    }

    /* fill hostent */
    s_hostent_addr          = addr;
    s_phostent_addr[0]      = &s_hostent_addr;
    s_phostent_addr[1]      = NULL;
    s_hostent.h_name        = (CHAR*) name;
    s_hostent.h_aliases     = &s_aliases;
    s_hostent.h_addrtype    = AF_INET;
    s_hostent.h_length      = sizeof(ULONG);
    s_hostent.h_addr_list   = (CHAR**) &s_phostent_addr;

    return &s_hostent;
}
