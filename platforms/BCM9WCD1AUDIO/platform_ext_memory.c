/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "platform_ext_memory.h"
#include "platform_external_memory.h"
#include "stm32f4xx.h"

#define FSMC_BCR1           ((volatile uint32_t*)(0xA0000000))
#define FSMC_BTCR1          ((volatile uint32_t*)(0xA0000004))
#define FSMC_BWTR1          ((volatile uint32_t*)(0xA0000104))

/* register fields - BCR */
#define CBURSTRW_START      (19)
#define ASYNCWAIT_START     (15)
#define EXTMOD_START        (14)
#define WAITEN_START        (13)
#define WAITEN_START        (13)
#define WREN_START          (12)
#define WAITCFG_START       (11)
#define WRAPMOD_START       (10)
#define WAITPOL_START       (9)
#define BURSTEN_START       (8)
#define FACCEN_START      (6)
#define MWID_START        (4)
#define MTYP_START        (2)
#define MUXEN_START       (1)
#define MBKEN_START       (0)

/* register fields - BTR, general timing settings */
#define ACCMOD_START      (28)
#define DATLAT_START      (24)
#define CLKDIV_START      (20)
#define BUSTURN_START     (16)
#define DATAST_START      (8)
#define ADDHLD_START      (4)
#define ADDSET_START      (0)

/* register fields BWTR, timing settings for write transactions */
#define ACCMOD_WR_START   (28)
#define DATLAT_WR_START   (24)
#define CLKDIV_WR_START   (20)
#define BUSTURN_WR_START  (16)
#define DATAST_WR_START   (8)
#define ADDHLD_WR_START   (4)
#define ADDSET_WR_START   (0)


static const stm32f4xx_platform_nor_sram_t IS66WVE4M16BLL_settings =
{
        /* timing value are given in hclk cycles. Must be given in nanoseconds in future */
        .extended_mode              = 0,
        .address_set_duration       = 2,
        .data_phase_duration        = 6,
        .rd_burst= 0,              /* burst mode is not supported */
        .wr_burst= 0,              /* burst mode is not supported */
        .async_wait_enabled         = 0,   /* wait pin is not connected */
        .sync_wait_enabled          = 0,    /* wait pin is not connected */
        //.wait_polarity - disregard
        .access_mode                = 0,          /* disregard, we are working in  */
        .data_latency               = 0,           /* disregard */
        //.clock_freq - disregard, we are in asynchronous mode
        .bus_turnaround             = 0,       /* at least 3ns - tlzwe */
        .address_hold_duration      = 1,  /* set to minimum */
        .extended_mode              = 0,        /* not used, writing timings and reading timings are almost identical */
        .wr_address_set_duration    = 0,        /* think */
        .wr_address_hold_duration   = 0,
        .wr_data_phase_duration     = 0,
        .wr_datast                  = 0,
        .wr_bus_turnaround          = 0,
        .wr_access_mode             = 0,
        .wr_data_latency            = 0,
};

static const wiced_sram_device_t IS66WVE4M16BLL =
{
        .name                       = "IS66WVE4M16BLL",
        .size                       = 8 * 1024 * 1024,
        .chip_select                = 0, /* Memory chip select is physically connected to NE1 */
        .bus_data_width             = 16,
        .byte_lane_select_enable    = 1,
        .async                      = 1,
        .mux_addr_data              = 0,
        .private_data               = &IS66WVE4M16BLL_settings,
};

typedef struct
{
    volatile uint32_t* bcr;
    volatile uint32_t* btcr;
    volatile uint32_t* bwtr;
} stm32f4_sram_nor_control_t;

static const stm32f4_sram_nor_control_t sram_control_registers =
{
    /* NE1 chip select control registers */
    .bcr    = FSMC_BCR1,
    .btcr   = FSMC_BTCR1,
    .bwtr   = FSMC_BWTR1,
};


typedef struct
{
    GPIO_TypeDef* port;
    uint32_t      mode_mask;
    uint32_t      otype_mask;
    uint32_t      speed_mask;
    uint32_t      pupd_mask;
    uint32_t      af_mask;
} sram_gpio_setting_t;

