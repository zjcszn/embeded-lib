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
 * This file contains resources like global variables and buffers that the DTA Test application uses.
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

/* *****************************************************************************************************************
 * Global and Static Variables
 * ***************************************************************************************************************** */
/* Global variable to hold the DTA test Pattern Number */
uint16_t gwPhDta_PatternNumber;

/* Global variable to save the code corresponding to the Protocol supported by the device detected. */
uint8_t gbPhDta_Int_Protocol;

/* DTA Application Memory Management Control Variables */
#ifdef PHDRIVER_LPC1769
    /* For LPC1769 platform, RAM2(32KBytes) is used for DTA application memory. */
    USE_RAM2_32 static uint8_t gaPhDta_AppBuffer[PHDTA_APP_BUFFER_SIZE];
#else
    static uint8_t gaPhDta_AppBuffer[PHDTA_APP_BUFFER_SIZE];
#endif /* PHDRIVER_LPC1769 */

static uint8_t *gpPhDta_AppBufferAvlPtr;
static uint32_t dwgPhDta_AppUsedBufferSize;

#ifdef NXPBUILD_DTA_READER_DEVICE_CLASS
/* Buffer to hold the response to Activation request(i.e. ATS) */
uint8_t gPhDta_ActResponse[64U];

/* Digital Protocol Tag Test Variables */
uint8_t *pgPhDta_TagTest_NdefData;
uint32_t dwgPhDta_TagTest_NdefDataLength;
#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */

#ifdef NXPBUILD__PHHAL_HW_TARGET
/* SENS Response */
const uint8_t aSensRes[2U]  = { 0x04, 0x00 };

/* user defined bytes of the UID needed for anti-collision */
const uint8_t aNfcId1[3U]   = { 0xA1, 0xA2, 0xA3 };

/* SAK (ISO14443P4 Card) - needed for anti-collision */
const uint8_t bSelRes = 0x60U;

#ifdef NXPBUILD__PHHAL_HW_PN7462AU
/* Buffer containing bytes for the SENSF Response in Listen mode */
const uint8_t gphDta_SensFRes[] = { 0x12, 0x01, 0x01, 0x27, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
        0xFF, 0xFF, 0x00, 0x00, 0x02, 0x06, 0x03, 0x00 };

/* 2 byte system code for Felica */
const uint8_t gphDta_bSystemCode[2U] = {0xFF,0xFF};
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

#if defined(NXPBUILD__PHHAL_HW_PN5180) || defined(NXPBUILD__PHHAL_HW_PN5190) || defined(NXPBUILD__PHHAL_HW_PN76XX)
/* NFC3 byte - required for anti-collision */
const uint8_t  nfc_id3 = 0xFA;

const uint8_t  poll_res[18U] = { 0x01, 0xFE, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
        0xFF, 0xFF, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0x23, 0x45 };
#endif /* defined(NXPBUILD__PHHAL_HW_PN5180) || defined(NXPBUILD__PHHAL_HW_PN5190) || defined(NXPBUILD__PHHAL_HW_PN76XX) */

/* Answer to RATS in Listen Mode */
const uint8_t gaPhDta_AtsResp[5U]  = { 0x05, 0x72, 0x00, 0x88, 0x00 };  /* |TL|T0|TA|TB|TC| */
#endif /* NXPBUILD__PHHAL_HW_TARGET */

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
/* Initialize/Reset Memory Management mechanism of DTA Application */
void phDta_InitAppMemory (void)
{
    /* Reset Application memory Buffer pointers and variables */
    gpPhDta_AppBufferAvlPtr = &gaPhDta_AppBuffer[0];
    dwgPhDta_AppUsedBufferSize = 0;
}

/* Memory allocation API for DTA Application */
phStatus_t phDta_AllocateMemory (uint32_t dwReqByteSize, uint8_t **pBufferPointer)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;
    uint32_t dwAvailableSize;

    /* Calculate Available Buffer size */
    dwAvailableSize = PHDTA_APP_BUFFER_SIZE - dwgPhDta_AppUsedBufferSize;

    if(dwReqByteSize <= dwAvailableSize)
    {
        /* Update Buffer used memory size variable */
        dwgPhDta_AppUsedBufferSize += dwReqByteSize;

        /* Pointer to allocated block */
        *pBufferPointer = gpPhDta_AppBufferAvlPtr;

        /* Update pointer to next available block*/
        gpPhDta_AppBufferAvlPtr = &gaPhDta_AppBuffer[0] + dwgPhDta_AppUsedBufferSize;
    }
    else
    {
        wStatus = PH_ERR_RESOURCE_ERROR;
    }

    return wStatus;
}
