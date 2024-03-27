/*----------------------------------------------------------------------------*/
/* Copyright 2013-2022 NXP                                                    */
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
* Software Tag Operation Application Layer Component of
* Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/

#include <ph_TypeDefs.h>
#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phNxpNfcRdLib_Config.h>

#include <phalT1T.h>
#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW

#include "phalTop_Sw.h"
#include "phalTop_Sw_Int_T1T.h"
#include "phalTop_Sw_Int_T2T.h"
#include "phalTop_Sw_Int_T3T.h"
#include "phalTop_Sw_Int_T4T.h"
#include "phalTop_Sw_Int_T5T.h"
#include "phalTop_Sw_Int_MfcTop.h"

static const pphalTop_Sw_CheckNdef pfphalTop_Sw_CheckNdef[PHAL_TOP_MAX_TAGTYPE_SUPPORTED] = {
#if defined(NXPBUILD__PHAL_TOP_T1T_SW)
    &phalTop_Sw_Int_T1T_CheckNdef,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T1T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */

#if defined(NXPBUILD__PHAL_TOP_T2T_SW)
    &phalTop_Sw_Int_T2T_CheckNdef,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T2T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */

#if defined(NXPBUILD__PHAL_TOP_T3T_SW)
    &phalTop_Sw_Int_T3T_CheckNdef,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T3T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */

#if defined(NXPBUILD__PHAL_TOP_T4T_SW)
    &phalTop_Sw_Int_T4T_CheckNdef,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T4T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */

#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    &phalTop_Sw_Int_T5T_CheckNdef,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T5T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */

#if defined(NXPBUILD__PHAL_TOP_MFC_SW)
    &phalTop_Sw_Int_MfcTop_CheckNdef
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
#if ! defined(NXPBUILD__PHAL_TOP_MFC_SW)
    NULL
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
};

static const pphalTop_Sw_FormatNdef pfphalTop_Sw_FormatNdef[PHAL_TOP_MAX_TAGTYPE_SUPPORTED] = {
#if defined(NXPBUILD__PHAL_TOP_T1T_SW)
    &phalTop_Sw_Int_T1T_FormatNdef,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T1T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */

#if defined(NXPBUILD__PHAL_TOP_T2T_SW)
    &phalTop_Sw_Int_T2T_FormatNdef,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T2T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */

#if defined(NXPBUILD__PHAL_TOP_T3T_SW)
    &phalTop_Sw_Int_T3T_FormatNdef,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T3T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */

#if defined(NXPBUILD__PHAL_TOP_T4T_SW)
    &phalTop_Sw_Int_T4T_FormatNdef,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T4T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */

#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    &phalTop_Sw_Int_T5T_FormatNdef,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T5T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */

#if defined(NXPBUILD__PHAL_TOP_MFC_SW)
    &phalTop_Sw_Int_MfcTop_FormatNdef
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
#if ! defined(NXPBUILD__PHAL_TOP_MFC_SW)
    NULL
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
};

static const pphalTop_Sw_EraseNdef pfphalTop_Sw_EraseNdef[PHAL_TOP_MAX_TAGTYPE_SUPPORTED] = {
#if defined(NXPBUILD__PHAL_TOP_T1T_SW)
    &phalTop_Sw_Int_T1T_EraseNdef,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T1T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */

#if defined(NXPBUILD__PHAL_TOP_T2T_SW)
    &phalTop_Sw_Int_T2T_EraseNdef,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T2T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */

#if defined(NXPBUILD__PHAL_TOP_T3T_SW)
    &phalTop_Sw_Int_T3T_EraseNdef,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T3T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */

#if defined(NXPBUILD__PHAL_TOP_T4T_SW)
    &phalTop_Sw_Int_T4T_EraseNdef,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T4T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */

#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    &phalTop_Sw_Int_T5T_EraseNdef,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T5T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */

#if defined(NXPBUILD__PHAL_TOP_MFC_SW)
    &phalTop_Sw_Int_MfcTop_EraseNdef
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
#if ! defined(NXPBUILD__PHAL_TOP_MFC_SW)
    NULL
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
};

