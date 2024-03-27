/*----------------------------------------------------------------------------*/
/* Copyright 2009 - 2020, 2022 NXP                                            */
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
* Generic Random Number Component of the Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6656 $ (v07.09.00)
* $Date: 2022-06-16 20:27:12 +0530 (Thu, 16 Jun 2022) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/
#include <phCryptoRng.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PH_CRYPTORNG_SW
#include "Sw/phCryptoRng_Sw.h"
#endif /* NXPBUILD__PH_CRYPTORNG_SW */



#ifdef NXPBUILD__PH_CRYPTORNG
phStatus_t phCryptoRng_Seed(void * pDataParams, uint8_t * pSeed, uint8_t bSeedLength)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoRng_Seed");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pSeed);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bSeedLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTORNG);
    if(0U != bSeedLength) PH_ASSERT_NULL_PARAM(pSeed, PH_COMP_CRYPTORNG);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pSeed), pSeed, bSeedLength);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bSeedLength), &bSeedLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTORNG)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTORNG);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTORNG_SW
        case PH_CRYPTORNG_SW_ID:
            status = phCryptoRng_Sw_Seed((phCryptoRng_Sw_DataParams_t *) pDataParams, pSeed, bSeedLength);
            break;
#endif /* NXPBUILD__PH_CRYPTORNG_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTORNG);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoRng_Rnd(void * pDataParams, uint16_t  wNoOfRndBytes, uint8_t * pRnd)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoRng_Rnd");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wNoOfRndBytes);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pRnd);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTORNG);
    PH_ASSERT_NULL_PARAM(pRnd, PH_COMP_CRYPTORNG);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wNoOfRndBytes), &wNoOfRndBytes);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTORNG)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTORNG);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTORNG_SW
        case PH_CRYPTORNG_SW_ID:
            status = phCryptoRng_Sw_Rnd((phCryptoRng_Sw_DataParams_t *) pDataParams, wNoOfRndBytes, pRnd);
            break;
#endif /* NXPBUILD__PH_CRYPTORNG_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTORNG);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pRnd), pRnd, wNoOfRndBytes);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoRng_GetLastStatus(void * pDataParams, uint16_t wStatusMsgLen, int8_t * pStatusMsg, int32_t * pStatusCode)
{
    phStatus_t PH_MEMLOC_REM wStatus = 0;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoRng_GetLastStatus");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wStatusMsgLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pStatusMsg);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pStatusCode);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wStatus);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTORNG);
    PH_ASSERT_NULL_PARAM(pStatusMsg, PH_COMP_CRYPTORNG);
    PH_ASSERT_NULL_PARAM(pStatusCode, PH_COMP_CRYPTORNG);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wStatusMsgLen), &wStatusMsgLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTORNG)
    {
        wStatus = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTORNG);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(wStatus), &wStatus);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return wStatus;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {

        default:
            wStatus = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTORNG);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pStatusMsg), pStatusMsg, wStatusMsgLen);
        PH_LOG_HELPER_ADDPARAM_INT32(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pStatusCode), pStatusCode);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(wStatus), &wStatus);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return wStatus;
}

#endif /* NXPBUILD__PH_CRYPTORNG */
