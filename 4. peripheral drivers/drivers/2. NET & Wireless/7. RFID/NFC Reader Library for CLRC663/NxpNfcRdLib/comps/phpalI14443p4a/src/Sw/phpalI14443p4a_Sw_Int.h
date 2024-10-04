/*----------------------------------------------------------------------------*/
/* Copyright 2016-2022 NXP                                                    */
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
* Internal definitions for Software ISO14443-4A Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#ifndef PHPALI14443P4A_SW_INT_H
#define PHPALI14443P4A_SW_INT_H

#include <ph_Status.h>

/** \addtogroup ph_Private
* @{
*/

/** HltA Command code. */
#define PHPAL_I14443P4A_SW_HALT_CMD             0x50U

/** Rats Command code. */
#define PHPAL_I14443P4A_SW_RATS                 0xE0U

/** PPS Command code. */
#define PHPAL_I14443P4A_SW_PPSS                 0xD0U

/** PPS0 code. */
#define PHPAL_I14443P4A_SW_PPS0                 0x11U

/** ISO 14443-4 S-Block. */
#define PHPAL_I14443P4A_SW_S_BLOCK              0xC0U

/** ISO 14443-4 S-Block RFU bits. */
#define PHPAL_I14443P4A_SW_S_BLOCK_RFU_BITS     0x02U

/** Bit within PCB which signals presence of CID byte .*/
#define PHPAL_I14443P4A_SW_PCB_CID_FOLLOWING    0x08U

/** Bits within PCB which signal a DESELECT command. */
#define PHPAL_I14443P4A_SW_PCB_DESELECT         0x00U

/** Activation FTW in microseconds. */
#define PHPAL_I14443P4A_SW_FWT_ACTIVATION_US    5286U

/** Position of T0 within ATS. */
#define PHPAL_I14443P4A_SW_ATS_T0               1U

/** Bit which signals TA(1) presence. */
#define PHPAL_I14443P4A_SW_ATS_TA1_PRESENT      0x10U

/** Bit which signals TB(1) presence. */
#define PHPAL_I14443P4A_SW_ATS_TB1_PRESENT      0x20U

/** Bit which signals TC(1) presence. */
#define PHPAL_I14443P4A_SW_ATS_TC1_PRESENT      0x40U

/** Bit within TC(1) which signals NAD support. */
#define PHPAL_I14443P4A_SW_ATS_TC1_NAD_SUPPORT  0x01U

/** Bit within TC(1) which signals CID support. */
#define PHPAL_I14443P4A_SW_ATS_TC1_CID_SUPPORT  0x02U

/** FWI default value. */
#define PHPAL_I14443P4A_SW_FWI_DEFAULT          0x04U

/** SFGI default value. */
#define PHPAL_I14443P4A_SW_SFGI_DEFAULT         0x00U

/** FSCI default value. */
#define PHPAL_I14443P4A_SW_FSCI_DEFAULT         0x02U

/** FTW min in microseconds. */
#define PHPAL_I14443P4A_SW_FWT_MIN_US           ((256*16)/13.56)

/** Wait time for halt command in microseconds. */
#define PHPAL_I14443P4A_SW_HALT_TIME_US         1100U

/** Time extension in microseconds for all commands.  */
#define PHPAL_I14443P4A_SW_EXT_TIME_US          60U

/** The maximum allowed FSDI/FSCI value as per ISO and EMVCo v3.1 */
#define PHPAL_I14443P4A_FRAMESIZE_MAX           12U

/** The minimum FSDI value as per EMVCo v3.1 - Requirements 5.9 */
#define PHPAL_I14443P4A_EMVCO_FRAMESIZE_MIN     8U

/* Delta SFGT as per EMVCo v3.1 and NFC Forum Digital Protocol v2.3 */
#define PHPAL_I14443P4A_DELTA_SFGT_US           (384 / 13.56)

/** Delta TPCD max in microseconds */
#define PHPAL_I14443P4A_SW_DELTA_TPCD_US_MAX    16400U

/** Delta TPCD minimum in microseconds */
#define PHPAL_I14443P4A_SW_DELTA_TPCD_US_MIN    0U

/** Delta frame wait time (FWT) in microseconds = 49152/fc = 3625us.
 * Providing additional 15us delay for NFC Forum compliance.
 * */
#define PHPAL_I14443P4A_SW_DELTA_FWT_US         3640U

/** Delta TPCD used for exchange (in microseconds) */
#define PHPAL_I14443P4A_SW_DELTA_TPCD_US        1000U

/** @}
* end of ph_Private
*/

#endif /* PHPALI14443P4A_SW_INT_H */
