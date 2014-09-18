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
#include "wiced.h"
#include "wiced_platform.h"
#include "wiced_rtos.h"
#include "wiced_audio.h"
#include "wm8533_dac.h"
#include "platform_i2s.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define MIN_WM8533_DB_LEVEL    (-100.00)/* in decibels */
#define MAX_WM8533_DB_LEVEL    (+12.00) /* in decibels */
#define GAIN_ADJUSTMENT_STEP   ( 0.25 ) /* in decibels */

#define GAIN_REG_VALUE_ON_ZERO_DB (0x190)

#define I2S_STANDARD_PHILLIPS

#ifdef I2S_STANDARD_PHILLIPS
 #define  CODEC_STANDARD                AIF_FMT_I2S
#elif defined(I2S_STANDARD_MSB)
 #define  CODEC_STANDARD                AIF_FMT_MSB
#elif defined(I2S_STANDARD_LSB)
 #define  CODEC_STANDARD                AIF_FMT_LSB
#else
 #error "Error: No audio communication standard selected !"
#endif

/******************************************************
 *                    Constants
 ******************************************************/

/* Register fields */
#define DIGITAL_VOLUME_UPDATE_FIELD_SHIFT   (9)
#define DIGITAL_VOLUME_UPDATE_FIELD_BITS    (1)
#define DIGITAL_VOLUME_DB_VALUE             (0)
#define VOLUME_UP_RAMP_FIELD_SHIFT          (1)
#define VOLUME_UP_RAMP_FIELD_BITS           (1)
#define VOLUME_DOWN_RAMP_FIELD              (0)
#define VOLUME_DOWN_RAMP_FIELD_BITS         (1)
#define DAC_ZERO_CROSS_FIELD_SHIFT          (4)
#define DAC_ZERO_CROSS_FIELD_BITS           (1)
#define SYS_ENA_FIELD_SHIFT                 (0)
#define SYS_ENA_BITS                        (2)
#define SYS_ENA_POWER_OFF                   (0)
#define SYS_ENA_POWER_DOWN                  (1)
#define SYS_ENA_POWER_UP_WITH_MUTE          (2)
#define SYS_ENA_POWER_UP                    (3)
#define AIF_WL_SHIFT                        (3)
#define AIF_WL_BITS                         (2)
#define AIF_WL_16_BITS                      (0)
#define AIF_WL_20_BITS                      (1)
#define AIF_WL_24_BITS                      (2)
#define AIF_WL_32_BITS                      (3)
#define AIF_FMT_SHIFT                       (0)
#define AIF_FMT_BITS                        (2)
#define AIF_FMT_LSB                         (0)
#define AIF_FMT_MSB                         (1)
#define AIF_FMT_I2S                         (2)
#define AIF_FMT_DSP                         (3)
#define AIF_SR_SHIFT                        (0)
#define AIF_SR_BITS                         (3)
#define AIF_SR_AUTO                         (0)
#define AIF_SR_128FS                        (1)
#define AIF_SR_192FS                        (2)
#define AIF_SR_256FS                        (3)
#define AIF_SR_384FS                        (4)
#define AIF_SR_512FS                        (5)
#define AIF_SR_768FS                        (6)
#define AIF_SR_1152FS                       (7)
#define AIF_MSTR_SHIFT                      (7)
#define AIF_MSTR_BITS                       (1)
#define AIF_MSTR_SLAVE                      (0)
#define AIF_MSTR_MASTER                     (1)
#define AIF_BCLKDIV_SHIFT                   (3)
#define AIF_BCLKDIV_BITS                    (3)
#define AIF_BCLKDIV_MCLKDIV4                (0)
#define AIF_BCLKDIV_MCLKDIV8                (1)
#define AIF_BCLKDIV_32FS                    (2)
#define AIF_BCLKDIV_64FS                    (3)
#define AIF_BCLKDIV_128FS                   (4)

