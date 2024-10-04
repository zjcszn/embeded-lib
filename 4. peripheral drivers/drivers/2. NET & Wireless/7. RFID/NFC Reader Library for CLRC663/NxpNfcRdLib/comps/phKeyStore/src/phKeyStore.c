/*----------------------------------------------------------------------------*/
/* Copyright 2006 - 2017, 2021, 2022 NXP                                      */
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
* Generic KeyStore Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6505 $ (v07.09.00)
* $Date: 2021-12-06 12:39:03 +0530 (Mon, 06 Dec 2021) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#include <ph_Status.h>
#include <phKeyStore.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PH_KEYSTORE_SW
#include "Sw/phKeyStore_Sw.h"
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
#include "Rc663/phKeyStore_Rc663.h"
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



#ifdef NXPBUILD__PH_KEYSTORE

#ifndef NXPRDLIB_REM_GEN_INTFS

/* Common Interfaces ------------------------------------------------------------------------------------------------------------------- */
phStatus_t phKeyStore_FormatKeyEntry(void * pDataParams, uint16_t wKeyNo, uint16_t wNewKeyType)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_FormatKeyEntry");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wNewKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wNewKeyType), &wNewKeyType);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_FormatKeyEntry((phKeyStore_Sw_DataParams_t *) pDataParams, wKeyNo, wNewKeyType);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_FormatKeyEntry((phKeyStore_Rc663_DataParams_t *) pDataParams, wKeyNo, wNewKeyType);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_SetKUC(void * pDataParams, uint16_t wKeyNo, uint16_t wRefNoKUC)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_SetKUC");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wRefNoKUC);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wRefNoKUC), &wRefNoKUC);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_SetKUC((phKeyStore_Sw_DataParams_t *) pDataParams, wKeyNo, wRefNoKUC);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_SetKUC((phKeyStore_Rc663_DataParams_t *) pDataParams, wKeyNo, wRefNoKUC);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_GetKUC(void * pDataParams, uint16_t wRefNoKUC, uint32_t * pdwLimit, uint32_t * pdwCurVal)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_GetKUC");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wRefNoKUC);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pdwLimit);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pdwCurVal);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pdwLimit, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pdwCurVal, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wRefNoKUC), &wRefNoKUC);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_GetKUC((phKeyStore_Sw_DataParams_t *) pDataParams, wRefNoKUC, pdwLimit, pdwCurVal);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_GetKUC((phKeyStore_Rc663_DataParams_t *) pDataParams, wRefNoKUC, pdwLimit, pdwCurVal);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pdwLimit), pdwLimit);
        PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pdwCurVal), pdwCurVal);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_ChangeKUC(void * pDataParams, uint16_t wRefNoKUC, uint32_t dwLimit)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_ChangeKUC");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wRefNoKUC);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(dwLimit);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wRefNoKUC), &wRefNoKUC);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(dwLimit), &dwLimit);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_ChangeKUC((phKeyStore_Sw_DataParams_t *) pDataParams, wRefNoKUC, dwLimit);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_ChangeKUC((phKeyStore_Rc663_DataParams_t *) pDataParams, wRefNoKUC, dwLimit);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_SetConfig(void * pDataParams, uint16_t wConfig, uint16_t wValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_SetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wConfig), &wConfig);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wValue), &wValue);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_SetConfig((phKeyStore_Sw_DataParams_t *) pDataParams, wConfig, wValue);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_SetConfig((phKeyStore_Rc663_DataParams_t *) pDataParams, wConfig, wValue);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_SetConfigStr(void * pDataParams, uint16_t wConfig, uint8_t *pBuffer, uint16_t wBufferLength)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_SetConfigStr");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wBufferLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pBuffer, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wConfig), &wConfig);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pBuffer), pBuffer, wBufferLength);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wBufferLength), &wBufferLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_SetConfigStr((phKeyStore_Sw_DataParams_t *) pDataParams, wConfig, pBuffer, wBufferLength);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_SetConfigStr((phKeyStore_Rc663_DataParams_t *) pDataParams, wConfig, pBuffer, wBufferLength);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_GetConfig(void * pDataParams, uint16_t wConfig, uint16_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_GetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pValue, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wConfig), &wConfig);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_GetConfig((phKeyStore_Sw_DataParams_t *) pDataParams, wConfig, pValue);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_GetConfig((phKeyStore_Rc663_DataParams_t *) pDataParams, wConfig, pValue);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pValue), pValue);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_GetConfigStr(void * pDataParams, uint16_t wConfig, uint8_t ** ppBuffer, uint16_t * pBufferLength)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_GetConfigStr");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(ppBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pBufferLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(ppBuffer, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pBufferLength, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wConfig), &wConfig);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_GetConfigStr((phKeyStore_Sw_DataParams_t *) pDataParams, wConfig, ppBuffer, pBufferLength);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_GetConfigStr((phKeyStore_Rc663_DataParams_t *) pDataParams, wConfig, ppBuffer, pBufferLength);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(ppBuffer), *ppBuffer, *pBufferLength);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pBufferLength), pBufferLength);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}





