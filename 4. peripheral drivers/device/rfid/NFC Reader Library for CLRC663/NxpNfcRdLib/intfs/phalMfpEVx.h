/*----------------------------------------------------------------------------*/
/* Copyright 2013-2021 NXP                                                    */
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
* Generic MIFARE Plus EVx contactless IC (Ev1, and future versions) contactless IC Application Component of Reader Library Framework.
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*
*/

#ifndef PHALMFPEVX_H
#define PHALMFPEVX_H

#include <ph_Status.h>
#include <phTMIUtils.h>
#include <phhalHw.h>
#include <phpalMifare.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* Macro to represent the AES block numbers. This macros are for internal use. */
#define PHAL_MFPEVX_ORIGINALITY_KEY_0                               0x8000U /**< Block numbers for Originality Key 0. */
#define PHAL_MFPEVX_ORIGINALITY_KEY_1                               0x8001U /**< Block numbers for Originality Key 1. */
#define PHAL_MFPEVX_ORIGINALITY_KEY_2                               0x8002U /**< Block numbers for Originality Key 2. */
#define PHAL_MFPEVX_ORIGINALITY_KEY_3                               0x8003U /**< Block numbers for Originality Key 3. */
#define PHAL_MFPEVX_L3SWITCHKEY                                     0x9003U /**< Block numbers for Level 3 switch key (SL3 switching) . */
#define PHAL_MFPEVX_SL1CARDAUTHKEY                                  0x9004U /**< Block numbers for SL1 card authentication key. */
#define PHAL_MFPEVX_L3SECTORSWITCHKEY                               0x9006U /**< Block numbers for L3 sector switch key (switching the sector). */
#define PHAL_MFPEVX_L1L3MIXSECTORSWITCHKEY                          0x9007U /**< Block numbers for L1L3 sector switch key (switching the sector to L1L3MIX). */
#define PHAL_MFPEVX_ORIGINALITY_KEY_FIRST   PHAL_MFPEVX_ORIGINALITY_KEY_0   /**< Macro to represent the first block number of originality key. */
#define PHAL_MFPEVX_ORIGINALITY_KEY_LAST    PHAL_MFPEVX_ORIGINALITY_KEY_3   /**< Macro to represent the last block number of originality key. */


#ifdef NXPBUILD__PHAL_MFPEVX_SW
/***************************************************************************************************************************************/
/* Software Dataparams and Initialization Interface.                                                                                   */
/***************************************************************************************************************************************/

#define PHAL_MFPEVX_SW_ID                                           0x01U   /**< ID for Software layer implementation of MIFARE Plus EVx product. */

/* Macro to represent the buffer length. This macros are for internal use. */
#define PHAL_MFPEVX_SIZE_TI                                             4U  /**< Size of Transaction Identifier. */
#define PHAL_MFPEVX_SIZE_TMC                                            4U  /**< Size of the transaction MAC counter */
#define PHAL_MFPEVX_SIZE_TMV                                            8U  /**< Size of the transaction MAC vale */
#define PHAL_MFPEVX_SIZE_IV                                             16U /**< Size of Initialization vector. */
#define PHAL_MFPEVX_SIZE_TMRI                                           16U /**< Size of TMRI */
#define PHAL_MFPEVX_SIZE_ENCTMRI                                        16U /**< Size of encrypted transaction MAC reader ID */
#define PHAL_MFPEVX_SIZE_KEYMODIFIER                                    6U  /**< Size of MIFARE KeyModifier. */
#define PHAL_MFPEVX_SIZE_MAC                                            16U /**< Size of (untruncated) MAC. */

#define PHAL_MFPEVX_VERSION_COMMAND_LENGTH                              41U /**< Version command buffer size. Size = Status(1) + R_Ctr(2) + TI(4) + VersionA(7) + VersionB(7) + VersionC(20) */
#define PHAL_MFPEVX_VERSION_INFO_LENGTH                                 33U /**< Version buffer size to store the complete information. Size = VersionA(7) + VersionB(7) + VersionC(20) */
#define PHAL_MFPEVX_VERSION_PART1_LENGTH                                07U /**< Version part 1 length in the received response. */
#define PHAL_MFPEVX_VERSION_PART2_LENGTH                                07U /**< Version part 2 length in the received response. */

#define PHAL_MFPEVX_VERSION_PART3_LENGTH_04B                            13U /**< Version part 3 length in the received response in case of 4 byte UID. */
#define PHAL_MFPEVX_VERSION_PART3_LENGTH_07B                            14U /**< Version part 3 length in the received response in case of 7 byte UID. */
#define PHAL_MFPEVX_VERSION_PART3_LENGTH_10B                            19U /**< Version part 3 length in the received response in case of 10 byte UID. */

/** \defgroup phalMfpEVx_Sw Component : Software
 * @{
 */

/** \brief MIFARE Plus EVx Software parameter structure. */
typedef struct
{
    uint16_t wId;                                                           /**< Layer ID for this component, NEVER MODIFY! */
    void * pPalMifareDataParams;                                            /**< Pointer to the parameter structure of the palMifare component. */
    void * pKeyStoreDataParams;                                             /**< Pointer to the parameter structure of the KeyStore layer. */
    void * pCryptoDataParamsEnc;                                            /**< Pointer to the parameter structure of the Crypto layer for encryption. */
    void * pCryptoDataParamsMac;                                            /**< Pointer to the parameter structure of the Crypto layer for macing. */
    void * pCryptoRngDataParams;                                            /**< Pointer to the parameter structure of the CryptoRng layer. */
    void * pCryptoDiversifyDataParams;                                      /**< Pointer to the parameter structure of the CryptoDiversify layer (can be NULL). */
    void * pTMIDataParams;                                                  /**< Pointer to the parameter structure for collecting TMI. */
    void * pVCADataParams;                                                  /**< Pointer to the parameter structure for Virtual Card. */
    uint16_t wRCtr;                                                         /**< R_CTR (read counter); The PICC's read counter is used for a following authentication. */
    uint16_t wWCtr;                                                         /**< W_CTR (write counter); The PICC's write counter is used for a following authentication. */
    uint8_t bWrappedMode;                                                   /**< Wrapped APDU mode. All native commands need to be sent wrapped in ISO 7816 APDUs. */
    uint8_t bExtendedLenApdu;                                               /**< Extended length APDU. If set the native commands should be wrapped in extended format */
    uint8_t bTi[PHAL_MFPEVX_SIZE_TI];                                       /**< Transaction Identifier; unused if 'bFirstAuth' = 1; uint8_t[4]. */
    uint8_t bNumUnprocessedReadMacBytes;                                    /**< Amount of data in the pUnprocessedReadMacBuffer. */
    uint8_t pUnprocessedReadMacBuffer[PHAL_MFPEVX_SIZE_MAC];                /**< Buffer containing unprocessed bytes for read mac answer stream. */
    uint8_t pIntermediateMac[PHAL_MFPEVX_SIZE_MAC];                         /**< Intermediate MAC for Read Calculation. */
    uint8_t bFirstRead;                                                     /**< Indicates whether the next read is a first read in a read (MACed) sequence or not. */
    uint8_t bIv[16];                                                        /**< Initialization vector. Max size of IV can be 16 bytes */
    uint8_t bSesAuthENCKey[16];                                             /**< Authentication ENC key for the session. */
    uint8_t bSesAuthMACKey[16];                                             /**< Authentication MAC key for the session. */
    uint8_t bAuthMode;                                                      /**< Security level authenticate */
    uint8_t bSMMode;                                                        /**< Secure messaging mode. \c 0: EV0 Mode; \c 1: EVx mode */
} phalMfpEVx_Sw_DataParams_t;

/**
 * \brief Initializes the AL component as software component.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_Sw_Init(
        phalMfpEVx_Sw_DataParams_t * pDataParams,                           /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wSizeOfDataParams,                                         /**< [In] Specifies the size of the data parameter structure. */
        void * pPalMifareDataParams,                                        /**< [In] Pointer to a palMifare data parameter structure. */
        void * pKeyStoreDataParams,                                         /**< [In] Pointer to a KeyStore data parameter structure. */
        void * pCryptoDataParamsEnc,                                        /**< [In] Pointer to a Crypto data parameter structure for encryption. */
        void * pCryptoDataParamsMac,                                        /**< [In] Pointer to a Crypto data parameter structure for Macing. */
        void * pCryptoRngDataParams,                                        /**< [In] Pointer to a CryptoRng data parameter structure. */
        void * pCryptoDiversifyDataParams,                                  /**< [In] Pointer to the parameter structure of the CryptoDiversify layer (can be NULL). */
        void * pTMIDataParams,                                              /**< [In] Pointer to a TMI data parameter structure. */
        void * pVCADataParams                                               /**< [In] Pointer to the parameter structure for Virtual Card. */
    );

/**
 * end of group phalMfpEVx_Sw
 * @}
 */
#endif /* NXPBUILD__PHAL_MFPEVX_SW */









/***************************************************************************************************************************************/
/* Mifare Plus EVx Generic command declarations.                                                                                       */
/***************************************************************************************************************************************/
#ifdef NXPBUILD__PHAL_MFPEVX

/** \defgroup phalMfpEVx MIFARE Plus EVx
 * \brief These Components implement the MIFARE Plus (EVx,  EV2, future versions) commands.
 * @{
 */

/** \defgroup phalMfpEVx_Errors ErrorCodes
 * \brief These component implement the MIFARE Plus (EVx,  EV2, future versions) Error codes.
 * @{
 */

/**
 * \defgroup phalMfpEVx_PICC_Errors PICC ErrorCodes
 * \brief These Components implement the actual PICC error codes.
 * These are the values that will not be returned to the user from the interface in case of error.
 * @{
 */
/** \name PICC response codes. */
/** @{ */
#define PHAL_MFPEVX_RESP_NACK0                                  0x00U   /**< MFP NACK 0 (in ISO14443-3 mode). */
#define PHAL_MFPEVX_RESP_NACK1                                  0x01U   /**< MFP NACK 1 (in ISO14443-3 mode). */
#define PHAL_MFPEVX_RESP_NACK4                                  0x04U   /**< MFP NACK 4 (in ISO14443-3 mode). */
#define PHAL_MFPEVX_RESP_NACK5                                  0x05U   /**< MFP NACK 5 (in ISO14443-3 mode). */
#define PHAL_MFPEVX_RESP_ACK_ISO3                               0x0AU   /**< MFP ACK (in ISO14443-3 mode). */
#define PHAL_MFPEVX_RESP_ACK_ISO4                               0x90U   /**< MFP ACK (in ISO14443-4 mode). */
#define PHAL_MFPEVX_RESP_ERR_TM                                 0x05U   /**< MFP Tranaction MAC related Error. */
#define PHAL_MFPEVX_RESP_ERR_AUTH                               0x06U   /**< MFP Authentication Error. */
#define PHAL_MFPEVX_RESP_ERR_CMD_OVERFLOW                       0x07U   /**< MFP Command Overflow Error. */
#define PHAL_MFPEVX_RESP_ERR_MAC_PCD                            0x08U   /**< MFP MAC Error. */
#define PHAL_MFPEVX_RESP_ERR_BNR                                0x09U   /**< MFP Blocknumber Error. */
#define PHAL_MFPEVX_RESP_ERR_EXT                                0x0AU   /**< MFP Extension Error. */
#define PHAL_MFPEVX_RESP_ERR_CMD_INVALID                        0x0BU   /**< MFP Invalid Command Error. */
#define PHAL_MFPEVX_RESP_ERR_FORMAT                             0x0CU   /**< MFP Format Error. */
#define PHAL_MFPEVX_RESP_ERR_NOT_SUP                            0x0DU   /**< MFP Not Supported Error. */
#define PHAL_MFPEVX_RESP_ERR_GEN_FAILURE                        0x0FU   /**< MFP Generic Error. */
#define PHAL_MFPEVX_RESP_ADDITIONAL_FRAME                       0xAFU   /**< MFP Additional data frame is expected to be sent. */
/** @} */

/** \name ISO 7816-4 error codes. */
/** @{ */
#define PHAL_MFPEVX_ISO7816_RESP_SUCCESS                        0x9000U /**< Correct execution. */
#define PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LENGTH               0x6700U /**< Wrong length. */
#define PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_PARAMS               0x6A86U /**< Wrong parameters P1 and/or P2. */
#define PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LC                   0x6A87U /**< Lc inconsistent with P1/p2. */
#define PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LE                   0x6C00U /**< Wrong Le. */
#define PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_CLA                  0x6E00U /**< Wrong Class byte. */
/** @} */

/**
 * end of group phalMfpEVx_PICC_Errors
 * @}
 */

/** \defgroup phalMfpEVx_Cust_Errors Custom ErrorCodes
 * \brief These Components implement the custom error codes mapped to PICC return codes.
 * These are the values that will be returned to the user from the interface in case of error.
 * @{
 */

/** \name Custom Error Codes mapping for PICC erro codes. */
/** @{ */
#define PHAL_MFPEVX_ERR_AUTH                    (PH_ERR_CUSTOM_BEGIN + 0U)  /**< MFP EVx Authentication Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_AUTH error. */
#define PHAL_MFPEVX_ERR_CMD_OVERFLOW            (PH_ERR_CUSTOM_BEGIN + 1U)  /**< MFP EVx Command Overflow Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_CMD_OVERFLOW error. */
#define PHAL_MFPEVX_ERR_MAC_PCD                 (PH_ERR_CUSTOM_BEGIN + 2U)  /**< MFP EVx MAC Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_MAC_PCD error. */
#define PHAL_MFPEVX_ERR_BNR                     (PH_ERR_CUSTOM_BEGIN + 3U)  /**< MFP EVx Blocknumber Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_BNR error. */
#define PHAL_MFPEVX_ERR_EXT                     (PH_ERR_CUSTOM_BEGIN + 4U)  /**< MFP EVx Extension Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_EXT error. */
#define PHAL_MFPEVX_ERR_CMD_INVALID             (PH_ERR_CUSTOM_BEGIN + 5U)  /**< MFP EVx Invalid Command Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_CMD_INVALID error. */
#define PHAL_MFPEVX_ERR_FORMAT                  (PH_ERR_CUSTOM_BEGIN + 6U)  /**< MFP EVx Authentication Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_FORMAT error. */
#define PHAL_MFPEVX_ERR_GEN_FAILURE             (PH_ERR_CUSTOM_BEGIN + 7U)  /**< MFP EVx Generic Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_GEN_FAILURE error. */
#define PHAL_MFPEVX_ERR_TM                      (PH_ERR_CUSTOM_BEGIN + 8U)  /**< MFP EVx Transaction MAC related Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_TM error. */
#define PHAL_MFPEVX_ERR_NOT_SUP                 (PH_ERR_CUSTOM_BEGIN + 9U)  /**< MFP EVx Not Supported Error. This error represents PICC's #PHAL_MFPEVX_RESP_ERR_NOT_SUP error. */
#define PHAL_MFPEVX_ISO7816_ERR_WRONG_LENGTH    (PH_ERR_CUSTOM_BEGIN + 10U) /**< MFP EVx 7816 wrong length error. This error represents PICC's #PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LENGTH error. */
#define PHAL_MFPEVX_ISO7816_ERR_WRONG_PARAMS    (PH_ERR_CUSTOM_BEGIN + 11U) /**< MFP EVx 7816 wrong params error. This error represents PICC's #PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_PARAMS error. */
#define PHAL_MFPEVX_ISO7816_ERR_WRONG_LC        (PH_ERR_CUSTOM_BEGIN + 12U) /**< MFP EVx 7816 wrong Lc error. This error represents PICC's #PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LC error. */
#define PHAL_MFPEVX_ISO7816_ERR_WRONG_LE        (PH_ERR_CUSTOM_BEGIN + 13U) /**< MFP EVx 7816 wrong LE error. This error represents PICC's #PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_LE error. */
#define PHAL_MFPEVX_ISO7816_ERR_WRONG_CLA       (PH_ERR_CUSTOM_BEGIN + 14U) /**< MFP EVx 7816 wrong CLA error. This error represents PICC's #PHAL_MFPEVX_ISO7816_RESP_ERR_WRONG_CLA error. */
/** @} */

/**
 * end of group phalMfpEVx_Cust_Errors
 * @}
 */

/**
 * end of group phalMfpEVx_Errors
 * @}
 */

/** \defgroup phalMfpEVx_CommonDefs Common Definitions
 * \brief These are common definitions for most of the Plus commands.
 * @{
 */

/** \name Options to indicate the ISO14443 protocol layer to be used. */
/** @{ */
#define PHAL_MFPEVX_ISO14443_L3                                     0x00U   /**< Option to use Iso14443 Layer 3 protocol. */
#define PHAL_MFPEVX_ISO14443_L4                                     0x01U   /**< Option to use Iso14443 Layer 4 protocol. */
/** @} */

/** \name Options to indicate the communication mode. */
/** @{ */
#define PHAL_MFPEVX_ENCRYPTION_OFF                                  0x00U   /**< Option to indicate the communication between PCD and PICC is plain. */
#define PHAL_MFPEVX_ENCRYPTION_ON                                   0x01U   /**< Option to indicate the communication between PCD and PICC is encrypted. */
/** @} */

/** \name Options to indicate the communication mode as maced for PCD to PICC transfer. */
/** @{ */
#define PHAL_MFPEVX_MAC_ON_COMMAND_OFF                              0x00U   /**< Option to indicate the communication is not maced for PCD to PICC transfer. */
#define PHAL_MFPEVX_MAC_ON_COMMAND_ON                               0x01U   /**< Option to indicate the communication is maced for PCD to PICC transfer. */
/** @} */

/** \name Options to indicate the communication mode as maced for PICC to PCD transfer. */
/** @{ */
#define PHAL_MFPEVX_MAC_ON_RESPONSE_OFF                             0x00U   /**< Option to indicate the communication is not maced for PICC to PCD transfer. */
#define PHAL_MFPEVX_MAC_ON_RESPONSE_ON                              0x01U   /**< Option to indicate the communication is maced for PICC to PCD transfer. */
/** @} */

/**
 * end of group phalMfpEVx_CommonDefs
 * @}
 */


/* Mifare Plus EVx personalization commands. ----------------------------------------------------------------------------------------- */
/** \defgroup phalMfpEVx_Personalization Commands_Personalization
 * \brief These Components implement the MIFARE Plus EVx personalization commands.
 * @{
 */

/**
 * \brief Performs a Write Perso command. The Write Perso command can be executed using the ISO14443-3 communication protocol
 * (after layer 3 activation) or using the ISO14443-4 protocol (after layer 4 activation).
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_WritePerso(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bLayer4Comm,                                                /**< [In] ISO14443 protocol to be used.
                                                                             *          \arg #PHAL_MFPEVX_ISO14443_L3
                                                                             *          \arg #PHAL_MFPEVX_ISO14443_L4
                                                                             */
        uint16_t wBlockNr,                                                  /**< [In] Block number to be personalized. */
        uint8_t bNumBlocks,                                                 /**< [In] Number of blocks to be personalized. \n
                                                                             *          \c 15 block for Native communication if configured as Software component. \n
                                                                             *          \c 15 block for Native communication if configured as Sam NonX component. \n
                                                                             *          \c 13 blocks if configured as Sam X component.
                                                                             */
        uint8_t * pValue                                                    /**< [In] The value for the block mentioned in BlockNr parameter. \n
                                                                             *        If number of blocks is 1, the length should be 16 bytes. \n
                                                                             *        If number of blocks more than 1, the length should be (NoBlocks * 16) bytes.
                                                                             */
    );

/** \name Options to switch the Security Level to 1 or 3. */
/** @{ */
#define PHAL_MFPEVX_MAINTIAN_BACKWARD_COMPATIBILITY                 0x00U   /**< Option to maintain the backward compatibility with Mifare Plus PICC. */
#define PHAL_MFPEVX_SWITCH_SECURITY_LEVEL_1                         0x01U   /**< Option to switch the Security Level to 1. */
#define PHAL_MFPEVX_SWITCH_SECURITY_LEVEL_3                         0x03U   /**< Option to switch the Security Level to 3. */
/** @} */

/**
 * \brief Performs a Commit Perso command. The Commit Perso command can be executed using the ISO14443-3 communication protocol
 * (after layer 3 activation) or using the ISO14443-4 protocol (after layer 4 activation). This command commits the written
 * data during WritePerso command and switches the SecurityLevel to 1 or 3 based on the option provided.
 *
 * If the Option parameter is 0, only the command code will be exchanges to PICC. This is to maintain the backward
 * compatibility with Mifare Plus PICC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_CommitPerso(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                    /**< [In] Option to be used for Security Level switching.
                                                                             *          \arg #PHAL_MFPEVX_MAINTIAN_BACKWARD_COMPATIBILITY
                                                                             *          \arg #PHAL_MFPEVX_SWITCH_SECURITY_LEVEL_1
                                                                             *          \arg #PHAL_MFPEVX_SWITCH_SECURITY_LEVEL_3
                                                                             */
        uint8_t bLayer4Comm                                                 /**< [In] ISO14443 protocol to be used.
                                                                             *          \arg #PHAL_MFPEVX_ISO14443_L3
                                                                             *          \arg #PHAL_MFPEVX_ISO14443_L4
                                                                             */
    );

/**
 * end of group phalMfpEVx_Personalization
 * @}
 */


/* Mifare Plus EVx authentication commands. ------------------------------------------------------------------------------------------ */
/** \defgroup phalMfpEVx_Authenticate Commands_Authenticate
 * \brief These Components implement the MIFARE Plus EVx authentication commands.
 * @{
 */

/** \name Key type options for MIFARE Classic contactless IC Authentication. */
/** @{ */
#define PHAL_MFPEVX_KEYA                                            0x0AU   /**< MIFARE(R) Key A. */
#define PHAL_MFPEVX_KEYB                                            0x0BU   /**< MIFARE(R) Key B. */
/** @} */

/**
 * \brief Perform MIFARE Authenticate command in Security Level 1 with MIFARE CLASSIC PICC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_AuthenticateMfc(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bBlockNo,                                                   /**< [In] PICC block number to be used for authentication. */
        uint8_t bKeyType,                                                   /**< [In] Authentication key type to be used.
                                                                             *          \arg #PHAL_MFPEVX_KEYA
                                                                             *          \arg #PHAL_MFPEVX_KEYB
                                                                             */
        uint16_t wKeyNumber,                                                /**< [In] Key number to used from software or hardware keystore. */
        uint16_t wKeyVersion,                                               /**< [In] Key version to used from software or hardware keystore. */
        uint8_t * pUid,                                                     /**< [In] UID of the PICC received during anti-collision sequence. */
        uint8_t bUidLength                                                  /**< [In] Length of the UID buffer. */
    );



/* Mifare Plus EVx special commands. ------------------------------------------------------------------------------------------------- */
/** \defgroup phalMfpEVx_Special Commands_Special
 * \brief These Components implement the MIFARE Plus EVx additional feature commands.
 * @{
 */

/**
 * \brief Returns manufacturing related data of the PICC
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_GetVersion(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pVerInfo                                                  /**< [Out] The version information of the PICC. \n
                                                                             *          \c If UID is 4 bytes, the buffer will have 27 bytes of version information. \n
                                                                             *          \c If UID is 7 bytes, the buffer will have 28 bytes of version information. \n
                                                                             *          \c If UID is 10 bytes, the buffer will have 33 bytes of version information.
                                                                             */
    );

/**
 * \brief Read originality Signature from the PICC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_ReadSign(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bLayer4Comm,                                                /**< [In] ISO14443 protocol to be used.
                                                                             *          \arg #PHAL_MFPEVX_ISO14443_L3
                                                                             *          \arg #PHAL_MFPEVX_ISO14443_L4
                                                                             */
        uint8_t bAddr,                                                      /**< [In] Targeted ECC originality check signature. */
        uint8_t ** pSignature                                               /**< [Out] PICC's orginality signature. The buffer will have 56 bytes of
                                                                             *         signature information.
                                                                             */
    );

/**
 * \brief Performs a Reset Auth command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_ResetAuth(
        void * pDataParams                                                  /**< [In] Pointer to this layer's parameter structure. */
    );

/** \name Options to personalize the UID. */
/** @{ */
#define PHAL_MFPEVX_UID_TYPE_UIDF0                                  0x00U   /**< MIFARE(R) Plus EVx UID type UIDF0. */
#define PHAL_MFPEVX_UID_TYPE_UIDF1                                  0x40U   /**< MIFARE(R) Plus EVx UID type UIDF1. */
#define PHAL_MFPEVX_UID_TYPE_UIDF2                                  0x20U   /**< MIFARE(R) Plus EVx UID type UIDF2. */
#define PHAL_MFPEVX_UID_TYPE_UIDF3                                  0x60U   /**< MIFARE(R) Plus EVx UID type UIDF3. */
/** @} */

/**
 * \brief Perform MIFARE(R) Personalize UID usage command sequence with MIFARE Picc.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_PersonalizeUid(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bUidType                                                    /**< [In] UID type to be used for personalization.
                                                                             *          \arg #PHAL_MFPEVX_UID_TYPE_UIDF0
                                                                             *          \arg #PHAL_MFPEVX_UID_TYPE_UIDF1
                                                                             *          \arg #PHAL_MFPEVX_UID_TYPE_UIDF2
                                                                             *          \arg #PHAL_MFPEVX_UID_TYPE_UIDF3
                                                                             */
    );

/** \name Options to Enable / Disable ISO14443 Layer 4 protocol. */
/** @{ */
#define PHAL_MFPEVX_ENABLE_ISO14443_L4                              0x00U   /**< Option to enable ISO14443 Layer 4 protocol. */
#define PHAL_MFPEVX_DISABLE_ISO14443_L4                             0x01U   /**< Option to disable ISO14443 Layer 4 protocol. */
/** @} */

