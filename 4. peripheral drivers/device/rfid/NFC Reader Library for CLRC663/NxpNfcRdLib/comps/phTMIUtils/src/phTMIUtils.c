/*----------------------------------------------------------------------------*/
/* Copyright 2009, 2014, 2020-2021, 2023 NXP                                  */
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
* Log Component of Reader Library Framework.
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*
*/

#include <phTMIUtils.h>

#ifdef NXPBUILD__PH_TMIUTILS

#include <ph_Status.h>
#include <ph_RefDefs.h>

phStatus_t phTMIUtils_Init(
                           phTMIUtils_t * pDataParams,
                           uint8_t * pTMIBuffer,
                           uint32_t dwBufLen
                           )
{
    PH_ASSERT_NULL (pDataParams);
    /* Check parameters */
    if ((pTMIBuffer == NULL) || (dwBufLen == 0U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_TMIUTILS);
    }
    pDataParams->pTMIBuffer = pTMIBuffer;
    pDataParams->dwTMIBufLen = dwBufLen;
    pDataParams->dwTMIbufIndex = 0;
    pDataParams->bTMIStatus = PH_OFF;
    pDataParams->dwOffsetInTMI = 0;

    return PH_ERR_SUCCESS;
}

phStatus_t phTMIUtils_ActivateTMICollection(
    phTMIUtils_t *pDataParams,
    uint8_t bOption
    )
{
    if ((pDataParams == NULL) || (pDataParams->dwTMIBufLen == 0U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_TMIUTILS);
    }

    switch (bOption)
    {
    case PH_TMIUTILS_DEACTIVATE_TMI:

        /* Deactivate TMI Collection */
        pDataParams->dwTMIBufLen = 0;
        pDataParams->dwOffsetInTMI = 0;
        pDataParams->bTMIStatus = PH_OFF;
        break;

    case PH_TMIUTILS_ACTIVATE_TMI:
    case PH_TMIUTILS_RESUME_TMI:

        /* Activate/Resume TMI collection */
        pDataParams->bTMIStatus = PH_ON;
        break;

    case PH_TMIUTILS_PAUSE_TMI:

        /* Pause TMI collection */
        pDataParams->bTMIStatus = PH_OFF;
        break;

    case PH_TMIUTILS_RESET_TMI:

        /* Reset TMI collection buffer index to 0 */
        pDataParams->dwTMIbufIndex = 0;
        pDataParams->dwOffsetInTMI = 0;
        break;

    default:
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_TMIUTILS);

    }
    return PH_ERR_SUCCESS;
}

phStatus_t phTMIUtils_GetTMI(
                             phTMIUtils_t * pDataParams,
                             uint8_t ** ppTMIBuffer,
                             uint32_t * dwTMILen
                             )
{
    PH_ASSERT_NULL (pDataParams);

    *ppTMIBuffer = pDataParams->pTMIBuffer;
    *dwTMILen = pDataParams->dwTMIbufIndex;

    return PH_ERR_SUCCESS;
}

