/*----------------------------------------------------------------------------*/
/* Copyright 2020 NXP                                                         */
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
* Example Header for NfcrdlibEx1_DiscoveryLoop to work with EMVCo Profile.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#ifndef NFCRDLIBEX1_EMVCOPROFILE_H
#define NFCRDLIBEX1_EMVCOPROFILE_H

#include <ph_Status.h>
#include <phacDiscLoop.h>

void EmvcoProfileProcess (phacDiscLoop_Sw_DataParams_t * pDataParams, phStatus_t eDiscStatus);

#endif /* NFCRDLIBEX1_EMVCOPROFILE_H */
