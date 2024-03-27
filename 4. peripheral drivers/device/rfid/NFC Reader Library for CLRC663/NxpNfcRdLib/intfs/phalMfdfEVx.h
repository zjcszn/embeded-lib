/*----------------------------------------------------------------------------*/
/* Copyright 2014-2020 NXP                                                    */
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
* Generic MIFARE DESFire EVx contactless IC (EV1, EV2, EV3, future versions ) Application Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6116 $ (v07.09.00)
* $Date: 2020-05-15 19:22:24 +0530 (Fri, 15 May 2020) $
*
*/

#ifndef PHALMFDFEVX_H
#define PHALMFDFEVX_H

#include <ph_Status.h>
#include <phTMIUtils.h>
#include <phhalHw.h>
#include <phpalMifare.h>
#include <ph_TypeDefs.h>
#include <ph_RefDefs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************************************************************************************/
/* Software Dataparams and Initialization Interface.                                                                                   */
/***************************************************************************************************************************************/
#ifdef NXPBUILD__PHAL_MFDFEVX_SW

/**
 * \defgroup phalMfdfEVx_Sw Component : Software
 * @{
 */

#define PHAL_MFDFEVX_SW_ID                                              0x01U   /**< ID for Software MF DesFire layer. */

/** \name Sizes */
/** @{ */
#define PHAL_MFDFEVX_SIZE_TI                                                4U  /**< Size of Transaction Identifier. */
#define PHAL_MFDFEVX_SIZE_MAC                                               16U /**< Size of (untruncated) MAC. */
#define PHAL_MFDFEVX_BLOCK_SIZE                                             16U /**< Block Size */
#define PHAL_MFDFEVX_MAX_WRITE_SIZE                                     0xFFFDU /**< Max size in a Write function. */
/** @} */

/** \brief Data structure for MIFARE DESFire EVx Software layer implementation  */
typedef struct
{
    uint16_t wId;                                                               /**< Layer ID for this component, NEVER MODIFY! */
    void * pPalMifareDataParams;                                                /**< Pointer to the parameter structure of the palMifare component. */
    void * pKeyStoreDataParams;                                                 /**< Pointer to the parameter structure of the KeyStore layer. */
    void * pCryptoDataParamsEnc;                                                /**< Pointer to the parameter structure of the Crypto layer for encryption. */
    void * pCryptoDataParamsMac;                                                /**< Pointer to the parameter structure of the CryptoMAC. */
    void * pCryptoRngDataParams;                                                /**< Pointer to the parameter structure of the CryptoRng layer. */
    void * pHalDataParams;                                                      /**< Pointer to the HAL parameters structure. */
    uint8_t bSesAuthENCKey[24];                                                 /**< Session key for this authentication */
    uint8_t bKeyNo;                                                             /**< Key number against which this authentication is done */
    uint8_t bIv[16];                                                            /**< Max size of IV can be 16 bytes */
    uint8_t bAuthMode;                                                          /**< Authenticate (0x0A), AuthISO (0x1A), AuthAES (0xAA) */
    uint8_t bCmdCode;                                                           /**< Command code. This will be used for differentiating the comman error codes between different commands. */
    uint8_t pAid[3];                                                            /**< Aid of the currently selected application */
    uint8_t bCryptoMethod;                                                      /**< DES,3DES, 3K3DES or AES */
    uint8_t bWrappedMode;                                                       /**< Wrapped APDU mode. All native commands need to be sent wrapped in ISO 7816 APDUs. */
    uint16_t wCrc;                                                              /**< 2 Byte CRC initial value in Authenticate mode. */
    uint32_t dwCrc;                                                             /**< 4 Byte CRC initial value in 0x1A, 0xAA mode. */
    uint16_t wAdditionalInfo;                                                   /**< Specific error codes for Desfire generic errors or To get the response length of some commands. */
    uint32_t dwPayLoadLen;                                                      /**< Amount of data to be read. Required for Enc read to verify CRC. */
    uint16_t wCmdCtr ;                                                          /**< Command count within transaction. */
    uint8_t bTi[PHAL_MFDFEVX_SIZE_TI];                                          /**< Transaction Identifier. */
    uint8_t bSesAuthMACKey[16];                                                 /**< Authentication MAC key for the session. */
    uint8_t pUnprocByteBuff[PHAL_MFDFEVX_SIZE_MAC];                             /**< Buffer containing unprocessed bytes for read mac answer stream. */
    uint8_t bNoUnprocBytes;                                                     /**< Amount of data in the pUnprocByteBuff. */
    uint8_t bLastBlockBuffer[16];                                               /**< Buffer to store last Block of encrypted data in case of chaining. */
    uint8_t bLastBlockIndex;                                                    /**< Last Block Buffer Index. */
    void * pTMIDataParams;                                                      /**< Pointer to the parameter structure for collecting TMI. */
    uint8_t bShortLenApdu;                                                      /**< Parameter for force set Short Length APDU in case of BIG ISO read. */
    void * pVCADataParams;                                                      /**< Pointer to the parameter structure for Virtual Card. */
    uint8_t bUid[15];                                                           /**< Complete UID of the PICC. \cond !PUBLIC This will be updated when \ref phalMfdfEVx_GetCardUID interface is called. \endcond */
    uint8_t bUidLength;                                                         /**< Length of bytes available in bUid buffer. */
    uint8_t bSDMStatus;                                                         /**< Get / Set the SDM availability in the file. */
} phalMfdfEVx_Sw_DataParams_t;

/**
 * \brief Initialization API for MIFARE DESFire EVx software component.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phalMfdfEVx_Sw_Init(
        phalMfdfEVx_Sw_DataParams_t * pDataParams,                              /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wSizeOfDataParams,                                             /**< [In] Specifies the size of the data parameter structure */
        void * pPalMifareDataParams,                                            /**< [In] Pointer to a palMifare component context. */
        void * pKeyStoreDataParams,                                             /**< [In] Pointer to Key Store data parameters. */
        void * pCryptoDataParamsEnc,                                            /**< [In] Pointer to a Crypto component context for encryption. */
        void * pCryptoDataParamsMac,                                            /**< [In] Pointer to a CryptoMAC component context. */
        void * pCryptoRngDataParams,                                            /**< [In] Pointer to a CryptoRng component context. */
        void * pTMIDataParams,                                                  /**< [In] Pointer to a TMI component. */
        void * pVCADataParams,                                                  /**< [In] Pointer to a VCA component. */
        void * pHalDataParams                                                   /**< [In] Pointer to the HAL parameters structure. */
    );

/**
 * \brief Assign the dataparams of VCA AL for VCA / PC command executions.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phalMfdfEVx_Sw_SetVCAParams(
        phalMfdfEVx_Sw_DataParams_t * pDataParams,                              /**< [In] Pointer to this layer's parameter structure. */
        void * pAlVCADataParams                                                 /**< [In] Pointer to VCA AL parameter structure. */
    );

/**
 * end of group phalMfdfEVx_Sw
 * @}
 */

#endif /* NXPBUILD__PHAL_MFDFEVX_SW */

/***************************************************************************************************************************************/
/* Sam NonX Dataparams and Initialization Interface.                                                                                   */
/***************************************************************************************************************************************/




/***************************************************************************************************************************************/
/* MIFARE DESFire EVx Generic command declarations.                                                                                    */
/***************************************************************************************************************************************/
#ifdef NXPBUILD__PHAL_MFDFEVX

/** \defgroup phalMfdfEVx MIFARE DESFire EVx
 * \brief These Functions implement the MIFARE DESFire EVx commands.
 * @{
 */

/** \name Other Options for various Functions */
/** @{ */
#define PHAL_MFDFEVX_ORIGINALITY_KEY_FIRST                                              0x01U   /**< Originality key for AES */
#define PHAL_MFDFEVX_ORIGINALITY_KEY_LAST                                               0x04U   /**< Originality key for AES  */
#define PHAL_MFDFEVX_MAC_DATA_INCOMPLETE                                                0x01U   /**< Option for indicating more data to come in next call for MAC calculation */
#define PHAL_MFDFEVX_COMMUNICATION_MAC_ON_RC                                            0x02U   /**< MAC is appended over response  */
#define PHAL_MFDFEVX_COMMUNICATION_MAC_ON_CMD                                           0x03U   /**< MAC is appended over command. */
#define PHAL_MFDFEVX_AUTHENTICATE_RESET                                                 0x08U   /**< authentication state shall be reset */
#define PHAL_MFDFEVX_KEYSETVALUES_PRESENT                                               0x01U   /**< Bit 0 of bKeySettings3 decides the presence of the key set values array. */
#define PHAL_MFDFEVX_KEYSETT3_PRESENT                                                   0x10U   /**< Bit 4 of bKeySettings2 decides the presence of the keysetting3. */
#define PHAL_MFDFEVX_MFC_KILL_KEY                                                       0x31U   /**< MFCKill key for AES */
/** @} */

/** \name Error Codes mapping to custom values. */
/** @{ */
#define PHAL_MFDFEVX_NO_CHANGES                                     (PH_ERR_CUSTOM_BEGIN + 0)   /**< MFDFEVx Response - No changes done to backup files. */
#define PHAL_MFDFEVX_ERR_OUT_OF_EEPROM_ERROR                        (PH_ERR_CUSTOM_BEGIN + 1)   /**< MFDFEVx Response - Insufficient NV-Memory. */
#define PHAL_MFDFEVX_ERR_NO_SUCH_KEY                                (PH_ERR_CUSTOM_BEGIN + 2)   /**< MFDFEVx Invalid key number specified. */
#define PHAL_MFDFEVX_ERR_PERMISSION_DENIED                          (PH_ERR_CUSTOM_BEGIN + 3)   /**< MFDFEVx Current configuration/status does not allow the requested command. */
#define PHAL_MFDFEVX_ERR_APPLICATION_NOT_FOUND                      (PH_ERR_CUSTOM_BEGIN + 4)   /**< MFDFEVx Requested AID not found on PICC. */
#define PHAL_MFDFEVX_ERR_BOUNDARY_ERROR                             (PH_ERR_CUSTOM_BEGIN + 5)   /**< MFDFEVx Attempt to read/write data from/to beyond the files/record's limits. */
#define PHAL_MFDFEVX_ERR_COMMAND_ABORTED                            (PH_ERR_CUSTOM_BEGIN + 6)   /**< MFDFEVx Previous cmd not fully completed. Not all frames were requested or provided by the PCD. */
#define PHAL_MFDFEVX_ERR_COUNT                                      (PH_ERR_CUSTOM_BEGIN + 7)   /**< MFDFEVx Num. of applns limited to 28. No additional applications possible. */
#define PHAL_MFDFEVX_ERR_DUPLICATE                                  (PH_ERR_CUSTOM_BEGIN + 8)   /**< MFDFEVx File/Application with same number already exists. */
#define PHAL_MFDFEVX_ERR_FILE_NOT_FOUND                             (PH_ERR_CUSTOM_BEGIN + 9)   /**< MFDFEVx Specified file number does not exist. */
#define PHAL_MFDFEVX_ERR_PICC_CRYPTO                                (PH_ERR_CUSTOM_BEGIN + 10)  /**< MFDFEVx Crypto error returned by PICC. */
#define PHAL_MFDFEVX_ERR_PARAMETER_ERROR                            (PH_ERR_CUSTOM_BEGIN + 11)  /**< MFDFEVx Parameter value error returned by PICC. */
#define PHAL_MFDFEVX_ERR_DF_GEN_ERROR                               (PH_ERR_CUSTOM_BEGIN + 12)  /**< MFDFEVx DesFire Generic error. Check additional Info. */
#define PHAL_MFDFEVX_ERR_DF_7816_GEN_ERROR                          (PH_ERR_CUSTOM_BEGIN + 13)  /**< MFDFEVx ISO 7816 Generic error. Check Additional Info. */
#define PHAL_MFDFEVX_ERR_CMD_INVALID                                (PH_ERR_CUSTOM_BEGIN + 14)  /**< MFDFEVx ISO 7816 Generic error. Check Additional Info. */
#define PHAL_MFDFEVX_ERR_NOT_SUPPORTED                              (PH_ERR_CUSTOM_BEGIN + 15)  /**< MFDFEVx Not Supported Error. */
#define PHAL_MFDFEVX_ERR_INTEGRITY_ERROR                            (PH_ERR_CUSTOM_BEGIN + 16)  /**< MFDFEVx integrity error. */
#define PHAL_MFDFEVX_ERR_MEMORY_ERROR                               (PH_ERR_CUSTOM_BEGIN + 17)  /**< MFDFEVx memory error. */
#define PHAL_MFDFEVX_ERR_OPERATION_OK_LIM                           (PH_ERR_CUSTOM_BEGIN + 27)  /**< MFDFEVx custom error code for Successful operation with limited functionality. */
#define PHAL_MFDFEVX_ERR_CMD_OVERFLOW                               (PH_ERR_CUSTOM_BEGIN + 28)  /**< MFDFEVx Too many commands in the session or transaction. */
#define PHAL_MFDFEVX_ERR_GEN_FAILURE                                (PH_ERR_CUSTOM_BEGIN + 29)  /**< MFDFEVx Failure in the operation of the PD. */
#define PHAL_MFDFEVX_ERR_BNR                                        (PH_ERR_CUSTOM_BEGIN + 30)  /**< MFDFEVx Invalid Block number: not existing in the implementation or not valid to target with this command. */
#define PHAL_MFDFEVX_ERR_FORMAT                                     (PH_ERR_CUSTOM_BEGIN + 31)  /**< MFDFEVx Format of the command is not correct (e.g. too many or too few bytes). */
/** @} */

/** \name ISO7816-4 Error Codes mapping to custom values. */
/** @{ */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_MEMORY_FAILURE              (PH_ERR_CUSTOM_BEGIN + 18)  /**< ISO7816 custom response code for memory failure. */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_WRONG_LENGTH                (PH_ERR_CUSTOM_BEGIN + 19)  /**< ISO7816 custom response code for wrong length, no further indication. */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_SECURITY_STAT_NOT_SATISFIED (PH_ERR_CUSTOM_BEGIN + 20)  /**< ISO7816 custom response code for security status not satisfied. */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_CONDITIONS_NOT_SATISFIED    (PH_ERR_CUSTOM_BEGIN + 21)  /**< ISO7816 custom response code for conditions of use not satisfied. */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_FILE_APPLICATION_NOT_FOUND  (PH_ERR_CUSTOM_BEGIN + 22)  /**< ISO7816 custom response code for file or application not found. */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_INCORRECT_PARAM_P1_P2       (PH_ERR_CUSTOM_BEGIN + 23)  /**< ISO7816 custom response code for incorrect parameters P1-P2. */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_LC_INCONSISTENT             (PH_ERR_CUSTOM_BEGIN + 24)  /**< ISO7816 custom response code for Lc inconsistent with parameter P1-P2. */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_WRONG_LE                    (PH_ERR_CUSTOM_BEGIN + 25)  /**< ISO7816 custom response code for wrong LE field. */
#define PHAL_MFDFEVX_ISO7816_CUSTOM_ERR_INS_CODE_NOT_SUPPORTED      (PH_ERR_CUSTOM_BEGIN + 26)  /**< ISO7816 custom response code for instruction code not supported or invalid. */
/** @} */

/** \name Authentication Modes */
/** @{ */
#define PHAL_MFDFEVX_NOT_AUTHENTICATED                                  0xFFU   /**< No authentication. */
#define PHAL_MFDFEVX_AUTHENTICATE                                       0x0AU   /**< D40 Authentication; 0x0A. */
#define PHAL_MFDFEVX_AUTHENTICATEISO                                    0x1AU   /**< ISO Authentication; 0x1A. */
#define PHAL_MFDFEVX_AUTHENTICATEAES                                    0xAAU   /**< AES Authentication; 0xAA. */
#define PHAL_MFDFEVX_AUTHENTICATEEV2                                    0x71U   /**< EV2 Authentication; 0x71. */

#define PHAL_MFDFEVX_AUTH_FIRST                                         0x01U   /**< Option to perform EVx First Authentication. */
#define PHAL_MFDFEVX_AUTH_NONFIRST                                      0x00U   /**< Option to perform EVx NonFirst Authentication. */
/** @} */

/** \name Option to be used for InitializeKeySet amd CreateTransactionMACFile command. */
/** @{*/
#define PHAL_MFDFEVX_KEY_TYPE_2K3DES                                    0x00U   /**< Option for 2 Key Triple Des key type. */
#define PHAL_MFDFEVX_KEY_TYPE_3K3DES                                    0x01U   /**< Option for 3 Key Triple Des key type. */
#define PHAL_MFDFEVX_KEY_TYPE_AES128                                    0x02U   /**< Option for AES 128 key type. */
/** @}*/

/** \name Option for most of the commands. */
/** @{ */
#define PHAL_MFDFEVX_SAI                                                0x80U   /**< Option for Secondary application indicator (SAI). */
/** @} */

/** \name Options for application / file creation commands command. */
/** @{ */
#define PHAL_MFDFEVX_ISO_FILE_INFO_NOT_AVAILABLE                        0x00U   /**< Option to indicate no ISO File ID or ISODFName are present. */
#define PHAL_MFDFEVX_ISO_FILE_ID_AVAILABLE                              0x01U   /**< Option to indicate the presence of ISO FileID. */
#define PHAL_MFDFEVX_ISO_DF_NAME_AVAILABLE                              0x02U   /**< Option to indicate the presence of ISO DFName. */
#define PHAL_MFDFEVX_ISO_FILE_ID_DF_NAME_AVAILABLE                      0x03U   /**< Option to indicate the presence of both ISO FileID and ISO DFName. */
/** @} */

/** \name The communication mode to be used. */
/** @{ */
#define PHAL_MFDFEVX_COMMUNICATION_PLAIN                                0x00U   /**< Plain mode of communication. */
#define PHAL_MFDFEVX_COMMUNICATION_PLAIN_1                              0x20U   /**< Plain mode of communication. */
#define PHAL_MFDFEVX_COMMUNICATION_MACD                                 0x10U   /**< MAC mode of communication. */
#define PHAL_MFDFEVX_COMMUNICATION_ENC                                  0x30U   /**< Enciphered mode of communication. */
/** @} */





/* MIFARE DESFire EVx Memory and Configuration mamangement commands. ------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_MemoryConfiguration Commands_MemoryConfiguration
 * \brief These Components implement the MIFARE DESFire EVx Memory and Configuration Management related commands.
 * @{
 */

/**
 * \brief Returns free memory available on the PICC
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_FreeMem(
        void *pDataParams,                                                      /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pMemInfo                                                      /**< [Out] Current free memory available. Will be of 3 bytes with LSB first. */
    );


/**
 * \brief Returns manufacturing related data of the PICC. Call \ref phalMfdfEVx_GetConfig with #PHAL_MFDFEVX_ADDITIONAL_INFO
 * to the get the length of bytes available in VerInfo buffer.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetVersion(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pVerInfo                                                      /**< [Out] The version information of the product. */
    );


/**
 * end of group phalMfdfEVx_MemoryConfiguration
 * @}
 */




/* MIFARE DESFire EVx Key mamangement commands. ---------------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_KeyManagement Commands_KeyManagement
 * \brief These Components implement the MIFARE DESFire EVx Key Management related commands.
 * @{
 */


/**
 * \brief Gets information on the PICC and application master key settings.
 *
 * \remarks
 * In addition it returns the maximum number of keys which are configured for the selected application.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetKeySettings(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pKeySettings,                                                 /**< [Out] Returns the key settings. Can be 2 or 3 bytes. */
        uint8_t * bRespLen                                                      /**< [Out] Length bytes available in KeySettings buffer. */
    );


/**
 * end of group phalMfdfEVx_KeyManagement
 * @}
 */




/* MIFARE DESFire EVx Application mamangement commands. -------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_AppManagement Commands_ApplicationManagement
 * \brief These Components implement the MIFARE DESFire EVx Application Management related commands.
 * @{
 */

/**
 * \brief Creates new applications on the PICC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CreateApplication(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Option to represent the present of ISO information.
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_INFO_NOT_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_ID_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_DF_NAME_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_ID_DF_NAME_AVAILABLE
                                                                                 */
        uint8_t * pAid,                                                         /**< [In] The application identifier to be used. Will be of 3 bytes with LSB first.
                                                                                 *          \c If application 01 need to be created then the Aid will be 01 00 00.
                                                                                 */
        uint8_t bKeySettings1,                                                  /**< [In] Application Key settings. */
        uint8_t bKeySettings2,                                                  /**< [In] Additional key settings.  */
        uint8_t bKeySettings3,                                                  /**< [In] Additional key settings.  */
        uint8_t * pKeySetValues,                                                /**< [In] The Key set values for the application. Should as mentioned below.\n
                                                                                 *          \c Byte0 = AKS ver\n
                                                                                 *          \c Byte1 = Number of Keysets\n
                                                                                 *          \c Byte2 = MaxKeysize\n
                                                                                 *          \c Byte3 = Application KeySet Settings.
                                                                                 */
        uint8_t * pISOFileId,                                                   /**< [In] ISO File ID to be used. Will be two bytes. */
        uint8_t * pISODFName,                                                   /**< [In] ISO DF Name to be used. Will be 16 bytes and can also be NULL based on the option. */
        uint8_t bISODFNameLen                                                   /**< [In] Length of bytes available in ISODFName buffer. */
    );

/**
 * \brief Permanently deletes the applications on the PICC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_DeleteApplication(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pAid                                                          /**< [In] The application identifier to be used. Will be of 3 bytes with LSB first. \n
                                                                                 *          \c If application 01 need to be deleted then the Aid will be 01 00 00.
                                                                                 */
        , uint8_t * pDAMMAC,                                                    /**< [In] [Optional, present if PICCDAMAuthKey or NXPDAMAuthKey is used for authentication]
                                                                                 *        The MAC calculated by the card issuer to allow delegated application deletion.
                                                                                 */
        uint8_t bDAMMAC_Len                                                     /**< [In] Length of bytes available in DAMMAC buffer. */
    );

/**
 * \brief Creates new Delegated application
 * For delegated application creation, the PICC level (AID 0x000000) must have been selected.
 * An active authentication with the PICCDAMAuthKey is required.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CreateDelegatedApplication(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Option to represent the present of ISO information.
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_INFO_NOT_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_ID_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_DF_NAME_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_ID_DF_NAME_AVAILABLE
                                                                                 */
        uint8_t * pAid,                                                         /**< [In] The application identifier to be used. Will be of 3 bytes with LSB first.
                                                                                 *          \c If application 01 need to be created then the Aid will be 01 00 00.
                                                                                 */
        uint8_t * pDamParams,                                                   /**< [In] The DAM information. Should as mentioned below.\n
                                                                                 *          \c Byte0 = DAM Slot No [LSB]\n
                                                                                 *          \c Byte1 = DAM Slot No [MSB]\n
                                                                                 *          \c Byte2 = DAM Slot Version\n
                                                                                 *          \c Byte3 = Quota Limit [LSB]\n
                                                                                 *          \c Byte4 = Quota Limit [MSB]
                                                                                 */
        uint8_t bKeySettings1,                                                  /**< [In] Application Key settings. */
        uint8_t bKeySettings2,                                                  /**< [In] Additional key settings.  */
        uint8_t bKeySettings3,                                                  /**< [In] Additional key settings.  */
        uint8_t * pKeySetValues,                                                /**< [In] The Key set values for the application. Should as mentioned below.\n
                                                                                 *          \c Byte0 = AKS ver\n
                                                                                 *          \c Byte1 = Number of Keysets\n
                                                                                 *          \c Byte2 = MaxKeysize\n
                                                                                 *          \c Byte3 = Application KeySet Settings.
                                                                                 */
        uint8_t * pISOFileId,                                                   /**< [In] ISO File ID to be used. Will be two bytes. */
        uint8_t * pISODFName,                                                   /**< [In] ISO DF Name to be used. Will be 16 bytes and can also be NULL based on the option. */
        uint8_t bISODFNameLen,                                                  /**< [In] Length of bytes available in ISODFName buffer. */
        uint8_t * pEncK,                                                        /**< [In] Encrypted initial application key. 32 bytes long. */
        uint8_t * pDAMMAC                                                       /**< [In] 8 byte DAM MAC. */
    );

