/*----------------------------------------------------------------------------*/
/* Copyright 2009, 2020 NXP                                                   */
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
* Generic MIFARE Plus contactless IC Application Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
* History:
*  CHu: Generated 31. August 2009
*
*/

#ifndef PHALMFP_H
#define PHALMFP_H

#include <ph_Status.h>
#include <phhalHw.h>
#include <phpalMifare.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifdef NXPBUILD__PHAL_MFP

/** \defgroup phalMfp MIFARE Plus
* \brief These Components implement the MIFARE Plus commands.
* @{
*/

/**
* \name Authentication Parameters
*/
/*@{*/
#define PHAL_MFP_KEYA           0x0AU   /**< MIFARE product Key A. */
#define PHAL_MFP_KEYB           0x0BU   /**< MIFARE product Key B. */
/*@}*/

/** \name Custom Error Codes
*/
/*@{*/
#define PHAL_MFP_ERR_AUTH           ((phStatus_t)PH_ERR_CUSTOM_BEGIN + 0U)     /*< MFP Authentication Error. */
#define PHAL_MFP_ERR_CMD_OVERFLOW   ((phStatus_t)PH_ERR_CUSTOM_BEGIN + 1U)     /*< MFP Command Overflow Error. */
#define PHAL_MFP_ERR_MAC_PCD        ((phStatus_t)PH_ERR_CUSTOM_BEGIN + 2U)     /*< MFP MAC Error. */
#define PHAL_MFP_ERR_BNR            ((phStatus_t)PH_ERR_CUSTOM_BEGIN + 3U)     /*< MFP Blocknumber Error. */
#define PHAL_MFP_ERR_EXT            ((phStatus_t)PH_ERR_CUSTOM_BEGIN + 4U)     /*< MFP Extension Error. */
#define PHAL_MFP_ERR_CMD_INVALID    ((phStatus_t)PH_ERR_CUSTOM_BEGIN + 5U)     /*< MFP Invalid Command Error. */
#define PHAL_MFP_ERR_FORMAT         ((phStatus_t)PH_ERR_CUSTOM_BEGIN + 6U)     /*< MFP Authentication Error. */
#define PHAL_MFP_ERR_GEN_FAILURE    ((phStatus_t)PH_ERR_CUSTOM_BEGIN + 7U)     /*< MFP Generic Error. */
/*@}*/

/**
* \name Sizes
*/
/*@{*/
#define PHAL_MFP_SIZE_TI            4U  /**< Size of Transaction Identifier. */
#define PHAL_MFP_SIZE_KEYMODIFIER   6U  /**< Size of MIFARE Plus contactless IC KeyModifier. */
#define PHAL_MFP_SIZE_MAC           16U /**< Size of (untruncated) MAC. */
/*@}*/

/** @} */
#endif /* NXPBUILD__PHAL_MFP */

#ifdef NXPBUILD__PHAL_MFP_SW

/** \defgroup phalMfp_Sw Component : Software
* @{
*/

#define PHAL_MFP_SW_ID          0x01U    /**< ID for Software MIFARE Plus layer. */

/**
* \brief MIFARE Plus Software parameter structure
*/
typedef struct
{
    uint16_t wId;                                           /**< Layer ID for this component, NEVER MODIFY! */
    void * pPalMifareDataParams;                            /**< Pointer to the parameter structure of the palMifare component. */
    void * pKeyStoreDataParams;                             /**< Pointer to the parameter structure of the KeyStore layer. */
    void * pCryptoDataParamsEnc;                            /**< Pointer to the parameter structure of the Crypto layer for encryption. */
    void * pCryptoDataParamsMac;                            /**< Pointer to the parameter structure of the Crypto layer for macing. */
    void * pCryptoRngDataParams;                            /**< Pointer to the parameter structure of the CryptoRng layer. */
    void * pCryptoDiversifyDataParams;                      /**< Pointer to the parameter structure of the CryptoDiversify layer (can be NULL). */
    uint8_t bKeyModifier[PHAL_MFP_SIZE_KEYMODIFIER];        /**< Key Modifier for MIFARE Plus SL2 authentication. */
    uint16_t wRCtr;                                         /**< R_CTR (read counter); The PICC's read counter is used for a following authentication. */
    uint16_t wWCtr;                                         /**< W_CTR (write counter); The PICC's write counter is used for a following authentication. */
    uint8_t bTi[PHAL_MFP_SIZE_TI];                          /**< Transaction Identifier; unused if 'bFirstAuth' = 1; uint8_t[4]. */
    uint8_t bNumUnprocessedReadMacBytes;                    /**< Amount of data in the pUnprocessedReadMacBuffer. */
    uint8_t pUnprocessedReadMacBuffer[PHAL_MFP_SIZE_MAC];   /**< Buffer containing unprocessed bytes for read mac answer stream. */
    uint8_t pIntermediateMac[PHAL_MFP_SIZE_MAC];            /**< Intermediate MAC for Read Calculation. */
    uint8_t bFirstRead;                                     /**< Indicates whether the next read is a first read in a read (MACed) sequence or not. */
} phalMfp_Sw_DataParams_t;

