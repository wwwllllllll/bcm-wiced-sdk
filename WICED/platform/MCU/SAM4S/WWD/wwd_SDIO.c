/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <string.h> /* For memcpy */
#include "wwd_platform_common.h"
#include "wwd_bus_protocol.h"
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"
#include "platform/wwd_sdio_interface.h"
#include "platform/wwd_bus_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "network/wwd_network_constants.h"
//#include "misc.h"
#include "hsmci.h"
#include "sdmmc.h"
#include "platform_cmsis.h"
#include "platform_peripheral.h"
#include "platform_config.h"

/******************************************************
 *             Constants
 ******************************************************/

#define SDIO_ENUMERATION_TIMEOUT_MS    (500)

/******************************************************
 *             Static Function Declarations
 ******************************************************/

static void              enable_sdio_rx_irq                  ( void );
static void              disable_sdio_rx_irq                 ( void );
static void              enable_sdio_not_busy_irq            ( void );
static void              disable_sdio_not_busy_irq           ( void );
//static void              enable_sdio_block_transfer_done_irq ( void );
//static void              disable_sdio_block_transfer_done_irq( void );
static sdio_block_size_t find_optimal_block_size             ( uint32_t data_size );
static uint8_t           send_sdio_command                   ( Mcid *pMcid, char ignore_err );
static void              reset_sdio_command                  ( MciCmd *pCommand );
static void              sdio_oob_irq_handler                ( void* arg );
extern wwd_result_t    platform_bus_enter_powersave          ( void );
extern wwd_result_t    platform_bus_exit_powersave           ( void );

/******************************************************
 *             Variables
 ******************************************************/

/* SDIO OOB interrupt configuration */
//static const platform_gpio_irq_config_t sdio_oob_irq_config =
//{
//    .wakeup_pin = WICED_TRUE,
//    .trigger    = IOPORT_SENSE_RISING,
//    .callback   = sdio_oob_irq_handler,
//    .arg        = NULL,
//};

//static const platform_wakeup_pin_config_t sdio_oob_irq_wakeup_pin_config =
//{
//    .is_wakeup_pin     = WICED_TRUE,
//    .wakeup_pin_number = SDIO_OOB_IRQ_WAKEUP_ID,
//    .trigger           = IOPORT_SENSE_RISING,
//};


/* SDIO driver */
static Mcid   sdio_driver;

/* SDIO command variable */
static MciCmd sdio_command;

/* Temporary buffer to contain SDIO RX data */
static volatile uint8_t temp_sdio_rx_buffer[2048 + 32];

/* Semaphore to wait on until SDIO transfer is done */
//host_semaphore_type_t sdio_transfer_done_semaphore;

/* SDIO bus initialisation flag */
static wiced_bool_t sdio_bus_initted = WICED_FALSE;

/******************************************************
 *             Function definitions
 ******************************************************/

wwd_result_t host_platform_bus_init( void )
{
    if ( sdio_bus_initted == WICED_FALSE )
    {
        uint8_t a = 0;

        platform_mcu_powersave_disable();

        /* SDIO bootstrapping: GPIO0 = 0 and GPIO1 = 0 */

#ifdef WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP
        platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0], OUTPUT_PUSH_PULL );
        platform_gpio_output_low( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0] );

        platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1], OUTPUT_PUSH_PULL );
        platform_gpio_output_low( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1] );
#endif /* WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP */
        /* Setup SDIO pins */
        for ( a = WWD_PIN_SDIO_CLK; a < WWD_PIN_SDIO_MAX; a++ )
        {
            if( a == WWD_PIN_SDIO_OOB_IRQ )
            {
                platform_gpio_peripheral_pin_init( &wifi_sdio_pins[a], 0 );
            }
            else
            {
                platform_gpio_peripheral_pin_init( &wifi_sdio_pins[a], ( IOPORT_MODE_MUX_C | IOPORT_MODE_PULLUP ));
            }
        }

        /* Enable the MCI peripheral */
        sysclk_enable_peripheral_clock( ID_HSMCI );

        HSMCI->HSMCI_CR = HSMCI_CR_SWRST;

        MCI_Disable( HSMCI );

        MCI_Init( &sdio_driver, HSMCI, ID_HSMCI, CPU_CLOCK_HZ );

        /* Enable SDIO interrupt */
        /* Priority must be set in platform.c file function platform_init_peripheral_irq_priorities */
        /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
        /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