/* registers */
#define WM8533_REG_DAC_GAINL    (0x06)
#define WM8533_REG_DAC_GAINR    (0x07)
#define WM8533_REG_DAC_CTRL3    (0x05)
#define WM8533_REG_PSCTRL1      (0x02)
#define WM8533_REG_AIF_CTRL2    (0x04)
#define WM8533_REG_AIF_CTRL1    (0x03)
#define WM8533_REG_CHIP_ID      (0x00)
#define WM8533_REG_REVISION_ID  (0x01)

#define I2C_XFER_RETRY_COUNT    3
#define I2C_DMA_POLICY          WICED_FALSE

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    uint32_t mclk;
    uint32_t fs;
    uint8_t  sr;
} mclk_lrclk_map_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static wiced_result_t wm8533_reg_write( wm8533_device_data_t* wm8533, uint8_t reg, uint16_t value );
static wiced_result_t wm8533_reg_read ( wm8533_device_data_t* wm8533, uint8_t reg, uint16_t* value );

static wiced_result_t wm8533_configure ( void* driver_data, wiced_audio_config_t *config, uint32_t* mclk);
static wiced_result_t wm8533_init      ( void* driver_data );
static wiced_result_t wm8533_deinit    ( void* driver_data );
static wiced_result_t wm8533_start_play( void* driver_data );
static wiced_result_t wm8533_pause     ( void* driver_data );
static wiced_result_t wm8533_resume    ( void* driver_data );
static wiced_result_t wm8533_set_volume( void* driver_data, double decibels );
static wiced_result_t wm8533_audio_device_port ( void* device_data, wiced_i2s_t* port );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_audio_data_provider_t  give_more_samples_callback;
wiced_audio_device_interface_t      wm8533_interface;

/* WM8533 data-sheet table 11. */
static const mclk_lrclk_map_t mclk_lrclk_map[] =
{
    {  2048000,   8000, AIF_SR_256FS  },
    {  3072000,   8000, AIF_SR_384FS  },
    {  4096000,   8000, AIF_SR_512FS  },
    {  6144000,   8000, AIF_SR_768FS  },
    {  8192000,  32000, AIF_SR_256FS  },
    {  9216000,   8000, AIF_SR_1152FS },
    { 11289600,  44100, AIF_SR_256FS  },
    { 11289600,  88200, AIF_SR_128FS  },
    { 12288000,  32000, AIF_SR_384FS  },
    { 12288000,  48000, AIF_SR_256FS  },
    { 16384000,  32000, AIF_SR_512FS  },
    { 16934400,  44100, AIF_SR_384FS  },
    { 16934400,  88200, AIF_SR_192FS  },
    { 18432000,  48000, AIF_SR_384FS  },
    { 22579200,  44100, AIF_SR_512FS  },
    { 22579200,  88200, AIF_SR_256FS  },
    { 22579200, 176400, AIF_SR_128FS  },
    { 24576000,  32000, AIF_SR_768FS  },
    { 24576000,  48000, AIF_SR_512FS  },
    { 24576000, 192000, AIF_SR_128FS  },
    { 33868800,  44100, AIF_SR_768FS  },
    { 33868800,  88200, AIF_SR_384FS  },
    { 33868800, 176400, AIF_SR_192FS  },
    { 36864000,  32000, AIF_SR_1152FS },
    { 36864000,  48000, AIF_SR_768FS  },
    { 36864000, 192000, AIF_SR_192FS  },
};