/* Interfaces for Symmetric Keys ------------------------------------------------------------------------------------------------------- */
phStatus_t phKeyStore_SetKey(void * pDataParams, uint16_t wKeyNo, uint16_t wKeyVersion, uint16_t wKeyType, uint8_t * pNewKey,
    uint16_t wNewKeyVersion)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_SetKey");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyVersion);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pNewKey);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wNewKeyVersion);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pNewKey, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyVersion), &wKeyVersion);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyType), &wKeyType);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pNewKey), pNewKey, phKeyStore_GetKeySize(wKeyType));
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wNewKeyVersion), &wNewKeyVersion);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_SetKey((phKeyStore_Sw_DataParams_t *) pDataParams, wKeyNo, wKeyVersion, wKeyType, pNewKey, wNewKeyVersion);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_SetKey((phKeyStore_Rc663_DataParams_t *) pDataParams, wKeyNo, wKeyVersion, wKeyType, pNewKey, wNewKeyVersion);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_SetKeyAtPos(void * pDataParams, uint16_t wKeyNo, uint16_t wPos, uint16_t wKeyType, uint8_t * pNewKey,
    uint16_t wNewKeyVersion)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_SetKeyAtPos");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wPos);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pNewKey);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wNewKeyVersion);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pNewKey, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wPos), &wPos);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyType), &wKeyType);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pNewKey), pNewKey, phKeyStore_GetKeySize(wKeyType));
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wNewKeyVersion), &wNewKeyVersion);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_SetKeyAtPos((phKeyStore_Sw_DataParams_t *) pDataParams, wKeyNo, wPos, wKeyType, pNewKey, wNewKeyVersion);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_SetKeyAtPos((phKeyStore_Rc663_DataParams_t *) pDataParams, wKeyNo, wPos, wKeyType, pNewKey, wNewKeyVersion);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_SetFullKeyEntry(void * pDataParams, uint16_t wNoOfKeys, uint16_t wKeyNo, uint16_t wNewRefNoKUC,
    uint16_t wNewKeyType, uint8_t * pNewKeys, uint16_t * pNewKeyVersionList)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_SetFullKeyEntry");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wNoOfKeys);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wNewRefNoKUC);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wNewKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pNewKeys);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pNewKeyVersionList);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pNewKeys, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pNewKeyVersionList, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wNoOfKeys), &wNoOfKeys);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wNewRefNoKUC), &wNewRefNoKUC);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wNewKeyType), &wNewKeyType);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pNewKeys), pNewKeys, phKeyStore_GetKeySize(wNewKeyType)*wNoOfKeys);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pNewKeyVersionList), pNewKeyVersionList, wNoOfKeys);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_SetFullKeyEntry((phKeyStore_Sw_DataParams_t *) pDataParams, wNoOfKeys, wKeyNo, wNewRefNoKUC, wNewKeyType, pNewKeys, pNewKeyVersionList);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_SetFullKeyEntry((phKeyStore_Rc663_DataParams_t *) pDataParams, wNoOfKeys, wKeyNo, wNewRefNoKUC, wNewKeyType, pNewKeys, pNewKeyVersionList);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_GetKeyEntry(void * pDataParams, uint16_t wKeyNo, uint16_t wKeyVersionBufSize, uint16_t * wKeyVersion,
    uint16_t * wKeyVersionLength, uint16_t * pKeyType)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_GetKeyEntry");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyVersionBufSize);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyVersion);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(wKeyVersion, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(wKeyVersionLength, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pKeyType, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyVersionBufSize), &wKeyVersionBufSize);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_GetKeyEntry((phKeyStore_Sw_DataParams_t *) pDataParams, wKeyNo, wKeyVersionBufSize, wKeyVersion, wKeyVersionLength, pKeyType);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_GetKeyEntry((phKeyStore_Rc663_DataParams_t *) pDataParams, wKeyNo, wKeyVersionBufSize, wKeyVersion, wKeyVersionLength, pKeyType);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyVersion), wKeyVersion, *wKeyVersionLength);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pKeyType), pKeyType);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phKeyStore_GetKey(void * pDataParams, uint16_t wKeyNo, uint16_t wKeyVersion, uint8_t bKeyBufSize, uint8_t * pKey,
    uint16_t * pKeyType)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phKeyStore_GetKey");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyVersion);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bKeyBufSize);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pKey);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pKey, PH_COMP_KEYSTORE);
    PH_ASSERT_NULL_PARAM(pKeyType, PH_COMP_KEYSTORE);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyVersion), &wKeyVersion);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bKeyBufSize), &bKeyBufSize);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_KEYSTORE)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_KEYSTORE_SW
        case PH_KEYSTORE_SW_ID:
            status = phKeyStore_Sw_GetKey((phKeyStore_Sw_DataParams_t *) pDataParams, wKeyNo, wKeyVersion, bKeyBufSize, pKey, pKeyType);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_SW */



#ifdef NXPBUILD__PH_KEYSTORE_RC663
        case PH_KEYSTORE_RC663_ID:
            status = phKeyStore_Rc663_GetKey((phKeyStore_Rc663_DataParams_t *) pDataParams, wKeyNo, wKeyVersion, bKeyBufSize, pKey, pKeyType);
            break;
#endif /* NXPBUILD__PH_KEYSTORE_RC663 */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_KEYSTORE);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pKey), pKey, phKeyStore_GetKeySize(*pKeyType));
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pKeyType), pKeyType);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}






#endif /* NXPRDLIB_REM_GEN_INTFS  */

/* Utility Interfaces ------------------------------------------------------------------------------------------------------------------ */
uint8_t phKeyStore_GetKeySize(uint16_t wKeyType)
{
    switch(wKeyType)
    {
        case PH_KEYSTORE_KEY_TYPE_MIFARE:
            /* 6 bytes for key A + 6 bytes for key B */
            return 12;

        case PH_KEYSTORE_KEY_TYPE_DES:
            return 8;

        case PH_KEYSTORE_KEY_TYPE_2K3DES:
        case PH_KEYSTORE_KEY_TYPE_AES128:
            return 16;

        case PH_KEYSTORE_KEY_TYPE_3K3DES:
        case PH_KEYSTORE_KEY_TYPE_AES192:
            return 24;

        case PH_KEYSTORE_KEY_TYPE_AES256:
            return 32;

        default:
            return 0;
    }
}
#endif /* NXPBUILD__PH_KEYSTORE */
