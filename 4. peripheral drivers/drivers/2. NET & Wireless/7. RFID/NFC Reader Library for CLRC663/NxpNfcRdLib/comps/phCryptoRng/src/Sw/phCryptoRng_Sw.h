/*----------------------------------------------------------------------------*/
/* Copyright 2009 - 2013, 2022 NXP                                            */
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
* Software specific HAL-Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6625 $ (v07.09.00)
* $Date: 2022-05-04 15:42:46 +0530 (Wed, 04 May 2022) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHCRYPTORNG_SW_H
#define PHCRYPTORNG_SW_H

#include <ph_Status.h>
#include <phCryptoRng.h>

phStatus_t phCryptoRng_Sw_Seed(phCryptoRng_Sw_DataParams_t * pDataParams, uint8_t * bSeed, uint8_t bSeedLength);

phStatus_t phCryptoRng_Sw_Rnd(phCryptoRng_Sw_DataParams_t * pDataParams, uint16_t  wNoOfRndBytes, uint8_t * pRnd);

#endif /* PHCRYPTORNG_SW_H */
