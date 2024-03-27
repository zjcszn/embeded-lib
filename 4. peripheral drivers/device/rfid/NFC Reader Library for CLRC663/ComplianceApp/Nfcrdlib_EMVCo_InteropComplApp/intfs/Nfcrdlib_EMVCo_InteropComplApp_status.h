/*----------------------------------------------------------------------------*/
/* Copyright 2017-2020,2022 NXP                                               */
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
* Example Source Header to return status for Nfcrdlib_EMVCo_InteropComplApp.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#ifndef INTFS_NFCRDLIB_EMVCO_INTEROPCOMPLAPP_STATUS_H_
#define INTFS_NFCRDLIB_EMVCO_INTEROPCOMPLAPP_STATUS_H_
#include <ph_Status.h>

#ifdef NXPBUILD__PHHAL_HW_PN7642
#include <Pcrm_Lp_Reg.h>
#include <Clif.h>
#endif

#define EMVCO_INTEROP_ERR_SUCCESS    0U
#define EMVCO_INTEROP_ERR_FAIL       1U

#if !defined(PHDRIVER_PIPN5180_BOARD) && !defined(PHDRIVER_PIRC663_BOARD) && !defined(PHDRIVER_PIPN5190_BOARD)

#if defined(PHDRIVER_LPC1769PN5180_BOARD) || defined(PHDRIVER_LPC1769RC663_BOARD) || defined(PHDRIVER_LPC1769PN5190_BOARD)
#define EMVCO_INTEROP_LED_ON    PH_DRIVER_SET_HIGH
#define EMVCO_INTEROP_LED_OFF   PH_DRIVER_SET_LOW
#endif

#if defined(PHDRIVER_FRDM_K82FPN5180_BOARD) || defined(PHDRIVER_FRDM_K82FRC663_BOARD) || defined(PHDRIVER_K82F_PNEV5190B_BOARD)
#define EMVCO_INTEROP_LED_ON    PH_DRIVER_SET_LOW
#define EMVCO_INTEROP_LED_OFF   PH_DRIVER_SET_HIGH
#endif

#ifdef NXPBUILD__PHHAL_HW_PN7462AU
#define EMVCO_INTEROP_GREEN_LED      (10U)  /* PNEV7462C board GREEN LED connected to GPIO 10 */
#define EMVCO_INTEROP_RED_LED        (12U)  /* PNEV7462C board RED LED connected to GPIO 12 */
#define EMVCO_INTEROP_STAT_SUCCESS   (1U)   /* PNEV7462C board GPIO 1 */
#define EMVCO_INTEROP_STAT_FAIL      (2U)   /* PNEV7462C board GPIO 2 */
#endif

#ifdef NXPBUILD__PHHAL_HW_PN7642
#define EMVCO_INTEROP_GREEN_LED      PCRM_PAD_GPIO2/* PNEV7642 board GREEN LED connected to GPIO2 */
#define EMVCO_INTEROP_RED_LED        PCRM_PAD_GPIO3/* PNEV7642 board RED LED connected to GPIO3 */
#define EMVCO_INTEROP_STAT_SUCCESS   PCRM_PAD_GPIO1/* PNEV7642 board GPIO1 */
#define EMVCO_INTEROP_STAT_FAIL      PCRM_PAD_GPIO0/* PNEV7642 board GPIO0 */
#endif

#if defined(NXPBUILD__PHHAL_HW_PN7462AU)
#define EMVCO_INTEROP_STAT_GLED_ON()            PH_HAL_GPIO_SETGPIOVAL(EMVCO_INTEROP_GREEN_LED, true)
#define EMVCO_INTEROP_STAT_RLED_ON()            PH_HAL_GPIO_SETGPIOVAL(EMVCO_INTEROP_RED_LED, true)
#define EMVCO_INTEROP_STAT_GLED_OFF()           PH_HAL_GPIO_SETGPIOVAL(EMVCO_INTEROP_GREEN_LED, false)
#define EMVCO_INTEROP_STAT_RLED_OFF()           PH_HAL_GPIO_SETGPIOVAL(EMVCO_INTEROP_RED_LED, false)

