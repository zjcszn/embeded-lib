/*----------------------------------------------------------------------------*/
/* Copyright 2014-2020 NXP                                                    */
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

#ifndef PHALMFDFEVX_SW_H
#define PHALMFDFEVX_SW_H





/* MIFARE DESFire EVx Memory and Configuration mamangement commands. ------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_FreeMem(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pMemInfo);


phStatus_t phalMfdfEVx_Sw_GetVersion(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pVerInfo);


phStatus_t phalMfdfEVx_Sw_GetKeySettings(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pKeySettings,
    uint8_t * bRespLen);





/* MIFARE DESFire EVx Application mamangement commands. -------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_CreateApplication(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pAid,
    uint8_t bKeySettings1, uint8_t bKeySettings2, uint8_t bKeySettings3, uint8_t * pKeySetValues, uint8_t * pISOFileId,
    uint8_t * pISODFName, uint8_t bISODFNameLen);

phStatus_t phalMfdfEVx_Sw_DeleteApplication(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pAid, uint8_t * pDAMMAC, uint8_t bDAMMAC_Len);

phStatus_t phalMfdfEVx_Sw_CreateDelegatedApplication(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pAid,
    uint8_t * pDamParams, uint8_t bKeySettings1, uint8_t bKeySettings2, uint8_t bKeySettings3, uint8_t * bKeySetValues,
    uint8_t * pISOFileId, uint8_t * pISODFName, uint8_t bISODFNameLen, uint8_t * pEncK, uint8_t * pDAMMAC);

phStatus_t phalMfdfEVx_Sw_SelectApplication(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pAppId,
    uint8_t * pAppId2);

phStatus_t phalMfdfEVx_Sw_GetApplicationIDs(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t ** pAidBuff,
    uint8_t * pNumAIDs);


phStatus_t phalMfdfEVx_Sw_GetDelegatedInfo(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pDAMSlot, uint8_t * pDamSlotVer,
    uint8_t * pQuotaLimit, uint8_t * pFreeBlocks, uint8_t * pAid);




/* MIFARE DESFire EVx File mamangement commands. --------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_CreateStdDataFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bFileNo,
    uint8_t * pISOFileId, uint8_t bFileOption, uint8_t * pAccessRights, uint8_t * pFileSize);

phStatus_t phalMfdfEVx_Sw_CreateBackupDataFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bFileNo,
    uint8_t * pISOFileId, uint8_t bFileOption, uint8_t * pAccessRights, uint8_t * pFileSize);

phStatus_t phalMfdfEVx_Sw_CreateValueFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bFileNo, uint8_t bCommSett,
    uint8_t * pAccessRights, uint8_t * pLowerLmit, uint8_t * pUpperLmit, uint8_t * pValue, uint8_t bLimitedCredit);

phStatus_t phalMfdfEVx_Sw_CreateLinearRecordFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t  bFileNo,
    uint8_t  *pIsoFileId, uint8_t bCommSett, uint8_t * pAccessRights, uint8_t * pRecordSize, uint8_t * pMaxNoOfRec);

phStatus_t phalMfdfEVx_Sw_CreateCyclicRecordFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t  bFileNo,
    uint8_t  *pIsoFileId, uint8_t bCommSett, uint8_t * pAccessRights, uint8_t * pRecordSize,
    uint8_t * pMaxNoOfRec);


phStatus_t phalMfdfEVx_Sw_DeleteFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bFileNo);

phStatus_t phalMfdfEVx_Sw_GetFileIDs(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pFid, uint8_t * bNumFID);

phStatus_t phalMfdfEVx_Sw_GetISOFileIDs(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pFidBuffer, uint8_t * pNumFID);

phStatus_t phalMfdfEVx_Sw_GetFileSettings(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bFileNo, uint8_t * pFSBuffer,
    uint8_t * pBufferLen);

phStatus_t phalMfdfEVx_Sw_GetFileCounters(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bFileNo,
    uint8_t * pFileCounters, uint8_t * pRxLen);

phStatus_t phalMfdfEVx_Sw_ChangeFileSettings(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption,
    uint8_t bFileNo, uint8_t bFileOption, uint8_t * pAccessRights, uint8_t bAddInfoLen, uint8_t * pAddInfo);




/* MIFARE DESFire EVx Data mamangement commands. --------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_ReadData(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bIns,
    uint8_t bFileNo, uint8_t * pOffset, uint8_t * pLength, uint8_t ** ppRxdata, uint16_t * pRxdataLen);

phStatus_t phalMfdfEVx_Sw_WriteData(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bIns,
    uint8_t bFileNo, uint8_t * pOffset, uint8_t * pData, uint8_t * pDataLen);

phStatus_t phalMfdfEVx_Sw_GetValue(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bFileNo,
    uint8_t * pValue);

phStatus_t phalMfdfEVx_Sw_Credit(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bFileNo,
    uint8_t * pValue);

phStatus_t phalMfdfEVx_Sw_Debit(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bFileNo,
    uint8_t * pValue);

phStatus_t phalMfdfEVx_Sw_LimitedCredit(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bFileNo,
    uint8_t * pValue);

phStatus_t phalMfdfEVx_Sw_ReadRecords(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bIns,
    uint8_t bFileNo, uint8_t * pRecNo, uint8_t * pRecCount, uint8_t * pRecSize, uint8_t ** ppRxdata, uint16_t * pRxdataLen);

phStatus_t phalMfdfEVx_Sw_WriteRecord(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bIns,
    uint8_t bFileNo, uint8_t * pOffset, uint8_t * pData, uint8_t * pDataLen);

phStatus_t phalMfdfEVx_Sw_UpdateRecord(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bIns,
    uint8_t bFileNo, uint8_t * pRecNo, uint8_t * pOffset, uint8_t * pData, uint8_t * pDataLen);

phStatus_t phalMfdfEVx_Sw_ClearRecordFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bFileNo);




/* MIFARE DESFire EVx Transaction mamangement commands. -------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_CommitTransaction(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t * pTMC,
    uint8_t * pTMAC);

phStatus_t phalMfdfEVx_Sw_AbortTransaction(phalMfdfEVx_Sw_DataParams_t * pDataParams);

phStatus_t phalMfdfEVx_Sw_CommitReaderID(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t * pTMRI, uint8_t * pEncTMRI);




/* MIFARE DESFire EVx ISO7816-4 commands. ---------------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_IsoSelectFile(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOption, uint8_t bSelector,
    uint8_t * pFid, uint8_t * pDFname, uint8_t bDFnameLen, uint8_t  bExtendedLenApdu, uint8_t ** ppFCI, uint16_t * pwFCILen);

phStatus_t phalMfdfEVx_Sw_IsoReadBinary(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wOption, uint8_t bOffset,
    uint8_t bSfid, uint32_t dwBytesToRead, uint8_t bExtendedLenApdu, uint8_t ** ppRxBuffer, uint32_t * pBytesRead);

phStatus_t phalMfdfEVx_Sw_IsoUpdateBinary(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bOffset, uint8_t bSfid,
    uint8_t bExtendedLenApdu, uint8_t * pData, uint32_t dwDataLen);

phStatus_t phalMfdfEVx_Sw_IsoReadRecords(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wOption, uint8_t bRecNo,
    uint8_t bReadAllFromP1, uint8_t bSfid, uint32_t dwBytesToRead, uint8_t bExtendedLenApdu, uint8_t ** ppRxBuffer,
    uint32_t * pBytesRead);

phStatus_t phalMfdfEVx_Sw_IsoAppendRecord(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bSfid, uint8_t bExtendedLenApdu,
    uint8_t * pData, uint32_t dwDataLen);

phStatus_t phalMfdfEVx_Sw_IsoUpdateRecord(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bIns, uint8_t bRecNo,
    uint8_t bSfid, uint8_t bRefCtrl, uint8_t * pData, uint8_t bDataLen);

phStatus_t phalMfdfEVx_Sw_IsoGetChallenge(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wKeyNo, uint16_t wKeyVer,
    uint8_t bExtendedLenApdu, uint32_t dwLe, uint8_t * pRPICC1);





/* MIFARE DESFire EVx Originality Check functions. ------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_ReadSign(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bAddr, uint8_t ** pSignature);




/* MIFARE DESFire EVx MIFARE Classic contactless IC functions. ---------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_CreateMFCMapping(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bComOption, uint8_t bFileNo,
    uint8_t bFileOption, uint8_t * pMFCBlockList, uint8_t bMFCBlocksLen, uint8_t bRestoreSource, uint8_t * pMFCLicense,
    uint8_t bMFCLicenseLen, uint8_t * pMFCLicenseMAC);

phStatus_t phalMfdfEVx_Sw_RestoreTransfer(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint8_t bCommOption, uint8_t bTargetFileNo, uint8_t bSourceFileNo);





/* MIFARE DESFire EVx Miscellaneous functions. ----------------------------------------------------------------------------------------- */
phStatus_t phalMfdfEVx_Sw_GetConfig(phalMfdfEVx_Sw_DataParams_t * pDataParams, uint16_t wConfig, uint16_t * pValue);

phStatus_t phalMfdfEVx_Sw_SetConfig(phalMfdfEVx_Sw_DataParams_t *pDataParams, uint16_t wConfig, uint16_t wValue);

phStatus_t phalMfdfEVx_Sw_ResetAuthentication(phalMfdfEVx_Sw_DataParams_t * pDataParams);


phStatus_t phalMfdfEVx_Sw_SetVCAParams(phalMfdfEVx_Sw_DataParams_t * pDataParams, void * pAlVCADataParams);
#endif /* PHALMFDFEVX_SW_H */
