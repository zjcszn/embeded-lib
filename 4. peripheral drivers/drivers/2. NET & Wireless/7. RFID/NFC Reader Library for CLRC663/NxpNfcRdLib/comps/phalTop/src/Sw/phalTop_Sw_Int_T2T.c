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
* Internal functions for Tag Operation Application Layer Component of
* Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*/
#include <ph_TypeDefs.h>
#include <ph_Status.h>
#include <ph_RefDefs.h>
#include <phNxpNfcRdLib_Config.h>

#include <phalMful.h>
#include <phalTop.h>

#ifdef NXPBUILD__PHAL_TOP_SW
#ifdef NXPBUILD__PHAL_TOP_T2T_SW

#include "phalTop_Sw_Int_T2T.h"

/* Global array for Lock control TLV and Memory control TLV structure */
static phalTop_T2T_MemCtrlTlv_t gbMemCtrlTlv[PHAL_TOP_T2T_MAX_MEM_CTRL_TLV] = {0};                                     /**< Memory TLV details for each TLV present */
static phalTop_T2T_LockCtrlTlv_t gbLockCtrlTlv[PHAL_TOP_T2T_MAX_LOCK_CTRL_TLV] = {0};                                 /**< Lock TLV details for each TLV present */

phStatus_t phalTop_Sw_Int_T2T_SetReadOnly(
                                          phalTop_Sw_DataParams_t * pDataParams
                                          )
{
    phStatus_t    PH_MEMLOC_REM   status;
    uint8_t       PH_MEMLOC_BUF   aData[16];
    uint16_t      PH_MEMLOC_REM   wBlockNo;
    uint16_t      PH_MEMLOC_COUNT wIndex;
    uint16_t      PH_MEMLOC_REM   wTempIndex;
    uint16_t      PH_MEMLOC_REM   wOffset;
    uint16_t      PH_MEMLOC_COUNT wLockBytesCount = 0;
    uint16_t      PH_MEMLOC_COUNT wLockedBytes = 0;
    uint16_t      PH_MEMLOC_REM   wDefaultLockAddress;
    uint16_t      PH_MEMLOC_REM   wDefaultLockLength = 0;
    phalTop_T2T_t PH_MEMLOC_REM * pT2T = &pDataParams->ualTop.salTop_T2T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT2T->pAlT2TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Check if tag is in valid state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Check for NDEF length > 0 (in initialized state NDEF length is 0) */
    if(pDataParams->bTagState == PHAL_TOP_STATE_INITIALIZED)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_EMPTY_NDEF, PH_COMP_AL_TOP);
    }

    /* Check if tag is already in read-only state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_READONLY)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_READONLY_TAG, PH_COMP_AL_TOP);
    }

    /* Read static lock bytes and CC */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
        pT2T,
        PHAL_TOP_T2T_STATIC_LOCK_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        aData));

    /* Update R/W access byte of CC to RO */
    pT2T->bRwa = PHAL_TOP_T2T_CC_RWA_RO;
    aData[7] = PHAL_TOP_T2T_CC_RWA_RO;

    /* Write back CC */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
        pT2T,
        PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        &aData[4]));

    /* Set static lock bytes */
    aData[2] = 0xFF;
    aData[3] = 0xFF;

    /* Write back static lock bytes */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
        pT2T,
        PHAL_TOP_T2T_STATIC_LOCK_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        aData));

    /* For static tags only static lock bits are present, so exit */
    if(pT2T->bTagMemoryType == PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC)
    {
        /* Update state */
        pDataParams->bTagState = PHAL_TOP_STATE_READONLY;

        return PH_ERR_SUCCESS;
    }

    /* Update dynamic lock bytes */
    if(0U != (pT2T->bLockTlvCount))
    {
        for(wIndex = 0; wIndex < pT2T->bLockTlvCount; wIndex++)
        {
            wBlockNo = (pT2T->pLockCtrlTlv[wIndex].wByteAddr / PHAL_TOP_T2T_BYTES_PER_BLOCK);
            wLockedBytes = (((uint16_t)(pT2T->pLockCtrlTlv[wIndex].bSizeInBits)) / 8U);
            wLockBytesCount = 0;
            while(wLockBytesCount < wLockedBytes)
            {
            /* Read dynamic lock bytes */
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
                    pT2T,
                    (wBlockNo * PHAL_TOP_T2T_BYTES_PER_BLOCK),
                    aData));

                if(wBlockNo == (pT2T->pLockCtrlTlv[wIndex].wByteAddr / PHAL_TOP_T2T_BYTES_PER_BLOCK))
                {
                    wOffset = pT2T->pLockCtrlTlv[wIndex].wByteAddr % PHAL_TOP_T2T_BYTES_PER_BLOCK;
                }
                else
                {
                    wOffset = 0;
                }
                wTempIndex = wOffset;
                wLockedBytes = (((uint16_t)(pT2T->pLockCtrlTlv[wIndex].bSizeInBits)) / 8U);
                for( ; ((wLockBytesCount < wLockedBytes) && (wTempIndex <= 16U));wLockBytesCount++,wTempIndex++)
                {
                    if(wTempIndex != 16U)
                    {
                        aData[wTempIndex] = 0xFF;
                    }

                    /* If one block is updated with lock bytes, write that block */
                    if((wTempIndex >= 4U) && ((wTempIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK) == 0U))
                    {
                        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                            pT2T,
                            ((wBlockNo + ((wTempIndex / PHAL_TOP_T2T_BYTES_PER_BLOCK) - 1U)) * PHAL_TOP_T2T_BYTES_PER_BLOCK),
                            &aData[(((wTempIndex / PHAL_TOP_T2T_BYTES_PER_BLOCK) - 1U) * 4U)]));
                    }

                }

                wTempIndex--;

                /* Write partially filled lock bytes */
                if((wTempIndex <= 16U) && ((wLockedBytes % PHAL_TOP_T2T_BYTES_PER_BLOCK) != 0U))
                {
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                        pT2T,
                        ((wBlockNo + (wTempIndex / PHAL_TOP_T2T_BYTES_PER_BLOCK)) * PHAL_TOP_T2T_BYTES_PER_BLOCK),
                        &aData[((wTempIndex / PHAL_TOP_T2T_BYTES_PER_BLOCK) * 4U)]));
                }
                wBlockNo += 4U;
            }
        }
    }
    /* If lock TLVs are not present, set default dynamic lock bits */
    else
    {
        /* Calculate default dynamic lock bytes starting address (data area +
         * first 4 blocks of sector 0) */
        wDefaultLockAddress = (uint16_t) ((((uint16_t)(pT2T->bTms)) * 8U) + /* */ 16U);

        /* Calculate default dynamic lock length (in bits) */
        wDefaultLockLength = (uint16_t) (((((uint16_t)(pT2T->bTms)) * 8U /* */) - 48U) / 8U);

        /* If there are one or more block(s) of dynamic lock bytes, set it */
        for(wIndex = 0; wIndex < ((wDefaultLockLength / 8U) / PHAL_TOP_T2T_BYTES_PER_BLOCK); wIndex++)
        {
            (void)memset(aData, 0xFF, 4);
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                pT2T,
                wDefaultLockAddress,
                aData));
            wDefaultLockAddress += 4U;
        }

        /* Check for partial dynamic lock block/byte */
        if((((wDefaultLockLength / 8U) % PHAL_TOP_T2T_BYTES_PER_BLOCK) != 0U) || ((wDefaultLockLength % 8U) != 0U))
        {
            (void)memset(aData, 0x00, 4);
            wIndex = 0;

            /* Calculate partial lock bytes (less than one block) */
            if(0U != ((wDefaultLockLength / 8U) % PHAL_TOP_T2T_BYTES_PER_BLOCK))
            {
                for(wIndex = 0; wIndex < ((wDefaultLockLength / 8U) % PHAL_TOP_T2T_BYTES_PER_BLOCK); wIndex++)
                {
                    aData[wIndex] = 0xFF;
                }
            }

            /* Calculate partial lock bits (less than one byte) */
            if(0U != (wDefaultLockLength % 8U))
            {
                aData[wIndex] = (uint8_t)((1U << (wDefaultLockLength % 8U)) - 1U);
            }

            /* Set partial dynamic lock block/byte */
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                pT2T,
                wDefaultLockAddress,
                aData));
        }
    }

    /* Update state */
    pDataParams->bTagState = PHAL_TOP_STATE_READONLY;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T2T_ClearState(
                                         phalTop_Sw_DataParams_t * pDataParams,
                                         phalTop_T2T_t * pT2T
                                         )
{
    /* Reset parameters */
    pT2T->bLockTlvCount = 0;
    pT2T->bMemoryTlvCount = 0;
    pT2T->bTagMemoryType = 0;
    pDataParams->bTagState = 0;
    pDataParams->dwMaxNdefLength = 0;
    pT2T->wNdefHeaderAddr = 0;
    pDataParams->dwNdefLength = 0;
    pT2T->wNdefMsgAddr = 0;
    pT2T->sSector.bAddress = 0;
    pT2T->sSector.bBlockAddress = 0;
    pT2T->sSector.bValidity = PH_OFF;
    pT2T->bDataIndex = 0;
    pT2T->wStartBlockNum = 0;
    /* Clearing T2T Lock and Reserved bytes */
    (void)memset(pT2T->sSector.bLockReservedOtp, 0x00, sizeof(pT2T->sSector.bLockReservedOtp));
    /* Clearing NDEF data(V) bytes along with NDEF T and L bytes, when reading 16 bytes data. */
    (void)memset(pT2T->bData, 0x00, sizeof(pT2T->bData));
    /* Clearing Memory control TLV bytes. */
    (void)memset(gbMemCtrlTlv, 0x00, (sizeof(phalTop_T2T_MemCtrlTlv_t) * PHAL_TOP_T2T_MAX_MEM_CTRL_TLV));
    /* Clearing Lock control TLV bytes. */
    (void)memset(gbLockCtrlTlv, 0x00, (sizeof(phalTop_T2T_LockCtrlTlv_t) * PHAL_TOP_T2T_MAX_MEM_CTRL_TLV));

    return PH_ERR_SUCCESS;
}

