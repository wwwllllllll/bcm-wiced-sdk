/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#ifndef INCLUDED_P2P_H_
#define INCLUDED_P2P_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#ifdef CONSOLE_ENABLE_P2P

#define P2P_COMMANDS \
    { (char*) "p2p_invite",   p2p_invite, 1,  DELIMIT, NULL, (char*) "<device id>", (char*) "Invite P2P peer"}, \
    { (char*) "p2p_peers",   p2p_peer_list, 0,  DELIMIT, NULL, (char*) "", (char*) "Print P2P peer list"}, \
    { (char*) "start_p2p",  start_p2p, 0, DELIMIT, NULL, (char*) "[Group Owner intent (0..15)]", (char*) "Run P2P"}, \
    { (char*) "stop_p2p",   stop_p2p, 0,  DELIMIT, NULL, (char*) "", (char*) "Stop P2P"}, \


#else /* ifdef CONSOLE_ENABLE_P2P */
#define P2P_COMMANDS
#endif /* ifdef CONSOLE_ENABLE_P2P */

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
int start_p2p    ( int argc, char* argv[] );
int stop_p2p     ( int argc, char* argv[] );
int p2p_peer_list( int argc, char* argv[] );
int p2p_invite   ( int argc, char* argv[] );

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_P2P_H_ */
