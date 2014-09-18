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
 * Defines BCM439x WWD bus
 */
#include "string.h" /* For memcpy */
#include "platform_config.h"
#include "wwd_assert.h"
#include "wwd_bus_protocol.h"
#include "platform/wwd_bus_interface.h"
#include "platform/wwd_sdio_interface.h"
#include "RTOS/wwd_rtos_interface.h"

/******************************************************
 *             Macros
 ******************************************************/

#define intseten1         (*(uint32_t *) 0xE000E104)
#define intclren1         (*(uint32_t *) 0xE000E184)
#define gci_gciintstatus  (*(uint32_t *) 0x650014)
#define gci_gcievent0     (*(uint32_t *) 0x6500E0)
#define gci_gcievent1     (*(uint32_t *) 0x6500E4)
#define intclrpending0    (*(uint32_t *) 0xE000E280)
#define intclrpending1    (*(uint32_t *) 0xE000E284)

/******************************************************
 *             Constants
 ******************************************************/

/******************************************************
 *             Structures
 ******************************************************/

typedef struct
{
    /*@shared@*/ /*@null@*/ uint8_t* data;
    uint16_t length;

} sdio_dma_segment_t;

/******************************************************
 *             Variables
 ******************************************************/

volatile uint32_t gci_cnt   = 0;
volatile uint32_t total_int = 0;
static   uint32_t intstatus;

/******************************************************
 *             Static Function Declarations
 ******************************************************/

int             rxactive_dma        ( void );
void            refill_dma          ( void );
extern void     setup_439x_interrupt( void );
extern void     stop_439x_interrupt ( void );
extern void     int_enab            ( void );
extern uint32_t get_gci_intstatus   ( void );
extern uint32_t read_intstatus      ( void );

/******************************************************
 *             Function definitions
 ******************************************************/

/*
static void sdio_oob_irq_handler( void* arg )
{
    UNUSED_PARAMETER(arg);
    wwd_thread_notify_irq( );
}
*/

static void int_disable( void )
{
    intclren1 = 0xFFFFFFFF;
}

void int_enab( void )
{
    intseten1 = 0x2000;
}

uint32_t get_gci_intstatus( void )
{
    return intstatus;
}

/*
static void sdio_enable_bus_irq( void )
{
}

static void sdio_disable_bus_irq( void )
{
}
*/

wwd_result_t host_platform_bus_init( void )
{
    return WWD_SUCCESS;
}

wwd_result_t host_platform_bus_enable_interrupt( void )
{
    setup_439x_interrupt( );
    return WWD_SUCCESS;
}

wwd_result_t host_platform_bus_disable_interrupt( void )
{
    stop_439x_interrupt( );
    return WWD_SUCCESS;
}

void host_platform_bus_buffer_freed( wwd_buffer_dir_t direction )
{
    int prev_active = rxactive_dma( );

    UNUSED_PARAMETER( direction );

    refill_dma( );
    if ( ( prev_active == 0 ) && ( rxactive_dma( ) != 0 ) )
    {
        // We will get flooded with interrupts if enabled when dma ring buffer is empty.
        int_enab( );
    }
}

wwd_result_t host_platform_bus_deinit( void )
{
    return WWD_SUCCESS;
}

wwd_result_t host_platform_sdio_transfer( wwd_bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response )
{
    UNUSED_PARAMETER( direction );
    UNUSED_PARAMETER( command );
    UNUSED_PARAMETER( mode );
    UNUSED_PARAMETER( block_size );
    UNUSED_PARAMETER( argument );
    UNUSED_PARAMETER( data );
    UNUSED_PARAMETER( data_size );
    UNUSED_PARAMETER( response_expected );
    UNUSED_PARAMETER( response );

    return WWD_SUCCESS;
}

void host_platform_enable_high_speed_sdio( void )
{
}

#ifndef  WICED_DISABLE_MCU_POWERSAVE
wwd_result_t host_enable_oob_interrupt( void )
{
    return WWD_SUCCESS;
}

uint8_t host_platform_get_oob_interrupt_pin( void )
{
    return 0;
}
#endif

WWD_RTOS_DEFINE_ISR( sdio_irq )
{
    uint32_t temp;
    intstatus = 0;
    intstatus = ( *(uint32_t *) 0x650064 );
    temp = intstatus;
    (void) temp;
    intstatus &= ( 0x3 << ( 41 & 31 ) );
    intstatus = intstatus >> ( 41 & 31 );

    int_disable( );
    total_int++;
    //intstatus = read_intstatus();
    gci_gciintstatus = 0xFFFFFFFF;
    /* Clear event interrupt */
    //gci_gcievent0 = 0xFFFFFFFF;
    //gci_gcievent1 = 0xFFFFFFFF;
    gci_gcievent1 = temp;

    /* clear ECSI MASK bit */
    intclrpending0 = 0xffffffff;
    intclrpending1 = 0xffffffff;
    ( *(uint32_t *) 0x314140 ) = 0xFFFFFFFF;

    if ( intstatus == 3 )
    {
        wwd_thread_notify_irq( );
        gci_cnt++;
    }
    else
        int_enab( );
    //WPRINT_APP_INFO(("sdio: %d\n", intstatus));
    // TODO: clear interrupt
}

WWD_RTOS_MAP_ISR( sdio_irq, WL2APPS_irq )
