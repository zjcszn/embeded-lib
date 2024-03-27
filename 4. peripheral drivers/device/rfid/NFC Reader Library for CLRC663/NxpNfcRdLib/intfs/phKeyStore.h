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
 * Generic KeyStore Component of Reader Library Framework.
 * $Author: Rajendran Kumar (nxp99556) $
 * $Revision: 6671 $ (v07.09.00)
 * $Date: 2022-08-07 19:39:47 +0530 (Sun, 07 Aug 2022) $
 *
 * History:
 *  CHu: Generated 19. May 2009
 *
 */

#ifndef PHKEYSTORE_H
#define PHKEYSTORE_H

#include <ph_Status.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifdef NXPBUILD__PH_KEYSTORE_SW

/**
 * \defgroup phKeyStore_Sw Component : Software
 * \brief KeyStore implementation in Software.
 * @{
 */

#define PH_KEYSTORE_SW_ID                                       0x01U   /**< ID for Software KeyStore component. */


#ifndef NXPBUILD__PH_KEYSTORE_ASYM
#define PH_KEYSTORE_SW_MAX_KEY_SIZE                             32U     /**< Maximum size of a Symmetric Key. */
#endif /* NXPBUILD__PH_KEYSTORE_ASYM */

#define PH_KEYSTORE_MAX_KEY_SIZE                                PH_KEYSTORE_SW_MAX_KEY_SIZE

/** \brief Software KeyVersionPair structure for Symmetric and ASymmetric keys. */
typedef struct
{
    uint8_t pKey[PH_KEYSTORE_MAX_KEY_SIZE];                             /**< Array containing a Symmetric Key or ASymmetric Private Key. */
    uint16_t wVersion;                                                  /**< Versions related to the Symmetric Key. Not applicable for ASymmetric keys storage. */

} phKeyStore_Sw_KeyVersionPair_t;

/** \brief Software KeyEntry structure. */
typedef struct
{
    uint16_t wKeyType;                                                  /**< Type of the keys in \ref phKeyStore_Sw_KeyVersionPair_t. */
    uint16_t wRefNoKUC;                                                 /**< Key usage counter number of the keys in pKeys. */
} phKeyStore_Sw_KeyEntry_t;

/** \brief Software KeyUsageCounter structure. */
typedef struct
{
    uint32_t dwLimit;                                                   /**< Limit of the Key Usage Counter. */
    uint32_t dwCurVal;                                                  /**< Current Value of the KUC. */
} phKeyStore_Sw_KUCEntry_t;

/** \brief Software parameter structure. */
typedef struct
{
    uint16_t  wId;                                                      /**< Layer ID for this component, NEVER MODIFY! */
    phKeyStore_Sw_KeyEntry_t * pKeyEntries;                             /**< Key entry storage, size = sizeof(#phKeyStore_Sw_KeyEntry_t) * wNumKeyEntries. */
    phKeyStore_Sw_KeyVersionPair_t * pKeyVersionPairs;                  /**< Key version pairs, size = sizeof(#phKeyStore_Sw_KeyVersionPair_t)* wNumKeyEntries * wNumVersions. */
    uint16_t wNoOfKeyEntries;                                           /**< Number of key entries in that storage. */
    uint16_t wNoOfVersions;                                             /**< Number of versions in each key entry. */
    phKeyStore_Sw_KUCEntry_t * pKUCEntries;                             /**< Key usage counter entry storage, size = sizeof(#phKeyStore_Sw_KUCEntry_t) * wNumKUCEntries. */
    uint16_t wNoOfKUCEntries;                                           /**< Number of Key usage counter entries. */
} phKeyStore_Sw_DataParams_t;

/**
 * \brief Initializes the KeyStore component as software component.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phKeyStore_Sw_Init(
        phKeyStore_Sw_DataParams_t * pDataParams,                       /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wSizeOfDataParams,                                     /**< [In] Specifies the size of the data parameter structure. */
        phKeyStore_Sw_KeyEntry_t * pKeyEntries,                         /**< [In] Pointer to a storage containing the key entries. */
        uint16_t wNoOfKeyEntries,                                       /**< [In] Size of pKeyEntries. */
        phKeyStore_Sw_KeyVersionPair_t * pKeyVersionPairs,              /**< [In] Pointer to a storage containing the key version pairs. */
        uint16_t wNoOfVersionPairs,                                     /**< [In] Amount of key versions available in each key entry. */
        phKeyStore_Sw_KUCEntry_t * pKUCEntries,                         /**< [In] Key usage counter entry storage, size = sizeof(phKeyStore_Sw_KUCEntry_t) * wNumKUCEntries */
        uint16_t wNoOfKUCEntries                                        /**< [In] Number of Key usage counter entries. */
    );

