/*----------------------------------------------------------------------------*/
/* Copyright 2009-2020 NXP                                                    */
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
* Software MIFARE Plus contactless IC Application Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
* History:
*  CHu: Generated 31. August 2009
*
*/

#ifndef PHALMFP_SW_H
#define PHALMFP_SW_H

#include <ph_Status.h>

phStatus_t phalMfp_Sw_WritePerso(
                                 phalMfp_Sw_DataParams_t * pDataParams,
                                 uint8_t bLayer4Comm,
                                 uint16_t wBlockNr,
                                 uint8_t * pValue
                                 );

phStatus_t phalMfp_Sw_CommitPerso(
                                  phalMfp_Sw_DataParams_t * pDataParams,
                                  uint8_t bLayer4Comm
                                  );


phStatus_t phalMfp_Sw_AuthenticateClassicSL2(
    phalMfp_Sw_DataParams_t * pDataParams,
    uint8_t bBlockNo,
    uint8_t bKeyType,
    uint16_t wKeyNo,
    uint16_t wKeyVersion,
    uint8_t * pUid,
    uint8_t bUidLength
    );

phStatus_t phalMfp_Sw_MultiBlockRead(
                                     phalMfp_Sw_DataParams_t * pDataParams,
                                     uint8_t bBlockNr,
                                     uint8_t bNumBlocks,
                                     uint8_t * pBlocks
                                     );

phStatus_t phalMfp_Sw_MultiBlockWrite(
                                      phalMfp_Sw_DataParams_t * pDataParams,
                                      uint8_t bBlockNr,
                                      uint8_t bNumBlocks,
                                      uint8_t * pBlocks
                                      );

phStatus_t phalMfp_Sw_ResetAuth(
                                phalMfp_Sw_DataParams_t * pDataParams
                                );


phStatus_t phalMfp_Sw_ResetSecMsgState(
                                       phalMfp_Sw_DataParams_t * pDataParams
                                       );


#endif /* PHALMFP_SW_H */
