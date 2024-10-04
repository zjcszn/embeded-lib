/*----------------------------------------------------------------------------*/
/* Copyright 2010-2022 NXP                                                    */
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
* Internal definitions for Software palFelica Component of Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#ifndef PHPALFELICA_SW_INT_H
#define PHPALFELICA_SW_INT_H

#include <ph_Status.h>

/** \addtogroup ph_Private
* @{
*/

/** \name FeliCa Command Codes
*/
/*@{*/
#define PHPAL_FELICA_SW_CMD_REQC            0x00U   /**< RequestC. */
#define PHPAL_FELICA_SW_CMD_REQ_RESP        0x04U   /**< Request Response. */
#define PHPAL_FELICA_SW_CMD_REQ_SERVICE     0x02U   /**< Request Service. */
#define PHPAL_FELICA_SW_CMD_READ            0x06U   /**< Read. */
#define PHPAL_FELICA_SW_CMD_WRITE           0x08U   /**< Write. */
#define PHPAL_FELICA_SW_CMD_AUTH1           0x10U   /**< Authentication1. */
#define PHPAL_FELICA_SW_CMD_AUTH2           0x12U   /**< Authentication2. */
#define PHPAL_FELICA_SW_CMD_READ_SECF       0x14U   /**< Read from secure file. */
#define PHPAL_FELICA_SW_CMD_WRITE_SECF      0x16U   /**< Write to secure file. */
#define PHPAL_FELICA_SW_RSP_REQC            0x01U   /**< Response to RequestC. */
#define PHPAL_FELICA_SW_LEN_BYTE_SIZE       0x01U   /**< Size of Length byte in RequestC response. */
#define PHPAL_FELICA_SW_RESP_REQC_SIZE      0x01U   /**< Length of Response byte in RequestC response. */
/*@}*/

/** \name Flags
*/
/*@{*/
/** Error Flag. */
#define PHPAL_FELICA_SW_FLAG_ERROR      0x01U
/** Addressed Flag. */
#define PHPAL_FELICA_SW_FLAG_ADDRESSED  0x20U
/** Selected Flag. */
#define PHPAL_FELICA_SW_FLAG_SELECTED   0x10U
/*@}*/

/** \name Values
*/
/*@{*/
/** Stored UID is valid. */
#define PHPAL_FELICA_SW_IDMPMM_VALID    0x01U
/** Stored UID is not valid. */
#define PHPAL_FELICA_SW_IDMPMM_INVALID  0x00U
/** Size of one FeliCa Response Frame. */
#define PHPAL_FELICA_SW_RESP_FRAME_SIZE 32U
/*@}*/

/** \name Timing constants
*/
/*@{*/
#define PHPAL_FELICA_SW_TIME_T_US               302U
#define PHPAL_FELICA_SW_EXT_TIME_US             500U     /**< Time extension in microseconds for all commands.  */
#define PHPAL_FELICA_SW_DELTA_TIME_T_US         15U      /**< MRT(Max Response Time) Check Update Timeout Delta value. */

#define PHPAL_FELICA_SW_T_DELAY_US              2417U    /**< NFC Forum Digital Protocol Technical Specification v2.3 : Appendix B.4 */
#define PHPAL_FELICA_SW_T_TIMESLOT_US           1208U    /**< NFC Forum Digital Protocol Technical Specification v2.3 : Appendix B.4 */
#define PHPAL_FELICA_SW_T_DELTA_F_POLL_US       1U       /**< Delta T_POLL as per NFC Forum Digital Protocol Spec v2.3 is 1000us.
                                                              This value can be configured from 0us to 1000us based on performance req. */

#define PHPAL_FELICA_SW_TR1_48BITS_AT212_US     227U     /**< NFC Forum Digital Protocol Technical Specification v2.3 : 8.1.2 */
#define PHPAL_FELICA_SW_TR1_56BITS_AT212_US     255U
#define PHPAL_FELICA_SW_TR1_64BITS_AT212_US     302U
#define PHPAL_FELICA_SW_TR1_72BITS_AT212_US     340U

#define PHPAL_FELICA_SW_TR1_48BITS_AT424_US     114U     /**< NFC Forum Digital Protocol Technical Specification v2.3 : 8.1.2 */
#define PHPAL_FELICA_SW_TR1_56BITS_AT424_US     128U
#define PHPAL_FELICA_SW_TR1_64BITS_AT424_US     151U
#define PHPAL_FELICA_SW_TR1_72BITS_AT424_US     170U

/*@}*/

/** @}
* end of ph_Private
*/

#endif /* PHPALFELICA_SW_INT_H */
