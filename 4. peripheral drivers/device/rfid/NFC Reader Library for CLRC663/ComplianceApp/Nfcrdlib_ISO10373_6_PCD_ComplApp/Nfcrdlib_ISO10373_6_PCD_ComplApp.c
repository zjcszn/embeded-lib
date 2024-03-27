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
* Example Source for Nfcrdlib_ISO10373_6_PCD_ComplApp.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

/**
* Reader Library Headers
*/
#include <phApp_Init.h>

/* Local headers */
#include "Nfcrdlib_ISO10373_6_PCD_ComplApp.h"

	
/*******************************************************************************
** Definitions
*******************************************************************************/

void *pspalI14443p3a;
void *pspalI14443p3b;
void *pspalI14443p4a;
void *pspalI14443p4;

uint8_t Atqb[14];
uint8_t AtqbLen;
uint8_t Ats[50];
uint8_t bCidEnabled;
uint8_t bCid;
uint8_t bNadSupported;
uint8_t bFwi;
uint8_t bFsdi;
uint8_t bFsci;
uint8_t bUid[10];
uint8_t bMoreCardsAvailable;
uint8_t bLength;
uint8_t bMbli;
uint8_t bSak;
uint8_t *pRxData;
uint16_t wRxDataLength;
uint8_t aAppBuffer[4100];
uint16_t wTxDataLength;

static  uint8_t SOT[] = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41,
                            0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44,
                            0x46, 0x30, 0x31, 0x00};
static uint8_t EOT[] = {0x00, 0x70, 0x04, 0x04, 0x00, 0x90, 0x00};

static volatile uint8_t bInfLoop = 1U;

#ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
uint32_t aISO10373PCDTaskBuffer[ISO10373_PCD_TASK_STACK];
#else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
#define aISO10373PCDTaskBuffer       NULL
#endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */

#ifdef PH_OSAL_FREERTOS
const uint8_t bTaskName[configMAX_TASK_NAME_LEN] = {"ISO10373PCD"};
#else
const uint8_t bTaskName[] = {"ISO10373PCD"};
#endif /* PH_OSAL_FREERTOS */

/*******************************************************************************
** Prototypes
*******************************************************************************/

void ISO10373_PCD_App(void * pParams);
static phStatus_t Configure_Mode(void);
static phStatus_t StartPolling(void);
static phStatus_t DetectTypeA(void);
static phStatus_t DetectTypeB(void);
static phStatus_t TypeB_ActivateCard(
    phpalI14443p3b_Sw_DataParams_t * pDataParams,
    uint8_t * pPupi,
    uint8_t  bPupiLength,
    uint8_t bNumSlots,
    uint8_t bAfi,
    uint8_t bExtAtqb,
    uint8_t bFsdi,
    uint8_t bCid,
    uint8_t bDri,
    uint8_t bDsi,
    uint8_t * pAtqb,
    uint8_t * pAtqbLen,
    uint8_t * pMbli,
    uint8_t * pMoreCardsAvailable
    );
static phStatus_t IBlockExchange(void);

/*******************************************************************************
**   Code
*******************************************************************************/

int main(void)
{
    phStatus_t            status = PH_ERR_INTERNAL_ERROR;
    phNfcLib_Status_t     dwStatus;

#ifdef PH_PLATFORM_HAS_ICFRONTEND
    phNfcLib_AppContext_t AppContext = {0};
#endif /* PH_PLATFORM_HAS_ICFRONTEND */

#ifndef PH_OSAL_NULLOS
    phOsal_ThreadObj_t ISO10373PCD;
#endif /* PH_OSAL_NULLOS */

    do
    {
	


        /* Perform Controller specific initialization. */
        phApp_CPU_Init();


        /* Perform OSAL Initialization. */
        (void)phOsal_Init();

        DEBUG_PRINTF("\n*** ISO10373-6 PCD compliance example ***");

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
        pspalI14443p3a = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3A);
        pspalI14443p3b = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3B);
        pspalI14443p4a = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P4A);
        pspalI14443p4 = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P4);

        /* Configure the IRQ */
        status = phApp_Configure_IRQ();
        CHECK_STATUS(status);
        if(status != PH_ERR_SUCCESS) break;

#ifndef PH_OSAL_NULLOS

        ISO10373PCD.pTaskName = (uint8_t *)bTaskName;
        ISO10373PCD.pStackBuffer = aISO10373PCDTaskBuffer;
        ISO10373PCD.priority = ISO10373_PCD_TASK_PRIO;
        ISO10373PCD.stackSizeInNum = ISO10373_PCD_TASK_STACK;
        phOsal_ThreadCreate(&ISO10373PCD.ThreadHandle, &ISO10373PCD, &ISO10373_PCD_App, pHal);
        phOsal_StartScheduler();
        DEBUG_PRINTF("RTOS Error : Scheduler exited. \n");

