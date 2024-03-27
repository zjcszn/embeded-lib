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
 * This file contains the T4T Test Module of the DTA Test application.
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
 * Internal Definitions
 * ***************************************************************************************************************** */
#if defined(NXPBUILD__PHHAL_HW_PN7462AU) || defined (NXPBUILD__PHHAL_HW_PN76XX)
    #define PHDTA_POLLMODE_T4T_READTEST_BUFFERSIZE   (2 * 1024U) /* 2K bytes memory allocated for NFC controllers for T4T Read tests. */
    #define PHDTA_POLLMODE_T4T_LOOPBACK_RXBUFFERSIZE (1700U)     /* 1700 bytes memory allocated for NFC controllers for T4T loop back tests. */
#else
    #if defined(PHDRIVER_KINETIS_K82)
        #define PHDTA_POLLMODE_T4T_READTEST_BUFFERSIZE (128 * 1024U) /* 128K bytes memory allocated for K82 platform for T4T Read tests. */
    #else
        #define PHDTA_POLLMODE_T4T_READTEST_BUFFERSIZE (32 * 1024U)  /* 32K bytes memory allocated for LPC1769 platform for T4T Read tests. */
    #endif
    #define PHDTA_POLLMODE_T4T_LOOPBACK_RXBUFFERSIZE   (5 * 1024U)   /* 5K bytes memory allocated for frontends for T4T loop back tests. */
#endif /* defined(NXPBUILD__PHHAL_HW_PN7462AU) || defined (NXPBUILD__PHHAL_HW_PN76XX) */

#define PHDTA_POLLMODE_T4T_LOOPBACK_TXBUFFERSIZE    (262U)
#define PHDTA_LISTENMODE_T4T_LOOPBACK_TXBUFFERSIZE  (256U)

/* *****************************************************************************************************************
 * Global and Static Variables
 * ***************************************************************************************************************** */
/* Poll T4T Test Data */
/* START OF Test Command C-APDU Frame */
const uint8_t gaPhDta_T4T_SotCapdu[20U] = {
        0x00, 0xA4, 0x04, 0x00, 0x0E,'2', 'N', 'F', 'C', '.', 'S', 'Y', 'S', '.', 'D', 'D', 'F', '0', '1', 0x00 };

/* End OF Test Command R-APDU Data */
const uint8_t gaPhDta_T4T_Eot[5U] = { 0xFF, 0xFF, 0xFF, 0x01, 0x01 };

/* Listen Mode T4T Test Data*/
/* C-APDU with Select Command, that is expected from LT */
const uint8_t gaPhDta_SelectCommand[20U] = {
        0x00, 0xA4, 0x04, 0x00, 0x0E,'1', 'N', 'F', 'C', '.', 'S', 'Y', 'S', '.', 'D', 'D', 'F', '0', '1', 0x00 };

/* R-APDU for Select Command */
const uint8_t gaPhDta_SelectResponse[4U] = {0x01, 0x00, 0x90, 0x00};

/* Read Data from file Command */
const uint8_t gaPhDta_ReadDataCommand[4U] = {0x00, 0xB0, 0x00, 0x00};

#ifdef NXPBUILD_DTA_READER_DEVICE_CLASS

/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
/* DTA T4T Read Test Functionality API */
static void phDta_T4TRead (void);

/* DTA T4T Write Test Functionality API */
static void phDta_T4TWrite (void);

/* API for performing Perform ISO14443-4 Data Exchange.
 * If data is chained, this API will accumulate chained blocks and then return one contiguous buffer */
static phStatus_t phDta_ISODEP_exchange(
        uint8_t *pbTxBuffer,
        uint16_t wTxBufferSize,
        uint8_t *pbRxBuffer,
        uint16_t *pwRxBufferSize);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
