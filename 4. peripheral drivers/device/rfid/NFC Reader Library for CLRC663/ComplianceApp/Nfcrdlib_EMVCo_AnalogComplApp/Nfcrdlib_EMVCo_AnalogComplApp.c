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
* Example Source for Nfcrdlib_EMVCo_AnalogComplApp.
* This application will configure Reader Library as per EMVCo DTE specification and start Emvco polling as per three mode of operation.
* 1.Loopback application -> As per Loopback Application
* 2.Transsend Type A -> As per Transaction Send Application
* 3.Transsend Type B -> As per Transaction Send Application
* This application will send a command list based on user selected mode of operation and is used to test EMVCo 3.1(L1) Analog compliance.
* Please refer Readme.txt file  for  Hardware Pin Configuration, Software Configuration and steps to build and
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

/* NFCLIB Header */
#include <phNfcLib.h>

/* Local headers */
#include "Nfcrdlib_EMVCo_AnalogComplApp.h"

	
/*******************************************************************************
**   Definitions
*******************************************************************************/

uint8_t  response_buffer[PHAC_EMVCO_MAX_BUFFSIZE];
uint16_t respsize = sizeof(response_buffer);

phNfcLib_PeerInfo_t    sNfcLib_Info; /* NFC LIB component holder */

#ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
uint32_t aEmvcoAnalogTaskBuffer[EMVCO_LOOPBACK_TASK_STACK];
#else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
#define aEmvcoAnalogTaskBuffer       NULL
#endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */

#ifdef PH_OSAL_FREERTOS
const uint8_t bTaskName[configMAX_TASK_NAME_LEN] = {"EmvcoAnalog"};
#else
const uint8_t bTaskName[] = {"EmvcoAnalog"};
#endif /* PH_OSAL_FREERTOS */

/*******************************************************************************
**   Static Defines
*******************************************************************************/
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50,
        0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31,
        0x00 };

static const uint8_t aTypeAFrames[] =
{ /* PROTOCOL_TYPE           , LENGTH, VALUES */
        PHHAL_HW_CARDTYPE_ISO14443A, 0x1, 0x52, \
        PHHAL_HW_CARDTYPE_ISO14443A, 0x4, 0x50, 0x0, 0x57, 0xcd, \
        PHHAL_HW_CARDTYPE_ISO14443B, 0x5, 0x5, 0x0, 0x8, 0x39, 0x73, \
        PHHAL_HW_CARDTYPE_ISO14443A, 0x1, 0x52, \
        PHHAL_HW_CARDTYPE_ISO14443A, 0x2, 0x93, 0x20, \
        PHHAL_HW_CARDTYPE_ISO14443A, 0x9, 0x93, 0x70, 0x27, 0xE9, 0x3B, 0x11, 0xE4, 0x53, 0x46, \
        PHHAL_HW_CARDTYPE_ISO14443A, 0x4, 0xE0, 0x80, 0x31, 0x73, \
        PHHAL_HW_CARDTYPE_ISO14443A, 23, 0x02, 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00, 0xE0, 0x42, \
        PHHAL_HW_CARDTYPE_ISO14443A, 21, 0x03, 0x00, 0xA4, 0x04, 0x00, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x00, 0x4D, 0xC9
};

static const uint8_t aTypeBFrames[] =
{ /* PROTOCOL_TYPE           , LENGTH, VALUES */
        PHHAL_HW_CARDTYPE_ISO14443B, 0x5, 0x5, 0x0, 0x8, 0x39, 0x73, \
        PHHAL_HW_CARDTYPE_ISO14443A, 0x1, 0x52, \
        PHHAL_HW_CARDTYPE_ISO14443B, 0x5, 0x5, 0x0, 0x8, 0x39, 0x73, \
        PHHAL_HW_CARDTYPE_ISO14443B, 11, 0x1d, 0x46, 0xb5, 0xc7, 0xa0, 0x00, 0x08, 0x01, 0x00, 0x69, 0xb5, \
        PHHAL_HW_CARDTYPE_ISO14443B, 23, 0x02, 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00, 0x2a, 0x2d, \
        PHHAL_HW_CARDTYPE_ISO14443B, 21, 0x03, 0x00, 0xA4, 0x04, 0x00, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x00, 0xAA, 0x2b
};

static volatile uint8_t bInfLoop = 1U;

/*******************************************************************************
 **   Prototypes
 *******************************************************************************/

void Emvco_Analog(void * pHal);
void Emvco_LoopBack(void * pHal);
void EmvcoTranssend_A_run(void * pHal);
void EmvcoTranssend_B_run(void * pHal);
static phNfcLib_Status_t emvco_cl_transaction_send(uint8_t const* pTransSendData, uint8_t length);
static phStatus_t EmvcoRfReset(uint32_t delayInMs);

