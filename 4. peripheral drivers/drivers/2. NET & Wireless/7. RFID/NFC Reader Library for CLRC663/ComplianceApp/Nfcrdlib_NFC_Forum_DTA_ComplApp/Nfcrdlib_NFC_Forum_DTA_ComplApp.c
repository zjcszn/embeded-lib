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
 * This file contains the Entry point for the DTA Test application.
 *
 * $Author: NXP $
 * $Revision: $ (v07.09.00)
 * $Date: $
 */

/**
* Reader Library Headers
*/
#include "phApp_Init.h"

/* Local headers */
#include "Nfcrdlib_NFC_Forum_DTA_ComplApp.h"

	

/*******************************************************************************
**   Definitions
*******************************************************************************/

#if defined (NXPBUILD__PHHAL_HW_RC663) && defined (NXPBUILD_DTA_CE_DEVICE_CLASS)
#   warning "This application is not supported for Card Emulation Device Class for RC663 frontend"
int main() {
    DEBUG_PRINTF("This application is not supported for Card Emulation Device Class for RC663 frontend.");
    while (1){

    }
    return 0;
}
/* Actual main defined below has no purpose in this example and hence over-riding it. */
#   define main dummy_main
#endif /* defined (NXPBUILD__PHHAL_HW_RC663) && defined (NXPBUILD_DTA_CE_DEVICE_CLASS) */

#ifdef PH_OSAL_FREERTOS
const uint8_t bTaskName[configMAX_TASK_NAME_LEN] = {"Dta_MainLoop"};
#else
const uint8_t bTaskName[] = {"Dta_MainLoop"};
#endif /* PH_OSAL_FREERTOS */

#ifdef NXPBUILD__PHPAL_I14443P3A_SW
phpalI14443p3a_Sw_DataParams_t * ppalI14443p3a;      /* PAL ISO I14443-A component */
#endif /* NXPBUILD__PHPAL_I14443P3A_SW */

#ifdef NXPBUILD__PHPAL_I14443P3B_SW
phpalI14443p3b_Sw_DataParams_t * ppalI14443p3b;      /* PAL ISO I14443-B component */
#endif /* NXPBUILD__PHPAL_I14443P3B_SW */

#ifdef NXPBUILD__PHPAL_I14443P4_SW
phpalI14443p4_Sw_DataParams_t * ppalI14443p4;        /* PAL ISO I14443-4 component */
#endif /* NXPBUILD__PHPAL_I14443P4_SW */

#ifdef NXPBUILD__PHPAL_I14443P4A_SW
phpalI14443p4a_Sw_DataParams_t * ppalI14443p4a;      /* PAL ISO I14443-4A component */
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */

#ifdef NXPBUILD__PHPAL_FELICA_SW
phpalFelica_Sw_DataParams_t * ppalFelica;            /* PAL Felica component */
#endif /* NXPBUILD__PHPAL_FELICA_SW */

#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
phpalI14443p4mC_Sw_DataParams_t * ppalI14443p4mC;    /* PAL ISO I14443-4mC Target component */
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

#ifdef NXPBUILD__PHAC_DISCLOOP_SW
phacDiscLoop_Sw_DataParams_t * pDiscLoop;            /* Discovery loop component */
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */

#ifdef NXPBUILD__PHAL_TOP_SW
phalTop_Sw_DataParams_t * palTop;                    /* AL Tag Operations component */
#endif /* NXPBUILD__PHAL_TOP_SW */

static volatile uint8_t bInfLoop = 1U;

/* *****************************************************************************************************************
 * Prototypes
 * ***************************************************************************************************************** */

/* RTOX Callback to be executed when RTOX Timer expires. */
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
void pWtoxCallBck(void);
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

/* *****************************************************************************************************************
 * Code
 * ***************************************************************************************************************** */

int main(void)
{
    do
    {
        phStatus_t status = PH_ERR_SUCCESS;
        phNfcLib_Status_t dwStatus;
        phOsal_ThreadObj_t sPhDta_MainLoop_ThreadObj;
        phNfcLib_AppContext_t AppContext = {0};

	


        /* Perform Controller specific initialization. */
        phApp_CPU_Init();


        /* Perform OSAL Initialization. */
        (void)phOsal_Init();

        /* Initialize running status indication mechanism (LED/GPIO) */
        phDta_OutputInit();

        DEBUG_PRINTF("\nNFC Forum DTA Application\n");

#ifdef PH_PLATFORM_HAS_ICFRONTEND
        status = phbalReg_Init(&sBalParams, sizeof(phbalReg_Type_t));
        CHECK_STATUS(status);

        AppContext.pBalDataparams = &sBalParams;
#endif
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
        AppContext.pWtxCallback = (void *)pWtoxCallBck;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */
        dwStatus = phNfcLib_SetContext(&AppContext);
        CHECK_NFCLIB_STATUS(dwStatus);

        /* Initialize library */
        dwStatus = phNfcLib_Init();
        CHECK_NFCLIB_STATUS(dwStatus);
        if(dwStatus != PH_NFCLIB_STATUS_SUCCESS)
        {
            DEBUG_PRINTF("\nNfcLib Initialization Failed!");
            break;
        }

        /* Set the generic pointer */
        pHal = phNfcLib_GetDataParams(PH_COMP_HAL);
        pDiscLoop = phNfcLib_GetDataParams(PH_COMP_AC_DISCLOOP);
#ifdef NXPBUILD_DTA_READER_DEVICE_CLASS
        ppalI14443p3a = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3A);
        ppalI14443p3b = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3B);
        ppalI14443p4 = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P4);
        ppalI14443p4a = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P4A);
        ppalFelica = phNfcLib_GetDataParams(PH_COMP_PAL_FELICA);
        palTop = phNfcLib_GetDataParams(PH_COMP_AL_TOP);
#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */
#if defined(NXPBUILD_DTA_CE_DEVICE_CLASS) && !defined(NXPBUILD__PHHAL_HW_RC663)
        ppalI14443p4mC = phNfcLib_GetDataParams(PH_COMP_PAL_I14443P4MC);
#endif /* defined(NXPBUILD_DTA_CE_DEVICE_CLASS) && !defined(NXPBUILD__PHHAL_HW_RC663) */

        /* Perform Platform Initialization */
        status = phApp_Configure_IRQ();
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nIRQ Configuration Failed!");
            break;
        }

        /* Create the task for the Main Loop of the DTA Application */
        sPhDta_MainLoop_ThreadObj.pTaskName = (uint8_t *)bTaskName;
        sPhDta_MainLoop_ThreadObj.priority = PHDTA_MAINLOOP_TASKPRIORITY;
        sPhDta_MainLoop_ThreadObj.stackSizeInNum = PHDTA_MAINLOOP_STACKSIZE;
        sPhDta_MainLoop_ThreadObj.pStackBuffer = aDTAMainTaskBuffer;

        status = phOsal_ThreadCreate(&sPhDta_MainLoop_ThreadObj.ThreadHandle, &sPhDta_MainLoop_ThreadObj, &phDta_MainLoop, pHal);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nMain Loop Task Create Failed!");
            break;
        }

        phOsal_StartScheduler();

        DEBUG_PRINTF("RTOS Error : Scheduler exited. \n");

    } while (0);

    while(bInfLoop); /* Comes here if initialization failure or scheduler exit due to error */

    return 0;
}

/* DTA Main Loop implementation API */
void phDta_MainLoop (void *pDataParams)
{
    phStatus_t status;

    /* This call shall allocate secure context before calling any secure function,
     * when FreeRtos trust zone is enabled.
     * */
    phOsal_ThreadSecureStack( 512 );

    /* Run DTA Input Module */
    status = phDta_GetUserInput();
    if(status != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_MainLoop: Getting User Input Failed!");
    }

    /* Initialization of the Input Engine Module */
    status = phDta_InputEngineInit();
    if(status != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_MainLoop: Input Engine Initialization Failed!");
    }

#ifdef NXPBUILD__PHHAL_HW_TARGET
    /* Configure HAL target parameters */
    status = phApp_HALConfigAutoColl();
    if(status != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_MainLoop: Initializing HAL Target Parameters Failed!");
    }
#endif /* NXPBUILD__PHHAL_HW_TARGET */

    /* Configure discover loop based on DTA application requirement. */
    status = phDta_DiscLoopConfig();
    if(status != PH_ERR_SUCCESS)
    {
        DEBUG_PRINTF("\nphDta_MainLoop: Initializing Discovery Loop Failed!");
    }

    DEBUG_PRINTF("\nDTA Main Loop Begins\n");

    /* Indicate to the user that DTA loop has started */
    phDta_IndicateTestRunning();

    while(1)
    {
#ifdef NXPBUILD_DTA_READER_DEVICE_CLASS
        /* Run Poll Mode Test Procedure */
        phDta_PollMode();

        /* Run Input Engine */
        phDta_InputEngine();
#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */

        /* Run Listen Mode Test Procedure */
#if defined(NXPBUILD_DTA_CE_DEVICE_CLASS) && !defined(NXPBUILD__PHHAL_HW_RC663)
        phDta_ListenMode();
#endif /* defined(NXPBUILD_DTA_CE_DEVICE_CLASS) && !defined(NXPBUILD__PHHAL_HW_RC663) */
    }
}