phStatus_t phTMIUtils_CollectTMI(
                                 phTMIUtils_t * pDataParams,
                                 uint8_t bOption,
                                 uint8_t * pCmdBuff,
                                 uint16_t wCmdLen,
                                 uint8_t * pData,
                                 uint32_t dwDataLen,
                                 uint16_t wBlockSize
                                 )
{
    uint32_t    PH_MEMLOC_REM dwTmp = 0;

    PH_ASSERT_NULL (pDataParams);

    /* Parameter check */
    if (((pCmdBuff == NULL) && (0U != (wCmdLen))) || ((pData == NULL) && (0U != (dwDataLen))))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_TMIUTILS);
    }
    if ((pDataParams->dwTMIbufIndex + wCmdLen + (wBlockSize - (wCmdLen % wBlockSize)) + dwDataLen + (wBlockSize - (dwDataLen % wBlockSize))) > pDataParams->dwTMIBufLen)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_TMIUTILS);
    }

    if (0U != (wCmdLen))
    {
        /* Copy the command buffer */
        (void)memcpy(&pDataParams->pTMIBuffer[pDataParams->dwTMIbufIndex], pCmdBuff, wCmdLen);
        pDataParams->dwTMIbufIndex += wCmdLen;

        if ((0U != ((bOption & PH_TMIUTILS_ZEROPAD_CMDBUFF))) && ((pDataParams->dwTMIbufIndex % wBlockSize) != 0U))
        {
            /* Zero padding */
            (void)memset(&pDataParams->pTMIBuffer[pDataParams->dwTMIbufIndex], 0x00, (wBlockSize - (pDataParams->dwTMIbufIndex % wBlockSize)));
            pDataParams->dwTMIbufIndex += (wBlockSize - (pDataParams->dwTMIbufIndex % wBlockSize));
        }
    }
    if (0U != (bOption & PH_TMIUTILS_READ_INS))
    {
        if (0U != (pDataParams->dwOffsetInTMI))
        {
            /* Update the Length field Offset in pDataParams */
            dwTmp = pDataParams->pTMIBuffer[pDataParams->dwOffsetInTMI + 1U];
            dwTmp <<= 8U;
            dwTmp |= pDataParams->pTMIBuffer[pDataParams->dwOffsetInTMI];
            dwTmp += dwDataLen;
            pDataParams->pTMIBuffer[pDataParams->dwOffsetInTMI] = (uint8_t)dwTmp;
            pDataParams->pTMIBuffer[pDataParams->dwOffsetInTMI + 1U] = (uint8_t)(dwTmp >> 8U);
        }
        else
        {
            /* Store the Length field Offset in pDataParams */
            pDataParams->dwOffsetInTMI = (pDataParams->dwTMIbufIndex - 11u);
        }
    }

    if (0U != (dwDataLen))
    {
        /* Copy the command buffer */
        (void)memcpy(&pDataParams->pTMIBuffer[pDataParams->dwTMIbufIndex], pData, dwDataLen);
        pDataParams->dwTMIbufIndex += dwDataLen;

        if ((0U != ((bOption & PH_TMIUTILS_ZEROPAD_DATABUFF))) && ((pDataParams->dwTMIbufIndex % wBlockSize) != 0U))
        {
            /* Zero padding */
            (void)memset(&pDataParams->pTMIBuffer[pDataParams->dwTMIbufIndex], 0x00, (wBlockSize - (pDataParams->dwTMIbufIndex % wBlockSize)));
            pDataParams->dwTMIbufIndex += (wBlockSize - (pDataParams->dwTMIbufIndex % wBlockSize));
        }
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phTMIUtils_SetConfig(
                                phTMIUtils_t * pDataParams,
                                uint16_t wConfig,
                                uint32_t dwValue
                                )
{
    switch (wConfig)
    {
    case PH_TMIUTILS_TMI_OFFSET_LENGTH:
        pDataParams->dwOffsetInTMI  = dwValue;
        break;
    case PH_TMIUTILS_TMI_OFFSET_VALUE:
        pDataParams->pTMIBuffer[pDataParams->dwOffsetInTMI] = (uint8_t)dwValue;
        pDataParams->pTMIBuffer[pDataParams->dwOffsetInTMI + 1U] = (uint8_t)(dwValue >> 8U);
        break;
    default:
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_TMIUTILS);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phTMIUtils_GetConfig(
                                phTMIUtils_t * pDataParams,
                                uint16_t wConfig,
                                uint32_t *pValue
                                )
{
    switch (wConfig)
    {
    case PH_TMIUTILS_TMI_STATUS:
        *pValue = pDataParams->bTMIStatus;
        break;
    case PH_TMIUTILS_TMI_OFFSET_LENGTH:
        *pValue = pDataParams->dwOffsetInTMI;
        break;
    case PH_TMIUTILS_TMI_BUFFER_INDEX:
        *pValue = pDataParams->dwTMIbufIndex;
        break;
    default:
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_TMIUTILS);
    }

    return PH_ERR_SUCCESS;
}

#endif /* NXPBUILD__PH_TMIUTILS */
