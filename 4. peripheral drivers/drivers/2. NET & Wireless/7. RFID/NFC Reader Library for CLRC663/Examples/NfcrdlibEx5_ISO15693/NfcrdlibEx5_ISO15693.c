/*----------------------------------------------------------------------------*/
/* Copyright 2016-2021,2023 NXP                                               */
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
 * Example Source for NfcrdlibEx5_ISO15693 that uses the ISO15693 implementation.
 * This example will load/configure Discovery loop for ISO15693, uses POLL mode to detect the ISO15693 card.
 * Displays detected ISO15693 card information like UID, Card Type, Block Read and Block Write operation status.
 * Example will activate the device at index zero whenever multiple MIFARE Classic contactless IC cards are detected.
 *
 * Please refer Readme.txt file for Hardware Pin Configuration, Software Configuration and steps to build and
 * execute the project which is present in the same project directory.
 * $Author: $
 * $Revision: $ (v07.09.00)
 * $Date: $
 *
 */

/**
* Reader Library Headers
*/
#include <phApp_Init.h>

/* Local headers */
#include  "NfcrdlibEx5_ISO15693.h"

/* PAL Headers */
#include <phpalSli15693.h>

/* AL Headers */
#include <phalICode.h>

	
/*******************************************************************************
**   Definitions
*******************************************************************************/

#define ISO15693_NXP_TAG_ID                 0x04    /* ISO15693 NXP Cards specific code */
#define ISO15693_UID_NXP_IDPOS              6       /* ISO15693 NXP Cards specific code position (UID6) */
#define ISO15693_UID_SIZE_BITS              64      /* ISO15693 UID Size in Bits */

#define ISO15693_MFGID_CARDSEL_IDPOS        4       /* NXP Card Type detection (UID4) */
#define ISO15693_MFGID_CARDSEL_MSK          0x18    /* NXP Card Type detection Mask value */
#define ISO15693_MFGID_CARDSEL_BITPOS       3       /* NXP Card Type detection Bit position in UID4 */
#define ISO15693_MFGID_DNA_CARDSEL_MSK      0x40    /* NXP DNA Card Type detection Mask value */
#define ISO15693_MFGID_DNA_CARDSEL_BITPOS   6       /* NXP Card Type detection Bit position in UID4 */

/* NXP Card Type(SLI/SLI-S/SLI-L or SLIX/SLIX-S/SLIX-L) detection (UID5) */
#define ISO15693_UID_CARDSEL_IDPOS      5

/* NXP Card Type states */
#define ISO15693_MFGID_SLI_STATE        0
#define ISO15693_MFGID_SLIX_STATE       2
#define ISO15693_MFGID_SLIX2_STATE      1
#define ISO15693_MFGID_DNA_STATE        3

/* SLI Card Type states */
#define ISO15693_UID_SLI_STATE          1
#define ISO15693_UID_SLI_S_STATE        2
#define ISO15693_UID_SLI_L_STATE        3

/* SLIX Card Type states */
#define ISO15693_UID_SLIX_STATE         1
#define ISO15693_UID_SLIX_S_STATE       2
#define ISO15693_UID_SLIX_L_STATE       3

/* ICode DNA and NTag 5 Series Card Types */
#define ISO15693_UID_ICODE_DNA_STATE    0
#define ISO15693_UID_NTAG_5_SERIES      1

phacDiscLoop_Sw_DataParams_t       * pDiscLoop;       /* Discovery loop component */
void * psalI15693;

#ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
uint32_t aIso15693TaskBuffer[ISO15693_DEMO_TASK_STACK];
#else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
#define aIso15693TaskBuffer       NULL
#endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */

/*******************************************************************************
**   Prototypes
*******************************************************************************/

void NfcrdlibEx5_ISO15693(void *pParams);
static phStatus_t phExample_Init(void);
static phStatus_t DisplayCardTypeInfo(uint8_t *pUID, uint8_t *pNTag5_State);
static void ReadMultipleBlock_HighDataRate(uint8_t bNTag5_State);

