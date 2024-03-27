/*----------------------------------------------------------------------------*/
/* Copyright 2009 - 2022 NXP                                                  */
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
* Generic ISO14443-4A Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <ph_Status.h>
#include <phpalI14443p4a.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PHPAL_I14443P4A_SW
#include "Sw/phpalI14443p4a_Sw.h"
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */




#ifdef NXPBUILD__PHPAL_I14443P4A

#ifndef NXPRDLIB_REM_GEN_INTFS

phStatus_t phpalI14443p4a_Rats(
                               void * pDataParams,
                               uint8_t bFsdi,
                               uint8_t bCid,
                               uint8_t * pAts
                               )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4a_Rats");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams);*/
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFsdi);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAts);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFsdi_log, &bFsdi);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCid_log, &bCid);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pAts);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_ISO14443P4A)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4A_SW
    case PHPAL_I14443P4A_SW_ID:
        status = phpalI14443p4a_Sw_Rats((phpalI14443p4a_Sw_DataParams_t *)pDataParams, bFsdi, bCid, pAts);
        break;
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */




    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAts_log, pAts, pAts[0]);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4a_Pps(
                              void * pDataParams,
                              uint8_t bDri,
                              uint8_t bDsi
                              )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4a_Pps");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams);*/
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bDri);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bDsi);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bDri_log, &bDri);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bDsi_log, &bDsi);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_ISO14443P4A)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4A_SW
    case PHPAL_I14443P4A_SW_ID:
        status = phpalI14443p4a_Sw_Pps((phpalI14443p4a_Sw_DataParams_t *)pDataParams, bDri, bDsi);
        break;
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */




    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4a_SetConfig(
                                    void * pDataParams,
                                    uint16_t wConfig,
                                    uint16_t wValue
                                   )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4a_SetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wValue_log, &wValue);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_ISO14443P4A)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4A_SW
    case PHPAL_I14443P4A_SW_ID:
        status = phpalI14443p4a_Sw_SetConfig((phpalI14443p4a_Sw_DataParams_t *)pDataParams, wConfig, wValue);
        break;
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */


    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4a_GetConfig(
                                        void * pDataParams,
                                        uint16_t wConfig,
                                        uint16_t *pValue
                                   )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4a_GetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_ISO14443P4A)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4A_SW
    case PHPAL_I14443P4A_SW_ID:
        status = phpalI14443p4a_Sw_GetConfig((phpalI14443p4a_Sw_DataParams_t *)pDataParams, wConfig, pValue);
        break;
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */


    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4a_ActivateCard(
                                       void * pDataParams,
                                       uint8_t bFsdi,
                                       uint8_t bCid,
                                       uint8_t bDri,
                                       uint8_t bDsi,
                                       uint8_t * pAts
                                       )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4a_ActivateCard");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams);*/
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFsdi);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bDri);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bDsi);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAts);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFsdi_log, &bFsdi);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCid_log, &bCid);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bDri_log, &bDri);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bDsi_log, &bDsi);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pAts);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_ISO14443P4A)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4A_SW
    case PHPAL_I14443P4A_SW_ID:
        status = phpalI14443p4a_Sw_ActivateCard((phpalI14443p4a_Sw_DataParams_t *)pDataParams, bFsdi, bCid, bDri, bDsi, pAts);
        break;
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */




    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAts_log, pAts, pAts[0]);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phpalI14443p4a_GetProtocolParams(
    void * pDataParams,
    uint8_t * pCidEnabled,
    uint8_t * pCid,
    uint8_t * pNadSupported,
    uint8_t * pFwi,
    uint8_t * pFsdi,
    uint8_t * pFsci
    )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phpalI14443p4a_GetProtocolParams");
    /*PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataParams);*/
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pCidEnabled);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pCid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pNadSupported);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFwi);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFsdi);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFsci);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);
    PH_ASSERT_NULL (pDataParams);
    PH_ASSERT_NULL (pCidEnabled);
    PH_ASSERT_NULL (pCid);
    PH_ASSERT_NULL (pNadSupported);
    PH_ASSERT_NULL (pFwi);
    PH_ASSERT_NULL (pFsdi);
    PH_ASSERT_NULL (pFsci);

    /* Check data parameters */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_PAL_ISO14443P4A)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHPAL_I14443P4A_SW
    case PHPAL_I14443P4A_SW_ID:
        status = phpalI14443p4a_Sw_GetProtocolParams((phpalI14443p4a_Sw_DataParams_t *)pDataParams, pCidEnabled, pCid, pNadSupported, pFwi, pFsdi, pFsci);
        break;
#endif /* NXPBUILD__PHPAL_I14443P4A_SW */




    default:
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_PAL_ISO14443P4A);
        break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, pCidEnabled_log, pCidEnabled);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, pCid_log, pCid);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, pNadSupported_log, pNadSupported);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, pFwi_log, pFwi);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, pFsdi_log, pFsdi);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, pFsci_log, pFsci);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

#endif /* NXPRDLIB_REM_GEN_INTFS */

#endif /* NXPBUILD__PHPAL_I14443P4A*/
