/*----------------------------------------------------------------------------*/
/* Copyright 2014-2023 NXP                                                    */
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
* Discovery Loop Activities for Type A polling.
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
#include <phpalI18092mPI.h>
#include <phpalI14443p3a.h>
#include <phpalI14443p4a.h>
#include <phpalI14443p4.h>
#include <phalT1T.h>

#ifdef NXPBUILD__PHAC_DISCLOOP_SW
#include "phacDiscLoop_Sw_Int.h"
#include "phacDiscLoop_Sw_Int_A.h"

/* *****************************************************************************************************************
 * Global and Static Variables
 * ***************************************************************************************************************** */
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS
static const uint8_t aCascadeCodes[] = {
                                        PHPAL_I14443P3A_CASCADE_LEVEL_1,
                                        PHPAL_I14443P3A_CASCADE_LEVEL_2,
                                        PHPAL_I14443P3A_CASCADE_LEVEL_3
                                        };
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */
/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */
phStatus_t phacDiscLoop_Sw_DetTechTypeA(
                                        phacDiscLoop_Sw_DataParams_t *pDataParams
                                        )
{
#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS)
    uint8_t    PH_MEMLOC_COUNT bIndex;
    phStatus_t PH_MEMLOC_REM   wStatus;

    /* Reset total tags found */
    pDataParams->sTypeATargetInfo.bTotalTagsFound = 0;

    /* Reset Collision Bit for Type A*/
    pDataParams->bCollPend &= (uint8_t)~(uint8_t)PHAC_DISCLOOP_POS_BIT_MASK_A;
    /* Reset the Sleep flag for Type 3 activation status */
    for(bIndex = 0U; bIndex < PHAC_DISCLOOP_CFG_MAX_CARDS_SUPPORTED; bIndex++)
    {
        /* Device is not in HLTA state */
        pDataParams->sTypeATargetInfo.aTypeA_I3P3[bIndex].bSleep = 0U;
        pDataParams->sTypeATargetInfo.aTypeA_I3P3[bIndex].bUidSize = 0U;
    }

    /* sending the WakeUpA */
    wStatus = phpalI14443p3a_WakeUpA(
        pDataParams->pPal1443p3aDataParams,
        pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa);

    if(0u != (phacDiscLoop_Sw_Int_IsValidPollStatus(wStatus)))
    {
        if((wStatus & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
        {
            pDataParams->bCollPend |= PHAC_DISCLOOP_POS_BIT_MASK_A;
        }

        PH_CHECK_SUCCESS_FCT(wStatus, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_TXWAIT_US, 500));

        /* Halt the detected cards. */
        PH_CHECK_ABORT_FCT(wStatus, phpalI14443p3a_HaltA(pDataParams->pPal1443p3aDataParams));

        pDataParams->sTypeATargetInfo.bTotalTagsFound = 1;

    }
    else
    {
        return wStatus;
    }

    return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_TECH_DETECTED, PH_COMP_AC_DISCLOOP);
#else /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AC_DISCLOOP);
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */
}

phStatus_t phacDiscLoop_Sw_Int_CollisionResolutionA(
                                                    phacDiscLoop_Sw_DataParams_t * pDataParams
                                                    )
{
#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS)
    phStatus_t PH_MEMLOC_REM   status = PH_ERR_SUCCESS;
    uint8_t    PH_MEMLOC_REM   bTypeANvbUid;
    uint8_t    PH_MEMLOC_COUNT bDeviceCount;
    uint8_t    PH_MEMLOC_COUNT bTypeATagIdx;
    uint8_t    PH_MEMLOC_COUNT bCascadeCodeIdx;
    uint8_t    PH_MEMLOC_BUF   aTypeAUid[7] = {0};
    uint8_t    PH_MEMLOC_COUNT bRetryCount;
    uint8_t    PH_MEMLOC_REM   bCollDetected;
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS
    uint16_t   *pUIDLen;
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS */

    bRetryCount = 0;
    bDeviceCount = 0;
    bTypeATagIdx = 0;
    bCollDetected = PH_OFF;

    /* Collision_Pending = 1 and Device limit  = 0 */
    if((0U != ((pDataParams->bCollPend & PHAC_DISCLOOP_POS_BIT_MASK_A))) && ((pDataParams->baPasConDevLim[PHAC_DISCLOOP_TECH_TYPE_A] == 0x00U)))
    {
        pDataParams->sTypeATargetInfo.bTotalTagsFound = 0;
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_DEVICE_RESOLVED, PH_COMP_AC_DISCLOOP);
    }

    /*Symbol 0*/
    /* Apply Guard time. */
    PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(
        pDataParams->pHalDataParams,
        PHHAL_HW_CONFIG_POLL_GUARD_TIME_US,
        pDataParams->waPasPollGTimeUs[PHAC_DISCLOOP_TECH_TYPE_A]));

    /* Configure HW for the TypeA technology */
    PH_CHECK_SUCCESS_FCT(status, phhalHw_ApplyProtocolSettings(
        pDataParams->pHalDataParams,
        PHHAL_HW_CARDTYPE_ISO14443A));

    /* Send WakeUpA */
    status = phpalI14443p3a_WakeUpA(
        pDataParams->pPal1443p3aDataParams,
        pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa);
    if ((status & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
    {
        /* In case of EMVCo, return Collision Pending status. */
        if(pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
        {
            /* Report Error to Application and Application will perform PICC Reset */
            return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_COLLISION_PENDING, PH_COMP_AC_DISCLOOP);
        }
        bCollDetected = PH_ON;
    }

    /* Reset card detected count */
    pDataParams->sTypeATargetInfo.bT1TFlag = 0;
    pDataParams->sTypeATargetInfo.bTotalTagsFound = 0;
    /*Symbol 1*/
    if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
    {
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS
        /*Symbol 2*/
        /* Check for T1T Tag*/
        if(pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa[0] == 0x00U)
        {
            /* Enable Jewel Mode */
            PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_JEWEL_MODE, PH_ON));

            pUIDLen = (uint16_t *)&(pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].bUidSize);
            /*Symbol 23*/
            status = phalT1T_ReadUID(
                pDataParams->pAlT1TDataParams,
                pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aUid,
                pUIDLen);

            if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
            {
                /*Symbol 27*/
                /* indicates T1T card*/
                pDataParams->sTypeATargetInfo.bT1TFlag = 1;
                pDataParams->sTypeATargetInfo.bTotalTagsFound = 1;
                pDataParams->bNumOfCards = 1;
                /* Symbol 26 */
                pDataParams->bCollPend &= (uint8_t)~(uint8_t)PHAC_DISCLOOP_POS_BIT_MASK_A;

                return PH_ADD_COMPCODE(PH_COMP_AC_DISCLOOP, PH_ERR_SUCCESS);
            }
            else
            {
                /*Symbol 24*/
                if ((status & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
                {
                    /* Symbol 25 */
                    pDataParams->bCollPend |= PHAC_DISCLOOP_POS_BIT_MASK_A;
                    return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_COLLISION_PENDING, PH_COMP_AC_DISCLOOP);
                }
                return status;
            }
        }
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_JEWEL_TAGS */
    }
    else
    {
        /* Device count = 0 and any Error */
        if(((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
          && (pDataParams->baPasConDevLim[PHAC_DISCLOOP_TECH_TYPE_A] == 0x00U))
        {
            pDataParams->sTypeATargetInfo.bTotalTagsFound = 0;
            pDataParams->bCollPend |= PHAC_DISCLOOP_POS_BIT_MASK_A;
            return status;
        }

        /* As per EMVCo 3.1, wait for at least Tmin retransmission in case of timeout error. */
        if (pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
        {
            bRetryCount = 0;
            while (((status & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT) && (bRetryCount < PH_NXPNFCRDLIB_CONFIG_EMVCO_RETRYCOUNT))
            {
                bRetryCount++;
                /* Wait for at least Tmin retransmission delay. */
                PH_CHECK_SUCCESS_FCT(status, phhalHw_Wait(
                    pDataParams->pHalDataParams,
                    PHHAL_HW_TIME_MICROSECONDS,
                    PH_NXPNFCRDLIB_CONFIG_EMVCO_DEFAULT_RETRANSMISSION));

                status = phpalI14443p3a_WakeUpA(
                    pDataParams->pPal1443p3aDataParams,
                    pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aAtqa);
            }

            /* Collision error may happen */
            pDataParams->bCollPend &= (uint8_t)~(uint8_t)PHAC_DISCLOOP_POS_BIT_MASK_A;

            /* Some error that can't be handled */
            PH_CHECK_SUCCESS(status);
        }
        else
        {
            /*Symbol 1*/
            if ((status & PH_ERR_MASK) != PH_ERR_COLLISION_ERROR)
            {
                pDataParams->bCollPend &= (uint8_t)~(uint8_t)PHAC_DISCLOOP_POS_BIT_MASK_A;
                return status;
            }
        }
    }

    /*Symbol 2*/
    if(0U == (pDataParams->bUseAntiColl))
    {
        /*Symbol 23*/
        pDataParams->bCollPend |= (uint8_t)PHAC_DISCLOOP_POS_BIT_MASK_A;
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_COLLISION_PENDING, PH_COMP_AC_DISCLOOP);
    }

    while (bDeviceCount <= pDataParams->baPasConDevLim[PHAC_DISCLOOP_TECH_TYPE_A])
    {
        if (bTypeATagIdx <= pDataParams->baPasConDevLim[PHAC_DISCLOOP_TECH_TYPE_A])
        {
            /*Symbol 4*/
            pDataParams->bCollPend &= (uint8_t)~(uint8_t)PHAC_DISCLOOP_POS_BIT_MASK_A;

            /*Symbol 4, 16*/
            for (bCascadeCodeIdx=0; bCascadeCodeIdx<3U; bCascadeCodeIdx++)
            {
                bTypeANvbUid = 0;

                /* Anti-collision loop */
                while (bTypeANvbUid != 0x40U)
                {
                    /*Symbol 5,6,7*/
                    status = phpalI14443p3a_Anticollision(
                        pDataParams->pPal1443p3aDataParams,
                        aCascadeCodes[bCascadeCodeIdx],      /* Cascade code */
                        aTypeAUid,                           /* Input UID */
                        bTypeANvbUid,                        /* UID len = 0 */
                        aTypeAUid,                           /* UID out */
                        &bTypeANvbUid);                      /* UID out size */

                    /* As per EMVCo 3.1, wait for at least Tmin retransmission in case of timeout error. */
                    if (pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
                    {
                        bRetryCount = 0;
                        while (((status & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT) && (bRetryCount < PH_NXPNFCRDLIB_CONFIG_EMVCO_RETRYCOUNT))
                        {
                            bRetryCount++;
                            /* Wait for at least Tmin retransmission delay. */
                            PH_CHECK_SUCCESS_FCT(status, phhalHw_Wait(
                                pDataParams->pHalDataParams,
                                PHHAL_HW_TIME_MICROSECONDS,
                                PH_NXPNFCRDLIB_CONFIG_EMVCO_DEFAULT_RETRANSMISSION));

                            status = phpalI14443p3a_Anticollision(
                                pDataParams->pPal1443p3aDataParams,
                                aCascadeCodes[bCascadeCodeIdx],      /* Cascade code */
                                aTypeAUid,                           /* Input UID */
                                bTypeANvbUid,                        /* UID len = 0 */
                                aTypeAUid,                           /* UID out */
                                &bTypeANvbUid);                      /* UID out size */
                        }
                    }

                    /*Symbol 8, 13*/
                    if ((status & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
                    {
                        /* Emvco: case_id TA302_00 */
                        if(pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
                        {
                            /* Report Error to Application and Application will perform PICC Reset */
                            return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_COLLISION_PENDING, PH_COMP_AC_DISCLOOP);
                        }

                        /*Symbol 9*/
                        pDataParams->bCollPend |= PHAC_DISCLOOP_POS_BIT_MASK_A;
                        if(bTypeATagIdx >= pDataParams->baPasConDevLim[PHAC_DISCLOOP_TECH_TYPE_A])
                        {
                            /*Symbol 10*/
                            return PH_ERR_SUCCESS;
                        }

                        /* Increment the number of valid bits and set the collision bit to one and handle based on Byte and Bit boundaries. */
                        if ((bTypeANvbUid & ((uint8_t)0x07U)) <  (uint8_t)7U)
                        {
                            bTypeANvbUid++;

                            /* Set collision bit to 1 to resolve tag with highest UID */
                            aTypeAUid[(uint8_t)((bTypeANvbUid & 0xF0U) >> 4U)] |= (uint8_t)(0x01 << ((bTypeANvbUid & ((uint8_t)0x07U)) - 1U));
                        }
                        else
                        {
                            bTypeANvbUid = (uint8_t)((((bTypeANvbUid & 0xF0U) >> 4U) + 1U) << 4U);

                            /* Set collision bit to 1 to resolve tag with highest UID */
                            aTypeAUid[(uint8_t)(((bTypeANvbUid & 0xF0U) >> 4U) - 1U)] |= (uint8_t)(0x80U);
                        }
                    }
                    else
                    {
                        /* Check success */
                        /* Some error that can't be handled */
                        PH_CHECK_SUCCESS(status); /* TA302_01*/
                    }
                } /*End of while (bTypeANvbUid != 0x40U)*/

                /* Atqa(which indicates NFCID1 size) is invalid since collision is encountered while receiving Atqa */
                if (bCollDetected == PH_OFF)
                {
                    /* EMVCO: case_id 302_10 */
                    /*
                    * Digital Proto: Article 4.7.2 and 4.7.2.4
                    * ISO14443-3 Article 6.5.2.1
                    * EMVCo article 5.4.2
                    * So For Single UID Size The NFC Forum Device MUST set nfcid10 of a single-size NFCID1 and nfcid13
                    * of a double-size NFCID1 to a value different from 88h.
                    */
                    if(((0U == ((pDataParams->bCollPend & PHAC_DISCLOOP_POS_BIT_MASK_A))))
                       && ((((pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aAtqa[0] & 0xC0U) == 0x00U)
                       && (aCascadeCodes[bCascadeCodeIdx] == PHPAL_I14443P3A_CASCADE_LEVEL_1))
                       || (((pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aAtqa[0] & 0xC0U) == 0x40U)
                       && (aCascadeCodes[bCascadeCodeIdx] == PHPAL_I14443P3A_CASCADE_LEVEL_2)))
                       && (aTypeAUid[0] == 0x88U))
                    {
                        return PH_ADD_COMPCODE_FIXED(PH_ERR_PROTOCOL_ERROR, PH_COMP_AC_DISCLOOP);
                    }
                }

                /*symbol 14*/
                status = phpalI14443p3a_Select(
                    pDataParams->pPal1443p3aDataParams,
                    aCascadeCodes[bCascadeCodeIdx],
                    aTypeAUid,
                    &pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak);

                /* As per EMVCo 3.1, wait for at least Tmin retransmission in case of timeout error. */
                if (pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
                {
                    bRetryCount = 0;
                    while (((status & PH_ERR_MASK) == PH_ERR_IO_TIMEOUT) && (bRetryCount < PH_NXPNFCRDLIB_CONFIG_EMVCO_RETRYCOUNT))
                    {
                        bRetryCount++;
                        /* Wait for at least Tmin retransmission delay. */
                        PH_CHECK_SUCCESS_FCT(status, phhalHw_Wait(
                            pDataParams->pHalDataParams,
                            PHHAL_HW_TIME_MICROSECONDS,
                            PH_NXPNFCRDLIB_CONFIG_EMVCO_DEFAULT_RETRANSMISSION));

                        status = phpalI14443p3a_Select(
                            pDataParams->pPal1443p3aDataParams,
                            aCascadeCodes[bCascadeCodeIdx],
                            aTypeAUid,
                            &pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak);
                    }
                }

                if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
                {
                    /*Symbol 15*/
                    if ((0U == (pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak & 0x04U)))
                    {
                        /*UID Complete*/
                        break;
                    }
                }
                else
                {
                    return status;
                }

            } /*for()*/

            /* If select was successful */
            if (status == PH_ERR_SUCCESS)
            {
                /*Symbol 18*/
                status = phpalI14443p3a_GetSerialNo(
                    pDataParams->pPal1443p3aDataParams,
                    &pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aUid[0],
                    &pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].bUidSize);

                PH_CHECK_SUCCESS(status);

                /*Symbol 17*/
                bTypeATagIdx++;

                /*Symbol 19*/
                if((0U != ((pDataParams->bCollPend & PHAC_DISCLOOP_POS_BIT_MASK_A))) && (bTypeATagIdx < pDataParams->baPasConDevLim[PHAC_DISCLOOP_TECH_TYPE_A]))
                {
                    /* Symbol 20 */
                    status = phpalI14443p3a_HaltA(pDataParams->pPal1443p3aDataParams);
                    if (status != PH_ERR_SUCCESS)
                    {
                        /* Halt A has failed, can't count this as detected tag */
                        return status;
                    }

                    /* Update the corresponding Layer 3 dataparam to indicate the Tag is in HALT State. */
                    (pDataParams->sTypeATargetInfo).aTypeA_I3P3[bTypeATagIdx - (uint8_t)1U].bSleep = (uint8_t)1U;

                    /* Symbol 21 */
                    status = phpalI14443p3a_RequestA(
                        pDataParams->pPal1443p3aDataParams,
                        pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aAtqa);

                    if((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
                    {
                        /*Symbol 3*/
                        if(pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aAtqa[0] == 0x00U)
                        {
                            /*Symbol 23*/
                            pDataParams->bCollPend |= PHAC_DISCLOOP_POS_BIT_MASK_A;
                            return PH_ERR_SUCCESS;
                        }
                    }
                    else if((status & PH_ERR_MASK) == PH_ERR_COLLISION_ERROR)
                    {
                        /* More device to resolve */
                        pDataParams->bCollPend |= PHAC_DISCLOOP_POS_BIT_MASK_A;
                    }
                    else
                    {
                        return status;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        bDeviceCount++;
    } /* End of while(bDeviceCount < pDataParams->baPasConDevLim[0]) */

    pDataParams->sTypeATargetInfo.bTotalTagsFound = bTypeATagIdx;
    pDataParams->bNumOfCards = bTypeATagIdx;

    if(pDataParams->sTypeATargetInfo.bTotalTagsFound == 0U)
    {
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_DEVICE_RESOLVED, PH_COMP_AC_DISCLOOP);
    }

    return PH_ERR_SUCCESS;
#else /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AC_DISCLOOP);
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */
}

phStatus_t phacDiscLoop_Sw_Int_ActivateA(
                                         phacDiscLoop_Sw_DataParams_t * pDataParams,
                                         uint8_t bTypeATagIdx
                                         )
{
#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS)
    phStatus_t   PH_MEMLOC_REM status = PH_ERR_SUCCESS;
    uint8_t      PH_MEMLOC_BUF aUid[10];
    uint8_t      PH_MEMLOC_REM bUidLen;
    uint8_t      PH_MEMLOC_REM bMoreCardsAvailable;
    uint8_t      PH_MEMLOC_REM bTagType;
    uint8_t      PH_MEMLOC_REM bIndex;
    uint8_t      PH_MEMLOC_REM bActiveDevice = 0;
#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS
    uint8_t      PH_MEMLOC_REM bCidEnabled;
    uint8_t      PH_MEMLOC_REM bCid;
    uint8_t      PH_MEMLOC_REM bNadSupported;
    uint8_t      PH_MEMLOC_REM bFwi;
    uint8_t      PH_MEMLOC_REM bFsdi;
    uint8_t      PH_MEMLOC_REM bFsci;
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS */

    if ((bTypeATagIdx >= pDataParams->sTypeATargetInfo.bTotalTagsFound) || ( bTypeATagIdx >= PHAC_DISCLOOP_CFG_MAX_CARDS_SUPPORTED ))
    {
        /* Out of range or no such card found yet */
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AC_DISCLOOP);
    }

    if(pDataParams->sTypeATargetInfo.bTotalTagsFound > PHAC_DISCLOOP_CFG_MAX_CARDS_SUPPORTED)
    {
        /* To fix coverity issue */
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INTERNAL_ERROR, PH_COMP_AC_DISCLOOP);
    }

    /* If it is Single device or selected device is at last position, In both case device is already active */
    if(pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].bSleep == 1U) /* Step 0 */
    {
        for (bIndex = 0; bIndex < pDataParams->sTypeATargetInfo.bTotalTagsFound; bIndex++)
        {
            /* Step 1 */
            if ((pDataParams->sTypeATargetInfo.aTypeA_I3P3[bIndex].bSleep == 0U) && (bIndex != bTypeATagIdx))
            {
                bActiveDevice = 1;
                /* Update dataparam to indicate that the tag will now be moved to IDLE state. */
                pDataParams->sTypeATargetInfo.aTypeA_I3P3[bIndex].bSleep = 1U;
            }
        }
        if (bActiveDevice != 0U)
        {
            /* Step 2 of Activation flow */
            status = phpalI14443p3a_HaltA(pDataParams->pPal1443p3aDataParams);
            if (status != PH_ERR_SUCCESS)
            {
                /* Halt A has failed, can't count this as detected tag */
                return status;
            }
        }

        /* Activate the card in sleep with given UID */
        PH_CHECK_SUCCESS_FCT(status, phpalI14443p3a_ActivateCard(
            pDataParams->pPal1443p3aDataParams,
            ((pDataParams->sTypeATargetInfo).aTypeA_I3P3[bTypeATagIdx]).aUid,
            ((pDataParams->sTypeATargetInfo).aTypeA_I3P3[bTypeATagIdx]).bUidSize,
            aUid,
            &bUidLen,
            &(((pDataParams->sTypeATargetInfo).aTypeA_I3P3[bTypeATagIdx]).aSak),
            &bMoreCardsAvailable));

        pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].bSleep = 0U;

    }


    bTagType = (pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aSak & 0x60U);
    bTagType = bTagType >> 5U;

    /* Type 4A */
    if(bTagType == PHAC_DISCLOOP_TYPEA_TYPE4A_TAG_CONFIG_MASK)
    {
#if defined (NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS)
        /* Enable Emd Check */
        PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_SET_EMD, PH_ON));

        PH_CHECK_SUCCESS_FCT(status, phpalI14443p4a_ActivateCard(
            pDataParams->pPal1443p4aDataParams,
            pDataParams->sTypeATargetInfo.sTypeA_I3P4.bFsdi,
            pDataParams->sTypeATargetInfo.sTypeA_I3P4.bCid,
            pDataParams->sTypeATargetInfo.sTypeA_I3P4.bDri,
            pDataParams->sTypeATargetInfo.sTypeA_I3P4.bDsi,
            pDataParams->sTypeATargetInfo.sTypeA_I3P4.pAts));

        /* Retrieve 14443-4A protocol parameter */
        PH_CHECK_SUCCESS_FCT(status, phpalI14443p4a_GetProtocolParams(
            pDataParams->pPal1443p4aDataParams,
            &bCidEnabled,
            &bCid,
            &bNadSupported,
            &bFwi,
            &bFsdi,
            &bFsci));

        /* EMVCo v3.1: Limit the FSCI value to be used based on the RdLib execution environment. */
        if (pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
        {
            if (bFsci > pDataParams->bFsciMax)
            {
                bFsci = pDataParams->bFsciMax;
            }
        }

        /* Set 14443-4 protocol parameter */
        PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_SetProtocol(
            pDataParams->pPal14443p4DataParams,
            bCidEnabled,
            bCid,
            bNadSupported,
            pDataParams->sTypeATargetInfo.sTypeA_I3P4.bNad,
            bFwi,
            bFsdi,
            bFsci));
#else /* NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS */
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_DEVICE_RESOLVED, PH_COMP_AC_DISCLOOP);
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS */
    }

    if (bTagType == PHAC_DISCLOOP_TYPEA_TYPE_NFC_DEP_TAG_CONFIG_MASK)
    {
        /* In case of EMVCo Profile and P2P tag is detected. */
        if (pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
        {
            /* Return to application with No device resolved; In EMVCo mode only Type-A ISO 14443-4 compliant cards should be activated. */
            return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_DEVICE_RESOLVED, PH_COMP_AC_DISCLOOP);
        }

#ifdef NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_TAGS
        /* Send ATR_REQ and PSL_REQ */
        PH_CHECK_SUCCESS_FCT(status, phpalI18092mPI_Atr(
            pDataParams->pPal18092mPIDataParams,
            pDataParams->sTypeATargetInfo.aTypeA_I3P3[bTypeATagIdx].aUid,
            pDataParams->sTypeATargetInfo.sTypeA_P2P.bDid,
            pDataParams->sTypeATargetInfo.sTypeA_P2P.bLri,
            pDataParams->sTypeATargetInfo.sTypeA_P2P.bNadEnable,
            pDataParams->sTypeATargetInfo.sTypeA_P2P.bNad,
            pDataParams->sTypeATargetInfo.sTypeA_P2P.pGi,
            pDataParams->sTypeATargetInfo.sTypeA_P2P.bGiLength,
            pDataParams->sTypeATargetInfo.sTypeA_P2P.pAtrRes,
            &(pDataParams->sTypeATargetInfo.sTypeA_P2P.bAtrResLength)));

        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_PASSIVE_TARGET_ACTIVATED, PH_COMP_AC_DISCLOOP);
#else /* NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_TAGS */
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_DEVICE_RESOLVED, PH_COMP_AC_DISCLOOP);
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_P2P_TAGS */
    }

    /* Support for both Type 4A and NFC-DEP (Merged SAK) */
    if (bTagType == PHAC_DISCLOOP_TYPEA_TYPE_NFC_DEP_TYPE4A_TAG_CONFIG_MASK)
    {
#ifdef  NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS
        if(pDataParams->bOpeMode == RD_LIB_MODE_EMVCO)
        {
            /* Enable Emd Check in Emvco mode */
            PH_CHECK_SUCCESS_FCT(status, phhalHw_SetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_SET_EMD, PH_ON));

            PH_CHECK_SUCCESS_FCT(status, phpalI14443p4a_ActivateCard(
                pDataParams->pPal1443p4aDataParams,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bFsdi,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bCid,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bDri,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bDsi,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.pAts));

            /* Retrieve 14443-4A protocol parameter */
            PH_CHECK_SUCCESS_FCT(status, phpalI14443p4a_GetProtocolParams(
                pDataParams->pPal1443p4aDataParams,
                &bCidEnabled,
                &bCid,
                &bNadSupported,
                &bFwi,
                &bFsdi,
                &bFsci));

            /* EMVCo v3.1: Limit the FSCI value to be used based on the RdLib execution environment. */
            if (bFsci > pDataParams->bFsciMax)
            {
                bFsci = pDataParams->bFsciMax;
            }

            /* Set 14443-4 protocol parameter */
            PH_CHECK_SUCCESS_FCT(status, phpalI14443p4_SetProtocol(
                pDataParams->pPal14443p4DataParams,
                bCidEnabled,
                bCid,
                bNadSupported,
                pDataParams->sTypeATargetInfo.sTypeA_I3P4.bNad,
                bFwi,
                bFsdi,
                bFsci));
        }
        else
#endif /*  NXPBUILD__PHAC_DISCLOOP_TYPEA_I3P4_TAGS */
        {
            /* Return to application; application can decide what to send RATS or ATR */
            return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_MERGED_SEL_RES_FOUND, PH_COMP_AC_DISCLOOP);
        }
    }

    /* Case of EMVCo Profile and type 2 tag */
    if ((bTagType == PHAC_DISCLOOP_TYPEA_TYPE2_TAG_CONFIG_MASK) && (pDataParams->bOpeMode == RD_LIB_MODE_EMVCO))
    {
        /* Return to application with No device resolved; In EMVCo mode only Type-A ISO 14443-4 compliant cards should be activated. */
        return PH_ADD_COMPCODE_FIXED(PHAC_DISCLOOP_NO_DEVICE_RESOLVED, PH_COMP_AC_DISCLOOP);
    }

    return PH_ERR_SUCCESS;
#else /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */
    return PH_ADD_COMPCODE_FIXED(PH_ERR_UNSUPPORTED_COMMAND, PH_COMP_AC_DISCLOOP);
#endif /* NXPBUILD__PHAC_DISCLOOP_TYPEA_TAGS */
}
#endif /* NXPBUILD__PHAC_DISCLOOP_SW */
