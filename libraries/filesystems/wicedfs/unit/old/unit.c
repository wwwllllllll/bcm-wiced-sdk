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
 *  Unit Tester for WicedFS
 *
 *  Runs a suite of tests on the WicedFS API to attempt
 *  to discover bugs
 */

#include <stdio.h>
#include <string.h>
#include "wicedfs.h"

/******************************************************
 *            Compatibility Macros
 ******************************************************/
#ifndef _WIN32
#define off64_t __off64_t
#define _stati64 stat64
#endif /* ifndef _WIN32 */

/******************************************************
 *                    Constants
 ******************************************************/
#if defined( WICEDFS_64BIT_LENGTHS )
#define IMAGE_FILENAME          "image64.bin"
#else /* if defined( WICEDFS_64BIT_LENGTHS ) */
#define IMAGE_FILENAME          "image32.bin"
#endif /* if defined( WICEDFS_64BIT_LENGTHS ) */
#define DIRECTORY_SEPARATOR_STR "/"
#define TEST_FILENAME           "test.txt"
#define TEST2_DIR               "test2"
#define TEST2_FILENAME          "test2/test2.txt"
#define GARBAGE_FILENAME        "badfood"
#define BASE_SIZE               ((int)sizeof(wicedfs_usize_t))

/******************************************************
 *                    Macros
 ******************************************************/

#define TEST_FAIL_ACTION( msg ) { printf msg; __asm__("int $3"); __asm__("nop"); __asm__("nop"); }

#define UNIT_WARNING_CHECK( condition, description ) { if (!(condition)) { TEST_FAIL_ACTION(("Test failed: %s (" #condition ") at %s:%d\n" ,description, __FILE__, __LINE__ )); } }
#define UNIT_ERROR_CHECK(   condition, description ) { if (!(condition)) { TEST_FAIL_ACTION(("Test failed: %s (" #condition ") at %s:%d\n" ,description, __FILE__, __LINE__ )); } }
#define UNIT_FATAL_CHECK(   condition, description ) { if (!(condition)) { TEST_FAIL_ACTION(("Test failed: %s (" #condition ") at %s:%d\n" ,description, __FILE__, __LINE__ )); return 1; } }

/******************************************************
 *                    Global Variables
 ******************************************************/

/******************************************************
 *                    Static Variables
 ******************************************************/

