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
 * This file contains the DTA Poll Mode Test Procedure Module.
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

#ifdef NXPBUILD_DTA_READER_DEVICE_CLASS
/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */
/* Flag to enable Polling for Technology A */
#define PHDTA_CON_POLL_A    (1U)

/* Flag to enable Polling for Technology F */
#define PHDTA_CON_POLL_F    (1U)

/* Flag to enable Polling for Technology V */
#define PHDTA_CON_POLL_V    (1U)

/* Bit masks for configuring technologies to poll for in Polling Mode */
#define PHDTA_CON_POLL_A_MASK         ((PHDTA_CON_POLL_A)? (PHAC_DISCLOOP_POS_BIT_MASK_A) : (0x0))
#define PHDTA_CON_POLL_B_MASK(X)      ((X)? (PHAC_DISCLOOP_POS_BIT_MASK_B) : (0x0))
#define PHDTA_CON_POLL_F_212_MASK(X)  ((PHDTA_CON_POLL_F && (!X))? (PHAC_DISCLOOP_POS_BIT_MASK_F212) : (0x0))
#define PHDTA_CON_POLL_F_424_MASK(X)  ((PHDTA_CON_POLL_F && (X))? (PHAC_DISCLOOP_POS_BIT_MASK_F424) : (0x0))
#define PHDTA_CON_POLL_V_MASK         ((PHDTA_CON_POLL_V)? (PHAC_DISCLOOP_POS_BIT_MASK_V) : (0x0))

#define PHDTA_SEL_RESPONSE_BITMASK_B6_B5   (0x60U)

/* Poll Mode field off Parameter */
#define PHDTA_TFIELD_OFF         (5100UL) /* Minimum TFIELD_OFF given by Spec is 5100 us. */
#define PHDTA_TFIELD_OFF_TICKS   ((PHDTA_TFIELD_OFF/1000UL) + 1U) /* Field Off Period in Ticks (1ms resolution )*/

/* *****************************************************************************************************************
 * Global and Static Variables
 * ***************************************************************************************************************** */
uint8_t gbPhDta_SensFReq_RC;  /* Request Code, used in the SENS F Request command */

/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
/* Poll Mode Initialization API */
static void phDta_PollModeInit (void);

/* Poll Mode Cleanup API */
static void phDta_PollModeCleanUp();

/* Poll Mode NFC-A Test Module */
static void phDta_PollModeNfcA (phStatus_t dwDiscLoopStatus, uint8_t bTechType);

/* Poll Mode NFC-B Test Module */
static void phDta_PollModeNfcB (void);

/* Poll Mode NFC-F Test Module */
static void phDta_PollModeNfcF (phStatus_t dwDiscLoopStatus, uint8_t bTechType);

/* Poll Mode NFC-V Test Module */
static void phDta_PollModeNfcV (phStatus_t dwDiscLoopStatus);

/* API to Check if detected device is Type 2 Tag device */
static bool phDta_T2T_detected (void);

/* NFC A T2T Deactivate API */
static void phDta_T2T_deactivate (void);

/* API to Check if detected device is Type 4A device */
static bool phDta_Type4A_detected (void);

/* NFC Type 4 Platform Deactivate API */
static void phDta_Type4Platform_deactivate (void);

/* API to Check if detected device is T3T device */
static bool phDta_T3T_detected (uint8_t bTotalNofTagsFound);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
/* DTA Poll Mode Test Module API */
void phDta_PollMode (void)
{
    phStatus_t dwStatus;
    uint16_t wTagsDetected;

    do
    {
        /* Run the Poll Mode Procedure Initialization routine */
        phDta_PollModeInit();

        /* Run the discovery Loop */
        dwStatus = phacDiscLoop_Run(pDiscLoop, PHAC_DISCLOOP_ENTRY_POINT_POLL);

        if (((dwStatus & PH_ERR_MASK) == PHAC_DISCLOOP_DEVICE_ACTIVATED) ||
            ((dwStatus & PH_ERR_MASK) == PHAC_DISCLOOP_PASSIVE_TARGET_ACTIVATED)||
            ((dwStatus & PH_ERR_MASK) == PHAC_DISCLOOP_MERGED_SEL_RES_FOUND) ||
            ((dwStatus & PH_ERR_MASK) == PHAC_DISCLOOP_MULTI_DEVICES_RESOLVED))
        {
            /* Get information of the Tag Types detected */
            phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTagsDetected);

            /* Check if NFC-A, NFC-B, NFC-F or NFC-V device was detected */
            if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_A))
            {
                /* Run the NFC-A DTA Test Module */
                phDta_PollModeNfcA(dwStatus, PHAC_DISCLOOP_POS_BIT_MASK_A);
            }
            else if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_B))
            {
                /* Run the NFC-B DTA Test Module */
                phDta_PollModeNfcB();
            }
            else if ((PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_F212) ||
                     (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_F424))))
            {
                /* Run the NFC-F DTA Test Module */
                phDta_PollModeNfcF(dwStatus,
                    (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_F212) ?
                    PHAC_DISCLOOP_POS_BIT_MASK_F212 : PHAC_DISCLOOP_POS_BIT_MASK_F424));
            }
            else if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_V))
            {
                /* Run the NFC-V DTA Test Module */
                phDta_PollModeNfcV(dwStatus);
            }
            else
            {
                /* Device with Unexpected Technology detected */
                DEBUG_PRINTF("\nphDta_PollMode: Unexpected Tech Detected!");
            }
        }

        /* Poll Mode Cleanup */
        phDta_PollModeCleanUp();

    }while(0);
}

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */
/* Poll Mode Initialization API */
static void phDta_PollModeInit (void)
{
    uint16_t wpollTechMask;
    uint8_t bConf_Poll_B;
    uint8_t bConf_Poll_F_Bitrate;

    /* Enable/Disable Polling for Type B */
    if(gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_7)
    {
        /* Disable Polling for Type B */
        bConf_Poll_B = 0;
    }
    else
    {
        /* Enable Polling for Type B */
        bConf_Poll_B = 1;
    }

    /* Enable Polling for Type F 212/424kbps */
    if((gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_5) ||
       (gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_6) ||
       (gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_8) ||
       (gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_A))
    {
        /* Enable Polling for Type F 424kbps */
        bConf_Poll_F_Bitrate = 1U;
    }
    else
    {
        /* Enable Polling for Type F 212kbps */
        bConf_Poll_F_Bitrate = 0;
    }

    /* Configure the technologies to poll */
    wpollTechMask = (PHDTA_CON_POLL_A_MASK |
                     PHDTA_CON_POLL_B_MASK(bConf_Poll_B) |
                     PHDTA_CON_POLL_F_212_MASK(bConf_Poll_F_Bitrate) |
                     PHDTA_CON_POLL_F_424_MASK(bConf_Poll_F_Bitrate) |
                     PHDTA_CON_POLL_V_MASK);
    phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_POLL_TECH_CFG, wpollTechMask);

    /* Determine SENSF Request parameters */
    if ((gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_A) || (gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_B))
    {
        gbPhDta_SensFReq_RC = 0x1U;
    }
    else
    {
        gbPhDta_SensFReq_RC = 0x0;
    }

    /* Set the discovery loop state to start with PHAC_DISCLOOP_POLL_STATE_DETECTION while in poll mode. */
    phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);

    /* Reset the DTA Application Memory Management Mechanism */
    phDta_InitAppMemory();

    /* Update Global variable which indicates the protocol supported by the device detected in Poll mode Tests */
    gbPhDta_Int_Protocol = PHDTA_PROTOCOL_DETECTED_NONE;
}

/* Poll Mode Cleanup API */
static void phDta_PollModeCleanUp (void)
{
    phStatus_t dwRetStatus;

    do
    {
        /* Switch off the RF for time period PHDTA_TFIELD_OFF */
        dwRetStatus = phhalHw_FieldOff(pHal);
        if(dwRetStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_PollModeCleanUp: RF Field Off Failed!");
            break;
        }

        /* APS for TypeA */
        dwRetStatus = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);
        if(dwRetStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_PollModeCleanUp: RF Field Off Failed!");
            break;
        }

        /* Wait for period of time PHDTA_TFIELD_OFF */
        phOsal_ThreadDelay((uint32_t)PHDTA_TFIELD_OFF_TICKS);

        /* If ISO-DEP device was detected, Reset PAL I14443-4 component */
        if(gbPhDta_Int_Protocol == PHDTA_PROTOCOL_DETECTED_ISO_DEP)
        {
            /* For Type 4 Tag operation tests, reset of phalTop is needed to clear the "current selected file" flag */
            phalTop_Reset(palTop);

            /* Reset PAL I14443-4 component */
            phpalI14443p4_ResetProtocol(ppalI14443p4);
        }
    }while(0);
}

/* Poll Mode NFC-A Test Module */
static void phDta_PollModeNfcA (phStatus_t dwDiscLoopStatus, uint8_t bTechType)
{
    /* Check if NFC A T2T device was detected */
    if(phDta_T2T_detected())
    {
        /* Set the Tag type to T2T at the Tag AL */
        phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T2T_TAG);

        /* Run the T2T Test Module */
        phDta_T2T();

        /* Deactivate the Type 2 Tag by sending SLP_REQ */
        phDta_T2T_deactivate();
    }
    /* Check if NFC A T4T device was detected */
    else if(phDta_Type4A_detected())
    {
        /* Update Global flag to indicate that device with support for ISO-DEP was detected */
        gbPhDta_Int_Protocol = PHDTA_PROTOCOL_DETECTED_ISO_DEP;

        /* Run the T4T Test Module */
        phDta_T4T();

        /* Deactivate the Type 4A device */
        phDta_Type4Platform_deactivate();
    }
    else
    {
        /* Do Nothing */
    }
}

/* API to Check if detected device is Type 2 Tag device */
static bool phDta_T2T_detected (void)
{
    bool dwT2T_detected = FALSE;
    uint8_t bSelResponse = pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].aSak;

    /* Bit 2 (counting from 0) of SEL_REQ response must be 0 */
    if ((bSelResponse & (1U << 2U)) == 0)
    {
        /* Bits 6 and 5, should be 00b */
        if (((bSelResponse & 0x60U) >> 5U) == PHAC_DISCLOOP_TYPEA_TYPE2_TAG_CONFIG_MASK)
        {
            dwT2T_detected = TRUE;
        }
    }

    return dwT2T_detected;
}

/* NFC A T2T Deactivate API */
static void phDta_T2T_deactivate (void)
{
    phStatus_t wStatus;

    /* Send SLP_REQ */
    wStatus = phpalI14443p3a_HaltA(ppalI14443p3a);
    if(wStatus != PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_ISO14443P3A))
    {
        DEBUG_PRINTF("\nphDta_T2T_deactivate: sending SLP_REQ Failed!");
    }
}

/* API to Check if detected device is Type 4A device */
static bool phDta_Type4A_detected (void)
{
    phStatus_t wStatus;
    bool dwT4T_detected = FALSE;
    uint8_t bSelResponse = pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].aSak;
    uint8_t bCidEnable;
    uint8_t bCid;
    uint8_t bNadSupported;
    uint8_t bFwi;
    uint8_t bFsdi;
    uint8_t bFsci;
    uint8_t bSelResponse_B6_B5;

    /* Bit 2 (counting from 0) of SEL_REQ response must be 0 */
    if ((bSelResponse & (1U << 2U)) == 0)
    {
        /* Bits 6 and 5, should be 01b or 11b.
         * In this design, T4AT_NFC-DEP_PRIORITY is 0. So if device is configured for both NFC-DEP Protocol and Type 4A Tag Platform,
         * treat it as T4T Platform */
        bSelResponse_B6_B5 = ((bSelResponse & PHDTA_SEL_RESPONSE_BITMASK_B6_B5) >> 5U);
        if ((bSelResponse_B6_B5 == PHAC_DISCLOOP_TYPEA_TYPE4A_TAG_CONFIG_MASK) ||
           (bSelResponse_B6_B5 == PHAC_DISCLOOP_TYPEA_TYPE_NFC_DEP_TYPE4A_TAG_CONFIG_MASK))
        {
            dwT4T_detected = TRUE;

            /* Disable CID, NAD */
            wStatus = phpalI14443p4a_GetProtocolParams(
                    ppalI14443p4a,
                    &bCidEnable,
                    &bCid,
                    &bNadSupported,
                    &bFwi,
                    &bFsdi,
                    &bFsci);
            if(wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_Type4A_detected: Get Protocol Param Failed!");
            }
            else
            {
                wStatus = phpalI14443p4_SetProtocol(
                        ppalI14443p4,
                        0,
                        0,
                        0,
                        0,
                        bFwi,
                        bFsdi,
                        bFsci);
                if(wStatus != PH_ERR_SUCCESS)
                {
                    DEBUG_PRINTF("\nphDta_Type4A_detected: Set Protocol Param Failed!");
                }
            }

            /* Set Configuration for Maximum retry count */
            wStatus = phpalI14443p4_SetConfig(ppalI14443p4, PHPAL_I14443P4_CONFIG_MAXRETRYCOUNT, 2U);
            if(wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_Type4A_detected: Set Config Failed!");
            }

            /* If device was configure for both NFC DEP and T4T, then RATS must be sent. */
            if(bSelResponse_B6_B5 == PHAC_DISCLOOP_TYPEA_TYPE_NFC_DEP_TYPE4A_TAG_CONFIG_MASK)
            {
                /* Enable Emd Check in NFC mode */
                (void)phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_SET_EMD, PH_ON);

                wStatus = phpalI14443p4a_Rats(ppalI14443p4a, bFsdi, 0, (uint8_t *)gPhDta_ActResponse);
                if(wStatus != PH_ERR_SUCCESS)
                {
                    DEBUG_PRINTF("\nphDta_Type4A_detected: Send RATS Fail!");
                }
            }

        }
    }

    return dwT4T_detected;
}

/* NFC Type 4 Platform Deactivate API */
static void phDta_Type4Platform_deactivate (void)
{
    phStatus_t wStatus;

    /* Deactivate the NFC Type 4 Platform (Type 4A or 4B ) device */
    wStatus = phpalI14443p4_Deselect(ppalI14443p4);

    if((wStatus & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_Type4Platform_deactivate: De-Select Failed!");
        /* Perform RF Reset */
        phhalHw_FieldReset(pHal);
    }
}

/* Poll Mode NFC-B Test Module */
static void phDta_PollModeNfcB (void)
{
    phStatus_t wStatus;
    uint8_t bCidEnable;
    uint8_t bCid;
    uint8_t bNadSupported;
    uint8_t bFwi;
    uint8_t bFsdi;
    uint8_t bFsci;

    /* Update Global flag to indicate that device with support for ISO-DEP was detected */
    gbPhDta_Int_Protocol = PHDTA_PROTOCOL_DETECTED_ISO_DEP;

    /* Disable CID, NAD */
    wStatus = phpalI14443p3b_GetProtocolParams(
            ppalI14443p3b,
            &bCidEnable,
            &bCid,
            &bNadSupported,
            &bFwi,
            &bFsdi,
            &bFsci);
    if(wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_T4T_detected: Get Protocol Param Failed!");
    }
    else
    {
        wStatus = phpalI14443p4_SetProtocol(
                ppalI14443p4,
                0,
                0,
                0,
                0,
                bFwi,
                bFsdi,
                bFsci);
        if(wStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_T4T_detected: Set Protocol Param Failed!");
        }
    }

    /* Set Configuration for Maximum retry count */
    wStatus = phpalI14443p4_SetConfig(ppalI14443p4, PHPAL_I14443P4_CONFIG_MAXRETRYCOUNT, 2U);
    if(wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_T4T_detected: Set Config Failed!");
    }

    /* Run the T4T Test Module */
    phDta_T4T();

    /* Deactivate the Type 4B device */
    phDta_Type4Platform_deactivate();
}

/* Poll Mode NFC-F Test Module */
static void phDta_PollModeNfcF (phStatus_t dwDiscLoopStatus, uint8_t bTechType)
{
    uint16_t bTotalNofTagsFound = 0;
    bool T3T_detected = FALSE;

    phacDiscLoop_Sw_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEF_TAGS_FOUND, &bTotalNofTagsFound);

    T3T_detected = phDta_T3T_detected(bTotalNofTagsFound);

    /* Check if NFC F device with support for T3T is detected and proceed further with T3T Tag Operation. */
    if (T3T_detected == TRUE)
    {
        /* Run the T3T Test Module */
        phDta_T3T();
    }
    else
    {
        /* Do Nothing */
    }
}

/* API to Check if detected device is Type F T3T device */
static bool phDta_T3T_detected (uint8_t bTotalNofTagsFound)
{
    bool T3T_detected = FALSE;
    uint8_t bIndex;

    for (bIndex = 0; bIndex < bTotalNofTagsFound; bIndex++)
    {
        /* check if Bytes 1-2 of NFCID2 is 0x02FE */
        if ( (pDiscLoop->sTypeFTargetInfo.aTypeFTag[bIndex].aIDmPMm[0] == 0x02U) &&
             (pDiscLoop->sTypeFTargetInfo.aTypeFTag[bIndex].aIDmPMm[1] == 0xFEU) )
        {
            T3T_detected = TRUE;
            break;
        }
        else
        {
            T3T_detected = FALSE;
        }
    }

    return T3T_detected;
}

/* Poll Mode NFC-V Test Module */
static void phDta_PollModeNfcV (phStatus_t dwDiscLoopStatus)
{
    /* Run the T5T Test Module */
    phDta_T5T(dwDiscLoopStatus);
}

#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */
