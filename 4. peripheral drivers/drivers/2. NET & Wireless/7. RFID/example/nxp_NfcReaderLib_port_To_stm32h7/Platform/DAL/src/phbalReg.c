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
#include "bsp_spi.h"

/**
* \brief Initialize the STM32H7 Open SPI BAL layer.
*
* \return Status code
* \retval #PH_DRIVER_SUCCESS Operation successful.
* \retval #PH_ERR_INVALID_DATA_PARAMS Parameter structure size is invalid.
*/
phStatus_t phbalReg_Init(void *pDataParams, uint16_t wSizeOfDataParams) {
    phStatus_t phStatus;
    
    if ((pDataParams == NULL) || (sizeof(phbalReg_Type_t) != wSizeOfDataParams)) {
        return (PH_DRIVER_ERROR | PH_COMP_DRIVER);
    }
    
    phStatus = spi_master_init() == SPI_OK ? PH_DRIVER_SUCCESS : PH_DRIVER_ERROR;
    
    ((phbalReg_Type_t *)pDataParams)->wId = PH_COMP_DRIVER;
    ((phbalReg_Type_t *)pDataParams)->bBalType = PHBAL_REG_TYPE_SPI;
    
    return phStatus;
}

phStatus_t phbalReg_Exchange(void *pDataParams, uint16_t wOption, uint8_t *pTxBuffer, uint16_t wTxLength,
                             uint16_t wRxBufSize, uint8_t *pRxBuffer, uint16_t *pRxLength) {
    
    (void)pDataParams;
    (void)wOption;
    uint16_t xferLen = 0;
    
    /* data exchange */
    if (wRxBufSize == 0) {
        if (spi_master_write(pTxBuffer, wTxLength) == SPI_OK) {
            xferLen = wTxLength;
        }
    } else {
        if (wTxLength == 0) {
            if (spi_master_read(pRxBuffer, wRxBufSize) == SPI_OK) {
                xferLen = wRxBufSize;
            }
        } else {
            if (spi_master_write_read(pTxBuffer, pRxBuffer, wTxLength) == SPI_OK) {
                xferLen = wTxLength;
            }
        }
    }
    
    if (wTxLength != 0 && wRxBufSize != 0) {
        if (xferLen != wTxLength) {
            return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
        }
    }
    if (pRxLength != NULL) {
        *pRxLength = xferLen;
    }
    
    return PH_DRIVER_SUCCESS;
}
