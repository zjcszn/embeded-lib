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
 * This file implements the Input module and the Input engine of the DTA Test application.
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
#include "PN76_Reg_Interface.h"
#include "Pcrm_Lp_Reg.h"
#endif /* NXPBUILD__PHHAL_HW_PN76XX */

/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */
#define PHDTA_DP_VALIDATE_PATTERN_NUM(x)    \
    ((x == PHDTA_PATTERN_NUM_0) ||          \
    (x == PHDTA_PATTERN_NUM_1)  ||          \
    (x == PHDTA_PATTERN_NUM_2)  ||          \
    (x == PHDTA_PATTERN_NUM_3)  ||          \
    (x == PHDTA_PATTERN_NUM_5)  ||          \
    (x == PHDTA_PATTERN_NUM_6)  ||          \
    (x == PHDTA_PATTERN_NUM_7)  ||          \
    (x == PHDTA_PATTERN_NUM_8)  ||          \
    (x == PHDTA_PATTERN_NUM_A)  ||          \
    (x == PHDTA_PATTERN_NUM_B)  ||          \
    (x == PHDTA_PATTERN_NUM_11) ||          \
    (x == PHDTA_PATTERN_NUM_12) ||          \
    (x == PHDTA_PATTERN_NUM_21) ||          \
    (x == PHDTA_PATTERN_NUM_31))

/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
/* API to initialize the GPIO Pin that will be polled to determine when the user needs to change
 * run time configuration */
static phStatus_t phDta_InitInputPin (void);

/* API to read the status of the GPIO Pin that is used to determine when the user needs to change
 * run time configuration */
static phStatus_t phDta_ReadInputPin (uint8_t *bDtaInputPinStatus);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
#ifdef NXPBUILD__PHHAL_HW_PN76XX
#if (SEGGER_RTT_ENABLE == 1)
void DTA_SCAN(char *Input, volatile uint32_t *dwUserPattern)
{
   uint8_t bGetdata;
   uint32_t dwUserInput = 0xFFFFU; /* Invalid Data */
   uint8_t bRxValidData = 0;
   do
   {
      DTA_GET_INPUT(&bGetdata);
      if(bGetdata == 0x0A) /* Enter Char */
      {
         break;
      }

      bRxValidData++;
      if (bRxValidData == 1U)
      {
         dwUserInput = 0;
      }

      if((bGetdata >= 'a') && (bGetdata <= 'f'))
      {
         bGetdata -= 0x57;
      }
      else if((bGetdata >= 'A') && (bGetdata <= 'F'))
      {
         bGetdata -= 0x37;
      }
      else if ((bGetdata >= '0') && (bGetdata <= '9'))
      {
         bGetdata -= 0x30;
      }
      else
      {
         /* Invalid input, do nothing */
      }
      dwUserInput = dwUserInput << 4U;
      dwUserInput |= bGetdata;
   } while(1);

   *dwUserPattern = dwUserInput;
}
#endif /* (SEGGER_RTT_ENABLE == 1) */
#endif /* NXPBUILD__PHHAL_HW_PN76XX */

phStatus_t phDta_GetUserInput (void)
{
    phStatus_t status = PH_ERR_SUCCESS;
    volatile uint32_t dwUserPattern;

    DEBUG_PRINTF("\nphDta_GetUserInput: Get User Inputs.");

    do
    {
#ifdef NXPBUILD_DTA_CE_DEVICE_CLASS
        /* All Card Emulation tests will be execute with pattern number 0x00 */
        dwUserPattern = 0x00;
#else
        /* Prompt user for the Pattern number for the next test run */
        DTA_PRINT("\nEnter Pattern No: ");

        DTA_SCAN("%X",&dwUserPattern);
#endif /* NXPBUILD_DTA_CE_DEVICE_CLASS */

        gwPhDta_PatternNumber = dwUserPattern;

        if(PHDTA_DP_VALIDATE_PATTERN_NUM(dwUserPattern))
        {
            break;
        }
        else
        {
            DTA_PRINT("\nInvalid Pattern Number!");
        }
    }while(1);

    DTA_PRINT("\nGot User Inputs:\nPattern Number : 0x%X",gwPhDta_PatternNumber);

    return status;
}

/* API to Initialize the Input Engine */
phStatus_t phDta_InputEngineInit (void)
{
    phStatus_t status = PH_ERR_SUCCESS;
    status = phDta_InitInputPin();
    return status;
}

