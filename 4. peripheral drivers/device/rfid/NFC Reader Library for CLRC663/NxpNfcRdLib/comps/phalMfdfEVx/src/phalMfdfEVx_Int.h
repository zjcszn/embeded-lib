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
* Internal functions of Software implementation of MIFARE DESFire EVx contactless IC application layer.
* $Author: Rajendran Kumar (nxp99556) $
* $Revision: 5949 $ (v07.09.00)
* $Date: 2020-01-29 19:49:02 +0530 (Wed, 29 Jan 2020) $
*/

#ifndef PHALMFDFEVX_INT_H
#define PHALMFDFEVX_INT_H

#include <ph_Status.h>
#include <phalMfdfEVx.h>

/** \addtogroup ph_Private
 * @{
 */

/** \name MIFARE DESFire Native Commands */
/* @{ */
#define PHAL_MFDFEVX_CMD_INVALID                                                0xFFU   /**< INVALID Command code. */
#define PHAL_MFDFEVX_CMD_AUTHENTICATE                                           0x0AU   /**< MFDFEVx Authenticate command. */
#define PHAL_MFDFEVX_CMD_AUTHENTICATE_ISO                                       0x1AU   /**< MFDFEVx Authenticate ISO command. */
#define PHAL_MFDFEVX_CMD_AUTHENTICATE_AES                                       0xAAU   /**< MFDFEVx Authenticate AES command. */
#define PHAL_MFDFEVX_CMD_AUTHENTICATE_EV2_FIRST                                 0x71U   /**< MFDFEVx Authenticate EV2 First command. */
#define PHAL_MFDFEVX_CMD_AUTHENTICATE_EV2_NON_FIRST                             0x77U   /**< MFDFEVx Authenticate EV2 Non First command. */
#define PHAL_MFDFEVX_CMD_AUTHENTICATE_PART2                                     0xAFU   /**< MFDFEVx Authentication second part; 0xAF. */
#define PHAL_MFDFEVX_CMD_CHANGE_KEY_SETTINGS                                    0x54U   /**< MFDFEVx Change key settings cmd. */
#define PHAL_MFDFEVX_CMD_GET_KEY_SETTINGS                                       0x45U   /**< MFDFEVx Get Key Settings Cmd.  */
#define PHAL_MFDFEVX_CMD_CHANGE_KEY                                             0xC4U   /**< MFDFEVx Change key cmd. */
#define PHAL_MFDFEVX_CMD_CHANGE_KEY_EV2                                         0xC6U   /**< MFDFEVx Change key stored on PICC cmd. */
#define PHAL_MFDFEVX_CMD_GET_KEY_VERSION                                        0x64U   /**< MFDFEVx Get Key Version. */
#define PHAL_MFDFEVX_CMD_INITIALIZE_KEY_SET                                     0x56U   /**< MFDFEVx Initialize a key set cmd. */
#define PHAL_MFDFEVX_CMD_FINALIZE_KEY_SET                                       0x57U   /**< MFDFEVx Finalize a key set cmd. */
#define PHAL_MFDFEVX_CMD_ROLL_KEY_SET                                           0x55U   /**< MFDFEVx Roll to a new key set cmd. */
#define PHAL_MFDFEVX_CMD_CREATE_APPLN                                           0xCAU   /**< MFDFEVx Create Application cmd. */
#define PHAL_MFDFEVX_CMD_CREATE_DELEGATED_APPLN                                 0xC9U   /**< MFDFEVx Create Delegated Application cmd. */
#define PHAL_MFDFEVX_CMD_DELETE_APPLN                                           0xDAU   /**< MFDFEVx Delete Application cmd. */
#define PHAL_MFDFEVX_CMD_GET_APPLN_IDS                                          0x6AU   /**< MFDFEVx Get Application Ids cmd. */
#define PHAL_MFDFEVX_CMD_GET_DF_NAMES                                           0x6DU   /**< MFDFEVx Get Dedicated Fine names cmd. */
#define PHAL_MFDFEVX_CMD_GET_DELEGATED_INFO                                     0x69U   /**< MFDFEVx Get Delegated info cmd. */
#define PHAL_MFDFEVX_CMD_SELECT_APPLN                                           0x5AU   /**< MFDFEVx Select Application Cmd. */
#define PHAL_MFDFEVX_CMD_FORMAT                                                 0xFCU   /**< MFDFEVx Format PICC Cmd. */
#define PHAL_MFDFEVX_CMD_GET_VERSION                                            0x60U   /**< MFDFEVx Get Version cmd. */
#define PHAL_MFDFEVX_CMD_FREE_MEM                                               0x6EU   /**< MFDFEVx Free Memory cmd. */
#define PHAL_MFDFEVX_CMD_SET_CONFIG                                             0x5CU   /**< MFDFEVx Set Configuration Cmd. */
#define PHAL_MFDFEVX_CMD_GET_CARD_UID                                           0x51U   /**< MFDFEVx Get Card UID cmd. */
#define PHAL_MFDFEVX_CMD_GET_FILE_IDS                                           0x6FU   /**< MFDFEVx Get File IDs cmd. */
#define PHAL_MFDFEVX_CMD_GET_ISO_FILE_IDS                                       0x61U   /**< MFDFEVx Get ISO File IDs cmd. */
#define PHAL_MFDFEVX_CMD_GET_FILE_SETTINGS                                      0xF5U   /**< MFDFEVx Get File settings cmd. */
#define PHAL_MFDFEVX_CMD_GET_FILE_COUNTERS                                      0xF6U   /**< MFDFEVx Get File Counters cmd. */
#define PHAL_MFDFEVX_CMD_CHANGE_FILE_SETTINGS                                   0x5FU   /**< MFDFEVx Change file settings cmd. */
#define PHAL_MFDFEVX_CMD_CREATE_STD_DATAFILE                                    0xCDU   /**< MFDFEVx Create Standard data file cmd. */
#define PHAL_MFDFEVX_CMD_CREATE_BKUP_DATAFILE                                   0xCBU   /**< MFDFEVx Create Backup data file cmd. */
#define PHAL_MFDFEVX_CMD_CREATE_VALUE_FILE                                      0xCCU   /**< MFDFEVx Create Value File cmd. */
#define PHAL_MFDFEVX_CMD_CREATE_LINEAR_RECFILE                                  0xC1U   /**< MFDFEVx Create Linear record file cmd. */
#define PHAL_MFDFEVX_CMD_CREATE_CYCLIC_RECFILE                                  0xC0U   /**< MFDFEVx Create Cyclic record file cmd. */
#define PHAL_MFDFEVX_CMD_CREATE_TRANSTN_MACFILE                                 0xCEU   /**< MFDFEVx Create Transaction MAC file cmd. */
#define PHAL_MFDFEVX_CMD_DELETE_FILE                                            0xDFU   /**< MFDFEVx Delete File cmd. */
#define PHAL_MFDFEVX_CMD_READ_DATA                                              0xBDU   /**< MFDFEVx Read Data cmd. */
#define PHAL_MFDFEVX_CMD_READ_DATA_ISO                                          0xADU   /**< MFDFEVx Read Data cmd using ISO chaining. */
#define PHAL_MFDFEVX_CMD_WRITE_DATA                                             0x3DU   /**< MFDFEVx Write data cmd. */
#define PHAL_MFDFEVX_CMD_WRITE_DATA_ISO                                         0x8DU   /**< MFDFEVx Write data cmd using ISO chaining. */
#define PHAL_MFDFEVX_CMD_GET_VALUE                                              0x6CU   /**< MFDFEVx Get Value cmd. */
#define PHAL_MFDFEVX_CMD_CREDIT                                                 0x0CU   /**< MFDFEVx Credit cmd. */
#define PHAL_MFDFEVX_CMD_DEBIT                                                  0xDCU   /**< MFDFEVx Debit cmd. */
#define PHAL_MFDFEVX_CMD_LIMITED_CREDIT                                         0x1CU   /**< MFDFEVx Limited Credit cmd. */
#define PHAL_MFDFEVX_CMD_WRITE_RECORD                                           0x3BU   /**< MFDFEVx Write Record cmd. */
#define PHAL_MFDFEVX_CMD_WRITE_RECORD_ISO                                       0x8BU   /**< MFDFEVx Write Record cmd using ISO chaining. */
#define PHAL_MFDFEVX_CMD_UPDATE_RECORD                                          0xDBU   /**< MFDFEVx Update Record cmd. */
#define PHAL_MFDFEVX_CMD_UPDATE_RECORD_ISO                                      0xBAU   /**< MFDFEVx Update Record cmd using ISO chaining. */
#define PHAL_MFDFEVX_CMD_READ_RECORDS                                           0xBBU   /**< MFDFEVx Read Records cmd. */
#define PHAL_MFDFEVX_CMD_READ_RECORDS_ISO                                       0xABU   /**< MFDFEVx Read Records cmd using ISO chaining. */
#define PHAL_MFDFEVX_CMD_UPDATE_RECORDS                                         0xF0U   /**< MFDFEVx Update Records cmd. */
#define PHAL_MFDFEVX_CMD_CLEAR_RECORD_FILE                                      0xEBU   /**< MFDFEVx Clear records file cmd. */
#define PHAL_MFDFEVX_CMD_COMMIT_TXN                                             0xC7U   /**< MFDFEVx Commit transaction cmd. */
#define PHAL_MFDFEVX_CMD_ABORT_TXN                                              0xA7U   /**< MFDFEVx Abort transaction cmd. */
#define PHAL_MFDFEVX_CMD_COMMIT_READER_ID                                       0xC8U   /**< MFDFEVx Commit Reader ID cmd. */
#define PHAL_MFDFEVX_CMD_READ_SIG                                               0x3CU   /**< MFDFEVx Verify read signature command. */
#define PHAL_MFDFEVX_CMD_FORMAT_PICC                                            0xFCU   /**< MFDFEVx Format PICC Cmd. */
#define PHAL_MFDFEVX_CMD_CLEAR_RECORDS_FILE                                     0xEBU   /**< MFDFEVx Clear records file cmd. */
#define PHAL_MFDFEVX_CMD_AUTH_PDC                                               0x7CU   /**< MFDFEVx Post Delivery Command. */
#define PHAL_MFDFEVX_CMD_AUTH2                                                  0x72U   /**< MFDFEVx Authenticate command (part 2). */
#define PHAL_MFDFEVX_CMD_CREATE_MFC_MAPPING                                     0xCFU   /**< MFDFEVx CreateMFCMapping. */
#define PHAL_MFDFEVX_CMD_RESTORE_TRANSFER                                       0xB1U   /**< MFDFEVx RestoreTransfer. */
#define PHAL_MFDFEVX_CMD_RESTORE_SHADOW                                         0xB5U   /**< MFDFEVx RestoreShadow. */
#define PHAL_MFDFEVX_CMD_RESTRICT_MFC_UPDATE                                    0xBFU   /**< MFDFEVx RestrictMFCUpdate. */
/* @} */

/** \name MIFARE DESFire Response Codes */
/* @{ */
#define PHAL_MFDFEVX_RESP_OPERATION_OK                                          0x00U   /**< MFDFEVx Response - Successful operation. */
#define PHAL_MFDFEVX_RESP_OPERATION_OK_LIM                                      0x01U   /**< MFDFEVx Response - Successful operation with limited functionality. */
#define PHAL_MFDFEVX_RESP_OK                                                    0x90U   /**< MFDFEVx Response - Successful operation. */
#define PHAL_MFDFEVX_RESP_NO_CHANGES                                            0x0CU   /**< MFDFEVx Response - No changes done to backup files. */
#define PHAL_MFDFEVX_RESP_ERR_NOT_SUP                                           0x0DU   /**< MFDFEVx Not Supported Error. */
#define PHAL_MFDFEVX_RESP_ERR_OUT_OF_EEPROM_ERROR                               0x0EU   /**< MFDFEVx Response - Insufficient NV-Memory. */
#define PHAL_MFDFEVX_RESP_ILLEGAL_COMMAND_CODE                                  0x1CU   /**< MFDFEVx command code not supported. */
#define PHAL_MFDFEVX_RESP_ERR_INTEGRITY_ERROR                                   0x1EU   /**< MFDFEVx CRC or MAC does not match data padding bytes not valid. */
#define PHAL_MFDFEVX_RESP_NO_SUCH_KEY                                           0x40U   /**< MFDFEVx Invalid key number specified. */
#define PHAL_MFDFEVX_RESP_CHAINING                                              0x71U   /**< MFDFEVx ISO Chaining Status. */
#define PHAL_MFDFEVX_RESP_ERR_LENGTH_ERROR                                      0x7EU   /**< MFDFEVx Length of command string invalid. */
#define PHAL_MFDFEVX_RESP_PERMISSION_DENIED                                     0x9DU   /**< MFDFEVx Current configuration/status does not allow the requested command. */
#define PHAL_MFDFEVX_RESP_ERR_PARAMETER_ERROR                                   0x9EU   /**< MFDFEVx Value of params invalid. */
#define PHAL_MFDFEVX_RESP_APPLICATION_NOT_FOUND                                 0xA0U   /**< MFDFEVx Requested AID not found on PICC. */
#define PHAL_MFDFEVX_RESP_ERR_APPL_INTEGRITY_ERROR                              0xA1U   /**< MFDFEVx Unrecoverable error within application, appln will be disabled. */
#define PHAL_MFDFEVX_RESP_ERR_AUTHENTICATION_ERROR                              0xAEU   /**< MFDFEVx Current authentication status does not allow the requested cmd. */
#define PHAL_MFDFEVX_RESP_ADDITIONAL_FRAME                                      0xAFU   /**< MFDFEVx Additional data frame is expected to be sent. */
#define PHAL_MFDFEVX_RESP_ERR_BOUNDARY_ERROR                                    0xBEU   /**< MFDFEVx Attempt to read/write data from/to beyond the files/record's limits. */
#define PHAL_MFDFEVX_RESP_ERR_PICC_INTEGRITY                                    0xC1U   /**< MFDFEVx Unrecoverable error within PICC. PICC will be disabled. */
#define PHAL_MFDFEVX_RESP_ERR_COMMAND_ABORTED                                   0xCAU   /**< MFDFEVx Previous cmd not fully completed. Not all frames were requested or provided by the PCD. */
#define PHAL_MFDFEVX_RESP_ERR_PIC_DISABLED                                      0xCDU   /**< MFDFEVx PICC was disabled by an unrecoverable error. */
#define PHAL_MFDFEVX_RESP_ERR_COUNT                                             0xCEU   /**< MFDFEVx Num. of applns limited to 28. No additional applications possible. */
#define PHAL_MFDFEVX_RESP_ERR_DUPLICATE                                         0xDEU   /**< MFDFEVx File/Application with same number already exists. */
#define PHAL_MFDFEVX_RESP_ERR_EEPROM                                            0xEEU   /**< MFDFEVx Could not complete NV-Write operation due to loss of power. */
#define PHAL_MFDFEVX_RESP_ERR_FILE_NOT_FOUND                                    0xF0U   /**< MFDFEVx Specified file number does not exist. */
#define PHAL_MFDFEVX_RESP_ERR_FILE_INTEGRITY                                    0xF1U   /**< MFDFEVx Unrecoverable error within file. File will be disabled. */
#define PHAL_MFDFEVX_RESP_ERR_AUTH_ERROR                                        0x06U   /**< MFDFEVx Authentication error. */
#define PHAL_MFDFEVX_RESP_ERR_CMD_OVERFLOW                                      0x07U   /**< MFDFEVx Too many commands in the session or transaction. */
#define PHAL_MFDFEVX_RESP_ERR_CMD_BNR                                           0x09U   /**< MFDFEVx Invalid Block number: not existing in the implementation or not valid to target with this command. */
#define PHAL_MFDFEVX_RESP_ERR_CMD_INVALID                                       0x0BU   /**< MFDFEVx Command is received in a state where this command is not supported, or a totally unknown command is received.*/
#define PHAL_MFDFEVX_RESP_ERR_CMD_ERR_FORMAT                                    0x0CU   /**< MFDFEVx Format of the command is not correct (e.g. too many or too few bytes). */
#define PHAL_MFDFEVX_RESP_ERR_CMD_GEN_FAILURE                                   0x0FU   /**< MFDFEVx Failure in the operation of the PD. */
/* @} */

/** \name ISO 7816 Instructions */
/* @{ */
#define PHAL_MFDFEVX_CMD_ISO7816_SELECT_FILE                                    0xA4U   /**< ISO Select File. */
#define PHAL_MFDFEVX_CMD_ISO7816_READ_RECORDS                                   0xB2U   /**< ISO Read records. */
#define PHAL_MFDFEVX_CMD_ISO7816_READ_BINARY                                    0xB0U   /**< ISO Read Binary. */
#define PHAL_MFDFEVX_CMD_ISO7816_UPDATE_BINARY                                  0xD6U   /**< ISO UPDATE Binary. */
#define PHAL_MFDFEVX_CMD_ISO7816_APPEND_RECORD                                  0xE2U   /**< ISO Append record. */
#define PHAL_MFDFEVX_CMD_ISO7816_UPDATE_RECORD                                  0xD2U   /**< ISO Update record. */
#define PHAL_MFDFEVX_CMD_ISO7816_GET_CHALLENGE                                  0x84U   /**< ISO Get challenge. */
#define PHAL_MFDFEVX_CMD_ISO7816_EXT_AUTHENTICATE                               0x82U   /**< ISO Ext. Authenticate. */
#define PHAL_MFDFEVX_CMD_ISO7816_INT_AUTHENTICATE                               0x88U   /**< ISO Int. Authenticate. */
/* @} */