static const pphalTop_Sw_ReadNdef pfphalTop_Sw_ReadNdef[PHAL_TOP_MAX_TAGTYPE_SUPPORTED] = {
#if defined(NXPBUILD__PHAL_TOP_T1T_SW)
    &phalTop_Sw_Int_T1T_ReadNdef,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T1T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */

#if defined(NXPBUILD__PHAL_TOP_T2T_SW)
    &phalTop_Sw_Int_T2T_ReadNdef,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T2T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */

#if defined(NXPBUILD__PHAL_TOP_T3T_SW)
    &phalTop_Sw_Int_T3T_ReadNdef,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T3T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */

#if defined(NXPBUILD__PHAL_TOP_T4T_SW)
    &phalTop_Sw_Int_T4T_ReadNdef,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T4T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */

#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    &phalTop_Sw_Int_T5T_ReadNdef,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T5T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */

#if defined(NXPBUILD__PHAL_TOP_MFC_SW)
    &phalTop_Sw_Int_MfcTop_ReadNdef
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
#if ! defined(NXPBUILD__PHAL_TOP_MFC_SW)
    NULL
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
};

static const pphalTop_Sw_WriteNdef pfphalTop_Sw_WriteNdef[PHAL_TOP_MAX_TAGTYPE_SUPPORTED] = {
#if defined(NXPBUILD__PHAL_TOP_T1T_SW)
    &phalTop_Sw_Int_T1T_WriteNdef,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T1T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */

#if defined(NXPBUILD__PHAL_TOP_T2T_SW)
    &phalTop_Sw_Int_T2T_WriteNdef,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T2T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */

#if defined(NXPBUILD__PHAL_TOP_T3T_SW)
    &phalTop_Sw_Int_T3T_WriteNdef,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T3T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */

#if defined(NXPBUILD__PHAL_TOP_T4T_SW)
    &phalTop_Sw_Int_T4T_WriteNdef,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T4T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */

#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    &phalTop_Sw_Int_T5T_WriteNdef,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T5T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */

#if defined(NXPBUILD__PHAL_TOP_MFC_SW)
    &phalTop_Sw_Int_MfcTop_WriteNdef
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
#if ! defined(NXPBUILD__PHAL_TOP_MFC_SW)
    NULL
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
};

static const pphalTop_Sw_Int_SetReadOnly pfphalTop_Sw_Int_SetReadOnly[PHAL_TOP_MAX_TAGTYPE_SUPPORTED]={
#if defined(NXPBUILD__PHAL_TOP_T1T_SW)
    &phalTop_Sw_Int_T1T_SetReadOnly,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T1T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */

#if defined(NXPBUILD__PHAL_TOP_T2T_SW)
    &phalTop_Sw_Int_T2T_SetReadOnly,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T2T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */

#if defined(NXPBUILD__PHAL_TOP_T3T_SW)
    &phalTop_Sw_Int_T3T_SetReadOnly,
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T3T_SW)
    NULL,
#endif

    NULL,
#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    &phalTop_Sw_Int_T5T_SetReadOnly,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T5T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
    NULL
};

static const pphalTop_Sw_LockBlock pfphalTop_Sw_LockBlock[PHAL_TOP_MAX_TAGTYPE_SUPPORTED] = {
    NULL,
    NULL,
    NULL,
    NULL,
#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    &phalTop_Sw_Int_LockBlock,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#if ! defined(NXPBUILD__PHAL_TOP_T5T_SW)
    NULL,
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
    NULL
};


