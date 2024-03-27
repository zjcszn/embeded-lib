/*----------------------------------------------------------------------------*/
/* Copyright 2014-2020,2022 NXP                                               */
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
* Generic MIFARE DESFire EVx contactless IC (EV1, EV2, EV3, future versions ) Application Component of Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6114 $ (v07.09.00)
* $Date: 2020-05-15 18:23:52 +0530 (Fri, 15 May 2020) $
*
*/

#include <phalMfdfEVx.h>
#include <ph_Status.h>
#include <ph_TypeDefs.h>

#ifdef NXPBUILD__PHAL_MFDFEVX_SW
#include "Sw/phalMfdfEVx_Sw.h"
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



#ifdef NXPBUILD__PHAL_MFDFEVX





/* MIFARE DESFire EVX Memory and Configuration mamangement commands. ------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_FreeMem(void * pDataParams, uint8_t * pMemInfo)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_FreeMem");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pMemInfo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pMemInfo, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_FreeMem((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pMemInfo);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pMemInfo_log, pMemInfo, 3);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}


phStatus_t phalMfdfEVx_GetVersion(void * pDataParams, uint8_t * pVerInfo)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetVersion");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pVerInfo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pVerInfo, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetVersion((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pVerInfo);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pVerInfo_log, pVerInfo, 28);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}





/* MIFARE DESFire EVX Key mamangement commands. ---------------------------------------------------------------------------------------- */

phStatus_t phalMfdfEVx_GetKeySettings(void * pDataParams, uint8_t * pKeySettings, uint8_t * bRespLen)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetKeySettings");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pKeySettings);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bRespLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pKeySettings, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetKeySettings((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pKeySettings, bRespLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pKeySettings_log, pKeySettings, (*bRespLen));
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_INFO, bRespLen_log, bRespLen);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}