#else
        ISO10373_PCD_App(pHal);
#endif /* PH_OSAL_NULLOS */
    } while(0);

    while(bInfLoop); /* Comes here if initialization failure or scheduler exit due to error */

    return 0;
}

/**
* \brief ISO10373 PCD application thread
* This function performs continuous poll for Type A and B technology and performs Layer 4 exchange.
* \return void
*/
void ISO10373_PCD_App(void * pParams)
{
    phStatus_t wStatus;

    /* This call shall allocate secure context before calling any secure function,
     * when FreeRtos trust zone is enabled.
     * */
    phOsal_ThreadSecureStack( 512 );

    /* Configure Mode */
    wStatus = Configure_Mode();
    CHECK_STATUS(wStatus);

    DEBUG_PRINTF("\nISO10373-6 PCD application thread started...\n");

    while(1)
    {
        /* RF Field OFF */
        wStatus = phhalHw_FieldOff(pHal);
        CHECK_STATUS(wStatus);

        /* Delay between Poll A and B */
        wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 2);
        CHECK_STATUS(wStatus);

        /* Polling */
        wStatus = StartPolling();
        if((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
        {
            /* L4 Exchange */
            wStatus = IBlockExchange();
        }
    }
}

/**
* \brief Configure Mode
* This function configures HAL and PAL layers to ISO Mode.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t Configure_Mode(void)
{
    phStatus_t wStatus;

    /* Configure HAL to ISO mode */
    wStatus = phhalHw_SetConfig(
        pHal,
        PHHAL_HW_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Configure I14443-A PAL to ISO mode */
    wStatus = phpalI14443p3a_SetConfig(
        pspalI14443p3a,
        PHPAL_I14443P3A_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Configure I14443-B PAL to ISO mode */
    wStatus = phpalI14443p3b_SetConfig(
        pspalI14443p3b,
        PHPAL_I14443P3B_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Configure I14443-4A PAL to ISO mode */
    wStatus = phpalI14443p4a_SetConfig(
        pspalI14443p4a,
        PHPAL_I14443P4A_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Configure I14443-4 PAL to ISO mode */
    wStatus = phpalI14443p4_SetConfig(
        pspalI14443p4,
        PHPAL_I14443P4_CONFIG_OPE_MODE,
        RD_LIB_MODE_ISO);
    CHECK_STATUS(wStatus);

    /* Return Status */
    return wStatus;
}

/**
* \brief Start Polling
* This function performs polling for Type A and Type B.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t StartPolling(void)
{
    phStatus_t wStatus;

    /* Poll for Type A */
    wStatus = DetectTypeA();
    if ((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        return PH_ERR_SUCCESS;
    }

#ifdef NXPBUILD__PHHAL_HW_PN7462AU
    wStatus = phhalHw_FieldOff(pHal);
    CHECK_STATUS(wStatus);

    /* Delay between poll A and B */
    wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 2);
   CHECK_STATUS(wStatus);
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

    /* Poll for Type B */
    wStatus = DetectTypeB();
    if ((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        return PH_ERR_SUCCESS;
    }

    /* Return Status */
    return wStatus;
}

/**
* \brief Type A detection
* This function performs Type A detection.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t DetectTypeA(void)
{
    phStatus_t wStatus;
    uint8_t bIndex;

    /* Apply Protocol Setting for Type A */
    wStatus = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);
    CHECK_STATUS(wStatus);

    /* RF Field ON */
    wStatus = phhalHw_FieldOn(pHal);
    CHECK_STATUS(wStatus);

    wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 6);
    CHECK_STATUS(wStatus);

    /* Activate Layer 3A card */
    wStatus = phpalI14443p3a_ActivateCard(
        pspalI14443p3a,
        NULL,
        0x00,
        bUid,
        &bLength,
        &bSak,
        &bMoreCardsAvailable);
    if (PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
    {
        /* Find CID to be used */
        for(bIndex = 0; bIndex < (bLength - 1); bIndex++)
        {
            if(bUid[bIndex] == 0xBB)
            {
                bCid = bUid[bIndex + 1];
                break;
            }
        }

        /* Send RATS */
        wStatus = phpalI14443p4a_Rats(
            pspalI14443p4a,
            PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE,    /* Fsdi */
            bCid,
            Ats);

        /* Find the baud rates which shall be used */
        if(bUid[0] == 0xAA)
        {
            /* Send PPS */
            (void)phpalI14443p4a_Pps(
                pspalI14443p4a,
                (bUid[1] & 0x03),
                ((bUid[1] & 0x0C) >> 2));

        }

        if (PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
        {
            /* Get parameters from 4A */
            wStatus = phpalI14443p4a_GetProtocolParams(
                pspalI14443p4a,
                &bCidEnabled,
                &bCid,
                &bNadSupported,
                &bFwi,
                &bFsdi,
                &bFsci);
            CHECK_STATUS(wStatus);

            if(bFsci > PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE)
            {
                bFsci = PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE;
            }

            /* Apply parameters to layer 4 */
            wStatus = phpalI14443p4_SetProtocol(
                pspalI14443p4,
                bCidEnabled,
                bCid,
                0,
                0,
                bFwi,
                bFsdi,
                bFsci);
            CHECK_STATUS(wStatus);
        }
    }

    /* Return Status */
    return wStatus;
}

/**
* \brief Type B detection
* This function performs Type B detection.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t DetectTypeB(void)
{
    phStatus_t wStatus;

    /* Apply Protocol Setting for Type B */
    wStatus = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443B);
    CHECK_STATUS(wStatus);

    /* RF Field ON */
    wStatus = phhalHw_FieldOn(pHal);
    CHECK_STATUS(wStatus);

    wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 6);
    CHECK_STATUS(wStatus);

    /* Activate Type B */
    wStatus = TypeB_ActivateCard(
        pspalI14443p3b,
        NULL,
        0,
        0,
        0,
        0,
        PH_NXPNFCRDLIB_CONFIG_FSDI_VALUE,    /* Fsdi */
        0,    /* Cid */
        0,    /* Dri */
        0,    /* Dsi */
        Atqb,
        &AtqbLen,
        &bMbli,
        &bMoreCardsAvailable);
    if(PH_ERR_SUCCESS == (wStatus & PH_ERR_MASK))
    {
        /* Get parameters from 3B */
        wStatus = phpalI14443p3b_GetProtocolParams(
            pspalI14443p3b,
            &bCidEnabled,
            &bCid,
            &bNadSupported,
            &bFwi,
            &bFsdi,
            &bFsci);
        CHECK_STATUS(wStatus);

        if(bFsci > PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE)
        {
            bFsci = PH_NXPNFCRDLIB_CONFIG_DEFAULT_FSCI_VALUE;
        }

        /* Apply parameters to layer 4 */
        wStatus = phpalI14443p4_SetProtocol(
            pspalI14443p4,
            bCidEnabled,
            bCid,
            0,
            0,
            bFwi,
            bFsdi,
            bFsci);
        CHECK_STATUS(wStatus);
    }

    /* Return Status */
    return wStatus;
}

