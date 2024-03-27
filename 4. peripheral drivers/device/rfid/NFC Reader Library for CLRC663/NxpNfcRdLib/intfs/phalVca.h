/*----------------------------------------------------------------------------*/
/* Copyright 2006 - 2021, 2022 NXP                                            */
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
* Generic Virtual Card Architecture (R) Application Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6537 $ (v07.09.00)
* $Date: 2022-01-14 11:13:00 +0530 (Fri, 14 Jan 2022) $
*
* History:
*  CHu: Generated 31. August 2009
*
*/

#ifndef PHALVCA_H
#define PHALVCA_H

#include <ph_Status.h>
#include <phhalHw.h>
#include <phpalMifare.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifdef NXPBUILD__PHAL_VCA_SW

/**
 * \defgroup phalVca_Sw Component : Software
 * @{
 */

#define PHAL_VCA_SW_ID                                              0x01    /**< ID for Software Virtual Card Architecture layer. */

/** \brief Structure for Card table */
typedef struct
{
    uint16_t wIidIndex;                                                     /**< Index of the associated IID. */
    uint8_t bValid;                                                         /**< Indicates if an entry is valid or not */
    uint8_t pCardData[16];                                                  /**< Card Data received from the Card. */
} phalVca_Sw_CardTableEntry_t;

/** \brief Structure for IID table */
typedef struct
{
    uint16_t wIidIndex;                                                     /**< Index of the associated IID. */
    uint16_t wKeyEncNumber;                                                 /**< Key number for Encryption key. */
    uint16_t wKeyEncVersion;                                                /**< Key version for Encryption key. */
    uint16_t wKeyMacNumber;                                                 /**< Key number for MAC key. */
    uint16_t wKeyMacVersion;                                                /**< Key version for MAC key. */
} phalVca_Sw_IidTableEntry_t;

/** \brief Enum defining VC states */
typedef enum
{
    VC_NOT_SELECTED                                                 = 0x00, /**< State to indicate no VC selection. */
    VC_PROXIMITYFAILED                                              = 0x01, /**< State to indicate Proximity Check Failure. */
    VC_PROXIMITYCHECK                                               = 0x02, /**< State to indicate Proximity Check. */
    VC_DF_NOT_AUTH                                                  = 0x03, /**< State to indicate DESFire PICC in not authenticaed. */
    VC_DF_AUTH_D40                                                  = 0x04, /**< State to indicate DESFire PICC in D40 authenticated. */
    VC_DF_AUTH_ISO                                                  = 0x05, /**< State to indicate DESFire PICC in ISO authenticated. */
    VC_DF_AUTH_EV2                                                  = 0x06, /**< State to indicate DESFire PICC in EV2 Authenticated. */
    VC_DF_AUTH_AES                                                  = 0x07, /**< State to indicate DESFire PICC in AES Authenticated. */
    VC_MFP_AUTH_AES_SL1                                             = 0x08, /**< State to indicate MIFARE Plus PICC authenticated in Security Level 1. */
    VC_MFP_AUTH_AES_SL3                                             = 0x09  /**< State to indicate MIFARE Plus PICC authenticated in Security Level 3. */
}phalVca_VirtualCardState;

/** \brief Enum defining PC states */
typedef enum
{
    PC_NO_PCHK_IN_PROGRESS                                          = 0x00, /**< State to indicate there is no Proximity check in progress. */
    PC_PPC_IN_PROGRESS                                              = 0x01, /**< State to indicate Proximity check preparation is in progress. */
    PC_PCHK_PREPARED                                                = 0x02, /**< State to indicate Proximity check is prepared. */
    PC_PCHK_IN_PROGRESS                                             = 0x03, /**< State to indicate Proximity check is in progress. */
    PC_WAITING_PC_VERIFICATION                                      = 0x04, /**< State to indicate waiting of Proximity check verification . */
    PC_VPC_IN_PROGRESS                                              = 0x05  /**< State to indicate Proximity check verification is in progress. */
}phalVca_ProximityCheckState;

/** \brief VCA Software parameter structure */
typedef struct
{
    uint16_t wId;                                                           /**< Layer ID for this component, NEVER MODIFY! */
    void * pPalMifareDataParams;                                            /**< Pointer to the parameter structure of the palMifare component. */
    void * pKeyStoreDataParams;                                             /**< Pointer to the parameter structure of the KeyStore layer. */
    void * pCryptoDataParams;                                               /**< Pointer to the parameter structure of the Crypto layer. */
    void * pCryptoRngDataParams;                                            /**< Pointer to the parameter structure of the CryptoRng layer. */
    phalVca_Sw_CardTableEntry_t * pCardTable;                               /**< Pointer to the Card Table of the layer. */
    phalVca_Sw_IidTableEntry_t * pIidTable;                                 /**< Pointer to the Iid Table of the layer. */
    uint16_t wCurrentCardTablePos;                                          /**< Position of current entry in the CardTable. */
    uint16_t wNumCardTableEntries;                                          /**< Number of Card table entries in the table. */
    uint16_t wNumIidTableEntries;                                           /**< Number of Iid table entries in the table. */
    uint16_t wCurrentIidIndex;                                              /**< Current index of the Iid sequence. */
    uint16_t wCurrentIidTablePos;                                           /**< Position of current entry in the Iid Table. */
    uint16_t wAdditionalInfo;                                               /**< Specific error codes for VC generic errors. */
    uint8_t  bSessionAuthMACKey[16];                                        /**< Session Keys for Authentication of MAC. */
    phalVca_VirtualCardState eVCState;                                      /**< Enum variable holding Virtual Card state. */
    phalVca_ProximityCheckState ePCState;                                   /**< Enum Variable holding Proximity Check state. */
    void * pAlDataParams;                                                   /**< Pointer to the parameter structure of the DesfireEVx / MFPEVx component. */
    uint8_t bWrappedMode;                                                   /**< Wrapped APDU mode. All native commands need to be sent wrapped in ISO 7816 APDUs. */
    uint8_t bExtendedLenApdu;                                               /**< Extended length APDU. If set the native commands should be wrapped in extended format */
    uint8_t bOption;                                                        /**< bOption parameter for Timing measurements ON/OFF. */
    uint8_t bLowerBoundThreshold;                                           /**< parameter for enable/disable the LowerBound threshold time calculation(-20%). */
} phalVca_Sw_DataParams_t;

/**
 * \brief Initialization API for AL VCA / PC to communicate with direct reader and PICC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phalVca_Sw_Init(
        phalVca_Sw_DataParams_t * pDataParams,                              /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wSizeOfDataParams,                                         /**< [In] Specifies the size of the data parameter structure. */
        void * pPalMifareDataParams,                                        /**< [In] Pointer to the parameter structure of the palMifare layer. */
        void * pKeyStoreDataParams,                                         /**< [In] Pointer to the parameter structure of the keyStore layer. */
        void * pCryptoDataParams,                                           /**< [In] Pointer to the parameter structure of the Crypto layer. */
        void * pCryptoRngDataParams,                                        /**< [In] Pointer to the parameter structure of the CryptoRng layer. */
        phalVca_Sw_IidTableEntry_t * pIidTableStorage,                      /**< [In] Pointer to the Iid Table storage for the layer. */
        uint16_t wNumIidTableStorageEntries,                                /**< [In] Number of possible Iid table entries in the storage. */
        phalVca_Sw_CardTableEntry_t * pCardTableStorage,                    /**< [In] Pointer to the Card Table storage for the layer. */
        uint16_t wNumCardTableStorageEntries                                /**< [In] Number of possible Card table entries in the storage. */
    );

/**
 * end of group phalVca_Sw
 * @}
 */
#endif /* NXPBUILD__PHAL_VCA_SW */



#ifdef NXPBUILD__PHAL_VCA

/** \defgroup phalVca Virtual Card Architecture (R)
 * \brief These Components implement the Virtual Card Architecture (R) commands.
 * @{
 */

/** \name Custom Error Codes */
/* @{ */
#define PHAL_VCA_ERR_CMD_INVALID                (PH_ERR_CUSTOM_BEGIN + 0U)  /**< VCA Invalid Command Error. */
#define PHAL_VCA_ERR_FORMAT                     (PH_ERR_CUSTOM_BEGIN + 1U)  /**< VCA Format Error. */
#define PHAL_VCA_ERR_AUTH                       (PH_ERR_CUSTOM_BEGIN + 2U)  /**< VCA MAC Error. */
#define PHAL_VCA_ERR_GEN                        (PH_ERR_CUSTOM_BEGIN + 3U)  /**< VCA GEN Error. */
#define PHAL_VCA_ERR_CMD_OVERFLOW               (PH_ERR_CUSTOM_BEGIN + 4U)  /**< VCA CMD Overflow Error. */
#define PHAL_VCA_ERR_COMMAND_ABORTED            (PH_ERR_CUSTOM_BEGIN + 5U)  /**< VCA Command Abort Error. */
#define PHAL_VCA_ERR_7816_GEN_ERROR             (PH_ERR_CUSTOM_BEGIN + 6U)  /**< VCA ISO 7816 Generic error. Check Additional Info. */
/* @} */