/* MIFARE DESFire EVX Application mamangement commands. -------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_CreateApplication(void * pDataParams, uint8_t bOption, uint8_t * pAid, uint8_t bKeySettings1, uint8_t bKeySettings2,
    uint8_t bKeySettings3, uint8_t * pKeySetValues, uint8_t * pISOFileId, uint8_t * pISODFName, uint8_t bISODFNameLen)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CreateApplication");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bKeySettings1);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bKeySettings2);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bKeySettings3);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pKeySetValues);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pISOFileId);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pISODFName);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bISODFNameLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bKeySettings1_log, &bKeySettings1);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bKeySettings2_log, &bKeySettings2);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bKeySettings3_log, &bKeySettings3);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bISODFNameLen_log, &bISODFNameLen);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAid_log, pAid, 3);
    if(bOption & 0x01)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pISOFileId_log, pISOFileId, 2);
    }
    if((bKeySettings2 & PHAL_MFDFEVX_KEYSETT3_PRESENT) && (bKeySettings3 & PHAL_MFDFEVX_KEYSETVALUES_PRESENT))
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pKeySetValues_log, pKeySetValues, 4);
    }
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pISODFName_log, pISODFName, bISODFNameLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    if(bOption & 0x01) PH_ASSERT_NULL_PARAM(pISOFileId, PH_COMP_AL_MFDFEVX);
    if(bISODFNameLen > 0) PH_ASSERT_NULL_PARAM(pISODFName, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAid, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CreateApplication((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, pAid, bKeySettings1,
                bKeySettings2, bKeySettings3, pKeySetValues, pISOFileId, pISODFName, bISODFNameLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_DeleteApplication(void * pDataParams, uint8_t * pAid, uint8_t * pDAMMAC, uint8_t bDAMMAC_Len)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_DeleteApplication");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDAMMAC);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAid_log, pAid, 3);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pDAMMAC_log, pDAMMAC, bDAMMAC_Len);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAid, PH_COMP_AL_MFDFEVX);
    if( bDAMMAC_Len ) PH_ASSERT_NULL_PARAM(pDAMMAC, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {

#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_DeleteApplication((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pAid, pDAMMAC, bDAMMAC_Len);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_CreateDelegatedApplication(void * pDataParams, uint8_t bOption, uint8_t * pAid, uint8_t * pDamParams, uint8_t bKeySettings1,
    uint8_t bKeySettings2, uint8_t bKeySettings3, uint8_t * bKeySetValues, uint8_t * pISOFileId, uint8_t * pISODFName, uint8_t bISODFNameLen,
    uint8_t * pEncK, uint8_t * pDAMMAC)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CreateDelegatedApplication");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDamParams);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bKeySettings1);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bKeySettings2);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bKeySettings3);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bKeySetValues);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pISOFileId);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pISODFName);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bISODFNameLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pEncK);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDAMMAC);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bKeySettings1_log, &bKeySettings1);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bKeySettings2_log, &bKeySettings2);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bKeySettings3_log, &bKeySettings3);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bISODFNameLen_log, &bISODFNameLen);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAid_log, pAid, 3);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pDamParams_log, pDamParams, 4);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, bKeySetValues_log, bKeySetValues, 4);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pEncK_log, pEncK, 32);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pDAMMAC_log, pDAMMAC, 8);
    if(bOption & 0x01)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pISOFileId_log, pISOFileId, 2);
    }
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pISODFName_log, pISODFName, bISODFNameLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    if(bOption & 0x01) PH_ASSERT_NULL_PARAM(pISOFileId, PH_COMP_AL_MFDFEVX);
    if(bISODFNameLen > 0) PH_ASSERT_NULL_PARAM(pISODFName, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAid, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CreateDelegatedApplication((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, pAid,
                pDamParams, bKeySettings1, bKeySettings2, bKeySettings3, bKeySetValues, pISOFileId, pISODFName, bISODFNameLen,
                pEncK, pDAMMAC);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_SelectApplication(void * pDataParams, uint8_t bOption, uint8_t * pAid, uint8_t * pAid2)
{
    phStatus_t  PH_MEMLOC_REM status = 0;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_SelectApplication");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAid2);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAid_log, pAid, 3);
    if(bOption == 0x01)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAid2_log, pAid2, 3);
    }
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAid, PH_COMP_AL_MFDFEVX);
    if(bOption) PH_ASSERT_NULL_PARAM(pAid2, PH_COMP_AL_MFDFEVX);

    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {

#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_SelectApplication((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, pAid, pAid2);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_GetApplicationIDs(void * pDataParams, uint8_t bOption, uint8_t ** pAidBuff, uint8_t * pNumAIDs)
{
    /**
    A PICC can store any number of applications limited by the PICC memory.
    PICC will return AIDs (3 Bytes/AID) until the RxBuffer is full and initimates the
    application of RX_CHAINING. Remaining AIDs can be retreived by sending 0xAF command.
    */
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetApplicationIDs");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAidBuff);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pNumAIDs);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAidBuff, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pNumAIDs, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetApplicationIDs((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption,
                pAidBuff, pNumAIDs);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAidBuff_log, pAidBuff, (*pNumAIDs) * 3);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, pNumAIDs_log, pNumAIDs);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}


phStatus_t phalMfdfEVx_GetDelegatedInfo(void * pDataParams, uint8_t * pDAMSlot, uint8_t * pDamSlotVer, uint8_t * pQuotaLimit,
    uint8_t * pFreeBlocks, uint8_t * pAid)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetDelegatedInfo");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDAMSlot);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDamSlotVer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pQuotaLimit);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFreeBlocks);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pDAMSlot, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pDamSlotVer, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pQuotaLimit, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pFreeBlocks, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAid, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetDelegatedInfo((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pDAMSlot, pDamSlotVer, pQuotaLimit,
                pFreeBlocks, pAid);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */




        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pDAMSlot_log, pDAMSlot, 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pDamSlotVer_log, pDamSlotVer, 1);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pQuotaLimit_log, pQuotaLimit, 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pFreeBlocks_log, pFreeBlocks, 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAid_log, pAid, 3);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}




