/*----------------------------------------------------------------------------*/
/* Copyright 2009-2023 NXP                                                    */
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
* Software ISO14443-4 Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$

*/

#ifndef PHPALI14443P4_SW_H
#define PHPALI14443P4_SW_H

#include <ph_Status.h>

phStatus_t phpalI14443p4_Sw_SetProtocol(
                                        phpalI14443p4_Sw_DataParams_t * pDataParams,
                                        uint8_t   bCidEnable,
                                        uint8_t   bCid,
                                        uint8_t   bNadEnable,
                                        uint8_t   bNad,
                                        uint8_t   bFwi,
                                        uint8_t   bFsdi,
                                        uint8_t   bFsci
                                        );

phStatus_t phpalI14443p4_Sw_ResetProtocol(
    phpalI14443p4_Sw_DataParams_t * pDataParams
    );

phStatus_t phpalI14443p4_Sw_Deselect(
                                     phpalI14443p4_Sw_DataParams_t * pDataParams
                                     );

phStatus_t phpalI14443p4_Sw_PresCheck(
                                      phpalI14443p4_Sw_DataParams_t * pDataParams
                                      );

phStatus_t phpalI14443p4_Sw_Exchange(
                                     phpalI14443p4_Sw_DataParams_t * pDataParams,
                                     uint16_t wOption,
                                     uint8_t * pTxBuffer,
                                     uint16_t wTxLength,
                                     uint8_t ** ppRxBuffer,
                                     uint16_t * pRxLength
                                     );

phStatus_t phpalI14443p4_Sw_SetConfig(
                                      phpalI14443p4_Sw_DataParams_t * pDataParams,
                                      uint16_t wConfig,
                                      uint16_t wValue
                                      );

phStatus_t phpalI14443p4_Sw_GetConfig(
                                      phpalI14443p4_Sw_DataParams_t * pDataParams,
                                      uint16_t wConfig,
                                      uint16_t * pValue
                                      );


#endif /* PHPALI14443P4_SW_H */