//        NVIC_SetPriority( HSMCI_IRQn, SAM4S_SDIO_IRQ_PRIO );
        NVIC_EnableIRQ( HSMCI_IRQn );

//        host_rtos_init_semaphore( &sdio_transfer_done_semaphore );

//        enable_sdio_block_transfer_done_irq();

        platform_mcu_powersave_enable();

        sdio_bus_initted = WICED_TRUE;
    }
    return WICED_SUCCESS;
}

wwd_result_t host_platform_sdio_enumerate( void )
{
    wwd_result_t result;
    uint32_t       loop_count;
    uint32_t       data = 0;

    loop_count = 0;
    do
    {
        /* Send CMD0 to set it to idle state */
        host_platform_sdio_transfer( BUS_WRITE, SDIO_CMD_0, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, NO_RESPONSE, NULL );

        /* CMD5. */
        host_platform_sdio_transfer( BUS_READ, SDIO_CMD_5, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, NO_RESPONSE, NULL );

        /* Send CMD3 to get RCA. */
        result = host_platform_sdio_transfer( BUS_READ, SDIO_CMD_3, SDIO_BYTE_MODE, SDIO_1B_BLOCK, 0, 0, 0, RESPONSE_NEEDED, &data );
        loop_count++;
        if ( loop_count >= (uint32_t) SDIO_ENUMERATION_TIMEOUT_MS )
        {
            return WICED_TIMEOUT;
        }
    } while ( ( result != WWD_SUCCESS ) && ( host_rtos_delay_milliseconds( (uint32_t) 1 ), ( 1 == 1 ) ) );
    /* If you're stuck here, check the platform matches your hardware */

    /* Send CMD7 with the returned RCA to select the card */
    host_platform_sdio_transfer( BUS_WRITE, SDIO_CMD_7, SDIO_BYTE_MODE, SDIO_1B_BLOCK, data, 0, 0, RESPONSE_NEEDED, NULL );

    return WICED_SUCCESS;
}

wwd_result_t host_platform_bus_deinit( void )
{
    if ( sdio_bus_initted == WICED_TRUE )
    {
        uint8_t a = 0;

        platform_mcu_powersave_disable();

//        host_rtos_deinit_semaphore( &sdio_transfer_done_semaphore );
//        disable_sdio_block_transfer_done_irq();

        MCI_Disable( HSMCI );
        NVIC_DisableIRQ( HSMCI_IRQn );

#ifdef WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP
        platform_gpio_deinit( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0] );
        platform_gpio_deinit( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1] );
#endif /* WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP */

        /* Reset all SDIO pins to input pull-up to save power */
        for ( a = WWD_PIN_SDIO_CLK; a < WWD_PIN_SDIO_MAX; a++ )
        {
            platform_gpio_deinit( &wifi_sdio_pins[a] );
        }

        platform_mcu_powersave_enable();

        sdio_bus_initted = WICED_FALSE;
    }

    return WICED_SUCCESS;
}