/**
 * \defgroup phalVca_VC Virtual Card commands
 * \brief These Components implement the Virtual Card Architecture (R) commands for Virtual Card feature.
 * @{
 */

/**
 * \brief Start Card Selection.
 * This command is used to start a VCS / VCSL sequence.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_StartCardSelection(
    void * pDataParams                                                      /**< [In] Pointer to this layer's parameter structure. */
    );

/**
 * \brief Finalize Card Selection.
 *
 * This command is used to finalize a VCS/VCSL sequence. The number of sucessfully detected IIDs is returned
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_FinalizeCardSelection(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint16_t * pNumValidIids                                            /**< [Out] Number of valid IIDs detected during VCS/VCSL sequence execution. */
    );

#ifdef NXPBUILD__PH_CRYPTOSYM
/**
 * \brief Performs a Select Virtual Card command (MIFARE Plus only).
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_SelectVc(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wValidIidIndex,                                            /**< [In] Index of valid IID, needs to be < pNumValidIids in FinalizeCardSelection call. */
        uint16_t wKeyNumber,                                                /**< [In] Key number of the MAC key used in SVC command. */
        uint16_t wKeyVersion                                                /**< [In] Key version of the MAC key used in SVC command. */
    );
#endif /* NXPBUILD__PH_CRYPTOSYM */

/**
 * \brief Performs a Deselect Virtual Card command.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_DeselectVc(
        void * pDataParams                                                  /**< [In] Pointer to this layer's parameter structure. */
    );

/**
 * \brief Performs a Virtual Card Support command (MIFARE Plus only).
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_VcSupport(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pIid,                                                     /**< [In] Installation Identifier (16 bytes). */
        uint16_t wKeyEncNumber,                                             /**< [In] Key number of the ENC key associated to the Iid. */
        uint16_t wKeyEncVersion,                                            /**< [In] Key version of the ENC key associated to the Iid. */
        uint16_t wKeyMacNumber,                                             /**< [In] Key number of the MAC key associated to the Iid. */
        uint16_t wKeyMacVersion                                             /**< [In] Key version of the MAC key associated to the Iid. */
    );

#ifdef NXPBUILD__PH_CRYPTOSYM
/**
 * \brief Perform a Virtual Card Support (Last) command. This command performs the Virtual card support command in
 * ISO14443 Layer 3 activated state. This comand is supported by MIFARE Ultralight EV1 and Mifare Plus product only.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_VcSupportLast(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pIid,                                                     /**< [In] Installation Identifier (16 bytes). */
        uint8_t bLenCap,                                                    /**< [In] Length of the PCDcaps (0-6). */
        uint8_t * pPcdCapabilities,                                         /**< [In] PCD Capabilities (bLenCap bytes), ignored if bLenCap == 0. */
        uint16_t wKeyEncNumber,                                             /**< [In] Key number of the ENC key used in VCSL command. */
        uint16_t wKeyEncVersion,                                            /**< [In] Key version of the ENC key used in VCSL command. */
        uint16_t wKeyMacNumber,                                             /**< [In] Key number of the MAC key used in VCSL command. */
        uint16_t wKeyMacVersion                                             /**< [In] Key version of the MAC key used in VCSL command. */
    );
#endif /* NXPBUILD__PH_CRYPTOSYM */

/**
 * \brief Retrieve card information.
 * This command retrieves card information like UID and Info byte and the associated IID
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_GetIidInfo(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wValidIidIndex,                                            /**< [In] Index of valid IID, needs to be < pNumValidIids in FinalizeCardSelection call. */
        uint16_t * pIidIndex,                                               /**< [Out] Corresponding IID to the key pair where the MAC was matching in VCSL command call. */
        uint8_t * pVcUidSize,                                               /**< [Out] Size of the VC UID (4 or 7 bytes). */
        uint8_t * pVcUid,                                                   /**< [Out] VC UID (pVcUidSize bytes). */
        uint8_t * pInfo,                                                    /**< [Out] Info byte. */
        uint8_t * pPdCapabilities                                           /**< [Out] PD Capabilities (2 bytes). */
    );