#ifdef PH_OSAL_FREERTOS
const uint8_t bTaskName[configMAX_TASK_NAME_LEN] = {"Ex5_ISO15693"};
#else
const uint8_t bTaskName[] = {"Ex5_ISO15693"};
#endif /* PH_OSAL_FREERTOS */

static volatile uint8_t bInfLoop = 1U;

/*******************************************************************************
**   Code
*******************************************************************************/

int main(void)
{
    do
    {
        phStatus_t status = PH_ERR_INTERNAL_ERROR;
        phNfcLib_Status_t     dwStatus;
#ifdef PH_PLATFORM_HAS_ICFRONTEND
        phNfcLib_AppContext_t AppContext = {0};
#endif /* PH_PLATFORM_HAS_ICFRONTEND */

#ifndef PH_OSAL_NULLOS
        phOsal_ThreadObj_t Iso15693;
#endif /* PH_OSAL_NULLOS */

	


        /* Perform Controller specific initialization. */
        phApp_CPU_Init();


        /* Perform OSAL Initialization. */
        (void)phOsal_Init();

        /* Print Example application name */
        DEBUG_PRINTF("\n *** ISO 15693 Example *** \n");

#ifdef PH_PLATFORM_HAS_ICFRONTEND
        status = phbalReg_Init(&sBalParams, sizeof(phbalReg_Type_t));
        CHECK_STATUS(status);

        AppContext.pBalDataparams = &sBalParams;
        dwStatus = phNfcLib_SetContext(&AppContext);
        CHECK_NFCLIB_STATUS(dwStatus);
#endif /* PH_PLATFORM_HAS_ICFRONTEND */

        /* Initialize library */
        dwStatus = phNfcLib_Init();
        CHECK_NFCLIB_STATUS(dwStatus);
        if(dwStatus != PH_NFCLIB_STATUS_SUCCESS) break;

        /* Set the generic pointer */
        pHal = phNfcLib_GetDataParams(PH_COMP_HAL);
        psalI15693 = phNfcLib_GetDataParams(PH_COMP_AL_ICODE);
        pDiscLoop = phNfcLib_GetDataParams(PH_COMP_AC_DISCLOOP);

        /* Initialize other components that are not initialized by NFCLIB and configure Discovery Loop. */
        status = phApp_Comp_Init(pDiscLoop);
        CHECK_STATUS(status);
        if(status != PH_ERR_SUCCESS) break;

        /* Perform Platform Init */
        status = phApp_Configure_IRQ();
        CHECK_STATUS(status);
        if(status != PH_ERR_SUCCESS) break;

#ifndef PH_OSAL_NULLOS

        Iso15693.pTaskName = (uint8_t *)bTaskName;
        Iso15693.pStackBuffer = aIso15693TaskBuffer;
        Iso15693.priority = ISO15693_DEMO_TASK_PRIO;
        Iso15693.stackSizeInNum = ISO15693_DEMO_TASK_STACK;
        phOsal_ThreadCreate(&Iso15693.ThreadHandle, &Iso15693, &NfcrdlibEx5_ISO15693, pDiscLoop);

        phOsal_StartScheduler();

        /* Print Error info */
        DEBUG_PRINTF("RTOS Error : Scheduler exited. \n");

#else
        (void)NfcrdlibEx5_ISO15693(pDiscLoop);
#endif /* PH_OSAL_NULLOS */
    } while(0);

    while(bInfLoop); /* Comes here if initialization failure or scheduler exit due to error */

    return 0;
}

/***********************************************************************************************
 * \brief   This function demonstrates the Type V (ISO 15693) card detection, Block Read and Write operation.
 * \param   *pParams
 * \return  This function will never return
 **********************************************************************************************/
void NfcrdlibEx5_ISO15693(void *pParams)
{
    phStatus_t  status = 0;
    uint16_t    wTagsDetected = 0;
    uint8_t     bBlock = 0x03;
    uint8_t     *pRxbuffer;
    uint16_t    bDataLength;
    uint8_t     aTempUid[8];
    uint8_t     aReceivedUid[8];
    uint8_t     bDsfid = 0;
    uint8_t     bNTag5_State;

    /* This call shall allocate secure context before calling any secure function,
     * when FreeRtos trust zone is enabled.
     * */
    phOsal_ThreadSecureStack( 512 );

    /* Initialize library */
    status = phExample_Init();
    CHECK_STATUS(status);

    while(1)    /* Continuous loop */
    {
        bNTag5_State = 0;
        DEBUG_PRINTF("\nReady to detect");
        DEBUG_PRINTF("\n");

        do
        {
            /* Field OFF */
            status = phhalHw_FieldOff(pHal);
            CHECK_STATUS(status);

            status = phhalHw_Wait(pDiscLoop->pHalDataParams,PHHAL_HW_TIME_MICROSECONDS, 5100);
            CHECK_STATUS(status);

            /* Configure Discovery loop for Poll Mode */
            status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
            CHECK_STATUS(status);

            /* Run Discovery loop */
            status = phacDiscLoop_Run(pDiscLoop, PHAC_DISCLOOP_ENTRY_POINT_POLL);

        }while((status & PH_ERR_MASK) != PHAC_DISCLOOP_DEVICE_ACTIVATED); /* Exit on Card detection */

        /* Card detected */
        /* Get the tag types detected info */
        status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTagsDetected);

        /* Check for Status */
        if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
        {
            /* Check for Type V(ISO 15693) tag detection */
            if(PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_V))
            {
                DEBUG_PRINTF("\nType V / ISO 15693 / T5T Detected \n");

                /* Print UID */
                DEBUG_PRINTF ("\nUID: ");
                phApp_Print_Buff(pDiscLoop->sTypeVTargetInfo.aTypeV[0].aUid, 0x08);

                /* Copy UID */
                memcpy(aReceivedUid, pDiscLoop->sTypeVTargetInfo.aTypeV[0].aUid, 0x08);

                /* Check and display Card type info */
                if (DisplayCardTypeInfo(pDiscLoop->sTypeVTargetInfo.aTypeV[0].aUid, &bNTag5_State) == PH_ERR_SUCCESS)
                {
                    do
                    {
                        /* Data length */
                        bDataLength = 0x04;

                        /* Block Read */
                        DEBUG_PRINTF("\nRead Data from Block %d", bBlock);

                        /* Read single block */
                        status = phalICode_ReadSingleBlock(psalI15693,
                            PHAL_ICODE_OPTION_OFF,
                            bBlock,
                            &pRxbuffer,
                            &bDataLength);
                        /* Check for Status */
                        if(status != PH_ERR_SUCCESS)
                        {
                            /* Print Error info */
                            DEBUG_PRINTF ("\nRead operation Failed!!!");
                            DEBUG_PRINTF("\nExecution aborted!!!\n");
                            break;
                        }

                        /* Read Success */
                        DEBUG_PRINTF("\nRead Success");
                        DEBUG_PRINTF("\nThe content of Block %d is:", bBlock);
                        phApp_Print_Buff (pRxbuffer, bDataLength);
                        DEBUG_PRINTF("\n\n --- End of Read Operation ---");

                        /* Block Write */
                        DEBUG_PRINTF("\n\nWrite data to Block %d", bBlock);

                        /* Write single block */
                        status = phalICode_WriteSingleBlock(psalI15693,
                            PHAL_ICODE_OPTION_OFF,
                            bBlock,
                            pRxbuffer,
                            bDataLength);
                        /* Check for Status */
                        if(status != PH_ERR_SUCCESS)
                        {
                            /* Print Error info */
                            DEBUG_PRINTF ("\nWrite operation Failed!!!");
                            DEBUG_PRINTF("\nExecution aborted!!!\n");
                            break;
                        }

                        /* Write Success */
                        DEBUG_PRINTF ("\nWrite Success");
                        DEBUG_PRINTF("\n\n --- End of Write Operation ---");

                        ReadMultipleBlock_HighDataRate(bNTag5_State);

                        DEBUG_PRINTF("\n\n --- End of Example ---\n\n");
                    }while(0);
                }

                DEBUG_PRINTF("\nPlease Remove the Card\n\n");

                /* Field RESET */
                status = phhalHw_FieldReset(pHal);
                CHECK_STATUS(status);

                /* Make sure that example application is not detecting the same card continuously */
                do
                {
                    /* Clear UID buffer */
                    memset(aTempUid, 0x00, 0x08);

                    /* Inventory request */
                    status = phpalSli15693_Inventory(pDiscLoop->pPalSli15693DataParams,
                        (PHPAL_SLI15693_FLAG_NBSLOTS | PHPAL_SLI15693_FLAG_DATA_RATE | PHPAL_SLI15693_FLAG_INVENTORY),
                        0,
                        aReceivedUid,
                        ISO15693_UID_SIZE_BITS,
                        &bDsfid,
                        aTempUid);

                    /* Check for Status */
                    if (status != PH_ERR_SUCCESS)
                    {
                        break; /* Card Removed, break from the loop */
                    }

                    /* Delay - 5 milli seconds*/
                    status = phhalHw_Wait(pDiscLoop->pHalDataParams, PHHAL_HW_TIME_MILLISECONDS, 5);
                    CHECK_STATUS(status);

                }while(1);
            }
        }
    }
}

