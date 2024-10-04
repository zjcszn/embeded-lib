/*----------------------------------------------------------------------------*/
/* Copyright 2016-2023 NXP                                                    */
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
* This File contains all support functions that's needed by Discovery Loop
* for performing activities defined by NFC Forum
* as part of the Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */
#include <ph_RefDefs.h>

#include <phacDiscLoop.h>
#include <phpalI14443p3a.h>
#include <phpalI14443p3b.h>
#include <phpalI18092mPI.h>

#include "phacDiscLoop_Sw_Int_A.h"
#include "phacDiscLoop_Sw_Int_B.h"
#include "phacDiscLoop_Sw_Int_F.h"
#include "phacDiscLoop_Sw_Int_V.h"
#include "phacDiscLoop_Sw_Int_I18000p3m3.h"

#ifdef NXPBUILD__PHAC_DISCLOOP_SW
#include "phacDiscLoop_Sw_Int.h"

/*********************************************************************************/
/** MACRO DEFINATIONS                                                            */
/*********************************************************************************/
/* HaltA timeout in micro seconds. */
#define PHAC_DISCLOOP_HALTA_TIMOUT_US     1100U

/*********************************************************************************/
/** LOCALIZED GLOBAL VARIABLES                                                   */
/*********************************************************************************/
static const uint32_t gPasTechTypeMapTable[] = {
    PHHAL_HW_CARDTYPE_ISO14443A,
    PHHAL_HW_CARDTYPE_ISO14443B,
    PHHAL_HW_CARDTYPE_FELICA_212,
    PHHAL_HW_CARDTYPE_FELICA_424,
    PHHAL_HW_CARDTYPE_ISO15693,
    PHHAL_HW_CARDTYPE_I18000P3M3,
};

static const pphacDiscLoop_Sw_Reslns pfColnRelsns[] = {
    &phacDiscLoop_Sw_Int_CollisionResolutionA,
    &phacDiscLoop_Sw_Int_CollisionResolutionB,
    &phacDiscLoop_Sw_Int_CollisionResolutionF,
    &phacDiscLoop_Sw_Int_CollisionResolutionF,
    &phacDiscLoop_Sw_Int_CollisionResolutionV,
    &phacDiscLoop_Sw_Int_CollisionResolutionI18000p3m3,
};

static const pphacDiscLoop_Sw_DetTechs pfDetTechs[] = {
    &phacDiscLoop_Sw_DetTechTypeA,
    &phacDiscLoop_Sw_DetTechTypeB,
    &phacDiscLoop_Sw_DetTechTypeF,
    &phacDiscLoop_Sw_DetTechTypeF,
    &phacDiscLoop_Sw_DetTechTypeV,
    &phacDiscLoop_Sw_DetTechTypeI18000p3m3,
};

static const pphacDiscLoop_Sw_DeviceActivate pfDeviceActivate[] = {
    &phacDiscLoop_Sw_Int_ActivateA,
    &phacDiscLoop_Sw_Int_ActivateB,
    &phacDiscLoop_Sw_Int_ActivateF,
    &phacDiscLoop_Sw_Int_ActivateF,
    &phacDiscLoop_Sw_Int_ActivateV,
    &phacDiscLoop_Sw_Int_ActivateI18000p3m3,
};

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */

static phStatus_t phacDiscLoop_Sw_Int_Apply_PTGT(
        phacDiscLoop_Sw_DataParams_t *pDataParams
        )
{
    phStatus_t PH_MEMLOC_REM   status = PH_ERR_SUCCESS;
    uint8_t    PH_MEMLOC_COUNT bTechLoopIndex;
    uint8_t    PH_MEMLOC_COUNT bPollTech;
    uint8_t    PH_MEMLOC_COUNT bTechIndex;
    uint16_t   PH_MEMLOC_COUNT wPTGT = 0U;

    /* Check for Proprietary technology and apply Proprietary Technology Guard Time(PTGT) based on preceding technology */
    for (bTechLoopIndex = 1U; bTechLoopIndex < PHAC_DISCLOOP_PASS_POLL_MAX_TECHS_SUPPORTED; bTechLoopIndex++)
    {
        bPollTech = pDataParams->pPasTechPollSeq[bTechLoopIndex];

        if (bPollTech > PHAC_DISCLOOP_TECH_TYPE_V)
        {
            if(((bPollTech == PHAC_DISCLOOP_TECH_TYPE_18000P3M3) &&
                ((pDataParams->bPasPollTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_18000P3M3) != PHAC_DISCLOOP_POS_BIT_MASK_18000P3M3))
                )
            {
                /* continue to check next technology if present technology is not enabled in Polling loop */
                continue;
            }


            /* Only one technology(ISO18000p3m3) enabled */
            if((bPollTech == PHAC_DISCLOOP_TECH_TYPE_18000P3M3) &&
                (pDataParams->bPasPollTechCfg == PHAC_DISCLOOP_POS_BIT_MASK_18000P3M3))
            {
                wPTGT = PH_NXPNFCRDLIB_CONFIG_PRE_PTGT;
                PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_GT18000P3M3_VALUE_US, wPTGT));
                return status;
            }

            /* Identify preceding technology which is enabled in the Polling loop */
            bTechIndex = bTechLoopIndex;
            do
            {
                bTechIndex--;

                /* Read preceding technology */
                bPollTech = pDataParams->pPasTechPollSeq[bTechIndex];

                if(((bPollTech == PHAC_DISCLOOP_TECH_TYPE_A) &&
                        ((pDataParams->bPasPollTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_A) == PHAC_DISCLOOP_POS_BIT_MASK_A)) ||
                    ((bPollTech == PHAC_DISCLOOP_TECH_TYPE_B) &&
                        ((pDataParams->bPasPollTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_B) == PHAC_DISCLOOP_POS_BIT_MASK_B)) ||
                    ((bPollTech == PHAC_DISCLOOP_TECH_TYPE_F212) &&
                        ((pDataParams->bPasPollTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_F212) == PHAC_DISCLOOP_POS_BIT_MASK_F212)) ||
                    ((bPollTech == PHAC_DISCLOOP_TECH_TYPE_F424) &&
                        ((pDataParams->bPasPollTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_F424) == PHAC_DISCLOOP_POS_BIT_MASK_F424)) ||
                    ((bPollTech == PHAC_DISCLOOP_TECH_TYPE_V) &&
                        ((pDataParams->bPasPollTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_V) == PHAC_DISCLOOP_POS_BIT_MASK_V)) ||
                    ((bPollTech == PHAC_DISCLOOP_TECH_TYPE_18000P3M3) &&
                        ((pDataParams->bPasPollTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_18000P3M3) == PHAC_DISCLOOP_POS_BIT_MASK_18000P3M3))
                    )
                {
                    break;
                }

                if(bTechIndex == 0U)
                {
                    /* No Preceding Polling technology */
                    bPollTech = PHAC_DISCLOOP_TECH_TYPE_UNKNOWN;
                }
            }while(bTechIndex != 0U);

            switch(bPollTech)
            {
            case PHAC_DISCLOOP_TECH_TYPE_A:
                wPTGT = PH_NXPNFCRDLIB_CONFIG_PRE_PTGTA;
                break;
            case PHAC_DISCLOOP_TECH_TYPE_B:
                wPTGT = PH_NXPNFCRDLIB_CONFIG_PRE_PTGTB;
                break;
            case PHAC_DISCLOOP_TECH_TYPE_F212:
            case PHAC_DISCLOOP_TECH_TYPE_F424:
                wPTGT = PH_NXPNFCRDLIB_CONFIG_PRE_PTGTF;
                break;
            case PHAC_DISCLOOP_TECH_TYPE_V:
                wPTGT = PH_NXPNFCRDLIB_CONFIG_PRE_PTGTV;
                break;
            case PHAC_DISCLOOP_TECH_TYPE_UNKNOWN:
                wPTGT = PH_NXPNFCRDLIB_CONFIG_PRE_PTGT;
                break;
            default: /* Preceding technology is Proprietary Technology !!! */
                wPTGT = PH_NXPNFCRDLIB_CONFIG_PRE_PTGTV;
                break;
            }


            if(pDataParams->pPasTechPollSeq[bTechLoopIndex] == PHAC_DISCLOOP_TECH_TYPE_18000P3M3)
            {
                PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_GT18000P3M3_VALUE_US, wPTGT));
            }
        }
    }

    return status;
}

static phStatus_t phacDiscLoop_Sw_Int_EmvcoRfFieldReset(
                                                        phacDiscLoop_Sw_DataParams_t *pDataParams
                                                        )
{
    phStatus_t PH_MEMLOC_REM status;

    PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldOff(pDataParams->pHalDataParams));

    PH_CHECK_SUCCESS_FCT(status, phhalHw_Wait(
        pDataParams->pHalDataParams,
        PHHAL_HW_TIME_MICROSECONDS,
        PH_NXPNFCRDLIB_CONFIG_EMVCO_PROP_TECH_POLL_RF_OFF_DELAY_US)
        );

    PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldOn(pDataParams->pHalDataParams));

    return PH_ERR_SUCCESS;
}

phStatus_t phacDiscLoop_Sw_Int_ListenMode(
                                          phacDiscLoop_Sw_DataParams_t *pDataParams
                                          )
{
#ifdef NXPBUILD__PHAC_DISCLOOP_TARGET
    phStatus_t PH_MEMLOC_REM status = PH_ERR_INTERNAL_ERROR;
    uint8_t    PH_MEMLOC_REM bRetryCount = pDataParams->sTargetParams.bRetryCount;
    uint16_t   PH_MEMLOC_REM wTempMode = 0x00;

    /* get active listen cfg except F424 */
    wTempMode = pDataParams->bActLisTechCfg & 0x07U;

    /* Map Active F424 configuration with Type F bit in Autocoll */
    if(0U != (pDataParams->bActLisTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_F424))
    {
        wTempMode = wTempMode | PHAC_DISCLOOP_POS_BIT_MASK_F212;
    }

    wTempMode = (uint16_t) (wTempMode << 8U);

    /* get passive listen cfg except F424 */
    wTempMode = wTempMode | (pDataParams->bPasLisTechCfg & /* */ 0x07);

    /* Map Passice F424 configuration with Type F bit in Autocoll */
    if(0U != (pDataParams->bPasLisTechCfg & PHAC_DISCLOOP_POS_BIT_MASK_F424))
    {
        wTempMode = wTempMode | PHAC_DISCLOOP_POS_BIT_MASK_F212;
    }

    /* AUTOCOLL retry loop */
    do
    {
        status = phhalHw_Autocoll(
            pDataParams->pHalDataParams,
            wTempMode,
            &pDataParams->sTargetParams.pRxBuffer,
            &pDataParams->sTargetParams.wRxBufferLen,
            &pDataParams->sTargetParams.wProtParams);
    }while(((status & PH_ERR_MASK) != PH_ERR_SUCCESS) && (bRetryCount--));

    /* Return RF OFF error, if external RF is OFF */
    if((status & PH_ERR_MASK) == PH_ERR_EXT_RF_ERROR)
    {
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_EXTERNAL_RFOFF, PH_COMP_AC_DISCLOOP);
    }
    else
    {
        /* Return, if error */
        PH_CHECK_SUCCESS(status);
    }

    return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_ACTIVATED_BY_PEER, PH_COMP_AC_DISCLOOP);
