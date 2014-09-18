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
#include "wiced_easy_setup.h"
#include "wiced_internal_api.h"

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

wiced_result_t wiced_easy_setup_start_cooee( wiced_easy_setup_cooee_callback_t callback )
{
    wiced_cooee_workspace_t* workspace = malloc_named( "cooee", sizeof(wiced_cooee_workspace_t) );
    if ( workspace == NULL )
    {
        return WICED_OUT_OF_HEAP_SPACE;
    }
    memset(workspace, 0, sizeof(wiced_cooee_workspace_t));
    wiced_wifi_cooee( workspace );

    callback( workspace->user_processed_data, (uint16_t) ( workspace->received_byte_count - ( workspace->user_processed_data - workspace->received_cooee_data ) ) );

    free( workspace );

    return WICED_SUCCESS;
}


wiced_result_t wiced_easy_setup_start_softap( const configuration_entry_t* config )
{
    return wiced_configure_device(config);
}