/**
* \brief Activate Type B Card
* This function performs Type B Activation.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t TypeB_ActivateCard(
    phpalI14443p3b_Sw_DataParams_t * pDataParams,
    uint8_t * pPupi,
    uint8_t bPupiLength,
    uint8_t bNumSlots,
    uint8_t bAfi,
    uint8_t bExtAtqb,
    uint8_t bFsdi,
    uint8_t bCid,
    uint8_t bDri,
    uint8_t bDsi,
    uint8_t * pAtqb,
    uint8_t * pAtqbLen,
    uint8_t * pMbli,
    uint8_t * pMoreCardsAvailable
    )
{
    phStatus_t wStatus;
    phStatus_t wStatusFirstError;
    uint8_t    bSlot;
    uint8_t    bPICCsFound;
    uint8_t    pLocalAtqb[13];
    uint8_t    bLocalAtqbLen;
    uint8_t    *pLocalPupi;

    bPICCsFound = 0;
    bLocalAtqbLen = 0x00;
    pLocalPupi = &pLocalAtqb[1];
    *pMoreCardsAvailable = 0x00;

    /* Initialize the statusFirstError with timeout, which is the trigger */
    wStatusFirstError = PH_ERR_IO_TIMEOUT;

    /* Number of slots and length of PUPI */
    if ((bNumSlots > 4) || ((bPupiLength > 0) && (bPupiLength != 4) && (pDataParams->bPollCmd != PHPAL_I14443P3B_USE_WUPB) ))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_ISO14443P3B);
    }

    /* Check given DSI, DRI, FSDI */
    if (((bFsdi & ~PHPAL_I14443P3B_DATARATE_FORCE) > 12)
        || ((bDri & ~PHPAL_I14443P3B_DATARATE_FORCE) > 3)
        || ((bDsi & ~PHPAL_I14443P3B_DATARATE_FORCE) > 3))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_ISO14443P3B);
    }

    /* Cid */
    if (bCid > 14)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_PAL_ISO14443P3B);
    }

    if (bPupiLength != 0)
    {
        wStatus = phpalI14443p3b_WakeUpB(pDataParams, bNumSlots, bAfi, bExtAtqb, pLocalAtqb, &bLocalAtqbLen);
    }
    else
    {
        wStatus = phpalI14443p3b_RequestB(pDataParams, bNumSlots, bAfi, bExtAtqb, pLocalAtqb, &bLocalAtqbLen);
    }

    /* If the bNumSlots equals to zero, we do not send any SlotMarker commands (the following for loop). */
    if (bNumSlots != 0)
    {
        /* Calculate the number of slots reusing bNumSlots */
        bNumSlots = (uint8_t)(1 << bNumSlots);
    }

    for ( bSlot = 2; ; bSlot++ )
    {
        /* Integrity Error means collision */
        if ((wStatus & PH_ERR_MASK) == PH_ERR_INTEGRITY_ERROR)
        {
            *pMoreCardsAvailable = 0x01;
        }

        /* Update first error */
        if ((wStatusFirstError & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
        {
            wStatusFirstError = wStatus;
        }

        if ((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
        {
            /* if the caller provided a PUPI, we try to find and activate
             * only the according PICC */
            if (bPupiLength != 0)
            {
                /* compare PUPIs */
                if (memcmp(pPupi, pLocalPupi, 4) == 0)
                {
                    ++bPICCsFound;
                    break;
                }
            }
            /* No PUPI provided by caller */
            else
            {
                ++bPICCsFound;
                break;
            }
        }

        if (bSlot > bNumSlots)
        {
            break;
        }

        wStatus = phpalI14443p3b_SlotMarker(pDataParams, bSlot, pLocalAtqb, &bLocalAtqbLen);
    }

    /* Check if we found no PICC */
    if (!bPICCsFound)
    {
        return wStatusFirstError;
    }

    memcpy(pAtqb, pLocalAtqb, bLocalAtqbLen ); /* PRQA S 3200 */
    *pAtqbLen = bLocalAtqbLen;

    /* Find the baud rates which shall be used */
    if(pLocalPupi[0] == 0xAA)
    {
        bDri = pLocalPupi[1] & 0x03;
        bDsi = (pLocalPupi[1] & 0x0C) >> 2;
    }

    /* Find CID to be used */
    for(bSlot = 0; bSlot < 3; bSlot++)
    {
        if(pLocalPupi[bSlot] == 0xBB)
        {
            bCid = pLocalPupi[bSlot + 1];
            break;
        }
    }

    /* AttriB: activate the first PICC found */
    return phpalI14443p3b_Attrib(pDataParams, pAtqb, *pAtqbLen, bFsdi, bCid, bDri, bDsi, pMbli);
}

/**
* \brief Layer 4 Exchange
* This function performs Layer 4 Exchange.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t IBlockExchange(void)
{
    phStatus_t wStatus;
    uint16_t wDataIndex = 0;
    uint16_t wIndex = 0;
    uint16_t wOption = PH_EXCHANGE_DEFAULT;

    do
    {
        /* I-Block exchange */
        wStatus = phpalI14443p4_Exchange(
            pspalI14443p4,
            wOption,
            SOT,
            sizeof(SOT),
            &pRxData,
            &wRxDataLength);

        if (((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING) || ((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS))
        {
            for (wIndex = 0; wIndex < wRxDataLength; wIndex++)
            {
                aAppBuffer[wDataIndex++] = pRxData[wIndex];
            }
        }

        wOption = PH_EXCHANGE_RXCHAINING;

    } while((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING);

    /* Do loop-back, if success */
    while(((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
          && !(memcmp(pRxData, EOT, sizeof(EOT)) == 0))
    {
        /* Last 2 bytes "90 00" need not be transmitted */
        wTxDataLength = wDataIndex - 2;

        /* Loop-back received data */
        wStatus = phpalI14443p4_Exchange(
            pspalI14443p4,
            PH_EXCHANGE_DEFAULT,
            aAppBuffer,
            wTxDataLength,
            &pRxData,
            &wRxDataLength);
        CHECK_STATUS(wStatus);
    }

    /* Send DESELECT if EOT is received */
    if((wRxDataLength == sizeof(EOT))
       && (memcmp(pRxData, EOT, sizeof(EOT)) == 0))
    {
        wStatus = phpalI14443p4_Deselect(pspalI14443p4);
    }

    /* Reset Protocol */
    wStatus = phpalI14443p4_ResetProtocol(pspalI14443p4);
    CHECK_STATUS(wStatus);

    /* Return Status */
    return wStatus;
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