#ifdef NXPBUILD__PH_CRYPTOSYM

/** \name Option to be used for ISOSelect command when called for SAM X or S configuration. */
/** @{ */
#define PHAL_VCA_ISO_SELECT_SELECTION_DF_NAME                       0x04    /**< Option mask to indicate the selection control as DFName. */
#define PHAL_VCA_ISO_SELECT_FCI_RETURNED                            0x00    /**< Option mask to indicate the return of FCI information. */

#define PHAL_VCA_ISO_SELECT_VARIANT_PART1                           0x00    /**< Option mask for complete VC selection in 1 part. */
#define PHAL_VCA_ISO_SELECT_VARIANT_PART2                           0x08    /**< Option mask for complete VC selection in 2 part. */
#define PHAL_VCA_ISO_SELECT_DIV_DISABLED                            0x00    /**< Default option mask to disable the diversification of VcSelect MAC or ENC key. */
#define PHAL_VCA_ISO_SELECT_ENC_KEY_DIV_INPUT                       0x01    /**< Option mask to perform diversification of VC SelectENC key using the diversification input provided. */
#define PHAL_VCA_ISO_SELECT_MAC_KEY_DIV_INPUT                       0x02    /**< Option mask to perform diversification of VC SelectMAC key using the diversification input provided. */
#define PHAL_VCA_ISO_SELECT_MAC_KEY_DIV_VCUID                       0x04    /**< Option mask to perform diversification of VC SelectMAC key using Virtual Card Identifier. */
#define PHAL_VCA_ISO_SELECT_MAC_KEY_DIV_INPUT_VCUID                 0x06    /**< Option mask to perform diversification of VC SelectMAC key using Virtual Card Identifier
                                                                             *   and Diversification input provided.
                                                                             */
/** @} */

/**
 * \brief PCD explicitly indicates which Virtual Card it wants to target by issuing this command.
 * Both PCD and the PD agree on which VC to use (if any) and they negotiate the capabilities that
 * they will use. This command is supported by DESFire EV1, DESFire EV2 and Plus EV1 only.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS on Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_IsoSelect(
        void *    pDataParams,                                              /**< [In] Pointer to this layer's parameter structure which is of type phalVca_Sw_DataParams_t. */
        uint8_t   bSelectionControl,                                        /**< [In] Value equals #PHAL_VCA_ISO_SELECT_SELECTION_DF_NAME which indicates VC Selection by DF name.
                                                                             *        Any other value results in PH_ERR_INVALID_PARAMETER.\n
                                                                             *
                                                                             *        Note: This parameter will be used for Software and Sam S mode only.
                                                                             *              For Sam X mode it can be neglected.
                                                                             */
        uint8_t   bOption,                                                  /**< [In] Value equals #PHAL_VCA_ISO_SELECT_FCI_RETURNED which indicates FCI template is returned.
                                                                             *        Any other value results in PH_ERR_INVALID_PARAMETER.\n
                                                                             *        Note: This value will be used for Software mode only.\n\n
                                                                             *
                                                                             *         Sam S or X communication, The macros can be combined by oring.
                                                                             *          \arg #PHAL_VCA_ISO_SELECT_DIV_DISABLED
                                                                             *          \arg #PHAL_VCA_ISO_SELECT_ENC_KEY_DIV_INPUT
                                                                             *          \arg #PHAL_VCA_ISO_SELECT_MAC_KEY_DIV_INPUT
                                                                             *          \arg #PHAL_VCA_ISO_SELECT_MAC_KEY_DIV_VCUID
                                                                             *          \arg #PHAL_VCA_ISO_SELECT_MAC_KEY_DIV_INPUT_VCUID
                                                                             *        \n
                                                                             *        The below onse are valid for Sam X only. To be used with the bove ones.
                                                                             *          \arg #PHAL_VCA_ISO_SELECT_VARIANT_PART1
                                                                             *          \arg #PHAL_VCA_ISO_SELECT_VARIANT_PART2
                                                                             */
        uint8_t   bDFnameLen,                                               /**< [In] Length of the IID which is nothing but DFName string provided by the user. */
        uint8_t*  pDFname,                                                  /**< [In] This is the IID which is DFName string upto 16 Bytes. Valid only when bSelectionControl = 0x04. */
        uint8_t*  pDivInput,                                                /**< [IN] Key diversification input. This is used for SAM Key diversification */
        uint8_t   bDivInputLen,                                             /**< [IN] Key diversification input Length. */
        uint8_t   bEncKeyNo,                                                /**< [IN] ENC Key number in Software or SAM keystore. */
        uint8_t   bEncKeyVer,                                               /**< [IN] ENC Key version in Software or SAM keystore. */
        uint8_t   bMacKeyNo,                                                /**< [IN] MAC Key number in Software or SAM keystore. */
        uint8_t   bMacKeyVer,                                               /**< [IN] MAC Key version in Software or SAM keystore. */
        uint8_t*  pResponse,                                                /**< [Out] Response as mentioned below. \n
                                                                             *          \c Software: Response received from PICC. \n
                                                                             *          \c SAM S   : 8 bytes MAC which should be used for ISOExternalAuthenticate +
                                                                             *                       Decrypted Virtual Card Data. (or) FileIdentifier
                                                                             *          \c SAM X   : Virtual Card Data or FCI.
                                                                             */
        uint16_t* pRespLen                                                  /**< [Out] Response received from the PICC. */
    );

