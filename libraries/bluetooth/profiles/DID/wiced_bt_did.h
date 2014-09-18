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
 *  Defines functions for Bluetooth Device ID (DID) Profile
 */

#pragma once

#include "wiced.h"
#include "wiced_bt.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/** Device ID Profile mandatory attributes
 */
typedef struct
{
    uint16_t     specification_id;  /**< Specification ID */
    uint16_t     vendor_id;         /**< Vendor ID        */
    uint16_t     product_id;        /**< Product ID       */
    uint16_t     version;           /**< Version          */
    wiced_bool_t primary_record;    /**< Primary Record   */
    uint16_t     vendor_id_source;  /**< Vendor ID Source */
} wiced_bt_did_mandatory_attributes_t;

/** Device ID Profile optional attributes
 */
typedef struct
{
    char* client_executable_url;     /**< Client Executable URL */
    char* service_description;       /**< Service Description   */
    char* documentation_url;         /**< Documentation URL     */
} wiced_bt_did_optional_attributes_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/*****************************************************************************/
/** @addtogroup btprof  Profiles
 *  @ingroup wicedbt
 *
 *  Bluetooth Profiles Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/*****************************************************************************/
/** @addtogroup btdid      Device ID (DID)
 *  @ingroup btprof
 *
 * Device ID Profile Functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/** Enable the Bluetooth Device ID (DID) Profile
 *
 * This function adds an SDP record for the Device ID Profile with the
 * attributes provided.
 *
 * @param mandatory_attributes : A pointer to a structure containing mandatory
 *                               attributes of the Device ID SDP record
 * @param optional_attributes  : A pointer to a structure containing optional
 *                               attributes of the Device ID SDP record
 *
 * @warning Only mandatory attributes are supported presently.
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred.
 */
wiced_result_t wiced_bt_did_enable( const wiced_bt_did_mandatory_attributes_t* mandatory_attributes, const wiced_bt_did_optional_attributes_t* optional_attributes );

/** Disable the Bluetooth Device ID Profile
 *
 * This functions removed the Device ID Profile's SDP record.
 *
 * @return    WICED_SUCCESS : on success;
 *            WICED_BADARG  : if bad argument(s) are inserted;
 *            WICED_ERROR   : if an error occurred.
 */
wiced_result_t wiced_bt_did_disable( void );

/** @} */
/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif
