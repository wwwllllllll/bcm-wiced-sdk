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
 *  Defines functions that allow access to the Device Configuration Table (DCT)
 *
 */

#pragma once

#include <stdint.h>
#include "platform_dct.h"
#include "wiced_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 * @cond                Macros
 ******************************************************/

#define DCT_OFFSET( type, member )  ( (uint32_t)&((type *)0)->member )

#if defined(__IAR_SYSTEMS_ICC__)
#pragma section="initial_dct_section"
#define DEFINE_APP_DCT( type ) \
    const type _app_dct @ "initial_dct_section"; \
    const type _app_dct =
#else  /* #if defined(__IAR_SYSTEMS_ICC__) */
#define DEFINE_APP_DCT( type ) const type _app_dct =
#endif /* #if defined(__IAR_SYSTEMS_ICC__) */

#define shared_waf_api ( (const shared_waf_api_t*)(&shared_api_addr_loc) )

#ifndef OFFSETOF
#define OFFSETOF( type, member )  ( (uintptr_t)&((type *)0)->member )
#endif /* OFFSETOF */

/******************************************************
 *                    Constants
 ******************************************************/

#define WICED_FACTORY_RESET_MAGIC_VALUE  ( 0xA6C5A54E )

/******************************************************
 *                   Enumerations
 ******************************************************/

/**
 * Application validity
 */
typedef enum
{
    APP_INVALID = 0,
    APP_VALID   = 1
} app_valid_t;

/**
 * Bootloader status
 */
typedef enum boot_status_enum
{
    BOOTLOADER_BOOT_STATUS_START                   = 0x00,
    BOOTLOADER_BOOT_STATUS_APP_OK                  = 0x01,
    BOOTLOADER_BOOT_STATUS_WLAN_BOOTED_OK          = 0x02,
    BOOTLOADER_BOOT_STATUS_WICED_STARTED_OK        = 0x03,
    BOOTLOADER_BOOT_STATUS_DATA_CONNECTIVITY_OK    = 0x04
} boot_status_t;

/**
 * DCT section
 */
typedef enum
{
    DCT_APP_SECTION,
    DCT_SECURITY_SECTION,
    DCT_MFG_INFO_SECTION,
    DCT_WIFI_CONFIG_SECTION,
    DCT_INTERNAL_SECTION, /* Do not use in apps */
} dct_section_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/**
 * Copy Function Source Structure
 */
typedef struct
{
    union
    {
        void*    file_handle;
        uint32_t fixed_address;
    } val;
    enum
    {
        COPY_SRC_FILE,
        COPY_SRC_FIXED,
    } type;
} platform_copy_src_t;

/**
 * Structure of shared APIs between bootloader and application
 */
typedef struct
{
    void           (*platform_perform_factory_reset)     ( void );
    void           (*platform_start_ota_upgrade)         ( void );
    void           (*platform_reboot)                    ( void );
    wiced_result_t (*platform_erase_app)                 ( void );
    wiced_result_t (*platform_write_app_chunk)           ( uint32_t offset, const uint8_t* data, uint32_t size );
    wiced_result_t (*platform_set_app_valid_bit)         ( app_valid_t validity );
    wiced_result_t (*platform_read_wifi_firmware)        ( uint32_t address, void* buffer, uint32_t requested_size, uint32_t* read_size );
    void           (*platform_start_app)                 ( uint32_t vector_table_address );
    wiced_result_t (*platform_copy_external_to_internal) ( void* destination, platform_copy_src_t* copy_src, uint32_t size );
    wiced_result_t (*platform_dct_write)                 ( const void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size );
    wiced_result_t (*platform_dct_read_with_copy)        ( void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size );
    void*          (*platform_get_current_dct_address)   (  dct_section_t section );
} shared_waf_api_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

extern const shared_waf_api_t shared_api_addr_loc; /* from link script */

/******************************************************
 *               Function Declarations
 * @endcond
 ******************************************************/

/*****************************************************************************/
/** @defgroup framework       WICED Application Framework
 *
 *  WICED functions for managing apps and non-volatile data
 */
/*****************************************************************************/

/*****************************************************************************/
/** @addtogroup dct       DCT
 *  @ingroup framework
 *
 * Device Configuration Table (Non-volatile flash storage space)
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Reads the DCT and returns a pointer to the DCT data
 *
 * The operation of this function depends on whether the DCT is located in
 * external or internal flash memory.
 * If ptr_is_writable is set to false and the DCT is located in internal flash,
 * then a direct pointer to the flash memory will be returned.
 * Otherwise memory will be allocated and the DCT data will be copied into it.
 *
 * @note : this function must be used in pairs with wiced_dct_read_unlock to
 *         ensure that any allocated memory is freed.
 *
 * @param info_ptr [out] : a pointer to the pointer that will be filled on return
 * @param ptr_is_writable [in] : if true then then the returned pointer will be in RAM
 *                          allowing it to be modified. e.g. before being written
 * @param section [in] : the section of the DCT which should be read
 * @param offset [in] : the offset in bytes within the section
 * @param size [in] : the length of data that should be read
 *
 * @return    Wiced Result
 */
extern wiced_result_t wiced_dct_read_lock(   void** info_ptr, wiced_bool_t ptr_is_writable, dct_section_t section, uint32_t offset, uint32_t size );


/** Frees any space allocated in wiced_dct_read_lock()
 *
 * @note : this function must be used in pairs with wiced_dct_read_lock
 *
 * @param info_ptr [in] : a pointer that was created with wiced_dct_read_lock()
 * @param ptr_is_writable[in] : indicates whether the pointer was retrevied as a writable pointer
 *
 * @return    Wiced Result
 */
extern wiced_result_t wiced_dct_read_unlock( void* info_ptr, wiced_bool_t ptr_is_writable );


/** Writes data to the DCT
 *
 * Writes a chunk of data to the DCT.
 *
 * @note : Ensure that this function is only called occasionally, otherwise
 *         the flash memory wear may result.
 *
 * @param info_ptr [in] : a pointer to the pointer that will be filled on return
 * @param section [in]: the section of the DCT which should be read
 * @param offset [in]: the offset in bytes within the section
 * @param size [in] : the length of data that should be read
 *
 * @return    Wiced Result
 */
static inline  wiced_result_t wiced_dct_write( const void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size );

/** @} */

/*****************************************************************************/
/** @addtogroup app       App management
 *
 * Application management functions
 *
 * @note: these functions are implemented as function pointers to allow
 *        them to be shared between a bootloader and application
 *
 *  @{
 */
/*****************************************************************************/

/** Overwrites the application with the factory reset application
 *
 * Wipes out the current application and restores the default factory application and additionally resets
 * all values stored in the DCT to factory defaults, then reboots. \p
 *
 * Accidental usage of this function is protected by a magic number. If the
 * magic number argument does not match WICED_FACTORY_RESET_MAGIC_VALUE,
 * the function fails and normal program execution continues
 *
 * @warning The current application immediately terminates when this function is called.
 *          The function does not return, and ALL state for the current application
 *          is discarded. A reboot into the factory reset application will be performed
 *          when the function has finished
 *
 * @param[in] magic_value : A magic value that must match WICED_FACTORY_RESET_MAGIC_VALUE for
 *                          the function to complete successfully
 *
 * @return Does not return!
 *
 */
static inline void wiced_framework_perform_factory_reset( uint32_t magic_value );


/** Starts the Over-The-Air upgrade application
 *
 * Starts the Over-The-Air (OTA) upgrade application.
 * This gives the user the opportunity to upgrade the application.
 * Once the OTA app has finished, the system reboots
 *
 * @warning The current application immediately terminates when this function is called.
 *          The function does not return, and ALL state for the current application is
 *          discarded. A reboot will be performed when the function has finished
 *
 *  @return Does not return!
 */
static inline void wiced_framework_start_ota_upgrade( void );


/** Reboots the system
 *
 * Causes a soft-reset of the processor, which will restart the program
 * from the boot vector. The function does not return.
 *
 *  @return Does not return!
 */
static inline void wiced_framework_reboot( void );


/** Erases the main application.
 *
 * Causes a soft-reset of the processor, which will restart the program
 * from the boot vector. The function does not return.
 *
 * @warning Do not call this function from the main application. That would
 *          result in the function returning to a wiped area of flash.
 *          Function must only be called from a bootloader or application
 *          running in RAM.
 *
 *  @return Wiced reuslt code
 */
static inline wiced_result_t wiced_framework_erase_app( void );


/** Writes a piece of a main application to flash
 *
 * Write a piece of a main application into internal flash.
 *
 * @warning This function should only be called by a bootloader or an
 *          application running from RAM. If called from a main application
 *          then unpredictable execution might result due to the application
 *          containing parts of two different versions
 *
 * @param[in] offset : The offset from the start of the application in bytes
 * @param[in] data : The data to be written
 * @param[in] size : The number of bytes to be written
 *
 *                          the function to complete successfully
 *
 *  @return Wiced reuslt code
 */
static inline wiced_result_t  wiced_framework_write_app_chunk( uint32_t offset, const uint8_t* data, uint32_t size );


/** Sets the bit in the DCT to indicate that the main application is valid.
 *
 * This function is used when overwriting the main application in internal
 * flash memory. e.g. for Over-the-air upgrade. When the application has
 * been successfully completed writing, this function should be called
 * to enable the application. This guards against corrupted or incomplete
 * application writes.
 *
 * @note This function should normally only be called by a bootloader or an
 *       application running from RAM. If a main application sets this to
 *       zero, then it will not boot on the next system reset.
 *
 * @param[in] val : The value to be written to the "app_valid" bit.
 *                  1 = application is valid, 0 = invalid application.
 *
 *  @return Wiced reuslt code
 */
static inline wiced_result_t wiced_framework_set_app_valid_bit( app_valid_t validity );


/** Reads the Wifi Firmware from the Factory reset image
 *
 * This function reads a Wifi Firmware image from external flash.
 * It allows applications such as Over-the-air upgrade to boot the
 * Wifi system without being made larger by containing the image in the app.
 * The firmware image that is read should be the one contained in the
 * factory reset application.
 *
 * @note This function is normally only called by a bootloader or an
 *       application running from RAM.
 *
 * @param[in] offset : The offset into the firmware image
 * @param[in] buffer : buffer to be filled
 * @param[in] requested_size : maximum number of bytes to read
 * @param[out] read_size : actual number of bytes that were read
 *
 *  @return Wiced reuslt code
 */
static inline wiced_result_t wiced_framework_read_wifi_firmware( uint32_t offset, void* buffer, uint32_t requested_size, uint32_t* read_size );


/** Starts an application
 *
 * This function jumps to a the start of an application.
 *
 * @param entry_address - the address of the first instruction
 *                        for the application
 *
 * @note This function is normally only called by a bootloader or an
 *       application running from RAM.
 * @warning This function does not return.
 *
 *  @return Function does not return!
 */
static inline void wiced_framework_start_app( uint32_t entry_address );


/** Copy data from external memory into internal memory
 *
 * This function allows the bootloader or other apps to copy
 * data from external (serial flash) memory into internal memory.
 *
 *
 * @param[in] destination : the destination internal memory address. Can be in flash or RAM.
 * @param[in] copy_src : a structure containing the source location details.
 * @param[in] size     : the number of bytes to copy.
 *
 *  @return a Wiced result code
 */
static inline wiced_result_t wiced_framework_copy_external_to_internal( void* destination, platform_copy_src_t* copy_src, uint32_t size );


/** Deprecated
 *
 * @deprecated Please use wiced_framework_perform_factory_reset( ) instead.
 *
 */
static inline void DEPRECATE( wiced_restore_factory_image( uint32_t magic_value ) );


/** Deprecated
 *
 * @deprecated Please use wiced_framework_start_ota_upgrade( ) instead.
 *
 */
static inline void DEPRECATE( wiced_start_ota_upgrade( void ) );

/** @} */

/*****************************************************************************/
/** @addtogroup deprecated_dct       Deprecated DCT functions
 *  @ingroup dct
 *
 * Device Configuration Table (Non-volatile flash storage space)
 *
 *
 *  @{
 */
/*****************************************************************************/

#ifndef EXTERNAL_DCT

/** Retrieves a pointer to the application section of the current DCT
 *
 * @deprecated Please use @wiced_dct_read_app_section instead.
 *             Function is not compatible with a DCT located in external flash, and will
 *             be removed in future SDK versions
 *
 * @return    The app section pointer
 */
extern void const* DEPRECATE( wiced_dct_get_app_section( void ) );


/** Retrieves a pointer to the manufacturing info section of the current DCT
 *
 * @deprecated Please use @wiced_dct_get_mfg_info instead.
 *             Function is not compatible with a DCT located in external flash, and will
 *             be removed in future SDK versions
 *
 * @return    The manufacturing section pointer
 */
extern platform_dct_mfg_info_t const* DEPRECATE( wiced_dct_get_mfg_info_section( void ) );


/** Retrieves a pointer to the security section of the current DCT
 *
 * @deprecated Please use @wiced_dct_get_security_section instead.
 *             Function is not compatible with a DCT located in external flash, and will
 *             be removed in future SDK versions
 *
 * @return    The security section pointer
 */
extern platform_dct_security_t const* DEPRECATE( wiced_dct_get_security_section( void ) );


/** Retrieves a pointer to the Wi-Fi config info section of the current DCT
 *
 * @deprecated Please use @wiced_dct_get_wifi_config instead.
 *             Function is not compatible with a DCT located in external flash, and will
 *             be removed in future SDK versions
 *
 * @return    The Wi-Fi section pointer
 */
extern platform_dct_wifi_config_t const* DEPRECATE( wiced_dct_get_wifi_config_section( void ) );

/*------------------------------------- */

/** Reads a volatile copy of the DCT security section from flash into a block of RAM
 *
 * @param[out] security_dct : A pointer to the RAM that will receive a copy of the DCT security section
 *
 * @return @ref wiced_result_t
 */
