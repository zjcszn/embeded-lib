/*----------------------------------------------------------------------------*/
/* Copyright 2016-2021 NXP                                                    */
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
* Software ISO14443-3A Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#ifndef PHPALI14443P3A_SW_H
#define PHPALI14443P3A_SW_H

#include <ph_Status.h>

phStatus_t phpalI14443p3a_Sw_SetConfig(
                                       phpalI14443p3a_Sw_DataParams_t * pDataParams,
                                       uint16_t wConfig,
                                       uint16_t wValue
                                     );
phStatus_t phpalI14443p3a_Sw_GetConfig(
                                       phpalI14443p3a_Sw_DataParams_t * pDataParams,
                                       uint16_t wConfig,
                                       uint16_t *pValue
                                     );
phStatus_t phpalI14443p3a_Sw_RequestA(
                                      phpalI14443p3a_Sw_DataParams_t * pDataParams,
                                      uint8_t * pAtqa
                                      );


phStatus_t phpalI14443p3a_Sw_WakeUpA(
                                     phpalI14443p3a_Sw_DataParams_t * pDataParams,
                                     uint8_t * pAtqa
                                     );

phStatus_t phpalI14443p3a_Sw_HaltA(
                                   phpalI14443p3a_Sw_DataParams_t * pDataParams
                                   );

phStatus_t phpalI14443p3a_Sw_Anticollision(
    phpalI14443p3a_Sw_DataParams_t * pDataParams,
    uint8_t bCascadeLevel,
    uint8_t * pUidIn,
    uint8_t bNvbUidIn,
    uint8_t * pUidOut,
    uint8_t * pNvbUidOut
    );

phStatus_t phpalI14443p3a_Sw_Select(
                                    phpalI14443p3a_Sw_DataParams_t * pDataParams,
                                    uint8_t bCascadeLevel,
                                    uint8_t * pUidIn,
                                    uint8_t * pSak
                                    );

phStatus_t phpalI14443p3a_Sw_ActivateCard(
    phpalI14443p3a_Sw_DataParams_t * pDataParams,
    uint8_t * pUidIn,
    uint8_t bLenUidIn,
    uint8_t * pUidOut,
    uint8_t * pLenUidOut,
    uint8_t * pSak,
    uint8_t * pMoreCardsAvailable
    );

phStatus_t phpalI14443p3a_Sw_Exchange(
                                      phpalI14443p3a_Sw_DataParams_t * pDataParams,
                                      uint16_t wOption,
                                      uint8_t * pTxBuffer,
                                      uint16_t wTxLength,
                                      uint8_t ** ppRxBuffer,
                                      uint16_t * pRxLength
                                      );

phStatus_t phpalI14443p3a_Sw_GetSerialNo(
    phpalI14443p3a_Sw_DataParams_t * pDataParams,
    uint8_t * pUidOut,
    uint8_t * pLenUidOut
    );

#endif /* PHPALI14443P3A_SW_H */
