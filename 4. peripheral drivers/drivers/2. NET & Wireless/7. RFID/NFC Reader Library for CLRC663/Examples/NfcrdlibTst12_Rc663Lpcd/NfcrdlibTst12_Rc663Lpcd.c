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
*
* Please refer Readme.txt file for hardware pin configuration, software configuration and steps to build and
* execute the project.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

/**
* Reader Library Headers
*/

/* Local headers */
#include "NfcrdlibTst12_Rc663Lpcd.h"
#include "phDriver.h"
#include "BoardSelection.h"
#include "phhalHw.h"

/* Check for LPC1769 controller based boards. */
#if defined(PHDRIVER_LPC1769RC663_BOARD)
#include "board.h"
#define PHDRIVER_LPC1769
#endif

/*******************************************************************************
**   Global Defines
*******************************************************************************/
/*HAL variables*/
uint8_t                     bHalBufferTx[260];     /* HAL TX buffer. Size 260 */
uint8_t                     bHalBufferRx[260];     /* HAL RX buffer. Size 260 */

phbalReg_Type_t sBalParams;

#ifdef NXPBUILD__PHHAL_HW_RC663
phhalHw_Rc663_DataParams_t sHalParams;
#endif

uint8_t                            bDataBuffer;

/*******************************************************************************
**   Extern Defines
*******************************************************************************/

/*******************************************************************************
**   Const Defines
*******************************************************************************/

/*******************************************************************************
**   Static Defines
*******************************************************************************/
#ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
uint32_t ExLpcdTaskBuffer[EX11_LPCD_TASK_STACK];
#else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
#define ExLpcdTaskBuffer         NULL
#endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */

#define PH_CONFIG_DONT_CARE        PH_OFF
#define PH_CONFIG_DISABLE          PH_OFF
#define PH_CONFIG_ENABLE           PH_ON

#ifdef   DEBUG
    #include <stdio.h>
    #define  DEBUG_PRINTF(...) printf(__VA_ARGS__); fflush(stdout)
    #define  DEBUG_SCANF(...) fflush(stdin); scanf(__VA_ARGS__); fflush(stdin)
#else /* DEBUG */
    #define  DEBUG_PRINTF(...)
    #define  DEBUG_SCANF(...)
#endif /* DEBUG */

#define CHECK_STATUS(x)                                      \
    if ((x) != PH_ERR_SUCCESS)                               \
{                                                            \
    DEBUG_PRINTF("Line: %d   Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n", __LINE__, (x));    \
}

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
static phStatus_t phApp_Clif_Init( uint8_t *pTxBuffer, uint16_t wTxBufSize, uint8_t *pRxBuffer, uint16_t wRxBufSize);

/*******************************************************************************
**   Extern function for RTOX
**   This example will not need RTOX and the function implementation is empty.
*******************************************************************************/

/*******************************************************************************
**   Main Function
*******************************************************************************/
int main (void)
{
    do
    {
        phStatus_t status = PH_ERR_INTERNAL_ERROR;
#ifndef PH_OSAL_NULLOS
        phOsal_ThreadObj_t Tst12LpcdThreadObj;
#endif

        /* Perform OSAL Initialization. */
        (void)phOsal_Init();

        DEBUG_PRINTF("**********Rc663 Lpcd Example**********\n");

        status = phbalReg_Init(&sBalParams, sizeof(phbalReg_Type_t));
        CHECK_STATUS(status);

        /* Perform Platform Init */
        status = phApp_Clif_Init(bHalBufferTx, sizeof(bHalBufferTx), bHalBufferRx, sizeof(bHalBufferRx));
        CHECK_STATUS(status);
        if(status != PH_ERR_SUCCESS) break;

#ifdef PH_OSAL_NULLOS
        Tst12_LpcdThread(NULL);
#else
        Tst12LpcdThreadObj.pTaskName = (uint8_t *) "Ex12Lpcd";
        Tst12LpcdThreadObj.pStackBuffer = ExLpcdTaskBuffer;
        Tst12LpcdThreadObj.priority = EX11_LPCD_TASK_PRIO;
        Tst12LpcdThreadObj.stackSizeInNum = EX11_LPCD_TASK_STACK;
        phOsal_ThreadCreate(&Tst12LpcdThreadObj.ThreadHandle, &Tst12LpcdThreadObj, &Tst12_LpcdThread, NULL);

        phOsal_StartScheduler();
#endif
        DEBUG_PRINTF("RTOS error : Scheduler exited. \n");
    } while(0);

    while(1); //Comes here if initialization failure or scheduler exit due to error

    return 0;
}

