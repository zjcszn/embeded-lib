/*----------------------------------------------------------------------------*/
/* Copyright 2009 - 2019, 2022 NXP                                            */
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
* Generic Symmetric Cryptography Component of the Reader Library Framework.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6656 $ (v07.09.00)
* $Date: 2022-06-16 20:27:12 +0530 (Thu, 16 Jun 2022) $
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#include <ph_Status.h>
#include <phCryptoSym.h>
#include <ph_RefDefs.h>

#ifdef NXPBUILD__PH_CRYPTOSYM_SW
#include "Sw/phCryptoSym_Sw.h"
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



#ifdef NXPBUILD__PH_CRYPTOSYM

phStatus_t phCryptoSym_InvalidateKey(void * pDataParams)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_InvalidateKey");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_InvalidateKey((phCryptoSym_Sw_DataParams_t *) pDataParams);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_Encrypt(void * pDataParams, uint16_t wOption, const uint8_t * pPlainBuffer, uint16_t wBufferLength,
    uint8_t * pEncryptedBuffer)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_Encrypt");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pPlainBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wBufferLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pEncryptedBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pPlainBuffer, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pEncryptedBuffer, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wOption), &wOption);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wBufferLength), &wBufferLength);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pPlainBuffer), pPlainBuffer, wBufferLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_Encrypt((phCryptoSym_Sw_DataParams_t *) pDataParams, wOption, pPlainBuffer, wBufferLength,
                pEncryptedBuffer);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pEncryptedBuffer), pEncryptedBuffer, wBufferLength);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_Decrypt(void * pDataParams, uint16_t wOption, const uint8_t * pEncryptedBuffer, uint16_t wBufferLength,
    uint8_t * pPlainBuffer)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_Decrypt");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pEncryptedBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wBufferLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pPlainBuffer);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pEncryptedBuffer, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pPlainBuffer, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wOption), &wOption);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wBufferLength), &wBufferLength);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pEncryptedBuffer), pEncryptedBuffer, wBufferLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_Decrypt((phCryptoSym_Sw_DataParams_t *) pDataParams, wOption, pEncryptedBuffer, wBufferLength, pPlainBuffer);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pPlainBuffer), pPlainBuffer, wBufferLength);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}


phStatus_t phCryptoSym_CalculateMac(void * pDataParams, uint16_t wOption, const uint8_t* pData, uint16_t wDataLength,
    uint8_t * pMac, uint8_t * pMacLength)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_CalculateMac");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pData);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wDataLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pMac);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pMacLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pData, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pMac, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pMacLength, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wOption), &wOption);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pData), pData, wDataLength);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wDataLength), &wDataLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_CalculateMac((phCryptoSym_Sw_DataParams_t *) pDataParams, wOption, pData, wDataLength,
                pMac, pMacLength);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pMac), pMac, *pMacLength);
        PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pMacLength), pMacLength);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_LoadIv(void * pDataParams, const uint8_t * pIV, uint8_t bIVLength)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_LoadIv");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pIV);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bIVLength);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pIV, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pIV), pIV, bIVLength);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bIVLength), &bIVLength);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_LoadIv((phCryptoSym_Sw_DataParams_t *) pDataParams, pIV, bIVLength);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_LoadKey(void * pDataParams, uint16_t wKeyNo, uint16_t wKeyVersion, uint16_t wKeyType)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_LoadKey");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyVersion);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyVersion), &wKeyVersion);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyType), &wKeyType);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Check data parameters */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_LoadKey((phCryptoSym_Sw_DataParams_t *) pDataParams, wKeyNo, wKeyVersion, wKeyType);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_LoadKeyDirect(void * pDataParams, const uint8_t * pKey, uint16_t wKeyType)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_LoadKeyDirect");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pKey);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pKey, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pKey), pKey, phCryptoSym_GetKeySize(wKeyType));
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyType), &wKeyType);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_LoadKeyDirect((phCryptoSym_Sw_DataParams_t *) pDataParams, pKey, wKeyType);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_DiversifyKey(void * pDataParams, uint16_t wOption, uint16_t wKeyNo, uint16_t wKeyVersion,
    uint8_t * pDivInput, uint8_t bLenDivInput, uint8_t * pDiversifiedKey)
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bDivKeyLen = 0;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_DiversifyKey");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyNo);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyVersion);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDivInput);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bLenDivInput);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDiversifiedKey);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    if(bLenDivInput) PH_ASSERT_NULL_PARAM(pDivInput, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pDiversifiedKey, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wOption), &wOption);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyNo), &wKeyNo);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyVersion), &wKeyVersion);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pDivInput), pDivInput, bLenDivInput);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bLenDivInput), &bLenDivInput);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Check data parameters */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* perform operation on active layer */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_DiversifyKey((phCryptoSym_Sw_DataParams_t *) pDataParams, wOption, wKeyNo, wKeyVersion, pDivInput,
                bLenDivInput, pDiversifiedKey, &bDivKeyLen);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pDiversifiedKey), pDiversifiedKey, bDivKeyLen);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_DiversifyDirectKey(void * pDataParams, uint16_t wOption, uint8_t * pKey, uint16_t wKeyType,
    uint8_t * pDivInput, uint8_t bLenDivInput, uint8_t * pDiversifiedKey)
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bDivKeyLen = 0;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_DiversifyDirectKey");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wOption);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pKey);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wKeyType);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDivInput);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(bLenDivInput);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pDiversifiedKey);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pKey, PH_COMP_CRYPTOSYM);
    if(bLenDivInput) PH_ASSERT_NULL_PARAM(pDivInput, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pDiversifiedKey, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wOption), &wOption);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pKey), pKey, phCryptoSym_GetKeySize(wKeyType));
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wKeyType), &wKeyType);
    PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pDivInput), pDivInput, bLenDivInput);
    PH_LOG_HELPER_ADDPARAM_UINT8(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(bLenDivInput), &bLenDivInput);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_DiversifyDirectKey((phCryptoSym_Sw_DataParams_t *) pDataParams, wOption, pKey, wKeyType, pDivInput,
                bLenDivInput, pDiversifiedKey, &bDivKeyLen);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
