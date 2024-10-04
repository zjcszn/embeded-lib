/*----------------------------------------------------------------------------*/
/* Copyright 2020,2022-2023 NXP                                               */
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
* Example Source header for NfcrdlibEx6_LPCD.
* This application will configure Reader Library to provide demo on LPCD functionality of PN5190 Reader IC's.
* Please refer Readme.txt file  for  Hardware Pin Configuration, Software Configuration and steps to build and
* execute the project which is present in the same project directory.
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <NfcrdlibEx6_LPCD.h>

#ifdef NXPBUILD__PHHAL_HW_PN5190
#include <phApp_Init.h>
#include "BoardSelection.h"

/* HAL specific headers */
#include <phhalHw_Pn5190_Instr.h>
#include <phhalHw_Pn5190_Reg.h>

#define NXPBUILD__PHHAL_HW_PN5190_ENABLE_SW_LPCD
#ifndef NXPBUILD__PHHAL_HW_PN5190_ENABLE_SW_LPCD
    #define NXPBUILD__PHHAL_HW_PN5190_ENABLE_ULPCD
#endif /* NXPBUILD__PHHAL_HW_PN5190_ENABLE_SW_LPCD */

/* Release Build Configuration to work with different Calibration options in Example */
#define NXPBUILD__PHHAL_HW_PN5190_LPCD_CALIBRATE_ONCE

/*******************************************************************************
**   Global Variable Declaration
*******************************************************************************/
phbalReg_Type_t                 sBalParams;
phhalHw_Pn5190_DataParams_t   * pHal;

#ifdef DEBUG_PRINT_RAW_DATA
#define MAX_RAW_DATA_LENGTH    (1100)
uint8_t g_Raw_Data[MAX_RAW_DATA_LENGTH];
uint32_t g_Raw_Data_Index = 0;
#endif /* DEBUG_PRINT_RAW_DATA */

#define PN5190_DIRECTION_BYTE_LEN      0x01U
#define PN5190_TYPE_FIELD_LEN          0x01U
#define PN5190_LENGTH_FIELD_LEN        0x02U
#define PN5190_STATUS_FIELD_LEN        0x01U

/*******************************************************************************
**   Global Defines
*******************************************************************************/
#define PHHAL_HW_PN5190_PWR_CONFIG_ADDR                    0x00U
#define PHHAL_HW_PN5190_TX_LDO_VDDPA_HIGH_ADDR             0x06U
#define PHHAL_HW_PN5190_TX_LDO_VDDPA_LOW_ADDR              0x07U
#define PHHAL_HW_PN5190_DPC_CONFIG_ADDR                    0x76U
#define PHHAL_HW_PN5190_GPIO3_ABORT_POLARITY_ADDR          0x4CCU

/* Configure the required VDDPA Voltage Value that should be used to work with ULPCD. */
#define PHHAL_HW_PN5190_TX_LDO_VDDPA_V2_7_VALUE            0x0CU

/*******************************************************************************
**   Static Defines
*******************************************************************************/
static phStatus_t SW_LPCD_Calib(uint32_t * pdwRefValue);
static phStatus_t SW_LPCD_Demo(uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime);
static phStatus_t ULPCD_Get_HFATT_Value();
static phStatus_t ULPCD_Configure_GPIO3();
static phStatus_t ULPCD_Calib(uint32_t * pdwRefValue);
static phStatus_t ULPCD_Demo(uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime);

/*******************************************************************************
**   Function Declarations
*******************************************************************************/
void CLIF_IRQHandler(void);

/*******************************************************************************
**   Function Definitions
*******************************************************************************/
static phStatus_t SW_LPCD_Calib(uint32_t * pdwRefValue)
{
    phStatus_t  wStatus;

    /* Configure the Wake-up time to Turn ON the RF Field to measure reference values for SW LPCD Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, 330U);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode to Default to perform SW LPCD Calibration Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN5190_LPCD_MODE_DEFAULT);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode for SW LPCD Calibration */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_CONFIG, PHHAL_HW_PN5190_LPCD_CTRL_LPCD_CALIB);
    CHECK_SUCCESS(wStatus);

    /* Perform LPCD Calibration Cycle */
    wStatus = phhalHw_Lpcd(pHal);
    CHECK_SUCCESS(wStatus);

    /* Get the Calibration value obtained during LPCD Calibration Cycle */
    return phhalHw_Pn5190_Instr_LPCD_GetConfig(pHal, PHHAL_HW_CONFIG_LPCD_REF_VAL, pdwRefValue);
}

static phStatus_t SW_LPCD_Demo(uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime)
{
    phStatus_t  wStatus;

    /* Configure the Wake-up time between successive RF Field ON to measure and compare against the reference value
     * during SW LPCD Cycle. This can be fine tuned by User to obtain required Power efficiency. */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, wLPCDWakeUpTime);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode to Power-down to perform SW LPCD Detection Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN5190_LPCD_MODE_POWERDOWN);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode for SW LPCD Detection Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_CONFIG, PHHAL_HW_PN5190_LPCD_CTRL_LPCD);
    CHECK_SUCCESS(wStatus);

    /* Set the Calibration value obtained during last LPCD Calibration Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_REF_VAL, dwLPCDRefValue);
    CHECK_SUCCESS(wStatus);

    /* Perform LPCD Detection Cycle */
    return phhalHw_Lpcd(pHal);
}

static phStatus_t ULPCD_Get_HFATT_Value()
{
    phStatus_t wStatus = PH_ERR_INTERNAL_ERROR;
    uint8_t bReadEEPROM_Val;
    uint8_t bHFATT_Val;
    uint32_t dwReadReg_Val;
    uint8_t bVddpa_Val = PHHAL_HW_PN5190_TX_LDO_VDDPA_V2_7_VALUE;
    uint8_t bDPC_EEPROM_Val;

    /* Read EEPROM Power Configuration value. */
    wStatus = phhalHw_Pn5190_Instr_ReadE2Prom(pHal, PHHAL_HW_PN5190_PWR_CONFIG_ADDR, &bReadEEPROM_Val, 0x01U);
    CHECK_SUCCESS(wStatus);

    if (bReadEEPROM_Val != 0x21U)
    {
        /* Update the EEPROM Power Configuration value to VUP connected to VBat. */
        bReadEEPROM_Val = 0x21U;
        wStatus = phhalHw_Pn5190_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5190_PWR_CONFIG_ADDR, &bReadEEPROM_Val, 0x01U);
        CHECK_SUCCESS(wStatus);
    }

    wStatus = phhalHw_Pn5190_Instr_ReadE2Prom(pHal, PHHAL_HW_PN5190_TX_LDO_VDDPA_HIGH_ADDR, &bReadEEPROM_Val, 0x01U);
    CHECK_SUCCESS(wStatus);

    /* VDDPA High EEPROM Configuration is updated to work with VDDPA value of 2.7V.
     * This can be different based on user configuration to use PN5190 Reader IC. */
    if (bReadEEPROM_Val != bVddpa_Val)
    {
        wStatus = phhalHw_Pn5190_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5190_TX_LDO_VDDPA_HIGH_ADDR, &bVddpa_Val, 0x01U);
        CHECK_SUCCESS(wStatus);
    }

    wStatus = phhalHw_Pn5190_Instr_ReadE2Prom(pHal, PHHAL_HW_PN5190_TX_LDO_VDDPA_LOW_ADDR, &bReadEEPROM_Val, 0x01U);
    CHECK_SUCCESS(wStatus);

    /* VDDPA High EEPROM Configuration is updated to work with VDDPA value of 2.7V.
     * This can be different based on user configuration to use PN5190 Reader IC. */
    if (bReadEEPROM_Val != bVddpa_Val)
    {
        wStatus = phhalHw_Pn5190_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5190_TX_LDO_VDDPA_LOW_ADDR, &bVddpa_Val, 0x01U);
        CHECK_SUCCESS(wStatus);
    }

    /* Read DPC Configuration value. */
    wStatus = phhalHw_Pn5190_Instr_ReadE2Prom(pHal, PHHAL_HW_PN5190_DPC_CONFIG_ADDR, &bDPC_EEPROM_Val, 0x01U);
    CHECK_SUCCESS(wStatus);

    if (bDPC_EEPROM_Val != 0x00U)
    {
        bReadEEPROM_Val = 0x00U;
        /* Disable DPC to get HFATT value by turning ON the RF Field. */
        wStatus = phhalHw_Pn5190_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5190_DPC_CONFIG_ADDR, &bReadEEPROM_Val, 0x01U);
        CHECK_SUCCESS(wStatus);
    }

    /* Apply Reader Mode Type A settings. */
    wStatus = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);
    CHECK_SUCCESS(wStatus);

    /* Turn ON the RF Field. */
    wStatus = phhalHw_FieldOn(pHal);
    CHECK_SUCCESS(wStatus);

    /* Wait for sometime to stabilize RF ON before reading HF ATT value. */
    wStatus = phhalHw_Wait(pHal, PHHAL_HW_TIME_MICROSECONDS, 1500);
    CHECK_SUCCESS(wStatus);

    /* Read HF ATT value. */
    wStatus = phhalHw_Pn5190_Instr_ReadRegister(pHal, CLIF_RXCTRL_STATUS, &dwReadReg_Val);
    CHECK_SUCCESS(wStatus);

    /* Get the HF ATT Value */
    bHFATT_Val = (uint8_t)((dwReadReg_Val & CLIF_RXCTRL_STATUS_RXCTRL_HF_ATT_VAL_MASK) >> CLIF_RXCTRL_STATUS_RXCTRL_HF_ATT_VAL_POS);

    /* Configure the HF ATT value that should be used with ULPCD Calib and Detection cycle. */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_HFATT_VAL, (uint16_t)bHFATT_Val);
    CHECK_SUCCESS(wStatus);

    if (bDPC_EEPROM_Val != 0x77U)
    {
        bReadEEPROM_Val = 0x77U;
        /* Enable DPC. */
        wStatus = phhalHw_Pn5190_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5190_DPC_CONFIG_ADDR, &bReadEEPROM_Val, 0x01U);
        CHECK_SUCCESS(wStatus);
    }

    /* Turn OFF the RF Field. */
    wStatus = phhalHw_FieldOff(pHal);
    CHECK_SUCCESS(wStatus);

    return wStatus;
}

