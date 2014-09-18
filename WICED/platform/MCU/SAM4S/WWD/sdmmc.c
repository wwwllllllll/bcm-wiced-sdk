/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/*
 *  Updated by ATMEL on June 2012.
 */

#include "platform_peripheral.h"
#include "hsmci.h"
#include "sdmmc.h"
#include "wwd_rtos.h"
#include "wwd_constants.h"
#include "wwd_assert.h"
#include "RTOS/wwd_rtos_interface.h"
#include "stdint.h"

/** Bit mask for status register errors. */
#define STATUS_ERRORS ((uint32_t)(HSMCI_SR_UNRE  \
                       | HSMCI_SR_OVRE \
                       | HSMCI_SR_ACKRCVE \
                       /*| HSMCI_SR_BLKOVRE*/ \
                       | HSMCI_SR_CSTOE \
                       | HSMCI_SR_DTOE \
                       | HSMCI_SR_DCRCE \
                       | HSMCI_SR_RTOE \
                       | HSMCI_SR_RENDE \
                       | HSMCI_SR_RCRCE \
                       | HSMCI_SR_RDIRE \
                       | HSMCI_SR_RINDE))

/** Bit mask for response errors */
#define STATUS_ERRORS_RESP ((uint32_t)(HSMCI_SR_CSTOE \
                            | HSMCI_SR_RTOE \
                            | HSMCI_SR_RENDE \
                            | HSMCI_SR_RCRCE \
                            | HSMCI_SR_RDIRE \
                            | HSMCI_SR_RINDE))

/** Bit mask for data errors */
#define STATUS_ERRORS_DATA ((uint32_t)(HSMCI_SR_UNRE \
                            | HSMCI_SR_OVRE \
                            /*| HSMCI_SR_BLKOVRE*/ \
                            /*| HSMCI_SR_CSTOE*/ \
                            | HSMCI_SR_DTOE \
                            | HSMCI_SR_DCRCE))

/** Transfer parameters */
typedef struct _MciTransfer
{
    uint8_t* pData; /** Pointer to data buffer            */
    uint32_t size; /** Total number of bytes to transfer */
    uint32_t transferred; /** Number of bytes finished          */
    uint16_t fifoed; /** bytes in FIFO                     */
    uint8_t isRx; /** 1 for RX                          */
    uint8_t reserved;
} MciTransfer;

/** Tx/Rx parameters */
static MciTransfer hsmciXfr;

/**
 * Reset MCI HW interface and disable it.
 * \param keepSettings Keep old register settings, including
 *                     _MR, _SDCR, _DTOR, _CSTOR, _DMA and _CFG.
 */
/** Reset MCI */
#define MCI_RESET(pMciHw)      (pMciHw->HSMCI_CR = HSMCI_CR_SWRST)

/**
 * Starts a MCI  transfer. This is a non blocking function. It will return
 * as soon as the transfer is started.
 * Return 0 if successful; otherwise returns MCI_ERROR_LOCK if the driver is
 * already in use.
 * \param pMci  Pointer to an MCI driver instance.
 * \param pCommand  Pointer to the command to execute.
 */

#define HSMCI_TIMEOUT  (0x900)
#define HSMCI_TIMEOUT2 (0x9000)


