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

/******************************************************
 *                     Macros
 ******************************************************/
#ifdef CONSOLE_ENABLE_WPS

#define WPS_COMMANDS \
    { (char*) "force_alignment",  force_alignment, 0, DELIMIT, NULL, (char*) "", (char*) "Force aligned memory accesses"}, \
    { (char*) "join_wps",  join_wps, 1, DELIMIT, NULL, (char*) "<pbc|pin> [pin] [<ip> <netmask> <gateway>]", (char*) "Join an AP using WPS"}, \
    { (char*) "start_registrar",  start_registrar, 1, DELIMIT, NULL, (char*) "<pbc|pin> [pin]", (char*) "Start the WPS Registrar"}, \
    { (char*) "stop_registrar",  stop_registrar, 0, DELIMIT, NULL, (char*) "", (char*) "Stop the WPS Registrar"}, \

#else /* ifdef CONSOLE_ENABLE_WPS */
#define WPS_COMMANDS
#endif /* ifdef CONSOLE_ENABLE_WPS */

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
int join_wps( int argc, char* argv[] );
int start_registrar( int argc, char* argv[] );
int force_alignment( int argc, char* argv[] );
int stop_registrar( int argc, char* argv[] );
wiced_result_t enable_ap_registrar_events( void );
void disable_ap_registrar_events( void );

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_WPS_H_ */
