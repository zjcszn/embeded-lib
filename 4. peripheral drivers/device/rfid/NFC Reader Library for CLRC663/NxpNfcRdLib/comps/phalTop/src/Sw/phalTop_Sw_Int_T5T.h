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


#ifndef PHALTOP_SW_INT_T5T_H
#define PHALTOP_SW_INT_T5T_H

#ifdef NXPBUILD__PHAL_TOP_SW
#ifdef NXPBUILD__PHAL_TOP_T5T_SW

#define PHAL_TOP_T5T_CC_LENGTH                   0x04U        /**< Length of T5T CC bytes */
#define PHAL_TOP_T5T_CC_BLOCK                    0x00U        /**< CC block number */
#define PHAL_TOP_T5T_BYTES_PER_BLOCK_DEFAULT     0x04U        /**< Number of bytes per block */
#define PHAL_TOP_T5T_MAXIMUM_BLOCK_SIZE          0x20U        /**< Maximum no of bytes in a block */
#define PHAL_TOP_T5T_DEFAULT_BLOCK_NUMBER        0xFFU        /**< Default block number (255) */
#define PHAL_TOP_T5T_MIMIMUM_NO_OF_BLOCK         0x01U        /**< Minimum No of blocks */
#define PHAL_TOP_T5T_MASK_HIGHER_NIBBLE          0xF0U        /**< Higher Nibble Mask  */
#define PHAL_TOP_T5T_READ_WRITE_ACCESS_BYTE      0x03U        /**< Read/Write Access Byte */

#define PHAL_TOP_T5T_NDEF_MAGIC_NUMBER           0xE1U        /**< NDEF Magical Number for T5T for 1-Byte Addressed Commands */
#define PHAL_TOP_T5T_NDEF_MAGIC_NUMBER_2         0xE2U        /**< NDEF Magical Number for T5T for 2-Byte Addressed Commands */

#define PHAL_TOP_T5T_NDEF_TLV                    0x03U        /**< NDEF Message TLV. */
#define PHAL_TOP_T5T_PROPRIETARY_TLV             0xFDU        /**< Proprietary TLV. */
#define PHAL_TOP_T5T_TERMINATOR_TLV              0xFEU        /**< Terminator TLV. */

#define PHAL_TOP_T5T_CC_RWA_RW                   0x00U        /**< Read/Write access */
#define PHAL_TOP_T5T_CC_RWA_RO                   0x03U        /**< Read Only access */

#define PHAL_TOP_T5T_MAJOR_VERSION               0x01U        /**< Major version is 1.x */

phStatus_t phalTop_Sw_Int_T5T_SetReadOnly(
                                          phalTop_Sw_DataParams_t * pDataParams
                                          );

phStatus_t phalTop_Sw_Int_LockBlock(
                                    phalTop_Sw_DataParams_t * pDataParams,
                                    uint16_t wBlockNum
                                    );

phStatus_t phalTop_Sw_Int_T5T_Read(
                                   phalTop_T5T_t * pT5T,
                                   uint16_t wBlockNum,
                                   uint16_t wNumberOfBlocks,
                                   uint8_t ** pRxBuffer,
                                   uint16_t * wLength
                                   );

phStatus_t phalTop_Sw_Int_T5T_Write(
                                    phalTop_T5T_t * pT5T,
                                    uint16_t wBlockNum,
                                    uint8_t * pRxBuffer,
                                    uint16_t wLength
                                    );

phStatus_t phalTop_Sw_Int_T5T_ClearState(
                                         phalTop_Sw_DataParams_t * pDataParams,
                                         phalTop_T5T_t * pT5T
                                         );

phStatus_t phalTop_Sw_Int_T5T_DetectTlvBlocks(
                                              phalTop_Sw_DataParams_t * pDataParams,
                                              phalTop_T5T_t * pT5T
                                              );

phStatus_t phalTop_Sw_Int_T5T_CheckNdef(
                                        phalTop_Sw_DataParams_t * pDataParams,
                                        uint8_t * pTagState
                                        );

phStatus_t phalTop_Sw_Int_T5T_EraseNdef(
                                        phalTop_Sw_DataParams_t * pDataParams
                                        );

phStatus_t phalTop_Sw_Int_T5T_FormatNdef(
                                         phalTop_Sw_DataParams_t * pDataParams
                                         );

phStatus_t phalTop_Sw_Int_T5T_ReadNdef(
                                       phalTop_Sw_DataParams_t * pDataParams,
                                       uint8_t * pData,
                                       uint32_t * pLength
                                       );

phStatus_t phalTop_Sw_Int_T5T_WriteNdef(
                                        phalTop_Sw_DataParams_t * pDataParams,
                                        uint8_t * pData,
                                        uint32_t dwLength
                                        );

phStatus_t phalTop_Sw_Int_T5T_CalculateTLVLength(
                                                 phalTop_T5T_t * pT5T,  /* [In] Pointer to a T5T component context. */
                                                 uint16_t wIndex,       /* [In] Index is the byte location where the tag value of the TLV is found */
                                                 uint8_t *pLength,      /* [Out] Length of the NDEF data as contained in 'L' field */
                                                 uint8_t *pData,        /* [In] Data read from the tag. */
                                                 uint16_t wBlockNum,    /* [In] Block num to read */
                                                 uint8_t *pNextBlockRead
                                                 );

#endif /* NXPBUILD__PHAL_TOP_T5T_SW */
#endif /* NXPBUILD__PHAL_TOP_SW */

#endif /* PHALTOP_SW_INT_T5T_H */
