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
 * This file contains the DTA Listen Mode Test Procedure Module.
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

#if defined(NXPBUILD_DTA_CE_DEVICE_CLASS) && !defined(NXPBUILD__PHHAL_HW_RC663)
/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */
#define PHDTA_CON_LISTEN_T4ATP   (1U) /* Flag to enable Listening for NFC-A Technology with Type 4 Tag Platform support */

#define PHDTA_RATS_BYTE1  (0xE0U)

/* Time period, in ticks, to wait for RF Field detection in Listen mode test module.
 * Affects IXIT. TLM = 500 x 1ms = 500ms */
#define PHDTA_TLM_TICKS   (500U)

/* Time period, in ticks, for which the DTA Application will wait in "NO_REMOTE_FIELD".
 * See "tRLM" in DTA Spec section 4.4.1.7.
 * Affects IXIT. tRLM = 6 x 1ms = 6ms */
#define PHDTA_NO_REMOTE_FIELD_TRLM    (6U)

#define PHDTA_TICK_DURATION   (1U) /* 1 OS Tick is 1ms in duration */

/* *****************************************************************************************************************
 * Global and Static Variables
 * Total Size: NNNbytes
 * ***************************************************************************************************************** */
/* Flag to indicate if RF field was detected in the Listen Mode Test run */
static uint8_t gbPhDta_RFfieldDetected;

/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
/* DTA Listen Mode Initialization API */
static void phDta_ListenModeInit (void);

/* For some DTA test cases(eg: TC_LIS_NFCA_UND_BV_05_x_y), the listen Mode discovery loop process should be run multiple times.
 * This API performs this extended Listen Mode Test procedure */
static void phDta_ListenModeExtendedProcess (void);

/* API to check for presence of external RF Field */
static phStatus_t phDta_RFdetect (uint32_t waitTimeTicks);

/* DTA Listen Mode Cleanup API */
static void phDta_ListenModeCleanUp (void);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
/* DTA Listen Mode Test Module API*/
void phDta_ListenMode (void)
{
    phStatus_t wStatus;
    bool dwActivatedByPeer = FALSE;

    /* Run Listen Mode Initialization Module */
    phDta_ListenModeInit();

    /* Run Discovery Loop in Listen Mode */
    wStatus = phacDiscLoop_Run(pDiscLoop, PHAC_DISCLOOP_ENTRY_POINT_LISTEN);

    if((wStatus & PH_ERR_MASK) == PHAC_DISCLOOP_ACTIVATED_BY_PEER)
    {
        dwActivatedByPeer = TRUE;
    }
    else if((wStatus & PH_ERR_MASK) == PHAC_DISCLOOP_EXTERNAL_RFOFF)
    {
        /* Reset SLEEP_AF state (Respond to both WupA ("52") and ReqA ("26")) as Field Off is observed.*/
        wStatus = phhalHw_SetConfig (pHal, PHHAL_HW_CONFIG_MFHALTED, PH_OFF);
        CHECK_STATUS(wStatus);

        /* External RF is not available. Wait for PHDTA_TLM_TICKS timeout till an external RF is detected. */
        wStatus = phDta_RFdetect(PHDTA_TLM_TICKS);

        if(wStatus == PH_ERR_SUCCESS)
        {
            /* External RF is available. Run Discovery loop */
            wStatus = phacDiscLoop_Run(pDiscLoop, PHAC_DISCLOOP_ENTRY_POINT_LISTEN);
            if((wStatus & PH_ERR_MASK) == PHAC_DISCLOOP_ACTIVATED_BY_PEER)
            {
                dwActivatedByPeer = TRUE;
            }
        }
    }

    if(dwActivatedByPeer)
    {
        {
            /* Check if RATS command is received */
            if ((PHDTA_CON_LISTEN_T4ATP) &&
                (pDiscLoop->sTargetParams.pRxBuffer[0] == PHDTA_RATS_BYTE1))
            {
                /* Update Global flag to indicate that device with support for ISO-DEP was detected */
                gbPhDta_Int_Protocol = PHDTA_PROTOCOL_DETECTED_ISO_DEP;

                /* Run Listen Mode T4T Test */
                phDta_ListenT4TLoopBack();
            }
        }
    }

    /* Run Listen Mode CleanUp Module */
    phDta_ListenModeCleanUp();
}

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */
/* DTA Listen Mode Initialization API */
static void phDta_ListenModeInit (void)
{
    uint16_t wListTechMask;

    /* Configure the technologies to Listen for */
    wListTechMask = PHAC_DISCLOOP_POS_BIT_MASK_A;

    phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_PAS_LIS_TECH_CFG, wListTechMask);

    /* Reset the DTA Application Memory Management Mechanism */
    phDta_InitAppMemory();

    /* Update Global variable which indicates the protocol used by the reader device in Listen mode Tests */
    gbPhDta_Int_Protocol = PHDTA_PROTOCOL_DETECTED_NONE;
}

