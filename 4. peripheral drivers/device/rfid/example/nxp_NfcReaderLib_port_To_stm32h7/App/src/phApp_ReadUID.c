phStatus_t phApp_ReadUID(void) {
    phStatus_t status = PH_ERR_SUCCESS;
    uint8_t bAtqbLen;
    uint8_t aAtqb[13];
    uint8_t aCmd[9] = {0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x08};
    uint8_t *pResp = NULL;
    uint16_t wRespLength;
    uint8_t bErrCnt = 0;
    phpalI14443p3b_Sw_DataParams_t *pPal14443B = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P3B);
    
    do {
        if (bErrCnt > 10) {
            DEBUG_PRINTF("[ERROR]: read uid of id card failed");
            break;
        }
        
        /*
        status = phpalI14443p3b_RequestB(pPal14443B, 0, 0, 0, aAtqb, &bAtqbLen);
        if ((status != PH_ERR_SUCCESS) || ((bAtqbLen != 12) && (bAtqbLen != 13))) {
            bErrCnt++;
            continue;
        }
        */
        
        status = phhalHw_Exchange(pPal14443B->pHalDataParams, PH_EXCHANGE_BUFFER_LAST, aCmd, 9, &pResp, &wRespLength);
        if ((status != PH_ERR_SUCCESS) || (wRespLength != 1) || (pResp[0] != 0x08)) {
            bErrCnt++;
            continue;
        }
        
        aCmd[0] = 0x00;
        aCmd[1] = 0x36;
        aCmd[2] = 0x00;
        aCmd[3] = 0x00;
        aCmd[4] = 0x08;
        status = phhalHw_Exchange(pPal14443B->pHalDataParams, PH_EXCHANGE_BUFFER_LAST, aCmd, 5, &pResp, &wRespLength);
        if ((status != PH_ERR_SUCCESS) || (wRespLength != 10) || (pResp[8] != 0x90) || (pResp[9] != 0x00)) {
            bErrCnt++;
            continue;
        } else {
            DEBUG_PRINTF("UID: ");
            phApp_Print_Buff(pResp, 8);
            DEBUG_PRINTF("\r\n");
            break;
        }
    } while (1);
    
    return status;
}
