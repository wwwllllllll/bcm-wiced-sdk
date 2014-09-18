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
 *  Public API of the WicedFS Read-Only file system.
 */

#ifndef INCLUDED_WICEDFS_H_
#define INCLUDED_WICEDFS_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/
/* Define the appropriate sized types for storing
 * address, offsest and size data
 * To support files or filesystems larger than
 *  2GBytes, the global define WICEDFS_64BIT_LENGTHS
 *  should be added.
 */
#if defined( WICEDFS_64BIT_LENGTHS )
typedef uint64_t wicedfs_usize_t;
typedef int64_t  wicedfs_ssize_t;
#else
typedef uint32_t wicedfs_usize_t;
typedef int32_t  wicedfs_ssize_t;
#endif

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/
typedef enum
{
    WICEDFS_FILE,
    WICEDFS_DIR,
} wicedfs_entry_type_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/
/* Forward declaration of filesystem, file and directory handle types */
typedef /*@abstract@*/ /*@immutable@*/ struct wiced_filesystem_struct  wiced_filesystem_t;
typedef /*@abstract@*/ /*@immutable@*/ struct WFILE_struct             WFILE;
typedef /*@abstract@*/ /*@immutable@*/ struct BDIR_struct              WDIR;

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
 * Typedef of Read Function which will be used to read data from the hardware device.
 *
 * @param[in]  user_param - The user_param value passed when wicedfs_init was called (use for hardware device handles)
 * @param[out] buf        - The buffer to be filled with data
 * @param[in]  size       - The number of bytes to read
 * @param[in]  pos        - Absolute hardware device location to read from
 *
 * @return Number of bytes successfully read  (i.e. 0 on failure)
 */
typedef wicedfs_usize_t (*wicedfs_read_func_t)( void* user_param, /*@out@*/ void* buf, wicedfs_usize_t size, wicedfs_usize_t pos );


/**
 * Initialise the file system
 *
 * Initialises the filesystem for use.
 * This function reads data from a hardware device via a function pointer.
 * It assumes that the device is ready to read immediately.
 *
 * @param[in]  base       - The location on the hardware device where the filesystem start header resides.
 * @param[in]  read_func  - Function pointer to be used for reading hardware
 * @param[out] fs_handle  - Receives the filesystem handle to be used when opening files/directories.
 * @param[in]  user_param - A user parameter which will be passed to the WICEDFS_READ macro. e.g. for hardware device handles
 *
 * @return 0 on success
 */
int wicedfs_init ( wicedfs_usize_t base, wicedfs_read_func_t read_func, /*@out@*/ wiced_filesystem_t* fs_handle, /*@dependent@*/ void * user_param );

/**
 * Returns the size of a file
 *
 * @param[in] filename  - The filename of the file to examine
 * @param[in] fs_handle - The filesystem handle to use - obtained from wicedfs_init
 *
 * @return The size in bytes of the file, or negative on error
 */
wicedfs_ssize_t wicedfs_filesize ( const wiced_filesystem_t* fs_handle, const char* filename );

/**
 * Open a file for reading
 *
 * Since the filesystem is read-only, a "file mode" is not needed.
 *
 * @param[in]  fs_handle - The filesystem handle to use - obtained from wicedfs_init
 * @param[out] handle    - a pointer to a WFILE structure which will receive the
 *                         file handle after it is opened
 * @param[in]  filename  - The filename of the file to open
 *
 * @return 0 on success
 */
int wicedfs_fopen ( /*@dependent@*/ const wiced_filesystem_t* fs_handle, /*@out@*/ WFILE* handle, const char* filename );

/**
 * Seek to a location within a file
 *
 * This is similar to the fseek() in ISO C.
 *
 * @param[in] stream - The file handle on which to perform the seek.
 *                     Must have been previously opened with wicedfs_fopen.
 * @param[in] offset - The offset in bytes
 * @param[in] whence - SEEK_SET = Offset from start of file
 *                     SEEK_CUR = Offset from current position in file
 *                     SEEK_END = Offset from end of file
 *
 * @return 0 on success
 */
int wicedfs_fseek ( WFILE* stream, wicedfs_ssize_t offset, int whence );

/**
 * Returns the current location within a file
 *
 * This is similar to the ftell() in ISO C.
 *
 * @param[in] stream - The file handle to be examined
 *
 * @return The current location within the file
 */
wicedfs_usize_t wicedfs_ftell ( WFILE* stream );

/**
 * Reads data from a file into a memory buffer
 *
 * This is similar to the fread() in ISO C.
 *
 * @param[out] data  - A pointer to the memory buffer that will
 *                     receive the data that is read
 * @param[in] size   - the number of bytes per item
 * @param[in] count  - the number of items to read
 *                     i.e. total bytes read = size * count
 * @param[in] stream - the file handle to read from
 *
 * @return the number of items successfully read.
 */
wicedfs_usize_t wicedfs_fread ( /*@out@*/ void* data, wicedfs_usize_t size, wicedfs_usize_t count, WFILE* stream );


/**
 * Check the end-of-file flag for a stream
 *
 * This is similar to the feof() in ISO C.
 *
 * @param[in] stream - the file handle to check for EOF
 *
 * @return 1 = EOF
 */
int wicedfs_feof ( WFILE* stream );

/**
 * Close a file
 *
 * This is similar to the fclose() in ISO C.
 *
 * @param[in] stream - the file handle to close
 *
 * @return 0 = success
 */
int wicedfs_fclose ( WFILE* stream );

/**
 * Opens a directory
 *
 * This is similar to the opendir() in ISO C.
 *
 * @param[in] fs_handle - The filesystem handle to use - obtained from wicedfs_init
 * @param[out] handle   - a pointer to a directory structure which
 *                        will be filled with the opened handle
 * @param[in] dirname   - the path of the directory to open
 *
 * @return 0 = Success
 */
int wicedfs_opendir ( /*@dependent@*/ const wiced_filesystem_t* fs_handle, /*@out@*/ WDIR* handle, const char* dirname );

/**
 * Reads a directory entry
 *
 * This is similar to the readdir() in ISO C.
 *
 * @param[in]  dirp         - the directory handle to read from
 * @param[out] name_buf     - pointer to a buffer that will receive the filename
 * @param[in]  name_buf_len - the maximum number of bytes that can be put in the buffer
 * @param[out] type         - pointer to variable that will receive entry type (file or dir)
 *
 * @return 0 = Success
 */
int wicedfs_readdir( WDIR* dirp, /*@out@*/ char* name_buf, unsigned int name_buf_len, wicedfs_entry_type_t* type );


/**
 * Check the end-of-directory flag for a directory
 *
 * Checks whether the selected directory handle is
 * at the end of the available directory entries.
 *
 * @param[in] stream - the file handle to check for EOF
 *
 * @return 1 = End-of-Directory
 */
int wicedfs_eodir ( WDIR* dirp );

/**
 * Returns a directory handle to the first entry
 *
 * This is similar to the rewinddir() in ISO C.
 *
 * @param[in] dirp - the directory handle to rewind
 */
void wicedfs_rewinddir ( WDIR *dirp );

/**
 * Closes a directory handle
 *
 * @param[in] dirp - the directory handle to close
 *
 * @return 0 = Success
 */
int wicedfs_closedir ( WDIR* dirp );


/**
 * File-system Handle Structure
 */
struct wiced_filesystem_struct
{
                    wicedfs_usize_t     root_dir_addr;     /** Absolute location on hardware of start of root directory header */
                    wicedfs_read_func_t read_func;         /** Function to be used for reading hardware device */
    /*@dependent@*/ void*               user_param;        /** User parameters pointer e.g. for hardware device handles */
};

/**
 * File Handle Structure
 */
struct WFILE_struct
{
    /*@dependent@*/ const wiced_filesystem_t* fs_handle;         /** The filesystem associated with this file handle */
                    wicedfs_usize_t           size;              /** Size in bytes of file content */
                    wicedfs_usize_t           location;          /** Absolute location on hardware of start of file content */
                    wicedfs_usize_t           current_location;  /** Current read location within file */
                    uint8_t                   eof;               /** Current end of file flag value stream, retrieved with wicedfs_feof */
};


/**
 * Directory handle structure
 */
struct BDIR_struct
{
    /*@dependent@*/ const wiced_filesystem_t* fs_handle;            /** The filesystem associated with this directory handle */
                    wicedfs_usize_t           file_table_location;  /** Absolute location on hardware of start of file table */
                    wicedfs_usize_t           num_files;            /** Number of files in file table */
                    wicedfs_usize_t           file_header_size;     /** Size in bytes of each entry in the file table */
                    wicedfs_usize_t           filename_size;        /** Size of the filename field within each entry of the file table */
                    wicedfs_usize_t           current_location;     /** The current read index within the file table */
};

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_WICEDFS_H_ */