/***********************************************************************************************
 * \brief   Initializes the Reader Library
 * \param   none
 * \return  status  Returns the function status
 **********************************************************************************************/
static phStatus_t phExample_Init(void)
{
    phStatus_t status;
#if defined NXPBUILD__PHHAL_HW_RC663
    uint8_t bChipVersion;
#endif /* NXPBUILD__PHHAL_HW_RC663 */

    /* Device limit for Type V (ISO 15693) */
    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_DEVICE_LIMIT, 1);
    CHECK_STATUS(status);

    /* Passive polling Tx Guard times in micro seconds. */
    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTV_VALUE_US, 5000);
    CHECK_STATUS(status);

    /* Bailout on Type V (ISO 15693) detect */
    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_BAIL_OUT, PHAC_DISCLOOP_POS_BIT_MASK_V);
    CHECK_STATUS(status);

    /* Read Chip Version */
#if defined NXPBUILD__PHHAL_HW_RC663
    status = phhalHw_Rc663_ReadRegister(pHal, PHHAL_HW_RC663_REG_VERSION, &bChipVersion);
    CHECK_STATUS(status);
    DEBUG_PRINTF("\nReader chip RC663: 0x%02x\n", bChipVersion);
#endif /* NXPBUILD__PHHAL_HW_RC663 */

    /* Return Success */
    return PH_ERR_SUCCESS;
}

/***********************************************************************************************
 * \brief   This functions prints the Card type information like SLI, SLIX etc.
 * \param   *pUID   UID Pointer
 * \return  status  Returns the function status
 **********************************************************************************************/