static phStatus_t ULPCD_Configure_GPIO3()
{
    phStatus_t wStatus = PH_ERR_INTERNAL_ERROR;
    uint8_t bGPIO3PolarityVal;

    /* Read EEPROM GPIO3 abort polarity value. */
    wStatus = phhalHw_Pn5190_Instr_ReadE2Prom(pHal, PHHAL_HW_PN5190_GPIO3_ABORT_POLARITY_ADDR, &bGPIO3PolarityVal, 0x01);
    CHECK_SUCCESS(wStatus);

    if(bGPIO3PolarityVal == 0x00U)
    {
        /* Update EEPROM GPIO3 abort polarity value to high.
         * For K82-PN5190, No external connection is required from NFC_GPIO3 to TP7 of K82.
         * For LPC1769_PN5190, Jumper 7-8 of j12 which connects Triton GPIO3 to LPC GPIO3 should be removed. */
        bGPIO3PolarityVal = 0x01U;
        wStatus = phhalHw_Pn5190_Instr_WriteE2Prom(pHal, PHHAL_HW_PN5190_GPIO3_ABORT_POLARITY_ADDR, &bGPIO3PolarityVal, 0x01);
        CHECK_SUCCESS(wStatus);
    }

    return wStatus;
}

static phStatus_t ULPCD_Calib(uint32_t * pdwRefValue)
{
    phStatus_t  wStatus;

    /* Configure EEPROM GPIO3 abort polarity value */
    wStatus = ULPCD_Configure_GPIO3();
    CHECK_SUCCESS(wStatus);

    /* Get the HF ATT value. */
    wStatus = ULPCD_Get_HFATT_Value();
    CHECK_SUCCESS(wStatus);

    /* Configure the Wake-up time to Turn ON the RF Field to measure reference values for ULPCD Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, 330U);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode to Default to perform ULPCD Calibration Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN5190_LPCD_MODE_DEFAULT);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode for ULPCD Calibration */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_CONFIG, PHHAL_HW_PN5190_LPCD_CTRL_ULPCD_CALIB);
    CHECK_SUCCESS(wStatus);

    /* Perform LPCD Calibration Cycle */
    wStatus = phhalHw_Lpcd(pHal);
    CHECK_SUCCESS(wStatus);

    /* Get the Calibration value obtained during LPCD Calibration Cycle */
    return phhalHw_Pn5190_Instr_LPCD_GetConfig(pHal, PHHAL_HW_CONFIG_LPCD_REF_VAL, pdwRefValue);
}

