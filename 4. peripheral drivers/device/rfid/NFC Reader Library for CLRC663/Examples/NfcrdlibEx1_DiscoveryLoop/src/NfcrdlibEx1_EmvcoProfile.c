/*----------------------------------------------------------------------------*/
/* Copyright 2020 NXP                                                         */
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
* Example Source for NfcrdlibEx1_EmvcoProfile.c that uses the Discovery loop implementation.
* Uses EMVCo POLL mode of discovery loop. Once a EMVCo Tag is been activated selected SELECT PPSE Exchange
* will be perform and print the Response of the same.
* This example will load/configure Discovery loop with user values based on EMVCo Profile via SetConfig.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <phApp_Init.h>
#include <NfcrdlibEx1_DiscoveryLoop.h>
#include <NfcrdlibEx1_EmvcoProfile.h>

#ifdef ENABLE_EMVCO_PROF
/*******************************************************************************
**   Static Defines
*******************************************************************************/

#define PRETTY_PRINTING                                         /**< Enable pretty printing */
#define MiN_VALID_DATA_SIZE                     6
#define PHAC_EMVCO_MAX_BUFFSIZE               600               /**< Maximum buffer size for Emvco. */
//#define RUN_TEST_SUIT

typedef enum{
    eEmdRes_EOT = 0x70,
    eEmdRes_SW_0 = 0x90,
    eEmdRes_SW_1 = 0x00,
}eEmvcoRespByte;


#ifdef RUN_TEST_SUIT
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };
#else
uint8_t PPSE_response_buffer[256];
uint8_t PPSE_respsize;
/*Define Macro for the PPSE command with different P2 value in PPSE command*/
#define PPSE_FCI
//#define PPSE_FMD
//#define PPSE_FCP
//#define PPSE_NO_LE


/* CLA = 0x00
 *  INS = 0xA4
 *  P1 = 0x04
 *  P2 = 0x00
 *  LC = 0x0E,
 *  DF = 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31
 *  LE = 0x00
 */
/*P2 parameter values of
 *  000000xxb = Return FCI template, optional use of FCI tag and length ,
 *  000001xxb = Return FCP template, mandatory use of FCP tag and length,
 *  000010xxb = Return FMD template, mandatory use of FMD tag and length,
 *  000011xxb = No response data if Le field absent, or proprietary if Le field present
 * */
#ifdef PPSE_FCI
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };
#endif
#ifdef PPSE_FCP
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x04, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };
#endif
#ifdef  PPSE_FMD
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x08, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00 };
#endif
#ifdef PPSE_NO_LE
/* EMVCo: Select PPSE Command */
static uint8_t PPSE_SELECT_APDU[] = { 0x00, 0xA4, 0x04, 0x0C, 0x0E, 0x32, 0x50, 0x41, 0x59,
        0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31 };
#endif
#endif
uint8_t command_buffer[PHAC_EMVCO_MAX_BUFFSIZE];
uint8_t *response_buffer;


static phStatus_t EmvcoDataExchange(uint8_t * com_buffer, uint8_t cmdsize, uint8_t ** resp_buffer, uint32_t * wRxLength);
static void EmvcoRfReset(phacDiscLoop_Sw_DataParams_t * pDataParams);
static phStatus_t EmvcoDataLoopBack(phacDiscLoop_Sw_DataParams_t * pDataParams);



void EmvcoProfileProcess (phacDiscLoop_Sw_DataParams_t * pDataParams,phStatus_t eDiscStatus)
{
	phStatus_t status = eDiscStatus;

	if((status & PH_ERR_MASK) == PHAC_DISCLOOP_DEVICE_ACTIVATED)
	{
		status = EmvcoDataLoopBack(pDataParams);
	}

	if((status & PH_ERR_MASK) != PHAC_DISCLOOP_NO_TECH_DETECTED)
	{
		/* Perform RF Reset */
		EmvcoRfReset(pDataParams);
	}
}



/**
* \brief Perform RF Reset as per Emvco Specification
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static void EmvcoRfReset(phacDiscLoop_Sw_DataParams_t * pDataParams)
{
    phStatus_t status = PH_ERR_SUCCESS;

    /*RF Field OFF*/
    status = phhalHw_FieldOff(pDataParams->pHalDataParams);
    CHECK_STATUS(status);

    status = phhalHw_Wait(pDataParams->pHalDataParams,PHHAL_HW_TIME_MICROSECONDS, 5100);
    CHECK_STATUS(status);

    /*RF Field ON*/
    status = phhalHw_FieldOn(pDataParams->pHalDataParams);
    CHECK_STATUS(status);

}

