/*----------------------------------------------------------------------------*/
/* Copyright 2009 - 2022 NXP                                                  */
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
* Software ISO14443-4A Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#ifndef PHPALI14443P4A_SW_H
#define PHPALI14443P4A_SW_H

#include <ph_Status.h>

phStatus_t phpalI14443p4a_Sw_Rats(
    phpalI14443p4a_Sw_DataParams_t * pDataParams,
    uint8_t bFsdi,
    uint8_t bCid,
    uint8_t * pAts
    );

phStatus_t phpalI14443p4a_Sw_Pps(
    phpalI14443p4a_Sw_DataParams_t * pDataParams,
    uint8_t bDri,
    uint8_t bDsi
    );

phStatus_t phpalI14443p4a_Sw_ActivateCard(
    phpalI14443p4a_Sw_DataParams_t * pDataParams,
    uint8_t bFsdi,
    uint8_t bCid,
    uint8_t bDri,
    uint8_t bDsi,
    uint8_t * pAts
    );

phStatus_t phpalI14443p4a_Sw_GetProtocolParams(
    phpalI14443p4a_Sw_DataParams_t * pDataParams,
    uint8_t * pCidEnabled,
    uint8_t * pCid,
    uint8_t * pNadSupported,
    uint8_t * pFwi,
    uint8_t * pFsdi,
    uint8_t * pFsci
    );

phStatus_t phpalI14443p4a_Sw_SetConfig(
    phpalI14443p4a_Sw_DataParams_t * pDataParams,
    uint16_t wConfig,
    uint16_t wValue
    );

phStatus_t phpalI14443p4a_Sw_GetConfig(
    phpalI14443p4a_Sw_DataParams_t * pDataParams,
    uint16_t wConfig,
    uint16_t *pValue
    );

#endif /* PHPALI14443P4A_SW_H */
