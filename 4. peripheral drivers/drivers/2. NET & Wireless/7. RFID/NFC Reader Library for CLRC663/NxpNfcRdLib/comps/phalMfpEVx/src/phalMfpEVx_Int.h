/*----------------------------------------------------------------------------*/
/* Copyright 2013 - 2018, 2022 NXP                                            */
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
* Internal functions of Software implementation of MIFARE (R) application layer.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 6138 $ (v07.09.00)
* $Date: 2020-05-25 23:21:48 +0530 (Mon, 25 May 2020) $
*
* History:
*  Kumar GVS: Generated 15. Apr 2013
*
*/


#ifndef PHALMFPEVX_INT_H
#define PHALMFPEVX_INT_H

#include <ph_Status.h>

/* MIFARE Plus EVx contactless IC Commands in MIFARE Plus contactless IC Mode */
#define PHAL_MFPEVX_CMD_AUTH1_FIRST                             0x70U   /* MFP EVx Authenticate command (part 1, first authentication). */
#define PHAL_MFPEVX_CMD_AUTH_FIRST_ALTERNATE                    0x73U   /* MFP EVx Authenticate command (part 1, first authentication, Alternate Authenticate). */
#define PHAL_MFPEVX_CMD_AUTH1                                   0x76U   /* MFP EVx Authenticate command (part 1, non-first authentication). */
#define PHAL_MFPEVX_CMD_AUTH2                                   0x72U   /* MFP EVx Authenticate command (part 2). */
#define PHAL_MFPEVX_CMD_AUTH3                                   0x74U   /* MFP EVx Authenticate command (part 3). */
#define PHAL_MFPEVX_CMD_RAUTH                                   0x78U   /* MFP EVx ResetAuth command. */
#define PHAL_MFPEVX_CMD_READ_ENM                                0x30U   /* MFP EVx Read Encrypted, No mac on response, Mac on command. */
#define PHAL_MFPEVX_CMD_READ_EMM                                0x31U   /* MFP EVx Read Encrypted, Mac on response, Mac on command. */
#define PHAL_MFPEVX_CMD_READ_PNM                                0x32U   /* MFP EVx Read Plain, No mac on response, Mac on command. */
#define PHAL_MFPEVX_CMD_READ_PMM                                0x33U   /* MFP EVx Read Plain, Mac on response, Mac on command. */
#define PHAL_MFPEVX_CMD_READ_ENU                                0x34U   /* MFP EVx Read Encrypted, No mac on response, Unmac'ed command. */
#define PHAL_MFPEVX_CMD_READ_EMU                                0x35U   /* MFP EVx Read Encrypted, Mac on response, Unmac'ed command. */
#define PHAL_MFPEVX_CMD_READ_PNU                                0x36U   /* MFP EVx Read Plain, No mac on response, Unmac'ed command. */
#define PHAL_MFPEVX_CMD_READ_PMU                                0x37U   /* MFP EVx Read Plain, Mac on response, Unmac'ed command. */
#define PHAL_MFPEVX_CMD_WRITE_EN                                0xA0U   /* MFP EVx Write Encrypted, No mac on response, (Mac on command). */
#define PHAL_MFPEVX_CMD_WRITE_EM                                0xA1U   /* MFP EVx Write Encrypted, Mac on response, (Mac on command). */
#define PHAL_MFPEVX_CMD_WRITE_PN                                0xA2U   /* MFP EVx Write Plain, No mac on response, (Mac on command). */
#define PHAL_MFPEVX_CMD_WRITE_PM                                0xA3U   /* MFP EVx Write Plain, Mac on response, (Mac on command). */
#define PHAL_MFPEVX_CMD_WRITEPERSO                              0xA8U   /* MFP EVx Write Perso. */
#define PHAL_MFPEVX_CMD_COMMITPERSO                             0xAAU   /* MFP EVx Commit Perso. */
#define PHAL_MFPEVX_CMD_INCR                                    0xB0U   /* MFP EVx Increment command. */
#define PHAL_MFPEVX_CMD_INCR_M                                  0xB1U   /* MFP EVx Increment command MACed. */
#define PHAL_MFPEVX_CMD_DECR                                    0xB2U   /* MFP EVx Decrement command. */
#define PHAL_MFPEVX_CMD_DECR_M                                  0xB3U   /* MFP EVx Decrement command MACed. */
#define PHAL_MFPEVX_CMD_TRANS                                   0xB4U   /* MFP EVx Transfer command. */
#define PHAL_MFPEVX_CMD_TRANS_M                                 0xB5U   /* MFP EVx Transfer command MACed. */
#define PHAL_MFPEVX_CMD_INCRTR                                  0xB6U   /* MFP EVx Increment Transfer command. */
#define PHAL_MFPEVX_CMD_INCRTR_M                                0xB7U   /* MFP EVx Increment Transfer command MACed. */
#define PHAL_MFPEVX_CMD_DECRTR                                  0xB8U   /* MFP EVx Decrement Transfer command. */
#define PHAL_MFPEVX_CMD_DECRTR_M                                0xB9U   /* MFP EVx Decrement Transfer command MACed. */
#define PHAL_MFPEVX_CMD_REST                                    0xC2U   /* MFP EVx Restore command. */
#define PHAL_MFPEVX_CMD_REST_M                                  0xC3U   /* MFP EVx Restore command MACed. */
#define PHAL_MFPEVX_CMD_PPC                                     0xF0U   /* MFP EVx Prepare Proximity Check. */
#define PHAL_MFPEVX_CMD_PC                                      0xF2U   /* MFP EVx Proximity Check command. */
#define PHAL_MFPEVX_CMD_VPC                                     0xFDU   /* MFP EVx Verify Proximity Check command. */
#define PHAL_MFPEVX_CMD_PERSOUID                                0x40U   /* MFP EVx Personalize UID command */
#define PHAL_MFPEVX_CMD_VCLAST_ISOL3                            0x4BU   /* VCA Virtual Card Support Last command code. */

/** MIFARE Plus EVx Commands in MIFARE Mode */
#define PHAL_MFPEVX_CMD_MFC_RESTORE                             0xC2U   /* MFP EVx Restore command in MIFARE Classic contactless IC mode. */
#define PHAL_MFPEVX_CMD_MFC_INCREMENT                           0xC1U   /* MFP EVx Increment command in MIFARE Classic mode. */
#define PHAL_MFPEVX_CMD_MFC_DECREMENT                           0xC0U   /* MFP EVx Decrement command in MIFARE Classic mode. */
#define PHAL_MFPEVX_CMD_MFC_TRANSFER                            0xB0U   /* MFP EVx Transfer command in MIFARE Classic mode. */
#define PHAL_MFPEVX_CMD_MFC_READ                                0x30U   /* MFP EVx Read command in MIFARE Classic mode. */
#define PHAL_MFPEVX_CMD_MFC_WRITE                               0xA0U   /* MFP EVx Write command in MIFARE Classic mode. */

/* MIFARE Plus EVx Special Commands */
#define PHAL_MFPEVX_CMD_GET_VERSION                             0x60U   /* MFP EVx Get Version cmd. */
#define PHAL_MFPEVX_CMD_COMMIT_READER_ID                        0xC8U   /* MFP EVx Commit Reader ID cmd. */
#define PHAL_MFPEVX_CMD_SSAUTH                                  0x7AU   /* MFP EVx Sector Switch Authentication cmd. */
#define PHAL_MFPEVX_CMD_SSAUTHC                                 0x72U   /* MFP EVx Sector Switch Authentication Continue cmd. */
#define PHAL_MFPEVX_CMD_AUTH_PDC                                0x7CU   /* MFP EVx Post Delivery Command. */
#define PHAL_MFPEVX_CMD_READ_SIG                                0x3CU   /* MFP EVx Verify read signature command. */
#define PHAL_MFPEVX_CMD_SET_CONFIG_SL1                          0x44U   /* MFP EVx SetConfigSL1 command. */

/* MIFARE Plus EVx options Diversification. */
#define PHAL_MFPEVX_NO_DIVERSIFICATION                          0xFFFFU /* No diversification. */

#define PHAL_MFPEVX_TRUNCATED_MAC_SIZE                          8U      /* Size of the truncated MAC. */
#define PHAL_MFPEVX_PC_RND_LEN                                  7U      /* Size of the Proximity Check Random numbers. */
#define PHAL_MFPEVX_SIG_LENGTH                                  0x38U   /* NXP Originality Signature length */
#define PHAL_MFPEVX_SIG_LENGTH_ENC                              0x40U   /* NXP Originality Signature length */

