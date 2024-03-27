/*----------------------------------------------------------------------------*/
/* Copyright 2009-2022 NXP                                                    */
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
* Generic MIFARE Ultralight contactless IC Application Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#ifndef PHALMFUL_H
#define PHALMFUL_H

#include <ph_Status.h>
#include <phpalMifare.h>
#include <phhalHw.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifdef NXPBUILD__PHAL_MFUL_SW

/** \defgroup phalMful_Sw Component : Software
 * @{
 */

#define PHAL_MFUL_SW_ID         0x01U    /**< ID for Software MIFARE Ultralight layer */

/**
* \brief Private parameter structure
*/
typedef struct
{
    uint16_t wId;                   /**< Layer ID for this component, NEVER MODIFY! */
    void * pPalMifareDataParams;    /**< Pointer to palMifare parameter structure. */
    void * pKeyStoreDataParams;     /**< Pointer to phKeystore parameter structure. */
    void * pCryptoDataParams;       /**< Pointer to phCrypto data parameters structure. */
    void * pCryptoRngDataParams;    /**< Pointer to the parameter structure of the CryptoRng layer. */
    uint8_t bCMACReq;               /**< Get/Set Config to include CMAC for required Commands */
    uint16_t wCmdCtr;               /**< Command count within transaction. */
    uint8_t bAuthMode;              /**< Authentication PWD [0x1B] or AES [0x1A] */
    uint8_t bAdditionalInfo;        /**< Specific error codes for MFUL generic errors or to get the response length of some commands. */
} phalMful_Sw_DataParams_t;

/**
* \brief Initialise this layer.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMful_Sw_Init(
                            phalMful_Sw_DataParams_t * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                            uint16_t wSizeOfDataParams,                 /**< [In] Specifies the size of the data parameter structure. */
                            void * pPalMifareDataParams,                /**< [In] Pointer to palMifare parameter structure. */
                            void * pKeyStoreDataParams,                 /**< [In] Pointer to phKeystore parameter structure. */
                            void * pCryptoDataParams,                   /**< [In] Pointer to phCrypto data parameters structure. */
                            void * pCryptoRngDataParams                 /**< [In] Pointer to the parameter structure of the CryptoRng layer. */
                            );

/**
 * end of phalMful_Sw
 * @}
 */
#endif /* NXPBUILD__PHAL_MFUL_SW */



#ifdef NXPBUILD__PHAL_MFUL

/** \defgroup phalMful MIFARE(R) Ultralight
* \brief These Components implement the MIFARE(R) Ultralight, C, Ultralight EV1 commands.
* @{
*/

/**
* \name Block definitions
*/
/*@{*/
#define PHAL_MFUL_READ_BLOCK_LENGTH         16U    /**< Length of a read MIFARE(R) Ultralight data block. */
#define PHAL_MFUL_WRITE_BLOCK_LENGTH        4U     /**< Length of a write MIFARE(R) Ultralight data block. */
#define PHAL_MFUL_COMPWRITE_BLOCK_LENGTH    16U    /**< Length of a compatibility write MIFARE(R) Ultralight data block. */
#define PHAL_MFUL_COUNTER_RD_VALUE_LENGTH   3U     /**< Length of a counter MIFARE(R) Ultralight Read data block. */
#define PHAL_MFUL_COUNTER_WR_VALUE_LENGTH   4U     /**< Length of a counter MIFARE(R) Ultralight Write data block. */
#define PHAL_MFUL_PACK_LENGTH               2U     /**< Length of a password ack MIFARE(R) Ultralight data block. */
#define PHAL_MFUL_VERSION_LENGTH            8U     /**< Length of a Version MIFARE(R) Ultralight data block. */
#define PHAL_MFUL_SIG_LENGTH                32U    /**< Length of a Version MIFARE(R) Ultralight data block. */
#define PHAL_MFUL_SIG_LENGTH_48             48U    /**< Length of a Version MIFARE(R) Ultralight data block. */
/** @} */

/**
* \name Crypto definitions
*/
/*@{*/
#define PHAL_MFUL_DES_BLOCK_SIZE    8U      /**< Length of an MIFARE(R) Ultralight DES block. */
#define PHAL_MFUL_AES_BLOCK_SIZE    16U     /**< Length of an MIFARE(R) Ultralight DES block. */
#define PHAL_MFUL_DES_KEY_LENGTH    16U     /**< Length of an MIFARE(R) Ultralight DES key. */
/*@}*/

/** \name Diversification option for Ultralight C Authenticate command. */
/** @{ */
#define PHAL_MFUL_CMD_UL_AUTHENTICATE_DIV_OFF                   0   /**< Option to indicate the diversification is disabled. */
#define PHAL_MFUL_CMD_UL_AUTHENTICATE_DIV_ON                    1   /**< Option to indicate the diversification is enabled. */
/** @} */

/** \name Lock option for Ultralight LockSign command. */
/** @{ */
#define PHAL_MFUL_CMD_UL_LOCK_SIGN_UNLOCK                       0U  /**< Option to unlock the signature. */
#define PHAL_MFUL_CMD_UL_LOCK_SIGN_LOCK                         1U  /**< Option to temporary lock the signature. */
#define PHAL_MFUL_CMD_UL_LOCK_SIGN_PERMENT_LOCK                 2U  /**< Option to permanently lock the signature. */
/** @} */

/**
* \name The configuration to be used for SetConfig / GetConfig
*/
/*@{*/
#define PHAL_MFUL_CMAC_STATUS                               0xA1U   /**< Option to Get/Set for inclusion of CMAC data for command and response. */
#define PHAL_MFUL_ADDITIONAL_INFO                           0xA2U   /**< Option to Get/Set additional info of a generic error or length of response data. */
/*@}*/

#ifdef NXPRDLIB_REM_GEN_INTFS
#include "../comps/phalMful/src/Sw/phalMful_Sw.h"

#define phalMful_UlcAuthenticate(pDataParams, bOption, wKeyNo, wKeyVer, pDivInput, bDivInputLen) \
        phalMful_Sw_UlcAuthenticate((phalMful_Sw_DataParams_t *)pDataParams, wKeyNo, wKeyVer)

#define phalMful_Read(pDataParams, bAddress, pData) \
        phalMful_Sw_Read((phalMful_Sw_DataParams_t *)pDataParams, bAddress, pData)

#define phalMful_SectorSelect(pDataParams, bSecNo) \
        phalMful_Sw_SectorSelect((phalMful_Sw_DataParams_t *)pDataParams, bSecNo)

#define phalMful_Write(pDataParams, bAddress, pData) \
        phalMful_Sw_Write((phalMful_Sw_DataParams_t *)pDataParams, bAddress, pData)

#define phalMful_FastWrite(pDataParams, pData) \
        phalMful_Sw_FastWrite((phalMful_Sw_DataParams_t *)pDataParams, pData)

#define phalMful_CompatibilityWrite(pDataParams, bAddress, pData) \
        phalMful_Sw_CompatibilityWrite((phalMful_Sw_DataParams_t *)pDataParams, bAddress, pData)

#define phalMful_IncrCnt(pDataParams, bCntNum, pCnt) \
        phalMful_Sw_IncrCnt((phalMful_Sw_DataParams_t *)pDataParams, bCntNum, pCnt)

#define phalMful_ReadCnt(pDataParams, bCntNum, pCntValue) \
        phalMful_Sw_ReadCnt((phalMful_Sw_DataParams_t *)pDataParams, bCntNum, pCntValue)

#define phalMful_PwdAuth(pDataParams, pPwd, pPack) \
        phalMful_Sw_PwdAuth((phalMful_Sw_DataParams_t *)pDataParams, pPwd, pPack)

#define phalMful_GetVersion(pDataParams, pVersion) \
        phalMful_Sw_GetVersion((phalMful_Sw_DataParams_t *)pDataParams, pVersion)

#define phalMful_FastRead(pDataParams, bStartAddr, bEndAddr, ppData, pNumBytes) \
        phalMful_Sw_FastRead((phalMful_Sw_DataParams_t *)pDataParams, bStartAddr, bEndAddr, ppData, pNumBytes)

#define phalMful_ReadSign(pDataParams, bAddr, pSignature) \
        phalMful_Sw_ReadSign((phalMful_Sw_DataParams_t *)pDataParams, bAddr, pSignature)

#define phalMful_ChkTearingEvent(pDataParams, bCntNum, pValidFlag) \
        phalMful_Sw_ChkTearingEvent((phalMful_Sw_DataParams_t *)pDataParams, bCntNum, pValidFlag)

#define phalMful_WriteSign(pDataParams, bAddress, pSignature) \
        phalMful_Sw_WriteSign((phalMful_Sw_DataParams_t *)pDataParams, bAddress, pSignature)

#define phalMful_LockSign(pDataParams, bLockMode) \
        phalMful_Sw_LockSign((phalMful_Sw_DataParams_t *)pDataParams, bLockMode)

#define phalMful_VirtualCardSelect(pDataParams, pVCIID, bVCIIDLen, pVCTID) \
        phalMful_Sw_VirtualCardSelect((phalMful_Sw_DataParams_t *)pDataParams, pVCIID, bVCIIDLen, pVCTID)

#ifdef NXPBUILD__PH_CRYPTOSYM
#define phalMful_AuthenticateAES(pDataParams, bOption, wKeyNo, wKeyVer, bRamKeyNo, bRamKeyVer, bKeyNoCard, pDivInput, bDivLen) \
        phalMful_Sw_AuthenticateAES((phalMful_Sw_DataParams_t *)pDataParams, bOption, wKeyNo, wKeyVer, bRamKeyNo, bRamKeyVer, bKeyNoCard, pDivInput, bDivLen)

#define phalMful_GetConfig(pDataParams, wConfig, pValue) \
        phalMful_Sw_GetConfig((phalMful_Sw_DataParams_t *)pDataParams, wConfig, pValue)

#define phalMful_SetConfig(pDataParams, wConfig, wValue) \
        phalMful_Sw_SetConfig((phalMful_Sw_DataParams_t *)pDataParams, wConfig, wValue)

#define phalMful_CalculateSunCMAC(pDataParams, wKeyNo, wKeyVer, pInData, wInDataLen, pRespMac) \
        phalMful_Sw_CalculateSunCMAC((phalMful_Sw_DataParams_t *)pDataParams, wKeyNo, wKeyVer, pInData, wInDataLen, pRespMac)
#endif /* NXPBUILD__PH_CRYPTOSYM */
#else

/**
* \brief Perform MIFARE(R) Ultralight-C Authenticate command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval #PH_ERR_AUTH_ERROR Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_UlcAuthenticate(
        void * pDataParams,                                         /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                            /**< [In] Option for specifying the diversification input.
                                                                     *           Only valid for Sam AV3
                                                                     *          \arg PHAL_MFUL_CMD_UL_AUTHENTICATE_DIV_OFF
                                                                     *          \arg PHAL_MFUL_CMD_UL_AUTHENTICATE_DIV_ON
                                                                     */
        uint16_t wKeyNo,                                            /**< [In] Key number to be used in authentication. */
        uint16_t wKeyVer,                                           /**< [In] Key version to be used in authentication. */
        uint8_t * pDivInput,                                        /**< [In] Diversification input for key diversification. (1 to 31 byte(s) input). */
        uint8_t bDivInputLen                                        /**< [In] Length of diversification input. */
    );

/**
* \brief Perform MIFARE(R) Ultralight Read command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_Read(
                         void * pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                         uint8_t bAddress,      /**< [In] Address on Picc to read from. */
                         uint8_t * pData        /**< [Out] pData[#PHAL_MFUL_READ_BLOCK_LENGTH] containing data returned from the Picc. */
                         );

/**
* \brief Perform MIFARE(R) Ultralight Write command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_Write(
                          void * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                          uint8_t bAddress,     /**< [In] Address on Picc to write to. */
                          uint8_t * pData       /**< [In] pData[#PHAL_MFUL_WRITE_BLOCK_LENGTH] containing block to be written to the Picc. */
                          );

/**
* \brief Perform MIFARE(R) Ultralight Fast Write command which writes 64 bytes from 0xF0 to 0xFF address blocks.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_FastWrite(
                              void * pDataParams,   /**< [In] Pointer to this layer's parameter structure. */
                              uint8_t * pData       /**< [In] pData[64Bytes] to be written to the Picc. */
                              );

/**
* \brief Perform MIFARE(R) Ultralight Compatibility-Write command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_CompatibilityWrite(
                                       void * pDataParams,  /**< [In] Pointer to this layer's parameter structure. */
                                       uint8_t bAddress,    /**< [In] Address on Picc to write to. */
                                       uint8_t * pData      /**< [In] pData[#PHAL_MFUL_COMPWRITE_BLOCK_LENGTH] containing block to be written to the Picc. */
                                       );