/*******************************************************************************
**   Code
*******************************************************************************/

int main (void)
{
    do
    {
        phStatus_t        status;
        phNfcLib_Status_t dwStatus;

#ifdef PH_PLATFORM_HAS_ICFRONTEND
        phNfcLib_AppContext_t AppContext = {0};
#endif /* PH_PLATFORM_HAS_ICFRONTEND */

#ifndef PH_OSAL_NULLOS
        phOsal_ThreadObj_t EmvcoAnalog;
#endif /* PH_OSAL_NULLOS */

	


        /* Perform Controller specific initialization. */
        phApp_CPU_Init();


        /* Perform OSAL Initialization. */
        (void)phOsal_Init();

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

        pHal = phNfcLib_GetDataParams(PH_COMP_HAL);

        /* IRQ configuration for different HOST environments */
        status = phApp_Configure_IRQ();
        CHECK_STATUS(status);
        if(status != PH_ERR_SUCCESS) break;

        DEBUG_PRINTF("\n Emvco compliance example: ");

#ifndef PH_OSAL_NULLOS
        EmvcoAnalog.pTaskName = (uint8_t *)bTaskName;
        EmvcoAnalog.pStackBuffer = aEmvcoAnalogTaskBuffer;
        EmvcoAnalog.priority = EMVCO_ANALOG_TASK_PRIO;
        EmvcoAnalog.stackSizeInNum = EMVCO_ANALOG_TASK_STACK;
        phOsal_ThreadCreate(&EmvcoAnalog.ThreadHandle, &EmvcoAnalog,&Emvco_Analog, pHal);

        phOsal_StartScheduler();

        DEBUG_PRINTF("RTOS Error : Scheduler exited. \n");

#else
        Emvco_Analog(pHal);
#endif
    } while(0);

    while(bInfLoop); /* Comes here if initialization failure or scheduler exit due to error */

    return 0;
}

/*
 * This function displays the Main menu to the user through Console window .
 * It will wait for user to enter the selection and run the mode of operation accordingly as below
 *      1.EMVCo LoopBack Application
 *      2.Trans send Type A application
 *      3.Trans send Type B application
 *  \param   pHal      HAL data parameters
 *  \return None
 * */
void Emvco_Analog(void * pHal)
{
    uint32_t dwApplicationSelection=0;

    /* This call shall allocate secure context before calling any secure function,
     * when FreeRtos trust zone is enabled.
     * */
    phOsal_ThreadSecureStack( 512 );

    DEBUG_PRINTF("\n Emvco Analog compliance example: ");

    while (1)
    {
        DEBUG_PRINTF("\n 1.EMVCo Loopback App ");
        DEBUG_PRINTF("\n 2.EMVCo Trans Send App -Type A ");
        DEBUG_PRINTF("\n 3.EMVCo Trans Send App -Type B ");
        DEBUG_PRINTF("\n Select the DTE application type and press Enter: ");
        DEBUG_SCANF("%X", &dwApplicationSelection);

        switch (dwApplicationSelection) {
        case EMVCO_LOOPBACK_APP:
            Emvco_LoopBack(pHal);
            break;
        case EMVCO_TRANSSEND_TYPEA:
            EmvcoTranssend_A_run(pHal);
            break;
        case EMVCO_TRANSSEND_TYPEB:
            EmvcoTranssend_B_run(pHal);
            break;
        default:
            DEBUG_PRINTF("\n\tINVALID Entry ,Please provide the valid input again\n");
            break;
        }
    }

}
/**
 * \brief This Function will Perform EMVCo Loopback operation.
 * This Loopback Application converts each received R-APDU into the next C-APDU (by stripping the status words), and sends this C-APDU back to the card simulator via the PCD.
 * The PCD exits the loopback application when receiving the EOT (End Of Test) Command from the card simulator.
 * \param   pHal      HAL data parameters
 * \return None
 */