/**
 * \brief Depending on the VC configuration, the PCD might be required to subsequently (after IsoSelect execution)
 * authenticate itself via this command before the targeted VC becomes selected.
 * This command is supported by DESFire EV1, DESFire EV2 and Plus EV1 only.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS on Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_IsoExternalAuthenticate(
        void *    pDataParams,                                              /**< [In] Pointer to this layer's parameter structure which is of type phalVca_Sw_DataParams_t. */
        uint8_t*  pInData,                                                  /**< [In] Input buffer to be passed for authentication. \n
                                                                             *          \c Software: Response data (RndChl||VCData) of IsoSelect command. \n
                                                                             *          \c SAM S   : Response data (MAC + Decrypted VCData) of IsoSelect command. \n
                                                                             *          \c SAM X   : Not Applicable. This command is not supported for X mode.
                                                                             */
        uint16_t  wKeyNumber,                                               /**< [In] Key number of the VCSelect MAC key. Only applicable for Software. */
        uint16_t  wKeyVersion                                               /**< [In] Key version of the VCSelect MAC key. Only applicable for Software. */
    );
#endif /* NXPBUILD__PH_CRYPTOSYM */

/**
 * end of group phalVca_VC
 * @}
 */




/**
 * \defgroup phalVca_PC Proximity Check commands
 * \brief These Components implement the Virtual Card Architecture (R) commands for Proximity Check feature.
 * @{
 */

#ifdef NXPBUILD__PH_CRYPTOSYM
/**
 * \brief Performs the whole Proximity Check command chain which calls all the three commands(PreparePC, ExecutePC and VerifyPC) together.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_ProximityCheck(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t bGenerateRndC,                                              /**< [In] 0: RndC provided; 1: generate RndC; */
        uint8_t * pRndC,                                                    /**< [In] Provided RndC (7 bytes), ignored if bGenerateRndC == 1. */
        uint8_t bPps1,                                                      /**< [In] Speed(PPS1). This indicates communication data rate between PD and PCD. */
        uint8_t bNumSteps,                                                  /**< [In] Number of ProximityCheck cycles; RFU, must be 1. */
        uint16_t wKeyNumber,                                                /**< [In] Proximity Check MAC Key number. */
        uint16_t wKeyVersion,                                               /**< [In] Proximity Check MAC Key version. */
        uint8_t * pUsedRndC                                                 /**< [Out] Used RndC (7 bytes), can be NULL. */
    );

/** \name Option to be used for ProximityCheckNew command. */
/** @{ */
#define PHAL_VCA_PC_RNDC_PROVIDED                                   0x00    /**< Option mask to indicate the Random number C is given by the user. */
#define PHAL_VCA_PC_RNDC_GENERATE                                   0x01    /**< Option mask to indicate the Random number C to be generated by the library. */

#define PHAL_VCA_PC_NORMAL_PROCESSING                               0x00    /**< Option mask to indicate VerifyProcessing should be Normal. */
#define PHAL_VCA_PC_RANDOM_PROCESSING                               0x02    /**< Option mask to indicate VerifyProcessing should be Random. */
/** @} */

