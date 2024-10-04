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
 * This file contains the Output API for the DTA Test application.
 *
 * $Date: $
 * $Author: $
 * $Revision: $ (v07.09.00)
 */

/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */
#include "phApp_Init.h"
#include "Nfcrdlib_NFC_Forum_DTA_ComplApp.h"

#ifdef NXPBUILD__PHHAL_HW_PN7462AU
#include "ph_Reg.h"
#include "phhalGpio.h"
#include "phLED.h"
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

#ifdef NXPBUILD__PHHAL_HW_PN76XX
#include <Clif.h>
#include <Pcrm_Lp_Reg.h>
#endif /* NXPBUILD__PHHAL_HW_PN76XX */

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
/********************************************************************************************************************
 * Note: LED ON indicates loop/test is running and OFF indicates Input engine(waiting for Pattern number) is running.
 *       - PNEV5190B V1.0 or PN5190 RFBGA Board LED indication:
 *         D3(Red LED)
 *       - CLEV6630B 2.0 or PNEV5180B v2.0 Customer Evaluation Board LED indication:
 *         LD201(Red LED)
 *       - PNEV7462C Board LED indication:
 *         GPIO LED12(Red LED)
 *       - FRDM-K82F PN5180 or RC663 Board LED indication:
 *         D3(Red LED)
 *       - PNEV7642A Board LED indication:
 *         GPIO3(RED LED)
 ********************************************************************************************************************/
/* API to Initialize LED status indication mechanism */
void phDta_OutputInit (void)
{
#if defined(PH_PLATFORM_HAS_ICFRONTEND)
    phStatus_t status = PH_ERR_SUCCESS;
    phDriver_Pin_Config_t pinCfg;

    pinCfg.bOutputLogic = PHDTA_OUTPUT_GPIO_PIN_LO;
    pinCfg.bPullSelect = PH_DRIVER_PULL_DOWN;

    status = phDriver_PinConfig(PHDRIVER_PIN_LED, PH_DRIVER_PINFUNC_OUTPUT, &pinCfg);
    CHECK_STATUS(status);
#elif defined(NXPBUILD__PHHAL_HW_PN76XX)
    // GREEN LED(GPIO3) Init
    CLIF_SETREG(PCRM_PAD_GPIO3, PCRM_PAD_GPIO3_GPIO3_EN_OUT_MASK);
    CLIF_SETREG_MASK(PCRM_PADOUT,(1UL << PCRM_PADOUT_PADOUT_GPIO3_POS));
#elif defined(NXPBUILD__PHHAL_HW_PN7462AU)
    phLED_Init();
    phLED_SetStatus(PHDTA_OUTPUT_GPIO_PIN_LO);
#else
#error "No Platform is enabled!"
#endif
}

/* API to light up LED to indicate that test is running */
void phDta_IndicateTestRunning (void)
{
#if defined(PH_PLATFORM_HAS_ICFRONTEND)
    phDriver_PinWrite(PHDRIVER_PIN_LED, PHDTA_OUTPUT_GPIO_PIN_HI);
#elif defined (NXPBUILD__PHHAL_HW_PN76XX)
    CLIF_CLEARREG_MASK(PCRM_PADOUT,~(1UL << PCRM_PADOUT_PADOUT_GPIO3_POS));
#elif defined(NXPBUILD__PHHAL_HW_PN7462AU)
    phLED_SetStatus(LED_R);
#else
#error "No Platform is enabled!"
#endif
}

/* API to switch off LED to indicate that Input Engine is running */
void phDta_IndicateInputEngineRunning (void)
{
#if defined(PH_PLATFORM_HAS_ICFRONTEND)
    phDriver_PinWrite(PHDRIVER_PIN_LED, PHDTA_OUTPUT_GPIO_PIN_LO);
#elif defined (NXPBUILD__PHHAL_HW_PN76XX)
    CLIF_SETREG_MASK(PCRM_PADOUT,(1UL << PCRM_PADOUT_PADOUT_GPIO3_POS));
#elif defined(NXPBUILD__PHHAL_HW_PN7462AU)
    phLED_SetStatus(PHDTA_OUTPUT_GPIO_PIN_LO);
#else
#error "No Platform is enabled!"
#endif
}