#else /* NXPBUILD__PHAC_DISCLOOP_TARGET */
    PH_UNUSED_VARIABLE(pDataParams);
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AC_DISCLOOP);
#endif /* NXPBUILD__PHAC_DISCLOOP_TARGET */
}

phStatus_t phacDiscLoop_Sw_Int_ActivePollMode(
                                              phacDiscLoop_Sw_DataParams_t *pDataParams
                                              )
{
#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_ACTIVE)   || \
         defined (NXPBUILD__PHAC_DISCLOOP_TYPEF212_P2P_ACTIVE) || \
         defined (NXPBUILD__PHAC_DISCLOOP_TYPEF424_P2P_ACTIVE)
    phStatus_t PH_MEMLOC_REM   status;
#endif
#if defined(NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_ACTIVE)
    uint16_t   PH_MEMLOC_REM   wIntFieldStatus = PH_OFF;
#endif
    uint8_t    PH_MEMLOC_COUNT bIndex;
#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEF212_P2P_ACTIVE) || defined (NXPBUILD__PHAC_DISCLOOP_TYPEF424_P2P_ACTIVE)
    uint8_t    PH_MEMLOC_BUF   aNfcId3[10];
#endif

    /* Reset detected technologies */
    pDataParams->bDetectedTechs = 0x00;

    /* Loop through all the supported active technologies */
    for (bIndex = 0; bIndex < PHAC_DISCLOOP_ACT_POLL_MAX_TECHS_SUPPORTED; bIndex++)
    {
        switch(pDataParams->bActPollTechCfg & (PH_ON << bIndex))
        {
            case PHAC_DISCLOOP_ACT_POS_BIT_MASK_106:
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_ACTIVE
                /* Get Config to check the internal Field On. */
                status = phhalHw_GetConfig(
                    pDataParams->pHalDataParams,
                    PHHAL_HW_CONFIG_INT_RF_ON,
                    &wIntFieldStatus);
                PH_CHECK_SUCCESS(status);

                if (wIntFieldStatus == PH_ON)
                {
                    /* Apply Guard time. */
                    PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                        pDataParams->wActPollGTimeUs));
                }

                /* Configure for active communication at 106 kbps */
                status = phhalHw_ApplyProtocolSettings(
                    pDataParams->pHalDataParams,
                    PHHAL_HW_CARDTYPE_I18092M_ACTIVE_106);
                PH_CHECK_SUCCESS(status);

                /* Perform I-RFCA and Switch on RF Field after Apply protocol settings and wait for guard time, if in Active mode. */
                PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_Int_FieldOn(pDataParams));

                if (wIntFieldStatus == PH_OFF)
                {
                    /* Apply Guard time. */
                    PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                        pDataParams->wActPollGTimeUs));
                }

                /* Send ATR Request */
                status = phpalI18092mPI_Atr(
                    pDataParams->pPal18092mPIDataParams,
                    pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aUid,
                    pDataParams->sTypeATargetInfo.sTypeA_P2P.bDid,
                    pDataParams->sTypeATargetInfo.sTypeA_P2P.bLri,
                    pDataParams->sTypeATargetInfo.sTypeA_P2P.bNadEnable,
                    pDataParams->sTypeATargetInfo.sTypeA_P2P.bNad,
                    pDataParams->sTypeATargetInfo.sTypeA_P2P.pGi,
                    pDataParams->sTypeATargetInfo.sTypeA_P2P.bGiLength,
                    pDataParams->sTypeATargetInfo.sTypeA_P2P.pAtrRes,
                    &(pDataParams->sTypeATargetInfo.sTypeA_P2P.bAtrResLength));
                if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
                {
                    /* Set the corresponding detected bit */
                    pDataParams->bDetectedTechs |= PH_ON << bIndex;
                    return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_ACTIVE_TARGET_ACTIVATED, PH_COMP_AC_DISCLOOP);
                }

                /* If Integrity error occurred then switch to Passive Polling without polling for higher Active Data rates. */
                if ((status & PH_ERR_MASK) == PH_ERR_INTEGRITY_ERROR)
                {
                    return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_TECH_DETECTED, PH_COMP_AC_DISCLOOP);
                }
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_ACTIVE */
                break;

            case PHAC_DISCLOOP_ACT_POS_BIT_MASK_212:
            case PHAC_DISCLOOP_ACT_POS_BIT_MASK_424:
#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEF212_P2P_ACTIVE) || defined (NXPBUILD__PHAC_DISCLOOP_TYPEF424_P2P_ACTIVE)

                /* Field OFF required between Active poll modes. Wait for recovery time and then apply other protocol settings. */
                PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldOff(pDataParams->pHalDataParams));
                PH_CHECK_SUCCESS_FCT(status, phhalHw_Wait(pDataParams->pHalDataParams, PHHAL_HW_TIME_MICROSECONDS, pDataParams->wActPollGTimeUs));

                /* Configure for active communication at 212 or 424 kbps */
                if((PH_ON << bIndex) == PHAC_DISCLOOP_ACT_POS_BIT_MASK_212)
                {
                    status = phhalHw_ApplyProtocolSettings(
                        pDataParams->pHalDataParams,
                        PHHAL_HW_CARDTYPE_I18092M_ACTIVE_212);
                }
                else
                {
                    status = phhalHw_ApplyProtocolSettings(
                        pDataParams->pHalDataParams,
                        PHHAL_HW_CARDTYPE_I18092M_ACTIVE_424);
                }
                PH_CHECK_SUCCESS(status);

                /* Perform I-RFCA and Switch on RF Field after Apply protocol settings and wait for guard time, if in Active mode. */
                PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_Int_FieldOn(pDataParams));

                /* Apply Guard time. */
                PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                    pDataParams->wActPollGTimeUs));

                /* Form NFCID3 */
                (void)memcpy(aNfcId3, pDataParams->sTypeFTargetInfo.aTypeFTag[0].aIDmPMm, PHAC_DISCLOOP_FELICA_IDM_LENGTH);
                aNfcId3[8] = 0x00;
                aNfcId3[9] = 0x00;

                /* Send ATR Request */
                status = phpalI18092mPI_Atr(
                    pDataParams->pPal18092mPIDataParams,
                    aNfcId3,
                    pDataParams->sTypeFTargetInfo.sTypeF_P2P.bDid,
                    pDataParams->sTypeFTargetInfo.sTypeF_P2P.bLri,
                    pDataParams->sTypeFTargetInfo.sTypeF_P2P.bNadEnable,
                    pDataParams->sTypeFTargetInfo.sTypeF_P2P.bNad,
                    pDataParams->sTypeFTargetInfo.sTypeF_P2P.pGi,
                    pDataParams->sTypeFTargetInfo.sTypeF_P2P.bGiLength,
                    pDataParams->sTypeFTargetInfo.sTypeF_P2P.pAtrRes,
                    &(pDataParams->sTypeFTargetInfo.sTypeF_P2P.bAtrResLength));
                if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
                {
                    /* Set the corresponding detected bit */
                    pDataParams->bDetectedTechs |= PH_ON << bIndex;
                    return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_ACTIVE_TARGET_ACTIVATED, PH_COMP_AC_DISCLOOP);
                }

                /* If Integrity error occurred then switch to Passive Polling without polling for higher Active Data rates. */
                if ((status & PH_ERR_MASK) == PH_ERR_INTEGRITY_ERROR)
                {
                    return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_TECH_DETECTED, PH_COMP_AC_DISCLOOP);
                }
#endif
                break;

            default:
                break;
        }
    }

    /* No Target present. */
    return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_TECH_DETECTED, PH_COMP_AC_DISCLOOP);
}

phStatus_t phacDiscLoop_Sw_Int_PollMode(
                                        phacDiscLoop_Sw_DataParams_t *pDataParams
                                       )
{
    phStatus_t PH_MEMLOC_REM status;
    uint8_t    PH_MEMLOC_REM bResolveTech;
    uint8_t    PH_MEMLOC_REM bTechType;
    uint8_t    PH_MEMLOC_REM bNumOfTechsFound = 0;

    status = PH_ERR_INVALID_PARAMETER;

    if(pDataParams->bPollState == PHAC_DISCLOOP_POLL_STATE_DETECTION)
    {
        if((pDataParams->bOpeMode == RD_LIB_MODE_NFC) ||
           (pDataParams->bOpeMode == RD_LIB_MODE_ISO))
        {
            /* Perform Technology detection Activity */
            status = phacDiscLoop_Sw_Int_TechDetectActivity(
                pDataParams,
                pDataParams->bPasPollBailOut,
                pDataParams->bPasPollTechCfg,
                &bNumOfTechsFound);
            /* Return if status is aborted. */
            PH_CHECK_ABORT(status);
        }
        else if(pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
        {
            /* Perform Technology detection Activity */
            status = phacDiscLoop_Sw_Int_EmvcoTechDetectActivity(
                pDataParams,
                pDataParams->bPasPollTechCfg,
                &bNumOfTechsFound);
            /* Return if status is aborted. */
            PH_CHECK_ABORT(status);
        }
        else
        {
            ;/* Do Nothing */
        }

        if ((0U != (pDataParams->bLpcdEnabled)) &&
           (0U != (pDataParams->bPasPollTechCfg)) &&
           ((status & PH_ERR_MASK) == PHAC_DISCLOOP_NO_TECH_DETECTED))
        {
            /* LPCD is success but card presence does not exist/errors */
            return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_LPCD_NO_TECH_DETECTED, PH_COMP_AC_DISCLOOP);
        }

        if((status & PH_ERR_MASK) == PHAC_DISCLOOP_TECH_DETECTED)
        {
            pDataParams->bPollState = PHAC_DISCLOOP_POLL_STATE_COLLISION_RESOLUTION;
        }
    }

    /* Go for collision resolution if single tech found */
    if(pDataParams->bPollState == PHAC_DISCLOOP_POLL_STATE_COLLISION_RESOLUTION)
    {
        /* Get Technology to be resolved */
        bResolveTech = pDataParams->bDetectedTechs & pDataParams->bPasPollTechCfg;
        if(0U != (bResolveTech))
        {
            if(0U != (bResolveTech & (uint8_t)(bResolveTech - 1U)))
            {
                return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AC_DISCLOOP);
            }
        }
        else
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AC_DISCLOOP);
        }

        PHAC_DISCLOOP_GET_BIT_POS(bResolveTech, bTechType);

        status = phacDiscLoop_Sw_Int_ColsnReslnActivity(pDataParams, (bTechType -  (uint8_t)1U));
        if((status & PH_ERR_MASK) == PHAC_DISCLOOP_DEVICE_RESOLVED)
        {
                /* Activation */
                status = phacDiscLoop_Sw_Int_ActivateDevice(pDataParams, (bTechType - (uint8_t)1U), (uint8_t)0x00U);
        }
        /* Return if status is aborted. */
        PH_CHECK_ABORT(status);
    }
    if(pDataParams->bPollState == PHAC_DISCLOOP_POLL_STATE_REMOVAL)
    {
        if(pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
        {
            status = phacDiscLoop_Sw_Int_EmvcoRemovalProcedure(pDataParams);
        }
    }
    return PH_ADD_COMPCODE(status, PH_COMP_AC_DISCLOOP);
}