/** \name Option to represent the selection of secondary application. */
/** @{ */
#define PHAL_MFDFEVX_SELECT_PRIMARY_APP                                 0x00U   /**< Option for Primary application selection. */
#define PHAL_MFDFEVX_SELECT_SECOND_APP                                  0x01U   /**< Option for Secondary application selection. */
/** @} */

/**
 * \brief Selects one particular application on the PICC for further access.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_SelectApplication(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] One of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_SELECT_PRIMARY_APP
                                                                                 *          \arg #PHAL_MFDFEVX_SELECT_SECOND_APP
                                                                                 */
        uint8_t * pAid,                                                         /**< [In] The primary application identifier to be used. Will be of 3 bytes with LSB first.
                                                                                 *          \c If application 01 need to be selected then the Aid will be 01 00 00.
                                                                                 */
        uint8_t * pAid2                                                         /**< [In] The secondary application identifier to be used. Will be of 3 bytes with LSB first.
                                                                                 *          \c If application 01 need to be selected then the Aid will be 01 00 00.
                                                                                 */
    );

/**
 * \brief Returns application identifiers of all applications on the PICC
 *
 * \remarks
 * This response will be #PH_ERR_SUCCESS if all the application ids can be obtained in one call.
 * If not, then #PH_ERR_SUCCESS_CHAINING is returned. The user has to call this function with
 * bOption = #PH_EXCHANGE_RXCHAINING to get the remaining AIDs.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval #PH_ERR_SUCCESS_CHAINING Operation successful with chaining.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetApplicationIDs(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] One of the below options.
                                                                                 *          \arg #PH_EXCHANGE_DEFAULT
                                                                                 *          \arg #PH_EXCHANGE_RXCHAINING
                                                                                 */
        uint8_t ** pAidBuff,                                                    /**< [Out] The available identifiers of the application(s). */
        uint8_t * pNumAid                                                       /**< [Out] The number of available application(s). */
    );


/**
 * \brief Parameters associated with the delegated application can be retrieved using this command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetDelegatedInfo(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pDAMSlot,                                                     /**< [In] Slot number associated with the memory space of the targeted
                                                                                 *        delegated application. LSB first.
                                                                                 */
        uint8_t * pDamSlotVer,                                                  /**< [Out] Slot version associated with the memory space of the targeted
                                                                                 *         delegated application.
                                                                                 */
        uint8_t * pQuotaLimit,                                                  /**< [Out] Maximal memory consumption of the targeted delegated application
                                                                                 *         (in 32-byte blocks).
                                                                                 */
        uint8_t * pFreeBlocks,                                                  /**< [Out] Amount of unallocated memory of the targeted delegated application
                                                                                 *         (in 32-byte blocks).
                                                                                 */
        uint8_t * pAid                                                          /**< [Out] Application Identifier of the targeted DAM slot. This parameter is set to
                                                                                 *         0x000000 if the DAM slot targeted by DAMSlotNo exists on the card, but is
                                                                                 *         currently not occupied with an application, i.e. the latest application has
                                                                                 *         been deleted.
                                                                                 */
    );

/**
 * end of group phalMfdfEVx_AppManagement
 * @}
 */




/* MIFARE DESFire EVx File mamangement commands. --------------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_FileManagement Commands_FileManagement
 * \brief These Components implement the MIFARE DESFire EVx File Management related commands.
 * @{
 */

/** \name The File Option to be used. */
/** @{ */
#define PHAL_MFDFEVX_FILE_OPTION_PLAIN                                                  0x00U   /**< Plain mode of communication. */
#define PHAL_MFDFEVX_FILE_OPTION_PLAIN_1                                                0x02U   /**< Plain mode of communication. */
#define PHAL_MFDFEVX_FILE_OPTION_MACD                                                   0x01U   /**< MAC mode of communication. */
#define PHAL_MFDFEVX_FILE_OPTION_ENC                                                    0x03U   /**< Enciphered mode of communication. */
#define PHAL_MFDFEVX_FILE_OPTION_MFC_MAPPING_ENABLED                                    0x20U   /**< MIFARE Classic contactless IC mapping support enabled. */
#define PHAL_MFDFEVX_FILE_OPTION_SDM_MIRRORING_ENABLED                                  0x40U   /**< Secure Dynamic Messaging and Mirroring support enabled. */
#define PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT                                  0x80U   /**< Additional Access Rigths enabled. */
/** @} */

/**
 * \brief Creates files for the storage of plain unformatted user data within an existing application
 * on the PICC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CreateStdDataFile(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Option to represent the present of ISO information.
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_INFO_NOT_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_ID_AVAILABLE
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to be created.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pISOFileId,                                                   /**< [In] ISO File ID to be used. Will be two bytes. */
        uint8_t bFileOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ENC
                                                                                 *        \n
                                                                                 *        Ored with one of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MFC_MAPPING_ENABLED
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_SDM_MIRRORING_ENABLED
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT
                                                                                 */
        uint8_t * pAccessRights,                                                /**< [In] The new access right to be applied for the file. Should be 2 byte. */
        uint8_t * pFileSize                                                     /**< [In] The size of the file. Will be of 3 bytes with LSB first.
                                                                                 *          \c If size 0x10 need to be created then the FileSize will be 10 00 00.
                                                                                 */
    );

/**
 * \brief Creates files for the storage of plain unformatted user data within an existing application
 * on the PICC, additionally supporting the feature of an integrated backup
 * mechanism.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CreateBackupDataFile(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Option to represent the present of ISO information.
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_INFO_NOT_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_ID_AVAILABLE
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to be created.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pISOFileId,                                                   /**< [In] ISO File ID to be used. Will be two bytes. */
        uint8_t bFileOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ENC
                                                                                 *        \n
                                                                                 *        Ored with one of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MFC_MAPPING_ENABLED
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT
                                                                                 */
        uint8_t * pAccessRights,                                                /**< [In] The new access right to be applied for the file. Should be 2 byte. */
        uint8_t * pFileSize                                                     /**< [In] The size of the file. Will be of 3 bytes with LSB first.
                                                                                 *          \c If size 0x10 need to be created then the FileSize will be 10 00 00.
                                                                                 */
    );

/** \name Option for CreateValueFile file command. */
/** @{ */
#define PHAL_MFDFEVX_ENABLE_LIMITEDCREDIT                               0x01U   /**< Bit 0 set to 1 to enable Limited credit. */
#define PHAL_MFDFEVX_ENABLE_FREE_GETVALUE                               0x02U   /**< Bit 1 set to 1 to enable free GetValue. */
/** @} */

/**
 * \brief Creates files for the storage and manipulation of 32bit signed integer values within
 * an existing application on the PICC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CreateValueFile(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bFileNo,                                                        /**< [In] The file number to be created.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t bCommSett,                                                      /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ENC
                                                                                 *        \n
                                                                                 *        Ored with one of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MFC_MAPPING_ENABLED
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT
                                                                                 */
        uint8_t * pAccessRights,                                                /**< [In] The new access right to be applied for the file. Should be 2 byte. */
        uint8_t * pLowerLmit,                                                   /**< [In] The lower limit for the file. Will be of 4 bytes with LSB first.
                                                                                 *          \c If size 0x10 need to be created then the value will be 10 00 00 00.
                                                                                 */
        uint8_t * pUpperLmit,                                                   /**< [In] The upper limit for the file. Will be of 4 bytes with LSB first.
                                                                                 *          \c If size 0x20 need to be created then the value will be 20 00 00 00.
                                                                                 */
        uint8_t * pValue,                                                       /**< [In] The initial value. Will be of 4 bytes with LSB first.
                                                                                 *          \c If size 0x10 need to be created then the value will be 10 00 00 00.
                                                                                 */
        uint8_t bLimitedCredit                                                  /**< [In] One of the below values.
                                                                                 *          \arg #PHAL_MFDFEVX_ENABLE_LIMITEDCREDIT
                                                                                 *          \arg #PHAL_MFDFEVX_ENABLE_FREE_GETVALUE
                                                                                 */
    );