/* Top level DTA T4T Test Module API */
void phDta_T4T (void)
{
    /* Based on the Pattern Number, run DTA T4T Test sub Modules */
    switch(gwPhDta_PatternNumber)
    {
        case (PHDTA_PATTERN_NUM_0):
        case (PHDTA_PATTERN_NUM_7):
        {
            /* Run T4T Loop back Test Functionality */
            phDta_PollT4TLoopBack();
        }
        break;
        case (PHDTA_PATTERN_NUM_1):
        case (PHDTA_PATTERN_NUM_11):
        {
            /* Run T4T Read Test Functionality */
            phDta_T4TRead();
        }
        break;

        case (PHDTA_PATTERN_NUM_2):
        case (PHDTA_PATTERN_NUM_12):
        {
            /* Run T4T Write Test Functionality */
            phDta_T4TWrite();
        }
        break;

        default:
            /* Do Nothing! */
            break;
    }
}

/* DTA Poll Mode T4T Loop Back Test Functionality module */
void phDta_PollT4TLoopBack (void)
{
    phStatus_t wStatus;
    uint8_t *pbRxBuffer;
    uint8_t *pbTxBuffer;
    uint16_t wTxBufferSize;
    uint16_t wRxBufferSize;
    uint8_t *pTmp;

    do
    {
        /* Allocate Memory for RX buffer */
        wStatus = phDta_AllocateMemory (PHDTA_POLLMODE_T4T_LOOPBACK_RXBUFFERSIZE, &pbRxBuffer);
        if(wStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_PollT4TLoopBack: Memory Alloc Failed.");
            break;
        }

        /* Allocate Memory for TX Buffer */
        wStatus = phDta_AllocateMemory (PHDTA_POLLMODE_T4T_LOOPBACK_TXBUFFERSIZE, &pbTxBuffer);
        if(wStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_PollT4TLoopBack: Memory Alloc Failed.");
            break;
        }

        /* Send Start Of Test C-APDU */
        memcpy(pbTxBuffer, gaPhDta_T4T_SotCapdu, sizeof(gaPhDta_T4T_SotCapdu));
        wTxBufferSize = sizeof(gaPhDta_T4T_SotCapdu);

        do
        {
            /* Perform ISO14443-4 Data Exchange */
            wStatus = phDta_ISODEP_exchange(
                    pbTxBuffer,
                    wTxBufferSize,
                    pbRxBuffer,
                    &wRxBufferSize);
            if(wStatus != PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_ISO14443P4))
            {
                DEBUG_PRINTF("\nphDta_PollT4TLoopBack: ISO DEP Exchange Failed!");
                break;
            }

            /*Check if End Of Test Command is received is received in R-APDU */
            if(memcmp((const void *)pbRxBuffer,(const void *)gaPhDta_T4T_Eot,sizeof(gaPhDta_T4T_Eot)) == 0)
            {
                /* EOT R-APDU is received*/
                break;
            }
            else
            {
                /* Create new C-APDU using the Data from received R-APDU (without the 2 status bytes at the end of the R-APDU) */
                if (wRxBufferSize < PHDTA_POLLMODE_T4T_LOOPBACK_TXBUFFERSIZE)
                {
                    /* Copy Received data to Transmit buffer */
                     memcpy(pbTxBuffer,pbRxBuffer,(wRxBufferSize - 2U));
                     wTxBufferSize = wRxBufferSize - 2U;
                }
                else
                {
                    /* Since Transmit buffer is not able to hold more than 256 bytes data (allocated only 256 bytes due to memory limitation),
                     * Swap Transmit and Receive buffers instead of copying Received data(more than 256 bytes) to Transmit buffer.
                     * */
                    pTmp = pbRxBuffer;
                    pbRxBuffer = pbTxBuffer;
                    pbTxBuffer = pTmp;
                    wTxBufferSize = wRxBufferSize - 2U;
                }
            }
        }while(1);

    }while(0);
}

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */
/* API for performing Perform ISO14443-4 Data Exchange.
 * If data is chained, this API will accumulate chained blocks and then return one contiguous buffer */