/**
 * \brief Performs the whole Proximity Check New command chain which calls all the three commands(PreparePC, PC and VerifyPC) together.
 * This command is supported by Desfire EV2, future and Mifare Plus Ev1, future product only.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_ProximityCheckNew(
        void    * pDataParams,                                              /**< [In] Pointer to this layer's parameter structure. */
        uint8_t   bOption,                                                  /**< [In] One of the below Option values. \n
                                                                             *          For Software and SAM S mode.
                                                                             *              \arg #PHAL_VCA_PC_RNDC_PROVIDED
                                                                             *              \arg #PHAL_VCA_PC_RNDC_GENERATE \n
                                                                             *
                                                                             *          For Sam X mode.
                                                                             *              \arg #PHAL_VCA_PC_NORMAL_PROCESSING
                                                                             *              \arg #PHAL_VCA_PC_RANDOM_PROCESSING
                                                                             */
        uint8_t * pPrndC,                                                   /**< [In] One of the below contents.\n
                                                                             *          \c For Software mode and SAM S, if #PHAL_VCA_PC_RNDC_GENERATE the buffer can be null
                                                                             *             else the random number should be provided by the user.
                                                                             *
                                                                             *          \c For Sam X mode, the buffer will not be utilized as the random number
                                                                             *             will be generated by SAM.
                                                                             */
        uint8_t   bNumSteps,                                                /**< [In] Number of ProximityCheck cycles. The value should be from 1 - 8. */
        uint16_t  wKeyNumber,                                               /**< [In] Proximity Check MAC Key number in Software or SAM keystore. */
        uint16_t  wKeyVersion,                                              /**< [In] Proximity Check MAC Key version in Software or SAM keystore. */
        uint8_t *pDivInput,                                                 /**< [In] Diversification input in case of SAM. */
        uint8_t bDivInputLen,                                               /**< [In] Diversification input length in case of SAM. */
        uint8_t * pOption,                                                  /**< [Out] Option field defining subsequent response content \n
                                                                             *          \c bit[0] of bOption denotes presence of PPS \n
                                                                             *          \c bit[1] of bOption denotes presence of ActBitRate
                                                                             */
        uint8_t * pPubRespTime,                                             /**< [Out] Published response time: time in microseconds. The PD will transmit the
                                                                            *          Cmd.ProximityCheck response as close as possible to this time.
                                                                            */
        uint8_t * pResponse,                                                /**< [Out] The response received by the PICC. Can be \n
                                                                             *              \c Speed(PPS1). This indicates communication data rate between PD and PCD. OR \n
                                                                             *              \c ActBitRate information.
                                                                             */
        uint16_t * pRespLen,                                                /**< [Out] Length of byte(s) available in Response buffer.
                                                                             *              \c One byte Speed(PPS1) OR \n
                                                                             *              \c N bytes of ActBitRate information.
                                                                             */
        uint8_t * pCumRndRC                                                 /**< [Out] Combination of both pRndCmd(cumulative of 8 bytes of Random data sent from PCD) and
                                                                             *         pRndResp(cumulative of 8 bytes response data received from PD).
                                                                             */
    );

/**
 * \brief "PrepareProximityCheckNew" command shall prepare the card by generating the Option, publish response time and optional PPS.
 * This command is supported by Desfire EV2, future and Mifare Plus Ev1, future product only.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_PrepareProximityCheckNew(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pOption,                                                  /**< [Out] Option field defining subsequent response content \n
                                                                             *          \c bit[0] of bOption denotes presence of PPS \n
                                                                             *          \c bit[1] of bOption denotes presence of ActBitRate
                                                                             */
        uint8_t * pPubRespTime,                                             /**< [Out] Published response time: time in microseconds. The PD will transmit the Cmd.ProximityCheck response as close as possible to this time. */
        uint8_t * pResponse,                                                /**< [Out] The response received by the PICC. Can be \n
                                                                             *              \c Speed(PPS1). This indicates communication data rate between PD and PCD. OR \n
                                                                             *              \c ActBitRate information.
                                                                             */
        uint16_t * pRespLen                                                 /**< [Out] Length of byte(s) available in Response buffer.
                                                                             *              \c One byte Speed(PPS1) OR \n
                                                                             *              \c N bytes of ActBitRate information.
                                                                             */
    );

