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

#include "tx_api.h" /* so that "tx_api.h" is included, which defines ThreadX BSD data structures */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

#define _POSIX_SOURCE /* NetX tries to be POSIX-compliant */

/* NetX should really do this itself... *sigh* */
#define _TIMEVAL_DEFINED /* prevents struct timeval being defined in <sys/time.h> because "netx_bsd_layer/nx_bsd.h" will define this */

/* socklen_t */
#ifndef socklen_t
/*define_style_exception_start*/
#define socklen_t INT
/*define_style_exception_end*/
#endif /* socklen_t */

/*define_style_exception_start*/
#define h_addr h_addr_list[0] /* for backward compatibility */
/*define_style_exception_end*/


#include <netx_applications/dns/nxd_dns.h>

/******************************************************
 *                    Constants
 ******************************************************/
/** DNS server IP address */
#ifndef DNS_SERVER_IP
#define DNS_SERVER_IP           IP_ADDRESS(208,67,222,222) /* resolver1.opendns.com */
#endif

/** Errors used by the DNS API functions, h_errno can be one of them */
#define EAI_NONAME      200
#define EAI_SERVICE     201
#define EAI_FAIL        202
#define EAI_MEMORY      203

#define HOST_NOT_FOUND  210
#define NO_DATA         211
#define NO_RECOVERY     212
#define TRY_AGAIN       213

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
/* To mimic <netdb.h> */
struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
};


/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THREADX_NETX_H_ */