/* API to check for presence of external RF Field */
static phStatus_t phDta_RFdetect (uint32_t waitTimeTicks)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    wStatus = phhalHw_EventConsume(pHal);

    wStatus = phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_RFON_INTERRUPT, PH_ON);
    if(wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_ListenMode: Enabling RF On Interrupt failed");
    }

    wStatus = phhalHw_EventWait(pHal, (waitTimeTicks * PHDTA_TICK_DURATION));
    if(wStatus == PH_ERR_SUCCESS)
    {
        gbPhDta_RFfieldDetected = 1; /* Update global flag to indicate that external RF Field was detected */
    }
    else
    {
        gbPhDta_RFfieldDetected = 0;
        phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_RFON_INTERRUPT, PH_OFF);
    }

    return wStatus;
}


/* For some DTA test cases(eg: TC_LIS_NFCA_UND_BV_05_x_y), the listen Mode discovery loop process should be run multiple times.
 * This API performs this extended Listen Mode Test procedure */
static void phDta_ListenModeExtendedProcess (void)
{
    /* If IUT emulated ISO-DEP device (HCE), Reset PAL I14443-4mC component component */
    if(gbPhDta_Int_Protocol == PHDTA_PROTOCOL_DETECTED_ISO_DEP)
    {
        phpalI14443p4mC_ResetProtocol(ppalI14443p4mC);
    }

    /* Check if RATS command is received */
    if ((PHDTA_CON_LISTEN_T4ATP) &&
        (pDiscLoop->sTargetParams.pRxBuffer[0] == PHDTA_RATS_BYTE1))
    {
        /* Update Global flag to indicate that device with support for ISO-DEP was detected */
        gbPhDta_Int_Protocol = PHDTA_PROTOCOL_DETECTED_ISO_DEP;

        /* Run Listen Mode T4T Test */
        phDta_ListenT4TLoopBack();

        /* For TC_LIS_NFCA_UND_BV_05_0_1, Discovery loop must be run 3 times before Poll profile runs in the DTA main loop.
         * This is the third call to discovery loop  */
        /* Run Discovery Loop in Listen Mode */
        phacDiscLoop_Run(pDiscLoop, PHAC_DISCLOOP_ENTRY_POINT_LISTEN);
    }
}


/* DTA Listen Mode Cleanup API */
static void phDta_ListenModeCleanUp (void)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    bool dwActivatedByPeer = FALSE;

    do
    {
        if(gbPhDta_RFfieldDetected)
        {
            /* section 4.4.1.7, DTA Spec */
            /* The IUT will have to remain in NO_REMOTE_FIELD for at least time period PHDTA_NO_REMOTE_FIELD_TRLM */
            /* Check if External RF Field is present */

            /* Run Discovery Loop in Listen Mode */
            wStatus = phacDiscLoop_Run(pDiscLoop, PHAC_DISCLOOP_ENTRY_POINT_LISTEN);
            if((wStatus & PH_ERR_MASK) == PHAC_DISCLOOP_ACTIVATED_BY_PEER)
            {
                dwActivatedByPeer = TRUE;
            }
            else if((wStatus & PH_ERR_MASK) == PHAC_DISCLOOP_EXTERNAL_RFOFF)
            {
                /* Reset SLEEP_AF state (Respond to both WupA ("52") and ReqA ("26")) as Field Off is observed.*/
                wStatus = phhalHw_SetConfig (pHal, PHHAL_HW_CONFIG_MFHALTED, PH_OFF);
                CHECK_STATUS(wStatus);

                /* External RF is not available. Wait for PHDTA_NO_REMOTE_FIELD_TRLM timeout till an external RF is detected. */
                wStatus = phDta_RFdetect(PHDTA_NO_REMOTE_FIELD_TRLM);

                if(wStatus == PH_ERR_SUCCESS)
                {
                    /* External RF is available. Run Discovery loop */
                    wStatus = phacDiscLoop_Run(pDiscLoop, PHAC_DISCLOOP_ENTRY_POINT_LISTEN);
                    if((wStatus & PH_ERR_MASK) == PHAC_DISCLOOP_ACTIVATED_BY_PEER)
                    {
                        dwActivatedByPeer = TRUE;
                    }
                }
            }
            if(dwActivatedByPeer)
            {
                phDta_ListenModeExtendedProcess();
            }
        }

        /* If IUT emulated ISO-DEP device (HCE), Reset PAL I14443-4mC component component */
        if(gbPhDta_Int_Protocol == PHDTA_PROTOCOL_DETECTED_ISO_DEP)
        {
            phpalI14443p4mC_ResetProtocol(ppalI14443p4mC);
        }
    }while(0);
}

#endif /* defined(NXPBUILD_DTA_CE_DEVICE_CLASS) && !defined(NXPBUILD__PHHAL_HW_RC663) */
