/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "wiced_utilities.h"
#include "wiced_bt_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/* Macro for checking if bus is ready */
#define BT_BUS_IS_READY( ) \
do \
{ \
    if ( bt_bus_is_ready( ) == WICED_FALSE ) \
    { \
        wiced_assert( "bus not ready", 0!=0 ); \
        return WICED_ERROR; \
    } \
}while ( 0 )

/* Macro for waiting until bus is ready */
#define BT_BUS_WAIT_UNTIL_READY( ) \
do \
{ \
    while ( bt_bus_is_ready( ) == WICED_FALSE ) \
    { \
        wiced_rtos_delay_milliseconds( 10 ); \
    } \
} while ( 0 )

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

wiced_result_t bt_bus_init( void );

wiced_result_t bt_bus_deinit( void );

wiced_result_t bt_bus_transmit( const uint8_t* data_out, uint32_t size );

wiced_result_t bt_bus_receive( uint8_t* data_in, uint32_t size, uint32_t timeout_ms );

wiced_bool_t   bt_bus_is_ready( void );

wiced_bool_t   bt_bus_is_on( void );

#ifdef __cplusplus
} /* extern "C" */
#endif