#ifdef NXPBUILD__PH_LOG
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        PH_LOG_HELPER_ADDPARAM_BUFFER(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(pDiversifiedKey), pDiversifiedKey, bDivKeyLen);
    }
#endif
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_ApplyPadding(uint8_t bOption, const uint8_t * pDataIn, uint16_t wDataInLength, uint8_t  bBlockSize,
    uint16_t wDataOutBufSize, uint8_t * pDataOut, uint16_t * pDataOutLength)
{
    uint8_t bPaddingLength = 0;
    *pDataOutLength = 0;

    /* Validate the parameters. */
    if(wDataInLength) PH_ASSERT_NULL_PARAM(pDataIn, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pDataOut, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pDataOutLength, PH_COMP_CRYPTOSYM);

    /* How many bytes we have to pad? */
    bPaddingLength = (uint8_t) (wDataInLength % bBlockSize);
    if((bPaddingLength == 0x00) && (bOption != PH_CRYPTOSYM_PADDING_MODE_1))
    {
        bPaddingLength = (uint8_t) bBlockSize;
    }
    else
    {
        bPaddingLength = (uint8_t) (bBlockSize - bPaddingLength);
    }

    /* Do we have sufficient space in the output buffer? */
    if(wDataOutBufSize < (wDataInLength + bPaddingLength))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_CRYPTOSYM);
    }

    /* Recopy the array in case pDataIn != pDataOut */
    if(pDataIn != pDataOut)
    {
        memcpy(pDataOut, pDataIn, wDataInLength); /* PRQA S 3200 */
    }

    /* Add the padding constant */
    switch(bOption)
    {
        case PH_CRYPTOSYM_PADDING_MODE_1:
            pDataOut[wDataInLength++] = 0x00;
            break;

        case PH_CRYPTOSYM_PADDING_MODE_2:
            pDataOut[wDataInLength++] = 0x80;
            break;

        default:
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    bPaddingLength--;
    while(bPaddingLength--)
    {
        /* Add zeros */
        pDataOut[wDataInLength++] = 0x00;
    }

    *pDataOutLength = wDataInLength;
    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_RemovePadding(uint8_t bOption, const uint8_t * pDataIn, uint16_t wDataInLength, uint8_t  bBlockSize,
    uint16_t wDataOutBufSize, uint8_t * pDataOut, uint16_t * pDataOutLength)
{
    uint8_t bFinished = 0;
    *pDataOutLength = 0;

    /* Validate the parameters. */
    if(wDataInLength) PH_ASSERT_NULL_PARAM(pDataIn, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pDataOut, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pDataOutLength, PH_COMP_CRYPTOSYM);

    /* Is the data in length a multiple of the blocksize?? */
    if(wDataInLength % bBlockSize)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
    }

    /* find the padding constant */
    while((wDataInLength > 0) && (bFinished == 0))
    {
        wDataInLength--;
        switch(bOption)
        {
            case PH_CRYPTOSYM_PADDING_MODE_1:
                if(pDataIn[wDataInLength] != 0x00)
                {
                    bFinished = 1;
                    wDataInLength++;
                    break;
                }
                else
                {
                    /* Continue processing */
                }
                break;

            case PH_CRYPTOSYM_PADDING_MODE_2:
                if(pDataIn[wDataInLength] == 0x80)
                {
                    bFinished = 1;
                    break;
                }
                else if(pDataIn[wDataInLength] != 0x00)
                {
                    return PH_ADD_COMPCODE_FIXED(PH_ERR_FRAMING_ERROR, PH_COMP_CRYPTOSYM);
                }
                else
                {
                    /* Continue processing */
                }
                break;

            default:
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_CRYPTOSYM);
        }
    }

    /* In case of it is not the last byte, we can set the correct new data out length, otherwise we have a out data length of 0. */
    *pDataOutLength = wDataInLength;

    /* do we have a buffer overflow? */
    if(wDataOutBufSize < *pDataOutLength)
    {
        *pDataOutLength = 0;
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_CRYPTOSYM);
    }

    /* Recopy the array in case pDataIn != pDataOut */
    if(pDataIn != pDataOut)
    {
        memcpy(pDataOut, pDataIn, *pDataOutLength); /* PRQA S 3200 */
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phCryptoSym_SetConfig(void * pDataParams, uint16_t wConfig, uint16_t wValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_SetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wConfig), &wConfig);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wValue), &wValue);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_SetConfig((phCryptoSym_Sw_DataParams_t *) pDataParams, wConfig, wValue);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
            break;
    }

    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

    return status;
}