/**
 * \brief This new command is implementated to support VCA for Desfire Ev2, future and Mifare Plus EV1, future. Proximity Device
 * answers with a prepared random number at the published response time in Command "PrepareProximityCheckNew". So
 * "PrepareProximityCheckNew" command may be repeated up to 8 times splitting the random number for different
 * time measurements.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_ExecuteProximityCheckNew(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t   bGenerateRndC,                                            /**< [In] One of the below Option values.
                                                                             *              \arg #PHAL_VCA_PC_RNDC_PROVIDED
                                                                             *              \arg #PHAL_VCA_PC_RNDC_GENERATE
                                                                             */
        uint8_t * pPrndC,                                                   /**< [In] Provided RndC (8 bytes), ignored if bGenerateRndC == #PHAL_VCA_PC_RNDC_GENERATE. */
        uint8_t   bNumSteps,                                                /**< [In] Number of ProximityCheck cycles. Will be one of the below sequence. \n
                                                                             *          \c Steps = 8: Only one iteration is made, All 8 random bytes in one Cmd.ProxmityCheck. \n
                                                                             *          \c Steps = 7: Sends the first 7 random bytes in one Cmd.ProxmityCheck and the remaining random
                                                                             *                        byte in another one. \n
                                                                             *          \c Steps = 6: 6 in one Cmd.ProxmityCheck and remaining 2 in Second. \n
                                                                             *          \c Steps = 5: 5 in one Cmd.ProxmityCheck and remaining 3 in Second. \n
                                                                             *          \c Steps = 4: 4 in one Cmd.ProxmityCheck and remaining 4 in Second. \n
                                                                             *          \c Steps = 3: 3 in one Cmd.ProxmityCheck, 3 in Second and remaining 2 in Third. \n
                                                                             *          \c Steps = 2: 2 in one Cmd.ProxmityCheck, 2 in Second, 2 in Third and remaining 2 in fourth. \n
                                                                             *          \c Steps = 1: Sends 8 Cmd.ProxmityCheck with one random byte for each Exchange.
                                                                             */
        uint8_t * pPubRespTime,                                             /**< [In] Published response time: time in microseconds. The PD will transmit the
                                                                             *        Cmd.ProximityCheck response as close as possible to this time.
                                                                             */
        uint8_t * pCumRndRC                                                 /**< [Out] Combination of both pRndCmd(cumulative of 8 bytes of Random data sent from PCD)
                                                                             *         and pRndResp(cumulative of 8 bytes response data received from PD).
                                                                             */
    );

/**
 * \brief "VerifyProximityCheckNew" command verifies all random numbers received after execution of command "ExecuteProximityCheckNew" using cryptographic methods.
 * This command is supported by Desfire EV2, future and Mifare Plus Ev1, future product only.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 *  \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_VerifyProximityCheckNew(
        void *    pDataParams,                                              /**< [In] Pointer to this layer's parameter structure. */
        uint8_t   bOption,                                                  /**< [In] Option field defining subsequent response content \n
                                                                             *          \c bit[0] of bOption denotes presence of PPS \n
                                                                             *          \c bit[1] of bOption denotes presence of ActBitRate
                                                                             */
        uint8_t * pPubRespTime,                                             /**< [In] Published response time: time in microseconds. The PD will transmit the Cmd.ProximityCheck response as close as possible to this time. */
        uint8_t * pResponse,                                                /**< [In] The response received by the PICC. Can be \n
                                                                             *              \c Speed(PPS1). This indicates communication data rate between PD and PCD. OR \n
                                                                             *              \c ActBitRate information.
                                                                             */
        uint16_t wRespLen,                                                  /**< [In] Length of bytes available in Response buffer. */
        uint16_t  wKeyNumber,                                               /**< [In] Proximity Check MAC Key number. */
        uint16_t  wKeyVersion,                                              /**< [In] Proximity Check MAC Key version. */
        uint8_t * pRndCmdResp                                               /**< [Out] Combination of both pRndCmd(cumulative of 8 bytes of Random data sent from PCD)
                                                                             *         and pRndResp(cumulative of 8 bytes response data received from PD).
                                                                             */
    );
#endif /* NXPBUILD__PH_CRYPTOSYM */

