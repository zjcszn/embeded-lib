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
* Generic phDriver Component of Reader Library Framework.
* $Author$
* $Revision$
* $Date$
*
* History:
*  RS: Generated 24. Jan 2017
*
*/

/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */
#include "phDriver.h"
#include "BoardSelection.h"

#ifdef PHDRIVER_LINUX_KERNEL_SPI

#include <phDriver.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>


/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */
#define PHBAL_REG_LINUX_ERROR                       0xFFFFFFFF

/* *****************************************************************************************************************
 * Type Definitions
 * ***************************************************************************************************************** */

/* *****************************************************************************************************************
 * Global and Static Variables
 * Total Size: NNNbytes
 * ***************************************************************************************************************** */

/* File descriptors */
static uint32_t dwSpiFd;


/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
static phStatus_t phbalReg_Linux_KernelSpi_OpenPort(uint32_t *pdwSpiFd);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
phStatus_t phbalReg_Init(void * pDataParams, uint16_t wSizeOfDataParams)
{
    phStatus_t status;

    if ((pDataParams == NULL) || (sizeof(phbalReg_Type_t) != wSizeOfDataParams))
    {
        return (PH_DRIVER_ERROR | PH_COMP_DRIVER);
    }

    ((phbalReg_Type_t *)pDataParams)->wId        = PH_COMP_DRIVER | PHDRIVER_KERNEL_SPI_ID;
    ((phbalReg_Type_t *)pDataParams)->bBalType   = PHBAL_REG_TYPE_KERNEL_SPI;

    status = phbalReg_Linux_KernelSpi_OpenPort(&dwSpiFd);

    return status;
}


phStatus_t phbalReg_Exchange(void * pDataParams, uint16_t wOption, uint8_t * pTxBuffer, uint16_t wTxLength,
        uint16_t wRxBufSize, uint8_t * pRxBuffer, uint16_t * pRxLength)
{
    uint16_t sNumBytesWritten;

    if ((pTxBuffer == NULL) && (pRxBuffer == NULL))
    {
        return (PH_DRIVER_ERROR | PH_COMP_DRIVER);
    }

    sNumBytesWritten = write(dwSpiFd, pTxBuffer, wTxLength);

    if (sNumBytesWritten != wTxLength)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    if ((pRxBuffer != NULL) && (wRxBufSize))
    {
        sNumBytesWritten = read(dwSpiFd, pRxBuffer, wRxBufSize);
    }

    if (pRxLength != NULL)
    {
        *pRxLength = sNumBytesWritten;
    }

    return PH_DRIVER_SUCCESS;
}

phStatus_t phbalReg_SetConfig(void * pDataParams, uint16_t wConfig, uint32_t dwValue)
{
	phStatus_t wStatus = PH_DRIVER_SUCCESS;

	switch(wConfig)
	{
	case PHBAL_KERNEL_SPI_IOCTL_MODE:
		if(ioctl(dwSpiFd, PHBAL_KERNEL_SPI_IOCTL_MODE, dwValue) != PH_DRIVER_SUCCESS)
		{
			wStatus = (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
		}
		break;

	default:
		wStatus = (PH_DRIVER_ERROR | PH_COMP_DRIVER);
	}

    return wStatus;
}

phStatus_t phbalReg_GetConfig(void * pDataParams, uint16_t wConfig, uint32_t * pValue)
{
    return (PH_DRIVER_ERROR | PH_COMP_DRIVER);
}


/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */


static phStatus_t phbalReg_Linux_KernelSpi_OpenPort(uint32_t *pdwSpiFd)
{

    *pdwSpiFd = open(PHDRIVER_KERNEL_SPI_CFG_DIR, O_RDWR);
    if((*pdwSpiFd) == PHBAL_REG_LINUX_ERROR)
    {
        return (PH_DRIVER_FAILURE | PH_COMP_DRIVER);
    }

    return PH_DRIVER_SUCCESS;
}

#endif /* PHDRIVER_LINUX_KERNEL_SPI */