/** \name MIFARE DESFire specific defines */
/* @{ */
#define PHAL_MFDFEVX_MAXWRAPPEDAPDU_SIZE                                        0x37u   /**< Maximum size of wrapped APDU 55 bytes. */
#define PHAL_MFDFEVX_MAXDFAPDU_SIZE                                             0x3Cu   /**< Maximum size of DESFire APDU 60 bytes. */
#define PHAL_MFDFEVX_DFAPPID_SIZE                                               0x03u   /**< Size of MFDFEVx application Id. */
#define PHAL_MFDFEVX_DATA_BLOCK_SIZE                                            0x10u   /**< Data block size need for internal purposes. */
#define PHAL_MFDFEVX_MAX_FRAME_SIZE                                             0x40u   /**< Max size in a ISO 14443-4 frame. */
/* @} */

/** \name ISO 7816 DFEVx return Codes */
/* @{ */
#define PHAL_MFDFEVX_ISO7816_SUCCESS                                            0x9000U /**< Correct execution. */
#define PHAL_MFDFEVX_ISO7816_ERR_WRONG_LENGTH                                   0x6700U /**< Wrong length. */
#define PHAL_MFDFEVX_ISO7816_ERR_INVALID_APPLN                                  0x6A82U /**< Application / file not found. */
#define PHAL_MFDFEVX_ISO7816_ERR_WRONG_PARAMS                                   0x6A86U /**< Wrong parameters P1 and/or P2. */
#define PHAL_MFDFEVX_ISO7816_ERR_WRONG_LC                                       0x6A87U /**< Lc inconsistent with P1/p2. */
#define PHAL_MFDFEVX_ISO7816_ERR_WRONG_LE                                       0x6C00U /**< Wrong Le. */
#define PHAL_MFDFEVX_ISO7816_ERR_NO_PRECISE_DIAGNOSTICS                         0x6F00U /**< No precise diagnostics. */
#define PHAL_MFDFEVX_ISO7816_ERR_EOF_REACHED                                    0x6282U /**< End of File reached. */
#define PHAL_MFDFEVX_ISO7816_ERR_FILE_ACCESS                                    0x6982U /**< File access not allowed. */
#define PHAL_MFDFEVX_ISO7816_ERR_FILE_EMPTY                                     0x6985U /**< File empty or access conditions not satisfied. */
#define PHAL_MFDFEVX_ISO7816_ERR_FILE_NOT_FOUND                                 0x6A82U /**< File not found. */
#define PHAL_MFDFEVX_ISO7816_ERR_MEMORY_FAILURE                                 0x6581U /**< Memory failure (unsuccessful update). */
#define PHAL_MFDFEVX_ISO7816_ERR_INCORRECT_PARAMS                               0x6B00U /**< Wrong parameter p1 or p2. READ RECORDS. */
#define PHAL_MFDFEVX_ISO7816_ERR_WRONG_CLA                                      0x6E00U /**< Wrong Class byte. */
#define PHAL_MFDFEVX_ISO7816_ERR_UNSUPPORTED_INS                                0x6D00U /**< Instruction not supported. */
#define PHAL_MFDFEVX_ISO7816_ERR_LIMITED_FUNCTIONALITY_INS                      0x6283U /**< Limited Functionality. */
/* @} */

