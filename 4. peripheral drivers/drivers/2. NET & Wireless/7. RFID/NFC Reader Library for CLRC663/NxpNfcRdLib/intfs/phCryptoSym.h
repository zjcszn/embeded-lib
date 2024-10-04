/*----------------------------------------------------------------------------*/
/* Copyright 2009 - 2020, 2022 NXP                                            */
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
* Generic Symmetric Cryptography Component of the Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6668 $ (v07.09.00)
* $Date: 2022-07-15 22:23:36 +0530 (Fri, 15 Jul 2022) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHCRYPTOSYM_H
#define PHCRYPTOSYM_H

#include <ph_Status.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifdef NXPBUILD__PH_CRYPTOSYM

/** \addtogroup phCryptoSym
 * @{
 */

/** \defgroup phCryptoSym_Defines Defines
 * \brief These are common definitions for most of the Crypto commands.
 * @{
 */

/** \defgroup phCryptoSym_Defines_KeyTypes KeyTypes
 * \brief Supported Key Types to be used in key loading functionality.
 * @{
 */
#define PH_CRYPTOSYM_KEY_TYPE_INVALID                                   0xFFFFU /**< Invalid Key */
#define PH_CRYPTOSYM_KEY_TYPE_AES128                                    0x0000U /**< AES 128 Key [16 Bytes]. */
#define PH_CRYPTOSYM_KEY_TYPE_AES192                                    0x0001U /**< AES 192 Key [24 Bytes]. */
#define PH_CRYPTOSYM_KEY_TYPE_AES256                                    0x0002U /**< AES 256 Key [32 Bytes]. */
#define PH_CRYPTOSYM_KEY_TYPE_DES                                       0x0003U /**< DES Single Key [8 Bytes]. This is basically the 56-Bit DES key. */
#define PH_CRYPTOSYM_KEY_TYPE_2K3DES                                    0x0004U /**< 2 Key Triple Des [16 Bytes]. This is basically the 112-Bit DES key. */
#define PH_CRYPTOSYM_KEY_TYPE_3K3DES                                    0x0005U /**< 3 Key Triple Des [24 Bytes]. This is basically the 168-Bit DES key. */
#define PH_CRYPTOSYM_KEY_TYPE_MIFARE                                    0x0006U /**< MIFARE (R) Key. */
/**
 * end of group phCryptoSym_Defines_KeyTypes
 * @}
 */

/** \defgroup phCryptoSym_Defines_KeySize KeySize
 * \brief Supported KeySizes for AES and DES algorithms.
 * @{
 */

/** \defgroup phCryptoSym_Defines_KeySize_DES DES
 * \brief Supported KeySizes DES algorithms.
 * @{
 */
#define PH_CRYPTOSYM_DES_BLOCK_SIZE                                         8U  /**< Block size in DES algorithm */
#define PH_CRYPTOSYM_DES_KEY_SIZE                                           8U  /**< Key size in DES algorithm for 56 bit key*/
#define PH_CRYPTOSYM_2K3DES_KEY_SIZE                                        16U /**< Key size in AES algorithm for 112 bit key*/
#define PH_CRYPTOSYM_3K3DES_KEY_SIZE                                        24U /**< Key size in AES algorithm for 168 bit key*/
/**
 * end of group phCryptoSym_Defines_KeySize_DES
 * @}
 */

/** \defgroup phCryptoSym_Defines_KeySize_AES AES
 * \brief Supported KeySizes AES algorithms.
 * @{
 */
#define PH_CRYPTOSYM_AES_BLOCK_SIZE                                         16U /**< Block size in AES algorithm */
#define PH_CRYPTOSYM_AES128_KEY_SIZE                                        16U /**< Key size in AES algorithm for 128 bit key*/
#define PH_CRYPTOSYM_AES192_KEY_SIZE                                        24U /**< Key size in AES algorithm for 192 bit key*/
#define PH_CRYPTOSYM_AES256_KEY_SIZE                                        32U /**< Key size in AES algorithm for 256 bit key*/
/**
 * end of group phCryptoSym_Defines_KeySize_AES
 * @}
 */

/**
 * end of group phCryptoSym_Defines_KeySize
 * @}
 */

/** \defgroup phCryptoSym_Defines_CipherModes Cipher Modes
 * \brief Supported Cipher Modes
 * @{
 */
#define PH_CRYPTOSYM_CIPHER_MODE_ECB                                    0x00U   /**< ECB Cipher Mode. */
#define PH_CRYPTOSYM_CIPHER_MODE_CBC                                    0x01U   /**< CBC Cipher Mode. */
#define PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4                                0x02U   /**< CBC Cipher Mode for D40 Secure Messaging. */
#define PH_CRYPTOSYM_CIPHER_MODE_LRP                                    0x03U   /**< LRP Cipher Mode. */
/**
 * end of group phCryptoSym_Defines_CipherModes
 * @}
 */

/** \defgroup phCryptoSym_Defines_MacModes MAC Modes
 * \brief Supported Mac Modes.
 * @{
 */
#define PH_CRYPTOSYM_MAC_MODE_CMAC                                      0x00U   /**< CMAC MAC Mode. */
#define PH_CRYPTOSYM_MAC_MODE_CBCMAC                                    0x01U   /**< CBCMAC MAC Mode. */
#define PH_CRYPTOSYM_MAC_MODE_LRP                                       0x02U   /**< LRP Mode. */
#define PH_CRYPTOSYM_MAC_MODE_LRP_SKM                                   0x03U   /**< Session Auth Master Key Generating Mode */
#define PH_CRYPTOSYM_MAC_MODE_LRP_GENPCDRESP                            0x04U   /**< Generating PCD Resp */
/**
 * end of group phCryptoSym_Defines_MacModes
 * @}
 */

/** \defgroup phCryptoSym_Defines_DivTypes Diversification Types
 * \brief Supported Diversification Types.
 * @{
 */
#define PH_CRYPTOSYM_DIV_MODE_MASK                                      0x00FFU /**< Bit-mask for diversification Mode. */
#define PH_CRYPTOSYM_DIV_MODE_DESFIRE                                   0x0000U /**< DESFire Key Diversification. */
#define PH_CRYPTOSYM_DIV_MODE_MIFARE_PLUS                               0x0001U /**< MIFARE Plus Key Diversification. */
#define PH_CRYPTOSYM_DIV_MODE_MIFARE_ULTRALIGHT                         0x0002U /**< MIFARE Ultralight Key Diversification. */
#define PH_CRYPTOSYM_DIV_OPTION_2K3DES_FULL                             0x0000U /**< Option for 2K3DES full-key diversification
                                                                                 * (only with \ref PH_CRYPTOSYM_DIV_MODE_DESFIRE "DESFire Key Diversification").
                                                                                 */
#define PH_CRYPTOSYM_DIV_OPTION_2K3DES_HALF                             0x8000U /**< Option for 2K3DES half-key diversification
                                                                                 * (only with \ref PH_CRYPTOSYM_DIV_MODE_DESFIRE "DESFire Key Diversification").
                                                                                 */
/**
 * end of group phCryptoSym_Defines_DivTypes
 * @}
 */

/** \defgroup phCryptoSym_Defines_PaddModes Padding Modes
 * \brief Supported Padding Modes.
 * @{
 */
#define PH_CRYPTOSYM_PADDING_MODE_1                                         00U /**< Pad with all zeros */
#define PH_CRYPTOSYM_PADDING_MODE_2                                         01U /**< Pad with a one followed by all zeros */
/**
 * end of group phCryptoSym_Defines_PaddModes
 * @}
 */

/** \defgroup phCryptoSym_Defines_Config Configuration
 * \brief CryptoSym Layer Configuration types.
 * @{
 */

/** \defgroup phCryptoSym_Defines_ConfigTypes Config Types
 * \brief CryptoSym Layer Configuration types.
 * @{
 */
#define PH_CRYPTOSYM_CONFIG_KEY_TYPE                                    0x0000U /**< Key Type. Read-only. Possible Values are:
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_INVALID
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES128
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES192
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES256
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_2K3DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_3K3DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_MIFARE
                                                                                 */
#define PH_CRYPTOSYM_CONFIG_KEY_SIZE                                    0x0001U /**< Key Size of currently loaded key. Read-only.  */
#define PH_CRYPTOSYM_CONFIG_BLOCK_SIZE                                  0x0002U /**< Block Size of currently loaded key. Read-only. */
#define PH_CRYPTOSYM_CONFIG_KEEP_IV                                     0x0003U /**< Keep init vector. Either #PH_CRYPTOSYM_VALUE_KEEP_IV_OFF or #PH_CRYPTOSYM_VALUE_KEEP_IV_ON.
                                                                                 *  This flag has to be used in combination with the option flag in the Encrypt/Decrypt/CalculateMac
                                                                                 *  function: If either the option in the function or this flag is set, the IV will be updated before
                                                                                 *  returning of the function. R/W access possible.
                                                                                 */
#define PH_CRYPTOSYM_CONFIG_LRP                                         0x0004U /**< Set the LRP mode on. */
#define PH_CRYPTOSYM_CONFIG_LRP_NUMKEYS_UPDATE                          0x0005U /**<  Number of times the loop to generate the UpdatedKey to be generated. */
#define PH_CRYPTOSYM_CONFIG_ADDITIONAL_INFO                             0x0006U /**<  Additional information to be provided like diversified key length. */
/**
 * end of group phCryptoSym_Defines_ConfigTypes
 * @}
 */

/** \defgroup phCryptoSym_Defines_KeepIV Keep IV
 * \brief Supported IV Updated Behavior Modes.
 * @{
 */
#define PH_CRYPTOSYM_VALUE_KEEP_IV_OFF                                  0x0000U /**< Switch off Keep-IV behavior. */
#define PH_CRYPTOSYM_VALUE_KEEP_IV_ON                                   0x0001U /**< Switch on Keep-IV behavior. */
/**
 * end of group phCryptoSym_Defines_KeepIV
 * @}
 */

/**
 * end of group phCryptoSym_Defines_Config
 * @}
 */

/**
 * end of group phCryptoSym_Defines
 * @}
 */

/**
 * end of group phCryptoSym
 * @}
 */
#endif /* NXPBUILD__PH_CRYPTOSYM */





#ifdef NXPBUILD__PH_CRYPTOSYM_SW

#define PH_CRYPTOSYM_SW_ID                                              0x01U   /**< ID for Software crypto component. */

