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
 * This file contains the T2T Test Module of the DTA Test application.
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
#define PHDTA_POLLMODE_T2T_READTEST_BUFFERSIZE        (2030U)  /* 2030 bytes memory allocated for the T2T Read tests */

/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
/* DTA T2T Read Test Functionality API */
static void phDta_T2TRead (void);

/* DTA T2T Write Test Functionality API */
static void phDta_T2TWrite (void);

/* DTA T2T Read Only Transition Test Functionality API */
static void phDta_T2TTrans (void);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
/* Top level DTA T2T Test Module API */
void phDta_T2T (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);
    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        /* Based on the Pattern Number, run DTA T2T Test sub Modules */
        switch(gwPhDta_PatternNumber)
        {
            case (PHDTA_PATTERN_NUM_1):
            {
                /* Run T2T Read Test Functionality */
                phDta_T2TRead();
            }
            break;

            case (PHDTA_PATTERN_NUM_2):
            {
                /* Run T2T Write Test Functionality */
                phDta_T2TWrite();
            }
            break;

            case (PHDTA_PATTERN_NUM_3):
            {
                /* Run T2T Read Only Transition Test Functionality */
                phDta_T2TTrans();
            }
            break;

            default:
                /* Do Nothing! */
                break;
        }
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T2T: Tag is not NDEF compliant.");
    }
}

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */
/* DTA T2T Read Test Functionality API */
static void phDta_T2TRead (void)
{
    phStatus_t wStatus;

    /* Reset NDEF Data variables at the starting of the Read Operation */
    pgPhDta_TagTest_NdefData = NULL;
    dwgPhDta_TagTest_NdefDataLength = 0;

    /* Allocate Memory for the Tag Read Operation */
    wStatus = phDta_AllocateMemory (PHDTA_POLLMODE_T2T_READTEST_BUFFERSIZE, &pgPhDta_TagTest_NdefData);

    if(wStatus == PH_ERR_SUCCESS)
    {
        /* Attempt to read the entire NDEF message from the Tag */
        wStatus = phalTop_ReadNdef(palTop, pgPhDta_TagTest_NdefData,&dwgPhDta_TagTest_NdefDataLength);

        if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
        {
            DEBUG_PRINTF("\nphDta_T2TRead: Read Tag Successfully.");
        }
        else
        {
            DEBUG_PRINTF("\nphDta_T2TRead: Read Tag UnSuccessful.");
        }
    }
}

/* DTA T2T Write Test Functionality API */
static void phDta_T2TWrite (void)
{
    phStatus_t wStatus;

    do
    {
        /* Check that Write Test Data was populated */
        if ((pgPhDta_TagTest_NdefData != NULL) && (dwgPhDta_TagTest_NdefDataLength != 0))
        {
            /* Attempt to write the entire NDEF message that was read earlier, to the Tag */
            wStatus = phalTop_WriteNdef(palTop, pgPhDta_TagTest_NdefData,dwgPhDta_TagTest_NdefDataLength);

            if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
            {
                DEBUG_PRINTF("\nphDta_T2TWrite: Write Tag Successful.");
            }
            else
            {
                DEBUG_PRINTF("\nphDta_T2TWrite: Write Tag UnSuccessful.");
            }
        }
        else
        {
            DEBUG_PRINTF("\nphDta_T2TWrite: Run the appropriate T2T Read Test before running this T2T Write Test!");
        }
    }while(0);
}

/* DTA T2T Read Only Transition Test Functionality API */
static void phDta_T2TTrans (void)
{
    phStatus_t wStatus;

    /* Attempt to Change the T2T Tag to Read Only NDEF Tag layout */
    wStatus = phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_STATE, PHAL_TOP_STATE_READONLY);

    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        DEBUG_PRINTF("\nphDta_T2TRead: RO Transition Tag Op Successful.");
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T2TRead: RO Transition Tag Op UnSuccessful!");
    }
}

#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */
