/*----------------------------------------------------------------------------*/
/* Copyright 2012-2021 NXP                                                    */
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
* Generic Discovery Loop Activities Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phacDiscLoop.h>

#ifdef NXPBUILD__PHAC_DISCLOOP_SW
#include "Sw/phacDiscLoop_Sw.h"
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
#ifdef NXPBUILD__PHAC_DISCLOOP

#ifndef NXPRDLIB_REM_GEN_INTFS

phStatus_t phacDiscLoop_Run(void * pDataParams,
                            uint8_t bEntryPoint)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_Run");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bEntryPoint);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bEntryPoint_log, &bEntryPoint);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }

    /* Select the active layer to perform the action */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
        case PHAC_DISCLOOP_SW_ID:
            status = phacDiscLoop_Sw_Run(pDataParams, bEntryPoint);
            break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
            break;
    }
    return status;
}

phStatus_t phacDiscLoop_GetConfig(void * pDataParams,
                                  uint16_t wConfig,
                                  uint16_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_GetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pValue);

     /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
    case PHAC_DISCLOOP_SW_ID:
        status = phacDiscLoop_Sw_GetConfig(pDataParams, wConfig, pValue);
        break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}



phStatus_t phacDiscLoop_SetConfig(
    void * pDataParams,
    uint16_t wConfig,
    uint16_t wValue
    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_SetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wValue_log, &wValue);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);


    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }

    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
    case PHAC_DISCLOOP_SW_ID:
        status = phacDiscLoop_Sw_SetConfig(pDataParams, wConfig, wValue);
        break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phacDiscLoop_ActivateCard(
    void * pDataParams,
    uint8_t bTagType,
    uint8_t bTagIndex
    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_ActivateCard");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bTagType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bTagIndex);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bTagType_log, &bTagType);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bTagIndex_log, &bTagIndex);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }

    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
    case PHAC_DISCLOOP_SW_ID:
        status = phacDiscLoop_Sw_ActivateCard(pDataParams, bTagType, bTagIndex);
        break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;
}

phStatus_t phacDiscLoop_CfgPollSeq(
    void * pDataParams,
    uint8_t * pDiscloopCfgPollSeq
    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phacDiscLoop_CfgPollSeq");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDiscloopCfgPollSeq);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pDiscloopCfgPollSeq);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AC_DISCLOOP)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
    }

    /* Select the active layer to perform the action */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_SW
        case PHAC_DISCLOOP_SW_ID:
            status = phacDiscLoop_Sw_CfgPollSeq(pDataParams, pDiscloopCfgPollSeq);
            break;
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AC_DISCLOOP);
            break;
    }
    return status;
}

#endif /* NXPRDLIB_REM_GEN_INTFS */

#endif /* NXPBUILD__PHAC_DISCLOOP */
