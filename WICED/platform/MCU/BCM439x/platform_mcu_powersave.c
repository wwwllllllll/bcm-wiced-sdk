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
 *
 */
#include <string.h> // For memcmp
#include "platform_cmsis.h"
#include "platform_init.h"
#include "platform_sleep.h"
#include "platform_config.h"
#include "platform_peripheral.h"
#include "platform_assert.h"
#include "platform_dct.h"
#include "wwd_constants.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"

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
 *               Function Declarations
 ******************************************************/

extern void pmu_SelectLpoClock( uint32_t *p_time_to_sleep ) __attribute__((long_call));

#define PMU_SLEEP_CR_CLEAR          ( 0x00 )
#define PMU_SLEEP_CR_ENABLE         ( 0x01 )
#define PMU_SLEEP_CR_POWER_OFF      ( 0x02 )
#define PMU_SLEEP_CR_EARLY_MASK     ( 0x04 )
#define PMU_SLEEP_CR_FOREVER        ( 0x08 )
#define PMU_SR_RPU                  ( 0x2000 )
#define cr_rpu_sleep_en_adr         0x0032041c                                          // pmu_adr_base + 0x0000041c
#define cr_rpu_sleep_en             (*(volatile unsigned int *)cr_rpu_sleep_en_adr)
#define cr_rpu_warmup_cnt_adr       0x00320420                                          // pmu_adr_base + 0x00000420
#define cr_rpu_warmup_cnt           (*(volatile unsigned int *)cr_rpu_warmup_cnt_adr)
#define cr_rpu_pllwarm_cnt_adr      0x00320424                                          // pmu_adr_base + 0x00000424
#define cr_rpu_pllwarm_cnt          (*(volatile unsigned int *)cr_rpu_pllwarm_cnt_adr)
#define cr_rpu_sleep_cnt_adr        0x00320428                                          // pmu_adr_base + 0x00000428
#define cr_rpu_sleep_cnt            (*(volatile unsigned int *)cr_rpu_sleep_cnt_adr)
#define sr_pmu_en_adr               0x003200c4                                          // pmu_adr_base + 0x000000c4
#define sr_pmu_en                   (*(volatile unsigned int *)sr_pmu_en_adr)
#define sr_pmu_status_adr           0x003200c0                                          // pmu_adr_base + 0x000000c0
#define sr_pmu_status               (*(volatile unsigned int *)sr_pmu_status_adr)
#define sr_rpu_sleep_remain0_adr    0x0032047c                                          // pmu_adr_base + 0x0000047c
#define sr_rpu_sleep_remain0        (*(volatile unsigned int *)sr_rpu_sleep_remain0_adr)

/******************************************************
 *               Variables Definitions
 ******************************************************/

#ifndef WICED_DISABLE_MCU_POWERSAVE
static wiced_bool_t wake_up_interrupt_triggered  = WICED_FALSE;
static int          bcm439x_clock_needed_counter = 0;
static char         bcm439x_sleep_flag           = 0;
#endif /* ifndef WICED_DISABLE_MCU_POWERSAVE */

/******************************************************
 *               Function Definitions
 ******************************************************/

/******************************************************
 *                 DCT Functions
 ******************************************************/

/******************************************************
 *                 DCT Functions
 ******************************************************/


/******************************************************
 *            Interrupt Service Routines
 ******************************************************/

platform_result_t platform_mcu_powersave_disable( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    WICED_DISABLE_INTERRUPTS();
    if ( bcm439x_clock_needed_counter <= 0 )
    {
//        SCB->SCR &= (~((unsigned long)SCB_SCR_SLEEPDEEP_Msk));
        bcm439x_sleep_flag = 0;
        bcm439x_clock_needed_counter = 0;
    }
    bcm439x_clock_needed_counter++;
    WICED_ENABLE_INTERRUPTS();
    return PLATFORM_SUCCESS;
#else
    return PLATFORM_UNSUPPORTED;
#endif /* ifndef WICED_DISABLE_MCU_POWERSAVE  */
}

platform_result_t platform_mcu_powersave_enable( void )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    WICED_DISABLE_INTERRUPTS();
    bcm439x_clock_needed_counter--;
    if ( bcm439x_clock_needed_counter <= 0 )
    {
//        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        bcm439x_sleep_flag = 1;
        bcm439x_clock_needed_counter = 0;
    }
    WICED_ENABLE_INTERRUPTS();
    return PLATFORM_SUCCESS;
#else
    return PLATFORM_UNSUPPORTED;
#endif /* ifndef WICED_DISABLE_MCU_POWERSAVE  */
}

void platform_idle_hook( void )
{
//    __asm("wfi");
}

void host_platform_get_mac_address( wiced_mac_t* mac )
{
#if 0 //ifndef WICED_DISABLE_BOOTLOADER
    wiced_mac_t* temp_mac;
    wiced_dct_read_lock( (void**)&temp_mac, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, OFFSETOF(platform_dct_wifi_config_t, mac_address), sizeof(wiced_mac_t) );
    memcpy(mac->octet, temp_mac, sizeof(wiced_mac_t));
    wiced_dct_read_unlock( temp_mac, WICED_FALSE );
#else
    UNUSED_PARAMETER( mac );
#endif
}

#ifndef WICED_DISABLE_MCU_POWERSAVE

void platform_mcu_powersave_exit_notify( void )
{
    wake_up_interrupt_triggered = WICED_TRUE;
}

static unsigned long stop_mode_power_down_hook( unsigned long delay_ms )
{
    unsigned long scale_factor = 0;
    UNUSED_PARAMETER(delay_ms);
    UNUSED_PARAMETER(scale_factor);

   /* pick up the appropriate prescaler for a requested delay */
//    select_wut_prescaler_calculate_wakeup_time(&rtc_timeout_start_time, delay_ms, &scale_factor );

    if ( bcm439x_sleep_flag != 0 )
    {
        WICED_DISABLE_INTERRUPTS();

        SysTick->CTRL &= (~(SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk)); /* systick IRQ off */

        /* TODO: May need to set warmup times, interrupt sources, save interrupt mask */
        /* TODO: May need to call pmu_pdsPreSleepControlSettings() and    pmu_startLpoCalibration */

#define SLEEP_RATIO (1000)
        /* TODO: ensure this multiplication produces an accurate time */
        delay_ms *= SLEEP_RATIO;
        pmu_SelectLpoClock( &delay_ms ); // this is done internaly by Apps for Apps clock sleeping
        cr_rpu_sleep_cnt = delay_ms;

        cr_rpu_sleep_en = /*PMU_SLEEP_CR_EARLY_MASK | */  PMU_SLEEP_CR_ENABLE;

        /* This code will be running with BASEPRI register value set to 0, the main intention behind that is that */
        /* all interrupts must be allowed to wake the CPU from the power-down mode */
        /* the PRIMASK is set to 1( see WICED_DISABLE_INTERRUPTS()), thus we disable all interrupts before entering the power-down mode */
        /* This may sound contradictory, however according to the ARM CM3 documentation power-management unit */
        /* takes into account only the contents of the BASEPRI register and it is an external from the CPU core unit */
        /* PRIMASK register value doesn't affect its operation. */
        /* So, if the interrupt has been triggered just before the wfi instruction */
        /* it remains pending and wfi instruction will be treated as a nop  */
        __asm("wfi");

        cr_rpu_sleep_en = PMU_SLEEP_CR_CLEAR;

        /* After CPU exits powerdown mode, the processer will not execute the interrupt handler(PRIMASK is set to 1) */
        /* Disable and clear PMU interrupt */
        sr_pmu_en     &= ~(PMU_SR_RPU);
        sr_pmu_status &=  (PMU_SR_RPU);

        /* Get the remaining sleep time */
        unsigned int sleepRemain = sr_rpu_sleep_remain0;


        /* Initialise the clocks again */
        init_4390_after_global_init( );

        /* Get the elapsed sleep time */
        uint32_t sleepTime = (cr_rpu_sleep_cnt - sleepRemain) / SLEEP_RATIO;


        /* TODO: Probably need to call pmu_ConvertLPOCyclestoTime() to convert sleep amount */

        /* Enable CPU ticks */
        SysTick->CTRL |= (SysTick_CTRL_TICKINT_Msk|SysTick_CTRL_ENABLE_Msk);

        /* as soon as interrupts are enabled, we will go and execute the interrupt handler */
        /* which triggered a wake up event */
        WICED_ENABLE_INTERRUPTS();
        wake_up_interrupt_triggered = WICED_FALSE;
        return sleepTime;
    }
    else
    {
//        WICED_DISABLE_INTERRUPTS();
        WICED_ENABLE_INTERRUPTS();
//        __asm("wfi");


        /* Note: We return 0 ticks passed because system tick is still going when wfi instruction gets executed */
        return 0;
    }
}

#else /* WICED_DISABLE_MCU_POWERSAVE */

static unsigned long idle_power_down_hook( unsigned long delay_ms  )
{
    UNUSED_PARAMETER( delay_ms );
    WICED_ENABLE_INTERRUPTS();
//    __asm("wfi");
    return 0;
}

#endif /* WICED_DISABLE_MCU_POWERSAVE */

unsigned long platform_power_down_hook( unsigned long delay_ms )
{
#ifndef WICED_DISABLE_MCU_POWERSAVE
    return stop_mode_power_down_hook( delay_ms );
#else
    return idle_power_down_hook( delay_ms );
#endif
}