/** \addtogroup phCryptoSym_Sw Component : Software
 * \brief Software implementation of the Symmetric Cryptography interface.
 *
 * This implementation was designed to optimize the footprint of crypto libraries used in embedded systems.
 * The following standards are implemented:
 * - Federal Information Processing Standards Publication 197: AES 128, 192 and 256
 * - Federal Information Processing Standards Publication 46-3: DES
 * - NIST Special Publication 800-67 Recommendation for the Triple Data Encryption Algorithm (TDEA) Block Cipher
 * - NIST Special Publication 800-38B: CMAC
 * - NIST Special Publication 800-38A: CBC and ECB mode
 * - NIST Special Publication 800-38A: CMC-MAC
 *
 * Hints for compiling the library:
 * - Carefully read the section on compile switches in order to find the optimum balance between speed and memory utilization.
 * - Using the appropriate compile switches either AES or DES can be removed from the built completely.
 *
 * Architecture of the phCryptoSym_Sw Component:
 * - The DES algorithm is implemented in the phCryptoSym_Sw_Des block
 * - The AES algorithm is implemented in the phCryptoSym_Sw_Aes block
 * - The phCryptoSym_Int block implements generic encrypt and decrypt functions. This offers the possibility to implement modes
 *   of operations without consideration of currently selected key type or key size.
 * - The phCryptoSym_Int block in addition implements helper functions for CMAC calculations.
 * @{
 */

/** \defgroup phCryptoSym_Sw_CompileSwitch Compile Switch
 * \brief Compile switches used to find the optimum trade-off between performance, memory footprint and supported features.
 * @{
 */

/**
 * \brief Enables DES support.
 *
 * Defines that the DES algorithm is supported. The defines for general DES capabilities like block sizes etc. are not affected
 * as they do not add to the memory footprint.
 */
#define PH_CRYPTOSYM_SW_DES

/**
 * \brief Enables AES support.
 *
 * Defines that the AES algorithm is supported. The defines for general AES capabilities like block sizes etc. are not affected
 * as they do not add to the memory footprint.
 */
#define PH_CRYPTOSYM_SW_AES

/**
 * \brief Enables online key scheduling.
 *
 * This define enables for both AES and DES the online key scheduling. This means, that the round keys are not pre-calculated
 * at key loading, but they are always calculated when a new block is going to be encrypted or decrypted.
 *
 * The following advantages come out of enabling online key scheduling:
 * - The pKey entry of the private data param structure decreases significantly from 384(DES enabled)/256(DES disabled) to 32 bytes.
 * - As the private data structure has to be created for each instance, the above mentioned savings count for each instance.
 * - Key loading is very fast (as there is almost nothing performed any more.
 * - On 8051 the keys can be located in fast RAM which counters some of the performance decrease compared to disabling that feature.
 *
 * The following disadvantages come out of enabling online key scheduling:
 * - Encryption gets slower as in addition to the ciphering also the round key generation has to be performed.
 * - For decryption in AES the situation is even worse, as the key scheduling is executed twice for each decryption.
 * - On small platforms like 8051 big key buffers can never reside in fast RAM as they exceed the memory size of data and data.
 *
 * On 8051 platforms in combination with the PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE enabling online key scheduling even gives better results
 * on execution time if only 1 or 2 blocks are encrypted with a given key. In case of keys are used longer (which is most likely the standard case),
 * it is faster to disable that feature.
 * Also note, that e.g. for a MIFARE Plus (R) instance of the library, two crypto instances are required, and as a consequence online key
 * scheduling can save 704(DES enabled)/(DES disabled)448 bytes of RAM.
 */
#define PH_CRYPTOSYM_SW_ONLINE_KEYSCHEDULING

/**
 * \brief Enables online CMAC subkey calculation.
 *
 * This define enables for both AES and DES the online CMAC subkey calculation. This means, that the CMAC subkeys are not stored in the
 * context of the individual instance of the crypto lib, but they are newly calculated for each MAC.
 *
 * The following advantages come out of enabling online CMAC subkey calculation:
 * - 32 bytes of RAM can be saved in the private data params (so they are saved on each instance of the crypto library).
 *
 * The following disadvantages come out of online CMAC subkey calculation:
 * - Each CMAC calculation needs 1 additional encryption and 2 additional shift operations, so the execution speed decreases.
 */
#define PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION

/**
 * \brief Enables ROM optimizations in the AES algorithm.
 *
 * This define removes some of the lookup tables in the AES implementation to save ROM space.
 *
 * The following advantages come out of enabling ROM optimizations:
 * - 3 lookup tables of 256 bytes can be saved (some additional code is needed, so in fact only ~600 bytes are saved).
 *
 * The following disadvantages come out of enabling ROM optimizations:
 * - The MixColumn and MixColumnInv implementation of the AES are getting slower.
 */
#define PH_CRYPTOSYM_SW_ROM_OPTIMIZATION

/**
 * \brief Enables 8051 data storage specifier.
 *
 * This define allows to specify any value for #PH_CRYTOSYM_SW_FAST_RAM. It takes care, that the buffers are recopied correctly,
 * and that most of the time consuming calculations are done on this fast memory. In case of #PH_CRYPTOSYM_SW_ONLINE_KEYSCHEDULING
 * is set, even the key scheduling can be performed on this fast memory.
 */
 /*
 #define PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
 */

