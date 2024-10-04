/*----------------------------------------------------------------------------*/
/* Copyright 2014-2020,2023 NXP                                               */
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
*
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#ifndef PHALMFDFEVX_SW_INT_H
#define PHALMFDFEVX_SW_INT_H

phStatus_t phalMfdfEVx_Sw_Int_CardExchange(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wBufferOption, uint16_t wTotDataLen,
    uint8_t bExchangeLE, uint8_t * pData, uint16_t wDataLen, uint8_t ** ppResponse, uint16_t * pRespLen, uint8_t * pPiccRetCode);

phStatus_t phalMfdfEVx_Sw_Int_SendDataToPICC(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bIns, uint8_t bCommOption,
    uint8_t * pCmd, uint16_t wCmdLen, uint8_t * pData, uint16_t wDataLen, uint8_t * bLastChunk, uint16_t wLastChunkLen,
    uint8_t * pResp, uint16_t * pRespLen );

phStatus_t phalMfdfEVx_Sw_Int_SendDataAndAddDataToPICC(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bIns, uint8_t * pCmd,
    uint16_t wCmdLen, uint8_t * pData, uint16_t wDataLen, uint8_t * pAddData, uint16_t wAddDataLen, uint8_t * pResp,
    uint16_t * pRespLen);

phStatus_t phalMfdfEVx_Sw_Int_GetData(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pSendBuff, uint16_t wCmdLen,
    uint8_t ** pResponse, uint16_t * pRxlen);

phStatus_t phalMfdfEVx_Sw_Int_ReadData_Plain(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * bCmdBuff,
    uint16_t wCmdLen, uint8_t ** ppRxdata, uint16_t * pRxdataLen);

phStatus_t phalMfdfEVx_Sw_Int_Write_Plain(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bIns, uint8_t * bCmdBuff,
    uint16_t wCmdLen, uint8_t bCommOption, uint8_t * pData, uint16_t  wDataLen);


phStatus_t phalMfdfEVx_Sw_Int_Write_New(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t * pCmdBuff,
    uint16_t wCmdLen, uint8_t * pData, uint16_t wDataLen);

void phalMfdfEVx_Sw_Int_ResetAuthStatus(phalMfdfEVx_Sw_DataParams_t * pDataParams);

phStatus_t phalMfdfEVx_Sw_Int_IsoRead(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wOption, uint8_t * bCmdBuff,
    uint16_t wCmdLen, uint8_t ** ppRxBuffer, uint32_t * pBytesRead);


phStatus_t phalMfdfEVx_Sw_Int_GetFrameLength(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t * pFSD, uint16_t * pFSC);

phStatus_t phalMfdfEVx_Sw_Int_ISOGetData(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pSendBuff, uint16_t wCmdLen,
    uint8_t ** pResponse, uint16_t * pRxlen);


#endif /* PHALMFDFEVX_SW_INT_H */
