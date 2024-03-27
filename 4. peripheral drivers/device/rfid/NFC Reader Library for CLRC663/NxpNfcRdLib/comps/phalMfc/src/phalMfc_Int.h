/*----------------------------------------------------------------------------*/
/* Copyright 2009-2020 NXP                                                   */
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
* Internal functions for MIFARE Classic contactless IC (R) application layer.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHALMFC_INT_H
#define PHALMFC_INT_H

#include <ph_Status.h>

/** \addtogroup ph_Private
* @{
*/

#define PHAL_MFC_RESP_ACK       0x0AU    /**< MIFARE Classic ACK response code */
#define PHAL_MFC_RESP_NAK0      0x00U    /**< MIFARE Classic NAK0 response code */
#define PHAL_MFC_RESP_NAK1      0x01U    /**< MIFARE Classic NAK1 response code */
#define PHAL_MFC_RESP_NAK4      0x04U    /**< MIFARE Classic NAK4 response code */
#define PHAL_MFC_RESP_NAK5      0x05U    /**< MIFARE Classic NAK5 response code */

#define PHAL_MFC_CMD_RESTORE    0xC2U    /**< MIFARE Classic Restore command byte */
#define PHAL_MFC_CMD_INCREMENT  0xC1U    /**< MIFARE Classic Increment command byte */
#define PHAL_MFC_CMD_DECREMENT  0xC0U    /**< MIFARE Classic Decrement command byte */
#define PHAL_MFC_CMD_TRANSFER   0xB0U    /**< MIFARE Classic Transfer command byte */
#define PHAL_MFC_CMD_READ       0x30U    /**< MIFARE Classic Read command byte */
#define PHAL_MFC_CMD_WRITE      0xA0U    /**< MIFARE Classic Write command byte */
#define PHAL_MFC_CMD_AUTHA      0x60U    /**< MIFARE Classic Authenticate A command byte */
#define PHAL_MFC_CMD_AUTHB      0x61U    /**< MIFARE Classic Authenticate B command byte */
#define PHAL_MFC_CMD_PERSOUID   0x40U    /**< MIFARE Classic Personalize UID command */
#define PHAL_MFC_CMD_GETVERSION 0x62U    /**< MIFARE Classic GET VERSION command */

/**
* \brief Perform MIFARE product Transfer command with MIFARE product-based PICC.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMfc_Int_Transfer(
                                void * pPalMifareDataParams,    /**< [In] Pointer to a palMifare component context. */
                                uint8_t bBlockNo                /**< [In] block number the transfer buffer shall be transferred to. */
                                );

/**
* \brief Perform a MIFARE product Value operation with MIFARE product-based PICC.
*
* This is used by INCREMENT, DECREMENT and RESTORE commands.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMfc_Int_Value(
                             void * pPalMifareDataParams,   /**< [In] Pointer to a palMifare component context. */
                             uint8_t bCmdCode,              /**< [In] Command Code - INC/DEC or RST. */
                             uint8_t bBlockNo,              /**< [In] Block Number. */
                             uint8_t * pValue               /**< [In] Value; (Needs to be zero in case of RST command). */
                             );

/**
* \brief Create a Value block for a given value/addr pair.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMfc_Int_CreateValueBlock(
                                        uint8_t * pValue,   /**< [In] Value to be converted. */
                                        uint8_t bAddrData,  /**< [In] bAddrData containing destination address. */
                                        uint8_t * pBlock    /**< [Out] Formatted Value block. */
                                        );

/**
* \brief Check value block format of a given block.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMfc_Int_CheckValueBlockFormat(
    uint8_t * pBlock    /**< [In] Formatted Value block. */
    );

/**
* \brief Resolve MIFARE product error code from response.
* \return Status code
* \retval #PH_ERR_SUCCESS ACK received.
* \retval #PHPAL_MIFARE_ERR_NAK0 NAK0 received.
* \retval #PHPAL_MIFARE_ERR_NAK1 NAK1 received.
* \retval #PHPAL_MIFARE_ERR_NAK4 NAK4 received.
* \retval #PHPAL_MIFARE_ERR_NAK5 NAK5 received.
* \retval #PH_ERR_PROTOCOL_ERROR Unspecified response.
*/
phStatus_t phalMfc_Int_ResolveReturnCode(
    uint8_t bRetCode   /**< [In] Return Code byte from Card. */
    );

/** @}
* end of ph_Private group
*/

#endif /* PHALMFC_INT_H */