wwd_result_t host_platform_sdio_transfer( wwd_bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size, sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response )
{
    uint8_t error;
    MciCmd *pCommand = &( sdio_command );

    platform_mcu_powersave_disable();
    /*
     direction => BUS_READ, BUS_WRITE,
     command => SDIO_CMD_0, SDIO_CMD_3, SDIO_CMD_5, SDIO_CMD_7, SDIO_CMD_52, SDIO_CMD_53
     mode => SDIO_BLOCK_MODE, SDIO_BYTE_MODE
     block_size => SDIO_1B_BLOCK to SDIO_2048B_BLOCK, we use SDIO_1B_BLOCK and SDIO_64B_BLOCK
     argument => data byte (or 0)
     response_expected => RESPONSE_NEEDED, NO_RESPONSE
     */

    UNUSED_PARAMETER( response_expected );

    reset_sdio_command( pCommand );

    //fill defaults
    pCommand->arg = argument;
    pCommand->pResp = response;

    //clear the response, if any
    if ( response != NULL)
    {
        *response = 0;
    };

    // note: do not forget to properly fill Rx response reg. number to pCommand->resType

    //do the command
    switch ( command )
    {
        case SDIO_CMD_0:
        {
            //CMD 0: typical direction: BUS_WRITE
            pCommand->cmd = ( 0 | HSMCI_CMDR_RSPTYP_48_BIT | HSMCI_CMDR_SPCMD_STD | HSMCI_CMDR_TRCMD_NO_DATA | HSMCI_CMDR_MAXLAT);

            //pCommand->resType = 1; // nah

            error = send_sdio_command( &sdio_driver, 1 );
            if ( error == 0 )
            {
                platform_mcu_powersave_enable();
                return WICED_SUCCESS;
            };
            break;
        }

        case SDIO_CMD_3:
        {
            //CMD 3: typical direction: BUS_READ
            pCommand->cmd  = 3;
            pCommand->cmd |= HSMCI_CMDR_RSPTYP_48_BIT;
            pCommand->cmd |= HSMCI_CMDR_SPCMD_STD;
            pCommand->cmd |= HSMCI_CMDR_OPDCMD_OPENDRAIN;
            pCommand->cmd |= HSMCI_CMDR_MAXLAT_64;
            pCommand->cmd |= HSMCI_CMDR_TRCMD_NO_DATA;
            pCommand->cmd |= (direction==BUS_READ)?HSMCI_CMDR_TRDIR_READ:HSMCI_CMDR_TRDIR_WRITE;
            pCommand->cmd |= HSMCI_CMDR_TRTYP_BYTE;
            pCommand->cmd |= HSMCI_CMDR_IOSPCMD_STD;
            pCommand->cmd |= HSMCI_CMDR_ATACS_NORMAL;


            pCommand->resType = 6;

            error = send_sdio_command( &sdio_driver, 0 );
            if ( error == SDMMC_ERROR_NORESPONSE )
            {
                platform_mcu_powersave_enable();
                return WICED_TIMEOUT;
            }
            else if ( error == 0 )
            {
                platform_mcu_powersave_enable();
                return WICED_SUCCESS;
            };
            break;
        }

        case SDIO_CMD_5:
        {
            //CMD 5: typical direction: BUS_READ
            pCommand->cmd = ( 5 | HSMCI_CMDR_RSPTYP_R1B | HSMCI_CMDR_SPCMD_STD | HSMCI_CMDR_MAXLAT | HSMCI_CMDR_TRCMD_NO_DATA );
            pCommand->resType = 4;

            error = send_sdio_command( &sdio_driver, 1 );
            if ( error == 0 )
            {
                platform_mcu_powersave_enable();
                return WICED_SUCCESS;
            };
            break;
        }

        case SDIO_CMD_7:
        {
            //CMD 7: typical direction: BUS_WRITE
            pCommand->cmd = ( 7 | HSMCI_CMDR_RSPTYP_R1B | HSMCI_CMDR_SPCMD_STD | HSMCI_CMDR_MAXLAT | HSMCI_CMDR_TRCMD_NO_DATA );
            pCommand->resType = 1;

            error = send_sdio_command( &sdio_driver, 1 );
            if ( error == 0 )
            {
                platform_mcu_powersave_enable();
                return WICED_SUCCESS;
            };
            break;
        }

        case SDIO_CMD_52:
        {
            //CMD 52: typical direction: BOTH
            pCommand->cmd = ( 52 | HSMCI_CMDR_RSPTYP_48_BIT | HSMCI_CMDR_SPCMD_STD | HSMCI_CMDR_MAXLAT | HSMCI_CMDR_TRCMD_NO_DATA );
            pCommand->resType = 5;

            error = send_sdio_command( &sdio_driver, 1 );
            if ( error == 0 )
            {
                platform_mcu_powersave_enable();
                return WICED_SUCCESS;
            };
            break;
        }

        case SDIO_CMD_53:
        {
            //specific

            pCommand->cmd = ( 53 | HSMCI_CMDR_TRCMD_START_DATA | HSMCI_CMDR_RSPTYP_48_BIT | HSMCI_CMDR_SPCMD_STD | HSMCI_CMDR_MAXLAT );

            /* Write/Read operation */
            if ( direction == BUS_WRITE )
            {
                pCommand->tranType = MCI_START_WRITE;
            }
            else
            {
                pCommand->cmd |= HSMCI_CMDR_TRDIR;
                pCommand->tranType = MCI_START_READ;
            }

            /* Always Block mode */
            pCommand->cmd |= HSMCI_CMDR_TRTYP_BLOCK | HSMCI_CMDR_TRCMD_START_DATA;

            if ( mode == SDIO_BYTE_MODE )
            {
                /* Dodgy STM32 hack to set the CMD53 byte mode size to be the same as the block size */
                pCommand->cmd |= HSMCI_CMDR_TRTYP_BYTE;
                block_size = find_optimal_block_size( data_size );
                if ( block_size < SDIO_512B_BLOCK )
                {
                    pCommand->arg = ( pCommand->arg & (uint32_t) ( ~0x1FF ) ) | block_size;
                }
                else
                {
                    pCommand->arg = ( pCommand->arg & (uint32_t) ( ~0x1FF ) );
                }

                // compute block count
                pCommand->blockSize = block_size;
                pCommand->nbBlock = ( data_size / block_size );
                if ( ( pCommand->nbBlock * block_size ) < data_size )
                {
                    pCommand->nbBlock++;
                };
            }
            else
            {

              // compute block count
              pCommand->blockSize = block_size;
              pCommand->nbBlock = ( data_size / block_size );
              if ( ( pCommand->nbBlock * block_size ) < data_size )
              {
                  pCommand->nbBlock++;
              };
            }

            pCommand->resType = 5; //select the response register

            /* Set the DMA data pointer to the temp data for SDIO read */
            if ( direction == BUS_WRITE )
            {
                pCommand->pData = (unsigned char *)data;
            }
            else
            {
                pCommand->pData = (unsigned char *)temp_sdio_rx_buffer;
            }

            error = send_sdio_command( &sdio_driver, 0 );
            if ( error == 0 )
            {
                if ( direction == BUS_READ )
                {
                    /* Copy the data read by DMA to the buffer */
                    memcpy( (void *) data, (void *) temp_sdio_rx_buffer, data_size );
                }

                platform_mcu_powersave_enable();
                return WICED_SUCCESS;
            }

            break;
        }

        case __MAX_VAL:
            break;
        default:
//            WICED_ERROR_PRINT_DEBUG_ONLY(( "\n INVALID SDIO CMD \n" ));
            break;
    }

    platform_mcu_powersave_enable();

    return WICED_ERROR;
}