/**
* \brief Perform MIFARE(R) Ultralight EV1 Increment count command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_IncrCnt(
                            void * pDataParams, /**< [In] Pointer to this layer's parameter structure. */
                            uint8_t bCntNum,    /**< [In] One byte counter number 00 to 02 */
                            uint8_t * pCnt      /**< [In] pCnt[4] counter value LSB first. Only first three data bytes are used by the PICC. The 4th byte is ignored */
                            );

/**
* \brief Perform MIFARE(R) Ultralight EV1 Read count command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_ReadCnt(
                            void * pDataParams, /**< [In] Pointer to this layer's parameter structure. */
                            uint8_t bCntNum,    /**< [In] One byte counter number 00 to 02 */
                            uint8_t * pCntValue /**< [Out] pCntValue[3] counter value. LSB first. Three bytes counter value returned from PICC. */
                            );
/**
* \brief Perform MIFARE(R) Ultralight EV1 Password auth command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_PwdAuth(
                            void * pDataParams, /**< [In] Pointer to this layer's parameter structure. */
                            uint8_t * pPwd,     /**< [In] Four byte array of type uint8_t containing four password bytes */
                            uint8_t * pPack     /**< [Out] Two byte array of type uint8_t containing two password acknowledge bytes */
                            );

/**
* \brief Perform MIFARE(R) Ultralight EV1 get version command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_GetVersion(
                               void * pDataParams, /**< [In] Pointer to this layer's parameter structure. */
                               uint8_t * pVersion  /**< [Out] An eight byte array containing version information bytes */
                               );
/**
* \brief Perform MIFARE(R) Ultralight EV1 Fast read command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_FastRead(
                             void * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                             uint8_t  bStartAddr,    /**< [In] One byte start address */
                             uint8_t bEndAddr,       /**< [In] One byte end address */
                             uint8_t ** ppData,       /**< [Out] Pointer to the data read from the card. The bytes are stored on the HAL's Rx buffer. */
                             uint16_t * wNumBytes    /**< [Out] Contains number of bytes read from the card */
                             );

/**
* \brief Perform Type 2 tag Sector Select command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_SectorSelect(
                                 void * pDataParams,    /**< [In] Pointer to this layer's parameter structure. */
                                 uint8_t bSecNo         /**< [In] Sector number to be selected. */
                                 );

/**
* \brief Perform MIFARE(R) Ultralight EV1 read signature command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_ReadSign(
                             void * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                             uint8_t bAddr,          /**< [In] Value is always 00. Present for forward compatibility reasons */
                             uint8_t ** pSignature   /**< [Out] Pointer to a 32/48 byte signature read from the card */
                             );

/**
* \brief Perform MIFARE(R) Ultralight EV1 check tearing event command with Picc.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_ChkTearingEvent(
                                    void * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                                    uint8_t bCntNum,        /**< [In] Value specifying the counter number 00 to 02 */
                                    uint8_t * pValidFlag    /**< [Out] Address of one byte containing the valid flag byte */
                                    );
/**
* \brief Perform MIFARE(R) Ultralight NANO Write Signature command with PICC.
*        This command writes the signature information to the PICC.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_WriteSign(
                              void * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                              uint8_t bAddress,       /**< [In] Relative page location of the signature part to be written. */
                              uint8_t * pSignature    /**< [Out] 4 bytes of signature value to be written to the specified relative page address. */
                              );

/**
* \brief Perform MIFARE(R) Ultralight NANO Lock Signature command with PICC.
*        This command locks the signature temporarily or permanently based on
*        the information provided in the API. The locking and unlocking of the
*        signature can be performed using this command if the signature is not
*        locked or temporary locked. If the signature is permanently locked,
*        then the unlocking cannot be done.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_LockSign(
                             void * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                             uint8_t bLockMode       /**< [In] Configuration to Lock, Unlock and Permanently Lock the signature data.
                                                      *          \arg PHAL_MFUL_CMD_UL_LOCK_SIGN_UNLOCK
                                                      *          \arg PHAL_MFUL_CMD_UL_LOCK_SIGN_LOCK
                                                      *          \arg PHAL_MFUL_CMD_UL_LOCK_SIGN_PERMENT_LOCK
                                                      */
                             );

/**
* \brief Perform MIFARE(R) Ultralight NANO Virtual Card Select command with PICC.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMful_VirtualCardSelect(
                                      void * pDataParams,     /**< [In] Pointer to this layer's parameter structure. */
                                      uint8_t * pVCIID,       /**< [In] The Virtual Card Installation Identifier data of EV1 and Nano for selecting the VC. */
                                      uint8_t bVCIIDLen,      /**< [In] Length of the VCIID data. */
                                      uint8_t * pVCTID        /**< [Out] 1 byte of Virtual card type identifier data. */
                                      );