/**
* This Task demonstrates the usage of discovery loop.
* It detects and reports the NFC technology type and performs read NDEF in case any NFC forum tags are detected.
* If a P2P device is detected then LLCP activate will be called to perform Client/Server operation from another Task.
* \param   pDataParams      The discovery loop data parameters
* \note    This Task will never return and Discovery loop run will be called in a infinite while loop.
*/
void Tst12_LpcdThread(void * pParam)
{
    phStatus_t status;
    uint8_t bValueI;
    uint8_t bValueQ;
    uint8_t bChargePump;
    uint8_t bDigitalFilter;
    uint16_t wDetectionOption;
    uint8_t bTCOption;
    uint8_t bTCScenario;
    uint8_t bLoopOptions;
    uint8_t bLoopScenarios;
    uint16_t wPowerDownTimeMs;
    uint16_t wDetectionTimeUs;
    uint32_t dwInput;

    while(1)
    {

        /* Default Values */
        status          = PH_ERR_SUCCESS;
        bValueI         = 0;
        bValueQ         = 0;
        bChargePump     = 0;
        bDigitalFilter  = 0;
        bTCOption       = 0;
        bTCScenario     = 0;
        bLoopOptions    = 0;
        bLoopScenarios  = 0;
        wDetectionOption= 0;
        wPowerDownTimeMs= 100;
        wDetectionTimeUs= 56;
        dwInput         = 0;
        do
        {
            DEBUG_PRINTF("Select the below Test Cases Options :-\n");
            DEBUG_PRINTF("0 :- Execute both the Test Case Options\n");
            DEBUG_PRINTF("1 :- Functional Test Cases\n");
            DEBUG_PRINTF("2 :- Timing Test Cases\n");
            DEBUG_PRINTF("Enter here : ");
            DEBUG_SCANF("%d", &dwInput);
            bTCOption = (uint8_t)dwInput;

            if (bTCOption == EX11_LPCD_TC_OPTION_DEFAULT)
            {
                bLoopOptions = 1;
                bTCOption = EX11_LPCD_TC_FUNCTIONAL;
            }

            do {
                if (bTCOption == EX11_LPCD_TC_FUNCTIONAL)
                {
                    DEBUG_PRINTF("Proceeding with Functional Test Cases.\n");

                    DEBUG_PRINTF("Select the below Test Cases Scenarios :-\n");
                    DEBUG_PRINTF("0 :- Execute all the scenarios\n");
                    DEBUG_PRINTF("1 :- Digital Filter = Disabled; Charge Pump = Disabled; Detection Option = Don't Care\n");
                    DEBUG_PRINTF("2 :- Digital Filter = Disabled; Charge Pump = Enabled; Detection Option = Don't Care\n");
                    DEBUG_PRINTF("3 :- Digital Filter = Enabled; Charge Pump = Disabled; Detection Option = Option 1\n");
                    DEBUG_PRINTF("4 :- Digital Filter = Enabled; Charge Pump = Disabled; Detection Option = Option 2\n");
                    DEBUG_PRINTF("5 :- Digital Filter = Enabled; Charge Pump = Enabled; Detection Option = Option 1\n");
                    DEBUG_PRINTF("6 :- Digital Filter = Enabled; Charge Pump = Enabled; Detection Option = Option 2\n");
                    DEBUG_PRINTF("7 :- Calibration is performed with load on the antenna. During Lpcd Loop unload the antenna.\n");
                    DEBUG_PRINTF("Enter here : ");
                    DEBUG_SCANF("%d", &dwInput);
                    bTCScenario = (uint8_t)dwInput;
                    if (bTCScenario > EX11_LPCD_TC_SCENARIO_7)
                    {
                        DEBUG_PRINTF("INVALID SELECTION\n");
                        break;
                    }
                }
                else if (bTCOption == EX11_LPCD_TC_TIMING)
                {
                    DEBUG_PRINTF("Proceeding with Rf/Power Test Cases.\n");

                    DEBUG_PRINTF("Select the below Test Cases Scenarios :-\n");
                    DEBUG_PRINTF("0 :- Execute all the scenarios\n");
                    DEBUG_PRINTF("2 :- Digital Filter = Disabled; Charge Pump = Enabled; Detection Option = Don't Care\n");
                    DEBUG_PRINTF("3 :- Digital Filter = Enabled; Charge Pump = Disabled; Detection Option = Disabled\n");
                    DEBUG_PRINTF("4 :- Digital Filter = Enabled; Charge Pump = Disabled; Detection Option = Enabled\n");
                    DEBUG_PRINTF("Enter here : ");
                    DEBUG_SCANF("%d", &dwInput);
                    bTCScenario = (uint8_t)dwInput;
                    if ((bTCScenario != EX11_LPCD_TC_SCENARIO_2) && (bTCScenario != EX11_LPCD_TC_SCENARIO_3)
                        && (bTCScenario != EX11_LPCD_TC_SCENARIO_4) && (bTCScenario != EX11_LPCD_TC_SCENARIO_DEFAULT))
                    {
                        DEBUG_PRINTF("INVALID SELECTION\n");
                        break;
                    }

                    if (bTCScenario != EX11_LPCD_TC_SCENARIO_DEFAULT)
                    {
                        DEBUG_PRINTF("Enter Detection Time/Rf-On Time in microseconds(us):- ");
                        DEBUG_SCANF("%d", &dwInput);
                        wDetectionTimeUs = (uint16_t)dwInput;
                        DEBUG_PRINTF("Enter Power Down/Standby Time milliseconds(ms):- ");
                        DEBUG_SCANF("%d", &dwInput);
                        wPowerDownTimeMs = (uint16_t)dwInput;
                    }
                }
                else
                {
                    DEBUG_PRINTF("INVALID SELECTION\n");
                    break;
                }

                if (bTCScenario == EX11_LPCD_TC_SCENARIO_DEFAULT)
                {
                    bLoopScenarios = 1;
                    bTCScenario = EX11_LPCD_TC_SCENARIO_1;
                }

                do {
                    if (bTCOption == EX11_LPCD_TC_TIMING)
                    {
                        switch(bLoopScenarios)
                        {
                        case 1: bTCScenario = EX11_LPCD_TC_SCENARIO_2;
                        bLoopScenarios++;
                        break;

                        case 2: bTCScenario = EX11_LPCD_TC_SCENARIO_3;
                        bLoopScenarios++;
                        break;

                        case 3: bTCScenario = EX11_LPCD_TC_SCENARIO_4;
                        bLoopScenarios++;
                        break;

                        default : bLoopScenarios = PH_OFF;
                        break;
                        }
                    }
                    if ((bTCOption == EX11_LPCD_TC_TIMING) && (bTCScenario == EX11_LPCD_TC_SCENARIO_5))
                    {
                        break;
                    }

                    switch (bTCScenario)
                    {
                    case EX11_LPCD_TC_SCENARIO_1:
                        bDigitalFilter  = PH_CONFIG_DISABLE;
                        bChargePump     = PH_CONFIG_DISABLE;
                        wDetectionOption= PH_CONFIG_DONT_CARE;
                        DEBUG_PRINTF("Executing Test Cases Scenario 1 : Digital Filter : Disabled; Charge Pump : Disabled; Detection Option : NA.\n");
                        break;
                    case EX11_LPCD_TC_SCENARIO_2:
                        bDigitalFilter  = PH_CONFIG_DISABLE;
                        bChargePump     = PH_CONFIG_ENABLE;
                        wDetectionOption= PH_CONFIG_DONT_CARE;
                        DEBUG_PRINTF("Executing Test Cases Scenario 2 : Digital Filter : Disabled; Charge Pump : Enabled; Detection Option : NA.\n");
                        break;
                    case EX11_LPCD_TC_SCENARIO_3:
                        bDigitalFilter  = PH_CONFIG_ENABLE;
                        bChargePump     = PH_CONFIG_DISABLE;
                        wDetectionOption= PHHAL_HW_RC663_FILTER_OPTION1;
                        DEBUG_PRINTF("Executing Test Cases Scenario 3 : Digital Filter : Enabled; Charge Pump : Disabled; Detection Option : Option 1.\n");
                        break;
                    case EX11_LPCD_TC_SCENARIO_4:
                        bDigitalFilter  = PH_CONFIG_ENABLE;
                        bChargePump     = PH_CONFIG_DISABLE;
                        wDetectionOption= PHHAL_HW_RC663_FILTER_OPTION2;
                        DEBUG_PRINTF("Executing Test Cases Scenario 4 : Digital Filter : Enabled; Charge Pump : Disabled; Detection Option : Option 2.\n");
                        break;
                    case EX11_LPCD_TC_SCENARIO_5:
                        bDigitalFilter  = PH_CONFIG_ENABLE;
                        bChargePump     = PH_CONFIG_ENABLE;
                        wDetectionOption= PHHAL_HW_RC663_FILTER_OPTION1;
                        DEBUG_PRINTF("Executing Test Cases Scenario 5 : Digital Filter : Enabled; Charge Pump : Enabled; Detection Option : Option 1.\n");
                        break;
                    case EX11_LPCD_TC_SCENARIO_6:
                        bDigitalFilter  = PH_CONFIG_ENABLE;
                        bChargePump     = PH_CONFIG_ENABLE;
                        wDetectionOption= PHHAL_HW_RC663_FILTER_OPTION2;
                        DEBUG_PRINTF("Executing Test Cases Scenario 6 : Digital Filter : Enabled; Charge Pump : Enabled; Detection Option : Option 2.\n");
                        break;
                    case EX11_LPCD_TC_SCENARIO_7:
                        bDigitalFilter  = PH_CONFIG_DISABLE;
                        bChargePump     = PH_CONFIG_DISABLE;
                        wDetectionOption= PH_CONFIG_DONT_CARE;
                        DEBUG_PRINTF("Executing Test Cases Scenario 7 : Calibration is performed with load on the antenna. During Lpcd Loop unload the antenna.\n");
                        break;
                    }

                    if ((bTCOption == EX11_LPCD_TC_FUNCTIONAL) && (bTCScenario == EX11_LPCD_TC_SCENARIO_7))
                    {
                        DEBUG_PRINTF("Starting Calibration Process. Load the antenna\nPress any key once done : ");
                    }
                    else
                    {
                        DEBUG_PRINTF("Starting Calibration Process. Unload the antenna\nPress any key once done : ");
                    }

                    DEBUG_SCANF("%d", &dwInput);
                    /* Set the Charge Pump */
                    status = phhalHw_SetConfig(&sHalParams, PHHAL_HW_RC663_CONFIG_LPCD_CHARGEPUMP, bChargePump);
                    CHECK_STATUS(status);

                    /* Set the Digital Filter */
                    status = phhalHw_SetConfig(&sHalParams, PHHAL_HW_RC663_CONFIG_LPCD_FILTER,
                        ((uint16_t)(bDigitalFilter) | wDetectionOption));
                    CHECK_STATUS(status);

                    /* Setting the timers. */
                    status = phhalHw_Rc663_Cmd_Lpcd_SetConfig(&sHalParams, PHHAL_HW_RC663_CMD_LPCD_MODE_DEFAULT,
                        bValueI, bValueQ, wPowerDownTimeMs, wDetectionTimeUs);
                    CHECK_STATUS(status);

                    /* Configure LPCD */
                    status = phhalHw_Rc663_Cmd_Lpcd_GetConfig(&sHalParams, &bValueI, &bValueQ);
                    CHECK_STATUS(status);

                    status = phhalHw_Rc663_Cmd_Lpcd_SetConfig(&sHalParams, PHHAL_HW_RC663_CMD_LPCD_MODE_POWERDOWN,
                        bValueI, bValueQ,wPowerDownTimeMs, wDetectionTimeUs);
                    CHECK_STATUS(status);

                    if ((bTCOption == EX11_LPCD_TC_FUNCTIONAL) && (bTCScenario == EX11_LPCD_TC_SCENARIO_7))
                    {
                        DEBUG_PRINTF("Calibration Process Completed\nI = 0x%x\nQ = 0x%x\nExecuting Lpcd Process, Unload the antenna\n", bValueI, bValueQ);
                    }
                    else
                    {
                        DEBUG_PRINTF("Calibration Process Completed\nI = 0x%x\nQ = 0x%x\nExecuting Lpcd Process, Load the antenna\n", bValueI, bValueQ);
                    }
                    /* Start LPCD. */
                    status = phhalHw_Lpcd(&sHalParams);
                    if(status == PH_ERR_SUCCESS)
                    {
                        DEBUG_PRINTF("Lpcd Success\n");
                    }
                    else
                    {
                        DEBUG_PRINTF("Lpcd Failed\n");
                    }

                }while((bLoopScenarios) && ((bTCScenario++) < EX11_LPCD_TC_SCENARIO_7));

            }while((bLoopOptions) && ((bTCOption++) < EX11_LPCD_TC_TIMING));
        }while(1);
    }
}


