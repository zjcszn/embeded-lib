/*----------------------------------------------------------------------------*/
/* Copyright 2013-2022 NXP                                                    */
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

#ifndef PHALTOP_SW_INT_T4T_H
#define PHALTOP_SW_INT_T4T_H

#ifdef NXPBUILD__PHAL_TOP_SW
#ifdef NXPBUILD__PHAL_TOP_T4T_SW

#define PHAL_TOP_T4T_NDEF_APP_ID                    0x000001U    /**< Application id of the NDEF Application */
#define PHAL_TOP_T4T_ISOFILEID_APP                  0x05E1U      /**< ISO file id of the NDEF application */
#define PHAL_TOP_T4T_SELECT_EF_ID                   0x02U        /**< File Selection by EF Id */
#define PHAL_TOP_T4T_SELECT_DF_NAME                 0x04U        /**< File Selection by DF Name */
#define PHAL_TOP_T4T_CCLEN_OFFSET                   0x00U        /**< Offset value of CCLEN */
#define PHAL_TOP_T4T_VERSION_OFFSET                 0x02U        /**< Offset value of Mapping Version */
#define PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_OFFSET     0x03U        /**< Offset value of MLe (bytes) i.e Maximum R-APDU data size */
#define PHAL_TOP_T4T_MAX_CMD_LENGTH_OFFSET          0x05U        /**< Offset value of MLc (bytes) i.e Maximum C-APDU data size */
#define PHAL_TOP_T4T_NDEFTLV_OFFSET                 0x07U        /**< Offset value of NDEF File Control TLV */
#define PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_OFFSET       0x09U        /**< Offset value of NDEF File Control TLV, NDEF File ID */
#define PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_OFFSET     0x0BU        /**< Offset value of NDEF File Control TLV, NDEF/ENDEF File size */
#define PHAL_TOP_T4T_NDEFTLV_V_RA_MV20_OFFSET       0x0DU        /**< Offset value of NDEF File Control TLV, NDEF File READ access as per MV 2.0 */
#define PHAL_TOP_T4T_NDEFTLV_V_WA_MV20_OFFSET       0x0EU        /**< Offset value of NDEF File Control TLV, NDEF File WRITE access as per MV 2.0 */
#define PHAL_TOP_T4T_NDEFTLV_V_RA_MV30_OFFSET       0x0FU        /**< Offset value of NDEF File Control TLV, NDEF File READ access as per MV 3.0 */
#define PHAL_TOP_T4T_NDEFTLV_V_WA_MV30_OFFSET       0x10U        /**< Offset value of NDEF File Control TLV, NDEF File WRITE access as per MV 3.0 */

/* File Control TLV V-Field offsets */
#define PHAL_TOP_T4T_NDEFTLV_V_FILE_ID_BUF_OFFSET   0x00U        /**< Buffer offset value of NDEF File Control TLV, NDEF File ID */
#define PHAL_TOP_T4T_NDEFTLV_V_FILE_SIZE_BUF_OFFSET 0x02U        /**< Buffer offset value of NDEF File Control TLV, NDEF/ENDEF File size */
#define PHAL_TOP_T4T_NDEFTLV_V_RA_MV20_BUF_OFFSET   0x04U        /**< Buffer offset value of NDEF File Control TLV, NDEF File READ access as per MV 2.0 */
#define PHAL_TOP_T4T_NDEFTLV_V_WA_MV20_BUF_OFFSET   0x05U        /**< Buffer offset value of NDEF File Control TLV, NDEF File WRITE access as per MV 2.0 */
#define PHAL_TOP_T4T_NDEFTLV_V_RA_MV30_BUF_OFFSET   0x06U        /**< Buffer offset value of NDEF File Control TLV, NDEF File READ access as per MV 3.0 */
#define PHAL_TOP_T4T_NDEFTLV_V_WA_MV30_BUF_OFFSET   0x07U        /**< Buffer offset value of NDEF File Control TLV, NDEF File WRITE access as per MV 3.0 */

#define PHAL_TOP_T4T_SELECTED_NONE                  0x00U
#define PHAL_TOP_T4T_SELECTED_NDEF_APP              0x01U
#define PHAL_TOP_T4T_SELECTED_CC_FILE               0x02U
#define PHAL_TOP_T4T_SELECTED_NDEF_FILE             0x03U

#define PHAL_TOP_T4T_MAX_LENGTH_EXPECTED_MIN        0x000FU      /**< MLe (bytes); Maximum R-APDU data size */
#define PHAL_TOP_T4T_MAX_CMD_LENGTH_MIN             0x000DU      /**< MLc (bytes), Maximum C-APDU data size */

#define PHAL_TOP_T4T_NDEF_TLV_T                     0x04U        /**< NDEF Message TLV. */
#define PHAL_TOP_T4T_PROPRIETARY_TLV_T              0x05U        /**< Proprietary TLV. */
#define PHAL_TOP_T4T_NDEF_TLV_L                     0x06U        /**< NDEF Message TLV, L-Field for MV 2.0. */
#define PHAL_TOP_T4T_ENDEF_TLV_T                    0x06U        /**< Extended NDEF Message TLV. */
#define PHAL_TOP_T4T_EPROPRIETARY_TLV_T             0x07U        /**< Extended Proprietary TLV. */
#define PHAL_TOP_T4T_ENDEF_TLV_L                    0x08U        /**< ENDEF Message TLV, L-Field for MV 3.0. */

#define PHAL_TOP_T4T_NDEF_FILE_READ_ACCESS               0x00U   /**< Read/Write access */
#define PHAL_TOP_T4T_NDEF_FILE_READ_ACCESS_PROP_START    0x80U   /**< Limited Read access */
#define PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS              0x00U   /**< Read/Write access */
#define PHAL_TOP_T4T_NDEF_FILE_WRITE_ACCESS_PROP_START   0x80U   /**< Limited Write access */
#define PHAL_TOP_T4T_NDEF_FILE_NO_WRITE_ACCESS           0xFFU   /**< Read Only access */

#define PHAL_TOP_T4T_NDEF_FILE_SIZE_MIN_MV20        0x0005U      /**< NDEF File size minimum as per MV 2.0 */
#define PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV20        0x7FFFU      /**< NDEF File size maximum as per MV 2.0 */
#define PHAL_TOP_T4T_NDEF_FILE_SIZE_MIN_MV30        0x00000007U  /**< NDEF File size minimum as per MV 3.0 */
#define PHAL_TOP_T4T_NDEF_FILE_SIZE_MAX_MV30        0xFFFFFFFEU  /**< NDEF File size maximum as per MV 3.0 */

#define PHAL_TOP_T4T_CC_FILE_MIN_LEN                0x0FU        /**< CC File length minimum */
#define PHAL_TOP_T4T_CC_FILE_MAX_LEN                0x7FFFU      /**< CC File length maximum */
#define PHAL_TOP_T4T_CCLEN_MV20                     0x0FU        /**< CC File length as per MV 2.0 */
#define PHAL_TOP_T4T_CCLEN_MV30                     0x11U        /**< CC File length as per MV 3.0 */
#define PHAL_TOP_T4T_NDEF_FILE_CTRL_TLV_T           0x04U        /**< NDEF File control TLV Standard Data Structure T-Field */
#define PHAL_TOP_T4T_NDEF_FILE_CTRL_TLV_L           0x06U        /**< NDEF File control TLV Standard Data Structure L-Field */
#define PHAL_TOP_T4T_ENDEF_FILE_CTRL_TLV_T          0x06U        /**< ENDEF File control TLV Extended Data Structure T-Field */
#define PHAL_TOP_T4T_ENDEF_FILE_CTRL_TLV_L          0x08U        /**< ENDEF File control TLV Standard Data Structure L-Field */

#define PHAL_TOP_T4T_NLEN_SIZE                      0x02U        /**< NDEF Length size as per Standard Data Structure */
#define PHAL_TOP_T4T_ENLEN_SIZE                     0x04U        /**< ENDEF Length size as per Extended Data Structure */

#define PH_EXCHANGE_ODO                             0x00010000U   /**< Exchange option - Read with ODO */
#define PH_EXCHANGE_ODO_DDO                         0x00020000U   /**< Exchange option - Write with ODO and DDO */