static phStatus_t phDta_ISODEP_exchange(
        uint8_t *pbTxBuffer,
        uint16_t wTxBufferSize,
        uint8_t *pbRxBuffer,
        uint16_t *pwRxBufferSize)
{
    phStatus_t wStatus;
    uint8_t *pbTempRxBuffer;
    uint16_t wChainedBlockRxSize = 0;
    uint16_t wTotalRxSize = 0;

    wStatus = phpalI14443p4_Exchange(
            ppalI14443p4,
            PH_EXCHANGE_DEFAULT,
            pbTxBuffer,
            wTxBufferSize,
            &pbTempRxBuffer,
            &wChainedBlockRxSize);

    if((wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS_CHAINING, PH_COMP_PAL_ISO14443P4)) ||
       (wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_ISO14443P4)) )
    {
        /* Copy the data received so far */
        memcpy((pbRxBuffer + wTotalRxSize), pbTempRxBuffer, wChainedBlockRxSize);

        /* Keep track of number of bytes received so far */
        wTotalRxSize += wChainedBlockRxSize;

         while(wStatus ==  PH_ADD_COMPCODE(PH_ERR_SUCCESS_CHAINING, PH_COMP_PAL_ISO14443P4))
        {
            /* Continue Block exchange */
            wStatus = phpalI14443p4_Exchange(ppalI14443p4, PH_EXCHANGE_RXCHAINING, pbTxBuffer, wTxBufferSize, &pbTempRxBuffer, &wChainedBlockRxSize);

            if((wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS_CHAINING, PH_COMP_PAL_ISO14443P4)) ||
               (wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_ISO14443P4)) )
            {
                if((wTotalRxSize + wChainedBlockRxSize) > PHDTA_POLLMODE_T4T_LOOPBACK_RXBUFFERSIZE)
                {
                    wStatus = PH_ERR_BUFFER_OVERFLOW;
                    DEBUG_PRINTF("\nphDta_ISODEP_exchange: RX Buffer Overflow.");
                }
                else
                {
                    /* Accumulate the data received so far */
                    memcpy((pbRxBuffer + wTotalRxSize), pbTempRxBuffer, wChainedBlockRxSize);

                    /* Keep track of number of bytes received so far */
                    wTotalRxSize += wChainedBlockRxSize;
                }
            }
        }
    }

    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_PAL_ISO14443P4))
    {
        /* Update Total Received Length */
        *pwRxBufferSize = wTotalRxSize;
    }
    else
    {
        /* Error Occurred. Update Total Received Length to Zero. */
        *pwRxBufferSize = 0;
    }

    return wStatus;
}

/* DTA T4T Read Test Functionality API */
static void phDta_T4TRead (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;
    uint32_t dwMLe;

    /* Set the Tag type to T4T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T4T_TAG);

    /* Reset NDEF Data variables at the starting of the Read Operation */
    pgPhDta_TagTest_NdefData = NULL;
    dwgPhDta_TagTest_NdefDataLength = 0;

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);
    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        /* Allocate Memory for the Tag Read Operation */
        wStatus = phDta_AllocateMemory (PHDTA_POLLMODE_T4T_READTEST_BUFFERSIZE, &pgPhDta_TagTest_NdefData);

        if(wStatus == PH_ERR_SUCCESS)
        {
            if (gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_11)
            {
                wStatus = phalTop_Sw_SetConfig(palTop, PHAL_TOP_CONFIG_T4T_SHORT_APDU, PH_OFF);
                if(wStatus != PH_ERR_SUCCESS)
                {
                    DEBUG_PRINTF("\nphDta_T4TRead: Enable extended APDU failed !!!");
                }
            }

            /* Configure MLe(maximum data size that can be read from the T4T using a single READ_BINARY Command)
             * value to maximum 0xFF to support all existing platforms.
             * */
            wStatus = phalTop_Sw_GetConfig(palTop, PHAL_TOP_CONFIG_T4T_MLE, &dwMLe);
            if(wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_T4TRead: Read MLe value failed !!!");
            }

            if(dwMLe > 0xFFU)
            {
                dwMLe = 0xFFU;
                wStatus = phalTop_Sw_SetConfig(palTop, PHAL_TOP_CONFIG_T4T_MLE, dwMLe);
                if(wStatus != PH_ERR_SUCCESS)
                {
                    DEBUG_PRINTF("\nphDta_T4TRead: Configure MLe value failed !!!");
                }
            }

            /* Attempt to read the entire NDEF message from the Tag */
            wStatus = phalTop_ReadNdef(palTop, pgPhDta_TagTest_NdefData, &dwgPhDta_TagTest_NdefDataLength);

            if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
            {
                DEBUG_PRINTF("\nphDta_T4TRead: Read Tag Successfully.");
            }
            else
            {
                DEBUG_PRINTF("\nphDta_T4TRead: Read Tag UnSuccessful.");
            }
        }
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T4TRead: Not NDEF compliant.");
    }
}

