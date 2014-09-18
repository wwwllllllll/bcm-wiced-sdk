/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include "platform_init.h"
#include "platform_cmsis.h"
#include "FreeRTOSConfig.h"

/** @file
 *
 */

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

/******************************************************
 *               Function Definitions
 ******************************************************/

/* FreeRTOS interrupt priority setup. Called by platform_init_mcu_infrastructure() */
void platform_init_rtos_irq_priorities( void )
{
    /* Setup the system handler priorities */
    NVIC_SetPriority( MemoryManagement_IRQn, 0                                    ); /* Mem Manage system handler priority    */
    NVIC_SetPriority( BusFault_IRQn        , 0                                    ); /* Bus Fault system handler priority     */
    NVIC_SetPriority( UsageFault_IRQn      , 0                                    ); /* Usage Fault system handler priority   */
    NVIC_SetPriority( SVCall_IRQn          , 0                                    ); /* SVCall system handler priority        */
    NVIC_SetPriority( DebugMonitor_IRQn    , 0                                    ); /* Debug Monitor system handler priority */
    NVIC_SetPriority( PendSV_IRQn          , configKERNEL_INTERRUPT_PRIORITY >> 4 ); /* PendSV system handler priority        */
    NVIC_SetPriority( SysTick_IRQn         , configKERNEL_INTERRUPT_PRIORITY >> 4 ); /* SysTick system handler priority       */
}