/* Finds the count and position of memory, lock and NDEF TLV */
phStatus_t phalTop_Sw_Int_T2T_DetectTlvBlocks(
                                              phalTop_Sw_DataParams_t * pDataParams,
                                              phalTop_T2T_t * pT2T
                                              )
{
    phStatus_t PH_MEMLOC_REM   status;
    uint16_t   PH_MEMLOC_COUNT wIndex;
    uint16_t   PH_MEMLOC_REM   wTotalBytes;
    uint16_t   PH_MEMLOC_REM   wPageAddr;
    uint8_t    PH_MEMLOC_REM   bByteOffset;
    uint8_t    PH_MEMLOC_REM   bBytesPerPage;
    uint8_t    PH_MEMLOC_BUF   aLength[3] = { 0U };
    uint8_t    PH_MEMLOC_REM   bExitLoop = 0;
    uint8_t    PH_MEMLOC_REM   bBlockNum;
    uint8_t    PH_MEMLOC_BUF   aData[16];
    uint8_t    PH_MEMLOC_REM   bDataLen;
    uint8_t    PH_MEMLOC_REM   bConsecutiveNullTlv = 0;
    uint16_t   PH_MEMLOC_REM   wShift = 1;
    uint8_t    PH_MEMLOC_REM   bOffset = 0;
    uint8_t    PH_MEMLOC_REM   bTemp = 0;

    /* Set maximum data length for looping */
    bDataLen = 16;

    /* Check for TLVs only in first sector */
    for(bBlockNum = 4;
        ((bBlockNum < (((pT2T->bTms * 8U) / PHAL_TOP_T2T_BYTES_PER_BLOCK) + 4U)) && (bBlockNum <= 252U));
        bBlockNum += 4U)
    {
        /* Read 16 bytes data and store it to buffer aData */
        wTotalBytes = ((uint16_t)(bBlockNum)) * PHAL_TOP_T2T_BYTES_PER_BLOCK;
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
            pT2T,
            wTotalBytes,
            aData));

        /* Set data starting index for loop */
        wIndex = 0;

        /* Check for TLVs in read data */
        while((wIndex < bDataLen) && (bExitLoop == 0U))
        {
            if(aData[wIndex] == PHAL_TOP_T2T_NULL_TLV)
            {
                /* Consecutive NULL TLV's are present in the Tag memory */
                bConsecutiveNullTlv++;

                /* T2T Tag SHALL NOT contain more than three consecutive Null TLVs. Requirements 4.9.4.3 of Type 2 Tag specification v1.2   */
                if(bConsecutiveNullTlv > 3)
                {
                    return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
                }
            }
            else
            {
                /* Reset the count if NULL TLV's are not consecutive in the Tag memory*/
                bConsecutiveNullTlv = 0;
            }
            switch(aData[wIndex])
            {
            case PHAL_TOP_T2T_NULL_TLV:
                 break;

            case PHAL_TOP_T2T_LOCK_CTRL_TLV:
                if(pT2T->bTagMemoryType != PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC)
                {
                    wTotalBytes = (((uint16_t)(bBlockNum)) * PHAL_TOP_T2T_BYTES_PER_BLOCK);
                    pT2T->pLockCtrlTlv[pT2T->bLockTlvCount].wOffset =
                            wTotalBytes + wIndex;

                    wPageAddr = ((uint16_t)(aData[wIndex + 2U])) & 0xF0U;
                    wPageAddr = wPageAddr >> 4U;
                    bByteOffset = aData[wIndex + 2U] & 0x0FU;

                    pT2T->pLockCtrlTlv[pT2T->bLockTlvCount].bSizeInBits = aData[wIndex + 3U];

                    bBytesPerPage = aData[wIndex + 4U] & 0x0FU;
                    pT2T->pLockCtrlTlv[pT2T->bLockTlvCount].bBytesPerPage = aData[wIndex + 4U] & 0x0FU;

                    wShift = 1U;
                    pT2T->pLockCtrlTlv[pT2T->bLockTlvCount].bBytesLockedPerBit =
                        (uint8_t)((aData[wIndex + 4U] & 0xF0U) >> 4U);

                    pT2T->pLockCtrlTlv[pT2T->bLockTlvCount].bBytesLockedPerBit =
                            (wShift << ((uint16_t)pT2T->pLockCtrlTlv[pT2T->bLockTlvCount].bBytesLockedPerBit));

                    pT2T->pLockCtrlTlv[pT2T->bLockTlvCount].wByteAddr =
                        (wPageAddr * (wShift << ((uint16_t)bBytesPerPage))) + bByteOffset;

                    pT2T->bLockTlvCount++;
                    wIndex = wIndex + 4U;

                    /* Update reserved/lock bytes status */
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
                        pT2T,
                        0x00));
                }
                break;

            case PHAL_TOP_T2T_MEMORY_CTRL_TLV:
                if(pT2T->bTagMemoryType != PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC)
                {
                    wTotalBytes = (((uint16_t)(bBlockNum)) * PHAL_TOP_T2T_BYTES_PER_BLOCK);
                    pT2T->pMemCtrlTlv[pT2T->bMemoryTlvCount].wOffset =
                            wTotalBytes + wIndex;
                    wPageAddr = ((uint16_t)(aData[wIndex + 2U])) & 0xF0U;
                    wPageAddr = wPageAddr >> 4U;
                    bByteOffset = aData[wIndex + 2U] & 0x0FU;
                    pT2T->pMemCtrlTlv[pT2T->bMemoryTlvCount].bSizeInBytes = aData[wIndex + 3U];

                    bBytesPerPage = aData[wIndex + 4U] & 0x0FU;
                    pT2T->pMemCtrlTlv[pT2T->bMemoryTlvCount].bBytesPerPage = aData[wIndex + 4U] & 0x0FU;

                    wShift = 1U;
                    pT2T->pMemCtrlTlv[pT2T->bMemoryTlvCount].wByteAddr =
                        (wPageAddr * (wShift << ((uint16_t)bBytesPerPage))) + bByteOffset;
                    pT2T->bMemoryTlvCount++;
                    wIndex = wIndex + 4U;

                    /* Update reserved/lock bytes status */
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
                        pT2T,
                        0x00));
                }
                break;

            case PHAL_TOP_T2T_NDEF_TLV:
                wTotalBytes = (((uint16_t)(bBlockNum)) * PHAL_TOP_T2T_BYTES_PER_BLOCK);
                pT2T->wNdefHeaderAddr = wTotalBytes + wIndex;

                /* Update block num to start reading NDEF data during Read_Ndef */
                pT2T->wStartBlockNum = (wTotalBytes + bDataLen) / PHAL_TOP_T2T_BYTES_PER_BLOCK;

                /* NDEF TLV Length offset */
                bOffset = ((pT2T->wNdefHeaderAddr % PHAL_TOP_T2T_BYTES_PER_BLOCK) + 1);

                /* NDEF TLV Length(L) bytes are split between the blocks and only partial Length(L) data is present in the current 16 bytes read,
                 * issue one more read cmd to read the remaining Length(L) data */
                if((wIndex == bDataLen - 1) ||
                        (((wIndex == bDataLen - 2) || (wIndex == bDataLen - 3)) && (aData[wIndex + 1] == 0xFF)))
                {
                    /* Copy NDEF TLV Header block, which will be used to clear/update NDEF length during WriteNdef  */
                    memcpy(pT2T->bNdefHeaderBlock, &aData[wIndex - (wIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK)], PHAL_TOP_T2T_BYTES_PER_BLOCK);

                    wTotalBytes += bDataLen;

                    /* Update block num to start reading NDEF data during Read_Ndef */
                    pT2T->wStartBlockNum = (wTotalBytes + bDataLen) / PHAL_TOP_T2T_BYTES_PER_BLOCK;

                    /* Read 16 bytes data and store it to buffer aData */
                    wTotalBytes = ((uint16_t)(bBlockNum)) * PHAL_TOP_T2T_BYTES_PER_BLOCK;
                    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
                        pT2T,
                        wTotalBytes,
                        aData));

                    /* Copy the first block in 16 bytes of data read, as NDEF TLV Length(L) bytes are split between the blocks */
                    memcpy(&pT2T->bNdefHeaderBlock[4], &aData[0], PHAL_TOP_T2T_BYTES_PER_BLOCK);

                    aLength[0] = pT2T->bNdefHeaderBlock[bOffset++];

                    if(aLength[0] == 0xFFU)
                    {
                        aLength[1] = pT2T->bNdefHeaderBlock[bOffset++];
                        aLength[2] = pT2T->bNdefHeaderBlock[bOffset];
                    }

                    /* Calculate the index from which the NDEF data bytes are present in the 16 bytes read data buffer */
                    if((wIndex == bDataLen - 1) && (aLength[0] == 0xFF))
                    {
                        bTemp = 3;
                    }
                    else if((wIndex == bDataLen - 2) && (aLength[0] == 0xFF))
                    {
                        bTemp = 2;
                    }
                    else
                    {
                        bTemp = 1;
                    }

                    /* Copy the remaining NDEF data bytes apart from T and L bytes to global buffer, which will be used during ReadNdef */
                    memcpy(pT2T->bData, &aData[bTemp], (bDataLen - bTemp));
                    pT2T->bDataIndex = (bDataLen - bTemp);
                }
                /* NDEF header lies at second last byte in the current 16 bytes read and length is less the 0xFEU */
                else if((wIndex == bDataLen - 2) && (aData[wIndex + 1] != 0xFF))
                {
                    /* Copy NDEF TLV Header block, which will be used to clear/update NDEF length during WriteNdef  */
                    memcpy(pT2T->bNdefHeaderBlock, &aData[wIndex - (wIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK)], PHAL_TOP_T2T_BYTES_PER_BLOCK);

                    aLength[0] = pT2T->bNdefHeaderBlock[bOffset];
                }
                /* NDEF TLV Length(L) bytes are split between the blocks and complete Length(L) data is present in the current 16 bytes read */
                else if((((wIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK) == PHAL_TOP_T2T_BYTES_PER_BLOCK - 1) ||
                ((((wIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK) == PHAL_TOP_T2T_BYTES_PER_BLOCK - 2) || ((wIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK) == PHAL_TOP_T2T_BYTES_PER_BLOCK - 3)) && (aData[wIndex + 1] == 0xFF))) && (bDataLen - wIndex >= 4))
                {
                    /* Copy NDEF TLV Header block and proceeding block data, if NDEF TLV is split between the block.
                     *  Which will be used to clear/update NDEF length during WriteNdef */
                    memcpy(pT2T->bNdefHeaderBlock, &aData[wIndex - (wIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK)], PHAL_TOP_T2T_BYTES_PER_BLOCK * 2);

                    aLength[0] = pT2T->bNdefHeaderBlock[bOffset++];

                    if(aLength[0] == 0xFFU)
                    {
                        aLength[1] = pT2T->bNdefHeaderBlock[bOffset++];
                        aLength[2] = pT2T->bNdefHeaderBlock[bOffset];
                    }
                }
                /* Complete NDEF (T) and (L) bytes are present in one block in the current 16 bytes read */
                else
                {
                    //* Copy NDEF TLV Header block, which will be used to clear/update NDEF length during WriteNdef  */
                    memcpy(pT2T->bNdefHeaderBlock, &aData[wIndex - (wIndex % PHAL_TOP_T2T_BYTES_PER_BLOCK)], PHAL_TOP_T2T_BYTES_PER_BLOCK);

                    aLength[0] = pT2T->bNdefHeaderBlock[bOffset++];

                    if(aLength[0] == 0xFFU)
                    {
                        aLength[1] = pT2T->bNdefHeaderBlock[bOffset++];
                        aLength[2] = pT2T->bNdefHeaderBlock[bOffset];
                    }
                }

                if(aLength[0] == 0xFFU)
                {
                    if(pT2T->bDataIndex == 0)
                    {
                        /* Copy the remaining NDEF data bytes apart from T and L bytes to global buffer, which will be used during ReadNdef */
                        memcpy(pT2T->bData, &aData[wIndex + 4U], (bDataLen - (wIndex + 4U)));
                        pT2T->bDataIndex = (bDataLen - (wIndex + 4U));
                    }

                    pDataParams->dwNdefLength = ((uint16_t)aLength[1] << 8U) | aLength[2];
                    pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 4U;
                }
                else
                {
                    if(pT2T->bDataIndex == 0)
                    {
                        /* Copy the remaining NDEF data bytes apart from T and L bytes to global buffer, which will be used during ReadNdef */
                        memcpy(pT2T->bData, &aData[wIndex + 2U], (bDataLen - (wIndex + 2U)));
                        pT2T->bDataIndex = (bDataLen - (wIndex + 2U));
                    }

                    pDataParams->dwNdefLength = aLength[0];
                    pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 2U;
                }

                /* Only one NDEF TLV is supported, exit loop */
                bExitLoop = 1;
                break;

            case PHAL_TOP_T2T_TERMINATOR_TLV:
                /* Exit loop after when terminator TLV is detected */
                bExitLoop = 1;
                break;

            default:
                /* Found RFU TLV, SKip the (L) bytes of tag memory and set the Index to next TLV byte */
                /* get the length of the respective TLV */
                aLength[0] = aData[wIndex + 1U];

                if(aLength[0] == 0xFFU)
                {
                    aLength[1] = aData[wIndex + 2U];
                    aLength[2] = aData[wIndex + 3U];

                    wIndex = (uint8_t)(wIndex + (((uint16_t)aLength[1] << 8U) | aLength[2]) + 3U);
                }
                else
                {
                    wIndex = wIndex + aLength[0] + 1U;
                }
                break;
            }
            wIndex++;
        }

        /* Exit condition set, exit loop */
        if(0U != (bExitLoop))
        {
            break;
        }
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T2T_CheckNdef(
                                        phalTop_Sw_DataParams_t * pDataParams,
                                        uint8_t * pTagState
                                        )
{
    phStatus_t    PH_MEMLOC_REM status;
    uint8_t       PH_MEMLOC_BUF aData[16];
    phalTop_T2T_t PH_MEMLOC_REM * pT2T = &pDataParams->ualTop.salTop_T2T;

    pDataParams->ualTop.salTop_T2T.pLockCtrlTlv = &gbLockCtrlTlv[0];
    pDataParams->ualTop.salTop_T2T.pMemCtrlTlv = &gbMemCtrlTlv[0];

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT2T->pAlT2TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Reset tag state */
    *pTagState = PHAL_TOP_STATE_NONE;

    /* Clear values from previous detection, if any */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_ClearState(pDataParams, pT2T));

    /* Read CC */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
        pT2T,
        0x0U,
        aData));

    /* Check for NDEF Magic Number */
    if(aData[12] != PHAL_TOP_T2T_NDEF_MAGIC_NUMBER)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_NON_NDEF_TAG, PH_COMP_AL_TOP);
    }

    /* Update version number */
    pDataParams->bVno = aData[13];

    /* Check for supported version number */
    if((uint8_t)(pDataParams->bVno & 0xF0U) != (uint8_t)(PHAL_TOP_T2T_NDEF_SUPPORTED_VNO & 0xF0U))
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_UNSUPPORTED_VERSION, PH_COMP_AL_TOP);
    }

    /* Update tag memory size */
    pT2T->bTms = aData[14];

    /* Validate tag memory size */
    if((pT2T->bTms * 8U) == 48U)
    {
        pT2T->bTagMemoryType = PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC;
    }
    else if((pT2T->bTms * 8U) > 48U)
    {
        pT2T->bTagMemoryType = PHAL_TOP_T2T_TAG_MEMORY_TYPE_DYNAMIC;
    }
    else
    {
        /* Min tag memory size should be 48 bytes */
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
    }

    /* Update read/write access */
    pT2T->bRwa = aData[15];

    /* Check for proprietary and RFU values of R/W access */
    if((pT2T->bRwa != PHAL_TOP_T2T_CC_RWA_RW) && (pT2T->bRwa != PHAL_TOP_T2T_CC_RWA_RO))
    {
        /* Proprietary options; Not supported */
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
    }

    /* Check for TLVs */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_DetectTlvBlocks(pDataParams, pT2T));

    /* Update tag state */
    if((pT2T->wNdefHeaderAddr != 0U) && (pDataParams->dwNdefLength != 0U))
    {
        if(pT2T->bRwa == PHAL_TOP_T2T_CC_RWA_RW)
        {
            pDataParams->bTagState = PHAL_TOP_STATE_READWRITE;
        }
        else
        {
            pDataParams->bTagState = PHAL_TOP_STATE_READONLY;
        }
    }
    else if((pT2T->wNdefHeaderAddr != 0U) && (pDataParams->dwNdefLength == 0U))
    {
        if(pT2T->bRwa == PHAL_TOP_T2T_CC_RWA_RW)
        {
            pDataParams->bTagState = PHAL_TOP_STATE_INITIALIZED;
        }
        else
        {
            return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
        }
    }
    else
    {
        /* No NDEF TLV detected */
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_MISCONFIGURED_TAG, PH_COMP_AL_TOP);
    }

    /* Update max. NDEF size */
    phalTop_Sw_Int_T2T_CalculateMaxNdefSize(pDataParams, pT2T);

    /* Update reserved/lock bytes status for first sector */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
        pT2T,
        0x00));

    /* Update state in out parameter */
    *pTagState = pDataParams->bTagState;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T2T_EraseNdef(
                                        phalTop_Sw_DataParams_t * pDataParams
                                        )
{
    phStatus_t    PH_MEMLOC_REM status;
    uint8_t       PH_MEMLOC_BUF aData[16];
    uint16_t      PH_MEMLOC_REM wOffset;
    phalTop_T2T_t PH_MEMLOC_REM * pT2T = &pDataParams->ualTop.salTop_T2T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT2T->pAlT2TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Check if tag is in valid state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Check if tag is read-only */
    if(pDataParams->bTagState == PHAL_TOP_STATE_READONLY)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_READONLY_TAG, PH_COMP_AL_TOP);
    }

    /* Check if tag is already in initialized state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_INITIALIZED)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_EMPTY_NDEF, PH_COMP_AL_TOP);
    }

    /* NDEF length offset */
    wOffset = pT2T->wNdefHeaderAddr + 1U;

    /* Read Block with NDEF length */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
        pT2T,
        (pT2T->wNdefHeaderAddr + 1U),
        aData));

    /* Set NDEF length as zero */
    aData[wOffset % PHAL_TOP_T2T_BYTES_PER_BLOCK] = 0x00;

    /* Write NDEF length */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
        pT2T,
        (pT2T->wNdefHeaderAddr + 1U),
        aData));

    /* Reset state variables */
    pDataParams->bTagState = PHAL_TOP_STATE_INITIALIZED;
    pDataParams->dwNdefLength = 0x00;
    pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 2U;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T2T_FormatNdef(
                                         phalTop_Sw_DataParams_t * pDataParams
                                         )
{
    phStatus_t    PH_MEMLOC_REM   status;
    uint8_t       PH_MEMLOC_BUF   aData[16];
    uint16_t      PH_MEMLOC_COUNT wIndex;
    uint16_t      PH_MEMLOC_COUNT wWriteIndex;
    uint16_t      PH_MEMLOC_REM   wTlvOffset;
    uint8_t       PH_MEMLOC_BUF   aTlv[8];
    uint8_t       PH_MEMLOC_BUF   aNdefData[3] = {0xD0, 0x00, 0x00};
    uint16_t      PH_MEMLOC_COUNT wShift = 1;

    phalTop_T2T_t PH_MEMLOC_REM * pT2T = &pDataParams->ualTop.salTop_T2T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT2T->pAlT2TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Check for NDEF presence */
    if(pDataParams->bTagState != PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_FORMATTED_TAG, PH_COMP_AL_TOP);
    }

    /* Set NMN */
    aData[0] = 0xE1;

    /* If version is not set, set default version number */
    pDataParams->bVno = PHAL_TOP_T2T_NDEF_SUPPORTED_VNO;
    aData[1] = pDataParams->bVno;

    /* If TMS is not set/wrong, set TMS of static tag  */
    if(pT2T->bTms < 0x06U)
    {
        pT2T->bTms = 0x06;
        pT2T->bTagMemoryType = PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC;
        aData[2] = pT2T->bTms;
    }
    else
    {
        pT2T->bTagMemoryType = PHAL_TOP_T2T_TAG_MEMORY_TYPE_DYNAMIC;
        aData[2] = pT2T->bTms;
    }

    /* Set R/W access */
    pT2T->bRwa = 0x00;
    aData[3] = pT2T->bRwa;

    /* Write CC (CC is OTP) */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
        pT2T,
        PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        aData));

    /* Read back CC */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
        pT2T,
        PHAL_TOP_T2T_CC_BLOCK * PHAL_TOP_T2T_BYTES_PER_BLOCK,
        aData));

    /* Check if CC write was successful (CC is OTP, configuring CC can fail) */
    if((aData[0] != 0xE1U) || (aData[1] != pDataParams->bVno) ||
       (aData[2] != pT2T->bTms) || (aData[3] != pT2T->bRwa))
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_UNSUPPORTED_TAG, PH_COMP_AL_TOP);
    }

    /* TLV offset (if present) */
    wTlvOffset = 4;

    /* Reset TLV data */
    (void)memset(aTlv, 0, 8);

    /* Write lock control TLVs (if specified) */
    for(wIndex = 0; ((wIndex < pT2T->bLockTlvCount) && (wIndex < PHAL_TOP_T2T_MAX_LOCK_CTRL_TLV)); wIndex++)
    {
        /* TLV offset from block start */
        pT2T->pLockCtrlTlv[wIndex].wOffset =  wTlvOffset;

        /* Create lock TLVs */
        aTlv[(wTlvOffset++) % 8U] = 0x01;
        aTlv[(wTlvOffset++) % 8U] = 0x03;
        {
            uint16_t wLockBytesPerPage = 0;
            wShift = 1U;
            wLockBytesPerPage = (uint16_t)(wShift << ((uint16_t)(pT2T->pLockCtrlTlv[wIndex].bBytesPerPage)));
            aTlv[(wTlvOffset++) % 8U] = (uint8_t)((uint16_t)(pT2T->pLockCtrlTlv[wIndex].wByteAddr /
                                            wLockBytesPerPage) << 4U) |
                                       (uint8_t)(pT2T->pLockCtrlTlv[wIndex].wByteAddr %
                                               wLockBytesPerPage);
        }

        aTlv[(wTlvOffset++) % 8U] = pT2T->pLockCtrlTlv[wIndex].bSizeInBits;
        aTlv[(wTlvOffset++) % 8U] = (pT2T->pLockCtrlTlv[wIndex].bBytesLockedPerBit << 4U) |
                                   pT2T->pLockCtrlTlv[wIndex].bBytesPerPage;

        /* Write lock TLVs */
        for(wWriteIndex = 0; wWriteIndex < 2U; wWriteIndex++)
        {
            /* Write byte block */
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                pT2T,
                (pT2T->pLockCtrlTlv[wIndex].wOffset + (wWriteIndex * 4U)),
                &aTlv[wWriteIndex * 4U]));
        }

         /* Copy bytes present is last block to beginning */
         (void)memcpy(aTlv, &aTlv[7U - (wTlvOffset % 4U)],(size_t)wTlvOffset % 4U);
    }

    /* Write memory control TLVs (if specified) */
    for(wIndex = 0; ((wIndex < pT2T->bMemoryTlvCount) && (wIndex < PHAL_TOP_T2T_MAX_MEM_CTRL_TLV)); wIndex++)
    {
        /* TLV offset from block start */
        pT2T->pMemCtrlTlv[wIndex].wOffset =  wTlvOffset;

        /* Create memory TLV */
        aTlv[(wTlvOffset++) % 8U] = 0x02;
        aTlv[(wTlvOffset++) % 8U] = 0x03;
        {
            uint16_t wBytesPerPage = 0;
            wShift = 1U;
            wBytesPerPage = (uint16_t)(wShift << ((uint16_t)(pT2T->pMemCtrlTlv[wIndex].bBytesPerPage)));
            aTlv[(wTlvOffset++) % 8U] = (uint8_t)((uint16_t)(pT2T->pMemCtrlTlv[wIndex].wByteAddr /
                                                    wBytesPerPage) << 4U) |
                                       (uint8_t)(pT2T->pMemCtrlTlv[wIndex].wByteAddr %
                                                   wBytesPerPage);
        }

        aTlv[(wTlvOffset++) % 8U] = pT2T->pMemCtrlTlv[wIndex].bSizeInBytes;
        aTlv[(wTlvOffset++) % 8U] = pT2T->pMemCtrlTlv[wIndex].bBytesPerPage;

        /* Write memory TLV */
        for(wWriteIndex = 0; wWriteIndex < 2U; wWriteIndex++)
        {
            /* Write block */
            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                pT2T,
                (pT2T->pMemCtrlTlv[wIndex].wOffset + (wWriteIndex * 4U)),
                &aTlv[wWriteIndex * 4U]));
        }
    }

    /* Update NDEF header address */
    if((pT2T->bLockTlvCount !=0U) && (pT2T->bMemoryTlvCount != 0U))
    {
        pT2T->wNdefHeaderAddr =
            ((pT2T->pLockCtrlTlv[pT2T->bLockTlvCount - 1U].wOffset >
            pT2T->pMemCtrlTlv[pT2T->bMemoryTlvCount - 1U].wOffset)?
            pT2T->pLockCtrlTlv[pT2T->bLockTlvCount - 1U].wOffset:
        pT2T->pMemCtrlTlv[pT2T->bMemoryTlvCount - 1U].wOffset) + 5U;
    }
    else if(pT2T->bLockTlvCount !=0U)
    {
        pT2T->wNdefHeaderAddr = pT2T->pLockCtrlTlv[pT2T->bLockTlvCount - 1U].wOffset + 5U;
    }
    else if(pT2T->bMemoryTlvCount != 0U)
    {
        pT2T->wNdefHeaderAddr = pT2T->pMemCtrlTlv[pT2T->bMemoryTlvCount - 1U].wOffset + 5U;
    }
    else
    {
        pT2T->wNdefHeaderAddr = 16;
    }

    /* Update the values for using WriteNdef API for writing default NDEF */
    pDataParams->bTagState = PHAL_TOP_STATE_INITIALIZED;
    pDataParams->dwNdefLength = 0;

    /* Update max. NDEF size */
    phalTop_Sw_Int_T2T_CalculateMaxNdefSize(pDataParams, pT2T);

    /* Write empty NDEF Message */
    status = phalTop_Sw_Int_T2T_WriteNdef(pDataParams, aNdefData, 3);

    if((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
    {
        /* Reset state on error */
        pDataParams->bTagState = PHAL_TOP_STATE_NONE;
        pT2T->wNdefHeaderAddr = 0x00;

        return status;
    }

    /* Update state and NDEF presence */
    pDataParams->bTagState = PHAL_TOP_STATE_READWRITE;
    pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 2U;
    pDataParams->dwNdefLength = 0x03;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T2T_Read(
                                   phalTop_T2T_t * pT2T,
                                   uint16_t wOffset,
                                   uint8_t * pData
                                   )
{
    phStatus_t PH_MEMLOC_REM status;

    /* Check if read offset is in current sector */
    if(pT2T->sSector.bAddress != (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE))
    {
        PH_CHECK_SUCCESS_FCT(status, phalMful_SectorSelect(
            pT2T->pAlT2TDataParams,
            (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE)));

        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
            pT2T,
            (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE)));
    }

    /* Read data */
    PH_CHECK_SUCCESS_FCT(status, phalMful_Read(
        pT2T->pAlT2TDataParams,
        (uint8_t)((wOffset % PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK),
        pData));

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T2T_Write(
                                    phalTop_T2T_t * pT2T,
                                    uint16_t wOffset,
                                    uint8_t * pData
                                    )
{
    phStatus_t PH_MEMLOC_REM status;

    /* Check if write offset is in current sector */
    if(pT2T->sSector.bAddress != (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE))
    {
        PH_CHECK_SUCCESS_FCT(status, phalMful_SectorSelect(
            pT2T->pAlT2TDataParams,
            (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE)));

        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
            pT2T,
            (uint8_t)(wOffset / PHAL_TOP_T2T_SECTOR_SIZE)));
    }

    /* Write data */
    PH_CHECK_SUCCESS_FCT(status, phalMful_Write(
        pT2T->pAlT2TDataParams,
        (uint8_t)((wOffset % PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK),
        pData));

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T2T_ReadNdef(
                                       phalTop_Sw_DataParams_t * pDataParams,
                                       uint8_t * pData,
                                       uint32_t * pLength
                                       )
{
    phStatus_t    PH_MEMLOC_REM   status;
    uint16_t      PH_MEMLOC_COUNT wIndex;
    uint32_t      PH_MEMLOC_COUNT wBytesRead;
    uint16_t      PH_MEMLOC_COUNT wCount;
    uint8_t       PH_MEMLOC_BUF   aData[16];
    phalTop_T2T_t PH_MEMLOC_REM * pT2T = &pDataParams->ualTop.salTop_T2T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT2T->pAlT2TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Reset NDEF length */
    *pLength = 0;

    /* Check if tag is in valid state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Check for NDEF length > 0 (in initialized state NDEF length is 0) */
    if(pDataParams->bTagState == PHAL_TOP_STATE_INITIALIZED)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_EMPTY_NDEF, PH_COMP_AL_TOP);
    }

    /* Buffer allocated by user is less than NDEF message size on Tag. */
    if (pDataParams->dwNdefLength > PH_NXPNFCRDLIB_CONFIG_MAX_NDEF_DATA)
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_BUFFER_OVERFLOW, PH_COMP_AL_TOP);
    }

    /* Copy the remaining NDEF data bytes apart from T and L bytes, which read during TLV detection in the Tag memory */
    if(pDataParams->dwNdefLength < pT2T->bDataIndex)
    {
        memcpy(pData, pT2T->bData, (uint8_t)pDataParams->dwNdefLength);
        wBytesRead = pDataParams->dwNdefLength;
    }
    else
    {
        memcpy(pData, pT2T->bData, pT2T->bDataIndex);
        wBytesRead = pT2T->bDataIndex;
    }

    /* Read NDEF data */
    wIndex = pT2T->wStartBlockNum * PHAL_TOP_T2T_BYTES_PER_BLOCK;
    while( wBytesRead < pDataParams->dwNdefLength)
    {
        /* Read 16 bytes */
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Read(
            pT2T,
            wIndex,
            aData));

        /* Copy valid data bytes */
        for(wCount = 0; ((wCount < 16U) && (wBytesRead < (uint16_t)(pDataParams->dwNdefLength))); wCount++)
        {
            if(0u != (phalTop_Sw_Int_T2T_CheckLockReservedOtp(pT2T, (((wIndex + wCount) % PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK))))
            {
                pData[wBytesRead++] = aData[wCount];
            }
        }

        wIndex = wIndex + 16U;
    }

    if ((wIndex / PHAL_TOP_T2T_SECTOR_SIZE) != 0U)
    {
        /* Reset Sector to 0 */
        PH_CHECK_SUCCESS_FCT(status, phalMful_SectorSelect(
            pT2T->pAlT2TDataParams,
            0U));
        pT2T->sSector.bAddress = 0U;
    }

    /* Update NDEF length */
    *pLength = wBytesRead;

    return PH_ERR_SUCCESS;
}

phStatus_t phalTop_Sw_Int_T2T_WriteNdef(
                                        phalTop_Sw_DataParams_t * pDataParams,
                                        uint8_t * pData,
                                        uint32_t dwLength
                                        )
{
    phStatus_t    PH_MEMLOC_REM   status;
    uint32_t      PH_MEMLOC_COUNT wIndex = 0;
    uint16_t      PH_MEMLOC_COUNT wTempIndex;
    uint8_t       PH_MEMLOC_BUF   aTempData[4];
    uint16_t      PH_MEMLOC_COUNT wCount = 0;
    uint16_t      PH_MEMLOC_REM   wOffset;
    uint16_t      PH_MEMLOC_REM   wTernimatorTlvLen = 0;
    phalTop_T2T_t PH_MEMLOC_REM * pT2T = &pDataParams->ualTop.salTop_T2T;

    if((pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U])  != NULL)
    {
        pT2T->pAlT2TDataParams = pDataParams->pTopTagsDataParams[pDataParams->bTagType - 1U];
    }
    else
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_USE_CONDITION, PH_COMP_AL_TOP);
    }

    /* Check if tag is in valid state */
    if(pDataParams->bTagState == PHAL_TOP_STATE_NONE)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_INVALID_STATE, PH_COMP_AL_TOP);
    }

    /* Check if tag is read-only */
    if(pDataParams->bTagState == PHAL_TOP_STATE_READONLY)
    {
        return PH_ADD_COMPCODE_FIXED(PHAL_TOP_ERR_READONLY_TAG, PH_COMP_AL_TOP);
    }

    /* Validate input parameters */
    if((dwLength > pDataParams->dwMaxNdefLength) || (dwLength == 0x00U))
    {
        return PH_ADD_COMPCODE_FIXED(PH_ERR_INVALID_PARAMETER, PH_COMP_AL_TOP);
    }

    /* NDEF TLV offset */
    wOffset = pT2T->wNdefHeaderAddr % PHAL_TOP_T2T_BYTES_PER_BLOCK;

    /* Set NDEF TLV length as 0 */
    pT2T->bNdefHeaderBlock[wOffset++] = PHAL_TOP_T2T_NDEF_TLV;
    pT2T->bNdefHeaderBlock[wOffset++] = 0x00;

    /* Calculate NDEF message address */
    if(dwLength > 0xFEU)
    {
        pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 4U;
        wOffset += 2U;
    }
    else
    {
        pT2T->wNdefMsgAddr = pT2T->wNdefHeaderAddr + 2U;
    }

    /* Update part of NDEF message in first block */
    wCount = 0;
    for( ; ((wOffset % PHAL_TOP_T2T_BYTES_PER_BLOCK) != 0U); wOffset++)
    {
        pT2T->bNdefHeaderBlock[wOffset] = pData[wCount];
        pT2T->bData[wCount] = pData[wCount];
        wCount++;
    }

    /* Ndef Header lies at the last byte and thus entire length info will lie in next block */
    if(wOffset == PHAL_TOP_T2T_BYTES_PER_BLOCK - 1)
    {
        /* Write the first block with NDEF TLV */
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
            pT2T,
            (pT2T->wNdefHeaderAddr + PHAL_TOP_T2T_BYTES_PER_BLOCK),
            &pT2T->bNdefHeaderBlock[4]));
    }
    else
    {
        /* Write the first block with NDEF TLV */
        PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
            pT2T,
            pT2T->wNdefHeaderAddr,
            &pT2T->bNdefHeaderBlock[0]));
    }

    /* Check if terminator TLV is needed/possible */
    if(dwLength < pDataParams->dwMaxNdefLength)
    {
        wTernimatorTlvLen = 1;
    }

    /* Write NDEF data */
    wIndex = (pT2T->wNdefMsgAddr + wCount);
    wTempIndex = wCount;
    while( wTempIndex < (dwLength + wTernimatorTlvLen))
    {
        /* Check if write offset is in current sector */
        if(pT2T->sSector.bAddress != (uint8_t)(wIndex / PHAL_TOP_T2T_SECTOR_SIZE))
        {
            PH_CHECK_SUCCESS_FCT(status, phalMful_SectorSelect(
                pT2T->pAlT2TDataParams,
                (uint8_t)(wIndex / PHAL_TOP_T2T_SECTOR_SIZE)));

            PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
                pT2T,
                (uint8_t)(wIndex / PHAL_TOP_T2T_SECTOR_SIZE)));
        }

        if(0U != (phalTop_Sw_Int_T2T_CheckLockReservedOtp(pT2T, ((uint16_t)wIndex % PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK)))
        {
            if((dwLength - wTempIndex) >= PHAL_TOP_T2T_BYTES_PER_BLOCK)
            {
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                    pT2T,
                    (uint16_t)wIndex,
                    &pData[wTempIndex]));

                wTempIndex += 4U;
            }
            /* Write last partially filled block */
            else
            {
                (void)memset(aTempData, 0, 4);
                wOffset = 0;
                for(; dwLength > wTempIndex; wTempIndex++)
                {
                    aTempData[wOffset] = pData[wTempIndex];
                    wOffset++;
                }

                /* Set terminator TLV */
                aTempData[wOffset] = 0xFE;

                /* Write last partially filled block */
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                    pT2T,
                    (uint16_t)wIndex,
                    aTempData));

                wTempIndex++;
            }
        }
        wIndex += 4U;
    }

    wOffset = pT2T->wNdefHeaderAddr % PHAL_TOP_T2T_BYTES_PER_BLOCK;

    /* If length more than 254, use 3 byte length format */
    if(dwLength > 0xFEU)
    {
        pT2T->bNdefHeaderBlock[wOffset] = PHAL_TOP_T2T_NDEF_TLV;
        pT2T->bNdefHeaderBlock[++wOffset] = 0xFF;
        pT2T->bNdefHeaderBlock[++wOffset] = (uint8_t)(dwLength >> 8U);
        pT2T->bNdefHeaderBlock[++wOffset] = (uint8_t)dwLength;
    }
    else
    {
        pT2T->bNdefHeaderBlock[wOffset] = PHAL_TOP_T2T_NDEF_TLV;
        pT2T->bNdefHeaderBlock[++wOffset] = (uint8_t)dwLength;
    }

    /* Write the first block with NDEF TLV */
    PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
        pT2T,
        pT2T->wNdefHeaderAddr,
        pT2T->bNdefHeaderBlock));

    /* Write second block */
    if(wOffset > PHAL_TOP_T2T_BYTES_PER_BLOCK)
    {
        for(wIndex = (pT2T->wNdefHeaderAddr + PHAL_TOP_T2T_BYTES_PER_BLOCK);
                wIndex < pDataParams->dwMaxNdefLength;
                wIndex += 4U)
        {
            /* Check for reserved/lock blocks */
            if(0U != (phalTop_Sw_Int_T2T_CheckLockReservedOtp(pT2T, (wIndex / 4U))))
            {
                /* Write the next block */
                PH_CHECK_SUCCESS_FCT(status, phalTop_Sw_Int_T2T_Write(
                    pT2T->pAlT2TDataParams,
                    (uint16_t)wIndex,
                    &pT2T->bNdefHeaderBlock[4]));
                break;
            }
        }
    }

    /* Update NDEF Length */
    pDataParams->dwMaxNdefLength = dwLength;
    pDataParams->bTagState = PHAL_TOP_STATE_READWRITE;

    return PH_ERR_SUCCESS;
}