/**
 * end of group phKeyStore_Sw
 * @}
 */
#endif /* NXPBUILD__PH_KEYSTORE_SW */

#ifdef NXPBUILD__PH_KEYSTORE_RC663

#include <phhalHw.h>

/**
 * \defgroup phKeyStore_Rc663 Component : Rc663
 * @{
 */
#define PH_KEYSTORE_RC663_ID                                    0x02U   /**< ID for Rc663 KeyStore component. */
#define PH_KEYSTORE_RC663_NUM_KEYS                              0x80U   /**< Maximum number of keys storable in Rc663. */
#define PH_KEYSTORE_RC663_NUM_VERSIONS                          0x01U   /**< Amount of versions for each key entry in the key store. */

/** \brief Rc663 parameter structure. */
typedef struct
{
    uint16_t  wId;                                                      /**< Layer ID for this component, NEVER MODIFY! */
    void * pHalDataParams;                                              /**< Pointer to the parameter structure of the underlying layer. */
} phKeyStore_Rc663_DataParams_t;

/**
 * \brief Initializes the KeyStore component as RC663 component.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phKeyStore_Rc663_Init(
        phKeyStore_Rc663_DataParams_t * pDataParams,                    /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wSizeOfDataParams,                                     /**< [In] Specifies the size of the data parameter structure. */
        void * pHalDataParams                                           /**< [In] Pointer to a HAL based on NXP RC663 IC. */
    );

/**
 * end of group phKeyStore_Rc663
 * @}
 */
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */





#ifdef NXPBUILD__PH_KEYSTORE

/**
 * \defgroup phKeyStore KeyStore
 *
 * \brief This is only a wrapper layer to abstract the different KeyStore implementations.
 * @{
 */

/**
 * \defgroup phKeyStore_Defines_Config Configuration
 * \brief Definitions for KeyStore layer configuration.
 * @{
 */
#define PH_KEYSTORE_CONFIG_SET_DEFAULT                                  0xFFFFU /**< Reset all bit of SET param. */

/**
 * end of group phKeyStore_Defines_Config
 * @}
 */

/**
 * \addtogroup phKeyStore_Sym
 * @{
 */

/**
 * \defgroup phKeyStore_Sym_Defines Common Definitions
 * \brief Definitions for Symmetric keys of KeyStore component.
 * @{
 */

/**
 * \defgroup phKeyStore_Sym_Defines_KeyType KeyType
 * \brief Definitions for Symmetric Key types.
 * @{
 */
#define PH_KEYSTORE_KEY_TYPE_AES128                                     0x00U   /**< AES 128 Key [16]. */
#define PH_KEYSTORE_KEY_TYPE_AES192                                     0x01U   /**< AES 192 Key [24]. */
#define PH_KEYSTORE_KEY_TYPE_AES256                                     0x02U   /**< AES 256 Key [32]. */
#define PH_KEYSTORE_KEY_TYPE_DES                                        0x03U   /**< DES Single Key [8 Bytes]. This is basically the 56-Bit DES key. */
#define PH_KEYSTORE_KEY_TYPE_2K3DES                                     0x04U   /**< 2 Key Triple Des [16 Bytes]. This is basically the 112-Bit DES key. */
#define PH_KEYSTORE_KEY_TYPE_3K3DES                                     0x05U   /**< 3 Key Triple Des [24 Bytes]. This is basically the 168-Bit DES key. */
#define PH_KEYSTORE_KEY_TYPE_MIFARE                                     0x06U   /**< MIFARE (R) Key. */
/**
 * end of group phKeyStore_Sym_Defines_KeyType
 * @}
 */

/**
 * \defgroup phKeyStore_Sym_Defines_Size Key Size
 * \brief Definitions for Symmetric Key sizes.
 * @{
 */
