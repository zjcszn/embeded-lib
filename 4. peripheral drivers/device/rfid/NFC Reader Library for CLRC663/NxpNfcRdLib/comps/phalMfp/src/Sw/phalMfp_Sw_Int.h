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
* Internal functions of Software implementation of MIFARE Plus contactless IC application layer.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHALMFP_SW_INT_H
#define PHALMFP_SW_INT_H

#include <ph_Status.h>
#include <phalMfp.h>

/** \addtogroup ph_Private
* @{
*/

#define PHAL_MFP_TAPEOUT_VERSION            30U

#define PHAL_MFP_ORIGINALITY_KEY_0          0x8000U
#define PHAL_MFP_ORIGINALITY_KEY_1          0x8001U
#define PHAL_MFP_ORIGINALITY_KEY_2          0x8002U
#define PHAL_MFP_ORIGINALITY_KEY_FIRST      PHAL_MFP_ORIGINALITY_KEY_0
#define PHAL_MFP_ORIGINALITY_KEY_LAST       PHAL_MFP_ORIGINALITY_KEY_2


/** @}
* end of ph_Private group
*/

#endif /* PHALMFP_SW_INT_H */