static phStatus_t DisplayCardTypeInfo(uint8_t *pUID, uint8_t *pNTag5_State)
{
    uint8_t bCardType;
    phStatus_t  status = PH_ERR_SUCCESS;

    /* Check for ISO15693 NXP TAG */
    if (pUID[ISO15693_UID_NXP_IDPOS] != ISO15693_NXP_TAG_ID)
    {
        /* Print Product type */
        DEBUG_PRINTF("\nProduct: Non NXP ISO15693 Tag Detected\n");

        /* Return Status */
        return (PH_COMP_PAL_SLI15693 | PH_ERR_INVALID_DATA_PARAMS);
    }

    /* Read SLI Card type information from UID (Byte 4) */
    bCardType = ((pUID[ISO15693_MFGID_CARDSEL_IDPOS] & ISO15693_MFGID_CARDSEL_MSK) >> ISO15693_MFGID_CARDSEL_BITPOS);

    /* Switch based on Card Type(SLI/SLIX/SLIX2) */
    switch (bCardType)
    {
    case ISO15693_MFGID_SLI_STATE:          /* SLI Card state */
        /* Switch based on Card Type(SLI/SLI-S/SLI-L) */
        switch (pUID[ISO15693_UID_CARDSEL_IDPOS])
        {
        case ISO15693_UID_SLI_STATE:        /* SLI Card state */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: ICODE SLI\n");
            break;

        case ISO15693_UID_SLI_S_STATE:      /* SLI-S Card state */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: ICODE SLI-S\n");
            break;

        case ISO15693_UID_SLI_L_STATE:      /* SLI-L Card state */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: ICODE SLI-L\n");
            break;

        default:                            /* default */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: Unidentified Tag\n");
            /* Update status */
            status = (PH_COMP_PAL_SLI15693 | PH_ERR_INVALID_DATA_PARAMS);
            break;
        }
        break;

    case ISO15693_MFGID_SLIX_STATE:         /* SLIX Card state */
        /* Switch based on Card Type(SLIX/SLIX-S/SLIX-L) */
        switch (pUID[ISO15693_UID_CARDSEL_IDPOS])
        {
        case ISO15693_UID_SLIX_STATE:       /* SLIX Card state */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: ICODE SLIX\n");
            break;

        case ISO15693_UID_SLIX_S_STATE:     /* SLIX-S Card state */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: ICODE SLIX-S\n");
            break;

        case ISO15693_UID_SLIX_L_STATE:     /* SLIX-L Card state */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: ICODE SLIX-L\n");
            break;

        default:                            /* default */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: Unidentified Tag\n");
            /* Update status */
            status = (PH_COMP_PAL_SLI15693 | PH_ERR_INVALID_DATA_PARAMS);
            break;
        }
        break;

    case ISO15693_MFGID_SLIX2_STATE:        /* SLIX2 Card state */
        /* Print Product type */
        DEBUG_PRINTF("\nProduct: ICODE SLIX2\n");
        break;

    case ISO15693_MFGID_DNA_STATE:
        /* Switch based on Card Type(ICode DNA or NTag 5 Series) */
        switch ((pUID[ISO15693_MFGID_CARDSEL_IDPOS] & ISO15693_MFGID_DNA_CARDSEL_MSK) >> ISO15693_MFGID_DNA_CARDSEL_BITPOS)
        {
        case ISO15693_UID_ICODE_DNA_STATE:  /* ICode DNA Card state */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: ICODE DNA\n");
            break;

        case ISO15693_UID_NTAG_5_SERIES:    /* NTag 5 Series */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: NTag 5 Series\n");
            *pNTag5_State = 1;
            break;

        default:                            /* default */
            /* Print Product type */
            DEBUG_PRINTF("\nProduct: Unidentified Tag\n");
            /* Update status */
            status = (PH_COMP_PAL_SLI15693 | PH_ERR_INVALID_DATA_PARAMS);
            break;
        }
        break;

    default:                                /* default */
        /* Print Product type */
        DEBUG_PRINTF("\nProduct: Unidentified Tag\n");
        /* Update status */
        status = (PH_COMP_PAL_SLI15693 | PH_ERR_INVALID_DATA_PARAMS);
        break;
    }

    /* Return Status */
    return status;
}

/***********************************************************************************************
 * \brief   This functions performs Read Multiple Blocks at higher data rates with NTag 5.
 * \param   *pUID   UID Pointer
 * \return  status  Returns the function status
 **********************************************************************************************/