#ifdef PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
#define PH_CRYTOSYM_SW_FAST_RAM data                                            /**< Fast RAM specifier, only useful in combination with
 *   #PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
 */
#define PH_CRYPTOSYM_SW_CONST_ROM                                               /**< Constant code specifier, only useful in combination
 *   with #PH_CRYPTOSYM_SW_USE_8051_DATA_STORAGE
 */
#define PH_CRYPTOSYM_CONST_ROM  PH_CRYPTOSYM_SW_CONST_ROM
#else
#define PH_CRYTOSYM_SW_FAST_RAM                                                 /**< Fast RAM specifier - not set per default */
#define PH_CRYPTOSYM_SW_CONST_ROM                                               /**< ROM specifier - not set per default */
#define PH_CRYPTOSYM_CONST_ROM  PH_CRYPTOSYM_SW_CONST_ROM
#endif

#ifndef PH_CRYPTOSYM_SW_AES
#ifdef PH_CRYPTOSYM_SW_DES
#ifndef PH_CRYPTOSYM_MAX_BLOCK_SIZE
#define PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE              PH_CRYPTOSYM_DES_BLOCK_SIZE /**< Maximum Block Size of the currently supported ciphers*/
#endif /* PH_CRYPTOSYM_MAX_BLOCK_SIZE */
#else
#error "No symmetric cipher available"
#endif /* PH_CRYPTOSYM_SW_DES */
#else
#define PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE              PH_CRYPTOSYM_AES_BLOCK_SIZE /**< Maximum Block Size of the currently supported ciphers*/

#ifndef PH_CRYPTOSYM_MAX_BLOCK_SIZE
#define PH_CRYPTOSYM_MAX_BLOCK_SIZE					PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE
#endif /* PH_CRYPTOSYM_MAX_BLOCK_SIZE */
#endif /* PH_CRYPTOSYM_SW_AES */

/* Key buffer size is calculated as follows: */
/* DES offline key scheduling: 3 #numKeys * 16 #numRounds * 8 #KeySize = 384 Bytes */
/* DES online key scheduling: 3 #numKeys * 2 #temporaryKey+originalKey * 8 #KeySize + 8 #intermediate result = 56 Bytes  */
/* AES offline key scheduling: (13u + 2U) (#numRounds + #original) * 16 #KeySize = 240 Bytes */
/* AES online key scheduling: (1u + 1U) (#temporary + #original) * 32 #KeySize = 64 Bytes */

#ifdef PH_CRYPTOSYM_SW_ONLINE_KEYSCHEDULING
#define PH_CRYPTOSYM_SW_KEY_BUFFER_SIZE                                         32U
#else
#ifdef PH_CRYPTOSYM_SW_DES
#define PH_CRYPTOSYM_SW_KEY_BUFFER_SIZE                                         384U
#else
#define PH_CRYPTOSYM_SW_KEY_BUFFER_SIZE                                         240U
#endif /* PH_CRYPTOSYM_SW_DES */
#endif /* PH_CRYPTOSYM_SW_ONLINE_KEYSCHEDULING */

 /**
 * end of group phCryptoSym_Sw_CompileSwitch
 * @}
 */

 /** \brief Data structure for Symmetric Crypto Software layer implementation. */
typedef struct
{
    uint16_t wId;                                                               /**< Layer ID for this component, NEVER MODIFY! */
    void * pKeyStoreDataParams;                                                 /**< Pointer to Key Store object - can be NULL. */
#ifndef NXPBUILD__PH_CRYPTOSYM_LRP
    uint8_t pKey[PH_CRYPTOSYM_SW_KEY_BUFFER_SIZE];                              /**< Internal key storage array */
#endif /* NXPBUILD__PH_CRYPTOSYM_LRP */
    uint8_t pIV[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE];                                /**< Internal IV storage array */
#ifndef PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION
    uint8_t pCMACSubKey1[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE];                       /**< Internal Key1 storage for MAC calculation. */
    uint8_t pCMACSubKey2[PH_CRYPTOSYM_SW_MAX_BLOCK_SIZE];                       /**< Internal Key2 storage for MAC calculation. */
    uint8_t bCMACSubKeysInitialized;                                            /**< Indicates whether the subkeys have been calculated. */
#endif /* PH_CRYPTOSYM_SW_ONLINE_CMAC_SUBKEY_CALCULATION */
    uint16_t wKeyType;                                                          /**< Key Type. */
    uint16_t wKeepIV;                                                           /**< Indicates if the init vector of a previous crypto operation shall be
                                                                                 * used for the next operation.
                                                                                 */
    uint16_t wAddInfo;                                                          /**< Additional information like diversified key length, etc. */
} phCryptoSym_Sw_DataParams_t;

/**
 * \brief Initialize the CryptoSym with Software as sub-component.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS
 *          - If the input size do not match the Dataparams size of this component.
 *          - If any of the DataParams are null.
 *
 */
phStatus_t phCryptoSym_Sw_Init(
        phCryptoSym_Sw_DataParams_t * pDataParams,                              /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wSizeOfDataParams,                                             /**< [In] Specifies the size of the data parameter structure. */
        void * pKeyStoreDataParams                                              /**< [In] Pointer to a key store structure (can be null). */
    );

/**
 * end of group phCryptoSym_Sw
 * @}
 */
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */















#ifdef NXPBUILD__PH_CRYPTOSYM

/** \addtogroup phCryptoSym CryptoSym
 *
 * \brief This is only a wrapper layer to abstract the different CryptoSym implementations.
 * With this wrapper it is possible to support more than one CryptoSym implementation
 * in parallel, by adapting this wrapper.
 *
 * Important hints for users of this component:
 * - Before use of any function, the dedicated crypto implementation has to be initialized using either of the interfaces mentioned below.
 *      - \ref phCryptoSym_Sw_Init "Software Initialization"
 *      - \ref phCryptoSym_mBedTLS_Init "mBedTLS Initialization"
 * - Functions using a key store (\ref phCryptoSym_LoadKey "LoadKey" and \ref phCryptoSym_DiversifyKey "Diversify Key") are only available if
 *   a key store has been passed during component initialization.
 * - Before any cipher operation or MAC operation (\ref phCryptoSym_Encrypt "Encrypt", \ref phCryptoSym_Decrypt "Decrypt", \ref phCryptoSym_CalculateMac
 *   "CalculateMAC") can be used, a key has to be loaded using either \ref phCryptoSym_LoadKey "LoadKey" or \ref phCryptoSym_LoadKeyDirect "LoadKeyDirect".
 * - Before any cipher operation or MAC operation (\ref phCryptoSym_Encrypt "Encrypt", \ref phCryptoSym_Decrypt "Decrypt", \ref phCryptoSym_CalculateMac
 *   "CalculateMAC") can be used, an appropriate IV has to be loaded by calling \ref phCryptoSym_LoadIv "LoadIv".
 * - Using \ref phCryptoSym_GetConfig "GetConfig", the block sizes, key lengths and Diversified key length for the currently loaded / diversified key can
 *   be retrieved.
 *
 * \note:
 *      The following are applicable when CryptoSym is initialized to use \ref phCryptoSym_mBedTLS "mBedTLS" as underlying layer.
 *      - LRP (Leakage Resilient Primitive) feature is not supported.
 *      - CMAC implementation of mBedTLS library is not utilized due to below mentioned reason(s)
 *          - When using \ref phalMfdfEVx "MIFARE DESFire" AL component, CMAC computation leaving the first call requires
 *            IV of the last subsequent calls. Here the IV is only zero for the first call and non zero for the rest of the
 *            calls. This behavior is required for EV1 Secure messaging of MIFARE DESFire product.
 *          - Its not possible to update the IV for intermediate / final calls provide by mBedTLS.
 *          - To over come this, CMAC is implemented directly in this component using cipher interfaces of mBedTLS.
 *          - The above limitation is valid only for \ref phCryptoSym_CalculateMac "CalculateMac" interface.
 *      - CMAC implementation of mBedTLS library is not utilized due to below mentioned reason(s)
 *          - CMAC implementation provided by mBedTLS library do not support 3DES-2Key key diversification.
 *          - Based on <a href="https://www.nxp.com/docs/en/application-note/AN10922.pdf">AN10922</a> CryptoSym should
 *            support diversification of 3DES-2Key but mBedTLS CMAC library do not support hit key type.
 *          - To over come this, CMAC is implemented directly in this component using cipher interfaces of mBedTLS.
 *          - The above limitation is valid only for below mentioned interfaces.
 *              - \ref phCryptoSym_DiversifyKey "Diversify the key available in KeyStore"
 *              - \ref phCryptoSym_DiversifyDirectKey "Diversify the provided key as input"
 * @{
 */

/**
 * \brief Invalidate the currently loaded key.
 * Resets the key, the IV, the keep IV flag and the key Type.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_InvalidateKey(
        void * pDataParams                                                      /**< [In] Pointer to this layer's parameter structure */
    );

/**
 * \brief Perform Encryption with one of the supported crypto modes
 *
 * The option word specifies the operation mode to use and the update behavior of the IV.
 * All modes of operation are coded in the LSB, the flags in the MSB.
 * The following Cipher modes are supported:
 * - #PH_CRYPTOSYM_CIPHER_MODE_ECB
 * - #PH_CRYPTOSYM_CIPHER_MODE_CBC
 * - #PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4
 *
 * The following Flags are supported:
 * - #PH_EXCHANGE_DEFAULT
 * - #PH_EXCHANGE_BUFFER_FIRST
 * - #PH_EXCHANGE_BUFFER_CONT
 * - #PH_EXCHANGE_BUFFER_LAST
 *
 * Note:  The input data length needs to be a multiple of the current block size
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_INVALID_PARAMETER        An unsupported key is loaded (or no key is loaded) or \b wBufferLength is not
 *                                          a multiple of the current block size.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER    An unknown cipher option wOption is specified.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_Encrypt(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wOption,                                                       /**< [In] Option byte specifying the cipher mode and the update behavior of the IV */
        const uint8_t * pPlainBuffer,                                           /**< [In] Plain data buffer.
                                                                                 *          - Should always be in multiple of current block size.
                                                                                 *          - If not of current block size then \ref phCryptoSym_ApplyPadding
                                                                                 *            "Apply Padding" needs to be used to make it upto current block size.
                                                                                 */
        uint16_t  wBufferLength,                                                /**< [In] Length of plain and encrypted data buffer - needs to be a multiple of the
                                                                                 *        current block size
                                                                                 */
        uint8_t * pEncryptedBuffer                                              /**< [Out] Encrypted data buffer. Will always be in multiple of current block size. */
    );