void Emvco_LoopBack(void * pHal)
{
    phNfcLib_Status_t dwStatus;
    uint16_t wRfTechnos = PH_NFCLIB_TECHNOLOGY_INITIATOR_ISO_14443_A | PH_NFCLIB_TECHNOLOGY_INITIATOR_ISO_14443_B;
    uint16_t cmdsize;
    uint8_t bEndOfLoopBack = 0;
    uint8_t bRemovalProcedure = PH_NFCLIB_DEACTIVATION_MODE_RF_OFF;
    uint8_t bEndOfTestAnalog = PH_OFF;

    DEBUG_PRINTF("\n Running Emvco Loopback Application  \n");

    /* Load Emvco Default setting */
    dwStatus = phNfcLib_SetConfig_Value(PH_NFCLIB_CONFIG_ACTIVATION_PROFILE,PH_NFCLIB_ACTIVATION_PROFILE_EMVCO);
    CHECK_NFCLIB_STATUS(dwStatus);

    dwStatus = phNfcLib_SetConfig_Value(PH_NFCLIB_CONFIG_ACTIVATION_BLOCKING,PH_ON);
    CHECK_NFCLIB_STATUS(dwStatus);

    while (1)
    {
        dwStatus = phNfcLib_Activate(wRfTechnos, &sNfcLib_Info, NULL);
        if (dwStatus == PH_NFCLIB_STATUS_PEER_ACTIVATION_DONE)
        {
            dwStatus = phNfcLib_Transmit(PPSE_SELECT_APDU,sizeof(PPSE_SELECT_APDU));
            if (dwStatus == PH_NFCLIB_STATUS_SUCCESS)
            {
                respsize = sizeof(response_buffer);
                dwStatus = phNfcLib_Receive(response_buffer, &respsize, NULL);
                if ((dwStatus == PH_NFCLIB_STATUS_SUCCESS) && (respsize > 0))
                {
                    while (!bEndOfLoopBack)
                    {
                        if (respsize >= MIN_VALID_DATA_SIZE)
                        {
                            /* EOT (End Of Test) Command. Exit the loop */
                            if (eEmdRes_EOT == response_buffer[1])
                            {
                                /* Second byte = 0x70, stop the loopback */
                                bEndOfLoopBack = 1;
                                bRemovalProcedure = PH_NFCLIB_DEACTIVATION_MODE_RELEASE;
                            }
                            /* EOT (End Of Test) Command used for analog tests. Exit the loop, perform long rf reset. */
                            else if (eEmdRes_EOT_AnalogTests == response_buffer[1])
                            {
                                /* Second byte = 0x72, stop the loopback */
                                bEndOfLoopBack = 1;
                                bEndOfTestAnalog = PH_ON;
                            }
                            else if (eEmdRes_SW_0 == response_buffer[respsize - 2])
                            {
                                /* Format the card response into a new command without the status word 0x90 0x00 */
                                cmdsize = respsize - 2; /* To Remove two bytes of status word */

                                /* Send back(Command) : Received Response - Status_Word */
                                dwStatus = phNfcLib_Transmit(response_buffer,cmdsize);
                                if (dwStatus == PH_NFCLIB_STATUS_SUCCESS)
                                {
                                    respsize = sizeof(response_buffer);
                                    dwStatus = phNfcLib_Receive(response_buffer,&respsize, NULL);
                                    if (dwStatus != PH_NFCLIB_STATUS_SUCCESS)
                                    {
                                        bEndOfLoopBack = 1;
                                    }
                                }
                                else
                                {
                                    bEndOfLoopBack = 1;
                                }
                            }
                            else
                            {
                                /* error Abort Loopback */
                                bEndOfLoopBack = 1;
                            }
                        }/*if (respsize >= MIN_VALID_DATA_SIZE)*/
                        else
                        {
                            /* re-send the select appli APDU */
                            dwStatus = phNfcLib_Transmit(PPSE_SELECT_APDU,sizeof(PPSE_SELECT_APDU));
                            if (dwStatus == PH_NFCLIB_STATUS_SUCCESS)
                            {
                                respsize = sizeof(response_buffer);
                                dwStatus = phNfcLib_Receive(response_buffer,&respsize, NULL);
                                if (dwStatus != PH_NFCLIB_STATUS_SUCCESS)
                                {
                                    bEndOfLoopBack = 1;
                                }
                            }
                            else
                            {
                                bEndOfLoopBack = 1;
                            }
                        }
                    }/*while (!bEndOfLoopBack)*/
                    bEndOfLoopBack = 0;

                    if (bEndOfTestAnalog == PH_ON)
                    {
                        /* analog tests - end of tests. long rf reset is required. */
                        EmvcoRfReset(25);
                        bEndOfTestAnalog = PH_OFF;
                    }
                }
            }
        }
        dwStatus = phNfcLib_SetConfig_Value(PH_NFCLIB_CONFIG_DEACTIVATION_BLOCKING, PH_ON);
        CHECK_NFCLIB_STATUS(dwStatus);

        dwStatus = phNfcLib_Deactivate(bRemovalProcedure, NULL);
        CHECK_NFCLIB_STATUS(dwStatus);

        bRemovalProcedure = PH_NFCLIB_DEACTIVATION_MODE_RF_OFF;
    }
}

/**
 * \brief This function will Perform Trans send Type A mode of operation.
 * \param   pHal      HAL data parameters
 * \return None
 */
void EmvcoTranssend_A_run(void * pHal)
{
    DEBUG_PRINTF("\n Running Emvco Transsend Type A Application  \n");

    phhalHw_ApplyProtocolSettings(pHal,PHHAL_HW_CARDTYPE_ISO14443A);
    phhalHw_FieldOn(pHal);

    while (1)
    {
        emvco_cl_transaction_send(aTypeAFrames, sizeof(aTypeAFrames));
    }

}

/**
 * \brief This function will Perform Trans send Type B mode of operation.
 * \param   pHal      HAL data parameters
 * \return None
 */
void EmvcoTranssend_B_run(void * pHal)
{
    DEBUG_PRINTF("\n Running Emvco Transsend Type B Application  \n");

    phhalHw_ApplyProtocolSettings(pHal,PHHAL_HW_CARDTYPE_ISO14443A);
    phhalHw_FieldOn(pHal);

    while (1)
    {
        emvco_cl_transaction_send(aTypeBFrames, sizeof(aTypeBFrames));
    }

}

/**
 * \brief This function will transmit Transsend command list for the Type of Protocol selected
 * PCD starts transmitting the commands defined below for the modulation type selected, with a 2 ms interval between the end of a command and the start of the next one.
 * At the end of the commands list, the PCD pauses for 100 ms and then restarts sending all the commands from the first one.
 * \param   pTransSendData      Array of Commands to be transmit.
 * \param   length              Length of the command array.
 * \return None
 */
static phNfcLib_Status_t emvco_cl_transaction_send(uint8_t const* pTransSendData, uint8_t length)
{
    uint8_t protocol;
    uint8_t bCmdIndex = 0;
    phNfcLib_Status_t dwStatus = PH_NFCLIB_STATUS_SUCCESS;
    uint8_t *   pRcvBuffer = 0;
    uint16_t    wRcvBytes;

#ifdef NXPBUILD__PHHAL_HW_PN5190
        /* Apply minimum FDT timeout as the default timeout is 5.1s. */
        phhalHw_SetConfig(pHal, PHHAL_HW_CONFIG_TIMEOUT_VALUE_US, 0x01U);
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

    for (bCmdIndex = 0; bCmdIndex < length;)
    {
        protocol = pTransSendData[bCmdIndex++];

        phhalHw_ApplyProtocolSettings(pHal,protocol);
        phhalHw_SetConfig(pHal,PHHAL_HW_CONFIG_TXCRC, PH_OFF);
        if(pTransSendData[bCmdIndex + 1]==0x52)
        {
            phhalHw_SetConfig(pHal,PHHAL_HW_CONFIG_TXLASTBITS, 0x7);
        }
        phhalHw_Exchange(pHal,0,(uint8_t* )&pTransSendData[bCmdIndex + 1], pTransSendData[bCmdIndex],&pRcvBuffer,&wRcvBytes);

        bCmdIndex += pTransSendData[bCmdIndex];
        bCmdIndex++;

#ifdef NXPBUILD__PHHAL_HW_PN5180
        /* No delay required */
#endif /* NXPBUILD__PHHAL_HW_PN5180 */

#ifdef NXPBUILD__PHHAL_HW_RC663
        /* No delay required */
#endif /* NXPBUILD__PHHAL_HW_RC663 */

#ifdef NXPBUILD__PHHAL_HW_PN7462AU
        //Introduce delay explicitly  to make sure that we follow  the Spec from EMVCO that 2ms delay between every command.
        phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 2);
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

#ifdef NXPBUILD__PHHAL_HW_PN5190
        //Introduce delay explicitly  to make sure that we follow  the Spec from EMVCO that 2ms delay between every command.
        phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 2);
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */
    }
    // Wait for 100ms Delay as per EMVCO Spec
    phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, 100);

    return dwStatus;
}

/**
 * \brief Perform RF Reset for the specified milli-seconds duration.
 * \return Status code
 * \retval #PH_ERR_SUCCESS Operation successful.
 * \retval Other Depending on implementation and underlying component.
 */
static phStatus_t EmvcoRfReset(uint32_t delayInMs)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    /*RF Field OFF*/
    wStatus = phhalHw_FieldOff(pHal);
    CHECK_STATUS(wStatus);

    wStatus = phhalHw_Wait(pHal,PHHAL_HW_TIME_MILLISECONDS, delayInMs);
    CHECK_STATUS(wStatus);

    /*RF Field ON*/
    wStatus = phhalHw_FieldOn(pHal);
    CHECK_STATUS(wStatus);

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
