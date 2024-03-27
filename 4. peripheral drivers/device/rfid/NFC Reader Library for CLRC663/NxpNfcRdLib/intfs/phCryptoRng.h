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
* Generic Random Number Component of the Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6660 $ (v07.09.00)
* $Date: 2022-06-27 15:09:30 +0530 (Mon, 27 Jun 2022) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHCRYPTORNG_H
#define PHCRYPTORNG_H

#include <ph_Status.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifdef NXPBUILD__PH_CRYPTORNG_SW
#define PH_CRYPTORNG_SW_ID                                              0x01U   /**< ID for Software crypto rng component. */

/** \defgroup phCryptoRng_Sw Component : Software
 * \brief Software implementation of the Random Number interface
 *
 * The following standard is implemented:
 * - NIST Special Publication 800-90: CTR_DRBG
 *
 * Architecture overview of the \ref phCryptoRng_Sw "Software" Component
 * - The AES 128 implementation of the phCryptoSym component is used to generate random numbers.
 * - This phCryptoSym instance has to be properly instantiated before generation of random numbers is possible.
 * - The phCryptoRng_Sw_Int block implements all functions required by the NIST standard.
 * - According to the state of the library component, the seed function choses whether to use #phCryptoRng_Sw_Instantiate or #phCryptoRng_Sw_Reseed.
 * - #phCryptoRng_Sw_Update is automatically called when random numbers are generated in order to be ready for retrieving the next chunk of
 * random bytes.
 * - #phCryptoRng_Sw_BlockCipherDf is used to derive the key and V-value for a given seed.
 * @{
 */

/** \brief Data structure for Random Number's Software layer implementation. */
typedef struct
{
    uint16_t wId;                                                               /**< Layer ID for this component, NEVER MODIFY! */
    void * pCryptoDataParams;                                                   /**< Data parameter structure for the AES engine */
    uint8_t V[16];
    uint32_t dwRequestCounter;                                                  /**< Counts the amount of requests between two seeding procedures.
                                                                                 *   Note: according to NIST SP800-90 for AES this is 2^48, for storage
                                                                                 *   reasons the limit is set to 2^32 in this particular implementation.
                                                                                 */
    uint8_t bState;
} phCryptoRng_Sw_DataParams_t;

/**
 * \brief Initialize the CryptoRng with software as sub-component.
 *
 * For seeding, the following considerations have to be taken:
 * - Take a seed of at 32 bytes, which consists of
 *   - entropy input
 *   - nonce
 *   - personalization string.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS
 *          - If the input size do not not match the Dataparams size of this component.
 *          - If the DataParams are null.
 */
phStatus_t phCryptoRng_Sw_Init(
        phCryptoRng_Sw_DataParams_t * pDataParams,                              /**< [In] Pointer to this layers parameter structure. */
        uint16_t wSizeOfDataParams,                                             /**< [In] Specifies the size of the data parameter structure. */
        void * pCryptoDataParams                                                /**< [In] Pointer to the parameter structure of the symmetric crypto layer. */
    );

/**
 * end of group phCryptoRng_Sw
 * @}
 */
#endif /* NXPBUILD__PH_CRYPTORNG_SW */















#ifdef NXPBUILD__PH_CRYPTORNG

/** \addtogroup phCryptoRng CryptoRng
 *
 * \brief This is only a wrapper layer to abstract the different crypto random number generator implementations.
 * With this wrapper it is possible to support more than one crypto random number generator implementation
 * in parallel, by adapting this wrapper.
 *
 * Important hints for users of this component:
 * - Before use of any function, the dedicated rng implementation has to be initialized (e.g. #phCryptoRng_Sw_Init)
 * - Before random numbers can be generated, the #phCryptoRng_Seed function has to be called
 * @{
 */

/**
 * \brief Seeds the random number generator with the given seed.
 *
 * The seed length is dependend on the underlaying implementation. Further details can be found in the detailed description
 * of the underlying layers.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoRng_Seed(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint8_t * pSeed,                                                        /**< [In] Seed */
        uint8_t bSeedLength                                                     /**< [In] Size of the seed. */
    );

/**
 * \brief Obtains random bytes from the random source.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS                  Operation successful.
 * \retval #PH_ERR_INVALID_DATA_PARAMS      - If the interface context (\b pDataparams) is not holding this layer ID.
 *                                          - If the component context holds a different sub-component ID that is not
 *                                            supported by this layer.
 * \retval #PH_ERR_INVALID_PARAMETER        - Ifd the buffer is null.
 * \retval Other Depending on implementation and underlying component.
 */
phStatus_t phCryptoRng_Rnd(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t  wNoOfRndBytes,                                                /**< [In] Number of random bytes to generate. */
        uint8_t * pRnd                                                          /**< [Out] Generated bytes; uint8_t[wNoOfRndBytes] */
    );

/**
 * \brief Returns the status code and respective message. This interfaces is supported only if CryptoSym component is
 *  initialized with \ref phCryptoRng_mBedTLS_Init "mBedTLS Initialization".
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 */
phStatus_t phCryptoRng_GetLastStatus(
        void * pDataParams,                                                     /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wStatusMsgLen,                                                 /**< [In] Size of bytes allocated for \b pStatusMsg parameter. */
        int8_t * pStatusMsg,                                                    /**< [Out] The equivalent status message for the information available in \b pStatusCode. */
        int32_t * pStatusCode                                                   /**< [Out] The status code returned by the underlying Crypto library. */
    );

/**
 * end of group phCryptoRng
 * @}
 */
#endif /* NXPBUILD__PH_CRYPTORNG */

#ifdef __cplusplus
} /* Extern C */
#endif

#endif /* PHCRYPTORNG_H */