uint8_t phalTop_Sw_Int_T2T_CheckLockReservedOtp(
                                                phalTop_T2T_t * pT2T,
                                                uint16_t wIndex
                                                )
{
    if(0u != ((pT2T->sSector.bLockReservedOtp[wIndex / 8U]) & (uint8_t)(1U << (wIndex % 8U))))
    {
        /* Lock/Reserved/OTP block */
        return 0x00;
    }
    else
    {
        /* Data block */
        return 0x01;
    }
}

void phalTop_Sw_Int_T2T_CalculateMaxNdefSize(
                                             phalTop_Sw_DataParams_t * pDataParams,
                                             phalTop_T2T_t * pT2T
                                             )
{
    uint16_t PH_MEMLOC_COUNT wIndex;
    uint16_t PH_MEMLOC_REM   wTagSize;

    if(pT2T->bTagMemoryType == PHAL_TOP_T2T_TAG_MEMORY_TYPE_STATIC)
    {
        pDataParams->dwMaxNdefLength = 64U - pT2T->wNdefMsgAddr;
    }
    else
    {
        /* Reset max. NDEF length */
        pDataParams->dwMaxNdefLength = 0;

        /* Calculate total tag size including lock and reserved bytes */
        wTagSize = (uint16_t) ((((uint16_t)(pT2T->bTms)) * 8U) + /* */ 16U);

        /* Add lock TLV length */
        for(wIndex = 0; wIndex < pT2T->bLockTlvCount; wIndex++)
        {
            wTagSize += (((uint16_t)pT2T->pLockCtrlTlv[wIndex].bSizeInBits) / 8U);

            if(0u != ((pT2T->pLockCtrlTlv[wIndex].bSizeInBits % 8U)))
            {
                wTagSize++;
            }
        }

        /* Add memory TLV length */
        for(wIndex = 0; wIndex < pT2T->bMemoryTlvCount; wIndex++)
        {
            wTagSize += pT2T->pMemCtrlTlv[wIndex].bSizeInBytes;
        }

        /* Calculate max. NDEF TLV length */
        for(wIndex = pT2T->wNdefHeaderAddr; wIndex < wTagSize; wIndex++)
        {
            /* Update Lock/Reserved/OTP block status when sector changes */
            if((wIndex % PHAL_TOP_T2T_SECTOR_SIZE) == 0U)
            {
                (void)phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
                    pT2T,
                    (uint8_t)(wIndex / PHAL_TOP_T2T_SECTOR_SIZE));
            }

            /* Increment max. NDEF length if not lock/OTP/reserved byte */
            if(0U != (phalTop_Sw_Int_T2T_CheckLockReservedOtp(pT2T, ((wIndex % PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK))))
            {
                pDataParams->dwMaxNdefLength++;
            }
        }

        /* Subtract T and L field length to get NDEF message (V field) length */
        if(pDataParams->dwMaxNdefLength < 0xFFU)
        {
            pDataParams->dwMaxNdefLength -= 2u;
        }
        else
        {
            pDataParams->dwMaxNdefLength -= 4u;
        }
    }
}

phStatus_t phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
                                                    phalTop_T2T_t * pT2T,
                                                    uint8_t bSector
                                                    )
{
    uint16_t PH_MEMLOC_REM   wOffset;
    uint16_t PH_MEMLOC_REM   wSectorOffset;
    uint16_t PH_MEMLOC_REM   wBlockOffset;
    uint16_t PH_MEMLOC_COUNT wIndex;
    uint16_t PH_MEMLOC_COUNT wBlockCount;

    /* Update sector */
    pT2T->sSector.bAddress = bSector;

    if(pT2T->sSector.bAddress == 0U)
    {
        /* First 3 blocks are internal/lock for sector 0 */
        pT2T->sSector.bLockReservedOtp[0x00] = 0x07;
    }
    else
    {
        /* Reset the buffer for sector other than 0 */
        pT2T->sSector.bLockReservedOtp[0x00] = 0x00;
    }

    if(pT2T->bTagMemoryType == PHAL_TOP_T2T_TAG_MEMORY_TYPE_DYNAMIC)
    {
        /* Update Lock Bytes */
        for(wIndex = 0; wIndex < pT2T->bLockTlvCount; wIndex++)
        {
            if((pT2T->pLockCtrlTlv[wIndex].wByteAddr / PHAL_TOP_T2T_SECTOR_SIZE) == pT2T->sSector.bAddress)
            {
                wSectorOffset = (pT2T->pLockCtrlTlv[wIndex].wByteAddr %
                                PHAL_TOP_T2T_SECTOR_SIZE) / PHAL_TOP_T2T_BYTES_PER_BLOCK;

                if(0U != ((pT2T->pLockCtrlTlv[wIndex].bSizeInBits / 8U) % PHAL_TOP_T2T_BYTES_PER_BLOCK))
                {
                    wBlockOffset = 1;
                }
                else
                {
                    wBlockOffset = 0;
                }

                for(wOffset = wSectorOffset;
                    wOffset < (wSectorOffset +
                    ((((uint16_t)(pT2T->pLockCtrlTlv[wIndex].bSizeInBits)) / 8U) / PHAL_TOP_T2T_BYTES_PER_BLOCK) +
                    wBlockOffset);
                    wOffset++)
                {
                    pT2T->sSector.bLockReservedOtp[wOffset / 8U] |= (uint8_t)(1U << (wOffset % 8U));
                }
            }
        }
        /* Update Memory Bytes */
        for(wIndex = 0; wIndex < pT2T->bMemoryTlvCount; wIndex++)
        {
            if((pT2T->pMemCtrlTlv[wIndex].wByteAddr / PHAL_TOP_T2T_SECTOR_SIZE) == pT2T->sSector.bAddress)
            {
                wSectorOffset = (pT2T->pMemCtrlTlv[wIndex].wByteAddr % PHAL_TOP_T2T_SECTOR_SIZE) /
                                PHAL_TOP_T2T_BYTES_PER_BLOCK;

                if(0u != (((pT2T->pMemCtrlTlv[wIndex].bSizeInBytes) % PHAL_TOP_T2T_BYTES_PER_BLOCK)))
                {
                    wBlockOffset = 1;
                }
                else
                {
                    wBlockOffset = 0;
                }

                wBlockCount = (((uint16_t)(pT2T->pMemCtrlTlv[wIndex].bSizeInBytes)) / PHAL_TOP_T2T_BYTES_PER_BLOCK);
                for(wOffset = wSectorOffset;
                    wOffset < (wSectorOffset +
                    wBlockCount +
                    wBlockOffset);
                    wOffset++)
                {
                    pT2T->sSector.bLockReservedOtp[wOffset / 8U] |= (uint8_t)(1U << (wOffset % 8U));
                }
            }
        }
    }
    return PH_ERR_SUCCESS;
}

#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#endif /* NXPBUILD__PHAL_TOP_SW */
