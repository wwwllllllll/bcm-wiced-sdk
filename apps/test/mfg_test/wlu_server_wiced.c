/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/*
 * Wl server for generic RTOS
 */

#include <stdio.h>
#include "typedefs.h"
#include "bcmutils.h"
#include "bcmcdc.h"
#include "wlioctl.h"
#include "wlu.h"
#include "wlu_remote.h"
#include "internal/wwd_sdpcm.h"
#include "wl_drv.h"
#include "network/wwd_buffer_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "chip_constants.h"
#include "wiced_rtos.h"
#include "wiced_management.h"
#include "wiced_defaults.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"
#include <string.h>
#include "wifi_nvram_image.h"
#include "resources.h"
#include "wwd_assert.h"
#include "mfg_test.h"

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

/******************************************************
 *               Variable Definitions
 ******************************************************/
static int                   ifnum                    = 0;
static int                   currently_downloading    = 0;
static host_semaphore_type_t downloading_semaphore;
static host_semaphore_type_t download_ready_semaphore;
static wiced_thread_t        downloading_thread;
unsigned short               defined_debug            = 0; //DEBUG_ERR | DEBUG_INFO;

/******************************************************
 *               Function Definitions
 ******************************************************/

int
rwl_create_dir(void)
{
    /* not supported */
    return 0;
}

int
remote_shell_execute(char *buf_ptr)
{
    /* not supported */
    UNUSED_PARAMETER(buf_ptr);
    return 0;
}

int
remote_shell_get_resp(char* shell_fname, char* buf_ptr, int msg_len)
{
    /* not supported */
    UNUSED_PARAMETER(shell_fname);
    UNUSED_PARAMETER(buf_ptr);
    UNUSED_PARAMETER(msg_len);
    return 0;
}
int
rwl_write_serial_port(void* hndle, char* write_buf, unsigned long size, unsigned long *numwritten)
{
    /* not invoked for dongle transports */
    UNUSED_PARAMETER(hndle);

    *numwritten = fwrite( (const void *) write_buf, (size_t) size, 1, stdout );

    return SUCCESS;
}

void*
rwl_open_transport(int remote_type, char *port, int ReadTotalTimeout, int debug)
{
    /* not invoked for dongle transports */
    UNUSED_PARAMETER(remote_type);
    UNUSED_PARAMETER(port);
    UNUSED_PARAMETER(ReadTotalTimeout);
    UNUSED_PARAMETER(debug);
    return NULL;
}

int
rwl_close_transport(int remote_type, void* Des)
{
    /* not invoked for dongle transports */
    UNUSED_PARAMETER(remote_type);
    UNUSED_PARAMETER(Des);
    return FAIL;
}

int
rwl_read_serial_port(void* hndle, char* read_buf, uint data_size, uint *numread)
{
    /* not invoked for dongle transports */
    UNUSED_PARAMETER(hndle);
    *numread = fread( (void *) read_buf, (size_t) data_size, 1, stdin );
    return SUCCESS;
}

void
rwl_sleep(int delay)
{
    host_rtos_delay_milliseconds( delay );
}

void
rwl_sync_delay(uint noframes)
{
    if (noframes > 1) {
        rwl_sleep(200);
    }
}

int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
    wwd_result_t ret;
    wiced_buffer_t internalPacket;
    wiced_buffer_t response_buffer;
    UNUSED_PARAMETER(wl);

    // Set Wireless Security Type
    void* ioctl_data = wwd_sdpcm_get_ioctl_buffer( &internalPacket, len );
    memcpy( ioctl_data, buf, len );
    ret = wwd_sdpcm_send_ioctl( (set==TRUE)?SDPCM_SET:SDPCM_GET, cmd, internalPacket, &response_buffer, (wwd_interface_t) ifnum );
    if (ret == WWD_SUCCESS)
    {
        if (set!=TRUE)
        {
            memcpy( buf, host_buffer_get_current_piece_data_pointer( response_buffer ), MIN( host_buffer_get_current_piece_size( response_buffer ), len )  );
        }
        host_buffer_release(response_buffer, WWD_NETWORK_RX);
    }
    else
    {
        ret = WLAN_ENUM_OFFSET - ret;
    }
    return ret; /*( ret == WICED_SUCCESS)?0:IOCTL_ERROR;*/
}


/*
 * Function to set interface.
 */
int
set_interface(void *wl, char *intf_name)
{
    if ( sscanf( intf_name, "eth%d", &ifnum ) != 1 )
    {
        ifnum = 0;
        return BCME_ERROR;
    }
    return BCME_OK;
}

static void downloading_init_func( uint32_t arg )
{
    wiced_init( );
    WICED_END_OF_CURRENT_THREAD_NO_LEAK_CHECK( );
}

void start_download( void )
{
    wiced_deinit( );
    currently_downloading = 1;

    host_rtos_init_semaphore( &downloading_semaphore );
    host_rtos_init_semaphore( &download_ready_semaphore );

    wiced_rtos_create_thread( &downloading_thread, WICED_NETWORK_WORKER_PRIORITY, "downloading_init_func", downloading_init_func, 5000, NULL);
    host_rtos_get_semaphore( &download_ready_semaphore, NEVER_TIMEOUT, WICED_FALSE );
    host_rtos_deinit_semaphore( &download_ready_semaphore );
}

void finish_download( void )
{
    host_rtos_set_semaphore( &downloading_semaphore, WICED_FALSE );
    host_rtos_deinit_semaphore( &downloading_semaphore );
    currently_downloading = 0;
}

void membytes_write( uint32_t address, uint8_t* buf, uint32_t length )
{
    wwd_bus_set_backplane_window( address );
    wwd_bus_transfer_bytes( BUS_WRITE, BACKPLANE_FUNCTION, ( address & BACKPLANE_ADDRESS_MASK ), length, (wwd_transfer_bytes_packet_t*) ( buf -  WWD_BUS_HEADER_SIZE ) );
}

wwd_result_t external_write_wifi_firmware_and_nvram_image( void )
{
    if ( currently_downloading == 0 )
    {
        wwd_result_t result;
        result = wwd_bus_write_wifi_firmware_image( );
        if ( result != WWD_SUCCESS )
        {
            return result;
        }
        return wwd_bus_write_wifi_nvram_image( );
    }

    host_rtos_set_semaphore( &download_ready_semaphore, WICED_FALSE );
    host_rtos_get_semaphore( &downloading_semaphore, NEVER_TIMEOUT, WICED_FALSE );
    return WWD_SUCCESS;
}

