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
 *  Private structures and definitions for the WicedFS Read-Only file system.
 */

#ifndef INCLUDED_WICEDFS_INTERNAL_H_
#define INCLUDED_WICEDFS_INTERNAL_H_

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INSIDE_WICEDFS_C_
#error wicedfs_internal.h should not be used externally
#endif /* ifndef INSIDE_WICEDFS_C_ */

/******************************************************
 *                     Macros
 ******************************************************/
/* Suppress unused parameter warning */
#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) /*@-noeffect@*/ ( (void)(x) ) /*@+noeffect@*/
#endif

/******************************************************
 *                    Constants
 ******************************************************/
/* File table Entry Type definitions
 * To be used for the type_flags_permissions field
 */

#define WICEDFS_TYPE_DIR       (0)
#define WICEDFS_TYPE_FILE      (1)
#define WICEDFS_TYPE_LINK      (2)
#define WICEDFS_TYPE_MASK      (0xf)

/* Magic Number for the filesystem header (wicedfs_filesystem_header_t) */
#define WICEDFS_MAGIC     ((uint32_t)(((uint8_t)'B'<<0)|((uint8_t)'R'<<8)|((uint8_t)'C'<<16)|((uint8_t)'M'<<24)))

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/
/*****************************************************************
 * The following structures define the filesystem data structures
 * on the hardware device.
 *
 * They are consequently byte packed.
 */

#pragma pack(1)

/* Overall File-system Header structure */
typedef struct
{
    uint32_t        magic_number;        /* Should be  'B', 'R', 'C', 'M' for valid filesystem - see WICEDFS_MAGIC below */
    uint32_t        version;             /* Version 1 only so far */
    wicedfs_usize_t root_dir_offset;     /* Offset to wicedfs_dir_header_t for root directory from start of this  wicedfs_filesystem_header_t */
} wicedfs_filesystem_header_t;


/* Directory Header structure*/
typedef struct
{
    wicedfs_ssize_t file_table_offset;   /* Offset from the start of this directory header to the File table */
    wicedfs_usize_t num_files;           /* Number of entries in the file table */
    uint32_t        file_header_size;    /* The size in bytes of each entry in the file table */
    uint32_t        filename_size;       /* The size in bytes of the filename part of each file table entry */
} wicedfs_dir_header_t;

/* File Table entry structure */
typedef struct
{
    uint32_t        type_flags_permissions;  /* A bit field containing the Type, Flags and Permissions for the entry - See definitions below */
    wicedfs_ssize_t offset;                  /* Offset from the start of this file table entry structure to the content data for this entry */
    wicedfs_usize_t size;                    /* Size in bytes of the content data for this entry */
    /* filename follows this - variable size as per wicedfs_dir_header_t.filename_size */
} wicedfs_file_header_t;

#pragma pack()

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_`_INTERNAL_H_ */
