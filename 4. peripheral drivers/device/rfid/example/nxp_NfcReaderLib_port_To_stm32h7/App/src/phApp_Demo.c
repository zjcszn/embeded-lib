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

/** \file
* Example Source for NfcrdlibEx1_DiscoveryLoop that uses the Discovery loop implementation.
* By default Discovery Loop will work as per NFC Forum Activity Specification v2.2
* which will configure the Reader in both POLL and LISTEN (only for Universal device)
* modes of discovery loop.Displays detected tag information(like UID, SAK, Product Type)
* and prints information when it gets activated as a target by an external Initiator/reader.
*
* By enabling "ENABLE_DISC_CONFIG" macro, few of the most common Discovery Loop configuration
* are been updated to values defined in this Example.
* By enabling "ENABLE_EMVCO_PROF", Discovery Loop will be configured as per EMVCo Polling
* specification else the Discovery Loop will still be configured to NFC Forum but user defined
* values as per this Application.
*
* NFC Forum Mode: Whenever multiple technologies are detected, example will select first
* detected technology to resolve. Example will activate device at index zero whenever multiple
* device is detected.
*
* For EMVCo profile, this example provide full EMVCo digital demonstration along with option to
* use different SELECT PPSE Commands.
*
* Please refer Readme.txt file for Hardware Pin Configuration, Software Configuration and steps to build and
* execute the project which is present in the same project directory.
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
#include <phApp_Demo.h>
#include <stdlib.h>


/*******************************************************************************
**   Definitions
*******************************************************************************/

#define NUMBER_OF_KEYENTRIES        2
#define NUMBER_OF_KEYVERSIONPAIRS   2
#define NUMBER_OF_KUCENTRIES        1

#define DATA_BUFFER_LEN             16 /* Buffer length */
#define MFC_BLOCK_DATA_SIZE         16 /* Block Data size - 16 Bytes */

phacDiscLoop_Sw_DataParams_t *pDiscLoop;       /* Discovery loop component */
void *psKeyStore;
void *psalMFC;

/*The below variables needs to be initialized according to example requirements by a customer */
uint8_t bDataBuffer[DATA_BUFFER_LEN];  /* universal data buffer */

/*PAL variables*/
phKeyStore_Sw_KeyEntry_t sKeyEntries[NUMBER_OF_KEYENTRIES];                                  /* Sw KeyEntry structure */
phKeyStore_Sw_KUCEntry_t sKUCEntries[NUMBER_OF_KUCENTRIES];                                  /* Sw Key usage counter structure */
phKeyStore_Sw_KeyVersionPair_t sKeyVersionPairs[NUMBER_OF_KEYVERSIONPAIRS * NUMBER_OF_KEYENTRIES]; /* Sw KeyVersionPair structure */

/* Set the key for the MIFARE (R) Classic cards. */
uint8_t Key[12] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

#ifdef NXPBUILD__PH_LOG
phLog_RegisterEntry_t phLogRegisterEntry[4];
phLog_LogEntry_t phLogLogEntry[16];
#endif

uint32_t aDiscTaskBuffer[DISC_DEMO_TASK_STACK];
const uint8_t bTaskName[configMAX_TASK_NAME_LEN] = {"DiscLoop"};


/*******************************************************************************
**   Static Defines
*******************************************************************************/


/*******************************************************************************
**   Prototypes
*******************************************************************************/

void DiscoveryLoop_Task(void *pDataParams);

/*******************************************************************************
**   Code
*******************************************************************************/
phStatus_t phApp_ReadUID(void) {
    phStatus_t status = PH_ERR_SUCCESS;
    uint8_t aAttriCmd[9] = {0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x08};
    uint8_t aQueryCmd[5] = {0x00, 0x36, 0x00, 0x00, 0x08};
    uint8_t *pResp = NULL;
    uint16_t wRespLength = 0;
    uint8_t bAtqbLen = 0;
    uint8_t aAtqb[13];
    uint8_t bErrCnt = 0;
    phpalI14443p3b_Sw_DataParams_t *psal14443b = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3B);
    
    while (bErrCnt < 10) {
        
        phhalHw_FieldReset(pHal);
        
        status = phpalI14443p3b_RequestB(psal14443b, 0, 0, 0, aAtqb, &bAtqbLen);
        if ((status != PH_ERR_SUCCESS) || ((bAtqbLen != 12) && (bAtqbLen != 13))) {
            bErrCnt++;
            continue;
        }
        
        status = phpalI14443p3b_Exchange(psal14443b, PH_EXCHANGE_DEFAULT, aAttriCmd, 9, &pResp, &wRespLength);
        if ((status != PH_ERR_SUCCESS) || (wRespLength != 1) || (pResp[0] != 0x08)) {
            bErrCnt++;
            continue;
        }
        
        status = phhalHw_Exchange(psal14443b->pHalDataParams, PH_EXCHANGE_DEFAULT, aQueryCmd, 5, &pResp, &wRespLength);
        if ((status != PH_ERR_SUCCESS) || (wRespLength != 10) || (pResp[8] != 0x90) || (pResp[9] != 0x00)) {
            bErrCnt++;
            continue;
        }
        
        DEBUG_PRINTF("UID: ");
        phApp_Print_Buff(pResp, 8);
        DEBUG_PRINTF("\r\n");
        break;
    }
    
    return status;
}

phStatus_t phApp_MifareClassics(void) {
    phStatus_t status;
    uint8_t bUid[PHAC_DISCLOOP_I3P3A_MAX_UID_LENGTH];
    uint8_t bUidSize;
    
    do {
        /* Print UID */
        DEBUG_PRINTF("\nUID: ");
        phApp_Print_Buff(pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].aUid,
                         pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].bUidSize);
        
        /* Print ATQA  and SAK */
        DEBUG_PRINTF("\nATQA:");
        phApp_Print_Buff(pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa, 2);
        DEBUG_PRINTF("\nSAK: 0x%x", pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].aSak);
        
        /* Print Product type */
        DEBUG_PRINTF("\nProduct: MIFARE Classic \n");
        
        bUidSize = pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].bUidSize;
        memcpy(bUid, pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].aUid, bUidSize);
        
        /* Authenticate with the Key
         * We can authenticate at any block of a sector and we will get the access to all blocks of the same sector
         * For example authenticating at block 5, we will get the access to blocks 4, 5, 6 and 7.
         */
        /* Send authentication for block 6 */
        status = phalMfc_Authenticate(psalMFC, 6, PHHAL_HW_MFC_KEYA, 1, 0, bUid, bUidSize);
        
        /* Check for Status */
        if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS) {
            /* Print Error info */
            DEBUG_PRINTF("\nAuthentication Failed!!!");
            DEBUG_PRINTF("\nPlease correct the used key");
            DEBUG_PRINTF("\nExecution aborted!!!\n");
            break;
        }
        
        DEBUG_PRINTF("\nAuthentication Successful");
        
        /* Empty the bDataBuffer */
        memset(bDataBuffer, '\0', DATA_BUFFER_LEN);
        
        DEBUG_PRINTF("\nRead data from Block 4");
        
        /* Read data from block 4 */
        status = phalMfc_Read(psalMFC, 6, bDataBuffer);
        
        /* Check for Status */
        if (status != PH_ERR_SUCCESS) {
            /* Print Error info */
            DEBUG_PRINTF("\nRead operation failed!!!\n");
            DEBUG_PRINTF("\nExecution aborted!!!\n\n");
            break; /* Break from the loop*/
        }
        
        DEBUG_PRINTF("\nRead Success");
        DEBUG_PRINTF("\nThe content of Block 4 is:\n");
        
        phApp_Print_Buff(&bDataBuffer[0], MFC_BLOCK_DATA_SIZE);
        DEBUG_PRINTF("\n\n --- End of Read Operation --- \n");
        
        DEBUG_PRINTF("\nWrite data to Block 4 \n");
        
        /* Write data to block 4 */
        for (int i = 0; i < DATA_BUFFER_LEN; i++) {
            bDataBuffer[i] = 0xFF;
        }
        bDataBuffer[6] = 0xFF;
        bDataBuffer[7] = 0X07;
        bDataBuffer[8] = 0x80;
        bDataBuffer[9] = 0x69;
        SCB_CleanDCache_by_Addr((uint32_t *)bDataBuffer, DATA_BUFFER_LEN);
        
        phApp_Print_Buff(&bDataBuffer[0], MFC_BLOCK_DATA_SIZE);
        
        status = phalMfc_Write(psalMFC, 7, bDataBuffer);
        
        /* Check for Status */
        if (status != PH_ERR_SUCCESS) {
            /* Print Error info */
            DEBUG_PRINTF("\nWrite operation failed!!!\n");
            DEBUG_PRINTF("\nExecution aborted!!!\n");
            break; /* Break from the loop*/
        }
        
        DEBUG_PRINTF("\nWrite Success");
        DEBUG_PRINTF("\n\n --- End of Write Operation --- ");
        
        /* End of example */
        DEBUG_PRINTF("\n\n --- End of Example --- \n\n");
        
    } while (0);
    
    return status;
}

/***********************************************************************************************
 * \brief   Initializes the discovery loop and keystore components required by Example-4
 * \param   none
 * \return  status  Returns the function status
 **********************************************************************************************/
static phStatus_t Ex4_NfcRdLibInit(void) {
    phStatus_t status;
    
    psKeyStore = phNfcLib_GetDataParams(PH_COMP_KEYSTORE);
    psalMFC = phNfcLib_GetDataParams(PH_COMP_AL_MFC);
    
    /* Device limit for Type A */
    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_DEVICE_LIMIT, 1);
    CHECK_STATUS(status);
    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTA_VALUE_US, 5100);
    CHECK_STATUS(status);
    
    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DEVICE_LIMIT, 1);
    CHECK_STATUS(status);
    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_GTB_VALUE_US, 5100);
    CHECK_STATUS(status);
    
    status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_BAIL_OUT, PHAC_DISCLOOP_POS_BIT_MASK_A);
    CHECK_STATUS(status);
    
    /* Initialize the keystore component */
    status = phKeyStore_Sw_Init(
        psKeyStore,
        sizeof(phKeyStore_Sw_DataParams_t),
        &sKeyEntries[0],
        NUMBER_OF_KEYENTRIES,
        &sKeyVersionPairs[0],
        NUMBER_OF_KEYVERSIONPAIRS,
        &sKUCEntries[0],
        NUMBER_OF_KUCENTRIES);
    CHECK_STATUS(status);
    
    /* load a Key to the Store */
    /* Note: If You use Key number 0x00, be aware that in SAM
      this Key is the 'Host authentication key' !!! */
    status = phKeyStore_FormatKeyEntry(psKeyStore, 1, PH_KEYSTORE_KEY_TYPE_MIFARE);
    CHECK_STATUS(status);
    
    /* Set Key Store */
    status = phKeyStore_SetKey(psKeyStore, 1, 0, PH_KEYSTORE_KEY_TYPE_MIFARE, &Key[0], 0);
    CHECK_STATUS(status);
    
    /* Read the version of the reader IC */
    status = phhalHw_Rc663_ReadRegister(pHal, PHHAL_HW_RC663_REG_VERSION, &bDataBuffer[0]);
    CHECK_STATUS(status);
    DEBUG_PRINTF("\nReader chip RC663: 0x%02x\n", bDataBuffer[0]);
    
    /* Return Success */
    return status;
}

phStatus_t phApp_Discovery_Loop(void) {
    phStatus_t status = PH_ERR_INTERNAL_ERROR;
    phNfcLib_Status_t dwStatus;
    phNfcLib_AppContext_t AppContext = {0};
    phOsal_ThreadObj_t DiscLoop;
    
    do {
        /* Perform Controller specific initialization. */
        phApp_CPU_Init();
        
        /* Perform OSAL Initialization. */
        (void)phOsal_Init();
        
        DEBUG_PRINTF("\nDiscoveryLoop Example: \n");
        
        status = phbalReg_Init(&sBalParams, sizeof(sBalParams));
        CHECK_STATUS(status);
        
        AppContext.pBalDataparams = &sBalParams;
        dwStatus = phNfcLib_SetContext(&AppContext);
        CHECK_NFCLIB_STATUS(dwStatus);
        
        /* Initialize library */
        dwStatus = phNfcLib_Init();
        CHECK_NFCLIB_STATUS(dwStatus);
        
        /* Set the generic pointer */
        pHal = phNfcLib_GetDataParams(PH_COMP_HAL);
        pDiscLoop = phNfcLib_GetDataParams(PH_COMP_AC_DISCLOOP);
        
        /* Initialize other components that are not initialized by NFCLIB and configure Discovery Loop. */
        status = phApp_Comp_Init(pDiscLoop);
        CHECK_STATUS(status);

#ifdef NXPBUILD__PH_LOG
        phLog_Init(phApp_Log, phLogRegisterEntry, 4);
        phLog_Register(phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3B), &phLogLogEntry[0], 16);
#endif
        status = Ex4_NfcRdLibInit();
        CHECK_STATUS(status);
        
        /* Perform Platform Init */
        status = phApp_Configure_IRQ();
        CHECK_STATUS(status);
        
        DiscLoop.pTaskName = (uint8_t *)bTaskName;
        DiscLoop.pStackBuffer = aDiscTaskBuffer;
        DiscLoop.priority = DISC_DEMO_TASK_PRIO;
        DiscLoop.stackSizeInNum = DISC_DEMO_TASK_STACK;
        phOsal_ThreadCreate(&DiscLoop.ThreadHandle, &DiscLoop, &DiscoveryLoop_Task, pDiscLoop);
        phOsal_StartScheduler();
        
    } while (0);
    
    return status;
}

