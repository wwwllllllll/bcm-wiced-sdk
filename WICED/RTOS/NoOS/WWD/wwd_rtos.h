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
 *  Definitions of the Wiced RTOS abstraction layer for the special case
 *  of having no RTOS
 *
 */

#ifndef INCLUDED_WWD_RTOS_H_
#define INCLUDED_WWD_RTOS_H_

#include "platform_isr.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Define System Tick interrupt handler needed by NoOS abstraction layer. This
 * defines is used by the vector table.
 */
#define SYSTICK_irq NoOS_systick_irq

/* Use this macro to define an RTOS-aware interrupt handler where RTOS
 * primitives can be safely accessed
 *
 * @usage:
 * WWD_RTOS_DEFINE_ISR( my_irq_handler )
 * {
 *     // Do something here
 * }
 */
#if defined( __GNUC__ )

#define WWD_RTOS_DEFINE_ISR( function ) \
        void function( void ); \
        __attribute__(( interrupt, used, section(IRQ_SECTION) )) void function( void )

#elif defined ( __IAR_SYSTEMS_ICC__ )

#define WWD_RTOS_DEFINE_ISR( function ) \
        void function( void ); \
        __irq __root void function( void )
#else

#define WWD_RTOS_DEFINE_ISR( function ) \
        void function( void )

#endif


/* Macro for mapping a function defined using WWD_RTOS_DEFINE_ISR
 * to an interrupt handler declared in
 * <Wiced-SDK>/WICED/platform/<Arch>/<Family>/platform_irq_handlers.h
 *
 * @usage:
 * WWD_RTOS_MAP_ISR( my_irq, USART1_irq )
 */
#if defined( __GNUC__ )

#define WWD_RTOS_MAP_ISR( function, isr ) \
        extern void isr( void ); \
        __attribute__(( alias( #function ))) void isr ( void );

#elif defined ( __IAR_SYSTEMS_ICC__ )

#define WWD_RTOS_MAP_ISR( function, isr ) \
        extern void isr( void ); \
        _Pragma( TO_STRING( weak isr=function ) )
#else

#define WWD_RTOS_MAP_ISR( function, isr )

#endif


#define RTOS_HIGHER_PRIORTIY_THAN(x)     (x)
#define RTOS_LOWER_PRIORTIY_THAN(x)      (x)
#define RTOS_LOWEST_PRIORITY             (0)
#define RTOS_HIGHEST_PRIORITY            (0)
#define RTOS_DEFAULT_THREAD_PRIORITY     (0)

#define RTOS_USE_DYNAMIC_THREAD_STACK
#define WWD_THREAD_STACK_SIZE            (544)

#define malloc_get_current_thread( ) (NULL)
typedef void* malloc_thread_handle;
#define wiced_thread_to_malloc_thread( thread ) (thread)

/*
 * The number of system ticks per second
 */
#define SYSTICK_FREQUENCY  (1000)

/******************************************************
 *             Structures
 ******************************************************/

typedef volatile unsigned char   host_semaphore_type_t;  /** NoOS definition of a semaphore */
typedef volatile unsigned char   host_thread_type_t;     /** NoOS definition of a thread handle - Would be declared void but that is not allowed. */
typedef volatile unsigned char   host_queue_type_t;      /** NoOS definition of a message queue */

/*@external@*/ extern void NoOS_setup_timing( void );
/*@external@*/ extern void NoOS_stop_timing( void );
/*@external@*/ extern void NoOS_systick_irq( void );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef INCLUDED_WWD_RTOS_H_ */
