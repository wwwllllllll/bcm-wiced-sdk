/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include "spi_flash.h"
#include "spi_flash_internal.h"
#include "spi_flash_platform_interface.h"
#include <string.h> /* for NULL */

/*@access sflash_handle_t@*/ /* Lint: permit access to abstract sflash handle implementation */

int sflash_read_ID( const sflash_handle_t* handle, /*@out@*/ device_id_t* data_addr )
{
    return generic_sflash_command( handle, SFLASH_READ_JEDEC_ID, 0, NULL, (unsigned long) 3, NULL, data_addr );
}

int sflash_write_enable( const sflash_handle_t* const handle )
{
    if ( handle->write_allowed == SFLASH_WRITE_ALLOWED )
    {
        unsigned char status_register;
        int status;

        /* Send write-enable command */
        status = generic_sflash_command( handle, SFLASH_WRITE_ENABLE, 0, NULL, 0, NULL, NULL );
        if ( status != 0 )
        {
            return status;
        }

        /* Check status register */
        if ( 0 != ( status = sflash_read_status_register( handle, &status_register ) ) )
        {
            return status;
        }

        /* Check if Block protect bits are set */
        if ( status_register != SFLASH_STATUS_REGISTER_WRITE_ENABLED )
        {
            /* Disable protection for all blocks */
            status = sflash_write_status_register( handle, (char) 0 );
            if ( status != 0 )
            {
                return status;
            }

            /* Re-Enable writing */
            status = generic_sflash_command( handle, SFLASH_WRITE_ENABLE, 0, NULL, 0, NULL, NULL );
            if ( status != 0 )
            {
                return status;
            }
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

int sflash_chip_erase( const sflash_handle_t* const handle )
{
    int status = sflash_write_enable( handle );
    if ( status != 0 )
    {
        return status;
    }
    return generic_sflash_command( handle, SFLASH_CHIP_ERASE1, 0, NULL, 0, NULL, NULL );
}

#include "wwd_assert.h"

int sflash_sector_erase ( const sflash_handle_t* const handle, unsigned long device_address )
{

    char device_address_array[3] =  { (char) ( ( device_address & 0x00FF0000 ) >> 16 ),
                                      (char) ( ( device_address & 0x0000FF00 ) >>  8 ),
                                      (char) ( ( device_address & 0x000000FF ) >>  0 ) };

    int retval;
    int status = sflash_write_enable( handle );
    if ( status != 0 )
    {
        return status;
    }
    retval = generic_sflash_command( handle, SFLASH_SECTOR_ERASE, (unsigned long) 3, device_address_array, 0, NULL, NULL );
    wiced_assert("error", retval == 0);
    return retval;
}

int sflash_read_status_register( const sflash_handle_t* const handle, /*@out@*/  /*@dependent@*/ unsigned char* const dest_addr )
{
    return generic_sflash_command( handle, SFLASH_READ_STATUS_REGISTER, 0, NULL, (unsigned long) 1, NULL, dest_addr );
}



int sflash_read( const sflash_handle_t* const handle, unsigned long device_address, /*@out@*/ /*@dependent@*/ void* const data_addr, unsigned int size )
{
    char device_address_array[3] =  { (char) ( ( device_address & 0x00FF0000 ) >> 16 ),
                                      (char) ( ( device_address & 0x0000FF00 ) >>  8 ),
                                      (char) ( ( device_address & 0x000000FF ) >>  0 ) };

    return generic_sflash_command( handle, SFLASH_READ, (unsigned long) 3, device_address_array, (unsigned long) size, NULL, data_addr );
}


int sflash_get_size( const sflash_handle_t* const handle, /*@out@*/ unsigned long* const size )
{
    *size = 0; /* Unknown size to start with */

#ifdef SFLASH_SUPPORT_MACRONIX_PARTS
    if ( handle->device_id == SFLASH_ID_MX25L8006E )
    {
        *size = (unsigned long) 0x100000; /* 1MByte */
    }
    else if ( handle->device_id == SFLASH_ID_MX25L1606E )
    {
        *size = (unsigned long) 0x200000; /* 2MByte */
    }
#endif /* ifdef SFLASH_SUPPORT_MACRONIX_PARTS */
#ifdef SFLASH_SUPPORT_SST_PARTS
    if ( handle->device_id == SFLASH_ID_SST25VF080B )
    {
        *size = (unsigned long) 0x100000; /* 1MByte */
    }
#endif /* ifdef SFLASH_SUPPORT_SST_PARTS */
#ifdef SFLASH_SUPPORT_EON_PARTS
    if ( handle->device_id == SFLASH_ID_EN25QH16 )
    {
        *size = (unsigned long) 0x200000; /* 2MByte */
    }
#endif /* ifdef SFLASH_SUPPORT_EON_PARTS */

    return 0;
}

int sflash_write( const sflash_handle_t* const handle, unsigned long device_address, /*@observer@*/ const void* const data_addr, unsigned int size )
{
    int status;
    unsigned int write_size;
    unsigned int max_write_size = (unsigned int) 1;
    int enable_before_every_write = 1;
    unsigned char* data_addr_ptr = (unsigned char*) data_addr;
    unsigned char curr_device_address[3];

    if ( handle->write_allowed != SFLASH_WRITE_ALLOWED )
    {
        return -1;
    }

    /* Some manufacturers support programming an entire page in one command. */

#ifdef SFLASH_SUPPORT_MACRONIX_PARTS
    if ( SFLASH_MANUFACTURER( handle->device_id ) == SFLASH_MANUFACTURER_MACRONIX )
    {
        max_write_size = (unsigned int) 1;  /* TODO: this should be 256, but that causes write errors */
        enable_before_every_write = 1;
    }
#endif /* ifdef SFLASH_SUPPORT_MACRONIX_PARTS */
#ifdef SFLASH_SUPPORT_SST_PARTS
    if ( SFLASH_MANUFACTURER( handle->device_id ) == SFLASH_MANUFACTURER_SST )
    {
        max_write_size = (unsigned int) 1;
        enable_before_every_write = 1;
    }
#endif /* ifdef SFLASH_SUPPORT_SST_PARTS */
#ifdef SFLASH_SUPPORT_EON_PARTS
    if ( SFLASH_MANUFACTURER( handle->device_id ) == SFLASH_MANUFACTURER_EON )
    {
        max_write_size = (unsigned int) 1;
        enable_before_every_write = 1;
    }
#endif /* ifdef SFLASH_SUPPORT_EON_PARTS */



    if ( enable_before_every_write == 0 )
    {
        status = sflash_write_enable( handle );
        if ( status != 0 )
        {
            return status;
        }
    }

    /* Generic x-bytes-at-a-time write */

    while ( size > 0 )
    {
        write_size = ( size > max_write_size )? max_write_size : size;
        curr_device_address[0] = (unsigned char) ( ( device_address & 0x00FF0000 ) >> 16 );
        curr_device_address[1] = (unsigned char) ( ( device_address & 0x0000FF00 ) >>  8 );
        curr_device_address[2] = (unsigned char) ( ( device_address & 0x000000FF ) >>  0 );

        if ( ( enable_before_every_write == 1 ) &&
             ( 0 != ( status = sflash_write_enable( handle ) ) ) )
        {
            return status;
        }

        status = generic_sflash_command( handle, SFLASH_WRITE, (unsigned long) 3, curr_device_address, (unsigned long) write_size, data_addr_ptr, NULL );
        if ( status != 0 )
        {
            return status;
        }

        data_addr_ptr += write_size;
        device_address += write_size;
        size -= write_size;

    }

    return 0;
}

int sflash_write_status_register( const sflash_handle_t* const handle, char value )
{
    char status_register_val = value;
#ifdef SFLASH_SUPPORT_SST_PARTS
    /* SST parts require enabling writing to the status register */
    if ( SFLASH_MANUFACTURER( handle->device_id ) == SFLASH_MANUFACTURER_SST )
    {
        int status;
        if ( 0 != ( status = generic_sflash_command( handle, SFLASH_ENABLE_WRITE_STATUS_REGISTER, 0, NULL, 0, NULL, NULL ) ) )
        {
            return status;
        }
    }
#endif /* ifdef SFLASH_SUPPORT_SST_PARTS */

    return generic_sflash_command( handle, SFLASH_WRITE_STATUS_REGISTER, 0, NULL, (unsigned long) 1, &status_register_val, NULL );
}


int init_sflash( /*@out@*/ sflash_handle_t* const handle, /*@shared@*/ void* peripheral_id, sflash_write_allowed_t write_allowed_in )
{
    int status;
    device_id_t tmp_device_id;

    status = sflash_platform_init( peripheral_id, &handle->platform_peripheral );
    if ( status != 0 )
    {
        return status;
    }

    handle->write_allowed = write_allowed_in;
    handle->device_id     = 0;

    status = sflash_read_ID( handle, &tmp_device_id );
    if ( status != 0 )
    {
        return status;
    }

    handle->device_id = ( ((uint32_t) tmp_device_id.id[0]) << 16 ) +
                        ( ((uint32_t) tmp_device_id.id[1]) <<  8 ) +
                        ( ((uint32_t) tmp_device_id.id[2]) <<  0 );


    if ( write_allowed_in == SFLASH_WRITE_ALLOWED )
    {
        /* Enable writing */
        if (0 != ( status = sflash_write_enable( handle ) ) )
        {
            return status;
        }
    }

    return 0;
}


static inline int is_write_command( sflash_command_t cmd )
{
    return ( ( cmd == SFLASH_WRITE             ) ||
             ( cmd == SFLASH_CHIP_ERASE1       ) ||
             ( cmd == SFLASH_CHIP_ERASE2       ) ||
             ( cmd == SFLASH_SECTOR_ERASE      ) ||
             ( cmd == SFLASH_BLOCK_ERASE_MID   ) ||
             ( cmd == SFLASH_BLOCK_ERASE_LARGE ) )? 1 : 0;
}

int generic_sflash_command(                                      const sflash_handle_t* const handle,
                                                                 sflash_command_t             cmd,
                                                                 unsigned long                num_initial_parameter_bytes,
                            /*@null@*/ /*@observer@*/            const void* const            parameter_bytes,
                                                                 unsigned long                num_data_bytes,
                            /*@null@*/ /*@observer@*/            const void* const            data_MOSI,
                            /*@null@*/ /*@out@*/ /*@dependent@*/ void* const                  data_MISO )
{
    int status;

    sflash_platform_message_segment_t segments[3] =
    {
            { &cmd,            NULL,       (unsigned long) 1 },
            { parameter_bytes, NULL,       num_initial_parameter_bytes },
            /*@-compdef@*/ /* Lint: Tell lint that it is OK that data_MISO is not completely defined */
            { data_MOSI,       data_MISO,  num_data_bytes }
            /*@+compdef@*/
    };

    status = sflash_platform_send_recv( handle->platform_peripheral, segments, (unsigned int) 3  );
    if ( status != 0 )
    {
        /*@-mustdefine@*/ /* Lint: do not need to define data_MISO due to failure */
        return status;
        /*@+mustdefine@*/
    }

    if ( is_write_command( cmd ) == 1 )
    {
        unsigned char status_register;
        /* write commands require waiting until chip is finished writing */

        do
        {
            status = sflash_read_status_register( handle, &status_register );
            if ( status != 0 )
            {
                /*@-mustdefine@*/ /* Lint: do not need to define data_MISO due to failure */
                return status;
                /*@+mustdefine@*/
            }
        } while( ( status_register & SFLASH_STATUS_REGISTER_BUSY ) != (unsigned char) 0 );

    }

    /*@-mustdefine@*/ /* Lint: lint does not realise data_MISO was set by sflash_platform_send_recv */
    return 0;
    /*@+mustdefine@*/
}

/*@noaccess sflash_handle_t@*/