#define PH_KEYSTORE_KEY_TYPE_MIFARE_SIZE                                0x0CU   /**< Size of an MIFARE Key. */
#define PH_KEYSTORE_KEY_TYPE_AES128_SIZE                                0x10U   /**< Size of an AES128 Key. */
#define PH_KEYSTORE_KEY_TYPE_AES192_SIZE                                0x18U   /**< Size of an AES192 Key. */
#define PH_KEYSTORE_KEY_TYPE_AES256_SIZE                                0x20U   /**< Size of an AES256 Key. */
/**
 * end of group phKeyStore_Sym_Defines_Size
 * @}
 */

/**
 * end of group phKeyStore_Sym_Defines
 * @}
 */

/**
 * end of group phKeyStore_Sym
 * @}
 */

#define PH_KEYSTORE_INVALID_ID                                          0xFFFFU /**< ID used for various parameters as a invalid default **/
#define PH_KEYSTORE_DEFAULT_ID                                          0x0000U /**< ID used for various parameters as a default **/

#ifdef NXPRDLIB_REM_GEN_INTFS

#if defined(NXPBUILD__PH_KEYSTORE_SW)
#include "../comps/phKeyStore/src/Sw/phKeyStore_Sw.h"

#define phKeyStore_FormatKeyEntry(pDataParams,wKeyNo,wNewKeyType) \
        phKeyStore_Sw_FormatKeyEntry((phKeyStore_Sw_DataParams_t *)pDataParams,wKeyNo,wNewKeyType)

#define phKeyStore_SetKUC(pDataParams, wKeyNo, wRefNoKUC) \
        phKeyStore_Sw_SetKUC((phKeyStore_Sw_DataParams_t *)pDataParams, wKeyNo, wRefNoKUC)

#define phKeyStore_GetKUC(pDataParams, wRefNoKUC, pdwLimit, pdwCurVal) \
        phKeyStore_Sw_GetKUC((phKeyStore_Sw_DataParams_t *)pDataParams, wRefNoKUC, pdwLimit, pdwCurVal)

#define phKeyStore_ChangeKUC(pDataParams, wRefNoKUC, dwLimit) \
        phKeyStore_Sw_ChangeKUC((phKeyStore_Sw_DataParams_t *)pDataParams, wRefNoKUC, dwLimit)

#define phKeyStore_SetConfig(pDataParams,wConfig,wValue) \
        phKeyStore_Sw_SetConfig((phKeyStore_Sw_DataParams_t *)pDataParams,wConfig,wValue)

#define phKeyStore_SetConfigStr(pDataParams,wConfig,pBuffer,wBufferLength) \
        phKeyStore_Sw_SetConfigStr((phKeyStore_Sw_DataParams_t *)pDataParams,wConfig,pBuffer,wBufferLength)

#define phKeyStore_GetConfig(pDataParams,wConfig,pValue) \
        phKeyStore_Sw_GetConfig((phKeyStore_Sw_DataParams_t *)pDataParams,wConfig,pValue)

#define phKeyStore_GetConfigStr(pDataParams,wConfig,ppBuffer,pBufferLength) \
        phKeyStore_Sw_GetConfigStr((phKeyStore_Sw_DataParams_t *)pDataParams,wConfig,ppBuffer,pBufferLength)

#define phKeyStore_SetKey(pDataParams, wKeyNo, wKeyVersion, wKeyType, pNewKey, wNewKeyVersion) \
        phKeyStore_Sw_SetKey((phKeyStore_Sw_DataParams_t *)pDataParams, wKeyNo, wKeyVersion, wKeyType, pNewKey, wNewKeyVersion)

#define phKeyStore_SetKeyAtPos(pDataParams, wKeyNo, wPos, wKeyType, pNewKey, wNewKeyVersion) \
        phKeyStore_Sw_SetKeyAtPos((phKeyStore_Sw_DataParams_t *)pDataParams, wKeyNo, wPos, wKeyType, pNewKey, wNewKeyVersion)

#define phKeyStore_SetFullKeyEntry(pDataParams, wNoOfKeys, wKeyNo, wNewRefNoKUC, wNewKeyType, pNewKeys, pNewKeyVersionList) \
        phKeyStore_Sw_SetFullKeyEntry((phKeyStore_Sw_DataParams_t *)pDataParams, wNoOfKeys, wKeyNo, wNewRefNoKUC, wNewKeyType, pNewKeys, pNewKeyVersionList)

#define phKeyStore_GetKeyEntry(pDataParams, wKeyNo, wKeyVersionBufSize, wKeyVersion, wKeyVersionLength, pKeyType) \
        phKeyStore_Sw_GetKeyEntry((phKeyStore_Sw_DataParams_t *)pDataParams, wKeyNo, wKeyVersionBufSize, wKeyVersion, wKeyVersionLength, pKeyType)

#define  phKeyStore_GetKey(pDataParams, wKeyNo, wKeyVersion, bKeyBufSize, pKey, pKeyType) \
         phKeyStore_Sw_GetKey((phKeyStore_Sw_DataParams_t *)pDataParams, wKeyNo, wKeyVersion, bKeyBufSize, pKey, pKeyType)
#endif /* NXPBUILD__PH_KEYSTORE_SW */

#else /* NXPRDLIB_REM_GEN_INTFS */

/**
 * \brief Format a key entry to a new KeyType.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_FormatKeyEntry(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                        /**< [In] KeyEntry number to be Formatted. */
        uint16_t wNewKeyType                                                    /**< [In] New Key type of the KeyEntry (predefined type of KeyType).
                                                                                 *           - \ref phKeyStore_Sym_Defines_KeyType "Symmetric KeyTypes"
                                                                                 *           \cond NXPBUILD__PH_KEYSTORE_ASYM
                                                                                 *           - \ref phKeyStore_ASym_Defines_KeyType "ASymmetric KeyTypes"
                                                                                 *           \endcond
                                                                                 */
    );

/**
 * \brief Change the KUC of a key entry.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_SetKUC(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                        /**< [In] KeyEntry number. */
        uint16_t wRefNoKUC                                                      /**< [In] Reference Number of the key usage counter used together with that key.*/
    );

/**
 * \brief Obtain a key usage counter entry.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_GetKUC(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wRefNoKUC,                                                     /**< [In] Number of the key usage counter to be looked at (00h to 0Fh) */
        uint32_t * pdwLimit,                                                    /**< [Out] Currently Set Limit in the KUC */
        uint32_t * pdwCurVal                                                    /**< [Out] Currently set value in the KUC */
    );

/**
 * \brief Change a key usage counter entry.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_ChangeKUC(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wRefNoKUC,                                                     /**< [In] Number of key entry. */
        uint32_t dwLimit                                                        /**< [In] Limit of the Key Usage Counter. */
    );

/**
 * \brief Set configuration parameter.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_SetConfig(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                       /**< [In] Configuration Identifier */
        uint16_t wValue                                                         /**< [In] Configuration Value */
    );

/**
 * \brief Set configuration parameter.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_SetConfigStr(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                       /**< [In] Configuration Identifier. */
        uint8_t *pBuffer,                                                       /**< [In] Buffer containing the configuration string. */
        uint16_t wBufferLength                                                  /**< [In] Length of configuration string. */
    );

/**
 * \brief Get configuration parameter.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_GetConfig(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                       /**< [In] Configuration Identifier */
        uint16_t * pValue                                                       /**< [Out] Configuration Value */
    );

/**
 * \brief Get configuration parameter.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_GetConfigStr(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                       /**< [In] Configuration Identifier */
        uint8_t ** ppBuffer,                                                    /**< [Out] Pointer to the buffer containing the configuration string. */
        uint16_t * pBufferLength                                                /**< [Out] Amount of valid bytes in the configuration string buffer. */
    );

/**
 * \defgroup phKeyStore_Sym Symmetric
 * \brief Interfaces for Symmetric keys of KeyStore component.
 * @{
 */

/**
 * \brief Change a symmetric key entry at a given version.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_SetKey(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                        /**< [In] Key number of the key to be loaded. Should be the Key index to
                                                                                 *        set the key to DataParams \b pKeyEntries buffer.
                                                                                 */
        uint16_t wKeyVersion,                                                   /**< [In] Key version of the key to be loaded. */
        uint16_t wKeyType,                                                      /**< [In] New Key type of the KeyEntry (predefined type of KeyType).
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_AES128
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_AES192
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_AES256
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_DES
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_2K3DES
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_3K3DES
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_MIFARE
                                                                                 */
        uint8_t * pNewKey,                                                      /**< [In] Pointer to the key itself. */
        uint16_t wNewKeyVersion                                                 /**< [In] New Key version of the key to be updated. */
    );

/**
 * \brief Change a symmetric key entry at the specified position.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_SetKeyAtPos(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                        /**< [In] Key number of the key to be loaded. Should be the Key index to
                                                                                 *        set the key to DataParams \b pKeyEntries member.
                                                                                 */
        uint16_t wPos,                                                          /**< [In] Key position to be updated.
                                                                                 *          - Should be the Key position to set the key to DataParams \b pKeyEntries buffer.
                                                                                 *          \cond NXPBUILD__PH_KEYSTORE_PN76XX
                                                                                 *          - Should be the following when used for PN76XX hardware.
                                                                                 *              - The Actual Symmetric Fixed Key Index (10h - 1Ah) of the hardware KeyStore.
                                                                                 *              - Will be Ignored below mentioned Keytypes.
                                                                                 *                  - \ref PH_KEYSTORE_KEY_TYPE_2K3DES "TripleDES - Two Key"
                                                                                 *                  - \ref PH_KEYSTORE_KEY_TYPE_3K3DES "TripleDES - Three Key"
                                                                                 *                  - \ref PH_KEYSTORE_KEY_TYPE_MIFARE "CRYPTO-1 (MIFARE) Key"
                                                                                 *          \endcond
                                                                                 */
        uint16_t wKeyType,                                                      /**< [In] Key type of the key to be loaded.*/
        uint8_t * pNewKey,                                                      /**< [In] Pointer to the key itself. */
        uint16_t wNewKeyVersion                                                 /**< [In] New Key version of the key to be updated. */
    );

/**
 * \brief Change a full symmetric key entry.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_SetFullKeyEntry(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wNoOfKeys,                                                     /**< [In] Number of keys in pNewKeys. */
        uint16_t wKeyNo,                                                        /**< [In] Key number of the key to be loaded. */
        uint16_t wNewRefNoKUC,                                                  /**< [In] Number of the key usage counter used together with that key. */
        uint16_t wNewKeyType,                                                   /**< [In] Key type of the key (if the current keyType of KeyEntry is different, error). */
        uint8_t * pNewKeys,                                                     /**< [In] Array of Keys to load. */
        uint16_t * pNewKeyVersionList                                           /**< [In] KeyVersionList of the key to be loaded. */
    );

/**
 * \brief Get a symmetric key entry information block.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_GetKeyEntry(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                        /**< [In] Key number of the key entry of interest. */
        uint16_t wKeyVersionBufSize,                                            /**< [In] Buffer Size of wKeyVersion in Bytes. */
        uint16_t * wKeyVersion,                                                 /**< [Out] Array for version information. */
        uint16_t * wKeyVersionLength,                                           /**< [Out] Length of valid data in wKeyVersion. */
        uint16_t * pKeyType                                                     /**< [Out] Type of the key. */
    );

/**
 * \brief Get a symmetric key.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phKeyStore_GetKey(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                        /**< [In] Key number of the key to be retrieved. */
        uint16_t wKeyVersion,                                                   /**< [In] Key version of the key to be retrieved. */
        uint8_t bKeyBufSize,                                                    /**< [In] Size of the key buffer. */
        uint8_t * pKey,                                                         /**< [Out] Pointer to the key itself. */
        uint16_t * pKeyType                                                     /**< [Out] Type of the key. */
    );

/**
 * end of group phKeyStore_Sym
 * @}
 */


#endif /* NXPRDLIB_REM_GEN_INTFS */

/**
 * \defgroup phKeyStore_Utility Utility
 * \brief Interfaces for utility interfaces for KeyStore component.
 * @{
 */
/**
 * \brief Gets the size of Symmetric key.
 *
 * \retval Symmetric Key Size.
 */
uint8_t phKeyStore_GetKeySize(
        uint16_t wKeyType                                                       /**< [In] Key type of the KeyEntry (predefined type of KeyType).
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_AES128
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_AES192
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_AES256
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_DES
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_2K3DES
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_3K3DES
                                                                                 *          - #PH_KEYSTORE_KEY_TYPE_MIFARE
                                                                                 */
    );

/**
 * end of group phKeyStore_Utility
 * @}
 */

/**
 * end of group phKeyStore
 * @}
 */
#endif /* NXPBUILD__PH_KEYSTORE */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHKEYSTORE_H */
