/*----------------------------------------------------------------------------*/
/* Copyright 2016, 2017, 2022 NXP                                             */
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
* Generic Tag Operation Application Layer Component of
* Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW
#include "Sw/phalTop_Sw.h"
#endif /* NXPBUILD__PHAL_TOP_SW */

#ifdef NXPBUILD__PHAL_TOP

#ifndef NXPRDLIB_REM_GEN_INTFS

phStatus_t phalTop_ReadNdef(
                            void * pDataParams,
                            uint8_t * pData,
                            uint32_t * pLength
                            )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_ReadNdef");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pData);
    PH_ASSERT_NULL (pLength);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_TOP_SW
    case PHAL_TOP_SW_ID:
        status = phalTop_Sw_ReadNdef(pDataParams, pData, pLength);
        break;
#endif /* NXPBUILD__PHAL_TOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if ( (status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pData_log, pData, *pLength);
    }
#endif /* NXPBUILD__PH_LOG */
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phalTop_WriteNdef(
                             void * pDataParams,
                             uint8_t * pData,
                             uint32_t dwLength
                             )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_WriteNdef");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(dwLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, dwLength_log, &dwLength);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pData_log, pData, dwLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pData);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_TOP_SW
    case PHAL_TOP_SW_ID:
        status = phalTop_Sw_WriteNdef(pDataParams, pData, dwLength);
        break;
#endif /* NXPBUILD__PHAL_TOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phalTop_CheckNdef(
                             void * pDataParams,
                             uint8_t * pTagState
                             )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_CheckNdef");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pTagState);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pTagState);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_TOP_SW
    case PHAL_TOP_SW_ID:
        status = phalTop_Sw_CheckNdef(pDataParams, pTagState);
        break;
#endif /* NXPBUILD__PHAL_TOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pTagState_log, pTagState, 1);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phalTop_Reset(
                         void * pDataParams
                         )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_Reset");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_TOP_SW
    case PHAL_TOP_SW_ID:
        status = phalTop_Sw_Reset(pDataParams);
        break;
#endif /* NXPBUILD__PHAL_TOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phalTop_SetConfig(
                             void * pDataParams,
                             uint16_t wConfig,
                             uint32_t dwValue
                             )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_SetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(dwValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, dwValue_log, &dwValue);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_TOP_SW
    case PHAL_TOP_SW_ID:
        status = phalTop_Sw_SetConfig(pDataParams, wConfig, dwValue);
        break;
#endif /* NXPBUILD__PHAL_TOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phalTop_GetConfig(
                             void * pDataParams,
                             uint16_t wConfig,
                             uint32_t * pValue
                             )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_GetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pValue);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_TOP_SW
    case PHAL_TOP_SW_ID:
        status = phalTop_Sw_GetConfig(pDataParams, wConfig, pValue);
        break;
#endif /* NXPBUILD__PHAL_TOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phalTop_EraseNdef(
                             void * pDataParams
                             )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_Sw_EraseNdef");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_TOP_SW
    case PHAL_TOP_SW_ID:
        status = phalTop_Sw_EraseNdef(pDataParams);
        break;
#endif /* NXPBUILD__PHAL_TOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phalTop_FormatNdef(
                              void * pDataParams
                              )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_FormatNdef");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_TOP_SW
    case PHAL_TOP_SW_ID:
        status = phalTop_Sw_FormatNdef(pDataParams);
        break;
#endif /* NXPBUILD__PHAL_TOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phalTop_LockBlock(
                             void * pDataParams,
                             uint16_t wBlockNum
                             )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_LockBlock");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wBlockNum);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

        /* Check data parameters */
        if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
        {
            PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
            PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
            PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        }

        switch (PH_GET_COMPID(pDataParams))
        {
#ifdef NXPBUILD__PHAL_TOP_SW
        case PHAL_TOP_SW_ID:
            status = phalTop_Sw_LockBlock(pDataParams, wBlockNum);
            break;
#endif /* NXPBUILD__PHAL_TOP_SW */
        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
            break;
        }

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return status;
}


#ifdef __DEBUG

phStatus_t phalTop_SetPtr(
                          void * pDataParams,
                          void * pT1T,
                          void * pT2T,
                          void * pT3T,
                          void * pT4T,
                          void * pT5T,
                          void * pMfcTop
                          )
{
    phStatus_t PH_MEMLOC_REM status;
    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalTop_SetPtr");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams); */
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pT1T);
    PH_ASSERT_NULL (pT2T);
    PH_ASSERT_NULL (pT3T);
    PH_ASSERT_NULL (pT4T);
    PH_ASSERT_NULL (pT5T);
    PH_ASSERT_NULL (pMfcTop);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_TOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    status = phalTop_Sw_SetPtr(pDataParams,
        pT1T,
        pT2T,
        pT3T,
        pT4T,
        pT5T,
        pMfcTop
        );

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}
#endif /* #if __DEBUG */

#endif /* NXPRDLIB_REM_GEN_INTFS */

#endif /* NXPBUILD__PHAL_TOP */