static const char lorem[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor\n" \
                           "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud\n" \
                           "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure\n" \
                           "dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.\n" \
                           "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit\n" \
                           "anim id est laborum.";

static const unsigned int lorem_len = sizeof(lorem)-1;

static int bad_read_countdown = -1;
static int short_read_countdown = -1;
static int corruption_countdown = -1;
static FILE* image_file = NULL;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static wicedfs_usize_t hostfile_wicedfs_read( void* user_param, void* buf, wicedfs_usize_t size, wicedfs_usize_t pos );
static wicedfs_usize_t hostfile_special_global_wicedfs_read( void* user_param, void* buf, wicedfs_usize_t size, wicedfs_usize_t pos );
static int unit_fopen( void );
static int unit_fread_ftell_feof_fseek( void );
static int unit_filesize( void );
static int unit_opendir( void );
static int unit_readdir_eodir( void );
static int unit_wicedfs_init( void );

/******************************************************
 *               Function Definitions
 ******************************************************/

int main (int argc, const char * argv[])
{
    int result;
    const char * image_filename = IMAGE_FILENAME;

    (void) argv;
    (void) argc;

    /* Open the WicedFS image file for reading back */
    image_file = fopen( image_filename, "rb" );
    if ( image_file == NULL )
    {
        printf( "Error opening file system image file %s\n", image_filename );
        return -1;
    }

    result = unit_wicedfs_init( );
    if ( result != 0 )
    {
        return result;
    }

    result = unit_fopen( );
    if ( result != 0 )
    {
        return result;
    }

    result = unit_fread_ftell_feof_fseek( );
    if ( result != 0 )
    {
        return result;
    }

    result = unit_filesize( );
    if ( result != 0 )
    {
        return result;
    }

    result = unit_opendir( );
    if ( result != 0 )
    {
        return result;
    }

    result = unit_readdir_eodir( );
    if ( result != 0 )
    {
        return result;
    }

    printf("Tests finished\n");

    return 0;
}


static int unit_wicedfs_init( void )
{
    wiced_filesystem_t fs_handle;
    int result;
    int count;

    /* Initialise WicedFS normally */
    result = wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_init normal" );


    /* null function pointer */
    result = wicedfs_init( (wicedfs_usize_t) 0, NULL, &fs_handle, image_file );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_init null function pointer" );


    /* null handle pointer */
    result = wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, NULL, image_file );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_init null handle pointer" );

    /* null user data pointer - shoud succeed if read function allows */
    result = wicedfs_init( (wicedfs_usize_t) 0, hostfile_special_global_wicedfs_read, &fs_handle, NULL );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_init null handle pointer" );

    /* Bad reads from hardware */
    count = 0;
    do
    {
        bad_read_countdown = count;
        count++;
        result = wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );
        UNIT_ERROR_CHECK( ( bad_read_countdown != -1 ) || ( result != 0),  "wicedfs_init bad reads" );
    } while ( bad_read_countdown == -1 );
    bad_read_countdown = -1;

    /* Short reads from hardware */
    count = 0;
    do
    {
        short_read_countdown = count;
        count++;
        result = wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );
        UNIT_ERROR_CHECK( ( short_read_countdown != -1 ) || ( result != 0 ),  "wicedfs_init short reads" );
    } while ( short_read_countdown == -1 );
    short_read_countdown = -1;

    /* File Corruption */
    count = 0;
    do
    {
        corruption_countdown = count;
        count++;
        result = wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );
        UNIT_ERROR_CHECK( ( result != 0 ) || ( count > 8 ),  "wicedfs_init file corruption" ); /* Error below offset 8 */
        UNIT_ERROR_CHECK( ( result == 0 ) || ( count <= 8 ),  "wicedfs_init file corruption" ); /* No error above offset 8 */
    } while ( corruption_countdown == -1 );
    corruption_countdown = -1;

    return 0;


}


static int unit_fopen( void )
{
    int result;
    WFILE file_handle;
    wiced_filesystem_t fs_handle;
    int count;

    /* Setup - Initialise WicedFS */
    wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );


    /* Should open */
    result = wicedfs_fopen( &fs_handle, &file_handle, TEST_FILENAME );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fopen with valid filename" );

    result = wicedfs_fclose( &file_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fclose after fopen with valid filename" );

    /* Subdir with leading separator - Should open */
    result = wicedfs_fopen( &fs_handle, &file_handle, DIRECTORY_SEPARATOR_STR  TEST2_FILENAME );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fopen Subdir with leading separator" );

    result = wicedfs_fclose( &file_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_flcose after wicedfs_fopen Subdir with leading separator" );


    /* Subdir without leading separator - Should open */
    result = wicedfs_fopen( &fs_handle, &file_handle,  TEST2_FILENAME );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fopen subdir without leading separator" );

    result = wicedfs_fclose( &file_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fclose after subdir without leading separator" );

    /* Blank filename - error*/
    result = wicedfs_fopen( &fs_handle, &file_handle, "" );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fopen with blank filename" );

    /* NULL filename - error*/
    result = wicedfs_fopen( &fs_handle, &file_handle, NULL );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fopen with NULL filename" );

    /* NULL fs handle - error */
    result = wicedfs_fopen( NULL, &file_handle, TEST_FILENAME );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fopen with null fs handle" );

    /* NULL file handle - error */
    result = wicedfs_fopen( &fs_handle, NULL, TEST_FILENAME );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fopen with null file handle" );


    /* Not Found filename - Should fail */
    result = wicedfs_fopen( &fs_handle, &file_handle, GARBAGE_FILENAME );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fopen with filename that is not in image" );

    /* Bad reads from hardware */
    count = 0;
    do
    {
        bad_read_countdown = count;
        count++;
        result = wicedfs_fopen( &fs_handle, &file_handle, TEST_FILENAME );
        UNIT_ERROR_CHECK( ( bad_read_countdown != -1 ) || ( result != 0),  "wicedfs_fopen bad reads" );
    } while ( bad_read_countdown == -1 );
    bad_read_countdown = -1;

    /* Short reads from hardware */
    count = 0;
    do
    {
        short_read_countdown = count;
        count++;
        result = wicedfs_fopen( &fs_handle, &file_handle, TEST_FILENAME );
        UNIT_ERROR_CHECK( ( short_read_countdown != -1 ) || ( result != 0),  "wicedfs_fopen short reads" );
    } while ( short_read_countdown == -1 );
    short_read_countdown = -1;

    /* File Corruption */
    count = 0;
    do
    {
        corruption_countdown = count;
        count++;
        result = wicedfs_fopen( &fs_handle, &file_handle, TEST_FILENAME );
        UNIT_ERROR_CHECK( (result != 0) || ( count > BASE_SIZE ),  "wicedfs_fopen file corruption" );
        UNIT_ERROR_CHECK( (result == 0) || ( count <= BASE_SIZE ),  "wicedfs_fopen file corruption" );
    } while ( corruption_countdown == -1 );
    corruption_countdown = -1;


    /* cleanup */
    wicedfs_fclose( &file_handle );

    return 0;
}


static int unit_fread_ftell_feof_fseek( void )
{
    int result;
    WFILE file_handle;
    char buffer[lorem_len];
    wicedfs_usize_t readsize;
    wicedfs_usize_t num_read;
    wicedfs_usize_t location;
    wiced_filesystem_t fs_handle;
    int count;

    /* Setup - Initialise WicedFS */
    wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );
    wicedfs_fopen( &fs_handle, &file_handle, TEST_FILENAME );

    /* read 0 bytes - should succeed */
    num_read = wicedfs_fread( buffer, 0, 0, &file_handle );
    UNIT_ERROR_CHECK( num_read == 0,  "wicedfs_fread with zero bytes request" );

    /* read part file - should succeed */
    readsize = (wicedfs_usize_t)lorem_len - 10;
    num_read = wicedfs_fread( buffer, 1, readsize, &file_handle );
    UNIT_ERROR_CHECK( num_read == readsize,  "wicedfs_fread read valid part of file" );

    /* compare content */
    result = memcmp( buffer, lorem, (size_t)readsize );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fread compare data read from file" );

    /* check current location */
    location = wicedfs_ftell( &file_handle );
    UNIT_ERROR_CHECK( location == readsize,  "wicedfs_ftell part way through file" );

    /* read past end of file - should succeed with less bytes */
    num_read = wicedfs_fread( buffer, 1, sizeof(buffer), &file_handle );
    UNIT_ERROR_CHECK( num_read == (wicedfs_usize_t)lorem_len - readsize,  "wicedfs_fread read past end of file" );

    /* should now be EOF */
    result = wicedfs_feof( &file_handle );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_feof check eof is set after read past end of file" );

    /* check current location */
    location = wicedfs_ftell( &file_handle );
    UNIT_ERROR_CHECK( location == lorem_len,  "wicedfs_ftell check location after read past end of file" );

    /* read whilst EOF - should succeed with zero bytes */
    num_read = wicedfs_fread( buffer, 1, sizeof(buffer), &file_handle );
    UNIT_ERROR_CHECK( num_read == 0,  "wicedfs_fread whilst EOF" );

    /* seek back to start and reset eof flag */
    result = wicedfs_fseek( &file_handle, 0, SEEK_SET );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fseek back to start of file" );

    /* should NOT be EOF */
    result = wicedfs_feof( &file_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_feof after seek back to start of file" );

    /* check current location */
    location = wicedfs_ftell( &file_handle );
    UNIT_ERROR_CHECK( location == 0,  "wicedfs_ftell after seek back to start of file" );

    /* read whole file - should succeed but NOT set eof */
    num_read = wicedfs_fread( buffer, 1, lorem_len, &file_handle );
    UNIT_ERROR_CHECK( num_read == lorem_len,  "wicedfs_fread whole file" );

    /* should NOT be EOF */
    result = wicedfs_feof( &file_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_feof after read whole file" );

    /* check current location */
    location = wicedfs_ftell( &file_handle );
    UNIT_ERROR_CHECK( location == lorem_len,  "wicedfs_ftell after read whole file" );

    /* seek backward 5 bytes */
    result = wicedfs_fseek( &file_handle, -5, SEEK_CUR );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fseek back 5 bytes from current location" );

    /* check current location */
    location = wicedfs_ftell( &file_handle );
    UNIT_ERROR_CHECK( location == lorem_len -5,  "wicedfs_ftell after seek back 5 bytes from current location" );

    /* seek from end of file backward 10 bytes */
    result = wicedfs_fseek( &file_handle, -10, SEEK_END );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_fseek back 10 bytes from end of file" );

    /* check current location */
    location = wicedfs_ftell( &file_handle );
    UNIT_ERROR_CHECK( location == lorem_len -10,  "wicedfs_ftell after seek back 10 bytes from end of file" );

    /* read 4 bytes in two items - should return 2 */
    num_read = wicedfs_fread( buffer, 2, 2, &file_handle );
    UNIT_ERROR_CHECK( num_read == 2,  "wicedfs_fread read 4 bytes in two items" );




    /* read with null handle */
    num_read = wicedfs_fread( buffer, 1, lorem_len, NULL );
    UNIT_ERROR_CHECK( num_read == 0,  "wicedfs_fread null handle" );

    /* read with null buffer */
    num_read = wicedfs_fread( NULL, 1, lorem_len, &file_handle );
    UNIT_ERROR_CHECK( num_read == 0,  "wicedfs_fread null handle" );

    /* ftell with null handle */
    location = wicedfs_ftell( NULL );
    UNIT_ERROR_CHECK( location == 0,  "wicedfs_ftell with null handle" );

    /* seek with null handle */
    result = wicedfs_fseek( NULL, -10, SEEK_END );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fseek with null handle" );

    /* seek before start of file */
    result = wicedfs_fseek( &file_handle, -10, SEEK_SET );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fseek before start of file" );

    /* seek before after end of file */
    result = wicedfs_fseek( &file_handle, 10, SEEK_END );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fseek after end of file" );

    /* seek with bad whence value - should fail */
    result = wicedfs_fseek( &file_handle, -10, SEEK_END + 100 );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_fseek with bad whence value" );

    /* feof with null handle*/
    result = wicedfs_feof( NULL );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_feof with null handle" );




    /* Bad reads from hardware */
    count = 0;
    do
    {
        bad_read_countdown = count;
        count++;
        num_read = wicedfs_fread( buffer, 1, lorem_len, &file_handle );
        UNIT_ERROR_CHECK( ( bad_read_countdown != -1 ) || ( num_read == 0 ),  "wicedfs_fread bad reads" );
    } while ( bad_read_countdown == -1 );
    bad_read_countdown = -1;

    /* Short reads from hardware */
    count = 0;
    do
    {
        short_read_countdown = count;
        count++;

        num_read = wicedfs_fread( buffer, 1, lorem_len, &file_handle );
        UNIT_ERROR_CHECK( ( short_read_countdown != -1 ) || ( num_read == 0 ),  "wicedfs_fread short reads" );
    } while ( short_read_countdown == -1 );
    short_read_countdown = -1;

    /* File Corruption */
    count = 0;
    do
    {
        corruption_countdown = count;
        count++;
        num_read = wicedfs_fread( buffer, 1, lorem_len, &file_handle );
        UNIT_ERROR_CHECK( num_read == 0,  "wicedfs_fread file corruption" );
        result = wicedfs_fseek( &file_handle, 0, SEEK_SET );
        UNIT_ERROR_CHECK( result == 0,  "wicedfs_fseek file corruption" );
    } while ( corruption_countdown == -1 );
    corruption_countdown = -1;



    /* cleanup */
    wicedfs_fclose( &file_handle );

    return 0;
}

static int unit_filesize( void )
{
    wicedfs_ssize_t filesize;
    wiced_filesystem_t fs_handle;
    int count;

    /* Setup - Initialise WicedFS */
    wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );


    /* get filesize of valid file */
    filesize = wicedfs_filesize( &fs_handle, TEST_FILENAME );
    UNIT_ERROR_CHECK(  (wicedfs_usize_t)filesize == lorem_len,  "wicedfs_filesize valid file" );

    /* get filesize of missing file */
    filesize = wicedfs_filesize( &fs_handle, GARBAGE_FILENAME );
    UNIT_ERROR_CHECK( filesize < 0,  "wicedfs_filesize missing file" );

    /* get filesize with blank filename */
    filesize = wicedfs_filesize( &fs_handle, "" );
    UNIT_ERROR_CHECK( filesize < 0,  "wicedfs_filesize blank filename" );

    /* get filesize with null filename */
    filesize = wicedfs_filesize( &fs_handle, NULL );
    UNIT_ERROR_CHECK( filesize < 0,  "wicedfs_filesize NULL filename" );

    /* get filesize with null handle */
    filesize = wicedfs_filesize( NULL, TEST_FILENAME );
    UNIT_ERROR_CHECK( filesize < 0,  "wicedfs_filesize NULL handle" );

    /* Bad reads from hardware */
    count = 0;
    do
    {
        bad_read_countdown = count;
        count++;
        filesize = wicedfs_filesize( &fs_handle, TEST_FILENAME );
        UNIT_ERROR_CHECK( ( bad_read_countdown != -1 ) || ( filesize < 0),  "wicedfs_filesize bad reads" );
    } while ( bad_read_countdown == -1 );
    bad_read_countdown = -1;

    /* Short reads from hardware */
    count = 0;
    do
    {
        short_read_countdown = count;
        count++;
        filesize = wicedfs_filesize( &fs_handle, TEST_FILENAME );
        UNIT_ERROR_CHECK( ( short_read_countdown != -1 ) || ( filesize < 0),  "wicedfs_filesize short reads" );
    } while ( short_read_countdown == -1 );
    short_read_countdown = -1;

    /* File Corruption */
    count = 0;
    do
    {
        corruption_countdown = count;
        count++;
        filesize = wicedfs_filesize( &fs_handle, TEST_FILENAME );
        UNIT_ERROR_CHECK( ( filesize < 0) || ( count > BASE_SIZE ),  "wicedfs_filesize file corruption" );
        UNIT_ERROR_CHECK( ( filesize == (wicedfs_ssize_t)lorem_len) || ( count <= BASE_SIZE ),  "wicedfs_filesize file corruption" );
    } while ( corruption_countdown == -1 );
    corruption_countdown = -1;

    return 0;

}


static int unit_opendir( void )
{
    WDIR dir_handle;
    int result;
    int count;
    wiced_filesystem_t fs_handle;

    /* Setup - Initialise WicedFS */
    wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );


    /* Open root directory */
    result = wicedfs_opendir( &fs_handle, &dir_handle, DIRECTORY_SEPARATOR_STR );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_opendir normal" );

    result = wicedfs_closedir( &dir_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_closedir normal" );

    /* Open sub directory */
    result = wicedfs_opendir( &fs_handle, &dir_handle, TEST2_DIR );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_opendir subdirectory" );

    result = wicedfs_closedir( &dir_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_closedir subdirectory" );


    /* Open sub directory with leading slash */
    result = wicedfs_opendir( &fs_handle, &dir_handle, DIRECTORY_SEPARATOR_STR  TEST2_DIR );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_opendir subdirectory with leading slash" );

    result = wicedfs_closedir( &dir_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_closedir subdirectory with leading slash" );

    /* Open sub directory with trailing slash */
    result = wicedfs_opendir( &fs_handle, &dir_handle, TEST2_DIR  DIRECTORY_SEPARATOR_STR );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_opendir subdirectory with trailing slash" );

    result = wicedfs_closedir( &dir_handle );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_closedir subdirectory with trailing slash" );

    /* Open garbage sub directory */
    result = wicedfs_opendir( &fs_handle, &dir_handle, GARBAGE_FILENAME );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_opendir missing directory" );

    /* Opendir with blank directory name */
    result = wicedfs_opendir( &fs_handle, &dir_handle, "" );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_opendir blank directory" );

    /* Opendir with NULL directory name */
    result = wicedfs_opendir( &fs_handle, &dir_handle, NULL );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_opendir null directory" );

    /* Opendir with NULL fs handle */
    result = wicedfs_opendir( NULL, &dir_handle, TEST2_DIR );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_opendir null fs handle" );

    /* Opendir with NULL dir handle */
    result = wicedfs_opendir( &fs_handle, NULL, TEST2_DIR );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_opendir null dir handle" );

    /* Bad reads from hardware */
    count = 0;
    do
    {
        bad_read_countdown = count;
        count++;
        result = wicedfs_opendir( &fs_handle, &dir_handle, TEST2_DIR );
        UNIT_ERROR_CHECK( ( bad_read_countdown != -1 ) || ( result != 0 ),  "wicedfs_opendir bad reads" );
    } while ( bad_read_countdown == -1 );
    bad_read_countdown = -1;

    /* Short reads from hardware */
    count = 0;
    do
    {
        short_read_countdown = count;
        count++;
        result = wicedfs_opendir( &fs_handle, &dir_handle, TEST2_DIR );
        UNIT_ERROR_CHECK( ( short_read_countdown != -1 ) || ( result != 0 ),  "wicedfs_opendir short reads" );
    } while ( short_read_countdown == -1 );
    short_read_countdown = -1;

    /* File Corruption */
    count = 0;
    do
    {
        corruption_countdown = count;
        count++;
        result = wicedfs_opendir( &fs_handle, &dir_handle, TEST2_DIR );
        UNIT_ERROR_CHECK( ( result != 0 ) || ( count > BASE_SIZE ),  "wicedfs_opendir corrupt file" );
        UNIT_ERROR_CHECK( ( result == 0 ) || ( count <= BASE_SIZE ) || ( count > BASE_SIZE * 2 ),  "wicedfs_opendir corrupt file" );
        UNIT_ERROR_CHECK( ( result != 0 ) || ( count <= BASE_SIZE * 2 ) || ( count > BASE_SIZE * 2 + 4 ),  "wicedfs_opendir corrupt file" );
        UNIT_ERROR_CHECK( ( result == 0 ) || ( count <= BASE_SIZE * 2 + 4 ) ,  "wicedfs_opendir corrupt file" );
    } while ( corruption_countdown == -1 );
    corruption_countdown = -1;


    return 0;
}





static int unit_readdir_eodir( void )
{
    WDIR dir_handle;
    int result;
    int count;
    char  filename[100];
    wicedfs_entry_type_t type;
    wiced_filesystem_t fs_handle;

    /* Setup - Initialise WicedFS */
    wicedfs_init( (wicedfs_usize_t) 0, hostfile_wicedfs_read, &fs_handle, image_file );
    /* Open root directory */
    wicedfs_opendir( &fs_handle, &dir_handle, DIRECTORY_SEPARATOR_STR );




    /* Read first entry */
    result = wicedfs_readdir( &dir_handle, filename, sizeof(filename), &type );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_readdir first root entry" );

    result = strcmp( filename, TEST_FILENAME );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_readdir check name validity of first root entry" );
    UNIT_ERROR_CHECK( type == WICEDFS_FILE,  "wicedfs_readdir check type validity of first root entry" );

    /* Read second entry */
    result = wicedfs_readdir( &dir_handle, filename, sizeof(filename), &type );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_readdir second root entry" );

    result = strcmp( filename, TEST2_DIR );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_readdir check name validity of second root entry" );
    UNIT_ERROR_CHECK( type == WICEDFS_DIR,  "wicedfs_readdir check type validity of second root entry" );

    /* should now indicate end of directory */
    result = wicedfs_eodir( &dir_handle );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_eodir at end of root entry" );

    /* Read off end of directory - should fail */
    result = wicedfs_readdir( &dir_handle, filename, sizeof(filename), &type );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_readdir off end of root entry" );

    /* Rewind directory */
    wicedfs_rewinddir( &dir_handle );

    /* Read first entry again */
    result = wicedfs_readdir( &dir_handle, filename, sizeof(filename), &type );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_readdir first root entry again" );

    result = strcmp( filename, TEST_FILENAME );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_readdir check name validity of first root entry again" );
    UNIT_ERROR_CHECK( type == WICEDFS_FILE,  "wicedfs_readdir check type validity of first root entry again" );

    /* Read second entry again but with buffer too small */
    result = wicedfs_readdir( &dir_handle, filename, 2, &type );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_readdir second root entry again but with buffer too small" );

    /* Rewind directory again */
    wicedfs_rewinddir( &dir_handle );

    /* Read with null dir handle */
    result = wicedfs_readdir( NULL, filename, sizeof(filename), &type );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_readdir with null dir handle" );

    /* Read with null buffer pointer */
    result = wicedfs_readdir( &dir_handle, NULL, sizeof(filename), &type );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_readdir with null buffer pointer" );

    /* Read with null type pointer */
    result = wicedfs_readdir( &dir_handle, filename, sizeof(filename), NULL );
    UNIT_ERROR_CHECK( result != 0,  "wicedfs_readdir with null type pointer" );

    /* eodir with null pointer */
    result = wicedfs_eodir( NULL );
    UNIT_ERROR_CHECK( result == 0,  "wicedfs_eodir with null pointer" );

    /* Rewind directory with null pointer */
    wicedfs_rewinddir( NULL );



    /* Bad reads from hardware */
    count = 0;
    do
    {
        bad_read_countdown = count;
        count++;
        result = wicedfs_readdir( &dir_handle, filename, sizeof(filename), &type );
        UNIT_ERROR_CHECK( ( bad_read_countdown != -1 ) || ( result != 0 ),  "wicedfs_readdir bad reads" );
    } while ( bad_read_countdown == -1 );
    bad_read_countdown = -1;

    /* Short reads from hardware */
    count = 0;
    do
    {
        short_read_countdown = count;
        count++;
        result = wicedfs_readdir( &dir_handle, filename, sizeof(filename), &type );
        UNIT_ERROR_CHECK( ( short_read_countdown != -1 ) || ( result != 0 ),  "wicedfs_readdir short reads" );
    } while ( short_read_countdown == -1 );
    short_read_countdown = -1;

    /* File Corruption */
    count = 0;
    do
    {
        corruption_countdown = count;
        count++;
        result = wicedfs_readdir( &dir_handle, filename, sizeof(filename), &type );
        UNIT_ERROR_CHECK( ( result != 0 ),  "wicedfs_readdir corrupt file" );
    } while ( corruption_countdown == -1 );
    corruption_countdown = -1;




    /* cleanup */
    result = wicedfs_closedir( &dir_handle );
    if ( result != 0 )
    {
        return result;
    }

    return 0;
}









/** Read function provided to WicedFS
 *
 *  This reads data from the hardware device (a local host file)
 *
 * @param[in]  user_param - The user_param value passed when wicedfs_init was called (use for hardware device handles)
 * @param[out] buf        - The buffer to be filled with data
 * @param[in]  size       - The number of bytes to read
 * @param[in]  pos        - Absolute hardware device location to read from
 *
 * @return Number of bytes successfully read  (i.e. 0 on failure)
 */
static wicedfs_usize_t hostfile_wicedfs_read( void* user_param, void* buf, wicedfs_usize_t size, wicedfs_usize_t pos )
{
    wicedfs_usize_t result;

    if ( bad_read_countdown == 0 )
    {
        bad_read_countdown--;
        return 0;
    }
    if ( bad_read_countdown != -1 )
    {
        bad_read_countdown--;
    }

    if ( short_read_countdown == 0 )
    {
        short_read_countdown--;
        return (size>0)? size-1 : 0;
    }
    if ( short_read_countdown != -1 )
    {
        short_read_countdown--;
    }

    /* Seek to the requested hardware location in the image file */
    if ( 0 != fseeko64( (FILE*)user_param, (off64_t)(pos), SEEK_SET) )
    {
        return 0;
    }

    /* Read the requested data from the image file */
    result = fread( buf, 1, (size_t)size, (FILE*)user_param );

    if ( ( corruption_countdown >= 0 ) &&
         ( corruption_countdown < (int)size ) )
    {
        ((unsigned char*)buf)[ corruption_countdown ] |= (unsigned char)0xa5;
        corruption_countdown = -1;
    }
    if ( corruption_countdown != -1 )
    {
        short_read_countdown -= (int)size;
    }

    return result;
}



static wicedfs_usize_t hostfile_special_global_wicedfs_read( void* user_param, void* buf, wicedfs_usize_t size, wicedfs_usize_t pos )
{
    (void) user_param;

    /* Seek to the requested hardware location in the image file */
    if ( 0 != fseeko64( image_file, (off64_t)(pos), SEEK_SET) )
    {
        return 0;
    }

    /* Read the requested data from the image file */
    return fread( buf, 1, (size_t)size, image_file );
}