extern wiced_result_t DEPRECATE( wiced_dct_read_security_section( platform_dct_security_t* security_dct ) );


/** Writes a volatile copy of the DCT security section in RAM to the flash
 *
 * @warning: To avoid flash wear, this function should only be used for settings which are changed rarely
 *
 * @param[in] security_dct : A pointer to the volatile copy of the DCT security section in RAM
 *
 * @return @ref wiced_result_t
 */
extern wiced_result_t DEPRECATE( wiced_dct_write_security_section( const platform_dct_security_t* security_dct ) );

/*------------------------------------- */

/** Reads a volatile copy of the DCT wifi config section from flash into a block of RAM
 *
 * @param[out] wifi_config_dct : A pointer to the RAM that will receive a copy of the DCT wifi config section
 *
 * @return @ref wiced_result_t
 */
extern wiced_result_t DEPRECATE( wiced_dct_read_wifi_config_section( platform_dct_wifi_config_t* wifi_config_dct ) );


/** Writes a volatile copy of the DCT Wi-Fi config section in RAM to the flash
 *
 * @warning: To avoid flash wear, this function should only be used for settings which are changed rarely
 *
 * @param[in] wifi_config_dct : A pointer to the volatile copy of DCT Wi-Fi config section in RAM
 *
 * @return @ref wiced_result_t
 */
extern wiced_result_t DEPRECATE( wiced_dct_write_wifi_config_section( const platform_dct_wifi_config_t* wifi_config_dct ) );

/*------------------------------------- */

/** Reads a volatile copy of the DCT app section from flash into a block of RAM
 *
 * @param[out] app_dct : A pointer to the RAM that will receive a copy of the DCT app section
 * @param[in]  size    : The size of the DCT app section
 *
 * @return @ref wiced_result_t
 */
extern wiced_result_t DEPRECATE( wiced_dct_read_app_section( void* app_dct, uint32_t size ) );


/** Writes a volatile copy of the DCT app section in RAM to the flash
 *
 * @note: To avoid wearing out the flash, this function should only be
 *        used for settings which are changed rarely.
 *
 * @param[in] size    : The size of the DCT app section
 * @param[in] app_dct : A pointer to the volatile copy of DCT app section in RAM
 *
 * @return @ref wiced_result_t
 */
extern wiced_result_t DEPRECATE( wiced_dct_write_app_section( const void* app_dct, uint32_t size ) );

/*------------------------------------- */

#endif /* ifndef EXTERNAL_DCT */

/** @} */

/*****************************************************************************/
/**  Implementations of inline functions                                    **/
/*****************************************************************************/

static inline  wiced_result_t wiced_dct_write( const void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size )
{
    return shared_waf_api->platform_dct_write( info_ptr, section, offset, size );
}

static inline void wiced_framework_perform_factory_reset( uint32_t magic_value )
{
    if ( magic_value == WICED_FACTORY_RESET_MAGIC_VALUE )
    {
        shared_waf_api->platform_perform_factory_reset( );
    }
}

static inline void wiced_framework_start_ota_upgrade( void )
{
    shared_waf_api->platform_start_ota_upgrade( );
}

static inline void wiced_framework_reboot( void )
{
    shared_waf_api->platform_reboot( );
}

static inline wiced_result_t wiced_framework_erase_app( void )
{
    return shared_waf_api->platform_erase_app( );
}

static inline wiced_result_t  wiced_framework_write_app_chunk( uint32_t offset, const uint8_t* data, uint32_t size )
{
    return shared_waf_api->platform_write_app_chunk( offset, data, size );
}

static inline wiced_result_t wiced_framework_set_app_valid_bit( app_valid_t validity )
{
    return shared_waf_api->platform_set_app_valid_bit( validity );
}

static inline wiced_result_t wiced_framework_read_wifi_firmware( uint32_t address, void* buffer, uint32_t requested_size, uint32_t* read_size )
{
    return shared_waf_api->platform_read_wifi_firmware( address, buffer, requested_size, read_size );
}

static inline void wiced_framework_start_app( uint32_t entry_address )
{
    shared_waf_api->platform_start_app( entry_address );
}

static inline wiced_result_t wiced_framework_copy_external_to_internal( void* destination, platform_copy_src_t* copy_src, uint32_t size )
{
    return shared_waf_api->platform_copy_external_to_internal( destination, copy_src, size );
}

/*****************************************************************************/
/**  Deprecated functions                                                   **/
/*****************************************************************************/

static inline void wiced_restore_factory_image( uint32_t magic_value )
{
    wiced_framework_perform_factory_reset( magic_value );
}

static inline void wiced_start_ota_upgrade( void )
{
    wiced_framework_start_ota_upgrade( );
}

#ifdef __cplusplus
} /*extern "C" */
#endif