/**
 * \brief Perform Decryption with one of the supported crypto modes
 *
 * The option word specifies the operation mode to use and the update behavior of the IV.
 * All modes of operation are coded in the LSB, the flags in the MSB.
 * The following Cipher modes are supported:
 * - #PH_CRYPTOSYM_CIPHER_MODE_ECB
 * - #PH_CRYPTOSYM_CIPHER_MODE_CBC
 * - #PH_CRYPTOSYM_CIPHER_MODE_CBC_DF4
 *
 * The following Flags are supported:
 * - #PH_EXCHANGE_DEFAULT
 * - #PH_EXCHANGE_BUFFER_FIRST
 * - #PH_EXCHANGE_BUFFER_CONT
 * - #PH_EXCHANGE_BUFFER_LAST
 *
 * Note:  The input data length needs to be a multiple of the current block size
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_INVALID_PARAMETER        An unsupported key is loaded (or no key is loaded) or \b wBufferLength is not
 *                                          a multiple of the current block size.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER    An unknown cipher option wOption is specified.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_Decrypt(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wOption,                                                       /**< [In] Option byte specifying the cipher mode and the update behavior of the IV */
        const uint8_t * pEncryptedBuffer,                                       /**< [In] Encrypted data buffer. Should always be in multiple of current block size. */
        uint16_t  wBufferLength,                                                /**< [In] Length of plain and encrypted data buffer - needs to be a multiple of the
                                                                                 *        current block size.
                                                                                 */
        uint8_t * pPlainBuffer                                                  /**< [Out] Plain data buffer. Will always be in multiple of current block size. Plain
                                                                                 *         data may be padded with zeros if not current block size and needs to be
                                                                                 *         removed using \ref phCryptoSym_RemovePadding "Remove Padding" interface.
                                                                                 */
    );



/**
 * \brief Calculate MAC with one of the supported MAC modes
 *
 * The option word specifies the MAC mode to use and the update behavior of the IV as well as the completion behavior.
 * All modes of operation are coded in the LSB, the flags in the MSB.
 * The following Cipher modes are supported:
 * - #PH_CRYPTOSYM_MAC_MODE_CMAC
 * - #PH_CRYPTOSYM_MAC_MODE_CBCMAC
 *
 * The following Flags are supported:
 * - #PH_EXCHANGE_DEFAULT
 * - #PH_EXCHANGE_BUFFER_FIRST
 * - #PH_EXCHANGE_BUFFER_CONT
 * - #PH_EXCHANGE_BUFFER_LAST
 *
 * Note: If #PH_EXCHANGE_BUFFERED_BIT is set, the input length needs to be a multiple of the block length!
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_INVALID_PARAMETER        An unsupported key is loaded (or no key is loaded) or wDataLength is not
 *                                          a multiple of the current block size and the option #PH_EXCHANGE_BUFFERED_BIT is set.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER    An unknown mac option wOption is specified.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_CalculateMac(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wOption,                                                       /**< [In] Option byte specifying the MAC mode and the update behavior of
                                                                                 *        the IV and the completion flag.
                                                                                 */
        const uint8_t * pData,                                                  /**< [In] Input data on which the MAC needs to be computed.
                                                                                 *        Input will be always be in multiple of current block size if wOption is
                                                                                 *          - #PH_EXCHANGE_BUFFER_FIRST
                                                                                 *          - #PH_EXCHANGE_BUFFER_CONT
                                                                                 */
        uint16_t  wDataLength,                                                  /**< [In] number of input data bytes */
        uint8_t * pMac,                                                         /**< [Out] Output MAC block; uint8_t[16] */
        uint8_t * pMacLength                                                    /**< [Out] Length of MAC */
    );

/**
 * \brief Load IV
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_INVALID_PARAMETER        \b bIVLength does not match the current block size.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_LoadIv(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        const uint8_t * pIV,                                                    /**< [In] Initialization vector to use. Should of current block size. */
        uint8_t bIVLength                                                       /**< [In] Length of bytes available in \b pIV buffer. */
    );

/**
 * \brief Load Key
 *
 * This function uses the key storage provided at component initialization to retrieve the key identified by wKeyNo and wKeyVersion.
 * After retrieving the key is loaded into the internal key storage array to be prepared for subsequent cipher operations.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 *                                          - No KeyStore specified at initialization.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER    Key Type not supported.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_LoadKey(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                        /**< [In] Key number in KeyStore to be loaded. */
        uint16_t wKeyVersion,                                                   /**< [In] Key Version in KeyStore to be loaded. */
        uint16_t wKeyType                                                       /**< [In] Type of Key to be loaded. Supported ones are
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES128
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES192
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES256
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_2K3DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_3K3DES
                                                                                 */
    );