/**
 * \brief Creates files for multiple storage of structural similar data, for example for layalty programs within an existing application.
 * Once the file is filled, further writing is not possible unless it is cleared.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CreateLinearRecordFile(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Option to represent the present of ISO information.
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_INFO_NOT_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_ID_AVAILABLE
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to be created.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pISOFileId,                                                   /**< [In] ISO File ID to be used. Will be two bytes. */
        uint8_t bCommSett,                                                      /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ENC
                                                                                 *        \n
                                                                                 *        Ored with one of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT
                                                                                 */
        uint8_t * pAccessRights,                                                /**< [In] The new access right to be applied for the file. Should be 2 byte. */
        uint8_t * pRecordSize,                                                  /**< [In] The size of the file. Will be of 3 bytes with LSB first.
                                                                                 *          \c If size 0x10 need to be created then the RecordSize will be 10 00 00.
                                                                                 */
        uint8_t * pMaxNoOfRec                                                   /**< [In] The maximum number of record in the file. Will be of 3 bytes with LSB first.
                                                                                 *          \c If size 0x04 need to be created then the value will be 04 00 00.
                                                                                 */
    );

/**
 * \brief Creates files for multiple storage of structural similar data, for example for logging transactions, within an existing application.
 * Once the file is filled, the PICC automatically overwrites the oldest record with the latest written one.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CreateCyclicRecordFile(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Option to represent the present of ISO information.
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_INFO_NOT_AVAILABLE
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_FILE_ID_AVAILABLE
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to be created.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pISOFileId,                                                   /**< [In] ISO File ID to be used. Will be two bytes. */
        uint8_t bCommSett,                                                      /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ENC
                                                                                 *        \n
                                                                                 *        Ored with one of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT
                                                                                 */
        uint8_t * pAccessRights,                                                /**< [In] The new access right to be applied for the file. Should be 2 byte. */
        uint8_t * pRecordSize,                                                  /**< [In] The size of the file. Will be of 3 bytes with LSB first.
                                                                                 *          \c If size 0x10 need to be created then the RecordSize will be 10 00 00.
                                                                                 */
        uint8_t * pMaxNoOfRec                                                   /**< [In] The maximum number of record in the file. Will be of 3 bytes with LSB first.
                                                                                 *          \c If size 0x04 need to be created then the value will be 04 00 00.
                                                                                 */
    );


/**
 * \brief Permanently deactivates a file within the file directory of the currently selected application.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_DeleteFile(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bFileNo                                                         /**< [In] The file number to be deleted.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
    );

/**
 * \brief Returns the file IDs of all active files within the currently selected application.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetFileIDs(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pFid,                                                         /**< [Out] The buffer containing the available file ID(s). Buffer should be 32 bytes. */
        uint8_t * bNumFid                                                       /**< [Out] The length of bytes available in Fid buffer. */
    );

/**
 * \brief Get the ISO File IDs.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetISOFileIDs(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pFidBuffer,                                                   /**< [Out] The buffer containing the available ISO file ID(s). Buffer should be 64 bytes. */
        uint8_t * bNumFid                                                       /**< [Out] The number of ISO File ID's read. */
    );

/**
 * \brief Get informtion on the properties of a specific file
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetFileSettings(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bFileNo,                                                        /**< [In] The file number for which the setting to be retrieved.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pFSBuffer,                                                    /**< [Out] The buffer containing the settings. */
        uint8_t * bBufferLen                                                    /**< [Out] The length of bytes available in FSBuffer buffer. */
    );

/**
 * \brief Get file related counters used for Secure Dynamic Messaging.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetFileCounters(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Indicates the mode of communication to be used while exchanging the data to PICC.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] File number for which the Counter information need to be received. */
        uint8_t * pFileCounters,                                                /**< [Out] The SDMReadCounter information returned by the PICC. */
        uint8_t * pRxLen                                                        /**< [Out] Length of bytes available in FileCounters buffer. */
    );

/** \name The File option and other flags for ChangeFileSettings command. */
/** @{ */
#define PHAL_MFDFEVX_EXCHANGE_ADD_INFO_BUFFER_COMPLETE                  0x80    /**< To exchange the information available in pAddInfo buffer as is. */
#define PHAL_MFDFEVX_FILE_OPTION_TMCLIMIT_PRESENT                       0x20    /**< 5th Bit of FileOption indicating TMC limit config. */
/** @} */

/**
 * \brief Changes the access parameters of an existing file.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_ChangeFileSettings(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Indicates the mode of communication to be used while exchanging the data to PICC.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 *        \n
                                                                                 *        Ored with one of the below flags if required.
                                                                                 *          \arg #PHAL_MFDFEVX_EXCHANGE_ADD_INFO_BUFFER_COMPLETE
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] File number for which the setting need to be updated.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t bFileOption,                                                    /**< [In] New communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ENC
                                                                                 *        \n
                                                                                 *        Ored with one of the below flags if required.
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_SDM_MIRRORING_ENABLED
                                                                                 *          \arg #PHAL_MFDFEVX_FILE_OPTION_TMCLIMIT_PRESENT
                                                                                 */
        uint8_t * pAccessRights,                                                /**< [In] The new access right to be applied for the file. Should be 2 byte. */
        uint8_t bAddInfoLen,                                                    /**< [In] Complete length consisting of\n
                                                                                 *          - If bOption is not having #PHAL_MFDFEVX_EXCHANGE_ADD_INFO_BUFFER_COMPLETE set\n
                                                                                 *              - Additional Access Rights if bFileOption & #PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT\n
                                                                                 *              - TMC Limit                if bFileOption & #PHAL_MFDFEVX_FILE_OPTION_TMCLIMIT_PRESENT\n
                                                                                 *                  - If TMCLimit and AddARs is not available, bAddInfoLen = 0\n
                                                                                 *                  - If AddARs is not available,              bAddInfoLen = 4\n
                                                                                 *                  - If TMCLimit is not available,            bAddInfoLen = 0 - 7\n
                                                                                 *                  - If TMCLimit and AddARs is available,     bAddInfoLen = ( ( 0 - 7 ) + 4 )
                                                                                 *
                                                                                 *          - If bOption is having #PHAL_MFDFEVX_EXCHANGE_ADD_INFO_BUFFER_COMPLETE set, where N is SDM information\n
                                                                                 *              - Additional Access Rights if bFileOption & #PHAL_MFDFEVX_FILE_OPTION_ADDITIONAL_AR_PRESENT\n
                                                                                 *              - SDM Information          if bFileOption & #PHAL_MFDFEVX_FILE_OPTION_SDM_MIRRORING_ENABLED\n
                                                                                 *              - TMC Limit                if bFileOption & #PHAL_MFDFEVX_FILE_OPTION_TMCLIMIT_PRESENT\n
                                                                                 *                  - If all the 3 are not available,                 bAddInfoLen = 0
                                                                                 *                  - If AddARs is not available,                     bAddInfoLen = N + 4\n
                                                                                 *                  - If TMCLimit is not available,                   bAddInfoLen = ( ( 0 - 7 ) * 2 ) + N\n
                                                                                 *                  - If SDM is not available,                        bAddInfoLen = ( ( 0 - 7 ) * 2 ) + 4\n
                                                                                 *                  - If all the 3 are available,                     bAddInfoLen = ( ( ( 0 - 7 ) * 2 ) + N + 4)
                                                                                 *
                                                                                 *
                                                                                 */
        uint8_t * pAddInfo                                                      /**< [In] Buffer should contain the following information. \n
                                                                                 *          \c [Additional access rights] || [SDMOption || SDM AccessRights || VCUIDOffset ||
                                                                                 *             SDMReadCtrOffset || PICCDataOffset || SDMACInputOffset || SDMENCOffset ||
                                                                                 *             SDMENCLength || SDMMACOffset] || [TMCLimit]
                                                                                 */
    );

/**
 * end of group phalMfdfEVx_FileManagement
 * @}
 */




/* MIFARE DESFire EVx Data mamangement commands. --------------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_DataManagement Commands_DataManagement
 * \brief These Components implement the MIFARE DESFire EVx Data Management related commands.
 * @{
 */