#define EMVCO_INTEROP_STAT_SUCCESS_GPIO_ON()    PH_HAL_GPIO_SETGPIOVAL(EMVCO_INTEROP_STAT_SUCCESS, true)
#define EMVCO_INTEROP_STAT_FAIL_GPIO_ON()       PH_HAL_GPIO_SETGPIOVAL(EMVCO_INTEROP_STAT_FAIL, true)
#define EMVCO_INTEROP_STAT_SUCCESS_GPIO_OFF()   PH_HAL_GPIO_SETGPIOVAL(EMVCO_INTEROP_STAT_SUCCESS, false)
#define EMVCO_INTEROP_STAT_FAIL_GPIO_OFF()      PH_HAL_GPIO_SETGPIOVAL(EMVCO_INTEROP_STAT_FAIL, false)

#elif defined(NXPBUILD__PHHAL_HW_PN7642)
#define EMVCO_INTEROP_STAT_GLED_OFF()           CLIF_SETREG_MASK(PCRM_PADOUT,(1UL << PCRM_PADOUT_PADOUT_GPIO2_POS))
#define EMVCO_INTEROP_STAT_RLED_OFF()           CLIF_SETREG_MASK(PCRM_PADOUT,(1UL << PCRM_PADOUT_PADOUT_GPIO3_POS))
#define EMVCO_INTEROP_STAT_GLED_ON()            CLIF_CLEARREG_MASK(PCRM_PADOUT,~(1UL << PCRM_PADOUT_PADOUT_GPIO2_POS))
#define EMVCO_INTEROP_STAT_RLED_ON()            CLIF_CLEARREG_MASK(PCRM_PADOUT,~(1UL << PCRM_PADOUT_PADOUT_GPIO3_POS))

#define EMVCO_INTEROP_STAT_SUCCESS_GPIO_ON()    CLIF_SETREG_MASK(PCRM_PADOUT,(1UL << PCRM_PADOUT_PADOUT_GPIO1_POS))
#define EMVCO_INTEROP_STAT_FAIL_GPIO_ON()       CLIF_SETREG_MASK(PCRM_PADOUT,(1UL << PCRM_PADOUT_PADOUT_GPIO0_POS))
#define EMVCO_INTEROP_STAT_SUCCESS_GPIO_OFF()   CLIF_CLEARREG_MASK(PCRM_PADOUT,~(1UL << PCRM_PADOUT_PADOUT_GPIO1_POS))
#define EMVCO_INTEROP_STAT_FAIL_GPIO_OFF()      CLIF_CLEARREG_MASK(PCRM_PADOUT,~(1UL << PCRM_PADOUT_PADOUT_GPIO0_POS))
#else

#define EMVCO_INTEROP_STAT_GLED_ON()            phDriver_PinWrite(PHDRIVER_PIN_GLED, EMVCO_INTEROP_LED_ON)
#define EMVCO_INTEROP_STAT_RLED_ON()            phDriver_PinWrite(PHDRIVER_PIN_RLED, EMVCO_INTEROP_LED_ON)
#define EMVCO_INTEROP_STAT_GLED_OFF()           phDriver_PinWrite(PHDRIVER_PIN_GLED, EMVCO_INTEROP_LED_OFF)
#define EMVCO_INTEROP_STAT_RLED_OFF()           phDriver_PinWrite(PHDRIVER_PIN_RLED, EMVCO_INTEROP_LED_OFF)

#define EMVCO_INTEROP_STAT_SUCCESS_GPIO_ON()    phDriver_PinWrite(PHDRIVER_PIN_SUCCESS, PH_DRIVER_SET_HIGH)
#define EMVCO_INTEROP_STAT_FAIL_GPIO_ON()       phDriver_PinWrite(PHDRIVER_PIN_FAIL, PH_DRIVER_SET_HIGH)
#define EMVCO_INTEROP_STAT_SUCCESS_GPIO_OFF()   phDriver_PinWrite(PHDRIVER_PIN_SUCCESS, PH_DRIVER_SET_LOW)
#define EMVCO_INTEROP_STAT_FAIL_GPIO_OFF()      phDriver_PinWrite(PHDRIVER_PIN_FAIL, PH_DRIVER_SET_LOW)
#endif

#endif /* !defined(PHDRIVER_PIPN5180_BOARD) && !defined(PHDRIVER_PIRC663_BOARD) */

extern phStatus_t Emvco_Interop_Init(void);
extern void Emvco_Interop_Status(uint8_t Error_Status);

#endif /* INTFS_NFCRDLIB_EMVCO_INTEROPCOMPLAPP_STATUS_H_ */
