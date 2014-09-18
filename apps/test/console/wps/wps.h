/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#ifndef INCLUDED_WPS_H_
#define INCLUDED_WPS_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifdef CONSOLE_ENABLE_WPS

extern int join_wps( int argc, char* argv[] );
extern int start_registrar( int argc, char* argv[] );
extern int force_alignment( int argc, char* argv[] );
extern int stop_registrar( int argc, char* argv[] );

#define WPS_COMMANDS \
    { (char*) "force_alignment",  force_alignment, 0, DELIMIT, NULL, (char*) "", (char*) "Force aligned memory accesses"}, \
    { (char*) "join_wps",  join_wps, 1, DELIMIT, NULL, (char*) "<pbc|pin> [pin] [<ip> <netmask> <gateway>]", (char*) "Join an AP using WPS"}, \
    { (char*) "start_registrar",  start_registrar, 1, DELIMIT, NULL, (char*) "<pbc|pin> [pin]", (char*) "Start the WPS Registrar"}, \
    { (char*) "stop_registrar",  stop_registrar, 0, DELIMIT, NULL, (char*) "", (char*) "Stop the WPS Registrar"}, \

#else /* ifdef CONSOLE_ENABLE_WPS */
#define WPS_COMMANDS
#endif /* ifdef CONSOLE_ENABLE_WPS */

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_WPS_H_ */