static phStatus_t ULPCD_Demo(uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime)
{
    phStatus_t  wStatus;

    /* Configure the Wake-up time between successive RF Field ON to measure and compare against the reference value
     * during SW LPCD Cycle. This can be fine tuned by User to obtain required Power efficiency. */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_SET_LPCD_WAKEUPTIME_MS, wLPCDWakeUpTime);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode to Power-down to perform SW LPCD Detection Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_MODE, PHHAL_HW_PN5190_LPCD_MODE_POWERDOWN);
    CHECK_SUCCESS(wStatus);

    /* Configure the LPCD Mode for SW LPCD Detection Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_CONFIG, PHHAL_HW_PN5190_LPCD_CTRL_ULPCD);
    CHECK_SUCCESS(wStatus);

    /* Set the Calibration value obtained during last LPCD Calibration Cycle */
    wStatus = phhalHw_Pn5190_Instr_LPCD_SetConfig(pHal, PHHAL_HW_CONFIG_LPCD_REF_VAL, dwLPCDRefValue);
    CHECK_SUCCESS(wStatus);

    /* Perform LPCD Detection Cycle */
    return phhalHw_Lpcd(pHal);
}

void PrintLPCDExitReason()
{
    uint32_t dwLPCDEventStatus;

    (void)phhalHw_Pn5190_Instr_LPCD_GetConfig(pHal, PHHAL_HW_CONFIG_LPCD_RETRIEVE_EVENT_STATUS, &dwLPCDEventStatus);
    switch(dwLPCDEventStatus)
    {
    case PHHAL_HW_LPCD_EVT_STATUS_GPIO3_ABORT:
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to GPIO3 Toggle.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
        break;

    case PHHAL_HW_LPCD_EVT_STATUS_EXTERNALRF_FIELD:
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to External RF field.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
        break;

    case PHHAL_HW_LPCD_EVT_STATUS_HIF_ACTIVITY:
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to HIF Activity.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
        break;

    case PHHAL_HW_LPCD_EVT_STATUS_VDDPA_LDO_OVERCURRENT:
    case PHHAL_HW_LPCD_EVT_STATUS_XTAL_TIMEOUT:
    case PHHAL_HW_LPCD_EVT_STATUS_CLKDET_ERROR:
    default:
        DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!LPCD Exited due to event = %X.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n", dwLPCDEventStatus);
        break;
    }
}

uint32_t   dwLPCDCalibrateOption = 1;
uint32_t   dwLPCDDemoOption = 1;
uint32_t   dwLPCDRefValue;

phStatus_t Configure_LPCD(uint32_t *pdwLPCDCalibrateOption, uint32_t *pdwLPCDDemoOption)
{
#ifdef DEBUG

    uint32_t   dwScanOption = 1;

    /* Prompt user to either select SW LPCD or ULPCD */
    DEBUG_PRINTF("\nChoose below LPCD Demo Option ");
    DEBUG_PRINTF("\nSW LPCD with calibration always => 11 ");
    DEBUG_PRINTF("\nSW LPCD with calibration once   => 12 ");
    DEBUG_PRINTF("\nULPCD with calibration always   => 21 ");
    DEBUG_PRINTF("\nULPCD with calibration once     => 22 ");
    DEBUG_PRINTF("\nTry Turn ON and OFF of RF Field => 30 ");
    DEBUG_PRINTF("\nEnter LPCD Demo Option : ");
    DEBUG_SCAN("%X",&dwScanOption);

    *pdwLPCDCalibrateOption = dwScanOption & 0xF;
    *pdwLPCDDemoOption = (dwScanOption >> 4);
#else

#ifdef NXPBUILD__PHHAL_HW_PN5190_ENABLE_SW_LPCD
    /* Demo of SW LPCD Feature of PN5190 */
    *pdwLPCDDemoOption = 0x01U;
#endif /* NXPBUILD__PHHAL_HW_PN5190_ENABLE_SW_LPCD */

#ifdef NXPBUILD__PHHAL_HW_PN5190_ENABLE_ULPCD
    /* Demo of ULPCD Feature of PN5190 */
    *pdwLPCDDemoOption = 0x02U;
#endif /* NXPBUILD__PHHAL_HW_PN5190_ENABLE_ULPCD */

#ifdef NXPBUILD__PHHAL_HW_PN5190_LPCD_CALIBRATE_ONCE
    /* Calibration before every LPCD Cycle */
    *pdwLPCDCalibrateOption = 0x02U;
#else
    /* Calibration Once before first LPCD Cycle */
    *pdwLPCDCalibrateOption = 0x01U;
#endif /* NXPBUILD__PHHAL_HW_PN5190_LPCD_CALIBRATE_ONCE */

#endif

    /* TODO : Configuration of LPCD Threshold to vary the LPCD Range */

    return PH_ERR_SUCCESS;
}

/* Perform LPCD Calibration */
phStatus_t Calibrate_LPCD(uint32_t dwLPCDDemo, uint32_t *pdwLPCDRefValue)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    switch(dwLPCDDemo)
    {
    case 1:
        /* Software LPCD feature that can be used with any Power Configuration of PN5190 Reader IC. */
        wStatus = SW_LPCD_Calib(pdwLPCDRefValue);
        if(wStatus == PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nSW LPCD Calibration Successful. ");
        }
        else
        {
            DEBUG_PRINTF("\nSW LPCD Calibration Failed. ");
        }
        break;

    case 2:
        /* Hardware LPCD feature that can be used with only VBat Power Configuration of PN5190 Reader IC. */
        wStatus = ULPCD_Calib(pdwLPCDRefValue);
        if(wStatus == PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\nULPCD Calibration Successful. ");
        }
        else
        {
            DEBUG_PRINTF("\nULPCD Calibration Failed. ");
        }
        break;

    case 3:
    default:
        break;
    }
    return wStatus;
}