/**
* This function demonstrates the usage of discovery loop.
* The discovery loop can run with default setting Or can be configured as demonstrated and
* is used to detects and reports the NFC technology type.
* \param   pDataParams      The discovery loop data parameters
* \note    This function will never return
*/
void DiscoveryLoop_Task(void *pDataParams) {
    phStatus_t status;
    uint16_t wTagsDetected = 0;
    uint16_t wNumberOfTags = 0;
    uint8_t bAtqbLen;
    uint8_t aAtqb[13];
    
    /* This call shall allocate secure context before calling any secure function,
     * when FreeRtos trust zone is enabled.
     * */
    phOsal_ThreadSecureStack(512);
    
    while (1) {
        DEBUG_PRINTF("\nReady to detect...\n");
        
        do {
            /* Field OFF */
            status = phhalHw_FieldOff(pHal);
            CHECK_STATUS(status);
            
            status = phhalHw_Wait(pDiscLoop->pHalDataParams, PHHAL_HW_TIME_MICROSECONDS, 100);
            CHECK_STATUS(status);
            
            /* Configure Discovery loop for Poll Mode */
            status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_DETECTION);
            CHECK_STATUS(status);
            
            /* Run Discovery loop */
            status = phacDiscLoop_Run(pDiscLoop, PHAC_DISCLOOP_ENTRY_POINT_POLL);
            
        } while ((status & PH_ERR_MASK) != PHAC_DISCLOOP_DEVICE_ACTIVATED); /* Exit on Card detection */
        
        /* Card detected */
        /* Get the tag types detected info */
        status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TECH_DETECTED, &wTagsDetected);
        CHECK_STATUS(status);
        status = phacDiscLoop_GetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_NR_TAGS_FOUND, &wNumberOfTags);
        CHECK_STATUS(status);
        
        DEBUG_PRINTF (" \n Card detected and activated successfully... \n");
        phApp_PrintTagInfo(pDiscLoop, wNumberOfTags, wTagsDetected);
        
        /* Check for Status */
        if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS) {
            /* Check for Type A tag detection */
            if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_A)) {
                /* Check for MIFARE Classic */
                if (0x08 == (pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].aSak & 0x08)) {
                    
                    phApp_MifareClassics();
                    DEBUG_PRINTF("\nPlease Remove the Card\n\n");
                    
                    /* Field RESET */
                    status = phhalHw_FieldReset(pHal);
                    CHECK_STATUS(status);
                }
                
                /* Field RESET */
                status = phhalHw_FieldReset(pHal);
                CHECK_STATUS(status);
                
                /* Make sure that example application is not detecting the same card continuously */
                do {
                    /* Send WakeUpA */
                    status = phpalI14443p3a_WakeUpA(pDiscLoop->pPal1443p3aDataParams,
                                                    pDiscLoop->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa);
                    
                    /* Check for Status */
                    if ((status & PH_ERR_MASK) == (PH_ERR_IO_TIMEOUT)) {
                        DEBUG_PRINTF("\nCard Removed, status = %04X\n", status);
                        break; /* Card Removed, break from the loop */
                    }
                    
                    /* Send HaltA */
                    status = phpalI14443p3a_HaltA(pDiscLoop->pPal1443p3aDataParams);
                    CHECK_STATUS(status);
                    
                    /* Delay - 5 milli seconds*/
                    status = phhalHw_Wait(pDiscLoop->pHalDataParams, PHHAL_HW_TIME_MILLISECONDS, 5);
                    CHECK_STATUS(status);
                    
                } while (1);
                
            } else if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_POS_BIT_MASK_B)) {
                
                phApp_ReadUID();
                DEBUG_PRINTF("\nPlease Remove the Card\n\n");
                
                /* Field RESET */
                status = phhalHw_FieldReset(pHal);
                CHECK_STATUS(status);
                /* Make sure that example application is not detecting the same card continuously */
                do {
                    /* Send WakeUpA */
                    status = phpalI14443p3b_WakeUpB(pDiscLoop->pPal1443p3bDataParams, 0, 0, 0, aAtqb, &bAtqbLen);
                    
                    /* Check for Status */
                    if ((status & PH_ERR_MASK) == (PH_ERR_IO_TIMEOUT)) {
                        DEBUG_PRINTF("\nCard Removed, status = %04X\n", status);
                        break; /* Card Removed, break from the loop */
                    }
                    
                    /* Send HaltA */
                    status = phpalI14443p3b_HaltB(pDiscLoop->pPal1443p3bDataParams);
                    CHECK_STATUS(status);
                    
                    /* Delay - 5 milli seconds*/
                    status = phhalHw_Wait(pDiscLoop->pHalDataParams, PHHAL_HW_TIME_MILLISECONDS, 5);
                    CHECK_STATUS(status);
                    
                } while (1);
            }
            
        }
    }
}