static void ReadMultipleBlock_HighDataRate(uint8_t bNTag5_State)
{
#ifdef CUSTOM_HIGH_DATA_RATE_DEMO
    phStatus_t status;
    uint8_t     bBlockNo;
    uint8_t     bNumOfBlocks;
    uint8_t     aRxBuf[256];
    uint16_t    wRxDataLength = 0x00;
    uint8_t     bTxRxBaudrate;
    uint8_t     bTiming = PHAL_ICODE_PARAMETERS_TIMING_320_9_US;

    if (bNTag5_State == 1)
    {
#ifdef CUSTOM_HIGH_DATA_RATE_DEMO_TX106_RX106
        bTxRxBaudrate = 0x22;
        DEBUG_PRINTF("\nPerform ParameterSelect command exchange to move to Tx and Rx at 106Kbps");
#else
        bTxRxBaudrate = 0x44;
        DEBUG_PRINTF("\nPerform ParameterSelect command exchange to move to Tx and Rx at 212Kbps");
        DEBUG_PRINTF("\n!!!!!!!!!!!!Use NTag 5 Boost supporting Active Load Modulation Feature to operate at 212Kbps data rate!!!!!!!!!!!!");
#endif /* CUSTOM_HIGH_DATA_RATE_DEMO_TX106_RX106 */

        /* Switch to higher baudrate */
        status = phalICode_ParameterSelect(
            psalI15693,
            bTxRxBaudrate,
            bTiming);
        if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nSuccessfully switched to selected higher baud rate using Parameter Select Exchange");

            bBlockNo = 3;
            bNumOfBlocks = 4;
            /* Block Read */
            DEBUG_PRINTF("\nExtended Read Multiple Blocks from Block No %d to Block No %d", bBlockNo, (bBlockNo + bNumOfBlocks - 1));

            /*Read the the contents of a single block*/
            status = phalICode_ExtendedReadMultipleBlocks(
                psalI15693,
                0,
                bBlockNo,
                bNumOfBlocks,
                &aRxBuf[0],
                &wRxDataLength);
            CHECK_STATUS(status);

            /* Check for Status */
            if(status != PH_ERR_SUCCESS)
            {
                /* Print Error info */
                DEBUG_PRINTF ("\nExtended Read Multiple Blocks operation Failed at higher data rates!!!");
            }
            else
            {
                /* Read Success */
                DEBUG_PRINTF("\nExtended Read Multiple Blocks Success at higher data rates");
                DEBUG_PRINTF("\n\n\tThe content from Block %d is:\t", bBlockNo);
                phApp_Print_Buff (&aRxBuf[0], wRxDataLength);
                DEBUG_PRINTF("\n\n --- End of Extended Read Multiple Blocks Operation ---");
            }
        }
        else
        {
            DEBUG_PRINTF("\nFailed to switch to selected higher baud rate using Parameter Select Exchange");
            DEBUG_PRINTF("\n!!!!!!!!!!!!Try with NTag 5 Cards supporting higer data rates!!!!!!!!!!!!");
        }
    }
#endif /* CUSTOM_HIGH_DATA_RATE_DEMO */
}

#ifdef NXPBUILD__PHHAL_HW_TARGET
/* Stubbed definitions in case TARGET is enabled */
uint8_t  sens_res[2]     = {0x04, 0x00};
uint8_t  nfc_id1[3]      = {0xA1, 0xA2, 0xA3};
uint8_t  sel_res         = 0x40;
uint8_t  nfc_id3         = 0xFA;
uint8_t  poll_res[18]    = {0x01, 0xFE, 0xB2, 0xB3, 0xB4, 0xB5,
                                   0xB6, 0xB7, 0xC0, 0xC1, 0xC2, 0xC3,
                                   0xC4, 0xC5, 0xC6, 0xC7, 0x23, 0x45 };
#endif /* NXPBUILD__PHHAL_HW_TARGET */