/* Input Engine API for checking if User wants to feed DTA input parameters, and get the parameters */
void phDta_InputEngine (void)
{
    phStatus_t status = PH_ERR_SUCCESS;
    uint8_t bGpioPinVal;
    volatile uint32_t dwUserPattern;

    /* Indicate to the User that the input engine is running */
    phDta_IndicateInputEngineRunning();

    do
    {
        /* Read the GPIO Pin */
        status = phDta_ReadInputPin(&bGpioPinVal);
        if (status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_InputEngine: Read Pin Value Failed!");
            break;
        }

        if(bGpioPinVal != PHDTA_INPUTENGINE_GPIO_DEFSTATE)
        {
            DEBUG_PRINTF("\nphDta_InputEngine: User Input Available.");

            do
            {
                /* Prompt user for the New Pattern number for the next test run */
                DTA_PRINT("\nEnter New Pattern No: ");

                DTA_SCAN("%X",&dwUserPattern);
                gwPhDta_PatternNumber = dwUserPattern;

                if(PHDTA_DP_VALIDATE_PATTERN_NUM(dwUserPattern))
                {
                    break;
                }
                else
                {
                    DTA_PRINT("\nThe given Pattern Number is Invalid!");
                }
            }while(1);

            DTA_PRINT("\nGot New User Inputs:\nNew Pattern Number : 0x%X",gwPhDta_PatternNumber);
        }
        else
        {
            /* No User Input Available. Nothing to do Here! */
        }

    }while(0);

    /* Light up LED to indicate that the DTA Main Loop has resumed */
    phDta_IndicateTestRunning();
}

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */
/********************************************************************************************************************
 * Note: The below mentioned GPIO is used to change runtime DTA Pattern number.
 *       GPIO default value is configured to HIGH, to change DTA Pattern number, GPIO should be shorted to GND.
 *       - PNEV5190B V1.0 Board GPIO:
 *         TP8(PortD, Pin12)
 *       - PN5190 RFBGA Board GPIO:
 *         J12-3(Port1, Pin25) [Note: Remove 3-4 pins jumper]
 *       - PNEV76FAMA Board GPIO:
 *         USER BUTTON(GPIO5)
 *       - CLEV6630B 2.0 or PNEV5180B v2.0 Customer Evaluation Board GPIO:
 *         J201,Pin1(TX)[Port0, Pin2]
 *       - PNEV7462C Board GPIO:
 *         GPIO1
 *       - FRDM-K82F PN5180 or RC663 Board GPIO indication:
 *         J2-18(PortA Pin1)
 ********************************************************************************************************************/

/* API to initialize the GPIO Pin that will be polled to determine when the user needs to
 * change run time configuration(DTA Pattern number).
 */
static phStatus_t phDta_InitInputPin (void)
{
    phStatus_t status = PH_ERR_SUCCESS;

#if defined(PH_PLATFORM_HAS_ICFRONTEND)
    phDriver_Pin_Config_t pinCfg;

    pinCfg.bOutputLogic = PHDTA_INPUTENGINE_GPIO_DEFSTATE;
    pinCfg.bPullSelect = PH_DRIVER_PULL_UP;

    status = phDriver_PinConfig(PHDRIVER_PIN_GPIO, PH_DRIVER_PINFUNC_INPUT, &pinCfg);
    CHECK_STATUS(status);
#elif defined(NXPBUILD__PHHAL_HW_PN76XX)
    /* Configured as per PNEV76XXFAMA Board */
    status = PN76_Sys_WriteRegister(PCRM_PAD_GPIO5, (PCRM_PAD_GPIO5_GPIO5_EN_IN_MASK | PCRM_PAD_GPIO5_GPIO5_PUPD_MASK));
#elif defined(NXPBUILD__PHHAL_HW_PN7462AU)
    phhalPcr_GpioStatus_t eGpioStatus;
    do
    {
        /* Enable/Disable Pull up and Pull Down for the selected GPIO Pin */
        status = phhalPcr_ConfigPuPd(PHDTA_INPUTENGINE_GPIO_PIN, TRUE, FALSE);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_InitInputPin: Config PU/PD Failed!");
            break;
        }

        /* Configure the selected GPIO pin as an input */
        status = phhalPcr_ConfigInput(PHDTA_INPUTENGINE_GPIO_PIN, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_InitInputPin: Config Pin as Input Failed!");
            break;
        }

        /* Check the status of the GPIO pin */
        status = phhalPcr_GetGpioStatus(PHDTA_INPUTENGINE_GPIO_PIN, &eGpioStatus);
        if((status != PH_ERR_SUCCESS) || (eGpioStatus != E_PCR_GPIO_HAL_INPUT_ENABLED))
        {
            DEBUG_PRINTF("\nphDta_InitInputPin: Pin Config Unsuccessful!");
            break;
        }

    }while(0);
#else
#error "No Platform is enabled!"
#endif

    return status;
}

/* API to read the status of the GPIO Pin that is used to determine when the user needs to
 * change run time configuration(DTA Pattern number).
 */
static phStatus_t phDta_ReadInputPin (uint8_t *bDtaInputPinStatus)
{
    phStatus_t status = PH_ERR_SUCCESS;
#if defined(PH_PLATFORM_HAS_ICFRONTEND)
    *bDtaInputPinStatus = phDriver_PinRead(PHDRIVER_PIN_GPIO, PH_DRIVER_PINFUNC_INPUT);
#elif defined(NXPBUILD__PHHAL_HW_PN76XX)
    /* Configured as per PNEV76XXFAMA Board */
    uint32_t dwRegVal = 0;
#ifdef NXPBUILD__PHHAL_HW_PN7640
    status = PN76_Sys_ReadRegister(PCRM_PADIN, &dwRegVal);
#endif /* NXPBUILD__PHHAL_HW_PN7640 */
#ifdef NXPBUILD__PHHAL_HW_PN7642
    dwRegVal = PN76_Sys_ReadRegister(PCRM_PADIN);
#endif /* NXPBUILD__PHHAL_HW_PN7642 */
    *bDtaInputPinStatus = (dwRegVal >> PCRM_PADIN_PADIN_GPIO5_POS) & 0x01U;
#elif defined(NXPBUILD__PHHAL_HW_PN7462AU)
    status = phhalPcr_GetGpioVal(PHDTA_INPUTENGINE_GPIO_PIN, bDtaInputPinStatus);
#else
#error "No Platform is enabled!"
#endif
    return status;
}
