/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *  Exported API for WicedFS filesystem creation code
 */

#ifndef INCLUDED_WICEDFS_CREATE_H_
#define INCLUDED_WICEDFS_CREATE_H_

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

/**
 * Create a WicedFS Filesystem Image
 *
 * Reads a directory from the PC filesystem, then
 * creates an image file containing a WicedFS Filesystem
 * which consists of all files and directories from the
 * PC filesystem directory.
 *
 * @param[in] out_file : Filename of the WicedFS image to be created
 * @param[in] dir_name : Pathname of the directory to read from the PC filesystem
 *
 * @return 0 = success
 */
int create_wicedfs( const char* out_file, const char* dir_name );

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_WICEDFS_CREATE_H_ */