static phStatus_t phalTop_Sw_ValidateTagType(phalTop_Sw_DataParams_t * pDataParams)
{
    /* Validate tag type */
    if((pDataParams->bTagType < PHAL_TOP_TAG_TYPE_T1T_TAG) ||
       (pDataParams->bTagType > PHAL_TOP_TAG_TYPE_MFC_TOP))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Init(
                           phalTop_Sw_DataParams_t * pDataParams,
                           uint16_t wSizeOfDataParams,
                           void * pAlT1T,
                           void * pAlMful,
                           void * pAlFelica,
                           void * pPalMifareDataParams,
                           void * pAl15693,
                           void * pPalI14443paDataParams
                           )
{
    if (sizeof(phalTop_Sw_DataParams_t) != wSizeOfDataParams)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_DATA_PARAMS, PH_COMP_AL_TOP);
    }

    PH_ASSERT_NULL (pDataParams);

    pDataParams->wId = PH_COMP_AL_TOP | PHAL_TOP_SW_ID;
    pDataParams->pTopTagsDataParams[0] = pAlT1T;
    pDataParams->pTopTagsDataParams[1] = pAlMful;
    pDataParams->pTopTagsDataParams[2] = pAlFelica;
    pDataParams->pTopTagsDataParams[3] = pPalMifareDataParams;
    pDataParams->pTopTagsDataParams[4] = pAl15693;
    pDataParams->pTopTagsDataParams[5] = pPalI14443paDataParams;

    return phalTop_Sw_Reset(pDataParams);
}

phStatus_t phalTop_Sw_CheckNdef(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint8_t * pTagState
                                )
{
    phStatus_t    PH_MEMLOC_REM status;

    /* Validate tag type */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_ValidateTagType(pDataParams));

    /* Check if the tag is enabled in build */
    if(pfphalTop_Sw_CheckNdef[pDataParams->bTagType - 1U] != NULL)
    {
        return pfphalTop_Sw_CheckNdef[pDataParams->bTagType - 1U](
            pDataParams,
            pTagState);
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }
}

phStatus_t phalTop_Sw_ReadNdef(
                               phalTop_Sw_DataParams_t * pDataParams,
                               uint8_t * pData,
                               uint32_t * pLength
                               )
{
    phStatus_t    PH_MEMLOC_REM status;

    /* Validate tag type */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_ValidateTagType(pDataParams));

    /* Check if the tag is enabled in build */
    if(pfphalTop_Sw_ReadNdef[pDataParams->bTagType - 1U] != NULL)
    {
        return pfphalTop_Sw_ReadNdef[pDataParams->bTagType - 1U](
            pDataParams,
            pData,
            pLength);
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }
}

phStatus_t phalTop_Sw_WriteNdef(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint8_t * pData,
                                uint32_t dwLength
                                )
{
    phStatus_t    PH_MEMLOC_REM status;

    /* Validate tag type */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_ValidateTagType(pDataParams));

    /* Check if the tag is enabled in build */
    if(pfphalTop_Sw_WriteNdef[pDataParams->bTagType - 1U] != NULL)
    {
        return pfphalTop_Sw_WriteNdef[pDataParams->bTagType - 1U](
            pDataParams,
            pData,
            dwLength);
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }
}

phStatus_t phalTop_Sw_EraseNdef(
                                phalTop_Sw_DataParams_t * pDataParams
                                )
{
    phStatus_t    PH_MEMLOC_REM status;

    /* Validate tag type */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_ValidateTagType(pDataParams));

    /* Check if the tag is enabled in build */
    if(pfphalTop_Sw_EraseNdef[pDataParams->bTagType - 1U] != NULL)
    {
        return pfphalTop_Sw_EraseNdef[pDataParams->bTagType - 1U](pDataParams);
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }
}

phStatus_t phalTop_Sw_FormatNdef(
                                 phalTop_Sw_DataParams_t * pDataParams
                                 )
{
    phStatus_t    PH_MEMLOC_REM status;

    /* Validate tag type */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_ValidateTagType(pDataParams));

    /* Check if the tag is enabled in build */
    if(pfphalTop_Sw_FormatNdef[pDataParams->bTagType - 1U] != NULL)
    {
        return pfphalTop_Sw_FormatNdef[pDataParams->bTagType - 1U](pDataParams);
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }
}