/**
* \brief Initialise this layer.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMfp_Sw_Init(
                           phalMfp_Sw_DataParams_t * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                           uint16_t wSizeOfDataParams,                  /**< [In] Specifies the size of the data parameter structure. */
                           void * pPalMifareDataParams,             /**< [In] Pointer to a palMifare component context. */
                           void * pKeyStoreDataParams,              /**< [In] Pointer to a KeyStore component context. */
                           void * pCryptoDataParamsEnc,             /**< [In] Pointer to a Crypto component context for encryption. */
                           void * pCryptoDataParamsMac,             /**< [In] Pointer to a Crypto component context for Macing. */
                           void * pCryptoRngDataParams,             /**< [In] Pointer to a CryptoRng component context. */
                           void * pCryptoDiversifyDataParams        /**< [In] Pointer to the parameter structure of the CryptoDiversify layer (can be NULL). */
                           );

/** @} */
#endif /* NXPBUILD__PHAL_MFP_SW */



#ifdef NXPBUILD__PHAL_MFP

/** \addtogroup phalMfp
* @{
*/

/**
* \name Security Level 0
*/
/*@{*/

/**
* \brief Performs a MIFARE Plus Write Perso command.
*
* The Write Perso command can be executed using the ISO14443-3 communication protocol (after layer 3 activation)\n
* or using the ISO14443-4 protocol (after layer 4 activation)\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_WritePerso(
                              void * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                              uint8_t bLayer4Comm,  /**< [In] \c 0: use ISO14443-3 protocol; \c 1: use ISO14443-4 protocol. */
                              uint16_t wBlockNr,    /**< [In] MIFARE Plus Block number. */
                              uint8_t * pValue      /**< [In] Value (16 bytes). */
                              );

/**
* \brief Performs a MIFARE Plus Commit Perso command.
*
* The Commit Perso command can be executed using the ISO14443-3 communication protocol (after layer 3 activation)\n
* or using the ISO14443-4 protocol (after layer 4 activation).\n
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_CommitPerso(
                               void * pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                               uint8_t bLayer4Comm  /**< [In] \c 0: use ISO14443-3 protocol; \c 1: use ISO14443-4 protocol; */
                               );


/**
* \brief Perform MIFARE Plus Authenticate command in Security Level 2 with MIFARE product-based PICC.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_AuthenticateClassicSL2(
    void * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
    uint8_t bBlockNo,       /**< [In] Block number on Card to authenticate to. */
    uint8_t bKeyType,       /**< [In] Either /ref PHHAL_HW_MFC_KEYA or /ref PHHAL_HW_MFC_KEYB. */
    uint16_t wKeyNumber,    /**< [In] Key number to be used in authentication. */
    uint16_t wKeyVersion,   /**< [In] Key version to be used in authentication. */
    uint8_t * pUid,         /**< [In] UID. */
    uint8_t bUidLength      /**< [In] UID length provided. */
    );

/**
* \brief Performs a Multi Block Read command.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_MultiBlockRead(
                                  void * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                                  uint8_t bBlockNr,     /**< [In] MIFARE Plus block number. */
                                  uint8_t bNumBlocks,   /**< [In] Number of blocks to read (must not be more than 3). */
                                  uint8_t * pBlocks     /**< [Out] Block(s) (16u*bNumBlocks bytes). */
                                  );

/**
* \brief Performs a Multi Block Write command.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_MultiBlockWrite(
                                   void * pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                                   uint8_t bBlockNr,    /**< [In] MIFARE Plus block number. */
                                   uint8_t bNumBlocks,  /**< [In] Number of blocks to write (must not be more than 3). */
                                   uint8_t * pBlocks    /**< [In] Block(s) (16u*bNumBlocks bytes). */
                                   );

/** @} */



/**
* \brief Performs a Reset Auth command.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_ResetAuth(
                             void * pDataParams     /**< [In] Pointer to this layer's parameter structure. */
                             );

/** @} */


/**
* \name Miscellaneous Functions
*/
/*@{*/

/**
* \brief Reset the libraries internal secure messaging state.
*
* This function must be called before interacting with the PICC to set the libraries internal card-state back to default.\n
* E.g. when an error occurred or after a reset of the field.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_ResetSecMsgState(
                                    void * pDataParams  /**< [In] Pointer to this layer's parameter structure. */
                                    );

/** @} */

/** @} */
#endif /* NXPBUILD__PHAL_MFP */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHMFPLIB_H */