/* ISO7816-4 command codes */
#define PHAL_T4T_CMD_ISO7816_SELECT_FILE            0xA4U    /**< ISOSelectFile command of ISO7816-4 Standard. */
#define PHAL_T4T_CMD_ISO7816_READ_BINARY            0xB0U    /**< ISOReadBinary command of ISO7816-4 Standard. */
#define PHAL_T4T_CMD_ISO7816_READ_BINARY_ODO        0xB1U    /**< ISOReadBinary command with ODO of ISO7816-4 Standard. */
#define PHAL_T4T_CMD_ISO7816_UPDATE_BINARY          0xD6U    /**< ISOUpdateBinary command of ISO7816-4 Standard. */
#define PHAL_T4T_CMD_ISO7816_UPDATE_BINARY_ODO_DDO  0xD7U    /**< ISOUpdateBinary command with ODO and DDO of ISO7816-4 Standard. */
#define PHAL_T4T_CMD_ISO7816_READ_RECORD            0xB2U    /**< ISOReadRecord command of ISO7816-4 Standard. */
#define PHAL_T4T_CMD_ISO7816_APPEND_RECORD          0xE2U    /**< ISOAppendRecord command of ISO7816-4 Standard. */
#define PHAL_T4T_CMD_ISO7816_GET_CHALLENGE          0x84U    /**< ISOGetChallenge command of ISO7816-4 Standard. */
#define PHAL_T4T_CMD_CREATE_STANDARD_DATA_FILE      0xCDU    /**< Create Standard Data File command code. */

/* Communication mode */
#define PHAL_ISO7816_COMMUNICATION_PLAIN            0x00U   /**< Plain mode of communication. */

/* Command Codes */
#define PHAL_MFECC_CMD_CREATE_STANDARD_DATA_FILE    0xCD    /**< Create Standard Data File command code. */

/* Encoding of ShortFile identifier or Offset */
#define PHAL_ISO7816_SFID_DISABLED                  0x00U   /**< Option to indicate Encoding as offset. */
#define PHAL_ISO7816_SFID_ENABLED                   0x80U   /**< Option to indicate Encoding as Short File Identifier. */

/* Options for application / file creation interfaces. */
#define PHAL_ISO7816_ISO_FILE_INFO_NOT_AVAILABLE    0x00U   /**< Option to indicate no ISO File ID or ISODFName are present. */
#define PHAL_ISO7816_ISO_FILE_ID_AVAILABLE          0x01U   /**< Option to indicate the presence of ISO FileID. */
#define PHAL_ISO7816_ISO_DF_NAME_AVAILABLE          0x02U   /**< Option to indicate the presence of ISO DFName. */
#define PHAL_ISO7816_ISO_FILE_ID_DF_NAME_AVAILABLE  0x03U   /**< Option to indicate the presence of both ISO FileID and ISO DFName. */

/* ISO/IEC 7816-4 FCI modes */
#define PHAL_ISO7816_FCI_RETURNED                   0x00U   /**< Option to indicate return of FCI. */
#define PHAL_ISO7816_FCI_NOT_RETURNED               0x0CU   /**< Option to indicate no return of FCI. */

/* ISO/IEC 7816-4 APDU format */
#define PHAL_ISO7816_APDU_FORMAT_SHORT_LEN          0x00U   /**< Option to indicate ISO7816-4 APDU format is Short Length format where LC and LE are of 1 byte. */
#define PHAL_ISO7816_APDU_FORMAT_EXTENDED_LEN       0x01U   /**< Option to indicate ISO7816-4 APDU format is Extended Length format where LC is 3 bytes and LE is either 2 or 3 bytes. */

#define PHAL_ISO7816_VALIDATE_APDU_FORMAT(x)        (x > PHAL_ISO7816_APDU_FORMAT_EXTENDED_LEN) ? \
                                                    (PH_ERR_INVALID_PARAMETER | PH_COMP_AL_TOP) : \
                                                    (PH_ERR_SUCCESS)

/* ISO/IEC 7816-4 Selection Controls */
#define PHAL_ISO7816_SELECTOR_0                     0x00U   /**< Option to indicate Selection by 2 byte file Id. */
#define PHAL_ISO7816_SELECTOR_1                     0x01U   /**< Option to indicate Selection by child DF. */
#define PHAL_ISO7816_SELECTOR_2                     0x02U   /**< Option to indicate Select EF under current DF. Fid = EF id. */
#define PHAL_ISO7816_SELECTOR_3                     0x03U   /**< Option to indicate Select parent DF of the current DF. */
#define PHAL_ISO7816_SELECTOR_4                     0x04U   /**< Option to indicate Selection by DF Name. DFName and len is then valid. */

/* ISO7816-4 BER-TLV Length fields */
#define PHAL_ISO7816_BER_TLV_L_00_7F                0x7FU   /**< Length byte(1st byte) indicates length of data field 0x00 to 0x7F */
#define PHAL_ISO7816_BER_TLV_L_81                   0x81U   /**< 1st byte 0x81 indicates 2nd byte is length field varies from 0x00 to 0xFF */
#define PHAL_ISO7816_BER_TLV_L_82                   0x82U   /**< 1st byte 0x82 indicates 2nd and 3rd bytes are length field varies from 0x0000 to 0xFFFF */

/** ISO/IEC 7816-4 command options. This flag will be used to compute the response. */
#define PHAL_ISO7816_OPTION_NONE                            0x00U   /**< Command option as None. This flag is used to discard the processing of reception from PICC. */
#define PHAL_ISO7816_OPTION_COMPLETE                        0x01U   /**< Command option as complete. This flag is used to check the response other than AF. */
#define PHAL_ISO7816_OPTION_PENDING                         0x02U   /**< Command option as complete. This flag is used to check for AF response. */
#define PHAL_ISO7816_NO_RETURN_PLAIN_DATA                   0x00U   /**< No Return plain data from SM application interface in case if communication mode is PLAIN. */
#define PHAL_ISO7816_RETURN_PLAIN_DATA                      0x04U   /**< Return plain data from SM application interface in case if communication mode is PLAIN. */
#define PHAL_ISO7816_PICC_STATUS_WRAPPED                    0x80U   /**< The PICC status is wrapped. */
#define PHAL_ISO7816_EXCLUDE_PICC_STATUS                    0x40U   /**< Exclude removal of status code from actual response length. */
#define PHAL_ISO7816_RETURN_CHAINING_STATUS                 0x20U   /**< Return the chaining status to the user if available. */
#define PHAL_ISO7816_RETURN_PICC_STATUS                     0x10U   /**< Return the PICC status to the caller. */

#define PHAL_ISO7816_AUTH_ISO_GENERAL_AUTH_DO_HDR_TAG       0x7CU   /**< Tag data for ISOGeneral Authenticate AuthDOHdr information. */
#define PHAL_ISO7816_AUTH_ISO_GENERAL_EPH_PUB_KEY_TAG       0x85U   /**< Tag data for ISOGeneral Authenticate Ephemeral PublicKey. */

#define PHAL_ISO7816_AUTH_ISO_GENERAL_ENC_MSG_A_TAG         0x86U   /**< Tag data for ISOGeneral Authenticate Encrypted Message A. */
#define PHAL_ISO7816_AUTH_ISO_GENERAL_ENC_MSG_B_TAG         0x82U   /**< Tag data for ISOGeneral Authenticate Encrypted Message B. */

#define PHAL_ISO7816_AUTH_ISO_INTERNAL_AUTH_DO_HDR_TAG      0x7CU   /**< Tag data for ISOInternal Authenticate AuthDOHdr information. */
#define PHAL_ISO7816_AUTH_ISO_INTERNAL_RNDA_TAG             0x81U   /**< Tag data for ISOInternal Authenticate RndA information. */
#define PHAL_ISO7816_AUTH_ISO_INTERNAL_SIGNATURE_TAG        0x82U   /**< Tag data for ISOInternal Authenticate Signature information. */

#define PHAL_ISO7816_CMD_INVALID                            0xFFU   /**< INVALID Command code. */
#define PHAL_ISO7816_ADDITIONAL_FRAME                       0xAFU   /**< Command / Response code for Additional Frame. */
#define PHAL_ISO7816_WRAPPED_HDR_LEN_NORMAL                 0x05U   /**< ISO7816 Header length. */
#define PHAL_ISO7816_WRAPPED_HDR_LEN_EXTENDED               0x08U   /**< ISO7816 Header length in Extended Mode. */
#define PHAL_ISO7816_GENERIC_CLA                            0x00U   /**< Class for Generic ISO7816 commands. */
#define PHAL_ISO7816_WRAPPEDAPDU_CLA                        0x90U   /**< Wrapped APDU code for class. */
#define PHAL_ISO7816_WRAPPEDAPDU_P1                         0x00U   /**< Wrapped APDU code for default P1. */
#define PHAL_ISO7816_WRAPPEDAPDU_P2                         0x00U   /**< Wrapped APDU code for default P2. */
#define PHAL_ISO7816_WRAPPEDAPDU_LC                         0x00U   /**< Wrapped APDU code for default LC. */
#define PHAL_ISO7816_WRAPPEDAPDU_LE                         0x00U   /**< Wrapped APDU code for default LE. */

#define PHAL_ISO7816_LC_POS                                 0x04U   /**< Position of LC in ISO7816 format. */
#define PHAL_ISO7816_EXCHANGE_LC_ONLY                       0x01U   /**< Option to indicate only LC should be exchanged to PICC and LE should not be exchanged. */
#define PHAL_ISO7816_EXCHANGE_LE_ONLY                       0x02U   /**< Option to indicate only LE should be exchanged to PICC and LC should not be exchanged. */
#define PHAL_ISO7816_EXCHANGE_LC_LE_BOTH                    0x03U   /**< Option to indicate both LC and LE should be exchanged to PICC. */

#define PHAL_ISO7816_KEYSETT3_PRESENT                       0x10U   /**< Bit 4 of bKeySettings2 decides the presence of the keysetting3. */
#define PHAL_ISO7816_KEYSETVALUES_PRESENT                   0x01U   /**< Bit 0 of bKeySettings3 decides the presence of the key set values array. */
#define PHAL_ISO7816_COMM_OPTIONS_MASK                      0xF0U   /**< Masking out communication options. */
#define PHAL_ISO7816_OPTION_MASK                            0x7FU   /**< Masking out Options. */

#define PHAL_ISO7816_PRODUCT_CMD                            0x00U   /**< Option to indicate ISO7816 product commands. */
#define PHAL_ISO7816_STANDARD_CMD                           0x01U   /**< Option to indicate ISO7816 commands. */

#define PHAL_ISO7816_SESSION_MAC                            0x01U   /**< Session key option for Macing of data. */
#define PHAL_ISO7816_SESSION_ENC                            0x02U   /**< Session key option for Encryption / Decryption of data. */

/* Native ISO 7816 Response Codes. */
#define PHAL_ISO7816_SUCCESS                                0x9000U /**< ISO7816 Response - Correct execution. */
#define PHAL_ISO7816_ERR_WRONG_LENGTH                       0x6700U /**< ISO7816 Response - Wrong length. */
#define PHAL_ISO7816_ERR_INVALID_APPLN                      0x6A82U /**< ISO7816 Response - Application / file not found. */
#define PHAL_ISO7816_ERR_WRONG_PARAMS                       0x6A86U /**< ISO7816 Response - Wrong parameters P1 and/or P2. */
#define PHAL_ISO7816_ERR_WRONG_LC                           0x6A87U /**< ISO7816 Response - Lc inconsistent with P1/p2. */
#define PHAL_ISO7816_ERR_WRONG_LE                           0x6C00U /**< ISO7816 Response - Wrong Le. */
#define PHAL_ISO7816_ERR_NO_PRECISE_DIAGNOSTICS             0x6F00U /**< ISO7816 Response - No precise diagnostics. */
#define PHAL_ISO7816_ERR_EOF_REACHED                        0x6282U /**< ISO7816 Response - End of File reached. */
#define PHAL_ISO7816_ERR_FILE_ACCESS                        0x6982U /**< ISO7816 Response - File access not allowed. */
#define PHAL_ISO7816_ERR_FILE_EMPTY                         0x6985U /**< ISO7816 Response - File empty or access conditions not satisfied. */
#define PHAL_ISO7816_ERR_FILE_NOT_FOUND                     0x6A82U /**< ISO7816 Response - File not found. */
#define PHAL_ISO7816_ERR_MEMORY_FAILURE                     0x6581U /**< ISO7816 Response - Memory failure (unsuccessful update). */
#define PHAL_ISO7816_ERR_INCORRECT_PARAMS                   0x6B00U /**< ISO7816 Response - Wrong parameter p1 or p2. READ RECORDS. */
#define PHAL_ISO7816_ERR_WRONG_CLA                          0x6E00U /**< ISO7816 Response - Wrong Class byte. */
#define PHAL_ISO7816_ERR_UNSUPPORTED_INS                    0x6D00U /**< ISO7816 Response - Instruction not supported. */
#define PHAL_ISO7816_ERR_LIMITED_FUNCTIONALITY_INS          0x6283U /**< ISO7816 Response - Limited Functionality. */

/* ISO7816-4 Native Response Codes. */
#define PHAL_ISO7816_RESP_OPERATION_OK                      0x00U   /**< Successful operation. */
#define PHAL_ISO7816_RESP_COMMAND_ABORTED                   0xCAU   /**< Previous cmd not fully completed. Not all frames were requested or provided by the PCD. */
#define PHAL_ISO7816_RESP_OUT_OF_EEPROM_ERROR               0x0EU   /**< Insufficient NV-Memory. */
#define PHAL_ISO7816_RESP_ILLEGAL_COMMAND_CODE              0x1CU   /**< Command code not supported. */
#define PHAL_ISO7816_RESP_INTEGRITY_ERROR                   0x1EU   /**< CRC or MAC does not match data padding bytes not valid. */
#define PHAL_ISO7816_RESP_PERMISSION_DENIED                 0x9DU   /**< Current configuration/status does not allow the requested command. */
#define PHAL_ISO7816_RESP_PERMISSION_DENIED                 0x9DU   /**< Current configuration/status does not allow the requested command. */
#define PHAL_ISO7816_RESP_PARAMETER_ERROR                   0x9EU   /**< Value of params invalid. */
#define PHAL_ISO7816_RESP_DUPLICATE                         0xDEU   /**< File/Application with same number already exists. */
#define PHAL_ISO7816_RESP_MEMORY_ERROR                      0xEEU   /**< Could not complete NV-Write operation due to loss of power. */

/* CommandCodes */
#define PHAL_ISO7816_CMD_CREATE_APPLICATION                 0xCA    /**< Create Application command code. */
#define PHAL_ISO7816_CMD_DELETE_APPLICATION                 0xDA    /**< Delete Application command code. */
#define PHAL_ISO7816_CMD_CREATE_DELEGATED_APPLICATION       0xC9    /**< Create Delegated Application command code. */
#define PHAL_ISO7816_CMD_SELECT_APPLICATION                 0x5A    /**< Select Application command code. */
#define PHAL_ISO7816_CMD_GET_APPLICATION_IDS                0x6A    /**< Get Application Ids command code. */
#define PHAL_ISO7816_CMD_GET_DF_NAMES                       0x6D    /**< Get Dedicated File names command code. */
#define PHAL_ISO7816_CMD_GET_DELEGATED_INFO                 0x69    /**< Get Delegated info command code. */

/* Error Codes mapping to custom values. */
#define PHAL_ISO7816_ERR_NO_CHANGES                         (PH_ERR_CUSTOM_BEGIN + 0U)   /**< ISO7816 Custom error code - No changes done to backup files.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_NO_CHANGES error.
                                                                                         */
#define PHAL_ISO7816_ERR_OUT_OF_EEPROM                      (PH_ERR_CUSTOM_BEGIN + 1U)   /**< ISO7816 Custom error code - Insufficient NV-Memory.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_OUT_OF_EEPROM_ERROR error.
                                                                                         */
#define PHAL_ISO7816_ERR_NO_SUCH_KEY                        (PH_ERR_CUSTOM_BEGIN + 2U)   /**< ISO7816 Custom error code - Invalid key number specified.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_NO_SUCH_KEY error.
                                                                                         */
#define PHAL_ISO7816_ERR_PERMISSION_DENIED                  (PH_ERR_CUSTOM_BEGIN + 3U)   /**< ISO7816 Custom error code - Current configuration/wStatus does not allow the requested command.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_PERMISSION_DENIED error.
                                                                                         */
#define PHAL_ISO7816_ERR_APPLICATION_NOT_FOUND              (PH_ERR_CUSTOM_BEGIN + 4U)   /**< ISO7816 Custom error code - Requested AID not found on PICC.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_APPLICATION_NOT_FOUND error.
                                                                                         */
#define PHAL_ISO7816_ERR_BOUNDARY_ERROR                     (PH_ERR_CUSTOM_BEGIN + 5U)   /**< ISO7816 Custom error code - Attempt to read/write data from/to beyond the files/record's limits.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_BOUNDARY_ERROR error.
                                                                                         */
#define PHAL_ISO7816_ERR_COMMAND_ABORTED                    (PH_ERR_CUSTOM_BEGIN + 6U)   /**< ISO7816 Custom error code - Previous cmd not fully completed. Not all frames were requested or
                                                                                         *   provided by the PCD. This error represents PICC's #PHAL_ISO7816_RESP_COMMAND_ABORTED error.
                                                                                         */
#define PHAL_ISO7816_ERR_DUPLICATE                          (PH_ERR_CUSTOM_BEGIN + 7U)   /**< ISO7816 Custom error code - File/Application with same number already exists.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_DUPLICATE error.
                                                                                         */
#define PHAL_ISO7816_ERR_FILE_NOT_EXIST                     (PH_ERR_CUSTOM_BEGIN + 8U)   /**< ISO7816 Custom error code - Specified file number does not exist.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_FILE_NOT_FOUND error.
                                                                                         */
#define PHAL_ISO7816_ERR_PICC_CRYPTO                        (PH_ERR_CUSTOM_BEGIN + 9U)   /**< ISO7816 Custom error code - Crypto error returned by PICC.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_INTEGRITY_ERROR error.
                                                                                         */
#define PHAL_ISO7816_ERR_PARAMETER_ERROR                    (PH_ERR_CUSTOM_BEGIN + 10U)  /**< ISO7816 Custom error code - Parameter value error returned by PICC.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_PARAMETER_ERROR error.
                                                                                         */
#define PHAL_ISO7816_ERR_DF_GEN_ERROR                       (PH_ERR_CUSTOM_BEGIN + 11U)  /**< ISO7816 Custom error code - MIFARE ECC Generic error.
                                                                                         *   Refer \ref phalISO7816_GetConfig with #PHAL_ISO7816_ADDITIONAL_INFO as config option to get the
                                                                                         *   exact error code. This code will be returned for the below PICC Error codes.
                                                                                         *      - #PHAL_ISO7816_RESP_MEMORY_ERROR
                                                                                         *      - #PHAL_ISO7816_RESP_ILLEGAL_COMMAND_CODE
                                                                                         */
#define PHAL_ISO7816_ERR_DF_7816_GEN_ERROR                  (PH_ERR_CUSTOM_BEGIN + 12U)  /**< ISO7816 Custom error code - MIFARE ECC ISO 7816 Generic error.
                                                                                         *   Refer \ref phalISO7816_GetConfig with #PHAL_ISO7816_ADDITIONAL_INFO as config option to get the
                                                                                         *   exact error code. This code will be returned for the below PICC Error codes.
                                                                                         *      - #PHAL_ISO7816_ERR_WRONG_LENGTH
                                                                                         *      - #PHAL_ISO7816_ERR_INVALID_APPLN
                                                                                         *      - #PHAL_ISO7816_ERR_WRONG_PARAMS
                                                                                         *      - #PHAL_ISO7816_ERR_WRONG_LC
                                                                                         *      - #PHAL_ISO7816_ERR_WRONG_LE
                                                                                         *      - #PHAL_ISO7816_ERR_NO_PRECISE_DIAGNOSTICS
                                                                                         *      - #PHAL_ISO7816_ERR_EOF_REACHED
                                                                                         *      - #PHAL_ISO7816_ERR_FILE_ACCESS
                                                                                         *      - #PHAL_ISO7816_ERR_FILE_EMPTY
                                                                                         *      - #PHAL_ISO7816_ERR_FILE_NOT_FOUND
                                                                                         *      - #PHAL_ISO7816_ERR_MEMORY_FAILURE
                                                                                         *      - #PHAL_ISO7816_ERR_INCORRECT_PARAMS
                                                                                         *      - #PHAL_ISO7816_ERR_WRONG_CLA
                                                                                         *      - #PHAL_ISO7816_ERR_UNSUPPORTED_INS
                                                                                         *      - #PHAL_ISO7816_ERR_LIMITED_FUNCTIONALITY_INS
                                                                                         */
