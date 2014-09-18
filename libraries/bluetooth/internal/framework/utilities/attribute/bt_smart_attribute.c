/**
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
#include "wiced_utilities.h"
#include "wiced_rtos.h"
#include "wwd_debug.h"
#include "bt_stack.h"
#include "bt_smart_gap.h"
#include "bt_smart_gatt.h"
#include "string.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define ATTR_NO_VALUE_SIZE                             ( ATTR_COMMON_FIELDS_SIZE + sizeof(uint8_t) )
#define ATTR_LONG_VALUE_SIZE                           ( ATTR_COMMON_FIELDS_SIZE + sizeof(uint8_t) * MAX_CHARACTERISTIC_VALUE_LENGTH )
#define ATTR_SERVICE_SIZE                              ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_service_t) )
#define ATTR_INCLUDE_SIZE                              ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_include_t) )
#define ATTR_CHARACTERISTIC_SIZE                       ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_characteristic_t) )
#define ATTR_CHARACTERISTIC_VALUE_SIZE( value_length ) ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_characteristic_value_t) + value_length )
#define ATTR_EXTENDED_PROPERTIES_SIZE                  ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_extended_properties_t) )
#define ATTR_USER_DESCRIPTION_SIZE( string_length )    ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_user_description_t) + string_length )
#define ATTR_CLIENT_CONFIG_SIZE                        ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_client_config_t) )
#define ATTR_SERVER_CONFIG_SIZE                        ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_server_config_t) )
#define ATTR_PRESENTATION_FORMAT_SIZE                  ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_presentation_format_t) )
#define ATTR_AGGREGATE_FORMAT_SIZE( handle_count )     ( ATTR_COMMON_FIELDS_SIZE + sizeof(attr_val_aggregate_format_t) + (sizeof(uint16_t) * handle_count) )

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

static const uint16_t fixed_attribute_size_list[] =
{
    [WICED_ATTRIBUTE_TYPE_NO_VALUE                                      ] =  ATTR_NO_VALUE_SIZE,
    [WICED_ATTRIBUTE_TYPE_LONG_VALUE                                    ] =  ATTR_LONG_VALUE_SIZE,
    [WICED_ATTRIBUTE_TYPE_PRIMARY_SERVICE                               ] =  ATTR_SERVICE_SIZE,
    [WICED_ATTRIBUTE_TYPE_INCLUDE                                       ] =  ATTR_INCLUDE_SIZE,
    [WICED_ATTRIBUTE_TYPE_CHARACTERISTIC                                ] =  ATTR_CHARACTERISTIC_SIZE,
    [WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_VALUE                          ] =  0, // Variable length
    [WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_DESCRIPTOR_EXTENDED_PROPERTIES ] =  ATTR_EXTENDED_PROPERTIES_SIZE,
    [WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_DESCRIPTOR_USER_DESCRIPTION    ] =  0, // Variable length
    [WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_DESCRIPTOR_CLIENT_CONFIGURATION] =  ATTR_CLIENT_CONFIG_SIZE,
    [WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_DESCRIPTOR_SERVER_CONFIGURATION] =  ATTR_SERVER_CONFIG_SIZE,
    [WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_DESCRIPTOR_PRESENTATION_FORMAT ] =  ATTR_PRESENTATION_FORMAT_SIZE,
    [WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_DESCRIPTOR_AGGREGATE_FORMAT    ] =  0, // Variable length
};

static uint32_t attributes_created = 0;
static uint32_t attributes_deleted = 0;

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

static wiced_result_t print_uuid( const wiced_bt_uuid_t* uuid );
static wiced_result_t print_type( const wiced_bt_uuid_t* uuid );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_bt_smart_attribute_create( wiced_bt_smart_attribute_t** attribute, wiced_bt_smart_attribute_type_t type, uint16_t variable_length )
{
    uint16_t size;

    if ( attribute == NULL )
    {
        return WICED_BT_BADARG;
    }

    switch ( type )
    {
        case WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_VALUE:
        {
            size = ATTR_CHARACTERISTIC_VALUE_SIZE( variable_length );
            break;
        }
        case WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_DESCRIPTOR_USER_DESCRIPTION:
        {
            size = ATTR_USER_DESCRIPTION_SIZE( variable_length );
            break;
        }
        case WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_DESCRIPTOR_AGGREGATE_FORMAT:
        {
            size = ATTR_AGGREGATE_FORMAT_SIZE( variable_length );
            break;
        }
        default:
        {
            size = fixed_attribute_size_list[type];
            break;
        }
    }

    *attribute = (wiced_bt_smart_attribute_t*)malloc_named("attribute", size );

    if ( *attribute == NULL )
    {
        return WICED_BT_OUT_OF_HEAP_SPACE;
    }

    memset( *attribute, 0, size );
    (*attribute)->value_struct_size = size - ATTR_COMMON_FIELDS_SIZE;
    attributes_created++;

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_delete( wiced_bt_smart_attribute_t* attribute )
{
    if ( attribute == NULL )
    {
        return WICED_BT_BADARG;
    }

    /* Set to NULL to make sure this doesn't point to any used attribute */
    attribute->next = NULL;

    /* For malloc debugging */
    malloc_transfer_to_curr_thread( attribute );
    free( attribute );

    attributes_deleted++;

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_create_list( wiced_bt_smart_attribute_list_t* list )
{
    if ( list == NULL )
    {
        return WICED_BT_BADARG;
    }

    memset( list, 0, sizeof( *list ) );
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_add_to_list( wiced_bt_smart_attribute_list_t* list, wiced_bt_smart_attribute_t* attribute )
{
    if ( list == NULL || attribute == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( list->count == 0 )
    {
        /* List is empty. Point list to the attribute */
        attribute->next = NULL;
        list->list = attribute;
    }
    else
    {
        wiced_bt_smart_attribute_t* curr = list->list;
        wiced_bt_smart_attribute_t* prev = NULL;

        /* Traverse the list and compare handle */
        while ( curr != NULL )
        {
            if ( curr->handle > attribute->handle )
            {
                if ( prev == NULL )
                {
                    /* Insert attribute at first position */
                    attribute->next = curr;
                    list->list = attribute;
                    break;
                }
                else
                {
                    /* Insert attribute in between prev and curr */
                    prev->next = attribute;
                    attribute->next = curr;
                    break;
                }
            }
            else if ( curr->next == NULL )
            {
                /* Insert attribute at the end of the list */
                curr->next = attribute;
                attribute->next = NULL;
                break;
            }

            /* Update previous and current attribute pointers */
            prev = curr;
            curr = curr->next;
        }
    }

    /* Increment count */
    list->count++;
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_remove_from_list( wiced_bt_smart_attribute_list_t* list, uint16_t handle )
{
    if ( list == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( list->count == 0 )
    {
        return WICED_BT_LIST_EMPTY;
    }
    else
    {
        wiced_bt_smart_attribute_t* curr = list->list;
        wiced_bt_smart_attribute_t* prev = NULL;

        /* Traverse the list and compare handle */
        while ( curr != NULL )
        {
            if ( curr->handle == handle )
            {
                if ( prev == NULL )
                {
                    /* Remove attribute at first position */
                    list->list = curr->next;
                }
                else
                {
                    /* Remove curr */
                    prev->next = curr->next;
                }

                wiced_bt_smart_attribute_delete( curr );

                /* Decrement count */
                list->count--;
                return WICED_BT_SUCCESS;
            }

            /* Update previous and current attribute pointers */
            prev = curr;
            curr = curr->next;
        }
    }

    return WICED_BT_ITEM_NOT_IN_LIST;
}

wiced_result_t wiced_bt_smart_attribute_delete_list( wiced_bt_smart_attribute_list_t* list )
{
    wiced_bt_smart_attribute_t* curr;

    if ( list == NULL )
    {
        return WICED_BT_BADARG;
    }

    curr = list->list;

    /* Traverse through the list and delete all attributes */
    while ( curr != NULL )
    {
        /* Store pointer to next because curr is about to be deleted */
        wiced_bt_smart_attribute_t* next = curr->next;

        wiced_bt_smart_attribute_delete( curr );

        /* Update curr */
        curr = next;
    }

    memset( list, 0, sizeof( *list ) );
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_search_list_by_handle( const wiced_bt_smart_attribute_list_t* list, uint16_t handle, wiced_bt_smart_attribute_t** attribute )
{
    if ( list == NULL || attribute == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( list->count == 0 )
    {
        return WICED_BT_LIST_EMPTY;
    }
    else
    {
        wiced_bt_smart_attribute_t* curr = list->list;

        /* Traverse the list and compare handle */
        while ( curr != NULL )
        {
            if ( curr->handle == handle )
            {
                *attribute = curr;
                return WICED_BT_SUCCESS;
            }

            /* Update current attribute pointers */
            curr = curr->next;
        }
    }

    return WICED_BT_ITEM_NOT_IN_LIST;
}

wiced_result_t wiced_bt_smart_attribute_search_list_by_uuid( const wiced_bt_smart_attribute_list_t* list, const wiced_bt_uuid_t* uuid, uint16_t starting_handle, uint16_t ending_handle, wiced_bt_smart_attribute_t** attribute )
{
    if ( list == NULL || uuid == NULL || attribute == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( list->count == 0 )
    {
        return WICED_BT_LIST_EMPTY;
    }
    else
    {
        wiced_bt_smart_attribute_t* curr = list->list;

        /* Traverse the list until it's larger or equal the starting handle provided */
        while ( curr != NULL && curr->handle <= ending_handle )
        {
            if ( curr->handle >= starting_handle )
            {
                break;
            }

            curr = curr->next;
        }

        /* Return if reaches the end of the list */
        if ( curr == NULL || curr->handle > ending_handle )
        {
            return WICED_BT_ITEM_NOT_IN_LIST;
        }

        /* Traverse the list and compare handle */
        while ( curr != NULL && curr->handle <= ending_handle )
        {
            /* Check if UUID is found */
            if ( memcmp( &curr->type.value, &uuid->value, uuid->size ) == 0 )
            {
                *attribute = curr;
                return WICED_BT_SUCCESS;
            }

            /* Update current attribute pointers */
            curr = curr->next;
        }
    }

    return WICED_BT_ITEM_NOT_IN_LIST;
}

wiced_result_t wiced_bt_smart_attribute_merge_lists( wiced_bt_smart_attribute_list_t* trunk_list, wiced_bt_smart_attribute_list_t* branch_list )
{
    wiced_bt_smart_attribute_t* curr;

    if ( trunk_list == NULL || branch_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    curr = branch_list->list;

    /* Traverse through the branch list */
    while ( curr != NULL )
    {
        wiced_bt_smart_attribute_t* prev = curr;

        /* Increment curr and take prev of the branch list */
        curr = curr->next;
        prev->next = NULL;

        /* Add prev to the trunk list */
        wiced_bt_smart_attribute_add_to_list( trunk_list, prev );
    }

    memset( branch_list, 0, sizeof( *branch_list ) );

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_print( const wiced_bt_smart_attribute_t* attribute )
{
    wiced_bt_smart_attribute_t* curr_attr = (wiced_bt_smart_attribute_t*)attribute;

    if ( attribute == NULL )
    {
        return WICED_BT_BADARG;
    }

    WPRINT_LIB_INFO( ( "----------------------------------------------------\n" ) );
    print_type( &curr_attr->type );
    WPRINT_LIB_INFO( ( "\n" ) );
    WPRINT_LIB_INFO( ( "Handle                         : %d\n", (int)curr_attr->handle ) );
    WPRINT_LIB_INFO( ( "Type                           : " ) );
    print_uuid( &curr_attr->type );
    WPRINT_LIB_INFO( ( "Permission                     : %d\n", (int)curr_attr->permission ) );
    WPRINT_LIB_INFO( ( "Value Length                   : %d\n", (int)curr_attr->value_length ) );

    if ( curr_attr->type.size == UUID_16BIT )
    {
        switch ( curr_attr->type.value.value_16_bit )
        {
            case 0x2800:
            {
                WPRINT_LIB_INFO( ( "Start Handle                   : %d\n", (int)curr_attr->value.service.start_handle ) );
                WPRINT_LIB_INFO( ( "End Handle                     : %d\n", (int)curr_attr->value.service.end_handle   ) );
                WPRINT_LIB_INFO( ( "Service UUID                   : ") );
                print_uuid( &curr_attr->value.service.uuid );
                break;
            }
            case 0x2802:
            {
                WPRINT_LIB_INFO( ( "Start Handle                   : %d\n", (int)curr_attr->value.include.included_service_handle ) );
                WPRINT_LIB_INFO( ( "End Handle                     : %d\n", (int)curr_attr->value.include.end_group_handle ) );
                WPRINT_LIB_INFO( ( "Service UUID                   : ") );
                print_uuid( &curr_attr->value.include.uuid );
                break;
            }
            case 0x2803:
            {
                WPRINT_LIB_INFO( ( "Properties                     : %d\n", (int)curr_attr->value.characteristic.properties ) );
                WPRINT_LIB_INFO( ( "Value Handle                   : %d\n", (int)curr_attr->value.characteristic.value_handle ) );
                WPRINT_LIB_INFO( ( "Value UUID                     : ") );
                print_uuid( &curr_attr->value.characteristic.uuid );
                break;
            }
            case 0x2900:
            {
                WPRINT_LIB_INFO( ( "Extended Properties            : %d\n", (int)curr_attr->value.extended_properties.properties ) );
                break;
            }
            case 0x2901:
            {
                WPRINT_LIB_INFO( ( "Extended Properties            : %s\n", curr_attr->value.user_description.string ) );
                break;
            }
            case 0x2902:
            {
                WPRINT_LIB_INFO( ( "Client Configuration           : %d\n", (int)curr_attr->value.client_config.config_bits ) );
                break;
            }
            case 0x2903:
            {
                WPRINT_LIB_INFO( ( "Server Configuration           : %d\n", (int)curr_attr->value.server_config.config_bits ) );
                break;
            }
            case 0x2904:
            {
                WPRINT_LIB_INFO( ( "Format                         : %d\n", (int)curr_attr->value.presentation_format.format ) );
                WPRINT_LIB_INFO( ( "Exponent                       : %d\n", (int)curr_attr->value.presentation_format.exponent ) );
                WPRINT_LIB_INFO( ( "Unit                           : %d\n", (int)curr_attr->value.presentation_format.unit ) );
                WPRINT_LIB_INFO( ( "Namespace                      : %d\n", (int)curr_attr->value.presentation_format.name_space ) );
                WPRINT_LIB_INFO( ( "Description                    : %d\n", (int)curr_attr->value.presentation_format.description ) );
                break;
            }
            case 0x2905:
            {
                uint32_t i;

                WPRINT_LIB_INFO( ( "List of Handles                : \n" ) );
                for ( i = 0; i < curr_attr->value_length / 2; i ++ )
                {
                    WPRINT_LIB_INFO( ( "%02d ", (int)curr_attr->value.aggregate_format.handle_list[i] ) );
                }
                WPRINT_LIB_INFO( ( "\n" ) );
                break;
            }
            case 0x2A00:
            {
                WPRINT_LIB_INFO( ( "Device Name                    : %s\n", curr_attr->value.device_name.device_name ) );
                break;
            }
            case 0x2A01:
            {
                WPRINT_LIB_INFO( ( "Appearance                     : %d\n", (int)curr_attr->value.appearance.appearance ) );
                break;
            }
            case 0x2A02:
            {
                WPRINT_LIB_INFO( ( "Peripheral Privacy Flag        : %d\n", (int)curr_attr->value.periph_privacy_flag.periph_privacy_flag ) );
                break;
            }
            case 0x2A03:
            {
                WPRINT_LIB_INFO( ( "Reconnection Address           : %02x:%02x:%02x:%02x:%02x:%02x\n",
                                   (int)curr_attr->value.reconn_address.reconn_address[0],
                                   (int)curr_attr->value.reconn_address.reconn_address[1],
                                   (int)curr_attr->value.reconn_address.reconn_address[2],
                                   (int)curr_attr->value.reconn_address.reconn_address[3],
                                   (int)curr_attr->value.reconn_address.reconn_address[4],
                                   (int)curr_attr->value.reconn_address.reconn_address[5] ) );
                break;
            }
            case 0x2A04:
            {
                WPRINT_LIB_INFO( ( "Max Connection Interval        : %d\n", (int)curr_attr->value.periph_preferred_conn_params.max_conn_interval ) );
                WPRINT_LIB_INFO( ( "Min Connection Interval        : %d\n", (int)curr_attr->value.periph_preferred_conn_params.min_conn_interval ) );
                WPRINT_LIB_INFO( ( "Slave Latency                  : %d\n", (int)curr_attr->value.periph_preferred_conn_params.slave_latency ) );
                WPRINT_LIB_INFO( ( "Supervision Timeout Multiplier : %d\n", (int)curr_attr->value.periph_preferred_conn_params.conn_supervision_timeout_multiplier ) );
                break;
            }
            default:
            {
                uint32_t i;

                WPRINT_LIB_INFO( ( "Value                          : \n" ) );
                for ( i = 0; i < curr_attr->value_length; i ++ )
                {
                    WPRINT_LIB_INFO( ( "%02x ", (int)curr_attr->value.value[i] ) );
                }
                WPRINT_LIB_INFO( ( "\n" ) );
                break;
            }
        }
    }

    WPRINT_LIB_INFO( ( "----------------------------------------------------\n" ) );
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_print_list( const wiced_bt_smart_attribute_list_t* list )
{
    wiced_bt_smart_attribute_t* curr_attr = (wiced_bt_smart_attribute_t*)list->list;

    while ( curr_attr != NULL )
    {
        wiced_bt_smart_attribute_print( curr_attr );
        curr_attr = curr_attr->next;
    }

    return WICED_BT_SUCCESS;
}

static wiced_result_t print_uuid( const wiced_bt_uuid_t* uuid )
{
    if ( uuid->size == UUID_16BIT )
    {
        WPRINT_LIB_INFO( ( "%04x\n", (int)uuid->value.value_16_bit ) );
    }
    else
    {
        WPRINT_LIB_INFO( ( "%04x %04x %04x %04x %04x %04x %04x %04x\n",
                           (int)uuid->value.value_128_bit[0],
                           (int)uuid->value.value_128_bit[1],
                           (int)uuid->value.value_128_bit[2],
                           (int)uuid->value.value_128_bit[3],
                           (int)uuid->value.value_128_bit[4],
                           (int)uuid->value.value_128_bit[5],
                           (int)uuid->value.value_128_bit[6],
                           (int)uuid->value.value_128_bit[7] ) );
    }

    return WICED_BT_SUCCESS;
}

static wiced_result_t print_type( const wiced_bt_uuid_t* uuid  )
{
    if ( uuid->size != UUID_16BIT )
    {
        WPRINT_LIB_INFO( ( "Unknown Type" ) );
        return WICED_SUCCESS;
    }

    switch ( uuid->value.value_16_bit )
    {
        case 0x2800:
        {
            WPRINT_LIB_INFO( ( "Primary Service" ) );
            break;
        }
        case 0x2801:
        {
            WPRINT_LIB_INFO( ( "Secondary Service" ) );
            break;
        }
        case 0x2802:
        {
            WPRINT_LIB_INFO( ( "Include" ) );
            break;
        }
        case 0x2803:
        {
            WPRINT_LIB_INFO( ( "Characteristic" ) );
            break;
        }
        case 0x2900:
        {
            WPRINT_LIB_INFO( ( "Characteristic Descriptor - Extended Characteristic Properties" ) );
            break;
        }
        case 0x2901:
        {
            WPRINT_LIB_INFO( ( "Characteristic Descriptor - User Description" ) );
            break;
        }
        case 0x2902:
        {
            WPRINT_LIB_INFO( ( "Characteristic Descriptor - Client Characteristic Configuration" ) );
            break;
        }
        case 0x2903:
        {
            WPRINT_LIB_INFO( ( "Characteristic Descriptor - Server Characteristic Configuration" ) );
            break;
        }
        case 0x2904:
        {
            WPRINT_LIB_INFO( ( "Characteristic Descriptor - Characteristic Presentation Format" ) );
            break;
        }
        case 0x2905:
        {
            WPRINT_LIB_INFO( ( "Characteristic Descriptor - Characteristic Aggregate Format" ) );
            break;
        }
        case 0x2A00:
        {
            WPRINT_LIB_INFO( ( "Characteristic Type - Device Name" ) );
            break;
        }
        case 0x2A01:
        {
            WPRINT_LIB_INFO( ( "Characteristic Type - Appearance" ) );
            break;
        }
        case 0x2A02:
        {
            WPRINT_LIB_INFO( ( "Characteristic Type - Peripheral Privacy Flags" ) );
            break;
        }
        case 0x2A03:
        {
            WPRINT_LIB_INFO( ( "Characteristic Type - Reconnection Address" ) );
            break;
        }
        case 0x2A04:
        {
            WPRINT_LIB_INFO( ( "Characteristic Type - Peripheral Preferred Connection Parameters" ) );
            break;
        }
        case 0x2A05:
        {
            WPRINT_LIB_INFO( ( "Service Changed" ) );
            break;
        }
        default:
        {
            WPRINT_LIB_INFO( ( "Unknown Type" ) );
            break;
        }
    }

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_get_list_head( const wiced_bt_smart_attribute_list_t* list, wiced_bt_smart_attribute_t** head )
{
    if ( list == NULL )
    {
        return WICED_BT_BADARG;
    }

    *head = list->list;
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smart_attribute_get_list_count( const wiced_bt_smart_attribute_list_t* list, uint32_t* count )
{
    if ( list == NULL )
    {
        return WICED_BT_BADARG;
    }

    *count = list->count;
    return WICED_BT_SUCCESS;
}
