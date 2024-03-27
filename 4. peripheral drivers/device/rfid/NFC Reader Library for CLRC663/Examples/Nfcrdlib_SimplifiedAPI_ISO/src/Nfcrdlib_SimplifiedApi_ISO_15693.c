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
 * Reference application file for ISO15693 interface of Simplified API
 * $Author: NXP $
 * $Revision: $ (v07.09.00)
 * $Date: $
 *
 */

#include <Nfcrdlib_SimplifiedApi_ISO.h>
extern phNfcLib_Transmit_t phNfcLib_TransmitInput;
extern phNfcLib_PeerInfo_t PeerInfo;

extern uint8_t bMoreDataAvailable;
extern uint16_t wNumberofBytes;
extern uint8_t  bDataBuffer[256];


/*
 * This application is to demonstrate the usage of simplified API related to ISO15693
 * The application reaches to this point only after the activation of a card with ISO 15693
 * interface is done. Transmit api is used to perform any command exchange with the card and
 * receive api is used to get back the out data.
 * In case the user already knows the UID of the card he wants to talk to he can directly call these api
 */
uint32_t NfcLib_ISO15693_Reference_app()
{
    uint32_t  dwStatus;
    uint8_t bData[16] = {0xA0, 0xA0, 0xA0, 0xA0,
                        0xA0, 0xA0, 0xA0, 0xA0,
                        0xA0, 0xA0, 0xA0, 0xA0,
                        0xA0, 0xA0, 0xA0, 0xA0};

    /*
     * Copying the UID of the card on which the user wish to perform the operation.
     * For the reference purpose we will be talking to the card at index value 0
     */
    memcpy(phNfcLib_TransmitInput.phNfcLib_ISO15693.bUid, PeerInfo.uTi.uInitiator.tIso15693.TagIndex[0].pUid, 8);


    do
    {

    /*************************************************************************************************************************
     ****************************************READ SINGLE BLOCK ***************************************************************
     *************************************************************************************************************************/

    /* The parameters for Read Single Block are the option and the Block number user wants to read */
    /* Option is kept #PH_OFF for refer purpose and the block number to Read is Block - 5 */
    READ_SINGLEBLOCK(PH_OFF, 5)

    /* Finally the command has to be sent, the command for 15693 will be send in addressed mode */
    dwStatus = phNfcLib_Transmit(&phNfcLib_TransmitInput,
                                0x0  /* This length paramter is used only when apart from the command, there is some data to be send*/
                                );


    /* The status should be success, if not break from the application */
    if(dwStatus != PH_NFCLIB_STATUS_SUCCESS)
    {
        DEBUG_PRINTF (" \n Card does not support Read single block... \n");
        break;
    }

    /* wNumberofBytes has to be reset before every receive */
    wNumberofBytes = 256;

    /*The data received over the above command can be retrieved by calling the receive */
    /* wNumberofBytes first as input tells the max supported recieve size and then as out tells the actual number of data bytes received */
    /* wNumberofBytes will vary depending upon the card layout*/
    dwStatus = phNfcLib_Receive(&bDataBuffer[0],
                                &wNumberofBytes,
                                &bMoreDataAvailable
                                );

    /* The status should be success and the number of bytes received should be 4 for Most NXP cards
     * The expectation of the number of bytes can be modified for different cards
     */
    if((dwStatus != PH_NFCLIB_STATUS_SUCCESS) ||(wNumberofBytes != 4))
    {
        break;
    }
    DEBUG_PRINTF("\nRead Data from Block 5 is");
    phApp_Print_Buff(&bDataBuffer[0], wNumberofBytes);

    /*************************************************************************************************************************
     ****************************************WRITE SINGLE BLOCK **************************************************************
     *************************************************************************************************************************/

    /* The parameters for Write Single Block are the option, the Block number user wants to write and the data */
    /* Option is kept #PH_OFF for refer purpose and the block number to write is Block - 5 */

    WRITE_SINGLEBLOCK(PH_OFF, 5, &bData[0])

    /* Finally the command has to be sent, the command for 15693 will be send in addressed mode */
    dwStatus = phNfcLib_Transmit(&phNfcLib_TransmitInput,
                                0x04  /* While writing a block we will be writing 4 bytes of data*/
                                );


    /* The status should be success, if not break from the application */
    if(dwStatus != PH_NFCLIB_STATUS_SUCCESS)
    {
        DEBUG_PRINTF (" \n Card does not support Write single block... \n");
        break;
    }

    /*************************************************************************************************************************
     ****************************************READ MULTIPLE BLOCK **************************************************************
     *************************************************************************************************************************/

    /* The parameters for Read Multiple Block are the option, the starting Block number and the no of blocks to read*/
    /* Option is kept #PH_OFF for refer purpose, the starting block  number is 5 and the no of blocks is 4*/
    READ_MULTIPLEBLOCK(PH_OFF, 5, 4)

    /* Finally the command has to be sent, the command for 15693 will be send in addressed mode */
    dwStatus = phNfcLib_Transmit(&phNfcLib_TransmitInput,
                                0x0  /* This length paramter is used only when apart from the command, there is some data to be send*/
                                );

    /* The status should be success, if not break from the application */
    if(dwStatus != PH_NFCLIB_STATUS_SUCCESS)
    {
        DEBUG_PRINTF (" \n Card does not support Read Multiple block... \n");
        break;
    }

    /* This parameter has to be reset before every receive */
    wNumberofBytes = 256;

    /*The data received over the above command can be retrieved by calling the receive */
    /* wNumberofBytes first as input tells the max supported recieve size and then as out tells the actual number of data bytes received */
    /* wNumberofBytes will vary depending upon the card layout*/
    dwStatus = phNfcLib_Receive(&bDataBuffer[0],
                                &wNumberofBytes,
                                &bMoreDataAvailable
                                );

    /* The status should be success and the number of bytes received should be 4 * numofblocks for Most NXP cards
     * The expectation of the number of bytes can be modified for different cards
     */
    if((dwStatus != PH_NFCLIB_STATUS_SUCCESS) ||(wNumberofBytes != 16))
    {
        break;
    }
    DEBUG_PRINTF("\nRead Data from Block 5  to 8 is");
    phApp_Print_Buff(&bDataBuffer[0], wNumberofBytes);

    /*************************************************************************************************************************
     *****************************************GET SYSTEM INFORMATION *********************************************************
     *************************************************************************************************************************/

    /* Get System Information takes no parameter and returns back the System Information */
    GET_SYSTEMINFORMATION()

    /* Finally the command has to be sent, the command for 15693 will be send in addressed mode */
    dwStatus = phNfcLib_Transmit(&phNfcLib_TransmitInput,
                                0x0  /* This length parameter is used only when apart from the command, there is some data to be send*/
                                 );

    /* The status should be success, if not break from the application */
    if(dwStatus != PH_NFCLIB_STATUS_SUCCESS)
    {
        DEBUG_PRINTF (" \n Card does not support GET SYSTEM INFORMATION... \n");
        break;
    }

    /* This parameter has to be reset before every receive */
    wNumberofBytes = 256;

    /*The System Information  received over the above command can be retrieved by calling the receive */
    /* wNumberofBytes first as input tells the max supported recieve size and then as out tells the actual number of data bytes received */
    dwStatus = phNfcLib_Receive(&bDataBuffer[0],
                &wNumberofBytes,
                &bMoreDataAvailable
                );

    /* The status should be success */

    if(dwStatus != PH_NFCLIB_STATUS_SUCCESS)
    {
        break;
    }
    DEBUG_PRINTF("\nSystem Information is ");
    phApp_Print_Buff(&bDataBuffer[0], wNumberofBytes);

    /*************************************************************************************************************************
     ************************************GET MULTIPLE BLOCK SECURITY STATUS **************************************************
     *************************************************************************************************************************/

    /* The parameters for Get Multiple Block security Status are the starting block nad the no of blocks whose status to read*/
    /* Starting block is 9 which is already locked and no of blocks is 4 */

    GET_MULTIPLEBLOCKSECURITYSTATUS(9, 4)

    /* Finally the command has to be sent, the command for 15693 will be send in addressed mode */
    dwStatus = phNfcLib_Transmit(&phNfcLib_TransmitInput,
                                0x0  /* This length parameter is used only when apart from the command, there is some data to be send*/
                                 );

    /* The status should be success, if not break from the application */
    if(dwStatus != PH_NFCLIB_STATUS_SUCCESS)
    {
        DEBUG_PRINTF (" \n Card does not support GET MULTIPLE BLOCK SECURITY STATUS... \n");
        break;
    }

    /* This parameter has to be reset before every receive */
    wNumberofBytes = 256;

    /*The System Information received over the above command can be retrieved by calling the receive */
    /* wNumberofBytes first as input tells the max supported recieve size and then as out tells the actual number of data bytes received */
    dwStatus = phNfcLib_Receive(&bDataBuffer[0],
                &wNumberofBytes,
                &bMoreDataAvailable
                );

    /* The status should be success */

    if(dwStatus != PH_NFCLIB_STATUS_SUCCESS)
    {
        break;
    }
    DEBUG_PRINTF("\nSecurity status for block 9 to 12  is ");
    phApp_Print_Buff(&bDataBuffer[0], wNumberofBytes);

    }while(0);

    return dwStatus;
}