static const sram_gpio_setting_t sram_gpio_settings[] =
{
    {
        .port       = GPIOD,
        .mode_mask  = (GPIO_Mode_AF << (0 *2)) | (GPIO_Mode_AF << (1 *2)) | (GPIO_Mode_AF << (3 *2)) | (GPIO_Mode_AF << (4 *2)) | (GPIO_Mode_AF << (5 *2)) | (GPIO_Mode_AF << (7 *2)) | (GPIO_Mode_AF << (8 *2)) | (GPIO_Mode_AF << (9 *2)) | (GPIO_Mode_AF << (10*2)) | (GPIO_Mode_AF << (11*2)) | (GPIO_Mode_AF << (12*2)) | (GPIO_Mode_AF << (13*2)) | (GPIO_Mode_AF << (14*2)) | (GPIO_Mode_AF << (15*2)),
        .otype_mask = (GPIO_OType_PP << 0 ) | (GPIO_OType_PP << 1 ) | (GPIO_OType_PP << 3 ) | (GPIO_OType_PP << 4 ) | (GPIO_OType_PP << 5 ) | (GPIO_OType_PP << 7 ) | (GPIO_OType_PP << 8 ) | (GPIO_OType_PP << 9 ) | (GPIO_OType_PP << 10) | (GPIO_OType_PP << 11) | (GPIO_OType_PP << 12) | (GPIO_OType_PP << 13) | (GPIO_OType_PP << 14) | (GPIO_OType_PP << 15),
        .speed_mask = (GPIO_Speed_100MHz << (0 *2)) | (GPIO_Speed_100MHz << (1 *2)) | (GPIO_Speed_100MHz << (3 *2)) | (GPIO_Speed_100MHz << (4 *2)) | (GPIO_Speed_100MHz << (5 *2)) | (GPIO_Speed_100MHz << (7 *2)) | (GPIO_Speed_100MHz << (8 *2)) | (GPIO_Speed_100MHz << (9 *2)) | (GPIO_Speed_100MHz << (10*2)) | (GPIO_Speed_100MHz << (11*2)) | (GPIO_Speed_100MHz << (12*2)) | (GPIO_Speed_100MHz << (13*2)) | (GPIO_Speed_100MHz << (14*2)) | (GPIO_Speed_100MHz << (15*2)),
        .pupd_mask  = 0,
    },
    {
        .port       = GPIOE,
        .mode_mask  = (GPIO_Mode_AF << (0 *2)) | (GPIO_Mode_AF << (1 *2)) | (GPIO_Mode_AF << (2 *2)) | (GPIO_Mode_AF << (3 *2)) | (GPIO_Mode_AF << (4 *2)) | (GPIO_Mode_AF << (5 *2)) | (GPIO_Mode_AF << (6 *2)) | (GPIO_Mode_AF << (7 *2)) | (GPIO_Mode_AF << (8 *2)) | (GPIO_Mode_AF << (9 *2)) | (GPIO_Mode_AF << (10*2)) | (GPIO_Mode_AF << (11*2)) | (GPIO_Mode_AF << (12*2)) | (GPIO_Mode_AF << (13*2)) | (GPIO_Mode_AF << (14*2)) | (GPIO_Mode_AF << (15*2)),
        .otype_mask = (GPIO_OType_PP << 0 ) | (GPIO_OType_PP << 1 ) | (GPIO_OType_PP << 2 ) | (GPIO_OType_PP << 3 ) | (GPIO_OType_PP << 4 ) | (GPIO_OType_PP << 5 ) | (GPIO_OType_PP << 6 ) | (GPIO_OType_PP << 7 ) | (GPIO_OType_PP << 8 ) | (GPIO_OType_PP << 9 ) | (GPIO_OType_PP << 10) | (GPIO_OType_PP << 11) | (GPIO_OType_PP << 12) | (GPIO_OType_PP << 13) | (GPIO_OType_PP << 14) | (GPIO_OType_PP << 15),
        .speed_mask = (GPIO_Speed_100MHz << (0 *2)) | (GPIO_Speed_100MHz << (1 *2)) | (GPIO_Speed_100MHz << (2 *2)) | (GPIO_Speed_100MHz << (3 *2)) | (GPIO_Speed_100MHz << (4 *2)) | (GPIO_Speed_100MHz << (5 *2)) | (GPIO_Speed_100MHz << (6 *2)) | (GPIO_Speed_100MHz << (7 *2)) | (GPIO_Speed_100MHz << (8 *2)) | (GPIO_Speed_100MHz << (9 *2)) | (GPIO_Speed_100MHz << (10*2)) | (GPIO_Speed_100MHz << (11*2)) | (GPIO_Speed_100MHz << (12*2)) | (GPIO_Speed_100MHz << (13*2)) | (GPIO_Speed_100MHz << (14*2)) | (GPIO_Speed_100MHz << (15*2)),
        .pupd_mask  = 0,
    },
    {
        .port       = GPIOF,
        .mode_mask  = (GPIO_Mode_AF << (0 *2)) | (GPIO_Mode_AF << (1 *2)) | (GPIO_Mode_AF << (2 *2)) | (GPIO_Mode_AF << (3 *2)) | (GPIO_Mode_AF << (4 *2)) | (GPIO_Mode_AF << (5 *2)) | (GPIO_Mode_AF << (12*2)) | (GPIO_Mode_AF << (13*2)) | (GPIO_Mode_AF << (14*2)) | (GPIO_Mode_AF << (15*2)),
        .otype_mask = (GPIO_OType_PP << 0 ) | (GPIO_OType_PP << 1 ) | (GPIO_OType_PP << 2 ) | (GPIO_OType_PP << 3 ) | (GPIO_OType_PP << 4 ) | (GPIO_OType_PP << 5 ) | (GPIO_OType_PP << 12) | (GPIO_OType_PP << 13) | (GPIO_OType_PP << 14) | (GPIO_OType_PP << 15),
        .speed_mask = (GPIO_Speed_100MHz << (0 *2)) | (GPIO_Speed_100MHz << (1 *2)) | (GPIO_Speed_100MHz << (2 *2)) | (GPIO_Speed_100MHz << (3 *2)) | (GPIO_Speed_100MHz << (4 *2)) | (GPIO_Speed_100MHz << (5 *2)) | (GPIO_Speed_100MHz << (12*2)) | (GPIO_Speed_100MHz << (13*2)) | (GPIO_Speed_100MHz << (14*2)) | (GPIO_Speed_100MHz << (15*2)),
        .pupd_mask  = 0,
    },
    {
        .port       = GPIOG,
        .mode_mask  = (GPIO_Mode_AF << (0 *2)) | (GPIO_Mode_AF << (1 *2)) | (GPIO_Mode_AF << (2 *2)) | (GPIO_Mode_AF << (3 *2)) | (GPIO_Mode_AF << (4 *2)) | (GPIO_Mode_AF << (5 *2)) | (GPIO_Mode_AF << (13*2)) | (GPIO_Mode_AF << (14*2)),
        .otype_mask = (GPIO_OType_PP <<  0 ) | (GPIO_OType_PP <<  1 ) | (GPIO_OType_PP <<  2 ) | (GPIO_OType_PP <<  3 ) | (GPIO_OType_PP <<  4 ) | (GPIO_OType_PP <<  5 ) | (GPIO_OType_PP <<  13) | (GPIO_OType_PP <<  14),
        .speed_mask = (GPIO_Speed_100MHz << (0 *2)) | (GPIO_Speed_100MHz << (1 *2)) | (GPIO_Speed_100MHz << (2 *2)) | (GPIO_Speed_100MHz << (3 *2)) | (GPIO_Speed_100MHz << (4 *2)) | (GPIO_Speed_100MHz << (5 *2)) | (GPIO_Speed_100MHz << (13*2)) | (GPIO_Speed_100MHz << (14*2)),
        .pupd_mask  = 0,
    }
};