phStatus_t phacDiscLoop_Sw_Int_EmvcoRemovalProcedure(
                                                     phacDiscLoop_Sw_DataParams_t * pDataParams
                                                     )
{
#if (defined NXPBUILD__PHAC_DISCLOOP_TYPEB_I3P4B_TAGS) && (defined NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS)

    phStatus_t PH_MEMLOC_REM   status;
    uint8_t    PH_MEMLOC_COUNT bPollingCount = 0;

    PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldOff(pDataParams->pHalDataParams));

    PH_CHECK_SUCCESS_FCT(status, phhalHw_Wait(
        pDataParams->pHalDataParams,
        PHHAL_HW_TIME_MICROSECONDS,
        PH_NXPNFCRDLIB_CONFIG_EMVCO_FIELD_OFF_DELAY_US)
        );

    /* Set poll state to Detection */
    pDataParams->bPollState = PHAC_DISCLOOP_POLL_STATE_DETECTION;

    if(PHAC_DISCLOOP_CHECK_ANDMASK(pDataParams->bDetectedTechs, PHAC_DISCLOOP_POS_BIT_MASK_A))
    {
        pDataParams->bDetectedTechs = 0x00;

        PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(
            pDataParams->pHalDataParams,
            PHHAL_HW_CARDTYPE_ISO14443A)
            );

        /* Perform Field ON after Apply protocol settings. */
        PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_Int_RfcaOff_FieldOn(pDataParams));

        while(bPollingCount < PH_NXPNFCRDLIB_CONFIG_EMVCO_REMOVAL_RETRY_COUNT)
        {
            /* Apply Guard time. */
            PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(
                pDataParams->pHalDataParams,
                PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                pDataParams->waPasPollGTimeUs[PHAC_DISCLOOP_TECH_TYPE_A]));

            status = phpalI14443p3a_WakeUpA(
                pDataParams->pPal1443p3aDataParams,
                pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa);
            if ((status & PH_ERR_MASK) != PH_ERR_IO_TIMEOUT)
            {
                /* Return if status is aborted. */
                PH_CHECK_ABORT(status);

                PH_CHECK_ABORT_FCT(status, phpalI14443p3a_HaltA(pDataParams->pPal1443p3aDataParams));
                bPollingCount = 0;
            }
            else
            {
                bPollingCount++;
            }
        }/* while(bPollingCount < Max_Retry_Count) */
    }
    else
    {
        pDataParams->bDetectedTechs = 0x00;

        PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(
            pDataParams->pHalDataParams,
            PHHAL_HW_CARDTYPE_ISO14443B)
            );

        /* Perform Field ON after Apply protocol settings. */
        PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_Int_RfcaOff_FieldOn(pDataParams));

        while(bPollingCount < PH_NXPNFCRDLIB_CONFIG_EMVCO_REMOVAL_RETRY_COUNT)
        {
            /* Apply Guard time. */
            PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(
                pDataParams->pHalDataParams,
                PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                pDataParams->waPasPollGTimeUs[PHAC_DISCLOOP_TECH_TYPE_B]));

            status = phpalI14443p3b_WakeUpB(
                pDataParams->pPal1443p3bDataParams,
                0,
                0,
                0,
                pDataParams->sTypeBTargetInfo.aTypeB_I3P3[0].aAtqB,
                &pDataParams->sTypeBTargetInfo.aTypeB_I3P3[0].bAtqBLength);
            if ((status & PH_ERR_MASK) != PH_ERR_IO_TIMEOUT)
            {
                bPollingCount = 0;

                /* Return if status is aborted. */
                PH_CHECK_ABORT(status);
            }
            else
            {
                bPollingCount++;
            }
        }/* while(bPollingCount < Max_Retry_Count) */
    }/* else */
    if ((status & PH_ERR_MASK) != PH_ERR_IO_TIMEOUT)
    {
        return status;
    }
    return PH_ERR_SUCCESS;
#else
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AC_DISCLOOP);
#endif
}


phStatus_t phacDiscLoop_Sw_Int_TechDetectActivity(
                                                  phacDiscLoop_Sw_DataParams_t *pDataParams,
                                                  uint8_t bPasPollBailOut,
                                                  uint8_t bDetectConfig,
                                                  uint8_t* pNumOfTechsDetect
                                                  )
{
    phStatus_t PH_MEMLOC_REM   status = PHAC_DISCLOOP_NO_TECH_DETECTED;
    uint8_t    PH_MEMLOC_COUNT bTechLoopIndex;
    uint8_t    PH_MEMLOC_COUNT bTechIndex;
    uint8_t    PH_MEMLOC_REM   bNumOfTechsFound = 0;
    uint8_t    PH_MEMLOC_REM   bTechTypeF_Detected = PH_OFF;
    uint16_t   PH_MEMLOC_REM   wIntFieldStatus;

    pDataParams->bDetectedTechs = 0x00;
    pDataParams->bNumOfCards = 0x00;

    /* Apply Proprietary Technology Guard Time(PTGT) if Proprietary Technology Poll enabled */
    status = phacDiscLoop_Sw_Int_Apply_PTGT(pDataParams);
    PH_CHECK_SUCCESS(status);

    /* Poll for requested NFC-Tech Type presence. */
    for (bTechLoopIndex = 0; bTechLoopIndex < PHAC_DISCLOOP_PASS_POLL_MAX_TECHS_SUPPORTED; bTechLoopIndex++)
    {
        /* In NFC mode, poll proprietary technologies only if NFC technologies are not detected */
        if((pDataParams->bOpeMode == RD_LIB_MODE_NFC) && (0U != bNumOfTechsFound)
           && (bTechLoopIndex == PHAC_DISCLOOP_POLL_MAX_NFC_TECHS_SUPPORTED))
        {
            break;
        }

        bTechIndex = pDataParams->pPasTechPollSeq[bTechLoopIndex];

        if (((bTechIndex == PHAC_DISCLOOP_TECH_TYPE_F212) || (bTechIndex == PHAC_DISCLOOP_TECH_TYPE_F424))
            && (bTechTypeF_Detected == PH_ON))
        {
            continue;
        }

        if (0U != (bDetectConfig & (PH_ON << bTechIndex)))
        {
            /* Get Config to check the internal Field On. */
            status = phhalHw_GetConfig(
                pDataParams->pHalDataParams,
                PHHAL_HW_CONFIG_INT_RF_ON,
                &wIntFieldStatus);
            PH_CHECK_SUCCESS(status);

            if (wIntFieldStatus == PH_ON)
            {
                if (((uint8_t)PHAC_DISCLOOP_TECH_TYPE_F212 == bTechIndex) ||
                    ((uint8_t)PHAC_DISCLOOP_TECH_TYPE_F424 ==  bTechIndex))
                {
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS
                    PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_Int_Config_GTF(pDataParams, bTechIndex));
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS */
                }
                else
                {
                    /* Apply Guard time. */
                    PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                        pDataParams->waPasPollGTimeUs[bTechIndex]));
                }
            }

            /* Apply Protocol Setting for Selected Tech  */
            PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(pDataParams->pHalDataParams,
                (uint8_t)(gPasTechTypeMapTable[bTechIndex] & 0xFFU)));

            /* Perform I-RFCA and Switch on RF Field after Apply protocol settings, if in NFC and ISO mode. */
            PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_Int_FieldOn(pDataParams));

            if (wIntFieldStatus == PH_OFF)
            {
                if (((uint8_t) PHAC_DISCLOOP_TECH_TYPE_F212 ==  bTechIndex) ||
                    ((uint8_t) PHAC_DISCLOOP_TECH_TYPE_F424 ==  bTechIndex))
                {
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS
                    PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_Int_Config_GTF(pDataParams, bTechIndex));
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEF_TAGS */
                }
                else
                {
                    /* Apply Guard time. */
                    PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                        pDataParams->waPasPollGTimeUs[bTechIndex]));
                }
            }

            /* Perform Tech detection. */
            status = pfDetTechs[bTechIndex](pDataParams);


            if ((status & PH_ERR_MASK) == PHAC_DISCLOOP_TECH_DETECTED)
            {
                /*Set the corresponding detected bit. */
                pDataParams->bDetectedTechs |= PH_ON << bTechIndex;
                bNumOfTechsFound++;
                /* Since Device detected at 212 Baud, Skip polling at 424 */
                if((0U != ((pDataParams->bDetectedTechs & PHAC_DISCLOOP_POS_BIT_MASK_F212)))
                        || (0U != ((pDataParams->bDetectedTechs & PHAC_DISCLOOP_POS_BIT_MASK_F424))))
                {
                    *pNumOfTechsDetect = bNumOfTechsFound;
                    bTechTypeF_Detected = PH_ON;
                }
            }
            else
            {
                PH_CHECK_ABORT(status);
            }

            if((0U != (bPasPollBailOut & (PH_ON << bTechIndex))) && (0U != bNumOfTechsFound))
            {
                /* Bailout set, Returning to application */
                *pNumOfTechsDetect = bNumOfTechsFound;
                break;
            }
        }
    }

    if (bNumOfTechsFound == 1U)
    {
        *pNumOfTechsDetect = bNumOfTechsFound;
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_TECH_DETECTED, PH_COMP_AC_DISCLOOP);
    }
    else if(bNumOfTechsFound > 1U)
    {
        *pNumOfTechsDetect = bNumOfTechsFound;
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_MULTI_TECH_DETECTED, PH_COMP_AC_DISCLOOP);
    }
    else if(PH_ERR_IO_TIMEOUT == (status & PH_ERR_MASK))
    {
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_TECH_DETECTED, PH_COMP_AC_DISCLOOP);
    }
    else
    {
        return status; /* Other Failure */
    }
}

phStatus_t phacDiscLoop_Sw_Int_EmvcoTechDetectActivity(
                                                       phacDiscLoop_Sw_DataParams_t *pDataParams,
                                                       uint8_t bDetectConfig,
                                                       uint8_t* pNumOfTechsDetect
                                                       )
{
    phStatus_t PH_MEMLOC_REM status = PHAC_DISCLOOP_NO_TECH_DETECTED;
    uint8_t    PH_MEMLOC_REM bNumOfTechsFound = 0;
    uint8_t    PH_MEMLOC_REM bCurrentPollTech = 0;
    uint8_t    PH_MEMLOC_COUNT bTechLoopIndex = 0;
    uint8_t    PH_MEMLOC_COUNT bTechIndex = 0;
    uint8_t    PH_MEMLOC_COUNT bSkipRfReset = PH_OFF;
    uint8_t    PH_MEMLOC_REM bPropPollTechEn = PH_OFF;
    uint8_t    PH_MEMLOC_REM bStopTechDetection = PH_OFF;
    uint16_t   PH_MEMLOC_REM wIntFieldStatus;
    uint16_t   PH_MEMLOC_REM wPasBGTimeUs;

    /* EMVCo Polling procedure section 9.2, Symbol 1 */
    pDataParams->bDetectedTechs = 0x00;


    /* Verify Type A and Type B polling sequence.
     * Note: As per the EMVCo specification,
     * 1st polling technology should be Type A and 2nd polling technology should be Type B.
     * */
    if ((pDataParams->pPasTechPollSeq[0] != PHAC_DISCLOOP_TECH_TYPE_A) ||
        (pDataParams->pPasTechPollSeq[1] != PHAC_DISCLOOP_TECH_TYPE_B))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AC_DISCLOOP);
    }


    /* Get Config to check the internal Field On. */
    status = phhalHw_GetConfig(
        pDataParams->pHalDataParams,
        PHHAL_HW_CONFIG_INT_RF_ON,
        &wIntFieldStatus);
    PH_CHECK_SUCCESS(status);

    if (wIntFieldStatus == PH_OFF)
    {
        /* Apply Protocol Setting */
        PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(
            pDataParams->pHalDataParams,
            (uint8_t)(gPasTechTypeMapTable[0] & 0xFFU)));

        /* Disable RFCA and switch on RF, if in EMV mode. */
        PH_CHECK_SUCCESS_FCT(status, phacDiscLoop_Sw_Int_RfcaOff_FieldOn(pDataParams));

        /* Get Config to check the internal Field On. */
        status = phhalHw_GetConfig(
            pDataParams->pHalDataParams,
            PHHAL_HW_CONFIG_INT_RF_ON,
            &wIntFieldStatus);
        PH_CHECK_SUCCESS(status);

        if (wIntFieldStatus != PH_ON)
        {
            return PH_ADD_COMPCODE_FIXED(PH_ERR_RF_ERROR, PH_COMP_AC_DISCLOOP);
        }
    }
    /* EMVCo: Run until a tag is discovered */
    while(0U == bStopTechDetection )
    {
        /* Poll for all requested technologies  */
        for (bTechLoopIndex = 0; bTechLoopIndex < PHAC_DISCLOOP_PASS_POLL_MAX_TECHS_SUPPORTED; bTechLoopIndex++)
        {
            bTechIndex = pDataParams->pPasTechPollSeq[bTechLoopIndex];
            bCurrentPollTech = (PH_ON << bTechIndex);

            /* Symbol 2 (8 and 13) */
            if ((0U != (bDetectConfig & bCurrentPollTech)) && (bStopTechDetection == 0U))
            {
                if((0U == ((pDataParams->bDetectedTechs & bCurrentPollTech))))
                {

                    if (bCurrentPollTech & (uint8_t)(~(PHAC_DISCLOOP_POS_BIT_MASK_A | PHAC_DISCLOOP_POS_BIT_MASK_B)))

                    {
                        bPropPollTechEn = PH_ON;
                    }

                    /* Optional Field Reset in case of Proprietary technologies polling */
                    if ((pDataParams->bEMVCoPropOpRfFieldReset & bCurrentPollTech) && (bPropPollTechEn == PH_ON))
                    {
                        /* Symbol 14 */
                        phacDiscLoop_Sw_Int_EmvcoRfFieldReset(pDataParams);
                    }

                    if(bCurrentPollTech == PHAC_DISCLOOP_POS_BIT_MASK_B)
                    {
                        if((0U != ((pDataParams->bDetectedTechs & PHAC_DISCLOOP_POS_BIT_MASK_A))) &&
                                (pDataParams->waPasPollGTimeUs[bTechIndex] > PHAC_DISCLOOP_HALTA_TIMOUT_US))
                        {
                            wPasBGTimeUs = pDataParams->waPasPollGTimeUs[bTechIndex] - PHAC_DISCLOOP_HALTA_TIMOUT_US;
                        }
                        else
                        {
                            wPasBGTimeUs = pDataParams->waPasPollGTimeUs[bTechIndex];
                        }
                        /* Apply Guard time. Symbol 3 (9 and 15) */
                        PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                            wPasBGTimeUs));
                    }
                    else
                    {
                        /* Apply Guard time. Symbol 3 (9 and 15) */
                        PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
                            pDataParams->waPasPollGTimeUs[bTechIndex]));
                    }

                    /* Apply Protocol Setting for Selected Technology  */
                    PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(
                        pDataParams->pHalDataParams,
                        (uint8_t)(gPasTechTypeMapTable[bTechIndex] & 0xFFU)));

                    /* Perform Tech detection. Symbol 4 (10 and 16) */
                    status = pfDetTechs[bTechIndex](pDataParams);

                    if ((status & PH_ERR_MASK) == PHAC_DISCLOOP_TECH_DETECTED)
                    {
                        /* Set the corresponding detected bit. Symbol 6 (12 and 18) */
                        pDataParams->bDetectedTechs |= bCurrentPollTech;
                        bNumOfTechsFound++;
                    }
                    else
                    {
                        PH_CHECK_ABORT(status);
                    }

                    /* Optional Field Reset in case of Proprietary technologies polling. */
                    if ((pDataParams->bEMVCoPropOpRfFieldReset & bCurrentPollTech) && (bPropPollTechEn == PH_ON))
                    {
                        /* Symbol 19 */
                        phacDiscLoop_Sw_Int_EmvcoRfFieldReset(pDataParams);
                        bSkipRfReset = PH_ON;
                    }
                    else
                    {
                        bSkipRfReset = PH_OFF;
                    }
                }
                else
                {
                    bStopTechDetection = PH_ON;
                }
            }
        }

        if(bNumOfTechsFound == 0U)
        {
            bStopTechDetection = PH_ON;
        }

        if((bPropPollTechEn == PH_ON) && (bSkipRfReset == PH_OFF))
        {
            /* Symbol 20 */
            phacDiscLoop_Sw_Int_EmvcoRfFieldReset(pDataParams);
        }
    } /* End of while(!bStopTechDetection ) */

    if (bNumOfTechsFound == 1U)
    {
        *pNumOfTechsDetect = bNumOfTechsFound;
        status = PHAC_DISCLOOP_TECH_DETECTED;
    }
    else if(bNumOfTechsFound > 1U)
    {
        *pNumOfTechsDetect = bNumOfTechsFound;
        status = PHAC_DISCLOOP_MULTI_TECH_DETECTED;
    }
    else if(PH_ERR_IO_TIMEOUT == (status & PH_ERR_MASK))
    {
        status = PHAC_DISCLOOP_NO_TECH_DETECTED;
    }
    else
    {
        ; /* Do Nothing */
    }
    return PH_ADD_COMPCODE(status, PH_COMP_AC_DISCLOOP);
}

phStatus_t phacDiscLoop_Sw_Int_ColsnReslnActivity(
                                                  phacDiscLoop_Sw_DataParams_t *pDataParams,
                                                  uint8_t bTechType
                                                  )
{
    phStatus_t PH_MEMLOC_REM wStatus;

    /* Since we are interested in one Technology, Reset DetectedTechs information */
    pDataParams->bDetectedTechs = 0x00;

    /*Re-set number of card*/
    pDataParams->bNumOfCards = 0x00;

    /* Call the selected collision resolution function */
    wStatus = pfColnRelsns[bTechType](pDataParams);

    if((wStatus & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        /* Collision Resolution activity is successful, update Detected Technology */
        pDataParams->bDetectedTechs = (PH_ON << bTechType);

        if(pDataParams->bNumOfCards > 1U)
        {
            return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_MULTI_DEVICES_RESOLVED, PH_COMP_AC_DISCLOOP);
        }
        else
        {
            return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_DEVICE_RESOLVED, PH_COMP_AC_DISCLOOP);
        }
    }

    if((wStatus & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT)
    {
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_DEVICE_RESOLVED, PH_COMP_AC_DISCLOOP);
    }

    return wStatus;
}

phStatus_t phacDiscLoop_Sw_Int_ActivateDevice(
                                              phacDiscLoop_Sw_DataParams_t * pDataParams,
                                              uint8_t bTechType,
                                              uint8_t bTagIndex
                                              )
{
    phStatus_t PH_MEMLOC_REM status;

    /* Since we are interested in one Technology, Reset DetectedTechs information */
    pDataParams->bDetectedTechs = 0x00;

    /* Activate selected tag */
    status = pfDeviceActivate[bTechType](pDataParams, bTagIndex);
    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
        /* Set Corresponding Tech Bit Position */
        pDataParams->bDetectedTechs = PH_ON << bTechType;
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_DEVICE_ACTIVATED, PH_COMP_AC_DISCLOOP);
    }
    else if(((status & PH_ERR_MASK) == PHAC_DISCLOOP_MERGED_SEL_RES_FOUND)
       || ((status & PH_ERR_MASK) == PHAC_DISCLOOP_PASSIVE_TARGET_ACTIVATED))
    {
        /* Set Corresponding Tech Bit Position */
        pDataParams->bDetectedTechs = PH_ON << bTechType;
    }
    else
    {
        return status;
    }
    return status;
}

phStatus_t phacDiscLoop_Sw_Int_RfcaOff_FieldOn(
                                               phacDiscLoop_Sw_DataParams_t * pDataParams
                                               )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_SUCCESS;

    /* Disable RFCA (if supported) */
    status = phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RFCA, PH_OFF);
    if (((status & PH_ERR_MASK) != PH_ERR_UNSUPPORTED_PARAMETER) && ((status & PH_ERR_MASK) != PH_ERR_SUCCESS))
    {
        return status;
    }

    /* Switch on RF without performing I-RFCA. */
    PH_CHECK_SUCCESS_FCT(status, phhalHw_FieldOn(pDataParams->pHalDataParams));

    return status;
}

phStatus_t phacDiscLoop_Sw_Int_FieldOn(
                                       phacDiscLoop_Sw_DataParams_t * pDataParams
                                       )
{
    phStatus_t PH_MEMLOC_REM status = PH_ERR_SUCCESS;

    /* Enable RFCA (if supported) */
    status = phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RFCA, PH_ON);
    if(((status & PH_ERR_MASK) != PH_ERR_UNSUPPORTED_PARAMETER) && ((status & PH_ERR_MASK) != PH_ERR_SUCCESS))
    {
        return status;
    }

    /* Perform I-RFCA and Switch on RF Field. */
    status = phhalHw_FieldOn(pDataParams->pHalDataParams);
    if((status & PH_ERR_MASK) == PH_ERR_RF_ERROR)
    {
        /* External RF is ON */
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_EXTERNAL_RFON, PH_COMP_AC_DISCLOOP);
    }
    else
    {
        return PH_ADD_COMPCODE(status, PH_COMP_AC_DISCLOOP);
    }
}

uint8_t phacDiscLoop_Sw_Int_IsValidPollStatus(
                                              phStatus_t wStatus
                                              )
{
    if((((wStatus) & PH_ERR_MASK) == PH_ERR_SUCCESS)         ||
       (((wStatus) & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR) ||
       (((wStatus) & PH_ERR_MASK) == PH_ERR_FRAMING_ERROR)   ||
       (((wStatus) & PH_ERR_MASK) == PH_ERR_INTEGRITY_ERROR) ||
       (((wStatus) & PH_ERR_MASK) == PH_ERR_PROTOCOL_ERROR)
       )
    {
        return PH_ON;
    }
    else
    {
        return PH_OFF;
    }
}

#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