phStatus_t phalTop_Sw_Reset(
                            phalTop_Sw_DataParams_t * pDataParams
                            )
{

    /* Reset common variables */
    pDataParams->bTagState = 0;
    pDataParams->bTagType = 0;
    pDataParams->bVno = 0;
    pDataParams->dwNdefLength = 0;
    pDataParams->dwMaxNdefLength = 0;

    (void)memset(&pDataParams->ualTop, 0x00, (size_t)(sizeof(pDataParams->ualTop)));

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_SetConfig(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint16_t wConfig,
                                uint32_t dwValue
                                )
{
    switch(wConfig)
    {
    case PHAL_TOP_CONFIG_TAG_TYPE:
        if((dwValue < PHAL_TOP_TAG_TYPE_T1T_TAG) || (dwValue > PHAL_TOP_TAG_TYPE_MFC_TOP))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        pDataParams->bTagType = (uint8_t)dwValue;
        break;

    case PHAL_TOP_CONFIG_TAG_STATE:
        /* This shall be used only to set tag to read-only state. */
        if(dwValue != PHAL_TOP_STATE_READONLY)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        /* Set to read only is supported only by T1T, T2T, T3T and T5T */
        if((pDataParams->bTagType != PHAL_TOP_TAG_TYPE_T1T_TAG) &&
           (pDataParams->bTagType != PHAL_TOP_TAG_TYPE_T2T_TAG) &&
	       (pDataParams->bTagType != PHAL_TOP_TAG_TYPE_T3T_TAG) &&
           (pDataParams->bTagType != PHAL_TOP_TAG_TYPE_T5T_TAG))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        /* Check if the tag is enabled in build */
        if(pfphalTop_Sw_Int_SetReadOnly[pDataParams->bTagType - 1U] != NULL)
        {
            return pfphalTop_Sw_Int_SetReadOnly[pDataParams->bTagType - 1U](
                pDataParams);
        }
        else
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }
        break;

    case PHAL_TOP_CONFIG_NDEF_VERSION:
        /* Allow only applicable tags */
        if(pDataParams->bTagType != PHAL_TOP_TAG_TYPE_T4T_TAG)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        if((!((dwValue == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20) || (dwValue == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO))) &&
            (pDataParams->bTagType == PHAL_TOP_TAG_TYPE_T4T_TAG))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        pDataParams->bVno = (uint8_t)dwValue;
        break;

#if defined(NXPBUILD__PHAL_TOP_T1T_SW)
    case PHAL_TOP_CONFIG_T1T_TMS:
        if((dwValue > 0x800U) || (dwValue < 0x0EU))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T1T_t *)(&pDataParams->ualTop.salTop_T1T))->bTms = (uint8_t)((dwValue / 8U) - 1U);
        break;

    case PHAL_TOP_CONFIG_T1T_TERMINATOR_TLV:
        if((dwValue != PH_ON) && (dwValue != PH_OFF))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T1T_t *)(&pDataParams->ualTop.salTop_T1T))->bTerminatorTlvPresence = (uint8_t)(dwValue);
        break;
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */

#if defined(NXPBUILD__PHAL_TOP_T2T_SW)
    case PHAL_TOP_CONFIG_T2T_TMS:
        if(dwValue < 0x30U)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T2T_t *)(&pDataParams->ualTop.salTop_T2T))->bTms = (uint8_t)(dwValue / 8U);
        break;
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */

#if defined(NXPBUILD__PHAL_TOP_T4T_SW)
    case PHAL_TOP_CONFIG_T4T_NDEF_FILE_ID:
        if((dwValue == 0x0000U) || (dwValue == 0x3F00U) || (dwValue == 0x3FFFU) ||
           (dwValue == 0xE102U) || (dwValue == 0xE103U) || (dwValue == 0xFFFFU))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T4T_t *)(&pDataParams->ualTop.salTop_T4T))->aNdefFileID[0] = (uint8_t)(dwValue & 0x00FFU);
        ((phalTop_T4T_t *)(&pDataParams->ualTop.salTop_T4T))->aNdefFileID[1] = (uint8_t)((dwValue & 0xFF00U) >> 8U);
        break;

    case PHAL_TOP_CONFIG_T4T_NDEF_FILE_SIZE:
        if((((dwValue < PHAL_TOP_T4T_NDEF_FILE_SIZE_MIN_MV20) || (dwValue > PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV20))
                && (pDataParams->bVno == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO_20)) ||
            (((dwValue < PHAL_TOP_T4T_NDEF_FILE_SIZE_MIN_MV30) || (dwValue > PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV30))
                && (pDataParams->bVno == PHAL_TOP_T4T_NDEF_SUPPORTED_VNO)))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T4T_t *)(&pDataParams->ualTop.salTop_T4T))->dwMaxFileSize = dwValue;
        break;

    case PHAL_TOP_CONFIG_T4T_MLE:
        if(dwValue < PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_MIN)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T4T_t *)(&pDataParams->ualTop.salTop_T4T))->wMLe = dwValue;
        break;

    case PHAL_TOP_CONFIG_T4T_MLC:
        if(dwValue < PHAL_TOP_T4T_MAX_CMD_LENGTH_MIN)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T4T_t *)(&pDataParams->ualTop.salTop_T4T))->wMLc = dwValue;
        break;

    case PHAL_TOP_CONFIG_T4T_SHORT_APDU:
        if((dwValue != PH_ON) && (dwValue != PH_OFF))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T4T_t *)(&pDataParams->ualTop.salTop_T4T))->bShortLenApdu = (uint8_t)(dwValue);
        break;

#endif /* NXPBUILD__PHAL_TOP_T4T_SW */

#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    case PHAL_TOP_CONFIG_T5T_MLEN:
        if(dwValue < 24U)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T5T_t *)(&pDataParams->ualTop.salTop_T5T))->wMlen = (dwValue / 8U);
        break;

    case PHAL_TOP_CONFIG_T5T_MBREAD:
        if((dwValue != PH_ON) && (dwValue != PH_OFF))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T5T_t *)(&pDataParams->ualTop.salTop_T5T))->bMbRead = (uint8_t)(dwValue);
        break;

    case PHAL_TOP_CONFIG_T5T_LOCKBLOCK:
        if((dwValue != PH_ON) && (dwValue != PH_OFF))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T5T_t *)(&pDataParams->ualTop.salTop_T5T))->bLockBlock = (uint8_t)(dwValue);
        break;

    case PHAL_TOP_CONFIG_T5T_SPL_FRM:
        if((dwValue != PH_ON) && (dwValue != PH_OFF))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T5T_t *)(&pDataParams->ualTop.salTop_T5T))->bSplFrm = (uint8_t)(dwValue);
        break;

    case PHAL_TOP_CONFIG_T5T_OPTION_FLAG:
        if((dwValue != PH_ON) && (dwValue != PH_OFF))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T5T_t *)(&pDataParams->ualTop.salTop_T5T))->bOptionFlag = (uint8_t)(dwValue);
        break;

    case PHAL_TOP_CONFIG_T5T_TERMINATOR_TLV:
        if((dwValue != PH_ON) && (dwValue != PH_OFF))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_T5T_t *)(&pDataParams->ualTop.salTop_T5T))->bTerminatorTlvPresence = (uint8_t)(dwValue);
        break;

    case PHAL_TOP_CONFIG_T5T_MAX_READ_LENGTH:
        if((dwValue < 255U) || ((dwValue > 255U) && (dwValue <= 1024U)))
        {
            ((phalTop_T5T_t *)(&pDataParams->ualTop.salTop_T5T))->wMaxReadLength = dwValue;
        }
        else
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        break;
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */

#if defined (NXPBUILD__PHAL_TOP_MFC_SW)

    case PHAL_TOP_CONFIG_MFCTOP_CARD_TYPE:
        if((dwValue < PHAL_MFCTOP_MFC_1K) || (dwValue > PHAL_MFCTOP_MFP_4K))
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
        }

        ((phalTop_MfcTop_t *)(&pDataParams->ualTop.salTop_MfcTop))->bCardType = (uint8_t)(dwValue);
        break;

#endif  /* NXPBUILD__PHAL_TOP_MFC_SW */

    default:
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_GetConfig(
                                phalTop_Sw_DataParams_t * pDataParams,
                                uint16_t wConfig,
                                uint32_t  * dwValue
                                )
{
    switch(wConfig)
    {
    case PHAL_TOP_CONFIG_TAG_TYPE:
        *dwValue = pDataParams->bTagType;
        break;

    case PHAL_TOP_CONFIG_TAG_STATE:
        *dwValue = pDataParams->bTagState;
        break;

    case PHAL_TOP_CONFIG_NDEF_LENGTH:
        *dwValue = pDataParams->dwNdefLength;
        break;

    case PHAL_TOP_CONFIG_MAX_NDEF_LENGTH:
        *dwValue = pDataParams->dwMaxNdefLength;
        break;

    case PHAL_TOP_CONFIG_NDEF_VERSION:
        *dwValue = pDataParams->bVno;
        break;

#if defined(NXPBUILD__PHAL_TOP_T4T_SW)
    case PHAL_TOP_CONFIG_T4T_MLE:
        *dwValue = (uint32_t)(((phalTop_T4T_t *)(&pDataParams->ualTop.salTop_T4T))->wMLe);
        break;

    case PHAL_TOP_CONFIG_T4T_MLC:
        *dwValue = (uint32_t)(((phalTop_T4T_t *)(&pDataParams->ualTop.salTop_T4T))->wMLc);
        break;
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */

    default:
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_LockBlock(
                                 phalTop_Sw_DataParams_t * pDataParams,
                                 uint16_t wBlockNum
                                 )
{
    /* Validate tag type */
    if(pDataParams->bTagType != PHAL_TOP_TAG_TYPE_T5T_TAG)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AL_TOP);
    }

    /* Check if the tag is enabled in build */
    if(pfphalTop_Sw_LockBlock[pDataParams->bTagType - 1U] != NULL)
    {
        return pfphalTop_Sw_LockBlock[pDataParams->bTagType - 1U](
                pDataParams,
                wBlockNum);
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }
}


#if defined(__DEBUG)
/* This API would be available only for debugging purpose */
phStatus_t phalTop_Sw_SetPtr(
                phalTop_Sw_DataParams_t * pDataParams,
                void * pT1T,
                void * pT2T,
                void * pT3T,
                void * pT4T,
                void * pT5T,
                void * pMfcTop
)
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_SUCCESS;;

#if defined(NXPBUILD__PHAL_TOP_T1T_SW)
    pDataParams->pTopTagsDataParams[0] = pT1T;
#endif /* NXPBUILD__PHAL_TOP_T1T_SW */
#if defined(NXPBUILD__PHAL_TOP_T2T_SW)
    pDataParams->pTopTagsDataParams[1] = pT2T;
#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#if defined(NXPBUILD__PHAL_TOP_T3T_SW)
    pDataParams->pTopTagsDataParams[2] = pT3T;
#endif /* NXPBUILD__PHAL_TOP_T3T_SW */
#if defined(NXPBUILD__PHAL_TOP_T4T_SW)
    pDataParams->pTopTagsDataParams[3] = pT4T;
#endif /* NXPBUILD__PHAL_TOP_T4T_SW */
#if defined(NXPBUILD__PHAL_TOP_T5T_SW)
    pDataParams->pTopTagsDataParams[4] = pT5T;
#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#if defined(NXPBUILD__PHAL_TOP_MFC_SW)
    pDataParams->pTopTagsDataParams[5] = pMfcTop;
#endif /* NXPBUILD__PHAL_TOP_MFC_SW */
    return status;
}
#endif /* __DEBUG */

#endif /* NXPBUILD__PHAL_TOP_SW */