/**
 * \brief Perform MIFARE(R) Ultralight NTAG Tag Temper Status Read.
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phalMful_ReadTTStatus(
        void * pDataParams,                                         /**< [In] Pointer to this layer's parameter structure. */
        uint8_t  bAddr,                                             /**< [In] Value is always 00. Present for forward compatibility reasonse */
        uint8_t * pData                                             /**< [Out] 5 bytes of Tag Temper Status. */
    );

#ifdef NXPBUILD__PH_CRYPTOSYM
/**
 * \brief Perform MIFARE(R) Ultralight-C AuthenticateAES command with Picc.
 * This will be using the AES128 keys and will
 * generate and verify the contents based on generic AES algorithm.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMful_AuthenticateAES (
        void * pDataParams,                                         /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bOption,                                            /**< [In] Option for specifying the diversification input.
                                                                     *           Only valid for Sam AV3
                                                                     *          \arg PHAL_MFUL_CMD_UL_AUTHENTICATE_DIV_OFF
                                                                     *          \arg PHAL_MFUL_CMD_UL_AUTHENTICATE_DIV_ON
                                                                     */
        uint16_t wKeyNo,                                            /**< [In] Key number to be used in authentication. */
        uint16_t wKeyVer,                                           /**< [In] Key version to be used in authentication. */
        uint8_t bRamKeyNo,                                          /**< [In] Key number of Destination Key where the computed session key will be stored.
                                                                     *        To be used for SAM AV3 only.
                                                                     */
        uint8_t bRamKeyVer,                                         /**< [In] Key version of Destination Key where the computed session key will be stored.
                                                                     *        To be used for SAM AV3 only.
                                                                     */
        uint8_t bKeyNoCard,                                         /**< [In] Key number on card. */
        uint8_t * pDivInput,                                        /**< [In] Diversification input. Can be NULL. */
        uint8_t bDivLen                                             /**< [In] Length of diversification input max 31B. */
    );

/**
 * \brief Perform a GetConfig command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMful_GetConfig(
        void * pDataParams,                                         /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                           /**< [In] Configuration to get the status of CMAC inclusion in command / response.
                                                                     *          \arg #PHAL_MFUL_CMAC_STATUS
                                                                     *          \arg #PHAL_MFUL_ADDITIONAL_INFO
                                                                     */
        uint16_t * pValue                                           /**< [Out] The value for the mentioned configuration. */
    );

/**
 * \brief Perform a SetConfig command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMful_SetConfig(
        void * pDataParams,                                         /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                           /**< [In] Configuration to set the status of CMAC inclusion in command / response.
                                                                     *          \arg #PHAL_MFUL_CMAC_STATUS
                                                                     *          \arg #PHAL_MFUL_ADDITIONAL_INFO
                                                                     */
        uint16_t wValue                                             /**< [In] The value for the mentioned configuration. */
    );

/**
 * \brief Calculate SUNCMAC.
 *
 * \note
 *      For the parameter pInData, Caller has to remove the ASCII formated data before passing to this interface. \n
 *      If the information is having TT status, User has to convert the ASCII bytes representation to NonASCII format. \n
 *          \c 43h should be converted to 03h \n
 *          \c 4Fh should be converted to 0Fh \n
 *          \c 49h should be converted to 09h \n
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalMful_CalculateSunCMAC(
        void * pDataParams,                                         /**< [In] Pointer to this layers parameter structure. */
        uint16_t wKeyNo,                                            /**< [In] Key number to be used from software or hardware keystore. */
        uint16_t wKeyVer,                                           /**< [In] Key version to be used from software or hardware keystore. */
        uint8_t * pInData,                                          /**< [In] Input Data for which the MAC to be calculated. */
        uint16_t wInDataLen,                                        /**< [In] Length of bytes available in InData buffer. */
        uint8_t * pRespMac                                          /**< [Out] Computed SUN CMAC. */
    );
#endif /* NXPBUILD__PH_CRYPTOSYM */
#endif /* NXPRDLIB_REM_GEN_INTFS */
/**
 * end of phalMful
 * @}
 */
#endif /* NXPBUILD__PHAL_MFUL */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHALMFUL_H */
