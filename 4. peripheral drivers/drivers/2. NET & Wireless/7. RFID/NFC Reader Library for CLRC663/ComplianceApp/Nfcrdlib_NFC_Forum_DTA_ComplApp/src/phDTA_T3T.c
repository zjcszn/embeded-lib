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
 * This file contains the T3T Test Module of the DTA Test application.
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
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
/* T3T NDEF Read Test API */
static void phDta_T3TRead (void);

/* T3T NDEF Write Test API */
static void phDta_T3TWrite (void);

/* T3T Transition from READ-WRITE to READ-ONLY functionality API */
static void phDta_T3T_Transition (void);

/* T3T Service Independent Application Test API */
static void phDta_T3TServiceIndependentApplication (void);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
/* Top level DTA T3T Test Module API */
void phDta_T3T (void)
{
    /* Based on the Pattern Number, run DTA T3T Test sub Modules */
    switch(gwPhDta_PatternNumber)
    {
        case (PHDTA_PATTERN_NUM_1):
        case (PHDTA_PATTERN_NUM_5):
        case (PHDTA_PATTERN_NUM_6):
        case (PHDTA_PATTERN_NUM_7):
        case (PHDTA_PATTERN_NUM_8):
        {
            /* Reset the DTA Application Memory Management Mechanism */
            phDta_InitAppMemory();

            /* Run T3T Ndef Read Test Functionality */
            phDta_T3TRead();
        }
        break;

        case (PHDTA_PATTERN_NUM_2):
        {
            /* Run T3T Ndef Write Test Functionality */
            phDta_T3TWrite();
        }
        break;

        case (PHDTA_PATTERN_NUM_3):
        {
            /* Run T3T Transition from READ-WRITE to READ-ONLY functionality */
            phDta_T3T_Transition();
        }
        break;

        case (PHDTA_PATTERN_NUM_0):
        case (PHDTA_PATTERN_NUM_A):
        case (PHDTA_PATTERN_NUM_B):
        {
            /* Run T3T Service Independent Application functionality */
            phDta_T3TServiceIndependentApplication();
        }
        break;

        default:
            /* Do Nothing! */
        break;
    }
}

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */
/* T3T NDEF Read Test API */
static void phDta_T3TRead (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;
    uint16_t wTemp_RC;

    /* Set the Tag type to T3T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T3T_TAG);

    /* Get the currently configured Request Code for SENSF_REQ */
    phpalFelica_GetConfig(ppalFelica, PHPAL_FELICA_CONFIG_RC, &wTemp_RC);

    /* Set the Request Code that will be sent in the SENSF_REQ */
    phpalFelica_SetConfig(ppalFelica, PHPAL_FELICA_CONFIG_RC, gbPhDta_SensFReq_RC);

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);

    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        /* Allocate Memory for the Tag Read Operation */
        wStatus = phDta_AllocateMemory (palTop->dwNdefLength, &pgPhDta_TagTest_NdefData);

        if(wStatus == PH_ERR_SUCCESS)
        {
            /* Attempt to read the entire NDEF message from the Tag */
            wStatus = phalTop_ReadNdef(palTop, pgPhDta_TagTest_NdefData,&dwgPhDta_TagTest_NdefDataLength);

            if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
            {
                DEBUG_PRINTF("\nphDta_T3TRead: Read Tag Successfully.");
            }
            else
            {
                DEBUG_PRINTF("\nphDta_T3TRead: Read Tag UnSuccessful.");
            }
        }
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T3TRead: Not NDEF compliant.");
    }

    /* Restore the previous value for the Request Code that will be sent in the SENSF_REQ */
    phpalFelica_SetConfig(ppalFelica, PHPAL_FELICA_CONFIG_RC, wTemp_RC);
}

/* T3T NDEF Write Test API */
static void phDta_T3TWrite (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;

    /* Set the Tag type to T3T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T3T_TAG);

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);

    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        /* Attempt to write the entire NDEF message to the Tag */
        wStatus = phalTop_WriteNdef(palTop, pgPhDta_TagTest_NdefData, dwgPhDta_TagTest_NdefDataLength);

        if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
        {
            DEBUG_PRINTF("\nphDta_T3TWrite: Write Tag Successfully.");
        }
        else
        {
            DEBUG_PRINTF("\nphDta_T3TWrite: Write Tag UnSuccessful.");
        }
     }
    else
    {
        DEBUG_PRINTF("\nphDta_T3TWrite: Not NDEF compliant.");
    }
}


/* T3T Service Independent Application Test API */
static void phDta_T3TServiceIndependentApplication (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;

    /* Set the Tag type to T3T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T3T_TAG);

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);

    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        DEBUG_PRINTF("\nphDta_T3TWrite:  NDEF compliant.");
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T3TWrite: Not NDEF compliant.");
    }
}

/* T3T Transition from READ-WRITE to READ-ONLY functionality API */
static void phDta_T3T_Transition (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;

    /* Set the Tag type to T3T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T3T_TAG);

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);

    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        DEBUG_PRINTF("\nphDta_T3TWrite:  NDEF compliant.");
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T3TWrite: Not NDEF compliant.");
    }

    /* Attempt to Change the T3T Tag to Read Only  */
    wStatus = phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_STATE, PHAL_TOP_STATE_READONLY);

    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        DEBUG_PRINTF("\nphDta_T3TWrite: RO Transition Tag Op Successful.");
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T3TWrite: RO Transition Tag Op UnSuccessful!");
    }
}

#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */
