//
// Created by zjcszn on 2024/4/22.
//

#include "phApp_Desfire.h"
#include "phhalHw.h"
#include "phNfcLib.h"
#include "clrc663/NxpNfcRdLib/comps/phpalI14443p4a/src/Sw/phpalI14443p4a_Sw_Int.h"
#include "clrc663/NxpNfcRdLib/comps/phCryptoSym/src/Sw/phCryptoSym_Sw_Des.h"
#include "phApp_Init.h"

phStatus_t phhalDesfire_Authenticate() {
    phStatus_t status;
    uint8_t *pResp = NULL;
    uint16_t wRespLength;
    uint8_t cmd[17] = {0x0A, 0X00};
    uint8_t tmp[16];
    uint8_t iv[PH_CRYPTOSYM_DES_BLOCK_SIZE] = {0};
    uint8_t randA[8];
    
    void *ppali14443p4 = phNfcLib_GetDataParams(PH_COMP_PAL_ISO14443P4);
    void *pcryptosym = phNfcLib_GetDataParams(PH_COMP_CRYPTOSYM);
    
    do {
        /* load key, key.no=2, key.ver=0 */
        status = phCryptoSym_LoadKey(pcryptosym, 2, 0, PH_KEYSTORE_KEY_TYPE_DES);
        if (status != PH_ERR_SUCCESS)
            break;
        
        status = phCryptoSym_LoadIv(pcryptosym, iv, PH_CRYPTOSYM_DES_BLOCK_SIZE);
        if (status != PH_ERR_SUCCESS)
            break;
        
        /* Set Activation timeout */
        status = phhalHw_SetConfig(
            phNfcLib_GetDataParams(PH_COMP_HAL),
            PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,
            PHPAL_I14443P4A_SW_FWT_ACTIVATION_US + PHPAL_I14443P4A_SW_EXT_TIME_US);
        if (status != PH_ERR_SUCCESS)
            break;
        
        /* step1-> send [0A 00] command, receive encrypted randB' form picc */
        status = phpalI14443p4_Exchange(ppali14443p4, PH_EXCHANGE_DEFAULT, cmd, 2, &pResp, &wRespLength);
        if (status != PH_ERR_SUCCESS)
            break;
        
        /* The PCD runs a DES/3DES deciphering operation on the received ekkeyNo(RndB) and thus retrieves RndB. */
        status = phCryptoSym_Decrypt(pcryptosym, PH_CRYPTOSYM_CIPHER_MODE_CBC, &pResp[1], 8, &tmp[7]);
        if (status != PH_ERR_SUCCESS)
            break;
        
        tmp[15] = tmp[7];
        
        /* PCD generates an 8 byte random number RndA. This RndA is concatenated with RndB’ and deciphered using
         * DES/3DES (The decryption of the two blocks is chained using the Cipher Block Chaining (CBC) send mode).*/
        /* todo:
         * generate rand data of randA */
        memcpy(&tmp[0], randA, 8);
        
        cmd[0] = 0xAF;
        phCryptoSym_Encrypt(pcryptosym, PH_CRYPTOSYM_CIPHER_MODE_CBC, &tmp[0], 16, &cmd[1]);
        
        status = phpalI14443p4_Exchange(ppali14443p4, PH_EXCHANGE_DEFAULT, cmd, 17, &pResp, &wRespLength);
        if (status != PH_ERR_SUCCESS)
            break;
        
        /* PCD runs a DES/3DES decipherment on the received ekkeyNo (RndA’) and thus gains RndA’ for
         * comparison with the PCD-internally rotated RndA' */
        status = phCryptoSym_Decrypt(pcryptosym, PH_CRYPTOSYM_CIPHER_MODE_CBC, &pResp[1], 8, &tmp[1]);
        if (status != PH_ERR_SUCCESS)
            break;
        
        tmp[0] = tmp[8];
        
        if (memcmp(&tmp[0], &randA, 8) != 0) {
            status = PH_ERR_AUTH_ERROR;
            DEBUG_PRINTF("MIFARE Desfire auth failed\n");
        } else {
            DEBUG_PRINTF("MIFARE Desfire auth success\n");
        }
        
    } while (0);
    
    DEBUG_PRINTF("status: %04X\n", status);
    
    return status;
}