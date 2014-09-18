/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
/*
 */

#ifndef THREADX_NETX_H_
#define THREADX_NETX_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************
 * Fixes before includes to ensure NetX BSD sockets work
 *******************************************************/
/* NetX should really do this itself... *sigh* */
/*define_style_exception_start*/
#define __suseconds_t_defined /* To prevent conflicting declaration of 'typedef ULONG suseconds_t' in "netx_bsd_layer/nx_bsd.h" */
/*define_style_exception_end*/

/* Enable extended BSD socket support */
#define NX_EXTENDED_BSD_SOCKET_SUPPORT

/* socklen_t */
#ifndef socklen_t
/*define_style_exception_start*/
#define socklen_t INT
/*define_style_exception_end*/
#endif /* socklen_t */

/*define_style_exception_start*/
#define h_addr h_addr_list[0] /* for backward compatibility */
/*define_style_exception_end*/


#include <tx_api.h> /* so that "tx_api.h" is included, which defines ThreadX BSD data structures */
#include <nx_api.h>
#include <netx_bsd_layer/nx_bsd.h>


/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/** <inet.h> */
/* 255.255.255.255 */
#define INADDR_NONE         IP_ADDRESS( 255, 255, 255, 255 )
/* 127.0.0.1 */
#define INADDR_LOOPBACK     IP_ADDRESS( 127,   0,   0,   1 )
/* 0.0.0.0 */
#define INADDR_ANY          IP_ADDRESS(   0,   0,   0,   0 )
/** 255.255.255.255 */
#define INADDR_BROADCAST    IP_ADDRESS( 255, 255, 255, 255 )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
/** <netdb.h> */
struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
};

#include <netx_applications/dns/nx_dns.h>

/******************************************************
 *                 Global Variables
 ******************************************************/
extern NX_DNS *dns_ptr;
/**
 * Returns an entry containing addresses of address family AF_INET
 * for the host with name name.
 * Due to dns_gethostbyname limitations, only one address is returned.
 *
 * @param name the hostname to resolve
 * @return an entry containing addresses of address family AF_INET
 *         for the host with name name
 */

/******************************************************
 *               Function Declarations
 ******************************************************/
struct hostent*
gethostbyname(const char *name)
{
    ULONG addr;

    /* buffer variables for lwip_gethostbyname() */
    static struct hostent s_hostent;
    static char *s_aliases;
    static ULONG s_hostent_addr;
    static ULONG *s_phostent_addr[2];

    if (TX_SUCCESS != nx_dns_host_by_name_get(dns_ptr, (UCHAR*) name, &addr, TX_WAIT_FOREVER))
    {
        return NULL;
    }

    /* fill hostent */
    s_hostent_addr = addr;
    s_phostent_addr[0] = &s_hostent_addr;
    s_phostent_addr[1] = NULL;
    s_hostent.h_name = (char*) name;
    s_hostent.h_aliases = &s_aliases;
    s_hostent.h_addrtype = AF_INET;
    s_hostent.h_length = sizeof(ULONG);
    s_hostent.h_addr_list = (char**) &s_phostent_addr;

    return &s_hostent;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THREADX_NETX_H_ */
