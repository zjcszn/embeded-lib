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
 * This file contains the T5T Test Module of the DTA Test application.
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
#include "phalTop_Sw_Int_T5T.h"

#ifdef NXPBUILD_DTA_READER_DEVICE_CLASS
/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */
#define PHDTA_POLLMODE_T5T_READTEST_BUFFERSIZE      (1050U)  /* 1050 bytes memory allocated for the T5T Read tests */

/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
/* DTA T5T Read Test Functionality API */
static void phDta_T5T_Read (phStatus_t dwDiscLoopStatus);

/* DTA T5T Write Test Functionality API */
static void phDta_T5T_Write (void);

/* DTA T5T Non Address mode Read Test Functionality API */
static void phDta_T5T_Read_NonAddressMode (phStatus_t dwDiscLoopStatus);

/* DTA T5T Non Select mode Read Test Functionality API */
static void phDta_T5T_Read_SelectMode (phStatus_t dwDiscLoopStatus);

/* DTA T5T Non Address mode Write Test Functionality API */
static void phDta_T5T_Write_NonAddressMode (void);

/* DTA T5T Transition from READ-WRITE to READ-ONLY functionality API */
static void phDta_T5T_Transition (void);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
/* Top level DTA T5T Test Module API */
void phDta_T5T (phStatus_t dwDiscLoopStatus)
{
    phStatus_t wStatus;
    uint16_t bFlags = 0;

    /* Based on the Pattern Number, run DTA T5T Test sub Modules */
    switch(gwPhDta_PatternNumber)
    {
        case (PHDTA_PATTERN_NUM_1):
        case (PHDTA_PATTERN_NUM_11):
        {
            /* Save previous Flags value */
            wStatus = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, &bFlags);
            if (wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_T5T: SetConfig Failed.");
                return;
            }

            /* Run T5T Non Address mode Read Test Functionality */
            phDta_T5T_Read_NonAddressMode(dwDiscLoopStatus);

            /* Restore the Flags value */
            wStatus = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, bFlags);
            if (wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_T5T: SetConfig Failed.");
                return;
            }
        }
        break;

        case (PHDTA_PATTERN_NUM_21):
        {
            /* Run T5T Addressed Mode Read Test Functionality */
            phDta_T5T_Read(dwDiscLoopStatus);
        }
        break;

        /* 2 devices resolved.NFC Forum Device SHALL at first put one tag to QUIET state, then the second tag SHALL be put to SELECTED state.
        perform the NDEF Read procedure for the tag in SELECTED state */
        case (PHDTA_PATTERN_NUM_31):
        {
            /* Save previous Flags value */
            wStatus = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, &bFlags);
            if (wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_T5T: SetConfig Failed.");
                return;
            }

            /* Run T5T Select mode Read Test Functionality */
            phDta_T5T_Read_SelectMode(dwDiscLoopStatus);

            /* Restore the Flags value */
            wStatus = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, bFlags);
            if (wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_T5T: SetConfig Failed.");
                return;
            }
        }
        break;

        case (PHDTA_PATTERN_NUM_12):
        {
            /* Run T5T Addressed Mode Read Test Functionality */
            phDta_T5T_Write();
        }
        break;

        case (PHDTA_PATTERN_NUM_2):
        {
            /* Save previous Flags value */
            wStatus = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, &bFlags);
            if (wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_T5T: SetConfig Failed.");
                return;
            }

            /* Run T5T Non Address mode Write Test Functionality */
            phDta_T5T_Write_NonAddressMode();

            /* Restore the Flags value */
            wStatus = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, bFlags);
            if (wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_T5T: SetConfig Failed.");
                return;
            }
        }
        break;

        case (PHDTA_PATTERN_NUM_3):
        {
            /* Run T5T Transition from READ-WRITE to READ-ONLY functionality */
            phDta_T5T_Transition();
        }
        break;

        default:
            /* Do Nothing! */
            break;
    }
}


/* DTA T5T Read Test Functionality API */
static void phDta_T5T_Read (phStatus_t dwDiscLoopStatus)
{
    phStatus_t wStatus;
    uint8_t bTagState;

    if ((dwDiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_MULTI_DEVICES_RESOLVED)
    {
        /* Set serial number (UID) */
        wStatus = phpalSli15693_SetSerialNo(pDiscLoop->pPalSli15693DataParams, pDiscLoop->sTypeVTargetInfo.aTypeV[0].aUid, 8U);
        if (wStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_T5TRead: Set Serial Number Failed.");
            return;
        }
    }

    /* Set the Tag type to T5T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T5T_TAG);

    /* Reset NDEF Data variables at the starting of the Read Operation */
    pgPhDta_TagTest_NdefData = NULL;
    dwgPhDta_TagTest_NdefDataLength = 0;

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);
    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        /* Allocate Memory for the Tag Read Operation */
        wStatus = phDta_AllocateMemory (PHDTA_POLLMODE_T5T_READTEST_BUFFERSIZE, &pgPhDta_TagTest_NdefData);
        if(wStatus == PH_ERR_SUCCESS)
        {
            /* Attempt to read the entire NDEF message from the Tag */
            wStatus = phalTop_ReadNdef(palTop, pgPhDta_TagTest_NdefData,&dwgPhDta_TagTest_NdefDataLength);

            if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
            {
                DEBUG_PRINTF("\nphDta_T5TRead: Read Tag Successfully.");
            }
            else
            {
                DEBUG_PRINTF("\nphDta_T5TRead: Read Tag UnSuccessful.");
            }
        }
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T5TRead: Not NDEF compliant.");
    }
}

