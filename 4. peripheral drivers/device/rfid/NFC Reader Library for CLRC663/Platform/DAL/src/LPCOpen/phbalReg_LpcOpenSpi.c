/*
*         Copyright (c), NXP Semiconductors Bangalore / India
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Generic phDriver(DAL) Component of Reader Library Framework.
* $Author$
* $Revision$
* $Date$
*
* History:
*  PGh: Fixed case sensitivity for linux build
*  RS:  Generated 24. Jan 2017
*
*/


#include "phDriver.h"
#include <board.h>
#include "BoardSelection.h"

#define PHBAL_REG_LPCOPEN_SPI_ID               0x0DU       /**< ID for LPC Open SPI BAL component */

static void phbalReg_LpcOpenSpiConfig(void);

#ifdef PERF_TEST
static uint32_t dwSpiBaudRate = SSP_CLOCKRATE;
#endif /* PERF_TEST */

/**
* \brief Initialize the LPC Open SPI BAL layer.
*
* \return Status code
* \retval #PH_DRIVER_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS Parameter structure size is invalid.
*/
phStatus_t phbalReg_Init(
                                      void * pDataParams,
                                      uint16_t wSizeOfDataParams
                                      )
{
    SSP_ConfigFormat ssp_format;
    volatile uint32_t delay;

    if((pDataParams == NULL) || (sizeof(phbalReg_Type_t) != wSizeOfDataParams))
    {
        return (PH_DRIVER_ERROR | PH_COMP_DRIVER);
    }

    ((phbalReg_Type_t *)pDataParams)->wId      = PH_COMP_DRIVER | PHBAL_REG_LPCOPEN_SPI_ID;
    ((phbalReg_Type_t *)pDataParams)->bBalType = PHBAL_REG_TYPE_SPI;

    //phPlatform_Port_Host_SetPinConfig(PHPLATFORM_PORT_PIN_SPI);
    phbalReg_LpcOpenSpiConfig();

    Chip_SSP_Init(LPC_SSP);
#ifdef PERF_TEST
    Chip_SSP_SetBitRate(LPC_SSP, dwSpiBaudRate);
#endif /* PERF_TEST */

#ifndef PERF_TEST
    Chip_SSP_SetBitRate(LPC_SSP, SSP_CLOCKRATE);
#endif /* PERF_TEST */

    ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
    ssp_format.bits = SSP_BITS_8;
    ssp_format.clockMode = SSP_CLOCK_MODE0;

    Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
    Chip_SSP_SetMaster(LPC_SSP, 1 /*Master*/);
    Chip_SSP_Enable(LPC_SSP);

    /* Wait Startup time */
    for(delay=0; delay<10000; delay++){}

    return PH_DRIVER_SUCCESS;
}

phStatus_t phbalReg_Exchange(
                                        void * pDataParams,
                                        uint16_t wOption,
                                        uint8_t * pTxBuffer,
                                        uint16_t wTxLength,
                                        uint16_t wRxBufSize,
                                        uint8_t * pRxBuffer,
                                        uint16_t * pRxLength
                                        )
{
    uint16_t              xferLen;
    Chip_SSP_DATA_SETUP_T xferConfig;

    xferConfig.length  = wTxLength;
    xferConfig.rx_data = pRxBuffer;
    xferConfig.tx_data = pTxBuffer;
    xferConfig.rx_cnt  = 0;
    xferConfig.tx_cnt  = 0;

    /* data exchange */
    if ( wRxBufSize == 0 )
    {
        Chip_SSP_WriteFrames_Blocking(LPC_SSP, pTxBuffer, wTxLength);
        xferLen = wTxLength; /* Fake as a full duplex */
    }
    else
    {
        if ( wTxLength == 0 )
        {
            xferLen = Chip_SSP_ReadFrames_Blocking(LPC_SSP, pRxBuffer, wRxBufSize);
        }
        else
        {
            xferLen = Chip_SSP_RWFrames_Blocking(LPC_SSP, &xferConfig);
        }
    }

    if ( wTxLength !=0 && wRxBufSize != 0 )
    {
        if (xferLen != wTxLength)
        {
            return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
        }
    }
    if ( pRxLength != NULL)
        *pRxLength = xferLen;

    return PH_DRIVER_SUCCESS;
}

phStatus_t phbalReg_SetConfig(
                                         void * pDataParams,
                                         uint16_t wConfig,
                                         uint32_t dwValue
                                         )
{
#ifdef PERF_TEST
    switch(wConfig)
    {
    case PHBAL_CONFIG_SPI_BAUD:
        dwSpiBaudRate = dwValue;
        break;
    default:
        return (PH_DRIVER_ERROR | PH_COMP_DRIVER);
    }
#endif /* PERF_TEST */
    return PH_DRIVER_SUCCESS;
}

phStatus_t phbalReg_GetConfig(
                                         void * pDataParams,
                                         uint16_t wConfig,
                                         uint32_t * pValue
                                         )
{
#ifdef PERF_TEST
    switch(wConfig)
    {
    case PHBAL_CONFIG_SPI_BAUD:
        *pValue = dwSpiBaudRate;
        break;
    default:
        return (PH_DRIVER_ERROR | PH_COMP_DRIVER);
    }
#endif /* PERF_TEST */
    return PH_DRIVER_SUCCESS;
}

static void phbalReg_LpcOpenSpiConfig(void)
{
    /* Configure SSP pins (SCK, MOSI and MISO) */
    Chip_IOCON_PinMux(LPC_IOCON, (uint8_t)((((uint32_t)PHDRIVER_PIN_MOSI) & 0xFF00) >> 8),
            (uint8_t)(((uint32_t)PHDRIVER_PIN_MOSI) & 0xFF),
            IOCON_MODE_INACT,
            (uint8_t)((((uint32_t)PHDRIVER_PIN_MOSI) & 0xFF0000) >> 16));
    Chip_IOCON_PinMux(LPC_IOCON, (uint8_t)((((uint32_t)PHDRIVER_PIN_MISO) & 0xFF00) >> 8),
            (uint8_t)(((uint32_t)PHDRIVER_PIN_MISO) & 0xFF),
            IOCON_MODE_INACT,
            (uint8_t)((((uint32_t)PHDRIVER_PIN_MISO) & 0xFF0000) >> 16));
    Chip_IOCON_PinMux(LPC_IOCON, (uint8_t)((((uint32_t)PHDRIVER_PIN_SCK) & 0xFF00) >> 8),
            (uint8_t)(((uint32_t)PHDRIVER_PIN_SCK) & 0xFF),
            IOCON_MODE_INACT,
            (uint8_t)((((uint32_t)PHDRIVER_PIN_SCK) & 0xFF0000) >> 16));
}
