/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <string.h>
#include "wwd_management.h"
#include "RTOS/wwd_rtos_interface.h"
#include "internal/wwd_internal.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"
#include "internal/wwd_sdpcm.h"
#include "chip_constants.h"
#include "wwd_bcmendian.h"

/******************************************************
 *             Constants
 ******************************************************/

#define BACKPLANE_ADDRESS_MASK  0x7FFF
#define AI_IOCTRL_OFFSET      0x408
#define SICF_FGC              0x0002
#define SICF_CLOCK_EN         0x0001
#define AI_RESETCTRL_OFFSET   0x800
#define AIRC_RESET            1

#define WRAPPER_REGISTER_OFFSET    0x100000

/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *             Variables
 ******************************************************/

static const uint32_t core_base_address[] =
{
    (uint32_t) ( ARMCM3_BASE_ADDRESS + WRAPPER_REGISTER_OFFSET  ),
    (uint32_t) ( SOCSRAM_BASE_ADDRESS + WRAPPER_REGISTER_OFFSET ),
    (uint32_t) ( SDIO_BASE_ADDRESS )
};

wwd_wlan_status_t wwd_wlan_status =
{
    .state             = WLAN_DOWN,
    .country_code      = WICED_COUNTRY_AUSTRALIA,
    .keep_wlan_awake = 0,
};

/******************************************************
 *             Static Function Declarations
 ******************************************************/

static uint32_t wwd_get_core_address( device_core_t core_id );

/******************************************************
 *             Function definitions
 ******************************************************/

/*
 * Returns the base address of the core identified by the provided coreId
 */
uint32_t wwd_get_core_address( device_core_t core_id )
{
    return core_base_address[(int) core_id];
}

/*
 * Returns WWD_SUCCESS is the core identified by the provided coreId is up, otherwise WWD result code
 */
wwd_result_t wwd_device_core_is_up( device_core_t core_id )
{
    uint8_t regdata;
    uint32_t base;
    wwd_result_t result;

    base = wwd_get_core_address( core_id );

    /* Read the IO control register */
    result = wwd_bus_read_backplane_value( base + AI_IOCTRL_OFFSET, (uint8_t) 1, &regdata );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    /* Verify that the clock is enabled and something else is not on */
    if ( ( regdata & ( SICF_FGC | SICF_CLOCK_EN ) ) != (uint8_t) SICF_CLOCK_EN )
    {
        return WWD_CORE_CLOCK_NOT_ENABLED;
    }

    /* Read the reset control and verify it is not in reset */
    result = wwd_bus_read_backplane_value( base + AI_RESETCTRL_OFFSET, (uint8_t) 1, &regdata );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }
    if ( ( regdata & AIRC_RESET ) != 0 )
    {
        return WWD_CORE_IN_RESET;
    }

    return WWD_SUCCESS;
}

/*
 * Disables the core identified by the provided coreId
 */
wwd_result_t wwd_disable_device_core( device_core_t core_id )
{
    uint32_t base = wwd_get_core_address( core_id );
    wwd_result_t result;
    uint8_t junk;

    /* Read the reset control */
    result = wwd_bus_read_backplane_value( base + AI_RESETCTRL_OFFSET, (uint8_t) 1, &junk );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    result = wwd_bus_read_backplane_value( base + AI_RESETCTRL_OFFSET, (uint8_t) 1, &junk );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    /* Write 0 to the IO control and read it back */
    result = wwd_bus_write_backplane_value( base + AI_IOCTRL_OFFSET, (uint8_t) 1, 0 );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    result = wwd_bus_read_backplane_value( base + AI_IOCTRL_OFFSET, (uint8_t) 1, &junk );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    (void) host_rtos_delay_milliseconds( (uint32_t) 1 );

    result = wwd_bus_write_backplane_value( base + AI_RESETCTRL_OFFSET, (uint8_t) 1, (uint32_t) AIRC_RESET );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    (void) host_rtos_delay_milliseconds( (uint32_t) 1 );

    return result;
}

/*
 * Resets the core identified by the provided coreId
 */
wwd_result_t wwd_reset_device_core( device_core_t core_id )
{
    uint32_t base = wwd_get_core_address( core_id );
    wwd_result_t result;
    uint8_t junk;

    result = wwd_bus_write_backplane_value( base + AI_IOCTRL_OFFSET, (uint8_t) 1, (uint32_t) ( SICF_FGC | SICF_CLOCK_EN ) );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    result = wwd_bus_read_backplane_value( base + AI_IOCTRL_OFFSET, (uint8_t) 1, &junk );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    result = wwd_bus_write_backplane_value( base + AI_RESETCTRL_OFFSET, (uint8_t) 1, 0 );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    (void) host_rtos_delay_milliseconds( (uint32_t) 1 );

    result = wwd_bus_write_backplane_value( base + AI_IOCTRL_OFFSET, (uint8_t) 1, (uint32_t) SICF_CLOCK_EN );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    result = wwd_bus_read_backplane_value( base + AI_IOCTRL_OFFSET, (uint8_t) 1, &junk );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    (void) host_rtos_delay_milliseconds( (uint32_t) 1 );

    return result;
}




inline uint16_t bcmswap16( uint16_t val )
{
    return BCMSWAP16(val);
}

inline uint32_t bcmswap32( uint32_t val )
{
    return BCMSWAP32(val);
}

inline uint32_t bcmswap32by16( uint32_t val )
{
    return BCMSWAP32BY16(val);
}

/* Reverse pairs of bytes in a buffer (not for high-performance use) */
/* buf  - start of buffer of shorts to swap */
/* len  - byte length of buffer */
inline void bcmswap16_buf( uint16_t* buf, uint32_t len )
{
    len = len / 2;

    while ( ( len-- ) != 0 )
    {
        *buf = bcmswap16( *buf );
        buf++;
    }
}

/*
 * Store 16-bit value to unaligned little-endian byte array.
 */
inline void htol16_ua_store( uint16_t val, uint8_t* bytes )
{
    bytes[0] = (uint8_t) ( val & 0xff );
    bytes[1] = (uint8_t) ( val >> 8 );
}

/*
 * Store 32-bit value to unaligned little-endian byte array.
 */
inline void htol32_ua_store( uint32_t val, uint8_t* bytes )
{
    bytes[0] = (uint8_t) ( val & 0xff );
    bytes[1] = (uint8_t) ( ( val >> 8 ) & 0xff );
    bytes[2] = (uint8_t) ( ( val >> 16 ) & 0xff );
    bytes[3] = (uint8_t) ( val >> 24 );
}

/*
 * Store 16-bit value to unaligned network-(big-)endian byte array.
 */
inline void hton16_ua_store( uint16_t val, uint8_t* bytes )
{
    bytes[0] = (uint8_t) ( val >> 8 );
    bytes[1] = (uint8_t) ( val & 0xff );
}

/*
 * Store 32-bit value to unaligned network-(big-)endian byte array.
 */
inline void hton32_ua_store( uint32_t val, uint8_t* bytes )
{
    bytes[0] = (uint8_t) ( val >> 24 );
    bytes[1] = (uint8_t) ( ( val >> 16 ) & 0xff );
    bytes[2] = (uint8_t) ( ( val >> 8 ) & 0xff );
    bytes[3] = (uint8_t) ( val & 0xff );
}

/*
 * Load 16-bit value from unaligned little-endian byte array.
 */
inline uint16_t ltoh16_ua( const void* bytes )
{
    return (uint16_t) _LTOH16_UA((const uint8_t*)bytes);
}

/*
 * Load 32-bit value from unaligned little-endian byte array.
 */
inline uint32_t ltoh32_ua( const void* bytes )
{
    return (uint32_t) _LTOH32_UA((const uint8_t*)bytes);
}

/*
 * Load 16-bit value from unaligned big-(network-)endian byte array.
 */
inline uint16_t ntoh16_ua( const void* bytes )
{
    return (uint16_t) _NTOH16_UA((const uint8_t*)bytes);
}

/*
 * Load 32-bit value from unaligned big-(network-)endian byte array.
 */
inline uint32_t ntoh32_ua( const void* bytes )
{
    return (uint32_t) _NTOH32_UA((const uint8_t*)bytes);
}