phStatus_t phCryptoSym_GetConfig(void * pDataParams, uint16_t wConfig, uint16_t * pValue)
{
    phStatus_t PH_MEMLOC_REM status;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_GetConfig");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wConfig);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pValue);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(status);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pValue, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wConfig), &wConfig);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(status), &status);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return status;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {
#ifdef NXPBUILD__PH_CRYPTOSYM_SW
        case PH_CRYPTOSYM_SW_ID:
            status = phCryptoSym_Sw_GetConfig((phCryptoSym_Sw_DataParams_t *) pDataParams, wConfig, pValue);
            break;
#endif /* NXPBUILD__PH_CRYPTOSYM_SW */



        default:
            status = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
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

phStatus_t phCryptoSym_GetLastStatus(void * pDataParams, uint16_t wStatusMsgLen, int8_t * pStatusMsg, int32_t * pStatusCode)
{
    phStatus_t PH_MEMLOC_REM wStatus = 0;

    PH_LOG_HELPER_ALLOCATE_TEXT(bFunctionName, "phCryptoSym_GetLastStatus");
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wStatusMsgLen);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pStatusMsg);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(pStatusCode);
    PH_LOG_HELPER_ALLOCATE_PARAMNAME(wStatus);

    /* Validate the parameters. */
    PH_ASSERT_NULL_DATA_PARAM(pDataParams, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pStatusMsg, PH_COMP_CRYPTOSYM);
    PH_ASSERT_NULL_PARAM(pStatusCode, PH_COMP_CRYPTOSYM);

    /* Log the information. */
    PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
    PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_DEBUG, PH_LOG_VAR(wStatusMsgLen), &wStatusMsgLen);
    PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_ENTER);

    /* Component Code Validation */
    if(PH_GET_COMPCODE(pDataParams) != PH_COMP_CRYPTOSYM)
    {
        wStatus = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);

        PH_LOG_HELPER_ADDSTRING(PH_LOG_LOGTYPE_INFO, bFunctionName);
        PH_LOG_HELPER_ADDPARAM_UINT16(PH_LOG_LOGTYPE_INFO, PH_LOG_VAR(wStatus), &wStatus);
        PH_LOG_HELPER_EXECUTE(PH_LOG_OPTION_CATEGORY_LEAVE);

        return wStatus;
    }

    /* Perform operation on active layer. */
    switch(PH_GET_COMPID(pDataParams))
    {

        default:
            wStatus = PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_CRYPTOSYM);
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

uint16_t phCryptoSym_GetKeySize(uint16_t wKeyType)
{
    switch(wKeyType)
    {
        case PH_CRYPTOSYM_KEY_TYPE_DES:
            return 8;

        case PH_CRYPTOSYM_KEY_TYPE_2K3DES:
        case PH_CRYPTOSYM_KEY_TYPE_AES128:
            return 16;

        case PH_CRYPTOSYM_KEY_TYPE_3K3DES:
        case PH_CRYPTOSYM_KEY_TYPE_AES192:
            return 24;

        case PH_CRYPTOSYM_KEY_TYPE_AES256:
            return 32;

        default:
            return 0;
    }
}

#endif /* NXPBUILD__PH_CRYPTOSYM */
