/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INCLUDED_BT_SMARTBRIDGE_CONSOLE_H
#define INCLUDED_BT_SMARTBRIDGE_CONSOLE_H

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

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

int start_smartbridge               ( int argc, char *argv[] );
int stop_smartbridge                ( int argc, char *argv[] );
int auto_test_smartbridge           ( int argc, char *argv[] );
int connect                         ( int argc, char *argv[] );
int disconnect                      ( int argc, char *argv[] );
int scan_connect_direct_adv         ( int argc, char *argv[] );
int scan                            ( int argc, char *argv[] );
int status                          ( int argc, char *argv[] );
int clear_bond_info_dct             ( int argc, char *argv[] );
int enable_attribute_cache          ( int argc, char *argv[] );
int disable_attribute_cache         ( int argc, char *argv[] );
int print_attribute_cache           ( int argc, char *argv[] );
int write_attribute_cache           ( int argc, char *argv[] );
int enable_notification             ( int argc, char *argv[] );
int disable_notification            ( int argc, char *argv[] );
int add_whitelist                   ( int argc, char *argv[] );
int remove_whitelist                ( int argc, char *argv[] );
int clear_whitelist                 ( int argc, char *argv[] );
int get_whitelist_size              ( int argc, char *argv[] );
int discover_all_services           ( int argc, char *argv[] );
int discover_services_by_uuid       ( int argc, char *argv[] );
int find_included_services          ( int argc, char *argv[] );
int discover_all_chars_in_a_service ( int argc, char *argv[] );
int discover_chars_by_uuid          ( int argc, char *argv[] );
int discover_all_char_descriptors   ( int argc, char *argv[] );
int read_char_descriptor            ( int argc, char *argv[] );
int read_long_char_descriptor       ( int argc, char *argv[] );
int write_char_descriptor           ( int argc, char *argv[] );
int write_long_char_descriptor      ( int argc, char *argv[] );
int read_char_value                 ( int argc, char *argv[] );
int read_long_char_value            ( int argc, char *argv[] );
int read_char_values_by_uuid        ( int argc, char *argv[] );
int write_char_value                ( int argc, char *argv[] );
int write_long_char_value           ( int argc, char *argv[] );
int dump_packet                     ( int argc, char *argv[] );
int stack_trace                     ( int argc, char *argv[] );
int set_tx_power                    ( int argc, char *argv[] );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INCLUDED_CONSOLE_H */