/**
 * \brief Performs a configuration for ISO1443-4 enabling in Security Level 1.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_SetConfigSL1(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption                                                     /**< [In] Option byte encoding the configuration to be set.
                                                                             *          \arg #PHAL_MFPEVX_ENABLE_ISO14443_L4
                                                                             *          \arg #PHAL_MFPEVX_DISABLE_ISO14443_L4
                                                                             */
    );

/**
 * \brief Performs read of the TMAC block in security layer 1 with ISO14443 Layer 3 activated.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_ReadSL1TMBlock(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wBlockNr,                                                  /**< [In] PICC TM  block number from which the data should be read. */
        uint8_t * pBlocks                                                   /**< [Out] The data read from the specified block number. The buffer will
                                                                             *         contain 16 bytes information.
                                                                             */
    );

/**
 * \brief Performs a VCSupportLastISOL3 command operation. This command is a special variant of virtual card operation.
 * This command can be sent after a ISO14443-3 activation.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_VCSupportLastISOL3(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pIid,                                                     /**< [In] The (last) IID that the PCD presents to the PD. The buffer
                                                                             *        should have 16 bytes of information.
                                                                             */
        uint8_t * pPcdCapL3,                                                /**< [In] Capability vector of the PCD.  The buffer should have 4 bytes
                                                                             *        of information.
                                                                             */
        uint8_t * pInfo                                                     /**< [Out] One byte Information returned by PICC. */
    );


/**
 * end of group phalMfpEVx_Special
 * @}
 */


/* Mifare Plus EVx utilities. -------------------------------------------------------------------------------------------------------- */
/** \defgroup phalMfpEVx_Utilities Utilities
 * \brief These Components implement the utility interfaces required for MIFARE Plus EVx application layer. These are not commands.
 * @{
 */

/**
 * \brief Reset the libraries internal secure messaging state.
 *
 * This function must be called before interacting with the PICC to set the libraries internal card-state back to default.\n
 * E.g. when an error occurred or after a reset of the field.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_ResetSecMsgState(
        void * pDataParams                                                  /**< [In] Pointer to this layer's parameter structure. */
    );

/** \name Option to configure some special operations. */
/** @{ */
#define PHAL_MFPEVX_WRAPPED_MODE                                    0xA1U   /**< Option for GetConfig/SetConfig to get/set current status of command wrapping in ISO 7816-4 APDUs. */
#define PHAL_MFPEVX_EXTENDED_APDU                                   0xA2U   /**< Option for GetConfig/SetConfig to get/set current status of extended wrapping in ISO 7816-4 APDUs. */
#define PHAL_MFPEVX_AUTH_MODE                                       0xA3U   /**< Option to set the auth mode to perform negative testing. */
/** @} */

/** \name Option to enable or disable the Wrapped or Extended APDU options. */
/** @{ */
#define PHAL_MFPEVX_DISABLE                                         0x00U   /**< Option to disable Wrapping or Extended Length APDU feature for ISO7816 support. */
#define PHAL_MFPEVX_ENABLE                                          0x01U   /**< Option to enable Wrapping or Extended Length APDU feature for ISO7816 support. */
#define PHAL_MFPEVX_DEFAULT                             PHAL_MFPEVX_DISABLE /**< Default Option value. This is equal to Disable. */
/** @} */

/** \name Option to configure the Authentication state. */
/** @{ */
#define PHAL_MFPEVX_NOTAUTHENTICATED                                0x00U    /**< Option to indicate the auth mode as MFP EVx not authenticated. */
#define PHAL_MFPEVX_SL1_MIFARE_AUTHENTICATED                        0x01U    /**< Option to indicate the auth mode as MFP EVx SL1 MIFARE Authentication mode. */
#define PHAL_MFPEVX_SL1_MFP_AUTHENTICATED                           0x02U    /**< Option to indicate the auth mode as MFP EVx SL1 Authentication mode. */
#define PHAL_MFPEVX_SL3_MFP_AUTHENTICATED                           0x03U    /**< Option to indicate the auth mode as MFP EVx SL3 Authentication mode. */
#define PHAL_MFPEVX_NOT_AUTHENTICATED_L3                            0x04U    /**< Option to indicate the auth mode as MFP EVx not authenticated in ISO Layer 3. */
#define PHAL_MFPEVX_NOT_AUTHENTICATED_L4                            0x05U    /**< Option to indicate the auth mode as MFP EVx not authenticated in ISO Layer 4. */
/** @} */

/**
 * \brief Perform a SetConfig command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_SetConfig(
        void * pDataParams,                                                 /**< [In] Pointer to this layers parameter structure. */
        uint16_t wOption,                                                   /**< [In] Option to set.
                                                                             *          \arg #PHAL_MFPEVX_WRAPPED_MODE
                                                                             *          \arg #PHAL_MFPEVX_EXTENDED_APDU
                                                                             *          \arg #PHAL_MFPEVX_AUTH_MODE
                                                                             */
        uint16_t wValue                                                     /**< [In] Value for the selected option.
                                                                             *          \arg #PHAL_MFPEVX_WRAPPED_MODE
                                                                             *              \c #PHAL_MFPEVX_DISABLE;
                                                                             *              \c #PHAL_MFPEVX_ENABLE;
                                                                             *              \c #PHAL_MFPEVX_DEFAULT;
                                                                             *        \n
                                                                             *          \arg #PHAL_MFPEVX_EXTENDED_APDU
                                                                             *              \c #PHAL_MFPEVX_DISABLE;
                                                                             *              \c #PHAL_MFPEVX_ENABLE;
                                                                             *              \c #PHAL_MFPEVX_DEFAULT;
                                                                             *        \n
                                                                             *          \arg #PHAL_MFPEVX_AUTH_MODE
                                                                             *              \c #PHAL_MFPEVX_NOTAUTHENTICATED;
                                                                             *              \c #PHAL_MFPEVX_SL1_MIFARE_AUTHENTICATED;
                                                                             *              \c #PHAL_MFPEVX_SL1_MFP_AUTHENTICATED;
                                                                             *              \c #PHAL_MFPEVX_SL3_MFP_AUTHENTICATED;
                                                                             *              \c #PHAL_MFPEVX_NOT_AUTHENTICATED_L3;
                                                                             *              \c #PHAL_MFPEVX_NOT_AUTHENTICATED_L4;
                                                                             */
    );

/**
 * \brief Perform a GetConfig command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_GetConfig(
        void * pDataParams,                                                 /**< [In] Pointer to this layers parameter structure. */
        uint16_t wOption,                                                   /**< [In] Option to get.
                                                                             *          \arg #PHAL_MFPEVX_WRAPPED_MODE
                                                                             *          \arg #PHAL_MFPEVX_EXTENDED_APDU
                                                                             *          \arg #PHAL_MFPEVX_AUTH_MODE
                                                                             */
        uint16_t * pValue                                                   /**< [Out] Value of the selected option.
                                                                             *          \arg #PHAL_MFPEVX_WRAPPED_MODE
                                                                             *              \c #PHAL_MFPEVX_DISABLE;
                                                                             *              \c #PHAL_MFPEVX_ENABLE;
                                                                             *              \c #PHAL_MFPEVX_DEFAULT;
                                                                             *        \n
                                                                             *          \arg #PHAL_MFPEVX_EXTENDED_APDU
                                                                             *              \c #PHAL_MFPEVX_DISABLE;
                                                                             *              \c #PHAL_MFPEVX_ENABLE;
                                                                             *              \c #PHAL_MFPEVX_DEFAULT;
                                                                             *        \n
                                                                             *          \arg #PHAL_MFPEVX_AUTH_MODE
                                                                             *              \c #PHAL_MFPEVX_NOTAUTHENTICATED;
                                                                             *              \c #PHAL_MFPEVX_SL1_MIFARE_AUTHENTICATED;
                                                                             *              \c #PHAL_MFPEVX_SL1_MFP_AUTHENTICATED;
                                                                             *              \c #PHAL_MFPEVX_SL3_MFP_AUTHENTICATED;
                                                                             *              \c #PHAL_MFPEVX_NOT_AUTHENTICATED_L3;
                                                                             *              \c #PHAL_MFPEVX_NOT_AUTHENTICATED_L4;
                                                                             */
    );

/**
 * \brief This is a utility API which sets the VCA structure in MFP Ev1 structure params.
 * This interface is mandatory to be called if the Virtual Card and Proximity Check features
 * are required.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMfpEVx_SetVCAParams(
        void * pDataParams,                                                 /**< [In] Pointer to this layers parameter structure. */
        void * pAlVCADataParams                                             /**< [In] The VCA application layer's dataparams. */
    );


/**
 * end of group phalMfpEVx_Utilities
 * @}
 */

/**
 * end of group phalMfpEVx
 * @}
 */

#endif /* NXPBUILD__PHAL_MFPEVX */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHALMFPEVX_H */
