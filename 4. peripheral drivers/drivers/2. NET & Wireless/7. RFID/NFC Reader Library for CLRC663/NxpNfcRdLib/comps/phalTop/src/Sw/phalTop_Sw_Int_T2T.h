/*----------------------------------------------------------------------------*/
/* Copyright 2016-2022 NXP                                                    */
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

#ifndef PHALTOP_SW_INT_T2T_H
#define PHALTOP_SW_INT_T2T_H

#ifdef NXPBUILD__PHAL_TOP_SW
#ifdef NXPBUILD__PHAL_TOP_T2T_SW

#define PHAL_TOP_T2T_SECTOR_SIZE                 0x400U       /**< T2T Segment size */
#define PHAL_TOP_T2T_BLOCKS_PER_SECTOR           0xFFU        /**< Number of blocks per sector for T2T */
#define PHAL_TOP_T2T_NDEF_MAGIC_NUMBER           0xE1U        /**< NDEF Magical Number for T2T */

#define PHAL_TOP_T2T_CC_LENGTH                   0x04U        /**< Length of T2T CC bytes */
#define PHAL_TOP_T2T_CC_BLOCK                    0x03U        /**< CC block number */
#define PHAL_TOP_T2T_STATIC_LOCK_BLOCK           0x02U        /**< Static lock block number */
#define PHAL_TOP_T2T_BYTES_PER_BLOCK             0x04U        /**< Number of bytes per block */
#define PHAL_TOP_T2T_NDEF_TLV_HEADER_LEN         0x01U        /**< NDEF TLV header(T field) length */

#define PHAL_TOP_T2T_NULL_TLV                    0x00U        /**< NULL TLV. */
#define PHAL_TOP_T2T_LOCK_CTRL_TLV               0x01U        /**< Lock Control TLV. */
#define PHAL_TOP_T2T_MEMORY_CTRL_TLV             0x02U        /**< Memory Control TLV. */
#define PHAL_TOP_T2T_NDEF_TLV                    0x03U        /**< NDEF Message TLV. */
#define PHAL_TOP_T2T_PROPRIETARY_TLV             0xFDU        /**< Proprietary TLV. */
#define PHAL_TOP_T2T_TERMINATOR_TLV              0xFEU        /**< Terminator TLV. */

#define PHAL_TOP_T2T_CC_RWA_RW                   0x00U        /**< Read/Write access */
#define PHAL_TOP_T2T_CC_RWA_RO                   0x0FU        /**< Read Only access */

/**< Get length of TLV length field */
#define PHAL_TOP_T2T_GET_TLV_LEN_BYTES(len)      ((len) > 0xFE? 3: 1)

void phalTop_Sw_Int_T2T_CalculateMaxNdefSize(
                                             phalTop_Sw_DataParams_t * pDataParams,
                                             phalTop_T2T_t * pT2T
                                             );

phStatus_t phalTop_Sw_Int_T2T_UpdateLockReservedOtp(
                                                    phalTop_T2T_t * pT2T,
                                                    uint8_t bSector
                                                    );

uint8_t phalTop_Sw_Int_T2T_CheckLockReservedOtp(
                                                phalTop_T2T_t * pT2T,
                                                uint16_t wIndex
                                                );

phStatus_t phalTop_Sw_Int_T2T_DetectTlvBlocks(
                                              phalTop_Sw_DataParams_t * pDataParams,
                                              phalTop_T2T_t * pT2T
                                              );

phStatus_t phalTop_Sw_Int_T2T_CheckNdef(
                                        phalTop_Sw_DataParams_t * pDataParams,
                                        uint8_t * pTagState
                                        );

phStatus_t phalTop_Sw_Int_T2T_FormatNdef(
                                         phalTop_Sw_DataParams_t * pDataParams
                                         );

phStatus_t phalTop_Sw_Int_T2T_EraseNdef(
                                        phalTop_Sw_DataParams_t * pDataParams
                                        );

phStatus_t phalTop_Sw_Int_T2T_Read(
                                   phalTop_T2T_t * pT2T,
                                   uint16_t wOffset,
                                   uint8_t * pData
                                   );

phStatus_t phalTop_Sw_Int_T2T_Write(
                                    phalTop_T2T_t * pT2T,
                                    uint16_t wOffset,
                                    uint8_t * pData
                                    );

phStatus_t phalTop_Sw_Int_T2T_ReadNdef(
                                       phalTop_Sw_DataParams_t * pDataParams,
                                       uint8_t * pData,
                                       uint32_t * pLength
                                       );

phStatus_t phalTop_Sw_Int_T2T_WriteNdef(
                                        phalTop_Sw_DataParams_t * pDataParams,
                                        uint8_t * pData,
                                        uint32_t dwLength
                                        );

phStatus_t phalTop_Sw_Int_T2T_SetReadOnly(
                                          phalTop_Sw_DataParams_t * pDataParams
                                          );

phStatus_t phalTop_Sw_Int_T2T_ClearState(
                                         phalTop_Sw_DataParams_t * pDataParams,
                                         phalTop_T2T_t * pT2T
                                         );

#endif /* NXPBUILD__PHAL_TOP_T2T_SW */
#endif /* NXPBUILD__PHAL_TOP_SW */
#endif /* PHALTOP_SW_INT_T2T_H */
