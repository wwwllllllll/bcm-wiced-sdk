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
 * Defines BCM439x vector table
 */
#include <stdint.h>
#include "platform_cmsis.h"
#include "platform_assert.h"
#include "platform_constants.h"
#include "platform_isr.h"
#include "platform_isr_interface.h"
#include "wwd_rtos.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef SVC_irq
#define SVC_irq UnhandledInterrupt
#endif

#ifndef PENDSV_irq
#define PENDSV_irq UnhandledInterrupt
#endif

#ifndef SYSTICK_irq
#define SYSTICK_irq UnhandledInterrupt
#endif

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

extern void UnhandledInterrupt( void );
extern void reset_handler     ( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Pointer to stack location */
extern void* link_stack_end;

uint32_t interrupt_vector_table[] =
{
    (uint32_t)&link_stack_end    ,
    (uint32_t)reset_handler      ,
    (uint32_t)NMIException       ,
    (uint32_t)HardFaultException ,
    (uint32_t)MemManageException ,
    (uint32_t)BusFaultException  ,
    (uint32_t)UsageFaultException,
    (uint32_t)0                  ,
    (uint32_t)0                  ,
    (uint32_t)0                  ,
    (uint32_t)0                  ,
    (uint32_t)SVC_irq            ,
    (uint32_t)DebugMonitor       ,
    (uint32_t)0                  ,
    (uint32_t)PENDSV_irq         ,
    (uint32_t)SYSTICK_irq        ,
    (uint32_t)Reserved016_irq    ,
    (uint32_t)Reserved017_irq    ,
    (uint32_t)Reserved018_irq    ,
    (uint32_t)Reserved019_irq    ,
    (uint32_t)Reserved020_irq    ,
    (uint32_t)Reserved021_irq    ,
    (uint32_t)Reserved022_irq    ,
    (uint32_t)Reserved023_irq    ,
    (uint32_t)Reserved024_irq    ,
    (uint32_t)Reserved025_irq    ,
    (uint32_t)Reserved026_irq    ,
    (uint32_t)Reserved027_irq    ,
    (uint32_t)Reserved028_irq    ,
    (uint32_t)Reserved029_irq    ,
    (uint32_t)Reserved030_irq    ,
    (uint32_t)PTU1_irq           ,
    (uint32_t)DmaDoneInt_irq     ,
    (uint32_t)Reserved033_irq    ,
    (uint32_t)Reserved034_irq    ,
    (uint32_t)WAKEUP_irq         ,
    (uint32_t)GPIOA_BANK0_irq    ,
    (uint32_t)Reserved037_irq    ,
    (uint32_t)Reserved038_irq    ,
    (uint32_t)Reserved039_irq    ,
    (uint32_t)Reserved040_irq    ,
    (uint32_t)GPIOA_BANK1_irq    ,
    (uint32_t)Reserved042_irq    ,
    (uint32_t)Reserved043_irq    ,
    (uint32_t)Reserved044_irq    ,
    (uint32_t)Reserved045_irq    ,
    (uint32_t)Reserved046_irq    ,
    (uint32_t)Reserved047_irq    ,
    (uint32_t)Reserved048_irq    ,
    (uint32_t)Reserved049_irq    ,
    (uint32_t)Reserved050_irq    ,
    (uint32_t)Reserved051_irq    ,
    (uint32_t)Reserved052_irq    ,
    (uint32_t)Reserved053_irq    ,
    (uint32_t)Reserved054_irq    ,
    (uint32_t)Reserved055_irq    ,
    (uint32_t)Reserved056_irq    ,
    (uint32_t)Reserved057_irq    ,
    (uint32_t)Reserved058_irq    ,
    (uint32_t)Reserved059_irq    ,
    (uint32_t)Reserved060_irq    ,
    (uint32_t)WL2APPS_irq        ,
    (uint32_t)WlanReady_irq      ,
    (uint32_t)Reserved063_irq    ,
    (uint32_t)PTU2_irq           ,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