void host_platform_enable_high_speed_sdio( void )
{

// no!    MCI_EnableHsMode(&MCI_Driver, 1);
// this function is really puzzling me, observing the scope and no change there

    // real speed = MCK / divisor
    // where divisor is : 2,4,6,8,10, etc.

    /*
     DIV   MCI FREQ
     2     32000000
     4     16000000
     6     10666667
     8     8000000
     10    6400000
     12
     14
     16

     */
    MCI_SetSpeed( &sdio_driver, 50000000, CPU_CLOCK_HZ );

    /* Enable high-speed mode */
    MCI_EnableHsMode( &sdio_driver, 1 );

    MCI_SetBusWidth( &sdio_driver, HSMCI_SDCR_SDCBUS_4 ); // 10: 4-bit

    /* Enable SDIO RX interrupt */
    enable_sdio_rx_irq();
}

#ifndef WICED_DISABLE_MCU_POWERSAVE
uint8_t host_platform_get_oob_interrupt_pin( void )
{
    return WICED_WIFI_OOB_IRQ_GPIO_PIN;
}
#endif /* WICED_DISABLE_MCU_POWERSAVE */

wwd_result_t host_enable_oob_interrupt( void )
{
    /* Enable OOB interrupt */
    platform_gpio_irq_enable( &wifi_sdio_pins[WWD_PIN_SDIO_OOB_IRQ], IOPORT_SENSE_RISING, sdio_oob_irq_handler, NULL);

    if( wifi_sdio_pins[WWD_PIN_SDIO_OOB_IRQ].wakeup_pin_config != NULL )
    {
        platform_powersave_enable_wakeup_pin( wifi_sdio_pins[WWD_PIN_SDIO_OOB_IRQ].wakeup_pin_config );
    }


    return WICED_SUCCESS;
}