/** \name Chaining format to be used for DataManagement commands. */
/** @{ */
#define PHAL_MFDFEVX_APPLICATION_CHAINING                               0x00U   /**< Option to represent the application chaining format.
                                                                                 *   Here AF will be available in command and response.
                                                                                 */
#define PHAL_MFDFEVX_ISO_CHAINING                                       0x01U   /**< Option to represent the ISO14443-4 chaining format.
                                                                                 *   Here AF will not be available in command and response rather
                                                                                 *   the chaining will be done using the ISO14443-4 protocol.
                                                                                 */
/** @} */

/**
 * \brief Reads data from standard data files or backup data files
 *
 * \remarks
 * Chaining upto the size of the HAL Rx buffer is handled within this function. If more data is to be read, the user has to call
 * this function again with bOption = #PH_EXCHANGE_RXCHAINING | [one of the communication options]
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval #PH_ERR_SUCCESS_CHAINING indicating more data to be read.
 * \retval Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_ReadData(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 *
                                                                                 *          \c #PH_EXCHANGE_RXCHAINING: To be Or'd with the above option
                                                                                 *              flag if Chaining status is returned.
                                                                                 */
        uint8_t bIns,                                                           /**< [In] One of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_APPLICATION_CHAINING
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_CHAINING
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number from where the data to be read.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pOffset,                                                      /**< [In] The offset from where the data should be read. Will be of 3 bytes with LSB first.
                                                                                 *          \c If 0x10 need to be offset then it will be 10 00 00.
                                                                                 */
        uint8_t * pLength,                                                      /**< [In] The number of bytes to be read. Will be of 3 bytes with LSB first.
                                                                                 *          \c If 0x10 bytes need to be read then it will be 10 00 00. \n
                                                                                 *          \c If complete file need to be read then it will be 00 00 00.
                                                                                 */
        uint8_t ** ppRxdata,                                                    /**< [Out] The data retuned by the PICC. */
        uint16_t * pRxdataLen                                                   /**< [Out] Length of bytes available in RxData buffer. */
    );

/**
 * \brief Writes data to standard data files or backup data files
 *
 * \remarks
 * Implements chaining to the card.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_WriteData(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bIns,                                                           /**< [In] One of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_APPLICATION_CHAINING
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_CHAINING
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to which the data to be written.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pOffset,                                                      /**< [In] The offset from where the data should be written. Will be of 3 bytes with LSB first.
                                                                                 *          \c If 0x10 need to be offset then it will be 10 00 00.
                                                                                 */
        uint8_t * pTxData,                                                      /**< [In] The data to be written to the PICC. */
        uint8_t * pTxDataLen                                                    /**< [In] The number of bytes to be written. Will be of 3 bytes with LSB first.
                                                                                 *          \c If 0x10 bytes need to be written then it will be 10 00 00.
                                                                                 */
    );

/**
 * \brief Reads the currently stored value from value files.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetValue(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number for which the value to the retrieved.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pValue                                                        /**< [Out] The value returned by the PICC. The buffer should be 4 bytes. */
    );

/**
 * \brief Increases a value stored in a Value File
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_Credit(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to which the value should be credited.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pValue                                                        /**< [Out] The value to be credited. Will be of 4 bytes with LSB first.
                                                                                 *          \c If 0x10 bytes need to be credited then it will be 10 00 00 00.
                                                                                 */
    );

/**
 * \brief Decreases a value stored in a Value File
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_Debit(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to which the value should be debited.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pValue                                                        /**< [Out] The value to be debited. Will be of 4 bytes with LSB first.
                                                                                 *          \c If 0x10 bytes need to be debited then it will be 10 00 00 00.
                                                                                 */
    );

/**
 * \brief Allows a limited increase of a value stored in a Value File without having full credit permissions to the file.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_LimitedCredit(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to which the value should be credited.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pValue                                                        /**< [Out] The value to be credited. Will be of 4 bytes with LSB first.
                                                                                 *          \c If 0x10 bytes need to be credited then it will be 10 00 00 00.
                                                                                 */
    );

/**
 * \brief Reads out a set of complete records from a Cyclic or Linear Record File.
 *
 * \remarks
 * The readrecords command reads and stores data in the rxbuffer upto the rxbuffer size before returning
 * to the user. The rxbuffer is configured during the HAL init and this is specified by the user.
 *
 * Chaining upto the size of the HAL Rx buffer is handled within this function.
 * If more data is to be read, the user has to call this function again with
 * bCommOption = #PH_EXCHANGE_RXCHAINING | [one of the communication options]
 *
 * If TMI collection is ON, if pRecCount is zero then pRecSize is madatory parameter.
 * If pRecSize and RecCount are zero and TMI collection is ON, then #PH_ERR_INVALID_PARAMETER error returned.
 * If TMI collection is ON; and if wrong pRecSize is provided, then wrong RecCount value will be calculated and updated for TMI collection.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval #PH_ERR_SUCCESS_CHAINING indicating more data to be read.
 * \retval Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_ReadRecords(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 *
                                                                                 *          \c #PH_EXCHANGE_RXCHAINING: To be Or'd with the above option
                                                                                 *              flag if Chaining status is returned.
                                                                                 */
        uint8_t bIns,                                                           /**< [In] One of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_APPLICATION_CHAINING
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_CHAINING
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number from where the data to be read.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pRecNo,                                                       /**< [In] 3 bytes LSB first. Record number of the newest record targeted, starting to count
                                                                                 *        from the latest record written.
                                                                                 */
        uint8_t * pRecCount,                                                    /**< [In] 3 bytes LSB first. Number of records to be read. If 0x00 00 00, then all the records are read. */
        uint8_t * pRecSize,                                                     /**< [In] The number of bytes to be read. Will be of 3 bytes with LSB first. */
        uint8_t ** ppRxdata,                                                    /**< [Out] The data retuned by the PICC. */
        uint16_t * pRxdataLen                                                   /**< [Out] Length of bytes available in RxData buffer. */
    );

/**
 * \brief Writes data to a record in a Cyclic or Linear Record File.
 *
 * \remarks
 * Implements chaining to the card.
 * The data provided on pData will be chained to the card by sending data upto the frame size of the DESFire PICC, at a time.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_WriteRecord(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bIns,                                                           /**< [In] One of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_APPLICATION_CHAINING
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_CHAINING
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to which the data should be written.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pOffset,                                                      /**< [In] The offset from where the data should be written. Will be of 3 bytes with LSB first.
                                                                                 *          \c If 0x10 need to be offset then it will be 10 00 00.
                                                                                 */
        uint8_t * pData,                                                        /**< [In] The data to be written to the PICC. */
        uint8_t * pDataLen                                                      /**< [In] The number of bytes to be written. Will be of 3 bytes with LSB first.
                                                                                 *          \c If 0x10 bytes need to be written then it will be 10 00 00.
                                                                                 */
    );

/**
 * \brief Updates data to a record in a Cyclic or Linear Record File.
 *
 * \remarks
 * Implements chaining to the card.
 * The data provided on pData will be chained to the card by sending data upto the frame size of the DESFire PICC, at a time.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_UpdateRecord(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bIns,                                                           /**< [In] One of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_APPLICATION_CHAINING
                                                                                 *          \arg #PHAL_MFDFEVX_ISO_CHAINING
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to which the data should be written.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t * pRecNo,                                                       /**< [In] Record number. Records are numbered starting with the latest record written. */
        uint8_t * pOffset,                                                      /**< [In] Starting position for the update operationwithin the targeted record. */
        uint8_t * pData,                                                        /**< [In] The data to be written to the PICC. */
        uint8_t * pDataLen                                                      /**< [In] The number of bytes to be written. Will be of 3 bytes with LSB first.
                                                                                 *          \c If 0x10 bytes need to be written then it will be 10 00 00.
                                                                                 */
    );

/**
 * \brief Resets a Cyclic or Linear Record File.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_ClearRecordFile(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bFileNo                                                         /**< [In] The file number to be cleared.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
    );

/**
 * end of group phalMfdfEVx_DataManagement
 * @}
 */




/* MIFARE DESFire EVx Transaction mamangement commands. -------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_TransactionnManagement Commands_TransactionManagement
 * \brief These Components implement the MIFARE DESFire EVx Transaction Management related commands.
 * @{
 */

