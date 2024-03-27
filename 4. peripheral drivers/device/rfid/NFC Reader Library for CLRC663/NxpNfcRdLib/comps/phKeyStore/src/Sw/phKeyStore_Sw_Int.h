/*----------------------------------------------------------------------------*/
/* Copyright 2006 - 2013, 2021 NXP                                            */
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
* Internal definitions for Keystore-Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6671 $ (v07.09.00)
* $Date: 2022-08-07 19:39:47 +0530 (Sun, 07 Aug 2022) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHKEYSTORE_SW_INT_H
#define PHKEYSTORE_SW_INT_H

#include <ph_Status.h>

/**
 * \brief Get the pointer to a KeyValue Pair given a Key number and a Key position.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phKeyStore_Sw_GetKeyValuePtrPos(
        phKeyStore_Sw_DataParams_t * pDataParams,                       /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                /**< [In] Desired key number.*/
        uint16_t wPos,                                                  /**< [In] Desired key position.*/
        phKeyStore_Sw_KeyVersionPair_t ** pKeyVersion                   /**< [Out] Pointer to the KeyVerison Pair.*/
    );

/**
 * \brief Get the pointer to a KeyValue Pair given a Key number and a KeyVersion.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phKeyStore_Sw_GetKeyValuePtrVersion(
        phKeyStore_Sw_DataParams_t * pDataParams,                       /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNumber,                                            /**< [In] Desired key number.*/
        uint16_t wKeyVersion,                                           /**< [In] Desired key version.*/
        phKeyStore_Sw_KeyVersionPair_t ** pKeyVersion                   /**< [Out] Pointer to the KeyVerison Pair.*/
    );

/**
 * \brief Check value of a counter and increment it accordingly.
 *
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlaying component.
 */
phStatus_t phKeyStore_Sw_CheckUpdateKUC(
        phKeyStore_Sw_DataParams_t * pDataParams,                       /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyUsageCtrNumber                                     /**< [In] KUC Number.*/
    );

phStatus_t phKeyStore_Sw_GetKeyValuePtr(
        phKeyStore_Sw_DataParams_t * pDataParams,                       /**< [In] Pointer to this layer's parameter structure. */
        uint16_t wKeyNo,                                                /**< [In] Desired key number.*/
        phKeyStore_Sw_KeyVersionPair_t ** pKeyPair                      /**< [Out] Pointer to the KeyVerison Pair.*/
    );

#endif /* PHKEYSTORE_SW_INT_H */
