/*----------------------------------------------------------------------------*/
/* Copyright 2016-2023 NXP                                                    */
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

/** @file
 *
 * Interface file for the DTA loop Test/Input Engine running status(LED Indication).
 *
 * $Author: NXP $
 * $Revision: $ (v07.09.00)
 * $Date: $
 */

#ifndef PHDTA_OUTPUT_H_
#define PHDTA_OUTPUT_H_

/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */
#ifdef NXPBUILD__PHHAL_HW_PN7462AU
#include "ph_Datatypes.h"
#endif
#include "ph_Status.h"

/* *****************************************************************************************************************
 * MACROS/Defines
 * ***************************************************************************************************************** */
#if defined (PHDRIVER_K82F_PNEV5190B_BOARD)
#define PHDTA_OUTPUT_GPIO_PIN_HI    (FALSE)
#define PHDTA_OUTPUT_GPIO_PIN_LO    (TRUE)
#else /* PHDRIVER_K82F_PNEV5190B_BOARD */
#define PHDTA_OUTPUT_GPIO_PIN_HI    (TRUE)
#define PHDTA_OUTPUT_GPIO_PIN_LO    (FALSE)
#endif /* PHDRIVER_K82F_PNEV5190B_BOARD */

#ifdef NXPBUILD__PHHAL_HW_PN76XX
#define NFCFORUM_DTA_RED_LED        PCRM_PAD_GPIO3/* PNEV7642 board RED LED connected to GPIO3 */
#endif /* NXPBUILD__PHHAL_HW_PN76XX */

/* *****************************************************************************************************************
 * Function Prototypes
 * ***************************************************************************************************************** */
/* API to Initialize status indication mechanism */
extern void phDta_OutputInit (void);

/* API to light up LED to indicate that test is running */
extern void phDta_IndicateTestRunning (void);

/* API to light up LED to indicate that Input Engine is running */
extern void phDta_IndicateInputEngineRunning (void);

#endif /* PHDTA_OUTPUT_H_ */