extern int f4_sram_timings_init ( const wiced_sram_device_t* sram, const stm32f4xx_platform_nor_sram_t* settings );
static int f4_sram_pins_init( const wiced_sram_device_t* sram, const stm32f4xx_platform_nor_sram_t* settings );

void platform_init_memory( void )
{
    /* Enable FSMC clock */
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

    /* Set the bus data width */
    *( sram_control_registers.bcr ) &= ( uint32_t )( ~( 0x03 << MWID_START ) );
    *( sram_control_registers.bcr ) |= ( uint32_t )( 1 << MWID_START );

    /* Enable writes on this memory device */
    *(sram_control_registers.bcr) |= ( uint32_t )( 1 << WREN_START );

    /* Clear address data muxing */
    *(sram_control_registers.bcr) &= ( uint32_t )( ~( 1 << MUXEN_START ) );

    f4_sram_timings_init( &IS66WVE4M16BLL, &IS66WVE4M16BLL_settings );
    f4_sram_pins_init( &IS66WVE4M16BLL, &IS66WVE4M16BLL_settings );

    /* enable this memory bank, nor a chip select( for every memory bank we can have 4 chip selects ) */
    /* But still remember that they are all shared between all 4 banks. Once used for one of the banks */
    /* chip select can not be used again */
    *(sram_control_registers.bcr) |= ( 1 << MBKEN_START );

    return;
}

static int f4_sram_pins_init( const wiced_sram_device_t* sram, const stm32f4xx_platform_nor_sram_t* settings )
{
    /* Enable clocks for associated gpios */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);

    GPIO_PinAFConfig(GPIOD, 7, GPIO_AF_FSMC);   /* one of chip select control signals NE1-NE4 to FSMC  */

    /* Configure gpios for FSMC alternate function */
    /* Connect all gpio lined to the FSMC */

    GPIO_PinAFConfig(GPIOF, 0, GPIO_AF_FSMC);   /* A0 connected to PF0 */
    GPIO_PinAFConfig(GPIOF, 1, GPIO_AF_FSMC);   /* A1 connected to PF1 */
    GPIO_PinAFConfig(GPIOF, 2, GPIO_AF_FSMC);   /* A2 connected to PF2 */
    GPIO_PinAFConfig(GPIOF, 3, GPIO_AF_FSMC);   /* A3 connented to PF3 */
    GPIO_PinAFConfig(GPIOF, 4, GPIO_AF_FSMC);   /* A4 connected to PF4 */
    GPIO_PinAFConfig(GPIOF, 5, GPIO_AF_FSMC);   /* A5 connected to PF5 */
    GPIO_PinAFConfig(GPIOF, 12, GPIO_AF_FSMC);  /* A6 connected to PF12 */
    GPIO_PinAFConfig(GPIOF, 13, GPIO_AF_FSMC);  /* A7 connected to PF13 */
    GPIO_PinAFConfig(GPIOF, 14, GPIO_AF_FSMC);  /* A8 connected to PF14 */
    GPIO_PinAFConfig(GPIOF, 15, GPIO_AF_FSMC);  /* A9 connected to PF15 */
    GPIO_PinAFConfig(GPIOG, 0, GPIO_AF_FSMC);   /* A10 connected to PG0 */
    GPIO_PinAFConfig(GPIOG, 1, GPIO_AF_FSMC);   /* A11 connected to PG1 */
    GPIO_PinAFConfig(GPIOG, 2, GPIO_AF_FSMC);   /* A12 connected to PG2 */
    GPIO_PinAFConfig(GPIOG, 3, GPIO_AF_FSMC);   /* A13 connected to PG3 */
    GPIO_PinAFConfig(GPIOG, 4, GPIO_AF_FSMC);   /* A14 connected to PG4 */
    GPIO_PinAFConfig(GPIOG, 5, GPIO_AF_FSMC);   /* A15 connected to PG5 */

    GPIO_PinAFConfig(GPIOD, 11, GPIO_AF_FSMC);      /* A16 connected to PD11 */
    GPIO_PinAFConfig(GPIOD, 12, GPIO_AF_FSMC);      /* A17 connected to PD12 */
    GPIO_PinAFConfig(GPIOD, 13, GPIO_AF_FSMC);      /* A18 connected to PD13 */
    GPIO_PinAFConfig(GPIOE, 3, GPIO_AF_FSMC);       /* A19 connected to PE3  */
    GPIO_PinAFConfig(GPIOE, 4, GPIO_AF_FSMC);       /* A20 connected to PE4 */
    GPIO_PinAFConfig(GPIOE, 5, GPIO_AF_FSMC);       /* A21 connected to PE5 */

    GPIO_PinAFConfig(GPIOE, 6, GPIO_AF_FSMC);       /* A22 connected to PE6 */

    GPIO_PinAFConfig(GPIOE, 2, GPIO_AF_FSMC);       /* A23 connected to PE2 */
    GPIO_PinAFConfig(GPIOG, 13, GPIO_AF_FSMC);      /* A24 connected to PG13 */
    GPIO_PinAFConfig(GPIOG, 14, GPIO_AF_FSMC);      /* A25 connected to PG14 */

    /* Data lines */
    GPIO_PinAFConfig(GPIOD, 14, GPIO_AF_FSMC);      /* D0 connected to PD14 */
    GPIO_PinAFConfig(GPIOD, 15, GPIO_AF_FSMC);      /* D1 connected to PD15 */
    GPIO_PinAFConfig(GPIOD, 0, GPIO_AF_FSMC);       /* D2 connected to PD0 */
    GPIO_PinAFConfig(GPIOD, 1, GPIO_AF_FSMC);       /* D3 connected to PD1 */
    GPIO_PinAFConfig(GPIOE, 7, GPIO_AF_FSMC);       /* D4 connected to PE7 */
    GPIO_PinAFConfig(GPIOE, 8, GPIO_AF_FSMC);       /* D5 connected to PE8 */
    GPIO_PinAFConfig(GPIOE, 9, GPIO_AF_FSMC);       /* D6 connected to PE9 */
    GPIO_PinAFConfig(GPIOE, 10, GPIO_AF_FSMC);      /* D7 connected to PE10 */

    /* Connect D7-D15 to FSMC is the data bus is 16 bit */
    GPIO_PinAFConfig(GPIOE, 11, GPIO_AF_FSMC);  /* D8 is connected to PE11 */
    GPIO_PinAFConfig(GPIOE, 12, GPIO_AF_FSMC);  /* D9 is connected to PE12 */
    GPIO_PinAFConfig(GPIOE, 13, GPIO_AF_FSMC);  /* D10 is connected to PE13 */
    GPIO_PinAFConfig(GPIOE, 14, GPIO_AF_FSMC);  /* D11 is connected to PE14 */
    GPIO_PinAFConfig(GPIOE, 15, GPIO_AF_FSMC);  /* D12 is connected to PE15 */
    GPIO_PinAFConfig(GPIOD, 8, GPIO_AF_FSMC);   /* D13 is connected to PD8 */
    GPIO_PinAFConfig(GPIOD, 9, GPIO_AF_FSMC);   /* D14 is connected to PD9 */
    GPIO_PinAFConfig(GPIOD, 10, GPIO_AF_FSMC);  /* D15 is connected to PD10 */

    /* control lines */
    /* NOE */
    GPIO_PinAFConfig(GPIOD, 4, GPIO_AF_FSMC); /* NOE is connected to PD4 */

    /* NWE */
    GPIO_PinAFConfig(GPIOD, 5, GPIO_AF_FSMC); /* NWE is connected to PD5 */

    /* NL(NADV) */
    /* NADV stays unconnected in SRAM memories */

    /* On BCM9WCD1AUDIO first spin board NWAIT is connected to the ZZ pin of the PSRAM memory */
    GPIO_InitTypeDef gpio_init_structure;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_structure.GPIO_Mode  = GPIO_Mode_OUT;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_PuPd  =  GPIO_PuPd_UP;
    gpio_init_structure.GPIO_Pin =  ( 1 << 6 );
    GPIO_Init( GPIOD, &gpio_init_structure );
    /* Set to high level, so memory never goes to power-save mode */
    GPIO_SetBits( GPIOD, ( 1 << 6 ) );

     /* NWAIT */

    /* NBL[1] and NBL[0] */
    GPIO_PinAFConfig(GPIOE, 1, GPIO_AF_FSMC); /* NBL[1] is connected to PE1 */
    GPIO_PinAFConfig(GPIOE, 0, GPIO_AF_FSMC); /* NBL[0] is connected to PE0 */

    /* CLK */
    GPIO_PinAFConfig(GPIOD, 3, GPIO_AF_FSMC); /* CLK is connected to PD3 */

    /* Initialise GPIO to operate in high speed Alternate function mode */
    int a;
    for (a = 0; a < 4; ++a)
    {
        sram_gpio_settings[a].port->MODER   |= sram_gpio_settings[a].mode_mask;
        sram_gpio_settings[a].port->OTYPER  |= sram_gpio_settings[a].otype_mask;
        sram_gpio_settings[a].port->OSPEEDR |= sram_gpio_settings[a].speed_mask;
        sram_gpio_settings[a].port->PUPDR   |= sram_gpio_settings[a].pupd_mask;
    }

    return 0;
}