/** \name Options for CommitTransaction command. */
/** @{ */
#define PHAL_MFDFEVX_COMMIT_TXN_OPTION_NOT_EXCHANGED                    0x00    /**< Option byte is not exchanged to the PICC. */
#define PHAL_MFDFEVX_COMMIT_TXN_NO_TMC_TMV_RETURNED                     0x80    /**< Option byte exchanged to PICC and represent no return of TMC and TMV. */
#define PHAL_MFDFEVX_COMMIT_TXN_TMC_TMV_RETURNED                        0x81    /**< Option byte exchanged to PICC and represent return of TMC and TMV. */
/** @} */

/**
 * \brief Validates all previous write access' on Backup Data files, value files and record files within one application.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CommitTransaction(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] One of the below options.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMIT_TXN_OPTION_NOT_EXCHANGED
                                                                                 *          \arg #PHAL_MFDFEVX_COMMIT_TXN_NO_TMC_TMV_RETURNED
                                                                                 *          \arg #PHAL_MFDFEVX_COMMIT_TXN_TMC_TMV_RETURNED
                                                                                 */
        uint8_t * pTMC,                                                         /**< [Out] The increased value as stored in FileType.TransactionMAC. \n
                                                                                 *          \c 4 byte Transaction MAC counter value.
                                                                                 */
        uint8_t * pTMV                                                          /**< [Out] 8 bytes Transaction MAC value. */
    );

/**
 * \brief Aborts all previous write accesses on FileType.BackupData, FileType.Value, File-Type.LinearRecord and
 * FileType.CyclicRecord files within the selected application(s). If applicable, the Transaction MAC calculation is aborted.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_AbortTransaction(
        void * pDataParams                                                      /**< [In] Pointer to this layer's parameter structure. */
    );

/**
 * \brief Secures the transaction by commiting the application to ReaderID specified
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CommitReaderID(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pTMRI,                                                        /**< [In] 16 bytes of Transaction MAC reader ID. */
        uint8_t * pEncTMRI                                                      /**< [Out] 16 bytes of Encrypted Transaction MAC ReaderID of the latest successful transaction. */
    );

/**
 * end of group phalMfdfEVx_TransactionnManagement
 * @}
 */




/* MIFARE DESFire EVx ISO7816-4 commands. ---------------------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_ISO7816 Commands_ISO7816
 * \brief These Components implement the MIFARE DESFire EVx ISO/IEC 7816-4 Basic commands.
 * @{
 */

/** \name Options for ISOSelectFile command. */
/** @{ */
#define PHAL_MFDFEVX_FCI_RETURNED                                       0x00U   /**< Option to indicate the return of FCI. */
#define PHAL_MFDFEVX_FCI_NOT_RETURNED                                   0x0CU   /**< Option to indicate the no return of FCI. */
#define PHAL_MFDFEVX_SELECTOR_0                                         0x00U   /**< Option to indicate Selection by 2 byte file Id. */
#define PHAL_MFDFEVX_SELECTOR_1                                         0x01U   /**< Option to indicate Selection by child DF. */
#define PHAL_MFDFEVX_SELECTOR_2                                         0x02U   /**< Option to indicate Select EF under current DF. Fid = EF id. */
#define PHAL_MFDFEVX_SELECTOR_3                                         0x03U   /**< Option to indicate Select parent DF of the current DF. */
#define PHAL_MFDFEVX_SELECTOR_4                                         0x04U   /**< Option to indicate Selection by DF Name. DFName and len is then valid. */
/** @} */

/**
 * \brief ISO Select. This command is implemented in compliance with ISO/IEC 7816-4.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_IsoSelectFile(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                                        /**< [In] Option for return / no return of FCI.
                                                                                 *          \arg #PHAL_MFDFEVX_FCI_RETURNED
                                                                                 *          \arg #PHAL_MFDFEVX_FCI_NOT_RETURNED
                                                                                 */
        uint8_t bSelector,                                                      /**< [In] The selector to be used.
                                                                                 *          \arg #PHAL_MFDFEVX_SELECTOR_0
                                                                                 *          \arg #PHAL_MFDFEVX_SELECTOR_1
                                                                                 *          \arg #PHAL_MFDFEVX_SELECTOR_2
                                                                                 *          \arg #PHAL_MFDFEVX_SELECTOR_3
                                                                                 *          \arg #PHAL_MFDFEVX_SELECTOR_4
                                                                                 */
        uint8_t * pFid,                                                         /**< [In] The ISO File number to be selected. */
        uint8_t * pDFname,                                                      /**< [In] The ISO DFName to be selected. Valid only when bOption = 0x04. */
        uint8_t bDFnameLen,                                                     /**< [In] Length of bytes available in DFname buffer. */
        uint8_t bExtendedLenApdu,                                               /**< [In] Flag for Extended Length APDU. */
        uint8_t ** ppFCI,                                                       /**< [Out] The FCI information returned by the PICC. */
        uint16_t * pwFCILen                                                     /**< [Out] Length of bytes available in FCI buffer. */
    );

/**
 * \brief ISO Read Binary. This command is implemented in compliance with ISO/IEC 7816-4.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval #PH_ERR_SUCCESS_CHAINING operation success with chaining.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_IsoReadBinary(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wOption,                                                       /**< [In] One of the below options.
                                                                                 *          \arg #PH_EXCHANGE_DEFAULT   : To exchange the initial command to the PICC.
                                                                                 *          \arg #PH_EXCHANGE_RXCHAINING: To exchange the chaining command to PICC in
                                                                                 *                                        case PICC returns AF as the status.
                                                                                 */
        uint8_t bOffset,                                                        /**< [In] The offset from where the data should be read. */
        uint8_t bSfid,                                                          /**< [In] Short ISO File Id. Bit 7 should be 1 to indicate Sfid is supplied.
                                                                                 *        Else it is treated as MSB of 2Byte offset.
                                                                                 */
        uint32_t dwBytesToRead,                                                 /**< [In] Number of bytes to read. If 0, then entire file to be read. */
        uint8_t bExtendedLenApdu,                                               /**< [In] Flag for Extended Length APDU. */
        uint8_t ** ppRxBuffer,                                                  /**< [Out] The data retuned by the PICC. */
        uint32_t * pBytesRead                                                   /**< [Out] Length of bytes available in RxData buffer. */
    );

/**
 * \brief Iso Update Binary. This command is implemented in compliance with ISO/IEC 7816-4.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_IsoUpdateBinary(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOffset,                                                        /**< [In] The offset from where the data should be updated. */
        uint8_t bSfid,                                                          /**< [In] Short ISO File Id. Bit 7 should be 1 to indicate Sfid is supplied.
                                                                                 *        Else it is treated as MSB of 2Byte offset.
                                                                                 */
        uint8_t bExtendedLenApdu,                                               /**< [In] Flag for Extended Length APDU. */
        uint8_t * pData,                                                        /**< [In] Data to be updated. */
        uint32_t dwDataLen                                                      /**< [In] Length of bytes available in Data buffer. */
    );

/**
 * \brief Iso Read Records. This command is implemented in compliance with ISO/IEC 7816-4.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_IsoReadRecords(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wOption,                                                       /**< [In] One of the below options.
                                                                                 *          \arg #PH_EXCHANGE_DEFAULT   : To exchange the initial command to the PICC.
                                                                                 *          \arg #PH_EXCHANGE_RXCHAINING: To exchange the chaining command to PICC in
                                                                                 *                                        case PICC returns AF as the status.
                                                                                 */
        uint8_t bRecNo,                                                         /**< [In] Record to read / from where to read. */
        uint8_t bReadAllFromP1,                                                 /**< [In] Whether to read all records from P1 or just one. */
        uint8_t bSfid,                                                          /**< [In] Short ISO File Id. */
        uint32_t dwBytesToRead,                                                 /**< [In] Number of bytes to read. */
        uint8_t bExtendedLenApdu,                                               /**< [In] Flag for Extended Length APDU. */
        uint8_t ** ppRxBuffer,                                                  /**< [Out] The data retuned by the PICC. */
        uint32_t * pBytesRead                                                   /**< [Out] Length of bytes available in RxData buffer. */
    );