/* Declare global audio device interface */
wiced_audio_device_interface_t wm8533_interface =
{
    .audio_device_start_streaming = NULL,
    .audio_device_init            = wm8533_init,
    .audio_device_deinit          = wm8533_deinit,
    .audio_device_configure       = wm8533_configure,
    .audio_device_port            = wm8533_audio_device_port,
    .audio_device_start_streaming = wm8533_start_play,
    .audio_device_pause           = wm8533_pause,
    .audio_device_resume          = wm8533_resume,
    .audio_device_set_volume      = wm8533_set_volume,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

static wiced_result_t i2c_reg_write(wiced_i2c_device_t *device, uint8_t reg, uint16_t value)
{
    wiced_result_t result;
    static wiced_i2c_message_t msg[1];
    struct i2c_wm8533_payload
    {
        uint8_t reg;
        uint8_t value_h;
        uint8_t value_l;
    };
    struct i2c_wm8533_payload payload;

    payload.reg = reg;
    payload.value_h = ( (value & 0xFF00) >> 8 )  ;
    payload.value_l = ( value & 0x00FF );

    result = wiced_i2c_init_tx_message(msg, &payload, 3, I2C_XFER_RETRY_COUNT, I2C_DMA_POLICY);
    if (WICED_SUCCESS == result)
    {
        result = wiced_i2c_transfer(device, msg, 1);
    }
    return result;
}

static wiced_result_t i2c_reg_read(wiced_i2c_device_t *device, uint8_t reg, uint16_t *value)
{
    wiced_result_t result;
    static wiced_i2c_message_t msg[1];
    uint8_t value_read[2];
    result = wiced_i2c_init_combined_message(msg, &reg, value_read, sizeof(reg), sizeof(value_read), I2C_XFER_RETRY_COUNT, I2C_DMA_POLICY);
    if (WICED_SUCCESS == result)
    {
        result = wiced_i2c_transfer(device, msg, 1);
        *value = ( value_read[0] << 8 ) + value_read[1];
    }
    return result;
}


static wiced_result_t wm8533_init ( void* driver_data )
{
    wm8533_device_data_t* wm8533 = ( wm8533_device_data_t* )driver_data;
    wiced_result_t result;

    /* Enable I2C clocks, init I2C peripheral. */
    result = wiced_i2c_init(wm8533->i2c_data);

    /* Initialize GPIOs. */
    if (WICED_SUCCESS == result)
    {
        if( wm8533->cifmode != WICED_GPIO_MAX )
        {
            result = wiced_gpio_init(wm8533->cifmode, OUTPUT_PUSH_PULL);
        }
    }
    if (WICED_SUCCESS == result)
    {
        if( wm8533->addr0 != WICED_GPIO_MAX )
        {
            result = wiced_gpio_init(wm8533->addr0, OUTPUT_PUSH_PULL);
        }
    }

    /* Configure DAC's control interface to I2C. */
    if (WICED_SUCCESS == result)
    {
        if( wm8533->cifmode != WICED_GPIO_MAX )
        {
            result = wiced_gpio_output_low(wm8533->cifmode);
        }
    }

        /* Configure DAC's I2C address to 0x1A (0011010b) */
    if (WICED_SUCCESS == result)
    {
        if( wm8533->addr0 != WICED_GPIO_MAX )
        {
            result = wiced_gpio_output_low(wm8533->addr0);
        }
    }
    if (WICED_SUCCESS == result)
    {
        uint16_t psctrl_value;
        psctrl_value = 0;
        psctrl_value = ( SYS_ENA_POWER_DOWN << SYS_ENA_FIELD_SHIFT );
        /* first stage, all analog block and digital signal blocks are powered off */
        /* the control interface is turned on only */
        result = wm8533_reg_write(wm8533, WM8533_REG_PSCTRL1, psctrl_value);
    }

    /* Reset CODEC registers. */
    /* When running as I2S master, registers don't appear to get
     * set correctly when the CODEC is being configured (seen when moving between
     * 44.1kHz to 48kHz FS).
     */
    if( WICED_SUCCESS == result )
    {
        result = wm8533_reg_write(wm8533, WM8533_REG_CHIP_ID, 0 );
    }

    /* set volume to approximately 8 db = 1b0, to both channels */
    /* where 190h is 0 db, and every 0.25db step is adding extra 1 */
    /* update immediately */
    if( WICED_SUCCESS == result )
        /* (1 << DIGITAL_VOLUME_UPDATE_FIELD_SHIFT) | ( uint16_t )( 0x1B0 << DIGITAL_VOLUME_DB_VALUE ) */
        result = wm8533_reg_write(wm8533, WM8533_REG_DAC_GAINL, (1 << DIGITAL_VOLUME_UPDATE_FIELD_SHIFT) | ( uint16_t )( 0x190 /*0x1B0*/ << DIGITAL_VOLUME_DB_VALUE ) );

    if( WICED_SUCCESS == result )
        result = wm8533_reg_write(wm8533, WM8533_REG_DAC_GAINR, (1 << DIGITAL_VOLUME_UPDATE_FIELD_SHIFT) | ( uint16_t )( 0x190 /*0x1B0*/ << DIGITAL_VOLUME_DB_VALUE ) );

    /* enable RAMP volume increase and decrease */
    if( WICED_SUCCESS == result )
        result = wm8533_reg_write(wm8533, WM8533_REG_DAC_CTRL3, (1 << VOLUME_DOWN_RAMP_FIELD) | ( 1 << VOLUME_UP_RAMP_FIELD_SHIFT ) );

    /* use zero cross */
    if( WICED_SUCCESS == result )
        result = wm8533_reg_write(wm8533, WM8533_REG_DAC_CTRL3, (1 << DAC_ZERO_CROSS_FIELD_SHIFT ) );

    /* Set codec to power up mode with soft mute on */
    if( WICED_SUCCESS == result )
    {
        uint16_t psctrl_value;
        psctrl_value = 0;
        psctrl_value = ( SYS_ENA_POWER_UP_WITH_MUTE << SYS_ENA_FIELD_SHIFT );
        /* Second stage of powering the device up, SYS_ENA= 0x02 */
        /* all blocks are enabled, the digital soft mute (100 db attenuation) is turned on */
        result = wm8533_reg_write(wm8533, WM8533_REG_PSCTRL1, psctrl_value);
    }

    /* Set codec to power up mode */
    if( WICED_SUCCESS == result )
    {
        uint16_t psctrl_value;
        psctrl_value = 0;
        psctrl_value = ( SYS_ENA_POWER_UP << SYS_ENA_FIELD_SHIFT );
        /* Third stage of powering the device up, SYS_ENA= 0x03 */
        /* digital soft mute (100 db attenuation) is released, the device blocks are all powered up */
        result = wm8533_reg_write(wm8533, WM8533_REG_PSCTRL1, psctrl_value);
    }

    if( WICED_SUCCESS == result )
    {
        uint16_t chip_id;
        uint16_t rev_id;
        result = wm8533_reg_read( wm8533, WM8533_REG_CHIP_ID, &chip_id );
        if (WICED_SUCCESS == result)
            result = wm8533_reg_read( wm8533, WM8533_REG_CHIP_ID, &rev_id );
#ifdef NOTYET
        if (WICED_SUCCESS == result)
            printf("Codec found %04X, %04X", chip_id, rev_id);
#endif /* NOTYET */
    }
    return result;
}

/* This function can only be called from the platform initialization routine */
wiced_result_t wm8533_device_register( wm8533_device_data_t* device_data, const char* name )
{
    wiced_result_t result;
    if( device_data == 0 )
    {
        return WICED_BADARG;
    }
    wm8533_interface.audio_device_driver_specific = device_data;
    wm8533_interface.name = name;

    /* Register a device to the audio device list and keep device data internally from this point */
    result = wiced_register_audio_device(name, &wm8533_interface);
    if( result != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wm8533_configure( void *driver_data, wiced_audio_config_t *config, uint32_t* mclk)
{
    wm8533_device_data_t* wm8533 = ( wm8533_device_data_t* )driver_data;
    wiced_result_t result;
    uint16_t ctrl1, ctrl2;
    size_t i;
    wiced_bool_t mclk_is_found= WICED_FALSE;
    uint mclk_lrck_ratio = 0;

    result = WICED_SUCCESS;

    /*
     * Configure platform.
     */

    if (WICED_SUCCESS == result)
    {
        if( *mclk == 0 )
        {
            switch (wm8533->fmt & WM8533_FMT_MASTER_MASK)
            {
                case WM8533_FMT_CCS_CFM:
                    /* Codec is a master and generates i2s bit and lr clocks */
                    /* Codec can be a master only when an external clock source is connected to it */
                    /* Find a configuration that agrees with the platform. */
                    for (i=0; i<sizeof(mclk_lrclk_map)/sizeof(mclk_lrclk_map[0]); i++)
                    {
                        if (mclk_lrclk_map[i].fs == config->sample_rate)
                        {
                            /* Keep trying until the platform selects a valid configuration. */
                            result = wm8533_platform_configure(wm8533, mclk_lrclk_map[i].mclk, mclk_lrclk_map[i].fs, config->bits_per_sample);
                            if (WICED_SUCCESS == result)
                            {
                                mclk_lrck_ratio = mclk_lrclk_map[i].sr;
                                mclk_is_found = WICED_TRUE;
                                break;
                            }
                        }
                    }
                    if( mclk_is_found == WICED_FALSE )
                    {
                        return WICED_ERROR;
                    }
                    break;

                case WM8533_FMT_CCS_CFS:
                    /* codec is a frame slave, I2S pll has been setup already  */

                    break;
                default:
                    /* Everything else is unsupported. */
                    result = WICED_UNSUPPORTED;
            }
        }
        else
        {
            for (i=0; i<sizeof(mclk_lrclk_map)/sizeof(mclk_lrclk_map[0]); i++)
            {
                if (mclk_lrclk_map[i].mclk == *mclk)
                {
                    mclk_lrck_ratio = mclk_lrclk_map[i].sr;
                    break;
                }
            }
        }
    }
    /*
     * Configure CODEC.
     */

    /* Read AIF_CTRL1. */
    if (WICED_SUCCESS == result)
        result = wm8533_reg_read(wm8533, WM8533_REG_AIF_CTRL1, &ctrl1);
    /* Read AIF_CTRL2. */
    if (WICED_SUCCESS == result)
        result = wm8533_reg_read(wm8533, WM8533_REG_AIF_CTRL2, &ctrl2);

    /* Reset AIF_CTRL1. */
    ctrl1 &= ~(
    /* Audio data interface format. */
        AIF_FMT_BITS << AIF_FMT_SHIFT |
    /* Master/slave select. */
        AIF_MSTR_BITS << AIF_MSTR_SHIFT |
    /* Audio data word length. */
        AIF_WL_BITS << AIF_WL_SHIFT
    );

    /* Reset AIF_CTRL2. */
    ctrl2 &= ~(
    /* Sample rate; MCLK:LRCLK ratio. */
        AIF_SR_BITS << AIF_SR_SHIFT |
    /* Bit clock divider. */
        AIF_BCLKDIV_BITS << AIF_BCLKDIV_SHIFT
    );

    if (WICED_SUCCESS == result)
    {
        /* Audio data interface format. */
        /* Must agree with I2S bus settings. */
        ctrl1 |= CODEC_STANDARD << AIF_FMT_SHIFT;

        switch (wm8533->fmt & WM8533_FMT_MASTER_MASK)
        {
        case WM8533_FMT_CCS_CFM:
            /* CODEC is frame master. */
            ctrl1 |= AIF_MSTR_MASTER << AIF_MSTR_SHIFT;
            ctrl2 |= mclk_lrck_ratio << AIF_SR_SHIFT;
            break;
        case WM8533_FMT_CCS_CFS:
            /* CODEC is frame slave. */
            ctrl1 |= AIF_MSTR_SLAVE << AIF_MSTR_SHIFT;
            /* Auto detect ratio from master. */
            ctrl2 |= mclk_lrck_ratio << AIF_SR_SHIFT;
            break;
        default:
            /* Everything else is unsupported. */
            result = WICED_UNSUPPORTED;
        }
    }
    if (WICED_SUCCESS == result)
    {
        /* XXX Dependent on STM32F4x settings (guessed per section 27.4.4 STM32F4 reference manual).
         * FIXME Move me to platform!
        */
        switch (config->bits_per_sample)
        {
        case 16:
            ctrl1 |= AIF_WL_16_BITS << AIF_WL_SHIFT;
            ctrl2 |= AIF_BCLKDIV_32FS << AIF_BCLKDIV_SHIFT;
            break;
        case 24:
            ctrl1 |= AIF_WL_24_BITS << AIF_WL_SHIFT;
            ctrl2 |= AIF_BCLKDIV_64FS << AIF_BCLKDIV_SHIFT;
            break;
        case 32:
            ctrl1 |= AIF_WL_32_BITS << AIF_WL_SHIFT;
            ctrl2 |= AIF_BCLKDIV_64FS << AIF_BCLKDIV_SHIFT;
            break;
        default:
            result = WICED_UNSUPPORTED;
        }
    }

    /* Write AIF_CTRL1. */
    if (WICED_SUCCESS == result)
        result = wm8533_reg_write(wm8533, WM8533_REG_AIF_CTRL1, ctrl1);
    /* Write AIF_CTRL2. */
    if (WICED_SUCCESS == result)
        result = wm8533_reg_write(wm8533, WM8533_REG_AIF_CTRL2, ctrl2);

    if (WICED_SUCCESS == result)
    {
        uint16_t psctrl_value;
        psctrl_value = 0;
        psctrl_value = ( SYS_ENA_POWER_DOWN << SYS_ENA_FIELD_SHIFT );
        /* first stage, all analog block and digital signal blocks are powered off */
        /* the control interface is turned on only */
        result = wm8533_reg_write(wm8533, WM8533_REG_PSCTRL1, psctrl_value);
    }


    /* Set codec to power up mode with soft mute on */
    if( WICED_SUCCESS == result )
    {
        uint16_t psctrl_value;
        psctrl_value = 0;
        psctrl_value = ( SYS_ENA_POWER_UP_WITH_MUTE << SYS_ENA_FIELD_SHIFT );
        /* Second stage of powering the device up, SYS_ENA= 0x02 */
        /* all blocks are enabled, the digital soft mute (100 db attenuation) is turned on */
        result = wm8533_reg_write(wm8533, WM8533_REG_PSCTRL1, psctrl_value);
    }

    /* Set codec to power up mode */
    if( WICED_SUCCESS == result )
    {
        uint16_t psctrl_value;
        psctrl_value = 0;
        psctrl_value = ( SYS_ENA_POWER_UP << SYS_ENA_FIELD_SHIFT );
        /* Third stage of powering the device up, SYS_ENA= 0x03 */
        /* digital soft mute (100 db attenuation) is released, the device blocks are all powered up */
        result = wm8533_reg_write(wm8533, WM8533_REG_PSCTRL1, psctrl_value);
    }


    return result;
}


wiced_result_t wm8533_start_play ( void* driver_data )
{
    return WICED_SUCCESS;
}


wiced_result_t wm8533_pause ( void* driver_data )
{
    wm8533_device_data_t* wm8533 = ( wm8533_device_data_t* )driver_data;
    UNUSED_PARAMETER(wm8533);
    return WICED_SUCCESS;
}


wiced_result_t wm8533_resume ( void* driver_data )
{
    wm8533_device_data_t* wm8533 = ( wm8533_device_data_t* )driver_data;
    UNUSED_PARAMETER(wm8533);
    return WICED_SUCCESS;
}


static wiced_result_t wm8533_deinit ( void* driver_data )
{
    wm8533_device_data_t* wm8533 = ( wm8533_device_data_t* )driver_data;
    wiced_result_t result;

    /* XXX: assert for running state. */

    /* XXX: This procedure doesn't follow the suggested
     * power-down sequence specified in the datasheet.
     */

    result = WICED_SUCCESS;

#ifdef NOTYET
    /* Power off CODEC. */
    if (WICED_SUCCESS == result)
        result = wm8533_reg_write(0x02, 0x01);
#endif /* NOTYET */

    /* Clean-up configuration if applicable. */
    if (NULL != give_more_samples_callback)
    {
        result = wiced_i2s_deinit(wm8533->data_port);
        if (WICED_SUCCESS == result)
            give_more_samples_callback=NULL;
    }

    /* Don't deinitialize I2C since it might be used by other modules.
     * Diddo for IOE.
     */

    return WICED_SUCCESS;
}


/* Set of Low level DAC access API, will be called from wiced_audio_device_param_set */

static wiced_result_t wm8533_reg_write( wm8533_device_data_t* wm8533, uint8_t address, uint16_t reg_data )
{
    return i2c_reg_write(wm8533->i2c_data, address, reg_data);
}

static wiced_result_t wm8533_reg_read(wm8533_device_data_t* wm8533, uint8_t address, uint16_t* reg_data)
{
    return i2c_reg_read(wm8533->i2c_data, address, reg_data);
}

static wiced_result_t wm8533_set_volume( void* driver_data, double decibels )
{

    int db_steps;
    uint16_t gain_reg_value;
    wiced_result_t result;

    wm8533_device_data_t* wm8533= (wm8533_device_data_t*)driver_data;

    /* set to max level if the value is very high */
    if( decibels > MAX_WM8533_DB_LEVEL )
    {
        decibels = MAX_WM8533_DB_LEVEL;
      /* set to minimum possible if the level is to low */
    } else if( decibels < MIN_WM8533_DB_LEVEL )
    {
        decibels = MIN_WM8533_DB_LEVEL;
    }

    /* get value which will be written to the DAC_GAINL and DAC_GAINR registers */
    if( decibels < 0 )
    {
        db_steps = (int)( (decibels * (-1.0)) / GAIN_ADJUSTMENT_STEP );
        gain_reg_value = GAIN_REG_VALUE_ON_ZERO_DB - db_steps;
    } else if( decibels == 0 )
    {
        gain_reg_value = GAIN_REG_VALUE_ON_ZERO_DB;
    } else
    {
        db_steps = (int)( (decibels / GAIN_ADJUSTMENT_STEP) );
        gain_reg_value = GAIN_REG_VALUE_ON_ZERO_DB + db_steps;
    }

    result = wm8533_reg_write(wm8533, WM8533_REG_DAC_GAINL, (1 << DIGITAL_VOLUME_UPDATE_FIELD_SHIFT) | ( uint16_t )( gain_reg_value  << DIGITAL_VOLUME_DB_VALUE ) );
    wiced_assert("Cant set volume on the dac's left channel", result == WICED_SUCCESS);
    result = wm8533_reg_write(wm8533, WM8533_REG_DAC_GAINR, (1 << DIGITAL_VOLUME_UPDATE_FIELD_SHIFT) | ( uint16_t )( gain_reg_value  << DIGITAL_VOLUME_DB_VALUE ) );
    wiced_assert("Cant set volume on the dac's right channel",result == WICED_SUCCESS);

    return result;
}

static wiced_result_t wm8533_audio_device_port ( void* device_data, wiced_i2s_t* port )
{
    wm8533_device_data_t* wm8533 = ( wm8533_device_data_t* )device_data;

    *port = wm8533->data_port;

    return WICED_SUCCESS;
}