wwd_result_t platform_bus_enter_powersave( void )
{
    if ( sdio_bus_initted == WICED_TRUE )
    {
        uint32_t a;

        /* Disable the MCI peripheral */
        sysclk_disable_peripheral_clock( ID_HSMCI );

        /* Disable SDIO peripheral clock */
        for ( a = WWD_PIN_SDIO_CLK; a < WWD_PIN_SDIO_MAX; a++ )
        {
            ioport_set_pin_mode( wifi_sdio_pins[0].pin, IOPORT_MODE_PULLUP );
            ioport_set_pin_dir ( wifi_sdio_pins[0].pin, IOPORT_DIR_INPUT );
        }
    }

    return WICED_SUCCESS;
}

wwd_result_t platform_bus_exit_powersave( void )
{
    if ( sdio_bus_initted == WICED_TRUE )
    {
        uint32_t a;

        /* Enable SDIO peripheral clock */
        for ( a = WWD_PIN_SDIO_CLK; a < WWD_PIN_SDIO_MAX; a++ )
        {
            if( a == WWD_PIN_SDIO_OOB_IRQ )
            {
                platform_gpio_peripheral_pin_init( &wifi_sdio_pins[a], 0 );
            }
            else
            {
                platform_gpio_peripheral_pin_init( &wifi_sdio_pins[a], ( IOPORT_MODE_MUX_C | IOPORT_MODE_PULLUP ));
            }
        }

        /* Enable the MCI peripheral */
        sysclk_enable_peripheral_clock( ID_HSMCI );
    }

    return WICED_SUCCESS;
}

/******************************************************
 *             Static function definitions
 ******************************************************/

static void enable_sdio_rx_irq( void )
{
    sdio_driver.pMciHw->HSMCI_IER = HSMCI_IER_SDIOIRQA;
}

static void disable_sdio_rx_irq( void )
{
    sdio_driver.pMciHw->HSMCI_IDR = HSMCI_IDR_SDIOIRQA;
}

static void enable_sdio_not_busy_irq( void )
{
    sdio_driver.pMciHw->HSMCI_IER = HSMCI_IER_NOTBUSY;
}

static void disable_sdio_not_busy_irq( void )
{
    sdio_driver.pMciHw->HSMCI_IDR = HSMCI_IDR_NOTBUSY;
}

//static void enable_sdio_block_transfer_done_irq( void )
//{
//    sdio_driver.pMciHw->HSMCI_IER = HSMCI_IER_BLKE;
//}
//
//static void disable_sdio_block_transfer_done_irq( void )
//{
//    sdio_driver.pMciHw->HSMCI_IDR = HSMCI_IDR_BLKE;
//}

static sdio_block_size_t find_optimal_block_size( uint32_t data_size )
{
    if ( data_size > (uint32_t) 256 )
        return SDIO_512B_BLOCK;
    if ( data_size > (uint32_t) 128 )
        return SDIO_256B_BLOCK;
    if ( data_size > (uint32_t) 64 )
        return SDIO_128B_BLOCK;
    if ( data_size > (uint32_t) 32 )
        return SDIO_64B_BLOCK;
    if ( data_size > (uint32_t) 16 )
        return SDIO_32B_BLOCK;
    if ( data_size > (uint32_t) 8 )
        return SDIO_16B_BLOCK;
    if ( data_size > (uint32_t) 4 )
        return SDIO_8B_BLOCK;
    if ( data_size > (uint32_t) 2 )
        return SDIO_4B_BLOCK;

    return SDIO_4B_BLOCK;
}