/**
*/
phStatus_t DemoLPCD(uint32_t dwLPCDDemo, uint32_t dwLPCDRefValue, uint16_t wLPCDWakeUpTime)
{
    phStatus_t wStatus = PH_ERR_SUCCESS;

    switch(dwLPCDDemo)
    {
    case 1:
        /* SW LPCD Demo */
        wStatus = SW_LPCD_Demo(dwLPCDRefValue, wLPCDWakeUpTime);
        if(wStatus == PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!SW LPCD Exited due to Load Change.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
        }
        else
        {
            PrintLPCDExitReason();
        }
        break;

    case 2:
        /* ULPCD Demo */
        wStatus = ULPCD_Demo(dwLPCDRefValue, wLPCDWakeUpTime);
        if(wStatus == PH_ERR_SUCCESS)
        {
            DEBUG_PRINTF("\n!!!!!!!!!!!!!!!!!!!!!!!!!ULPCD Exited due to Load Change.!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
        }
        else
        {
            PrintLPCDExitReason();
        }
        break;

    case 3:
        wStatus = phhalHw_FieldOff(pHal);
        CHECK_SUCCESS(wStatus);

        wStatus = phhalHw_ApplyProtocolSettings(pHal, PHHAL_HW_CARDTYPE_ISO14443A);
        CHECK_SUCCESS(wStatus);

        wStatus = phhalHw_FieldOn(pHal);
        CHECK_SUCCESS(wStatus);

        wStatus = phhalHw_Wait(pHal, PHHAL_HW_TIME_MILLISECONDS, 500);
        CHECK_SUCCESS(wStatus);

        wStatus = phhalHw_FieldOff(pHal);
        CHECK_SUCCESS(wStatus);

        wStatus = phhalHw_Wait(pHal, PHHAL_HW_TIME_MILLISECONDS, 500);
        CHECK_SUCCESS(wStatus);
        break;

    default:
        DEBUG_PRINTF("\nWrong Input is been provided, please enter correct option once again. ");
        DEBUG_PRINTF("\nRestart to provide correct input. ");
        while(1);
        break;
    }

    DEBUG_PRINTF("\n---------------Restart to try another option.---------------\n\n");

    return wStatus;
}


/*******************************************************************************
**   BAL Exchange
*******************************************************************************/
/* Read the data over SPI */
phStatus_t phApp_Pn5190_SPIRead(uint8_t * pRxBuffer,
        uint16_t wRxLength)
{
    phStatus_t  PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint16_t    PH_MEMLOC_REM wResponseLen = 0U;

    /* Enable chip select connected to reader IC (NSS low). */
    phDriver_PinWrite(PHDRIVER_PIN_SSEL, PH_DRIVER_SET_LOW);

    /* Read back response */
    status =  phbalReg_Exchange(
            &sBalParams,
            PH_EXCHANGE_DEFAULT,
            NULL,
            0U,
            wRxLength,
            pRxBuffer,
            &wResponseLen);

    /* Disable chip select connected to reader IC (NSS high). */
    phDriver_PinWrite(PHDRIVER_PIN_SSEL, PH_DRIVER_SET_HIGH);

    if(wResponseLen != wRxLength)
    {
        status = PH_ERR_INTERNAL_ERROR;
    }

#ifdef DEBUG_PRINT_RAW_DATA
    uint16_t cnt = 0;

    if((g_Raw_Data_Index + wRxLength+4) < MAX_RAW_DATA_LENGTH)
    {
        g_Raw_Data[g_Raw_Data_Index++] = 'C';
        g_Raw_Data[g_Raw_Data_Index++] = 'C';
        for(cnt=0; cnt < wRxLength; cnt++)
        {
            g_Raw_Data[g_Raw_Data_Index++] = pRxBuffer[cnt];
        }
        g_Raw_Data[g_Raw_Data_Index++] = 'D';
        g_Raw_Data[g_Raw_Data_Index++] = 'D';
    }

    uint16_t cnt = 0;
    printf("\nRX: ");
    for(cnt=0; cnt < wRxLength; cnt++)
    {
        printf("%02X ", pRxBuffer[cnt]);
    }
#endif /* DEBUG_PRINT_RAW_DATA */
    return status;
}

volatile uint8_t bSkipEventPost = 0x0U;
/* Application callback to handle Asynchronous events which cannot be processed by RdLib */
void phApp_CallBack(void)
{
    uint32_t dwEventStatusReg = 0x0U;
    uint16_t wEventLen = 0x0U;
    uint8_t  bBackUp = 0U;
    uint8_t  bSPIReadLen = 0U;

    /* Event length is of 2 byte @bIndex  2 and 3 */
    wEventLen = (uint16_t) pHal->sIrqResp.pIsrEvtBuffPtr[2];
    wEventLen <<= 8;
    wEventLen |= ((uint16_t) pHal->sIrqResp.pIsrEvtBuffPtr[3]);

    dwEventStatusReg = (uint32_t) pHal->sIrqResp.pIsrEvtBuffPtr[4];
    dwEventStatusReg |= ((uint32_t) pHal->sIrqResp.pIsrEvtBuffPtr[5]) << 8U ;
    dwEventStatusReg |= ((uint32_t) pHal->sIrqResp.pIsrEvtBuffPtr[6]) << 16U;
    dwEventStatusReg |= ((uint32_t) pHal->sIrqResp.pIsrEvtBuffPtr[7]) << 24U;

    if ((dwEventStatusReg & PH_PN5190_EVT_TX_OVERCURRENT_ERROR) != 0U)
    {
        DEBUG_PRINTF ("\n\tReceived TX Over Current Event... \n");
        (void)phhalHw_AsyncAbort(pHal);
    }
    if ((dwEventStatusReg & PH_PN5190_EVT_CTS) != 0U)
    {
        DEBUG_PRINTF ("\n\tReceived CTS Event... \n");
        (void)phhalHw_SetConfig(pHal, PHHAL_HW_PN5190_CONFIG_CTS_EVENT_STATUS, PH_ON);
    }
    if ((dwEventStatusReg & PH_PN5190_EVT_RFON_DETECT) != 0U)
    {
        (void)phhalHw_SetConfig(pHal, PHHAL_HW_PN5190_CONFIG_RF_ON_EVENT_STATUS, PH_ON);
        bSkipEventPost = 0x1U;

        bSPIReadLen = (pHal->sIrqResp.bIsrBytesRead -
                (PN5190_DIRECTION_BYTE_LEN + PN5190_TYPE_FIELD_LEN + PN5190_LENGTH_FIELD_LEN));

        if (wEventLen > bSPIReadLen)
        {
            bBackUp = pHal->sIrqResp.pIsrEvtBuffPtr[wEventLen - 1];

            phApp_Pn5190_SPIRead(&pHal->sIrqResp.pIsrEvtBuffPtr[wEventLen - 1],
                    (uint16_t) (wEventLen - bSPIReadLen + PN5190_DIRECTION_BYTE_LEN));

            pHal->sIrqResp.pIsrEvtBuffPtr[wEventLen - 1] = bBackUp;
        }
    }
    if ((dwEventStatusReg & (0xFFFFFFFF << 0x0C)) != 0U)
    {
        DEBUG_PRINTF ("\n\tReceived Unexpected Event that should not be reported by PN5190... \n");
    }

}

/*******************************************************************************
**   CLIF ISR Handler
*******************************************************************************/
void CLIF_IRQHandler(void)
{
    /* Read the interrupt status of external interrupt attached to the reader IC IRQ pin */
#ifndef PH_OSAL_LINUX
    if (phDriver_PinRead(PHDRIVER_PIN_IRQ, PH_DRIVER_PINFUNC_INTERRUPT))
#endif /* PH_OSAL_LINUX */
    {
        phDriver_PinClearIntStatus(PHDRIVER_PIN_IRQ);

        /* Call application registered callback. */
        if ((pHal->wId == (PH_COMP_HAL | PHHAL_HW_PN5190_ID)) && (pHal->pRFISRCallback != NULL))
        {
            /* Read the data over SPI */
            phApp_Pn5190_SPIRead(pHal->sIrqResp.pHandlerModeBuffPtr,
                    (uint16_t) pHal->sIrqResp.bIsrBytesRead);

            /* Handle events triggered by PN5190 for previous command execution */
            if( pHal->sIrqResp.pHandlerModeBuffPtr[1] == PH_PN5190_EVT_RSP )
            {
                pHal->sIrqResp.pIsrEvtBuffPtr = pHal->sIrqResp.pHandlerModeBuffPtr;

                /* Call application registered callback to handle PN5190 Asynchronous events  */
                phApp_CallBack();

                if (bSkipEventPost == 0x0U)
                {
                    /* Call application RF callback to handle PN5190 synchronous events  */
                    pHal->pRFISRCallback(pHal);
                }
                bSkipEventPost = 0x0U;
            }
            else
            {
                pHal->sIrqResp.pIsrBuffPtr = pHal->sIrqResp.pHandlerModeBuffPtr;

                pHal->pRFISRCallback(pHal);
            }

            if (pHal->sIrqResp.pHandlerModeBuffPtr == &pHal->sIrqResp.aISRReadBuf[0])
            {
                pHal->sIrqResp.pHandlerModeBuffPtr = &pHal->sIrqResp.aISRReadBuf2[0];
            }
            else
            {
                pHal->sIrqResp.pHandlerModeBuffPtr = &pHal->sIrqResp.aISRReadBuf[0];
            }
        }
    }
}

#endif /* NXPBUILD__PHHAL_HW_PN5190 */