/* MIFARE Plus EVx ISO 7816-4 header values */
#define PHAL_MFPEVX_WRAP_HDR_LEN                                0x04U   /* Wrapped APDU header length */
#define PHAL_MFPEVX_WRAPPEDAPDU_CLA                             0x90U   /* Wrapped APDU default class. */
#define PHAL_MFPEVX_WRAPPEDAPDU_P1                              0x00U   /* Wrapped APDU default P1. */
#define PHAL_MFPEVX_WRAPPEDAPDU_P2                              0x00U   /* Wrapped APDU default P2. */

/* Block definitions */
#define PHAL_MFPEVX_DATA_BLOCK_SIZE                             16U     /* Length of MFP EVx data block. */
#define PHAL_MFPEVX_VALUE_BLOCK_SIZE                            4U      /* Length of MFP EVx value block. */
#define PHAL_MFPEVX_MAX_WRITE_BLOCK (PHAL_MFPEVX_DATA_BLOCK_SIZE * 15)  /* Maximum blocks that can be written. */

/* MIFARE Plus EVx Secure messaging flags. */
#define PHAL_MFPEVX_SECURE_MESSAGE_EV0                          0x00U   /* EV0 Secure messaging.*/
#define PHAL_MFPEVX_SECURE_MESSAGE_EV1                          0x01U   /* EV0 Secure messaging.*/

/* MIFARE Plus EVx supported UID size. */
#define PHAL_MFPEVX_UID_LENGTH_4B                               0x04U   /* UID length of 4 bytes. */
#define PHAL_MFPEVX_UID_LENGTH_7B                               0x07U   /* UID length of 7 bytes. */
#define PHAL_MFPEVX_UID_LENGTH_10B                              0x0AU   /* UID length of 10 bytes. */

phStatus_t phalMfpEVx_Int_ComputeErrorResponse(uint16_t wNumBytesReceived, uint16_t wStatus, uint8_t bLayer4Comm);

phStatus_t phalMfpEVx_Int_ComputeErrorResponseMfc(uint16_t wNumBytesReceived, uint8_t bStatus);

phStatus_t phalMfpEVx_Int_WritePerso(void * pPalMifareDataParams, uint8_t bLayer4Comm, uint8_t bWrappedMode, uint8_t bExtendedLenApdu,
    uint16_t wBlockNr, uint8_t bNumBlocks, uint8_t * pValue);

phStatus_t phalMfpEVx_Int_CommitPerso(void * pPalMifareDataParams, uint8_t bOption, uint8_t bLayer4Comm, uint8_t bWrappedMode,
    uint8_t bExtendedLenApdu);

phStatus_t phalMfpEVx_Int_ResetAuth(void * pPalMifareDataParams, uint8_t bWrappedMode, uint8_t bExtendedLenApdu);

phStatus_t phalMfpEVx_Int_PersonalizeUid(void * pPalMifareDataParams, uint8_t bUidType);

phStatus_t phalMfpEVx_Int_SetConfigSL1(void * pPalMifareDataParams, uint8_t bOption);

phStatus_t phalMfpEVx_Int_ReadSL1TMBlock(void * pPalMifareDataParams, uint16_t wBlockNr, uint8_t * pBlocks);

phStatus_t phalMfpEVx_Int_VCSupportLastISOL3(void * pPalMifareDataParams, uint8_t * pIid, uint8_t * pPcdCapL3, uint8_t * pInfo);

phStatus_t phalMfpEVx_Int_CreateValueBlock(uint8_t * pValue, uint8_t bAddrData, uint8_t * pBlock);

phStatus_t phalMfpEVx_Int_CheckValueBlockFormat(uint8_t * pBlock);

phStatus_t phalMfpEVx_Int_Send7816Apdu(void * pPalMifareDataParams, uint16_t wOptions, uint16_t wLc, uint8_t bExtendedLenApdu,
    uint8_t * pData, uint16_t wDataLen, uint8_t ** ppRxBuffer, uint16_t * pRxBufLen);

#endif /* PHALMFPEVX_INT_H */