/* API to configure the Discovery Loop parameters as per DTA requirement */
phStatus_t phDta_DiscLoopConfig(void)
{
    phStatus_t status = PH_ERR_SUCCESS;

    do
    {
        /* Configure operation mode of discovery loop */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_OPE_MODE, RD_LIB_MODE_NFC);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: set NFC Mode Failed!");
            break;
        }

#ifdef NXPBUILD_DTA_READER_DEVICE_CLASS
        /* Configure BAIL out for Polling mode */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_BAIL_OUT,
           (PHAC_DISCLOOP_POS_BIT_MASK_A |
            PHAC_DISCLOOP_POS_BIT_MASK_B |
            PHAC_DISCLOOP_POS_BIT_MASK_F212 |
            PHAC_DISCLOOP_POS_BIT_MASK_F424) |
            PHAC_DISCLOOP_POS_BIT_MASK_V);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: BailOut Configuration Failed!");
            break;
        }

        /* Configure Type A CON_DEVICES_LIMIT to 2 */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_DEVICE_LIMIT, PHDTA_CON_DEVICES_LIMIT);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: Type A Device limit Configuration Failed!");
            break;
        }

        /* Configure Type B CON_DEVICES_LIMIT to 2 */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_DEVICE_LIMIT, PHDTA_CON_DEVICES_LIMIT);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: Type B Device limit Configuration Failed!");
            break;
        }

        /* Configure Type F CON_DEVICES_LIMIT to 2 */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEF_DEVICE_LIMIT, PHDTA_CON_DEVICES_LIMIT);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: Type F Device limit Configuration Failed!");
            break;
        }

        /* Configure Type V CON_DEVICES_LIMIT to 2 */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEV_DEVICE_LIMIT, PHDTA_CON_DEVICES_LIMIT);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: Type V Device limit Configuration Failed!");
            break;
        }

        /* Frame Size Device Integer (FSDI) for the 14443P4A tags in passive poll mode */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_FSDI, PHDTA_TYPEA_I3P4_RATS_FSDI);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: FSDI Configuration Failed!");
            break;
        }

        /* Card Identifier (CID) for the 14443P4A tags in passive poll mode */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEA_I3P4_CID, PHDTA_TYPEA_I3P4_RATS_CID);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: CID Configuration Failed!");
            break;
        }

        /* Frame Size Device Integer (FSDI) value for the Type B tags in passive poll mode*/
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_FSDI, PHDTA_TYPEB_ATTRIB_FSDI);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: FSDI Configuration Failed!");
            break;
        }

        /* Card Identifier (CID) value for the Type B tags in passive poll mode */
        status = phacDiscLoop_SetConfig(pDiscLoop, PHAC_DISCLOOP_CONFIG_TYPEB_CID, PHDTA_TYPEB_ATTRIB_CID);
        if(status != PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nphDta_DiscLoopInit: CID Configuration Failed!");
            break;
        }

        /* Assign ATS buffer for Type A */
        pDiscLoop->sTypeATargetInfo.sTypeA_I3P4.pAts = gPhDta_ActResponse;
#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */
    }while(0);

    return status;
}

/*
 * WTX Callback called from WTX timer of 14443p3mC PAL.
 */
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
void pWtoxCallBck(void)
{
   (void)phOsal_EventPost(ppalI14443p4mC->mcEventObj.EventHandle, E_OS_EVENT_OPT_POST_ISR, E_PH_OSAL_EVT_RTO_TIMEOUT, NULL);
}
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */
