/*----------------------------------------------------------------------------*/
/* Copyright 2009-2020 NXP                                                    */
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
* Internal functions of Software implementation of MIFARE (R) Ultralight contactless IC application layer.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
* History:
*  CHu: Generated 05. October 2009
*
*/

#ifndef PHALMFUL_INT_H
#define PHALMFUL_INT_H

#include <ph_Status.h>

/** \addtogroup ph_Private
* @{
*/

/** \name MIFARE Ultralight contactless IC error codes. */
/** @{ */
#define PHAL_MFUL_RESP_ACK      0x0AU   /**< MIFARE Ultralight ACK response code */
#define PHAL_MFUL_RESP_NAK0     0x00U   /**< MIFARE Ultralight NAK0 response code */
#define PHAL_MFUL_RESP_NAK1     0x01U   /**< MIFARE Ultralight NAK1 response code */
#define PHAL_MFUL_RESP_NAK4     0x04U   /**< MIFARE Ultralight NAK4 response code */
#define PHAL_MFUL_RESP_NAK5     0x05U   /**< MIFARE Ultralight NAK5 response code */
/** @} */

/** \name MIFARE Ultralight error codes. */
/** @{ */
#define PHAL_MFUL_CMD_READ          0x30U   /**< MIFARE Ultralight Read command byte */
#define PHAL_MFUL_CMD_COMPWRITE     0xA0U   /**< MIFARE Ultralight Compatibility Write command byte */
#define PHAL_MFUL_CMD_WRITE         0xA2U   /**< MIFARE Ultralight Write command byte */
#define PHAL_MFUL_CMD_FAST_WRITE    0xA6U   /**< MIFARE Ultralight Write command byte */
#define PHAL_MFUL_CMD_AUTH          0x1AU   /**< MIFARE Ultralight Authenticate command byte */
#define PHAL_MFUL_CMD_INCR_CNT      0xA5U   /**< MIFARE Ultralight Increment count command byte */
#define PHAL_MFUL_CMD_READ_CNT      0x39U   /**< MIFARE Ultralight Read counter command byte */
#define PHAL_MFUL_CMD_PWD_AUTH      0x1BU   /**< MIFARE Ultralight Password Auth command byte */
#define PHAL_MFUL_CMD_GET_VER       0x60U   /**< MIFARE Ultralight Get version command byte */
#define PHAL_MFUL_CMD_FAST_READ     0x3AU   /**< MIFARE Ultralight Fast read command byte */
#define PHAL_MFUL_CMD_SECTOR_SELECT 0xC2U   /**< MIFARE Ultralight SECTOR SELECT command byte */
#define PHAL_MFUL_CMD_READ_SIG      0x3CU   /**< MIFARE Ultralight Read signature command byte */
#define PHAL_MFUL_CMD_CHK_TRG_EVT   0x3EU   /**< MIFARE Ultralight Check tearing event command byte */
#define PHAL_MFUL_CMD_WRITE_SIGN    0xA9U   /**< MIFARE Ultralight Write signature command byte */
#define PHAL_MFUL_CMD_LOCK_SIGN     0xACU   /**< MIFARE Ultralight Lock signature command byte */
#define PHAL_MFUL_CMD_VCSL          0x4BU   /**< MIFARE Ultralight Virtual card select command byte */
#define PHAL_MFUL_CMD_READ_TT_STATUS    0xA4U   /**< MIFARE Ultralight Virtual card select command byte */
/** @} */

#define PHAL_MFUL_NOT_AUTHENTICATED         0xFFU   /**< MIFARE Ultralight macro to represent not authenticated state. */
#define PHAL_MFUL_PREAMBLE_TX               0xAFU   /**< MIFARE Ultralight preamble byte (tx) for authentication. */
#define PHAL_MFUL_PREAMBLE_RX               0x00U   /**< MIFARE Ultralight preamble byte (rx) for authentication. */

#define PHAL_MFUL_ORIGINALITY_KEY           0x02U   /**< MIFARE Ultralight Originality Check Key. */
#define PHAL_MFUL_COMPWRITE_BLOCK_LENGTH    16U     /**< Length of a compatibility write MIFARE(R) Ultralight data block. */

/** @} */

phStatus_t phalMful_Int_Read(
                             void * pPalMifareDataParams,
                             uint8_t bAddress,
                             uint8_t * pData
                             );

phStatus_t phalMful_Int_Write(
                              void * pPalMifareDataParams,
                              uint8_t bAddress,
                              uint8_t * pData
                              );

phStatus_t phalMful_Int_FastWrite(
                                  void * pPalMifareDataParams,
                                  uint8_t * pData
                                  );

phStatus_t phalMful_Int_CompatibilityWrite(
    void * pPalMifareDataParams,
    uint8_t bAddress,
    uint8_t * pData
    );

phStatus_t phalMful_Int_IncrCnt(
                                void * pPalMifareDataParams,
                                uint8_t bCntNum,
                                uint8_t * pCnt
                                );

phStatus_t phalMful_Int_ReadCnt(
                                void * pPalMifareDataParams,
                                uint8_t bCntNum,
                                uint8_t * pCntValue
                                );

phStatus_t phalMful_Int_PwdAuth(
                                void * pPalMifareDataParams,
                                uint8_t * pPwd,
                                uint8_t * pPack
                                );

phStatus_t phalMful_Int_GetVersion(
                                   void * pPalMifareDataParams,
                                   uint8_t * pVersion
                                   );

phStatus_t phalMful_Int_FastRead(
                                 void * pPalMifareDataParams,
                                 uint8_t  bStartAddr,
                                 uint8_t bEndAddr,
                                 uint8_t ** ppData,
                                 uint16_t * pNumBytes
                                 );

phStatus_t phalMful_Int_SectorSelect(
                                     void * pPalMifareDataParams,
                                     uint8_t bSecNo
                                     );

phStatus_t phalMful_Int_ReadSign(
                                 void * pPalMifareDataParams,
                                 uint8_t bAddr,
                                 uint8_t ** pSignature,
                                  uint16_t* pDataLen
                                 );

phStatus_t phalMful_Int_ChkTearingEvent(
                                        void * pPalMifareDataParams,
                                        uint8_t bCntNum,
                                        uint8_t * pValidFlag
                                        );

phStatus_t phalMful_Int_WriteSign(
                                  void * pPalMifareDataParams,
                                  uint8_t bAddress,
                                  uint8_t * pSignature
                                  );

phStatus_t phalMful_Int_LockSign(
                                 void * pPalMifareDataParams,
                                 uint8_t bLockMode
                                 );

phStatus_t phalMful_Int_VirtualCardSelect(
                                          void * pPalMifareDataParams,
                                          uint8_t * pVCIID,
                                          uint8_t bVCIIDLen,
                                          uint8_t * pVCTID
                                          );

phStatus_t phalMful_Int_ReadTTStatus(
                                    void * pPalMifareDataParams,
                                    uint8_t bAddr,
                                    uint8_t * pData
                                    );

phStatus_t phalMful_Int_Exchange (
                                    void * pDataParams,
                                    uint8_t bCmd,
                                    uint8_t * pData,
                                    uint8_t bDataLen,
                                    uint8_t ** ppResponse,
                                    uint16_t * pRespLen );

phStatus_t phalMful_Int_GetPalDataParams(
                                        void * pDataParams,
                                        void** pPalMifareDataParams
                                        );

phStatus_t phalMful_Int_GetCMAC(
                                void * pDataParams,
                                uint8_t* pCMACVal
                                );

phStatus_t phalMful_Int_GetAUTHMODE(
                                    void * pDataParams,
                                    uint8_t* pAUTHMODE
                                    );

phStatus_t phalMful_Int_TruncateMac(uint8_t * pMac);

#endif /* PHALMFUL_INT_H */