/* MIFARE DESFire EVX File mamangement commands. --------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_CreateStdDataFile(void * pDataParams, uint8_t bOption, uint8_t bFileNo, uint8_t *pISOFileId, uint8_t bFileOption,
    uint8_t *pAccessRights, uint8_t * pFileSize)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CreateStdDataFile");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pISOFileId);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAccessRights);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFileSize);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileOption_log, &bFileOption);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAccessRights_log, pAccessRights, 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pFileSize_log, pFileSize, 3);
    if(bOption == 0x01)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pISOFileId_log, pISOFileId, 2);
    }

    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    if(bOption == 0x01) PH_ASSERT_NULL_PARAM(pISOFileId, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAccessRights, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pFileSize, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CreateStdDataFile((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bFileNo,
                pISOFileId, bFileOption, pAccessRights, pFileSize);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_CreateBackupDataFile(void * pDataParams, uint8_t bOption, uint8_t bFileNo, uint8_t *pISOFileId, uint8_t bFileOption,
    uint8_t *pAccessRights, uint8_t * pFileSize)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CreateBackupDataFile");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pISOFileId);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAccessRights);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFileSize);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileOption_log, &bFileOption);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAccessRights_log, pAccessRights, 2);
    if(bOption == 0x01)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pISOFileId_log, pISOFileId, 2);
    }
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pFileSize_log, pFileSize, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    if(bOption == 1)PH_ASSERT_NULL_PARAM(pISOFileId, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAccessRights, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pFileSize, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CreateBackupDataFile((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bFileNo,
                pISOFileId, bFileOption, pAccessRights, pFileSize);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_CreateValueFile(void * pDataParams, uint8_t bFileNo, uint8_t bCommSett, uint8_t * pAccessRights, uint8_t * pLowerLmit,
    uint8_t * pUpperLmit, uint8_t * pValue, uint8_t bLimitedCredit)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CreateValueFile");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommSett);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAccessRights);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pLowerLmit);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pUpperLmit);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bLimitedCredit);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommSett_log, &bCommSett);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAccessRights_log, pAccessRights, 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pLowerLmit_log, pLowerLmit, 4);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pUpperLmit_log, pUpperLmit, 4);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue, 4);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bLimitedCredit_log, &bLimitedCredit);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAccessRights, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pLowerLmit, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pUpperLmit, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pValue, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CreateValueFile((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bFileNo, bCommSett, pAccessRights,
                pLowerLmit, pUpperLmit, pValue, bLimitedCredit);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_CreateLinearRecordFile(void * pDataParams, uint8_t bOption, uint8_t  bFileNo, uint8_t  *pIsoFileId, uint8_t bCommSett,
    uint8_t * pAccessRights, uint8_t * pRecordSize, uint8_t * pMaxNoOfRec)

{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CreateLinearRecordFile");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pIsoFileId);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommSett);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAccessRights);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRecordSize);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pMaxNoOfRec);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    if(bOption == 0x01)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pIsoFileId_log, pIsoFileId, 2);
    }
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommSett_log, &bCommSett);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAccessRights_log, pAccessRights, 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pRecordSize_log, pRecordSize, 3);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pMaxNoOfRec_log, pMaxNoOfRec, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    if(bOption == 1) PH_ASSERT_NULL_PARAM(pIsoFileId, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAccessRights, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pRecordSize, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pMaxNoOfRec, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CreateLinearRecordFile((phalMfdfEVx_Sw_DataParams_t *)pDataParams, bOption,
                bFileNo, pIsoFileId, bCommSett, pAccessRights, pRecordSize, pMaxNoOfRec);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_CreateCyclicRecordFile(void * pDataParams, uint8_t bOption, uint8_t  bFileNo, uint8_t  *pIsoFileId, uint8_t bCommSett,
    uint8_t * pAccessRights, uint8_t * pRecordSize, uint8_t * pMaxNoOfRec)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CreateCyclicRecordFile");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pIsoFileId);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommSett);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAccessRights);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRecordSize);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pMaxNoOfRec);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    if(bOption == 0x01)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pIsoFileId_log, pIsoFileId, 2);
    }
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommSett_log, &bCommSett);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAccessRights_log, pAccessRights, 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pRecordSize_log, pRecordSize, 3);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pMaxNoOfRec_log, pMaxNoOfRec, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAccessRights, PH_COMP_AL_MFDFEVX);
    if(bOption == 0x01) PH_ASSERT_NULL_PARAM(pIsoFileId, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pRecordSize, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pMaxNoOfRec, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CreateCyclicRecordFile((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption,
                bFileNo, pIsoFileId, bCommSett, pAccessRights, pRecordSize, pMaxNoOfRec);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}


phStatus_t phalMfdfEVx_DeleteFile(void * pDataParams, uint8_t bFileNo)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_DeleteFile");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_DeleteFile((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bFileNo);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_GetFileIDs(void * pDataParams, uint8_t * pFid, uint8_t * bNumFID)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetFileIDs");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bNumFID);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pFid, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(bNumFID, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetFileIDs((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pFid, bNumFID);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pFid_log, pFid, (*bNumFID));
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, bNumFID_log, bNumFID, 1);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_GetISOFileIDs(void * pDataParams, uint8_t * pFidBuffer, uint8_t * pNumFID)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetISOFileIDs");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFidBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pNumFID);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pFidBuffer, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pNumFID, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetISOFileIDs((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pFidBuffer, pNumFID);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pFidBuffer_log, pFidBuffer, (*pNumFID) * 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pNumFID_log, pNumFID, 1);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_GetFileSettings(void * pDataParams, uint8_t bFileNo, uint8_t * pFSBuffer, uint8_t * bBufferLen)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetFileSettings");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFSBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bBufferLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pFSBuffer, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(bBufferLen, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetFileSettings((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bFileNo, pFSBuffer, bBufferLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pFSBuffer_log, pFSBuffer, (*bBufferLen));
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, bBufferLen_log, bBufferLen, 1);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_GetFileCounters(void * pDataParams, uint8_t bOption, uint8_t bFileNo, uint8_t * pFileCounters, uint8_t * pRxLen)
{
    phStatus_t PH_MEMLOC_REM status = 0;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetFileCounters");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFileCounters);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRxLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    /* Component Code Validation */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetFileCounters((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bFileNo,
                pFileCounters, pRxLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pFileCounters_log, pFileCounters, (*pRxLen));
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pRxLen_log, pRxLen, 1);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_ChangeFileSettings(void * pDataParams, uint8_t bOption, uint8_t bFileNo, uint8_t bFileOption, uint8_t *pAccessRights,
    uint8_t bAddInfoLen, uint8_t * pAddInfo)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_ChangeFileSettings");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAccessRights);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bAddInfoLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pAddInfo);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileOption_log, &bFileOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bAddInfoLen_log, &bAddInfoLen);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAccessRights_log, pAccessRights, 2);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pAddInfo_log, pAddInfo, bAddInfoLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAccessRights, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAddInfo, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_ChangeFileSettings((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bFileNo,
                bFileOption, pAccessRights, bAddInfoLen, pAddInfo);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}