/**
* \brief EMVCo Loop-Back function
* This Loop-Back function converts each received R-APDU into the next C-APDU (by stripping the
* status words), and sends this C-APDU back to the card simulator.
* Also this function send SELECT_PPSE command after card activation.
* Loop-Back Function exist when EOT (End Of Test) Command is received from the card simulator.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t EmvcoDataLoopBack(phacDiscLoop_Sw_DataParams_t * pDataParams)
{
    uint32_t cmdsize, respsize;
    phStatus_t status;
    uint8_t bEndOfLoopBack = 0;
    uint8_t bRemovalProcedure = PH_OFF;
    cmdsize = sizeof(PPSE_SELECT_APDU);

    status = EmvcoDataExchange(PPSE_SELECT_APDU, cmdsize, &response_buffer, &respsize);

#ifndef RUN_TEST_SUIT

    /*Check if P1 is 0x04 which means that the data field consists of DF name */
    if(PPSE_SELECT_APDU[2] == 0x04)
    {
        DEBUG_PRINTF("\n DF Name: \n");
        /* DF Size = Total Command size - size of(PDU Header + Expected Len(Le))*/
        phApp_Print_Buff(&PPSE_SELECT_APDU[5], PPSE_SELECT_APDU[4]);
    }
    if (respsize > 0)
    {
        memcpy(&PPSE_response_buffer[0],response_buffer,respsize);
        DEBUG_PRINTF("\n SELECT_PPSE Res:\n");
        /* Status word removed */
        phApp_Print_Buff(PPSE_response_buffer, (respsize - 2));
        DEBUG_PRINTF("\nTransaction Done Remove card\n");
    }
    else
    {
        DEBUG_PRINTF("\nFCI not recieved\n");
#ifdef PPSE_NO_LE
        DEBUG_PRINTF("Transaction Done Remove card\n");
#else
        DEBUG_PRINTF("Transaction Failed Replace the card\n");
#endif
    }

#endif

    while (!bEndOfLoopBack)
    {
        if (respsize > 0)
        {
            if (respsize >= MiN_VALID_DATA_SIZE)
            {
                /* EOT (End Of Test) Command. Exit the loop */
                if (eEmdRes_EOT == response_buffer[1])
                {
                    /* Second byte = 0x70, stop the loopback */
                    bEndOfLoopBack = 1;
                    bRemovalProcedure = PH_ON;
                }
                else if (eEmdRes_SW_0 == response_buffer[respsize - 2])
                {
                    /* Format the card response into a new command without the status word 0x90 0x00 */
                    cmdsize = respsize - 2;  /* To Remove two bytes of status word */
                    memcpy(command_buffer, response_buffer, cmdsize);

                    /* Send back(Command) : Received Response - Status_Word */
                    status = EmvcoDataExchange(command_buffer, cmdsize, &response_buffer, &respsize);
                }
                else
                {
                    /* error Abort Loopback */
                    bEndOfLoopBack = 1;
                }
            }
            else/*if (respsize <6)*/
            {
                /* re-send the select appli APDU */
                status = EmvcoDataExchange(PPSE_SELECT_APDU, cmdsize, &response_buffer, &respsize);
                if (respsize == 0)
                {
                    bEndOfLoopBack = 1;
                }
            }
        }/*if(respsize > 0)*/
        else
        {
            bEndOfLoopBack = 1;
        }
    }/*while (!bEndOfLoopBack)*/

    if(bRemovalProcedure == PH_ON)
    {
        /* Set Poll state to perform Tag removal procedure*/
        status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_NEXT_POLL_STATE, PHAC_DISCLOOP_POLL_STATE_REMOVAL);
        CHECK_STATUS(status);

        status = phacDiscLoop_Run(pDataParams, PHAC_DISCLOOP_ENTRY_POINT_POLL);
    }
    return status;
}


/**
* \brief Exchange Data APDU Packets for EMVCO (ISO14443-4 Exchange)
* This function will Exchange APDU data packets provided by Loop-Back Application
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
static phStatus_t EmvcoDataExchange(uint8_t * com_buffer, uint8_t cmdsize, uint8_t ** resp_buffer, uint32_t * wRxLength)
{
    phStatus_t status;
    uint8_t *ppRxBuffer;
    uint16_t wRxLen = 0;

    status = phpalI14443p4_Exchange(phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P4), PH_EXCHANGE_DEFAULT,
    		com_buffer, cmdsize, &ppRxBuffer, &wRxLen);
    if (PH_ERR_SUCCESS == status)
    {
        /* set the pointer to the start of the R-APDU */
        *resp_buffer = &ppRxBuffer[0];
    }
    else
    {
        /* Exchange not successful, reset the number of rxd bytes */
        wRxLen = 0x00;
    }

    *wRxLength = wRxLen;

    return status;
}

#endif /* ENABLE_EMVCO_PROF */