/* DTA T5T Write Test Functionality API */
static void phDta_T5T_Write (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;

    /* Set the Tag type to T5T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T5T_TAG);

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);
    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        /* Check that Write Test Data was populated */
        if ((pgPhDta_TagTest_NdefData != NULL) && (dwgPhDta_TagTest_NdefDataLength != 0))
        {
            /* Attempt to write the entire NDEF message that was read earlier, to the Tag */
            wStatus = phalTop_WriteNdef(palTop, pgPhDta_TagTest_NdefData, dwgPhDta_TagTest_NdefDataLength);
            if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
            {
                DEBUG_PRINTF("\nphDta_T5TWrite: Write Tag Successful.");
            }
            else
            {
                DEBUG_PRINTF("\nphDta_T5TWrite: Write Tag UnSuccessful.");
            }
        }
        else
        {
            DEBUG_PRINTF("\nphDta_T5TWrite: Run the appropriate T5T Read Test before running this T5T Write Test!");
        }
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T5TWrite: Not NDEF compliant.");
    }
}

/* DTA T5T Non Address mode Read Test Functionality API */
static void phDta_T5T_Read_NonAddressMode (phStatus_t dwDiscLoopStatus)
{
    phStatus_t wStatus;

    /* Clear the Adress_Flag to use Non Address mode for T5T Tag operations  */
    wStatus = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, PHPAL_SLI15693_FLAG_NON_ADDRESSED);
    if (wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_T5T_Read_NonAddressMode: SetConfig Failed.");
        return;
    }

    /* Run T5T Read Test Functionality */
    phDta_T5T_Read(dwDiscLoopStatus);
}

/* DTA T5T Non Select mode Read Test Functionality API */
static void phDta_T5T_Read_SelectMode (phStatus_t dwDiscLoopStatus)
{
    phStatus_t wStatus;

    if ((dwDiscLoopStatus & PH_ERR_MASK) == PHAC_DISCLOOP_MULTI_DEVICES_RESOLVED)
    {
        /* Set serial number (UID) */
        wStatus = phpalSli15693_SetSerialNo(pDiscLoop->pPalSli15693DataParams, pDiscLoop->sTypeVTargetInfo.aTypeV[0].aUid, 8U);
        if (wStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_T5T_Read_SelectMode: Set Serial Number Failed.");
            return;
        }
    }

    /* Put the tag to SLEEP state */
    wStatus = phpalSli15693_StayQuiet(pDiscLoop->pPalSli15693DataParams);
    if (wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_T5T_Read_SelectMode: Stay quiet Failed.");
        return;
    }

    /* Set the Select_Flag for T5T Tag operations  */
    wStatus = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, PHPAL_SLI15693_FLAG_SELECTED);
    if (wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_T5T_Read_SelectMode: SetConfig Failed.");
        return;
    }

    /* Put the device in Select state, only VICC in selected state shall respond for T5T Tag operations*/
    wStatus = phacDiscLoop_ActivateCard(pDiscLoop, PHAC_DISCLOOP_TECH_TYPE_V, 1U);
    if ((wStatus & PH_ERR_MASK) != PHAC_DISCLOOP_DEVICE_ACTIVATED)
    {
        DEBUG_PRINTF("\nphDta_T5T_Read_SelectMode: Activate Card Failed.");
        return;
    }

    /* Run T5T Read Test Functionality */
    phDta_T5T_Read(dwDiscLoopStatus);
}

/* DTA T5T Non Address mode Write Test Functionality API */
static void phDta_T5T_Write_NonAddressMode (void)
{
    phStatus_t wStatus;

    /* Clear the Adress_Flag to use Non Address mode for T5T Tag operations  */
    wStatus = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_MODE, PHPAL_SLI15693_FLAG_NON_ADDRESSED);
    if (wStatus != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_T5T_Write_NonAddressMode: SetConfig Failed.");
        return;
    }

    /* Run T5T Write Test Functionality */
    phDta_T5T_Write();
}

/* DTA T5T Transition from READ-WRITE to READ-ONLY functionality API */
static void phDta_T5T_Transition (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;
    uint16_t wBlockNum;

    /* Set the Tag type to T5T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T5T_TAG);

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);
    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
        {
            /* Updating tag access conditions to READ-ONLY  */
            palTop->ualTop.salTop_T5T.bCcBlockData[1] = (palTop->ualTop.salTop_T5T.bCcBlockData[1] | 0x03);

            /* Update Block-0 CC block data with READ-ONLY access conditions */
            wStatus = phalTop_Sw_Int_T5T_Write(&(palTop->ualTop.salTop_T5T), 0, palTop->ualTop.salTop_T5T.bCcBlockData, palTop->ualTop.salTop_T5T.bBlockSize);
            if(wStatus != PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
            {
                DEBUG_PRINTF("\nphDta_T5T_Transition: Write CC Block Failed!.");
            }

            for(wBlockNum = 0; wBlockNum < palTop->ualTop.salTop_T5T.bMaxBlockNum; wBlockNum++)
            {
                /* Lock all the block of tag */
                wStatus = phalTop_LockBlock(palTop, wBlockNum);
                if(wStatus != PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
                {
                    DEBUG_PRINTF("\nphDta_T5T_Transition: LOCK Block = %d Failed!", wBlockNum);
                }
            }
        }
        else
        {
            DEBUG_PRINTF("\nphDta_T5T_Transition: Not NDEF compliant.");
        }
    }
}

#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */
