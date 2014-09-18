/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <stdio.h>
#include "wicedfs.h"
#include "platform_dct.h"
#include "elf.h"
#include "wiced_framework.h"
#include "wiced_utilities.h"
#include "platform_config.h"
#include "platform_resource.h"

/******************************************************
 *                      Macros
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
 *               Static Function Declarations
 ******************************************************/
static void load_program( const load_details_t * load_details, uint32_t* new_entry_point );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
int main( void )
{
    const load_details_t * load_details;
    uint32_t entry_point;

    boot_detail_t boot;
    shared_waf_api->platform_dct_read_with_copy( &boot, DCT_INTERNAL_SECTION, OFFSET( platform_dct_header_t, boot_detail ), sizeof(boot_detail_t) );

    load_details = &boot.load_details;
    entry_point  = boot.entry_point;

/*    platform_check_factory_reset( load_details, &entry_point ); */

    if ( load_details->valid )
    {
        load_program( load_details, &entry_point );
    }

    wiced_framework_start_app( entry_point );

    while(1)
    {

    }

    return 0;  /* should never get here */
}



static void load_program( const load_details_t * load_details, uint32_t* new_entry_point )
{

    /* Image copy has been requested */
    if ( load_details->destination.id == EXTERNAL_FIXED_LOCATION)
    {
        /* SFlash destination */
        return;  /* not currently allowed */
    }

    /* Internal Flash/RAM destination */

    if ( load_details->source.id == EXTERNAL_FIXED_LOCATION )
    {
        /* Fixed location in     sflash source - i.e. no filesystem */

        platform_copy_src_t copy_src;

        copy_src.val.fixed_address = load_details->source.detail.external_fixed.location;
        copy_src.type = COPY_SRC_FIXED;

        wiced_framework_copy_external_to_internal( (void*)load_details->destination.detail.internal_fixed.location,
                                            &copy_src,
                                            load_details->destination.detail.internal_fixed.size );
    }
#ifdef BOOTLOADER_LOAD_MAIN_APP_FROM_FILESYSTEM
    else
    {
        int i;
        WFILE f_in;

        /* Source is a filesystem file */
        wicedfs_fopen( &resource_fs_handle, &f_in, load_details->source.detail.filesytem_filename );

        elf_header_t header;

        wicedfs_fread( &header, sizeof(header), 1, &f_in );

        for( i = 0; i < header.program_header_entry_count; i++ )
        {
            platform_copy_src_t copy_src;
            elf_program_header_t prog_header;

            wicedfs_fseek( &f_in, header.program_header_offset + header.program_header_entry_size * i, SEEK_SET);
            wicedfs_fread( &prog_header, sizeof(prog_header), 1, &f_in );

            if ( ( prog_header.data_size_in_file == 0 ) ||     /* size is zero */
                 ( ( prog_header.type & 0x1 ) == 0 ) )         /* non- loadable segment */
            {
                continue;
            }

            copy_src.val.file_handle = &f_in;
            copy_src.type = COPY_SRC_FILE;

            wicedfs_fseek( &f_in, prog_header.data_offset, SEEK_SET);

            wiced_framework_copy_external_to_internal( (void*)prog_header.physical_address,
                                                   &copy_src,
                                                   prog_header.data_size_in_file );
        }

        *new_entry_point = header.entry;

        wicedfs_fclose( &f_in );
    }
#endif /* ifdef USES_FILESYSTEM */


    if ( load_details->load_once )
    {
        char valid_val = 0;
        uint32_t offset = OFFSET(platform_dct_header_t, boot_detail) + OFFSET(load_details_t, valid);
        wiced_dct_write( &valid_val, DCT_INTERNAL_SECTION, offset, sizeof(valid_val) );
    }
}