#define PHAL_MFDFEVX_WRAP_HDR_LEN                                               0x05U   /**< Wrapped APDU header length */
#define PHAL_MFDFEVX_WRAPPEDAPDU_CLA                                            0x90U   /**< Wrapped APDU default class. */
#define PHAL_MFDFEVX_WRAPPEDAPDU_P1                                             0x00U   /**< Wrapped APDU default P1. */
#define PHAL_MFDFEVX_WRAPPEDAPDU_P2                                             0x00U   /**< Wrapped APDU default P2. */
#define PHAL_MFDFEVX_WRAPPEDAPDU_LE                                             0x00U   /**< Wrapped APDU default LE. */

#define PHAL_MFDFEVX_TRUNCATED_MAC_SIZE                                         8u      /**< Size of the truncated MAC. */
#define PHAL_MFDFEVX_KEYSETVERSIONS                                             0x40U   /**< 6th bit KeyNo used to retrieve all keyset versions. */
#define PHAL_MFDFEVX_ISO_CHAINING_MODE                                          0x04U   /**< Option to enable ISO chaining. */
#define PHAL_MFDFEVX_DEFAULT_MODE                                               0x00U   /**< Native Chaining. */
#define PHAL_MFDFEVX_PC_RND_LEN                                                 7u      /**< Size of the Proximity Check Random numbers. */
#define PHAL_MFDFEVX_SIG_LENGTH                                                 0x38U   /**< NXP Originality Signature length */
#define PHAL_MFDFEVX_DEFAULT_UID_LENGTH                                         0x07U   /**< By default EVx card is configures as 7 byte UID */
#define PHAL_MFDFEVX_10B_UID_LENGTH                                             0x0CU   /**< EVx can also be configured as 10 byte UID. */
#define PHAL_MFDFEVX_4B_UID_LENGTH                                              0x06U   /**< EVx can also be configured as 4 byte UID. */
#define PHAL_MFDFEVX_DEF_VERSION_LENGTH                                         0x1CU   /**< Version String is of 28 bytes by default(If its 7 Byte UID) */
#define PHAL_MFDFEVX_10B_VERSION_LENGTH                                         0x21U   /**< Version String is of 30 bytes If its 10B Byte UID */
#define PHAL_MFDFEVX_4B_VERSION_LENGTH                                          0x1BU   /**< Version String is of 27 bytes If its 4B Byte UID */
#define PHAL_MFDFEVX_ISO_CHAINING_MODE_MAPPED       (PHAL_MFDFEVX_ISO_CHAINING_MODE)    /**< Option to enable ISO chaining mapped for internal use. */

#define PHAL_MFDFEVX_ISO_7816_NO_LC_LE                                          0x00U   /**< Check if Lc=0 and Le=0 */
#define PHAL_MFDFEVX_ISO_7816_LC_SHORT_APDU                                     0x01U   /**< Indicates Lc=1 byte */
#define PHAL_MFDFEVX_ISO_7816_LC_EXT_APDU_3B                                    0x03U   /**< Indicates Lc=3 bytes */
#define PHAL_MFDFEVX_ISO_7816_LE_SHORT_APDU     PHAL_MFDFEVX_ISO_7816_LC_SHORT_APDU     /**< Indicates Le=1 byte */
#define PHAL_MFDFEVX_ISO_7816_LE_EXT_APDU_2B                                    0x02U   /**< Indicates Le=2 bytes */
#define PHAL_MFDFEVX_ISO_7816_LE_EXT_APDU_3B    PHAL_MFDFEVX_ISO_7816_LC_EXT_APDU_3B    /**< Indicates Le=3 bytes */

/** \name Proximity Check return Codes */
/* @{ */
#define PHAL_MFDFEVX_RESP_NACK0                                                 0x00U   /*< MFDFEVx NACK 0 (in ISO14443-3 mode). */
#define PHAL_MFDFEVX_RESP_NACK1                                                 0x01U   /*< MFDFEVx NACK 1 (in ISO14443-3 mode). */
#define PHAL_MFDFEVX_RESP_NACK4                                                 0x04U   /*< MFDFEVx NACK 4 (in ISO14443-3 mode). */
#define PHAL_MFDFEVX_RESP_NACK5                                                 0x05U   /*< MFDFEVx NACK 5 (in ISO14443-3 mode). */
#define PHAL_MFDFEVX_RESP_ACK_ISO3                                              0x0AU   /*< MFDFEVx ACK (in ISO14443-3 mode). */
#define PHAL_MFDFEVX_RESP_ACK_ISO4                                              0x90U   /*< MFDFEVx ACK (in ISO14443-4 mode). */
#define PHAL_MFDFEVX_RESP_ERR_AUTH                                              0x06U   /*< MFDFEVx Authentication Error. */
#define PHAL_MFDFEVX_RESP_ERR_CMD_OVERFLOW                                      0x07U   /*< MFDFEVx Command Overflow Error. */
#define PHAL_MFDFEVX_RESP_ERR_MAC_PCD                                           0x08U   /*< MFDFEVx MAC Error. */
#define PHAL_MFDFEVX_RESP_ERR_BNR                                               0x09U   /*< MFDFEVx Blocknumber Error. */
#define PHAL_MFDFEVX_RESP_ERR_EXT                                               0x0AU   /*< MFDFEVx Extension Error. */
#define PHAL_MFDFEVX_RESP_ERR_CMD_INVALID                                       0x0BU   /*< MFDFEVx Invalid Command Error. */
#define PHAL_MFDFEVX_RESP_ERR_FORMAT                                            0x0CU   /*< MFDFEVx Format Error. */
#define PHAL_MFDFEVX_RESP_ERR_GEN_FAILURE                                       0x0FU   /*< MFDFEVx Generic Error. */
/* @} */