static phStatus_t phApp_Clif_Init( uint8_t *pTxBuffer, uint16_t wTxBufSize, uint8_t *pRxBuffer, uint16_t wRxBufSize)
{
    phStatus_t  wStatus;

#ifdef PH_PLATFORM_HAS_ICFRONTEND
    phDriver_Pin_Config_t pinCfg;
#endif


#ifdef NXPBUILD__PHHAL_HW_RC663
/* Initialize the RC663 HAL component */
    PH_CHECK_SUCCESS_FCT(wStatus, phhalHw_Rc663_Init(
            &sHalParams,
            sizeof(phhalHw_Rc663_DataParams_t),
            &sBalParams,
            (uint8_t *)gkphhalHw_Rc663_LoadConfig,
            pTxBuffer,
            wTxBufSize,
            pRxBuffer,
            wRxBufSize
    ));
#endif /* NXPBUILD__PHHAL_HW_RC663 */


#ifdef PH_PLATFORM_HAS_ICFRONTEND
    pinCfg.bOutputLogic = PH_DRIVER_SET_LOW;
    pinCfg.bPullSelect = PHDRIVER_PIN_IRQ_PULL_CFG;

    pinCfg.eInterruptConfig = PIN_IRQ_TRIGGER_TYPE;
    phDriver_PinConfig(PHDRIVER_PIN_IRQ, PH_DRIVER_PINFUNC_INTERRUPT, &pinCfg);

#ifdef PHDRIVER_LPC1769
    NVIC_SetPriority(EINT_IRQn, EINT_PRIORITY);
    /* Enable interrupt in the NVIC */
    NVIC_ClearPendingIRQ(EINT_IRQn);
    NVIC_EnableIRQ(EINT_IRQn);
#endif /* PHDRIVER_LPC1769 */

#endif /* #ifdef PH_PLATFORM_HAS_ICFRONTEND */

    wStatus = phhalHw_Rc663_ReadRegister(&sHalParams, PHHAL_HW_RC663_REG_VERSION, &bDataBuffer);
    CHECK_STATUS(wStatus);
    DEBUG_PRINTF("\nReader chip RC663: 0x%02x\n", bDataBuffer);

    return PH_ERR_SUCCESS;
}

#ifdef PH_PLATFORM_HAS_ICFRONTEND
void CLIF_IRQHandler(void)
{
    /* Read the interrupt status of external interrupt attached to the reader IC IRQ pin */
    if (phDriver_PinRead(PHDRIVER_PIN_IRQ, PH_DRIVER_PINFUNC_INTERRUPT))
    {
        /* Call application registered callback. */
        if (sHalParams.pRFISRCallback != NULL)
        {
            sHalParams.pRFISRCallback(&sHalParams);
        }
    }
    phDriver_PinClearIntStatus(PHDRIVER_PIN_IRQ);
}
#endif /* PH_PLATFORM_HAS_ICFRONTEND */