static uint8_t send_sdio_command( Mcid *pMcid, char ignore_err )
{
    MciCmd *pCommand = &( sdio_command );
    uint8_t error;

    /* Fill callback */
    pCommand->callback = NULL;
    pCommand->pArg     = NULL; //   // note this is a pArg, not the Arg !

    /* Disable SDIO receive interrupt during transmission */
    disable_sdio_rx_irq( );

    /* Enable SDIO NOTBUSY interrupt. This is to tell the software that "Interrupt Period"
     * begins and receive interrupt needs to be enabled
     */
    enable_sdio_not_busy_irq( );

    /* Send command */
    error = Sdmmc_SendCommand( pMcid, pCommand, NULL );

    if ( ( error != 0 ) && ( error != SDMMC_ERROR_NORESPONSE ) )
    {
        /* Transmission error, set interrupts settings back to "Interrupt Period" */
        disable_sdio_not_busy_irq( );
        enable_sdio_rx_irq( );
        return SDMMC_ERROR;
    }

    if ( ignore_err != 0 )
    {
        // some commands do not really reproduce any answer on the CMD line
        // but the HSMCI reports this as an error
        // so we need to ignore this
        if ( pCommand->status == 0x03 )
        {
            pCommand->status = 0;
        };
    }

    return pCommand->status;
}

static void reset_sdio_command( MciCmd *pCommand )
{
    pCommand->cmd       = 0;
    pCommand->arg       = 0;
    pCommand->pData     = 0;
    pCommand->blockSize = 0;
    pCommand->nbBlock   = 0;
    pCommand->pResp     = 0;
    pCommand->callback  = 0;
    pCommand->pArg      = 0;
    pCommand->resType   = 0;
    pCommand->tranType  = 0;
    pCommand->busyCheck = 0;
    pCommand->status    = 0;
    pCommand->state     = 0;
}

static void sdio_oob_irq_handler( void* arg )
{
    UNUSED_PARAMETER( arg );
    platform_mcu_powersave_exit_notify( );
    wwd_thread_notify_irq( );
}

wwd_result_t host_platform_bus_enable_interrupt( void )
{
    return  WWD_SUCCESS;
}

wwd_result_t host_platform_bus_disable_interrupt( void )
{
    return  WWD_SUCCESS;
}

void host_platform_bus_buffer_freed( wwd_buffer_dir_t direction )
{
    UNUSED_PARAMETER( direction );
}

/******************************************************
 *                  ISR definitions
 ******************************************************/

WWD_RTOS_DEFINE_ISR(sdio_irq)
{
    /* Reading status register (HSMCI_SR) clears the interrupts */
    uint32_t status_register = sdio_driver.pMciHw->HSMCI_SR;
    uint32_t mask_register   = sdio_driver.pMciHw->HSMCI_IMR;

    if ( ( status_register & mask_register ) & HSMCI_SR_SDIOIRQA )
    {
        /* Once received, disable SDIO receive interrupt */
        disable_sdio_rx_irq( );

        /* Notify WICED bus to read WLAN. Packet(s) waiting */
        wwd_thread_notify_irq( );
    }

    if ( ( status_register & mask_register ) & HSMCI_SR_NOTBUSY )
    {
        disable_sdio_not_busy_irq( );

        /* SDIO receive interrupt is continuously sampled, it can't be
         * enabled all the time or otherwise it triggers false interrupts.
         * NOTBUSY interrupt marks the beginning of SDIO "Interrupt Period".
         * Enable SDIO RX interrupt now.
         */
        enable_sdio_rx_irq( );
    }

//    if ( ( status_register & mask_register ) & HSMCI_SR_BLKE )
//    {
//        host_rtos_set_semaphore( &sdio_transfer_done_semaphore, WICED_TRUE );
//    }
}


/******************************************************
 *             IRQ Handler Mapping
 ******************************************************/

WWD_RTOS_MAP_ISR( sdio_irq,     MCI_irq )