/* MIFARE DESFire EVX Data mamangement commands. --------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_ReadData(void * pDataParams, uint8_t bOption, uint8_t bIns, uint8_t bFileNo, uint8_t * pOffset, uint8_t * pLength,
    uint8_t ** ppRxdata, uint16_t * pRxdataLen)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_ReadData");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bIns);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pOffset);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(ppRxdata);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRxdataLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bIns_log, &bIns);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pOffset_log, pOffset, 3);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pLength_log, pLength, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pOffset, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pLength, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(ppRxdata, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pRxdataLen, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_ReadData((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bIns, bFileNo,
                pOffset, pLength, ppRxdata, pRxdataLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    if(*pRxdataLen != 0)
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, ppRxdata_log, *ppRxdata, (*pRxdataLen));
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, pRxdataLen_log, pRxdataLen);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_WriteData(void * pDataParams, uint8_t bOption, uint8_t bIns, uint8_t bFileNo, uint8_t * pOffset, uint8_t * pTxData,
    uint8_t * pTxDataLen)
{
    phStatus_t  PH_MEMLOC_REM status;
    uint16_t    PH_MEMLOC_REM wDataLen;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_WriteData");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bIns);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pOffset);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pTxData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pTxDataLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bIns_log, &bIns);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pOffset_log, pOffset, 3);

    wDataLen = (uint16_t) pTxDataLen[1];
    wDataLen = wDataLen << 8;
    wDataLen |= pTxDataLen[0];

    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pTxData_log, pTxData, wDataLen);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pTxDataLen_log, pTxDataLen, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pOffset, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pTxData, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pTxDataLen, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_WriteData((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bIns, bFileNo,
                pOffset, pTxData, pTxDataLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_GetValue(void * pDataParams, uint8_t bOption, uint8_t bFileNo, uint8_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetValue");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pValue, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetValue((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bFileNo, pValue);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue, 4);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_Credit(void * pDataParams, uint8_t bOption, uint8_t bFileNo, uint8_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_Credit");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pValue, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_Credit((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bFileNo, pValue);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue, 4);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
    return status;

}

phStatus_t phalMfdfEVx_Debit(void * pDataParams, uint8_t bCommOption, uint8_t bFileNo, uint8_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_Debit");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommOption_log, &bCommOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pValue, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_Debit((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bCommOption, bFileNo, pValue);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue, 4);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_LimitedCredit(void * pDataParams, uint8_t bCommOption, uint8_t bFileNo, uint8_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_LimitedCredit");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommOption_log, &bCommOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue, 4);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pValue, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_LimitedCredit((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bCommOption,
                bFileNo, pValue);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_WriteRecord(void * pDataParams, uint8_t bCommOption, uint8_t bIns, uint8_t bFileNo, uint8_t * pOffset, uint8_t * pData,
    uint8_t * pDataLen)
{
    phStatus_t  PH_MEMLOC_REM status;
    uint16_t    PH_MEMLOC_REM wDataLen;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_WriteRecord");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bIns);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pOffset);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommOption_log, &bCommOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bIns_log, &bIns);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pOffset_log, pOffset, 3);

    wDataLen = (uint16_t) pDataLen[1];
    wDataLen = wDataLen << 8;
    wDataLen |= pDataLen[0];

    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pData_log, pData, wDataLen);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pDataLen_log, pDataLen, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pOffset, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pData, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pDataLen, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_WriteRecord((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bCommOption, bIns, bFileNo, pOffset, pData, pDataLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_ReadRecords(void * pDataParams, uint8_t bCommOption, uint8_t bIns, uint8_t bFileNo, uint8_t * pRecNo, uint8_t * pRecCount,
    uint8_t * pRecSize, uint8_t ** ppRxdata, uint16_t * pRxdataLen)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_ReadRecords");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bIns);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRecNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRecCount);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRecSize);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(ppRxdata);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRxdataLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommOption_log, &bCommOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bIns_log, &bIns);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pRecNo_log, pRecNo, 3);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pRecCount_log, pRecCount, 3);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pRecSize_log, pRecSize, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pRecNo, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pRecCount, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(ppRxdata, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pRxdataLen, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pRecSize, PH_COMP_AL_MFDFEVX);

    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_ReadRecords((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bCommOption, bIns, bFileNo, pRecNo,
                pRecCount, pRecSize, ppRxdata, pRxdataLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    if(*pRxdataLen != 0)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, ppRxdata_log, *ppRxdata, (*pRxdataLen));
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, pRxdataLen_log, pRxdataLen);
    }
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_UpdateRecord(void * pDataParams, uint8_t bCommOption, uint8_t bIns, uint8_t bFileNo, uint8_t * pRecNo, uint8_t * pOffset,
    uint8_t * pData, uint8_t * pDataLen)
{
    phStatus_t PH_MEMLOC_REM status;
    uint16_t    PH_MEMLOC_REM wDataLen;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_UpdateRecord");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bIns);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRecNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pOffset);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDataLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommOption_log, &bCommOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bIns_log, &bIns);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pRecNo_log, pRecNo, 3);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pOffset_log, pOffset, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    wDataLen = (uint16_t) pDataLen[1];
    wDataLen = wDataLen << 8;
    wDataLen |= pDataLen[0];

    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pData_log, pData, wDataLen);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pDataLen_log, pDataLen, 3);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pOffset, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pData, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pDataLen, PH_COMP_AL_MFDFEVX);

    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_UpdateRecord((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bCommOption, bIns,
                bFileNo, pRecNo, pOffset, pData, pDataLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_ClearRecordFile(void * pDataParams, uint8_t bFileNo)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_ClearRecordFile");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bFileNo_log, &bFileNo);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_ClearRecordFile((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bFileNo);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */




        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}




/* MIFARE DESFire EVX Transaction mamangement commands. -------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_CommitTransaction(void * pDataParams, uint8_t bOption, uint8_t * pTMC, uint8_t * pTMV)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CommitTransaction");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pTMC);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pTMV);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_INFO, bOption_log, &bOption);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    if(bOption & 0x01)
    {
        PH_ASSERT_NULL_PARAM(pTMC, PH_COMP_AL_MFDFEVX);
        PH_ASSERT_NULL_PARAM(pTMV, PH_COMP_AL_MFDFEVX);
    }

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CommitTransaction((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption,
                pTMC, pTMV);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    if(bOption & 0x01)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pTMC_log, pTMC, 4);
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pTMV_log, pTMV, 8);
    }
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_AbortTransaction(void * pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_AbortTransaction");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_AbortTransaction((phalMfdfEVx_Sw_DataParams_t *) pDataParams);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_CommitReaderID(void * pDataParams, uint8_t * pTMRI, uint8_t * pEncTMRI)

{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CommitReaderID");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pTMRI);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pEncTMRI);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pTMRI_log, pTMRI, 16);
    }
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    {
        PH_ASSERT_NULL_PARAM(pTMRI, PH_COMP_AL_MFDFEVX);
    }
    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CommitReaderID((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pTMRI, pEncTMRI);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pEncTMRI_log, pEncTMRI, 16);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}




/* MIFARE DESFire EVX ISO7816-4 commands. ---------------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_IsoSelectFile(void * pDataParams, uint8_t bOption, uint8_t bSelector, uint8_t * pFid, uint8_t * pDFname,
    uint8_t bDFnameLen, uint8_t bExtendedLenApdu, uint8_t **ppFCI, uint16_t *pwFCILen)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_IsoSelectFile");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bSelector);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pFid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDFname);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bDFnameLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOption_log, &bOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bSelector_log, &bSelector);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pFid_log, pFid, 2);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bDFnameLen_log, &bDFnameLen);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pDFname_log, pDFname, (uint16_t) bDFnameLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_IsoSelectFile((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOption, bSelector,
                pFid, pDFname, bDFnameLen, bExtendedLenApdu, ppFCI, pwFCILen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_IsoReadBinary(void * pDataParams, uint16_t wOption, uint8_t bOffset, uint8_t bSfid, uint32_t dwBytesToRead,
    uint8_t bExtendedLenApdu, uint8_t ** ppRxBuffer, uint32_t * pBytesRead)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_IsoReadBinary");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOffset);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bSfid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(dwBytesToRead);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(ppRxBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pBytesRead);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOffset_log, &bOffset);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bSfid_log, &bSfid);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, dwBytesToRead_log, &dwBytesToRead);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(ppRxBuffer, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pBytesRead, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_IsoReadBinary((phalMfdfEVx_Sw_DataParams_t *) pDataParams, wOption, bOffset,
                bSfid, dwBytesToRead, bExtendedLenApdu, ppRxBuffer, pBytesRead);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, ppRxBuffer_log, *ppRxBuffer, (uint16_t) (*pBytesRead));
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_INFO, pBytesRead_log, pBytesRead);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_IsoUpdateBinary(void * pDataParams, uint8_t bOffset, uint8_t bSfid, uint8_t bExtendedLenApdu, uint8_t * pData,
    uint32_t dwDataLen)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_IsoUpdateBinary");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bOffset);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bSfid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(dwDataLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);

    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bOffset_log, &bOffset);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bSfid_log, &bSfid);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pData_log, pData, (uint16_t) dwDataLen);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_INFO, dwDataLen_log, &dwDataLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pData, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_IsoUpdateBinary((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bOffset, bSfid,
                bExtendedLenApdu, pData, dwDataLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_IsoReadRecords(void * pDataParams, uint16_t wOption, uint8_t bRecNo, uint8_t bReadAllFromP1, uint8_t bSfid,
    uint32_t dwBytesToRead, uint8_t bExtendedLenApdu, uint8_t ** ppRxBuffer, uint32_t * pBytesRead)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_IsoReadRecords");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bRecNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bReadAllFromP1);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bSfid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(dwBytesToRead);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(ppRxBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pBytesRead);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bRecNo_log, &bRecNo);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bReadAllFromP1_log, &bReadAllFromP1);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bSfid_log, &bSfid);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, dwBytesToRead_log, &dwBytesToRead);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(ppRxBuffer, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pBytesRead, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_IsoReadRecords((phalMfdfEVx_Sw_DataParams_t *) pDataParams, wOption, bRecNo, bReadAllFromP1,
                bSfid, dwBytesToRead, bExtendedLenApdu, ppRxBuffer, pBytesRead);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, ppRxBuffer_log, *ppRxBuffer, (uint16_t) (*pBytesRead));
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, pBytesRead_log, pBytesRead);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_IsoAppendRecord(void * pDataParams, uint8_t bSfid, uint8_t * pData, uint32_t dwDataLen, uint8_t bExtendedLenApdu)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_IsoAppendRecord");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bSfid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(dwDataLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bSfid_log, &bSfid);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pData_log, pData, (uint16_t) dwDataLen);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, dwDataLen_log, &dwDataLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pData, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_IsoAppendRecord((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bSfid, bExtendedLenApdu,
                pData, dwDataLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_IsoUpdateRecord(void * pDataParams, uint8_t bIns, uint8_t bRecNo, uint8_t bSfid, uint8_t bRefCtrl, uint8_t * pData,
    uint8_t bDataLen)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_IsoUpdateRecord");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bIns);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bRecNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bSfid);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bRefCtrl);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bDataLen);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bIns_log, &bIns);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bRecNo_log, &bRecNo);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bSfid_log, &bSfid);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bRefCtrl_log, &bRefCtrl);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pData_log, pData, bDataLen);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bDataLen_log, &bDataLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pData, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_IsoUpdateRecord((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bIns, bRecNo,
                bSfid, bRefCtrl, pData, bDataLen);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_IsoGetChallenge(void * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer, uint8_t bExtendedLenApdu, uint32_t dwLe,
    uint8_t * pRPICC1)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_IsoGetChallenge");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyVer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(dwLe);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRPICC1);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wKeyNo_log, &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wKeyVer_log, &wKeyVer);
    PH_LOG_HELPER_ADDPARAM_UINT32(PH_LOG_LOGTYPE_DEBUG, dwLe_log, &dwLe);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pRPICC1, PH_COMP_AL_MFDFEVX);

    /* Parameter validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_IsoGetChallenge((phalMfdfEVx_Sw_DataParams_t *) pDataParams, wKeyNo, wKeyVer, bExtendedLenApdu,
                dwLe, pRPICC1);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_INFO, pRPICC1_log, pRPICC1, 8);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}





/* MIFARE DESFire EVX Originality Check functions. ------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_ReadSign(void * pDataParams, uint8_t bAddr, uint8_t ** pSignature)
{
    phStatus_t PH_MEMLOC_REM status = 0;
    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_ReadSign");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bAddr);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pSignature);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bAddr_log, &bAddr);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pSignature, PH_COMP_AL_MFDFEVX);

    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return  PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_ReadSign((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bAddr, pSignature);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, pSignature_log, *pSignature, 56);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}




/* MIFARE DESFire EVX MIFARE Classic contactless IC functions. ---------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_CreateMFCMapping(void * pDataParams, uint8_t bCommOption, uint8_t bFileNo, uint8_t bFileOption, uint8_t * pMFCBlockList,
    uint8_t bMFCBlocksLen, uint8_t bRestoreSource, uint8_t * pMFCLicense, uint8_t bMFCLicenseLen, uint8_t * pMFCLicenseMAC)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_CreateMFCMapping");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bFileOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pMFCBlockList);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bMFCBlocksLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bRestoreSource);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pMFCLicense);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bMFCLicenseLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pMFCLicenseMAC);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_DATA_PARAM(pMFCBlockList, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_DATA_PARAM(pMFCLicense, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_DATA_PARAM(pMFCLicenseMAC, PH_COMP_AL_MFDFEVX);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bCommOption), &bCommOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bFileNo), &bFileNo);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bFileOption), &bFileOption);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pMFCBlockList), pMFCBlockList, bMFCBlocksLen);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bMFCBlocksLen), &bMFCBlocksLen);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bRestoreSource), &bRestoreSource);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pMFCLicense), pMFCLicense, bMFCLicenseLen);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bMFCLicenseLen), &bMFCLicenseLen);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pMFCLicenseMAC), pMFCLicenseMAC, 8);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_CreateMFCMapping((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bCommOption, bFileNo, bFileOption, pMFCBlockList,
                bMFCBlocksLen, bRestoreSource, pMFCLicense, bMFCLicenseLen, pMFCLicenseMAC);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_RestoreTransfer(void * pDataParams,uint8_t bCommOption, uint8_t bTargetFileNo, uint8_t bSourceFileNo)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_RestoreTransfer");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bCommOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bTargetFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bSourceFileNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, bCommOption_log, &bCommOption);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bTargetFileNo), &bTargetFileNo);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bSourceFileNo), &bSourceFileNo);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if (PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch (PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_RestoreTransfer((phalMfdfEVx_Sw_DataParams_t *) pDataParams, bCommOption ,bTargetFileNo, bSourceFileNo);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}





/* MIFARE DESFire EVX Miscellaneous functions. ----------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_GetConfig(void * pDataParams, uint16_t wConfig, uint16_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_GetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pValue, PH_COMP_AL_MFDFEVX);

    /* Check data parameters */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_GetConfig((phalMfdfEVx_Sw_DataParams_t *) pDataParams, wConfig, pValue);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, pValue_log, pValue);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_SetConfig(void * pDataParams, uint16_t wConfig, uint16_t wValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_SetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wConfig_log, &wConfig);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, wValue_log, &wValue);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    /* Check data parameters */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_SetConfig((phalMfdfEVx_Sw_DataParams_t *) pDataParams, wConfig, wValue);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);

    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phalMfdfEVx_ResetAuthentication(void * pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_ResetAuthentication");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);

    /* Check data parameters */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_ResetAuthentication((phalMfdfEVx_Sw_DataParams_t *) pDataParams);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);

    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}


phStatus_t phalMfdfEVx_SetVCAParams(void * pDataParams, void * pAlVCADataParams)
{
    phStatus_t PH_MEMLOC_REM status = 0;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phalMfdfEVx_SetVCAParams");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Validate the parameters */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_AL_MFDFEVX);
    PH_ASSERT_NULL_PARAM(pAlVCADataParams, PH_COMP_AL_MFDFEVX);

    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_AL_MFDFEVX)
    {
        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return  PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
    }

    /* Perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PHAL_MFDFEVX_SW
        case PHAL_MFDFEVX_SW_ID:
            status = phalMfdfEVx_Sw_SetVCAParams((phalMfdfEVx_Sw_DataParams_t *) pDataParams, pAlVCADataParams);
            break;
#endif /* NXPBUILD__PHAL_MFDFEVX_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_MFDFEVX);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, status_log, &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

#endif /* NXPBUILD__PHAL_MFDFEVX */