/** \name Macros for SDM functionality */
/* @{ */
#define PHAL_MFDFEVX_PICC_DATA                                                  0x10U   /**< PICCDATA . */
#define PHAL_MFDFEVX_MAC_DATA                                                   0x08U   /**< MAC DATA. */
#define PHAL_MFDFEVX_SESSION_ENC                                                0x01U   /**< Optionto perform generate the SDM session encryption keys. */
#define PHAL_MFDFEVX_SESSION_MAC                                                0x02U   /**< Optionto perform generate the SDM session MAC keys. */
#define PHAL_MFDFEVX_PLAIN_PICCDATA_MIRRIORING                                  0xE0U   /**< Option to indicate the presence of VCUID or RdCtr in the data. */
#define PHAL_MFDFEVX_NO_PICCDATA_MIRRIORING                                     0xF0U   /**< Option to indicate the presence of Encrypted VCUID or RdCtr in the data. */
/* @} */

/* To avoid unused variable warnings. */
#define PHAL_MFDFEVX_UNUSED_VARIABLE(x)  {for( ( x ) = ( x ) ; ( x ) != ( x ) ; );}
#define PHAL_MFDFEVX_UNUSED_ARRAY(x)  {for( ( (x)[0] ) = ( (x)[0] ) ; ( (x)[0] ) != ( (x)[0] ) ; );}

phStatus_t phalMfdfEVx_ExchangeCmd(void * pDataParams, void * pPalMifareDataParams, uint8_t bWrappedMode, uint8_t * pCmd, uint16_t wCmdLen,
    uint8_t ** ppResponse, uint16_t * wRxlen);

phStatus_t phalMfdfEVx_Int_ComputeErrorResponse(void * pDataParams, uint16_t wStatus);

phStatus_t phalMfdfEVx_Int_Send7816Apdu(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t bIns, uint8_t p1, uint8_t p2,
    uint32_t Lc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint32_t bLe, uint8_t ** pDataOut, uint16_t *pDataLen);

phStatus_t phalMfdfEVx_Int_ISOSelectFile(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint8_t bLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint8_t bLe, uint8_t ** ppDataOut, uint16_t *pDataLen);

phStatus_t phalMfdfEVx_Int_ISOUpdateBinary(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint32_t dwLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint8_t ** ppDataOut, uint16_t *pDataLen);

phStatus_t phalMfdfEVx_Int_ISOAppendRecord(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint32_t dwLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint8_t ** ppDataOut, uint16_t *pDataLen);

phStatus_t phalMfdfEVx_Int_ISOGetChallenge(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint8_t bLe, uint8_t bExtendedLenApdu, uint8_t ** ppDataOut, uint16_t *pDataLen);

phStatus_t phalMfdfEVx_Int_ISOExternalAuthenticate(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint8_t bLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint8_t ** ppDataOut, uint16_t *pDataLen);

phStatus_t phalMfdfEVx_Int_ISOInternalAuthenticate(void * pDataParams, void * pPalMifareDataParams, uint8_t bOption, uint8_t * bCmdBuff, uint16_t wCmdLen,
    uint8_t bLc, uint8_t bExtendedLenApdu, uint8_t * pDataIn, uint32_t dwLe, uint8_t ** ppDataOut, uint16_t *pDataLen);

phStatus_t phalMfdfEVx_Int_GetCmdCode(void * pDataParams, uint8_t * pCmdCode);

phStatus_t phalMfdfEVx_Int_SetCmdCode(void * pDataParams, uint8_t bCmdCode);
/* @} */

#endif /* PHALMFDFEVX_INT_H */