uint8_t Sdmmc_SendCommand( Mcid *pMci, MciCmd *pCommand, host_semaphore_type_t* semaphore )
{
    Hsmci* pMciHw = pMci->pMciHw;
    uint32_t masked_SR;
    uint32_t timeout;
    uint8_t resSize;
    uint8_t i;

    UNUSED_PARAMETER( semaphore );

    /* Command is now being executed */
    pMci->pCommand = pCommand;

    /* Enable the MCI peripheral clock */
    if ( pmc_is_periph_clk_enabled( pMci->mciId ) == 0 )
    {
        /* PMC: enable HSMCI for the first time */
        pmc_enable_periph_clk( pMci->mciId );

        /* no time clock division as a power saving method */
        pMciHw->HSMCI_CR = HSMCI_CR_PWSDIS | HSMCI_CR_MCIEN;
    }

    /* Prepare Default Mode register value */
    pMciHw->HSMCI_MR = pMciHw->HSMCI_MR & ( ~( HSMCI_MR_WRPROOF | HSMCI_MR_RDPROOF | HSMCI_MR_PDCMODE | HSMCI_MR_FBYTE ) );

    /* Stop transfer: idle the bus */
    if ( pCommand->tranType == MCI_STOP_TRANSFER )
    {
        // Nothing to do ?
        return SDMMC_OK;
    }
    /* No data transfer: stop at the end of the command */
    else if ( pCommand->tranType == MCI_NO_TRANSFER )
    {
        while ( ( pMciHw->HSMCI_SR & HSMCI_SR_XFRDONE ) == 0 )
        {
        }

        /* Send the command */
        pMciHw->HSMCI_ARGR = pCommand->arg;
        pMciHw->HSMCI_CMDR = pCommand->cmd;
        timeout = 0;
        do
        {
            masked_SR = pMciHw->HSMCI_SR;
            if ( masked_SR & 0xC07F0000 /*HSMCI_SR_RTOE*/)
            {
                pCommand->status = SDMMC_ERROR_NORESPONSE;
                //no success anyway
                return SDMMC_ERROR_NORESPONSE;
            }
        }
        while ( ( ( masked_SR & HSMCI_SR_CMDRDY ) == 0 ) && ( ++timeout < HSMCI_TIMEOUT ) );
        if ( timeout == HSMCI_TIMEOUT )
        {
            return SDMMC_ERROR;
        }

        /* read the response */
        /* Store the card response in the provided buffer */
        if ( pCommand->pResp )
        {
            switch ( pCommand->resType )
            {
                case 1:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                    resSize = 1;
                    break;

                case 2:
                    resSize = 4;
                    break;

                default:
                    resSize = 0;
                    break;
            }
            for ( i = 0; i < resSize; i++ )
            {
                pCommand->pResp[i] = pMciHw->HSMCI_RSPR[0];
            }
        }
        return SDMMC_OK;
    }
    else
    {
        /* Command with DATA stage */
        /* Check data size */
        if ( pCommand->nbBlock == 0 )
        {
            return SDMMC_ERROR_PARAM;
        }

        /* Set number of blocks to transfer */
        pMciHw->HSMCI_BLKR = HSMCI_BLKR_BLKLEN( (uint32_t)pCommand->blockSize ) | (uint32_t)( pCommand->nbBlock << HSMCI_BLKR_BCNT_Pos );

        /* Force byte transfer if needed */
        if ( pCommand->blockSize & 0x3 )
        {
            pMciHw->HSMCI_MR |= HSMCI_MR_FBYTE;
        }

        /* Enable PDC */
        pMciHw->HSMCI_MR |= HSMCI_MR_PDCMODE;

        /* Update Tx struct for write */
        if ( ( pCommand->tranType == MCI_START_WRITE ) || ( pCommand->tranType == MCI_WRITE ) )
        {
            hsmciXfr.isRx = 0;  // write
            // Be carefull, HSMCI_TPR address must not be in flash
            pMciHw->HSMCI_TPR = (uint32_t)pCommand->pData;

            if ( pCommand->blockSize & 0x3 )
            {
                pMciHw->HSMCI_TCR = (uint32_t)( pCommand->blockSize * pCommand->nbBlock );
            }
            else
            {
                pMciHw->HSMCI_TCR = (uint32_t)( pCommand->blockSize * pCommand->nbBlock / 4 );
            }
            pMciHw->HSMCI_TNCR = 0;  // PDC: TXNCTR contains next transmit buffer size
        }
        /* Update Rx struct for read */
        else
        {
            hsmciXfr.isRx = 1; // read
            pMciHw->HSMCI_RPR = (uint32_t)pCommand->pData;
            if ( pCommand->blockSize & 0x3 )
            {
                pMciHw->HSMCI_RCR = (uint32_t)( pCommand->blockSize * pCommand->nbBlock );
            }
            else
            {
                pMciHw->HSMCI_RCR = (uint32_t)( pCommand->blockSize * pCommand->nbBlock / 4 );
            }

            pMciHw->HSMCI_RNCR = 0;  //PDC RXNCTR contains next receive buffer size.
            pMciHw->HSMCI_PTCR = HSMCI_PTCR_RXTEN; // PDC Receiver Transfer Enable
        }
    }

    /* Send the command */
    if ( ( pCommand->tranType != MCI_WRITE && pCommand->tranType != MCI_READ ) || ( pCommand->blockSize == 0 ) )
    {
        while ( ( pMciHw->HSMCI_SR & HSMCI_SR_XFRDONE ) == 0 )
        {
        }

        pMciHw->HSMCI_ARGR = pCommand->arg;
        pMciHw->HSMCI_CMDR = pCommand->cmd;

        timeout = 0;
        do
        {
            masked_SR = pMciHw->HSMCI_SR;
            if ( masked_SR & 0xC07F0000 /*HSMCI_SR_RTOE*/)
            {
                pCommand->status = SDMMC_ERROR_NORESPONSE;
                //no success anyway
                return SDMMC_ERROR_NORESPONSE;
            }
        }
        while ( ( ( masked_SR & HSMCI_SR_CMDRDY ) == 0 ) && ( ++timeout < HSMCI_TIMEOUT ) );
        if ( timeout == HSMCI_TIMEOUT )
        {
            return SDMMC_ERROR;
        }

        if ( ( pCommand->tranType == MCI_START_WRITE ) || ( pCommand->tranType == MCI_WRITE ) )
        {
            pMciHw->HSMCI_PTCR = HSMCI_PTCR_TXTEN;  // PDC Transmitter Transfer Enable
        }

//        host_rtos_get_semaphore( semaphore, 100, WICED_FALSE );
    }

    timeout = 0;
    do
    {
        masked_SR = pMciHw->HSMCI_SR;
        if ( masked_SR & 0xC07F0000 ) //HSMCI_SR_DCRCE))
        {
            pCommand->status = SDMMC_ERROR_NORESPONSE;
            //no success anyway
            return SDMMC_ERROR_NORESPONSE;
        }

    }
    while ( ( ( masked_SR & HSMCI_SR_XFRDONE ) == 0 ) && ( ++timeout < HSMCI_TIMEOUT2 ) );

    if ( timeout == HSMCI_TIMEOUT2 )
    {
        return SDMMC_ERROR;
    }

    // command is clocking out the SDIO now
    /* read the response */
    /* Store the card response in the provided buffer */
    if ( pCommand->pResp )
    {
        switch ( pCommand->resType )
        {
            case 1:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                resSize = 1;
                break;

            case 2:
                resSize = 4;
                break;

            default:
                resSize = 0;
                break;
        }
        for ( i = 0; i < resSize; i++ )
        {
            pCommand->pResp[i] = pMciHw->HSMCI_RSPR[0];
        }
    }
    if ( ( pCommand->tranType == MCI_START_WRITE ) || ( pCommand->tranType == MCI_WRITE ) )
    {
        pMciHw->HSMCI_PTCR = HSMCI_PTCR_TXTDIS;
    }
    else
    {
        pMciHw->HSMCI_PTCR = HSMCI_PTCR_RXTDIS;
    }
    // all ok
    return SDMMC_OK;
}
