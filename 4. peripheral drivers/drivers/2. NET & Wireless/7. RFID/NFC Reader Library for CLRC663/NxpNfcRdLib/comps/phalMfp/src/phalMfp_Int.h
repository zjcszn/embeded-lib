/*----------------------------------------------------------------------------*/
/* Copyright 2009, 2020 NXP                                                   */
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
* Internal functions of Software implementation of MIFARE Plus contactless IC application layer.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
* History:
*  CHu: Generated 19. May 2009
*
*/

#ifndef PHALMFP_INT_H
#define PHALMFP_INT_H

#include <ph_Status.h>

/** \addtogroup ph_Private
* @{
*/

/**
* \name MIFARE Plus contactless IC Commands in MIFARE Plus Mode
*/
/*@{*/
#define PHAL_MFP_CMD_AUTH1_FIRST    0x70U    /**< MFP Authenticate command (part 1, first authentication). */
#define PHAL_MFP_CMD_AUTH1          0x76U    /**< MFP Authenticate command (part 1, non-first authentication). */
#define PHAL_MFP_CMD_AUTH2          0x72U    /**< MFP Authenticate command (part 2). */
#define PHAL_MFP_CMD_AUTH3          0x74U    /**< MFP Authenticate command (part 3). */
#define PHAL_MFP_CMD_RAUTH          0x78U    /**< MFP ResetAuth command. */

#define PHAL_MFP_CMD_READ_ENM       0x30U    /**< MFP Read Encrypted, No mac on response, Mac on command. */
#define PHAL_MFP_CMD_READ_EMM       0x31U    /**< MFP Read Encrypted, Mac on response, Mac on command. */
#define PHAL_MFP_CMD_READ_PNM       0x32U    /**< MFP Read Plain, No mac on response, Mac on command. */
#define PHAL_MFP_CMD_READ_PMM       0x33U    /**< MFP Read Plain, Mac on response, Mac on command. */
#define PHAL_MFP_CMD_READ_ENU       0x34U    /**< MFP Read Encrypted, No mac on response, Unmac'ed command. */
#define PHAL_MFP_CMD_READ_EMU       0x35U    /**< MFP Read Encrypted, Mac on response, Unmac'ed command. */
#define PHAL_MFP_CMD_READ_PNU       0x36U    /**< MFP Read Plain, No mac on response, Unmac'ed command. */
#define PHAL_MFP_CMD_READ_PMU       0x37U    /**< MFP Read Plain, Mac on response, Unmac'ed command. */

#define PHAL_MFP_CMD_WRITE_EN       0xA0U    /**< MFP Write Encrypted, No mac on response, (Mac on command). */
#define PHAL_MFP_CMD_WRITE_EM       0xA1U    /**< MFP Write Encrypted, Mac on response, (Mac on command). */
#define PHAL_MFP_CMD_WRITE_PN       0xA2U    /**< MFP Write Plain, No mac on response, (Mac on command). */
#define PHAL_MFP_CMD_WRITE_PM       0xA3U    /**< MFP Write Plain, Mac on response, (Mac on command). */

#define PHAL_MFP_CMD_WRITEPERSO     0xA8U    /**< MFP Write Perso. */
#define PHAL_MFP_CMD_COMMITPERSO    0xAAU    /**< MFP Commit Perso. */
#define PHAL_MFP_CMD_INCR           0xB0U    /**< MFP Increment command. */
#define PHAL_MFP_CMD_INCR_M         0xB1U    /**< MFP Increment command MACed. */
#define PHAL_MFP_CMD_DECR           0xB2U    /**< MFP Decrement command. */
#define PHAL_MFP_CMD_DECR_M         0xB3U    /**< MFP Decrement command MACed. */
#define PHAL_MFP_CMD_TRANS          0xB4U    /**< MFP Transfer command. */
#define PHAL_MFP_CMD_TRANS_M        0xB5U    /**< MFP Transfer command MACed. */
#define PHAL_MFP_CMD_INCRTR         0xB6U    /**< MFP Increment Transfer command. */
#define PHAL_MFP_CMD_INCRTR_M       0xB7U    /**< MFP Increment Transfer command MACed. */
#define PHAL_MFP_CMD_DECRTR         0xB8U    /**< MFP Decrement Transfer command. */
#define PHAL_MFP_CMD_DECRTR_M       0xB9U    /**< MFP Decrement Transfer command MACed. */
#define PHAL_MFP_CMD_REST           0xC2U    /**< MFP Restore command. */
#define PHAL_MFP_CMD_REST_M         0xC3U    /**< MFP Restore command MACed. */
#define PHAL_MFP_CMD_PPC            0xF0U    /**< MFP Prepare Proximity Check. */
#define PHAL_MFP_CMD_PC             0xF2U    /**< MFP Proximity Check command. */
#define PHAL_MFP_CMD_VPC            0xFDU    /**< MFP Verify Proximity Check command. */
/*@}*/

/**
* \name MIFARE Plus Commands in SL2 using ISO14443-3 exchange
*/
/*@{*/
#define PHAL_MFP_CMD_MBREAD     0x38U    /**< MFP Multi Block Read. */
#define PHAL_MFP_CMD_MBWRITE    0xA8U    /**< MFP Multi Block Read. */
/*@}*/

/**
* \name MIFARE Plus Response Codes
*/
/*@{*/
#define PHAL_MFP_RESP_NACK0             0x00U   /*< MFP NACK 0 (in ISO14443-3 mode). */
#define PHAL_MFP_RESP_NACK1             0x01U   /*< MFP NACK 1 (in ISO14443-3 mode). */
#define PHAL_MFP_RESP_NACK4             0x04U   /*< MFP NACK 4 (in ISO14443-3 mode). */
#define PHAL_MFP_RESP_NACK5             0x05U   /*< MFP NACK 5 (in ISO14443-3 mode). */
#define PHAL_MFP_RESP_ACK_ISO3          0x0AU   /*< MFP ACK (in ISO14443-3 mode). */
#define PHAL_MFP_RESP_ACK_ISO4          0x90U   /*< MFP ACK (in ISO14443-4 mode). */
#define PHAL_MFP_RESP_ERR_AUTH          0x06U   /*< MFP Authentication Error. */
#define PHAL_MFP_RESP_ERR_CMD_OVERFLOW  0x07U   /*< MFP Command Overflow Error. */
#define PHAL_MFP_RESP_ERR_MAC_PCD       0x08U   /*< MFP MAC Error. */
#define PHAL_MFP_RESP_ERR_BNR           0x09U   /*< MFP Blocknumber Error. */
#define PHAL_MFP_RESP_ERR_EXT           0x0AU   /*< MFP Extension Error. */
#define PHAL_MFP_RESP_ERR_CMD_INVALID   0x0BU   /*< MFP Invalid Command Error. */
#define PHAL_MFP_RESP_ERR_FORMAT        0x0CU   /*< MFP Format Error. */
#define PHAL_MFP_RESP_ERR_GEN_FAILURE   0x0FU   /*< MFP Generic Error. */
/*@}*/

#define PHAL_MFP_TRUNCATED_MAC_SIZE         8U   /**< Size of the truncated MAC. */
#define PHAL_MFP_MIFARE_VALUE_BLOCK_SIZE    4U   /**< Size of a MIFARE Classic contactless IC value block. */
#define PHAL_MFP_MIFARE_BLOCK_SIZE          16U  /**< Size of the MIFARE Classic blocks. */
#define PHAL_MFP_PC_RND_LEN                 7U   /**< Size of the Proximity Check Random numbers. */

/**
* \brief Evaluate if the received response is erroneus.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMfp_Int_ComputeErrorResponse(
    uint16_t wNumBytesReceived,     /**< [In] Number of bytes received from the card. */
    uint8_t bStatus,                /**< [In] Status byte received from the card. */
    uint8_t bLayer4Comm             /**< [In] \c 0: use ISO14443-3 protocol; \c 1: use ISO14443-4 protocol. */
    );

/**
* \brief Perform a MIFARE Plus Write Perso command.
*
* The Write Perso command can be executed using the ISO14443-3 communication protocol (after layer 3 activation)
* or using the ISO14443-4 protocol (after layer 4 activation).
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_Int_WritePerso(
                                  void * pPalMifareDataParams,  /**< [In] Pointer to a palMifare component context. */
                                  uint8_t bLayer4Comm,          /**< [In] \c 0: use ISO14443-3 protocol; \c 1: use ISO14443-4 protocol. */
                                  uint16_t wBlockNr,            /**< [In] Block number. */
                                  uint8_t * pValue              /**< [In] Value (16 bytes). */
                                  );

/**
* \brief Perform a MIFARE Plus Commit Perso command.
*
* The Commit Perso command can be executed using the ISO14443-3 communication protocol (after layer 3 activation)
* or using the ISO14443-4 protocol (after layer 4 activation).
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_Int_CommitPerso(
                                   void * pPalMifareDataParams,     /**< [In] Pointer to a palMifare component context. */
                                   uint8_t bLayer4Comm              /**< [In] \c 0: use ISO14443-3 protocol; \c 1: use ISO14443-4 protocol. */
                                   );

/**
* \brief Perform a MIFARE Plus Reset Authenticate command.
*
* The Reset Authenticate command is executed using the ISO14443-4 communication protocol (after layer 4 activation)
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_Int_ResetAuth(
                                 void * pPalMifareDataParams     /**< [In] Pointer to a palMifare component context. */
                                 );

/**
* \brief Perform a Prepare Proximity Check command.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_Int_PrepareProximityCheck(
    void * pPalMifareDataParams     /**< [In] Pointer to a palMifare component context. */
    );

/**
* \brief Perform a Proximity Check command.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_Int_ProximityCheck(
                                      void * pPalMifareDataParams,  /**< [In] Pointer to a palMifare component context. */
                                      uint8_t bNumSteps,            /**< [In] Number of steps for the PC (1..7). */
                                      uint8_t * pRndC,              /**< [In] RndC[7]. */
                                      uint8_t * pRndRC              /**< [Out] RndRC[14]. */
                                      );

/**
* \brief Perform a Multi Block Read command.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_Int_MultiBlockRead(
                                      void * pPalMifareDataParams,  /**< [In] Pointer to a palMifare component context. */
                                      uint8_t bBlockNr,             /**< [In] Block number. */
                                      uint8_t bNumBlocks,           /**< [In] Number of blocks to read (should not be more than 3). */
                                      uint8_t * pBlocks             /**< [Out] Block(s); uint8_t[16U * \c bNumBlocks]. */
                                      );

/**
* \brief Perform a Multi Block Write command.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
* \retval Other Depending on implementation and underlying component.
*/
phStatus_t phalMfp_Int_MultiBlockWrite(
                                       void * pPalMifareDataParams,     /**< [In] Pointer to a palMifare component context. */
                                       uint8_t bBlockNr,                /**< [In] Block number. */
                                       uint8_t bNumBlocks,              /**< [In] Number of blocks to write (should not be more than 3). */
                                       uint8_t * pBlocks                /**< [In] Block(s); uint8_t[16U * \c bNumBlocks]. */
                                       );
/**
* \brief Create a Value block for a given value/addr pair.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMfp_Int_CreateValueBlock(
                                        uint8_t * pValue,   /**< [In] Value to be converted. */
                                        uint8_t bAddrData,  /**< [In] bAddrData containing destination address. */
                                        uint8_t * pBlock    /**< [Out] Formatted Value block. */
                                        );

/**
* \brief Check value block format of a given block.
* \return Status code
* \retval #PH_ERR_SUCCESS Operation successful.
*/
phStatus_t phalMfp_Int_CheckValueBlockFormat(
    uint8_t * pBlock    /**< [In] Formatted Value block. */
    );

/** @}
* end of ph_Private
*/

#endif /* PHALMFP_INT_H */