/**
 * \brief Direct Load Key
 *
 * The key provided in the pKey parameter is loaded into the internal key storage array to be prepared for subsequent cipher operations.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER    Key Type not supported.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_LoadKeyDirect(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        const uint8_t* pKey,                                                    /**< [In] Key to be loaded. Number of bytes should be based on the key
                                                                                 *        type mentioned in \b wKeyType parameter.
                                                                                 */
        uint16_t wKeyType                                                       /**< [In] Type of Key to be loaded. Supported ones are
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES128
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES192
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES256
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_2K3DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_3K3DES
                                                                                 */
    );

/**
 * \brief Diversify Key - Note: This function invalidates the currently loaded key.
 *
 * Using the key stored in the KeyStore passed at initialization of the component and identified by wKeyNo and wKeyVersion
 * this function calculates a diversified key according to the wOption specified that can be used in different applications.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 *                                          - No KeyStore specified at Initialization.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER    Key Type not supported (for key diversification).
 * \retval #PH_ERR_LENGTH_ERROR             Length of diversification input is wrong.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_DiversifyKey(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wOption,                                                       /**< [In] Option to specify the diversification method.
                                                                                 *        One of the below mentioned information.
                                                                                 *          - #PH_CRYPTOSYM_DIV_MODE_DESFIRE
                                                                                 *          - #PH_CRYPTOSYM_DIV_MODE_MIFARE_PLUS
                                                                                 *          - #PH_CRYPTOSYM_DIV_MODE_MIFARE_ULTRALIGHT
                                                                                 *          - #PH_CRYPTOSYM_DIV_OPTION_2K3DES_FULL
                                                                                 *          - #PH_CRYPTOSYM_DIV_OPTION_2K3DES_HALF
                                                                                 */
        uint16_t wKeyNo,                                                        /**< [In] Key number in KeyStore to be loaded */
        uint16_t wKeyVersion,                                                   /**< [In] Key Version in KeyStore to be loaded */
        uint8_t * pDivInput,                                                    /**< [In] Diversification Input used to diversify the key. */
        uint8_t  bLenDivInput,                                                  /**< [In] Length of diversification input used to diversify the key.
                                                                                 *        If 0, no diversification is performed.
                                                                                 */
        uint8_t * pDiversifiedKey                                               /**< [Out] Diversified key. Will be of current block size. */
    );

/**
 * \brief Diversify Direct Key - Note: This function invalidates the currently loaded key.
 *
 * Using the key passed in the pKey parameter this function calculates a diversified key according to the wOption
 * specified that can be used in different applications.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER    Key Type not supported (for key diversification).
 * \retval #PH_ERR_LENGTH_ERROR             Length of diversification input is wrong.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_DiversifyDirectKey(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wOption,                                                       /**< [In] Option to specify the diversification method.
                                                                                 *        One of the below mentioned information.
                                                                                 *          - #PH_CRYPTOSYM_DIV_MODE_DESFIRE
                                                                                 *          - #PH_CRYPTOSYM_DIV_MODE_MIFARE_PLUS
                                                                                 *          - #PH_CRYPTOSYM_DIV_MODE_MIFARE_ULTRALIGHT
                                                                                 *          - #PH_CRYPTOSYM_DIV_OPTION_2K3DES_FULL
                                                                                 *          - #PH_CRYPTOSYM_DIV_OPTION_2K3DES_HALF
                                                                                 */
        uint8_t * pKey,                                                         /**< [In] Key to be loaded. Number of bytes should be based on the key
                                                                                 *        type mentioned in \b wKeyType parameter.
                                                                                 */
        uint16_t wKeyType,                                                      /**< [In] Type of Key to be loaded. Supported ones are
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES128
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES192
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_AES256
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_2K3DES
                                                                                 *      - #PH_CRYPTOSYM_KEY_TYPE_3K3DES
                                                                                 */
        uint8_t * pDivInput,                                                    /**< [In] Diversification Input used to diversify the key. */
        uint8_t bLenDivInput,                                                   /**< [In] Length of diversification input used to diversify the key.
                                                                                 *        If 0, no diversification is performed.
                                                                                 */
        uint8_t * pDiversifiedKey                                               /**< [Out] Diversified key. Will be of current block size. */
    );

/**
 * \brief Apply Padding to a given data buffer.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_BUFFER_OVERFLOW          \b wDataOutBufSize is too small.
 * \retval #PH_ERR_INVALID_PARAMETER        Unsupported bOption.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_ApplyPadding(
        uint8_t bOption,                                                        /**< [In] Specifies padding mode
                                                                                 *          - #PH_CRYPTOSYM_PADDING_MODE_1
                                                                                 *          - #PH_CRYPTOSYM_PADDING_MODE_2
                                                                                 */
        const uint8_t *  pDataIn,                                               /**< [In] Input data for which padding is required. */
        uint16_t wDataInLength,                                                 /**< [In] Length of bytes available in \b pDataIn buffer. */
        uint8_t bBlockSize,                                                     /**< [In] Block size to be used for padding. */
        uint16_t wDataOutBufSize,                                               /**< [In] Size of output data buffer. */
        uint8_t * pDataOut,                                                     /**< [Out] Output data containing the information with padded bytes added. */
        uint16_t * pDataOutLength                                               /**< [Out] Length of bytes available in \b pDataOut buffer. */
    );

/**
 * \brief Remove Padding to a given data buffer.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_PARAMETER        Unsupported \b bOption or \b wDataInLength is not a multiple of the bBlockSize parameter.
 * \retval #PH_ERR_FRAMING_ERROR            Padding byte wrong. Expected 80h as the first padding byte if \b bOption = #PH_CRYPTOSYM_PADDING_MODE_2.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_RemovePadding(
        uint8_t bOption,                                                        /**< [In] Specifies padding mode
                                                                                 *          - #PH_CRYPTOSYM_PADDING_MODE_1
                                                                                 *          - #PH_CRYPTOSYM_PADDING_MODE_2
                                                                                 */
        const uint8_t * pDataIn,                                                /**< [In] Input data from which padding should be removed. */
        uint16_t wDataInLength,                                                 /**< [In] Length of bytes available in \b pDataIn buffer */
        uint8_t bBlockSize,                                                     /**< [In] Block size to be used for padding */
        uint16_t wDataOutBufSize,                                               /**< [In] Size of output data buffer */
        uint8_t * pDataOut,                                                     /**< [Out] Output data containing the information with padded bytes removed. */
        uint16_t * pDataOutLength                                               /**< [Out] Length of bytes available in \b pDataOut buffer */
    );

/**
 * \brief Set configuration parameter.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_INVALID_PARAMETER         Valid wConfig but invalid wValue for that config.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER     Invalid (Unsupported) wConfig.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_SetConfig(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                       /**< [In] Configuration Identifier. One of the below mentioned ones,
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_KEEP_IV
                                                                                 *          \cond !PUBLIC
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_LRP
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_LRP_NUMKEYS_UPDATE
                                                                                 *          \endcond
                                                                                 */
        uint16_t wValue                                                         /**< [In] Configuration Value for the provided configuration identifier.
                                                                                 *          - Refer \ref phCryptoSym_Defines_KeepIV "KeepIV Options" for
                                                                                 *            #PH_CRYPTOSYM_CONFIG_KEEP_IV configuration identifier.
                                                                                 *          \cond !PUBLIC
                                                                                 *          - #PH_ON to enable LRP and #PH_OFF to disable LRP are the valid values for
                                                                                 *            #PH_CRYPTOSYM_CONFIG_LRP configuration identifier.
                                                                                 *          \endcond
                                                                                 */
    );

/**
 * \brief Get configuration parameter.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_INVALID_PARAMETER         Value behind wConfig not valid at the moment.
 * \retval #PH_ERR_UNSUPPORTED_PARAMETER     Invalid (Unsupported) wConfig.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoSym_GetConfig(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wConfig,                                                       /**< [In] Configuration Identifier. One of the below mentioned ones,
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_KEY_TYPE
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_KEY_SIZE
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_BLOCK_SIZE
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_KEEP_IV
                                                                                 *          \cond !PUBLIC
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_LRP
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_LRP_NUMKEYS_UPDATE
                                                                                 *          \endcond
                                                                                 *          - #PH_CRYPTOSYM_CONFIG_ADDITIONAL_INFO
                                                                                 */
        uint16_t * pValue                                                       /**< [Out] Configuration Value for the provided configuration identifier.
                                                                                 *          - Refer \ref phCryptoSym_Defines_KeyTypes "Keytypes" for
                                                                                 *            #PH_CRYPTOSYM_CONFIG_KEY_TYPE configuration identifier.
                                                                                 *          - Refer \ref phCryptoSym_Defines_KeySize_DES "DES Key / Block Sizes" or
                                                                                 *            \ref phCryptoSym_Defines_KeySize_AES "AES Key / Block Sizes" for
                                                                                 *            #PH_CRYPTOSYM_CONFIG_KEY_SIZE or #PH_CRYPTOSYM_CONFIG_BLOCK_SIZE
                                                                                 *            configuration identifier.
                                                                                 *          - Refer \ref phCryptoSym_Defines_KeepIV "KeepIV Options" for
                                                                                 *            #PH_CRYPTOSYM_CONFIG_KEEP_IV configuration identifier.
                                                                                 *          \cond !PUBLIC
                                                                                 *          - #PH_ON to enable LRP and #PH_OFF to disable LRP are the valid values for
                                                                                 *            #PH_CRYPTOSYM_CONFIG_LRP configuration identifier.
                                                                                 *          \endcond
                                                                                 */
    );

/**
 * \brief Returns the status code and respective message. This interfaces is supported only if CryptoSym component is
 *  initialized with \ref phCryptoSym_mBedTLS_Init "mBedTLS Initialization".
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phCryptoSym_GetLastStatus(
        void * pDataParams,                                                         /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wStatusMsgLen,                                                     /**< [In] Size of bytes allocated for \b pStatusMsg parameter. */
        int8_t * pStatusMsg,                                                        /**< [Out] The equivalent status message for the information available in \b pStatusCode. */
        int32_t * pStatusCode                                                       /**< [Out] The status code returned by the underlying Crypto library. */
    );

/**
 * \brief Gets the size of the key.
 *
 * \return Status code
 * \retval 0        : If the \b wKeyType is not supported.
 * \retval Any Value: Size for the mentioned \b wKeyType information.
 */
uint16_t phCryptoSym_GetKeySize(uint16_t wKeyType);

/**
 * end of group phCryptoSym
 * @}
 */
#endif /* NXPBUILD__PH_CRYPTOSYM */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHCRYPTOSYM_H */