/**
 * \brief It does all the operations as is which the above API "phalVca_VerifyProximityCheckNew" does. It is specially designed to fulfil the requirements for REC.
 * It directly sends the MAC and receive it without any further operations on the sent MAC and the received MAC.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_VerifyProximityCheckUtility(
        void *    pDataParams,                                              /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pCmdMac,                                                  /**< [In] 8 bytes Command MAC, to be sent directly to the PICC */
        uint8_t * pCmdResp                                                  /**< [Out] 8 bytes Response MAC received from PICC */
    );
/**
 * end of group phalVca_PC
 * @}
 */




/**
 * \defgroup phalVca_Utilities Utility
 * \brief These Components implement the Virtual Card Architecture (R) commands.
 * @{
 */

/** \name Option to configure some special operations */
/** @{ */
#define PHAL_VCA_ADDITIONAL_INFO                                    0x0001  /**< Option for GetConfig / SetConfig to get / set additional info of a generic error. */
#define PHAL_VCA_WRAPPED_MODE                                       0x0002  /**< Option for GetConfig / SetConfig to get / set Wrapped mode. */
#define PHAL_VCA_TIMING_MODE                                        0x0003  /**< Option for GetConfig / SetConfig to get / set Timing measurements ON/OFF. */
#define PHAL_VCA_PC_LOWER_THRESHOLD                                 0x0004  /**< Option for GetConfig / SetConfig to Enable / Diable lower boundary(-20%) on threshold time. */
#define PHAL_VCA_PC_EXTENDED_APDU                                   0x0005  /**< Option for GetConfig / SetConfig to get / set current status of extended wrapping in ISO 7816-4 APDUs. */
/** @} */

/**
 * \brief Perform a SetConfig command where a particular configuration item vis. Error code can be set in VC structure.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phalVca_SetConfig(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                   /**< [In] wConfig can be one of the following.
                                                                             *          \arg #PHAL_VCA_ADDITIONAL_INFO
                                                                             *          \arg #PHAL_VCA_WRAPPED_MODE
                                                                             *          \arg #PHAL_VCA_TIMING_MODE
                                                                             *          \arg #PHAL_VCA_PC_LOWER_THRESHOLD
                                                                             *          \arg #PHAL_VCA_PC_EXTENDED_APDU
                                                                             */
        uint16_t wValue                                                     /**< [In] Value for the mentioned configuration. */
    );

/**
 * \brief Perform a GetConfig command where a particular configuration item vis. Error code can be retreived from VC structure.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phalVca_GetConfig(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                   /**< [In] wConfig can be one of the following.
                                                                             *          \arg #PHAL_VCA_ADDITIONAL_INFO
                                                                             *          \arg #PHAL_VCA_WRAPPED_MODE
                                                                             *          \arg #PHAL_VCA_TIMING_MODE
                                                                             *          \arg #PHAL_VCA_PC_LOWER_THRESHOLD
                                                                             *          \arg #PHAL_VCA_PC_EXTENDED_APDU
                                                                             */
        uint16_t * pValue                                                   /**< [Out] Value for the mentioned configuration. */
    );

/**
 * \brief This is a utility API which passes card type specific session key to VCA
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_SetSessionKeyUtility(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pSessionKey,                                              /**< [In] 16 bytes Session Key */
        uint8_t bAuthMode                                                   /**< [In] Current Authentication Mode (Card type specific) */
    );

/**
 * \brief This is a utility API which sets the application type(Desfire Ev2, MFP Ev1 etc) in the VCA structure
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_SetApplicationType(
        void * pDataParams,
        void * pAlDataParams
        );

#ifdef NXPBUILD__PH_CRYPTOSYM
/**
 * \brief This is a utility API which decrypts the response data(32 bytes) of IsoSelect
 * to get the RndChal and VCData(INFO||PDCap1||VCUID||ZeroPadding)
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phalVca_DecryptResponse(
        void * pDataParams,                                                 /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                    /**< [In] Key number of the VCSelect ENC key. */
        uint16_t wKeyVersion,                                               /**< [In] Key version of the VCSelect ENC key. */
        uint8_t * pInData,                                                  /**< [In] Input data to be decrypted. */
        uint8_t * pRandChal,                                                /**< [Out] Random Challenge. */
        uint8_t * pVCData                                                   /**< [Out] Decrypted VC Data. */
    );
#endif /* NXPBUILD__PH_CRYPTOSYM */

/**
 * end of group phalVca_Utilities
 * @}
 */

/**
 * end of group phalVca
 * @}
 */

#endif /* NXPBUILD__PHAL_VCA */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHALVCA_H */