#define PHAL_ISO7816_ERR_CMD_INVALID                        (PH_ERR_CUSTOM_BEGIN + 13U)  /**< ISO7816 Custom error code - Command Invalid.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_ERR_CMD_INVALID error.
                                                                                         */
#define PHAL_ISO7816_ERR_NOT_SUPPORTED                      (PH_ERR_CUSTOM_BEGIN + 14U)  /**< ISO7816 Custom error code - Not Supported Error.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_ERR_NOT_SUP error.
                                                                                         */
#define PHAL_ISO7816_ERR_OPERATION_OK_LIM                   (PH_ERR_CUSTOM_BEGIN + 15U)  /**< ISO7816 Custom error code - Successful operation with limited functionality.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_OPERATION_OK_LIM error.
                                                                                         */
#define PHAL_ISO7816_ERR_CMD_OVERFLOW                       (PH_ERR_CUSTOM_BEGIN + 16U)  /**< ISO7816 Custom error code - Too many commands in the session or transaction.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_ERR_CMD_OVERFLOW error.
                                                                                         */
#define PHAL_ISO7816_ERR_GEN_FAILURE                        (PH_ERR_CUSTOM_BEGIN + 17U)  /**< ISO7816 Custom error code - Failure in the operation of the PD.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_ERR_GEN_FAILURE error.
                                                                                         */
#define PHAL_ISO7816_ERR_BNR                                (PH_ERR_CUSTOM_BEGIN + 18U)  /**< ISO7816 Custom error code - Invalid Block number: not existing in the implementation
                                                                                         *   or not valid to target with this command.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_ERR_BNR error.
                                                                                         */
#define PHAL_ISO7816_ERR_FORMAT                             (PH_ERR_CUSTOM_BEGIN + 19U)  /**< ISO7816 Custom error code - Format of the command is not correct (e.g. too many or too few bytes).
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_ERR_FORMAT error.
                                                                                         */
#define PHAL_ISO7816_ERR_CERTIFACATE                        (PH_ERR_CUSTOM_BEGIN + 20U)  /**< ISO7816 Custom error code - Reader certificate or CertAccessRights related error.
                                                                                         *   This error represents PICC's #PHAL_ISO7816_RESP_CERT_ERROR error.
                                                                                         */

phStatus_t phalTop_Sw_Int_T4T_ClearState(
                                         phalTop_Sw_DataParams_t * pDataParams,
                                         phalTop_T4T_t * pT4T
                                         );

phStatus_t phalTop_Sw_Int_T4T_CheckNdef(
                                        phalTop_Sw_DataParams_t * pDataParams,
                                        uint8_t * pTagState
                                        );

phStatus_t phalTop_Sw_Int_T4T_ReadNdef(
                                       phalTop_Sw_DataParams_t * pDataParams,
                                       uint8_t * pData,
                                       uint32_t * pLength
                                       );

phStatus_t phalTop_Sw_Int_T4T_WriteNdef(
                                        phalTop_Sw_DataParams_t * pDataParams,
                                        uint8_t * pData,
                                        uint32_t dwLength
                                        );

phStatus_t phalTop_Sw_Int_T4T_FormatNdef(
                                         phalTop_Sw_DataParams_t * pDataParams
                                         );

phStatus_t phalTop_Sw_Int_T4T_EraseNdef(
                                        phalTop_Sw_DataParams_t * pDataParams
                                        );

#endif /* NXPBUILD__PHAL_TOP_T4T_SW */
#endif /* NXPBUILD__PHAL_TOP_SW */

#endif /* PHALTOP_SW_INT_T4T_H */