/**
 * \brief Iso Append record. This command is implemented in compliance with ISO/IEC 7816-4.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_IsoAppendRecord(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bSfid,                                                          /**< [In] Short ISO File Id. */
        uint8_t * pData,                                                        /**< [In] Data to be appended. */
        uint32_t dwDataLen,                                                     /**< [In] Length of bytes available in Data buffer. */
        uint8_t bExtendedLenApdu                                                /**< [In] Flag for Extended Length APDU. */
    );

/**
 * \brief Iso Update record
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_IsoUpdateRecord(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bIns,                                                           /**< [In] Either 0xDC or 0xDD. */
        uint8_t bRecNo,                                                         /**< [In] Record number. 00 indicates current record. */
        uint8_t bSfid,                                                          /**< [In] Short Iso File Id bits 0..4 only code this value. Either 0 or sfid. */
        uint8_t bRefCtrl,                                                       /**< [In] Bit 0 to bit 3 code the value based on bIns as defined in ISO 7816-4 Update Record. */
        uint8_t * pData,                                                        /**< [In] Data to write. */
        uint8_t bDataLen                                                        /**< [In] number of bytes to write. */
    );

/**
 * \brief ISOGetChallenge. This command is implemented in compliance with ISO/IEC 7816-4.
 * Returns the random number from the PICC. Size depends on the key type referred by wKeyNo and wKeyVer.
 *
 * \remarks
 * THIS COMMAND IS NOT SUPPORTED IN SAM-X Configuration.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_IsoGetChallenge(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                        /**< [In] Key number to be used form keystore. */
        uint16_t wKeyVer,                                                       /**< [In] Key version to be used from key store. */
        uint8_t bExtendedLenApdu,                                               /**< [In] Flag for Extended Length APDU. */
        uint32_t dwLe,                                                          /**< [In] Length of expected challenge RPICC1. */
        uint8_t * pRPICC1                                                       /**< [Out] RPICC1 returned from PICC. */
    );


/**
 * end of group phalMfdfEVx_ISO7816
 * @}
 */




/* MIFARE DESFire EVx Originality Check functions. ------------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_OriginalityCheck Commands_OriginalityCheck
 * \brief These Components implement the MIFARE DESFire EVx Originality Check functions.
 * @{
 */

/**
 * \brief Performs the originality check to verify the genuineness of PICC
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_ReadSign(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bAddr,                                                          /**< [In] Value is always 00. Present for forward compatibility reasons */
        uint8_t ** pSignature                                                   /**< [Out] The plain 56 bytes originality signature of the PICC. */
    );

/**
 * end of group phalMfdfEVx_OriginalityCheck
 * @}
 */




/* MIFARE DESFire EVx MIFARE Classic functions. ---------------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_MFC Commands_MIFARE_ClassicSupport
 * \brief These Components implement the MIFARE DESFire EVx MIFARE Classic Support functions.
 * @{
 */

/**
 * \brief Configure a mapping between one or more MIFARE Classic blocks and a DESFire FileType.StandardData,
 * FileType.BackupData and FileType.Value file. \cond !PUBLIC To compute the MFCLicenseMAC call \ref phalMfdfEVx_ComputeMFCLicenseMAC
 * command. \endcond
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_CreateMFCMapping(
        void * pDataParams,                                                     /**< [In] Pointer to this layers parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] The option to be enabled / disabled of the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_ENC
                                                                                 */
        uint8_t bFileNo,                                                        /**< [In] The file number to be created.
                                                                                 *        ORed with #PHAL_MFDFEVX_SAI to indicate secondary application indicator.
                                                                                 */
        uint8_t bFileOption,                                                    /**< [In] The option to be enabled / disabled of the file. */
        uint8_t * pMFCBlockList,                                                /**< [In] List of block numbers of the targeted blocks. */
        uint8_t bMFCBlocksLen,                                                  /**< [In] Length of Bytes available in MFCBlockList buffer. */
        uint8_t bRestoreSource,                                                 /**< [In] File number of the allowed restore source. */
        uint8_t * pMFCLicense,                                                  /**< [In] MIFARE Classic License. Should be as follows
                                                                                 *          \c BlockCount || BlockNr1 || BlockOption1 [|| BlockNr2 || BlockOption2 || ...
                                                                                 *             || BlockNrN || BlockOptionN]
                                                                                 */
        uint8_t bMFCLicenseLen,                                                 /**< [In] Length of Bytes available in MFCLicense buffer. */
        uint8_t * pMFCLicenseMAC                                                /**< [In] The computed MFCLicenseMAC. Should be 8 bytes. */
    );

/**
 * \brief Restores the value from one FileType.Value file by transfering it into another File-Type.Value file.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_RestoreTransfer(
        void * pDataParams,                                                     /**< [In] Pointer to this layers parameter structure. */
        uint8_t bCommOption,                                                    /**< [In] Communication settings for the file.
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_PLAIN
                                                                                 *          \arg #PHAL_MFDFEVX_COMMUNICATION_MACD
                                                                                 */
        uint8_t bTargetFileNo,                                                  /**< [In] File number of the TargetFile to be updated. */
        uint8_t bSourceFileNo                                                   /**< [In] File number of the SourceFile to be restored. */
    );


/**
 * end of group phalMfdfEVx_MFC
 * @}
 */








/* MIFARE DESFire EVx Miscellaneous functions. ------------------------------------------------------------------------------------- */
/**
 * \defgroup phalMfdfEVx_Miscellaneous Commands_Miscellaneous
 * \brief These Components implement the MIFARE DESFire EVx Miscellaneous functions. These are not part of actual DESFire EVx data sheet
 * rather its for internal purpose.
 * @{
 */

/** \name The configuration to be used for SetConfig / GetConfig. */
/** @{ */
#define PHAL_MFDFEVX_ADDITIONAL_INFO                                    0x00A1  /**< Option for Getconfig/SetConfig to get/set additional info of a generic error or some some length exposed by interfaces. */
#define PHAL_MFDFEVX_WRAPPED_MODE                                       0x00A2  /**< Option for GetConfig/SetConfig to get/set current status of command wrapping in ISO 7816-4 APDUs. */
#define PHAL_MFDFEVX_SHORT_LENGTH_APDU                                  0x00A3  /**< Option for Getconfig/SetConfig to get/set current status of Short Length APDU wrapping in ISO 7816-4 APDUs. */
/** @} */

/**
 * \brief Perform a GetConfig command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_GetConfig(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                       /**< [In] Configuration to read. Will be one of the below values.
                                                                                 *          \arg #PHAL_MFDFEVX_ADDITIONAL_INFO
                                                                                 *          \arg #PHAL_MFDFEVX_WRAPPED_MODE
                                                                                 *          \arg #PHAL_MFDFEVX_SHORT_LENGTH_APDU
                                                                                 */
         uint16_t * pValue                                                      /**< [Out] The value for the mentioned configuration. */
    );

/**
 * \brief Perform a SetConfig command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_SetConfig(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                       /**< [In] Configuration to set. Will be one of the below values.
                                                                                 *          \arg #PHAL_MFDFEVX_ADDITIONAL_INFO
                                                                                 *          \arg #PHAL_MFDFEVX_WRAPPED_MODE
                                                                                 *          \arg #PHAL_MFDFEVX_SHORT_LENGTH_APDU
                                                                                 */
        uint16_t wValue                                                         /**< [In] The value for the mentioned configuration. */
    );

/**
 * \brief Reset the authentication
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_ResetAuthentication(
        void * pDataParams                                                      /**< [In] Pointer to this layer's parameter structure. */
    );


/**
 * \brief This is a utility API which sets the VCA structure in Desfire EVx structure params
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMfdfEVx_SetVCAParams(
        void * pDataParams,                                                     /**< [In] Pointer to this layers parameter structure. */
        void * pAlVCADataParams                                                 /**< [In] Pointer to VCA AL parameter structure. */
    );

/**
 * end of group phalMfdfEVx_Miscellaneous
 * @}
 */

/**
 * end of group phalMfdfEVx
 * @}
 */
#endif /* NXPBUILD__PHAL_MFDFEVx */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHALMFDFEVX_H */
