/*----------------------------------------------------------------------------*/
/* Copyright 2016, 2020 NXP                                                   */
/*                                                                            */
/* NXP Confidential. This software is owned or controlled by NXP and may only */
/* be used strictly in accordance with the applicable license terms.          */
/* By expressly accepting such terms or by downloading, installing,           */
/* activating and/or otherwise using the software, you are agreeing that you  */
/* have read, and that you agree to comply with and are bound by, such        */
/* license terms. If you do not agree to be bound by the applicable license   */
/* terms, then you may not retain, install, activate or otherwise use the     */
/* software.                                                                  */
/*----------------------------------------------------------------------------*/

/** \file
* NFC Library Top Level API of Reader Library Framework.
* $Author: jenkins_ cm (nxp92197) $
* $Revision: 4184 $ (v07.09.00)
* $Date: 2016-05-20 18:04:59 +0530 (Fri, 20 May 2016) $
*
* History:
*  KK: Generated 20. May 2016
*/

#include <ph_Status.h>

#ifdef NXPBUILD__PH_NFCLIB_ISO_MFC

#include "phNfcLib_Initialization.h"
#include "phNfcLib_Int.h"

/**
 * Macro for the MIFARE Classic contactless IC Commands
 */
#define PH_NFCLIB_MFC_READ        0x30U    /**< MIFARE Classic Read command byte */
#define PH_NFCLIB_MFC_WRITE       0xA0U    /**< MIFARE Classic Write command byte */
#define PH_NFCLIB_MFC_INCREMENT   0xC1U    /**< MIFARE Classic Increment command byte */
#define PH_NFCLIB_MFC_DECREMENT   0xC0U    /**< MIFARE Classic Decrement command byte */
#define PH_NFCLIB_MFC_RESTORE     0xC2U    /**< MIFARE Classic Restore command byte */
#define PH_NFCLIB_MFC_TRANSFER    0xB0U    /**< MIFARE Classic Transfer command byte */
#define PH_NFCLIB_MFC_PERSOUID    0x40U    /**< MIFARE Classic Personalize UID command */


phStatus_t phNfcLib_MFC_Transmit(void * const pTxBuffer, uint16_t wTxBufferLength)
{
    phStatus_t wStatus = PH_ERR_INVALID_PARAMETER;
    uint8_t    aCommand[2] = {0};
    uint8_t    aKey[PHHAL_HW_MFC_KEY_LENGTH * 2U];
#ifdef NXPBUILD__PH_KEYSTORE_SW
    uint16_t   bKeystoreKeyType;
#endif /* NXPBUILD__PH_KEYSTORE_SW */

    switch(((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bCommand)
    {
    case Read:
        aCommand[0] = PH_NFCLIB_MFC_READ;
        aCommand[1] = ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bBlockNumber;

        wStatus = phhalHw_Exchange(
            gphNfcLib_Params.sDiscLoop.pHalDataParams,
            PH_EXCHANGE_DEFAULT,
            aCommand,
            2,
            &gphNfcLib_State.pRxBuffer,
            &gphNfcLib_State.wRxLength);
        if ((wStatus & PH_ERR_MASK) == PH_ERR_INTEGRITY_ERROR)
        {
            wStatus = PH_ERR_PROTOCOL_ERROR;
        }
        break;

    case Write:
        aCommand[0] = PH_NFCLIB_MFC_WRITE;
        aCommand[1] = ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bBlockNumber;

        PH_CHECK_SUCCESS_FCT(wStatus, phNfcLib_Mifare_Transmit(
            aCommand,
            2));

        wStatus = phNfcLib_Mifare_Transmit(
            ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.pBuffer,
            16);
        break;

    case Increment:
        aCommand[0] = PH_NFCLIB_MFC_INCREMENT;
        aCommand[1] = ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bBlockNumber;


        PH_CHECK_SUCCESS_FCT(wStatus, phNfcLib_Mifare_Transmit(
            aCommand,
            2));

        wStatus = phNfcLib_Mifare_Transmit(
            ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.pBuffer,
            4);
        if((wStatus & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
        {
            return PH_ERR_SUCCESS;
        }
        break;

    case Decrement:
        aCommand[0] = PH_NFCLIB_MFC_DECREMENT;
        aCommand[1] = ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bBlockNumber;

        PH_CHECK_SUCCESS_FCT(wStatus, phNfcLib_Mifare_Transmit(
            aCommand,
            2));

        wStatus = phNfcLib_Mifare_Transmit(
            ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.pBuffer,
            4);
        if((wStatus & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
        {
            return PH_ERR_SUCCESS;
        }
        break;

    case Restore:
        (void)memset(&gphNfcLib_State.bHalBufferRx[0], 0x00, 4);
        aCommand[0] = PH_NFCLIB_MFC_RESTORE;
        aCommand[1] = ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bBlockNumber;

        PH_CHECK_SUCCESS_FCT(wStatus, phNfcLib_Mifare_Transmit(
            aCommand,
            2));

        wStatus = phNfcLib_Mifare_Transmit(
            &gphNfcLib_State.bHalBufferRx[0],
            4);
        if((wStatus & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
        {
            return PH_ERR_SUCCESS;
        }
        break;

    case Transfer:
        aCommand[0] = PH_NFCLIB_MFC_TRANSFER;
        aCommand[1] = ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bBlockNumber;

        PH_CHECK_SUCCESS_FCT(wStatus, phNfcLib_Mifare_Transmit(
            aCommand,
            2));
        break;

    case Authenticate:
#ifdef  NXPBUILD__PH_KEYSTORE_SW
        PH_CHECK_SUCCESS_FCT(wStatus, phKeyStore_GetKey(
            &gphNfcLib_Params.sKeyStore,
            ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bKeyNumber,
            ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bKeyVersion,
            (uint8_t)(sizeof(aKey)),
            &aKey[0],
            &bKeystoreKeyType));
#endif /* NXPBUILD__PH_KEYSTORE_SW */

        wStatus = phhalHw_MfcAuthenticate(
            gphNfcLib_Params.sDiscLoop.pHalDataParams,
            ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bBlockNumber,
            ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bKeyType,
            &aKey[0],
            (gphNfcLib_State.pPeerInfo->uTi.uInitiator.tIso14443_3a.bUidLength == 4U) ? (&gphNfcLib_State.pPeerInfo->uTi.uInitiator.tIso14443_3a.aUid[0]) : (&gphNfcLib_State.pPeerInfo->uTi.uInitiator.tIso14443_3a.aUid[3])
            );
        break;

    case PersonalizeUID:
        aCommand[0] = PH_NFCLIB_MFC_PERSOUID;
        aCommand[1] = ((phNfcLib_Transmit_t *)pTxBuffer)->phNfcLib_MifareClassic.bUidType;

        wStatus = phNfcLib_Mifare_Transmit(
            aCommand,
            2);
        break;

    default:
        break;
    }
    PH_UNUSED_VARIABLE(wTxBufferLength);
    return wStatus;
}

#endif /* NXPBUILD__PH_NFCLIB_ISO_MFC*/
