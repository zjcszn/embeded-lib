/*----------------------------------------------------------------------------*/
/* Copyright 2014-2021 NXP                                                    */
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
* Generic ISO 14443p4mC mode Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phpalI14443p4mC.h>

#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
#include "Sw/phpalI14443p4mC_Sw.h"
#endif/* NXPBUILD__PHPAL_I14443P4MC_SW */

#ifdef NXPBUILD__PHPAL_I14443P4MC

#ifndef NXPRDLIB_REM_GEN_INTFS

phStatus_t phpalI14443p4mC_ResetProtocol(
                                         void * pDataParams
                                         )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4mC_ResetProtocol");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_I14443P4MC)
    {
        status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
       case PHPAL_I14443P4MC_SW_ID:
           status = phpalI14443p4mC_Sw_ResetProtocol((phpalI14443p4mC_Sw_DataParams_t *)pDataParams);
           break;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

       default:
           status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);
           break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4mC_Receive(
                                   void * pDataParams,
                                   uint16_t wOption,
                                   uint8_t ** ppRxBuffer,
                                   uint16_t * pRxLength
                                   )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4mC_Receive");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(ppRxBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wOption_log, &wOption);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_I14443P4MC)
    {
        status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
        case PHPAL_I14443P4MC_SW_ID:
            status = phpalI14443p4mC_Sw_Receive(
                (phpalI14443p4mC_Sw_DataParams_t *)pDataParams,
                wOption,
                ppRxBuffer,
                pRxLength);
            break;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

        default:
            status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if ((((status & PH_ERR_MASK) == PH_ERR_SUCCESS) ||
        ((status & PH_ERR_MASK) == PH_ERR_SUCCESS_CHAINING) ||
        ((status & PH_ERR_MASK) == PH_ERR_SUCCESS_INCOMPLETE_BYTE)) &&
        (0U == ((wOption & PH_EXCHANGE_BUFFERED_BIT))))
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, ppRxBuffer_log, *ppRxBuffer, *pRxLength);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4mC_Transmit(
                                    void * pDataParams,
                                    uint16_t wOption,
                                    uint8_t * pTxBuffer,
                                    uint16_t  wTxLength
                                    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4mC_Transmit");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pTxBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wOption_log, &wOption);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pTxBuffer_log, pTxBuffer, wTxLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    if (0U != (wTxLength)) PH_ASSERT_NULL (pTxBuffer);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_I14443P4MC)
    {
        status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
        case PHPAL_I14443P4MC_SW_ID:
            status = phpalI14443p4mC_Sw_Transmit(
                (phpalI14443p4mC_Sw_DataParams_t *)pDataParams,
                wOption,
                pTxBuffer,
                wTxLength);
            break;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

        default:
            status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4mC_Activate(
                                    void * pDataParams,
                                    uint8_t * pRats,
                                    uint8_t bRatsLength,
                                    uint8_t * pAts,
                                    uint16_t wAtsLength
                                    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4mC_Activate");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRats);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAts);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pRats_log, pRats, bRatsLength);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAts_log, pAts, wAtsLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pRats);
    PH_ASSERT_NULL (pAts);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_I14443P4MC)
    {
        status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
        case PHPAL_I14443P4MC_SW_ID:
            status = phpalI14443p4mC_Sw_Activate(
                (phpalI14443p4mC_Sw_DataParams_t *)pDataParams,
                pRats, bRatsLength,
                pAts,
                wAtsLength);
            break;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

        default:
            status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4mC_Wtx(
                               void * pDataParams
                               )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4mC_Wtx");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_I14443P4MC)
    {
        status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
       case PHPAL_I14443P4MC_SW_ID:
           status = phpalI14443p4mC_Sw_Wtx((phpalI14443p4mC_Sw_DataParams_t *)pDataParams);
           break;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

       default:
           status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);
           break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4mC_SetConfig(
                                     void * pDataParams,
                                     uint16_t wConfig,
                                     uint16_t wValue
                                     )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4mC_SetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wValue_log, &wValue);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL(pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_I14443P4MC)
    {
        status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
        case PHPAL_I14443P4MC_SW_ID:
            status = phpalI14443p4mC_Sw_SetConfig(
                (phpalI14443p4mC_Sw_DataParams_t *)pDataParams,
                wConfig,
                wValue);
            break;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

        default:
            status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4mC_GetConfig(
                                     void * pDataParams,
                                     uint16_t wConfig,
                                     uint16_t *pValue
                                     )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4mC_GetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pValue);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_I14443P4MC)
    {
        status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4MC_SW
        case PHPAL_I14443P4MC_SW_ID:
            status = phpalI14443p4mC_Sw_GetConfig(
                (phpalI14443p4mC_Sw_DataParams_t *)pDataParams,
                wConfig,
                pValue);
            break;
#endif /* NXPBUILD__PHPAL_I14443P4MC_SW */

        default:
            status = (PH_ERR_INVALID_DATA_PARAMS | PH_COMP_PAL_I14443P4MC);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

#endif /* NXPRDLIB_REM_GEN_INTFS */

#endif/*NXPBUILD__PHPAL_I14443P4MC */
