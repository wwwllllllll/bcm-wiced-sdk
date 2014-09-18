/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define s6_addr     _S6_un._S6_u8
#define s6_addr32   _S6_un._S6_u32

#define FD_SET(socket, fds)    (*fds = socket)
#define FD_ZERO(fds)           (*fds = -1)

/******************************************************
 *                    Constants
 ******************************************************/

#define SOCK_STREAM                         1                       /* TCP Socket */
#define SOCK_DGRAM                          2                       /* UDP Socket */
#define IPPROTO_TCP                         6                       /* TCP. */
#define IPPROTO_UDP                         17                      /* UDP. */
#define IPPROTO_IP                          0

#define     AF_INET                         2                       /* IPv4 socket (UDP, TCP, etc) */
#define     AF_INET6                        3                       /* IPv6 socket (UDP, TCP, etc) */
/* Protocol families, same as address families.  */
#define     PF_INET                         AF_INET
#define     PF_INET6                        AF_INET6

#define SOL_SOCKET      1   /* Define the option category (the only one). */

#define SO_MIN          1   /* Minimum Socket option ID */
#define SO_DEBUG        1   /* Debugging information is being recorded.*/
#define SO_REUSEADDR    2   /* Enable reuse of local addresses in the time wait state */
#define SO_TYPE         3   /* Socket type */
#define SO_ERROR        4   /* Socket error status */
#define SO_DONTROUTE    5   /* Bypass normal routing */
#define SO_BROADCAST    6   /* Transmission of broadcast messages is supported.*/
#define SO_SNDBUF       7   /* Enable setting trasnmit buffer size */
#define SO_RCVBUF       8   /* Enable setting receive buffer size */
#define SO_KEEPALIVE    9   /* Connections are kept alive with periodic messages */
#define SO_OOBINLINE    10  /* Out-of-band data is transmitted in line */
#define SO_NO_CHECK     11  /* Disable UDP checksum */
#define SO_PRIORITY     12  /* Set the protocol-defined priority for all packets to be sent on this socket */
#define SO_LINGER       13  /* Socket lingers on close pending remaining send/receive packets. */
#define SO_BSDCOMPAT    14  /* Enable BSD bug-to-bug compatibility */
#define SO_REUSEPORT    15  /* Rebind a port already in use */

/*
 * User-settable options (used with setsockopt).
 */
#define TCP_NODELAY 0x01    /* don't delay send to coalesce packets     */
#define TCP_MAXSEG  0x02    /* set maximum segment size                 */
#define TCP_NOPUSH  0x04    /* don't push last block of write           */
#define TCP_NOOPT   0x08    /* don't use TCP options                    */

/*
 * Options and types for UDP multicast traffic handling
 */
#define IP_ADD_MEMBERSHIP  3
#define IP_DROP_MEMBERSHIP 4
#define IP_MULTICAST_TTL   5
#define IP_MULTICAST_IF    6
#define IP_MULTICAST_LOOP  7

/*
 * Options for level IPPROTO_IP
 */
#define IP_TOS             1

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef  int        INT;
typedef  uint16_t   USHORT;
typedef  uint8_t    UCHAR;
typedef  uint32_t   ULONG;
typedef  char       CHAR;

#define VOID        void
#define in_addr_t   ULONG

typedef  INT        fd_set;

/******************************************************
 *                    Structures
 ******************************************************/

struct sockaddr
{
    USHORT          sa_family;              /* Address family (e.g. , AF_INET).                                                             */
    UCHAR           sa_data[14];            /* Protocol- specific address information.                                                      */
};

struct in6_addr
{
    union
    {
        UCHAR _S6_u8[16];
        ULONG _S6_u32[4];
    } _S6_un;
};

struct sockaddr_in6
{
    USHORT          sin6_family;                 /* AF_INET6 */
    USHORT          sin6_port;                   /* Transport layer port. # */
    ULONG           sin6_flowinfo;               /* IPv6 flow information. */
    struct in6_addr sin6_addr;                   /* IPv6 address. */
    ULONG           sin6_scope_id;               /* set of interafces for a scope. */

};

/* Internet address (a structure for historical reasons).  */

struct in_addr
{
    ULONG           s_addr;             /* Internet address (32 bits).                                                                  */
};

/* Socket address, Internet style. */

struct sockaddr_in
{
    USHORT              sin_family;         /* Internet Protocol (AF_INET).                                                                 */
    USHORT              sin_port;           /* Address port (16 bits).                                                                      */
    struct in_addr      sin_addr;           /* Internet address (32 bits).                                                                  */
    CHAR                sin_zero[8];        /* Not used.                                                                                    */
};

struct timeval
{
    ULONG     tv_sec;             /* Seconds      */
    ULONG     tv_usec;            /* Microseconds */
};

typedef struct ip_mreq {
    struct in_addr imr_multiaddr; /* IP multicast address of group */
    struct in_addr imr_interface; /* local IP address of interface */
} ip_mreq;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

void sockets_layer_init(void);

INT  getpeername(INT sockID, struct sockaddr *remoteAddress, INT *addressLength);
INT  getsockname(INT sockID, struct sockaddr *localAddress, INT *addressLength);
INT  recvfrom(INT sockID, CHAR *buffer, INT buffersize, INT flags,struct sockaddr *fromAddr, INT *fromAddrLen);
INT  recv(INT sockID, VOID *rcvBuffer, INT bufferLength, INT flags);
INT  sendto(INT sockID, CHAR *msg, INT msgLength, INT flags, struct sockaddr *destAddr, INT destAddrLen);
INT  send(INT sockID, const CHAR *msg, INT msgLength, INT flags);
INT  accept(INT sockID, struct sockaddr *ClientAddress, INT *addressLength);
INT  listen(INT sockID, INT backlog);
INT  bind(INT sockID, struct sockaddr *localAddress, INT addressLength);
INT  connect(INT sockID, struct sockaddr *remoteAddress, INT addressLength);
INT  select(INT nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
INT  soc_close( INT sockID);
INT  socket(INT protocolFamily, INT type, INT protocol);
INT  setsockopt(INT sockID, INT option_level, INT option_name, const void *option_value, INT option_length);
INT  getsockopt(INT sockID, INT option_level, INT option_name, void *option_value, INT *option_length);

#ifdef __cplusplus
} /*extern "C" */
#endif