/* DTA T4T Write Test Functionality API */
static void phDta_T4TWrite (void)
{
    phStatus_t wStatus;
    uint8_t bTagState;
    uint32_t dwMLc;

    /* Set the Tag type to T4T at the Tag AL */
    phalTop_SetConfig(palTop, PHAL_TOP_CONFIG_TAG_TYPE, PHAL_TOP_TAG_TYPE_T4T_TAG);

    /* Read Tag Details - Layout, CC, Presence Of NDEF message */
    wStatus = phalTop_CheckNdef(palTop, &bTagState);
    if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
    {
        /* Check that Write Test Data was populated */
        if ((pgPhDta_TagTest_NdefData != NULL) && (dwgPhDta_TagTest_NdefDataLength != 0))
        {
            if (gwPhDta_PatternNumber == PHDTA_PATTERN_NUM_12)
            {
                wStatus = phalTop_Sw_SetConfig(palTop, PHAL_TOP_CONFIG_T4T_SHORT_APDU, PH_OFF);
                if(wStatus != PH_ERR_SUCCESS)
                {
                    DEBUG_PRINTF("\nphDta_T4TWrite: Enable extended APDU failed !!!");
                }
            }

            /* Configure MLc(maximum data size that can be sent to the T4T using a single Command)
             * value to maximum 0xFF to support all existing platforms.
             * */
            wStatus = phalTop_Sw_GetConfig(palTop, PHAL_TOP_CONFIG_T4T_MLC, &dwMLc);
            if(wStatus != PH_ERR_SUCCESS)
            {
                DEBUG_PRINTF("\nphDta_T4TWrite: Read MLc value failed !!!");
            }

            if(dwMLc > 0xFFU)
            {
                dwMLc = 0xFFU;
                wStatus = phalTop_Sw_SetConfig(palTop, PHAL_TOP_CONFIG_T4T_MLC, dwMLc);
                if(wStatus != PH_ERR_SUCCESS)
                {
                    DEBUG_PRINTF("\nphDta_T4TWrite: Configure MLc value failed !!!");
                }
            }

            /* Attempt to write the entire NDEF message that was read earlier, to the Tag */
            wStatus = phalTop_WriteNdef(palTop, pgPhDta_TagTest_NdefData, dwgPhDta_TagTest_NdefDataLength);

            if(wStatus == PH_ADD_COMPCODE(PH_ERR_SUCCESS, PH_COMP_AL_TOP))
            {
                DEBUG_PRINTF("\nphDta_T4TWrite: Write Tag Successful.");
            }
            else
            {
                DEBUG_PRINTF("\nphDta_T4TWrite: Write Tag UnSuccessful.");
            }
        }
        else
        {
            DEBUG_PRINTF("\nphDta_T4TWrite: Run the appropriate T4T Read Test before running this T4T Write Test!");
        }
    }
    else
    {
        DEBUG_PRINTF("\nphDta_T4TWrite: Not NDEF compliant.");
    }
}
#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */

#if defined(NXPBUILD_DTA_CE_DEVICE_CLASS) && !defined(NXPBUILD__PHHAL_HW_RC663)
/* DTA Listen Mode T4T Loop Back Test Functionality module */
void phDta_ListenT4TLoopBack (void)
{
    uint16_t wStatus;
    uint8_t *pbRxData;
    uint8_t *pbTxBuffer;
    uint16_t wRxDataLength;
    uint16_t wTxDataLength = 0;

    do
    {
        /* Allocate Memory for TX Buffer */
        wStatus = phDta_AllocateMemory (PHDTA_LISTENMODE_T4T_LOOPBACK_TXBUFFERSIZE, &pbTxBuffer);
        if(wStatus != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_ListenT4TLoopBack: Memory Alloc Failed.");
            break;
        }

        /* Validate Received RATS and Send ATS */
        wStatus = phpalI14443p4mC_Activate(
                ppalI14443p4mC,
                pDiscLoop->sTargetParams.pRxBuffer,
                (uint8_t)pDiscLoop->sTargetParams.wRxBufferLen,
                (uint8_t *)gaPhDta_AtsResp,
                sizeof(gaPhDta_AtsResp));

        while((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
        {
            /* Perform Receive Operation */
            wStatus = phpalI14443p4mC_Receive(
                    ppalI14443p4mC,
                    PH_RECEIVE_DEFAULT,
                    &pbRxData,
                    &wRxDataLength);

            if((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
            {
                /* Received an I-Block */
                /* Check if the received C-APDU contains Select Command */
                if(memcmp(pbRxData, gaPhDta_SelectCommand, sizeof(gaPhDta_SelectCommand)))
                {
                    /* Not Select Command. The received C-APDU contains data to be looped back. copy received data to TX buffer to form the R-APDU */
                    if (wRxDataLength >= 6U)
                    {
                        //wTxDataLength = wRxDataLength - 6;  /* Apart from Data C-APDU contains CLA, INS, P1, P2, Lc and Le bytes */
                        wTxDataLength = wRxDataLength;
                        memcpy(pbTxBuffer, &pbRxData[0], wTxDataLength); /* The Data bytes start from the 5th byte in the C-APDU */
                    }
                    else
                    {
                        if(!(memcmp(pbRxData, gaPhDta_ReadDataCommand, sizeof(gaPhDta_ReadDataCommand))))
                        {
                            /* Load dummy data(0xA5) to Tx Buffer based on Le value*/
                            memset(pbTxBuffer, 0xA5, pbRxData[4U]);
                            wTxDataLength = pbRxData[4U];
                        }
                    }

                    /* Insert Status bytes SW1 SW2 at the end of the R-APDU */
                    pbTxBuffer[wTxDataLength++] = 0x90U; /* SW1 */
                    pbTxBuffer[wTxDataLength++] = 0x00U; /* SW2 */

                    /* loop-back the formed R-APDU */
                    wStatus = phpalI14443p4mC_Transmit(
                            ppalI14443p4mC,
                            PH_TRANSMIT_DEFAULT,
                            pbTxBuffer,
                            wTxDataLength);
                }
                else
                {
                    /* The received C-APDU contains the Select Command */
                    /* Transmit the fixed response data */
                    wStatus = phpalI14443p4mC_Transmit(
                            ppalI14443p4mC,
                            PH_TRANSMIT_DEFAULT,
                            (uint8_t *)gaPhDta_SelectResponse,
                            sizeof(gaPhDta_SelectResponse));
                }
            }
            else if((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS_DESELECTED)
            {
                (void)phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_MFHALTED, PH_ON);
            }
            else if((wStatus & PH_ERR_MASK) == PH_ERR_EXT_RF_ERROR)
            {
                /* Reset MFHalted bit to accept all commands */
                (void)phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_MFHALTED, PH_OFF);
            }

            /* Reset TX data length */
            wTxDataLength = 0;
        }

    }while(0);
}
#endif /* defined(NXPBUILD_DTA_CE_DEVICE_CLASS) && !defined(NXPBUILD__PHHAL_HW_RC663) */
