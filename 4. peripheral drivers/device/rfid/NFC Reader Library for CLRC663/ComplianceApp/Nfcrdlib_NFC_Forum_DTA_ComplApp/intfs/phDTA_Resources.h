/*----------------------------------------------------------------------------*/
/* Copyright 2016-2023 NXP                                                    */
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

/** @file
 *
 * Interface file for resources like global variables and buffers that the DTA Test application uses.
 *
 * $Author: NXP $
 * $Revision: $ (v07.09.00)
 * $Date: $
 */

#ifndef PHDTA_RESOURCES_H_
#define PHDTA_RESOURCES_H_

/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */
#ifdef NXPBUILD__PHHAL_HW_PN7462AU
#include "ph_Datatypes.h"
#endif
#include "ph_Status.h"

#ifdef PHDRIVER_LPC1769
#include <cr_section_macros.h>
#endif /* PHDRIVER_LPC1769 */

/* *****************************************************************************************************************
 * MACROS/Defines
 * ***************************************************************************************************************** */
/* DTA Digital Protocol Test Pattern Numbers */
#define PHDTA_PATTERN_NUM_0           (0x00)     /* Pattern Number 0x00 */
#define PHDTA_PATTERN_NUM_1           (0x01U)    /* Pattern Number 0x01 */
#define PHDTA_PATTERN_NUM_2           (0x02U)    /* Pattern Number 0x02 */
#define PHDTA_PATTERN_NUM_3           (0x03U)    /* Pattern Number 0x03 */
#define PHDTA_PATTERN_NUM_5           (0x05U)    /* Pattern Number 0x05 */
#define PHDTA_PATTERN_NUM_6           (0x06U)    /* Pattern Number 0x06 */
#define PHDTA_PATTERN_NUM_7           (0x07U)    /* Pattern Number 0x07 */
#define PHDTA_PATTERN_NUM_8           (0x08U)    /* Pattern Number 0x08 */
#define PHDTA_PATTERN_NUM_A           (0x0AU)    /* Pattern Number 0x0A */
#define PHDTA_PATTERN_NUM_B           (0x0BU)    /* Pattern Number 0x0B */
#define PHDTA_PATTERN_NUM_11          (0x11U)    /* Pattern Number 0x11 */
#define PHDTA_PATTERN_NUM_12          (0x12U)    /* Pattern Number 0x12 */
#define PHDTA_PATTERN_NUM_21          (0x21U)    /* Pattern Number 0x21 */
#define PHDTA_PATTERN_NUM_31          (0x31U)    /* Pattern Number 0x31 */

/* Codes to indicate the type of Protocol supported by the device detected */
#define PHDTA_PROTOCOL_DETECTED_ISO_DEP   (0x01U)  /* ISO-DEP */
#define PHDTA_PROTOCOL_DETECTED_NONE      (0xFFU)  /* NO devices detected */

/* DTA Application Memory Management Macros */
#if defined(NXPBUILD__PHHAL_HW_PN7462AU) || defined (NXPBUILD__PHHAL_HW_PN76XX)
    #define PHDTA_APP_BUFFER_SIZE         (2U * 1024U)    /* 2K bytes memory allocated for NFC controllers */
#elif defined(PHDRIVER_KINETIS_K82)
    #define PHDTA_APP_BUFFER_SIZE         (128U * 1024U)  /* 128K bytes memory allocated for K82 platform */
#else
    #define PHDTA_APP_BUFFER_SIZE         (32U * 1024U)   /* 32K bytes memory allocated for LPC1769 platform */
#endif /* defined(NXPBUILD__PHHAL_HW_PN7462AU) || defined (NXPBUILD__PHHAL_HW_PN76XX) */

#ifdef PHDRIVER_LPC1769
#define USE_RAM2_32 __DATA(RAM2) /* Note: For LPC1769 platform, RAM2(32KBytes) is used for DTA application memory. */
#endif /* PHDRIVER_LPC1769 */

/* *****************************************************************************************************************
 * External Variables
 * ***************************************************************************************************************** */
/* Global variable to hold the DTA test Pattern Number */
extern uint16_t gwPhDta_PatternNumber;

/* Global variable to save the code corresponding to the Protocol supported by the device detected. */
extern uint8_t gbPhDta_Int_Protocol;

#ifdef NXPBUILD_DTA_READER_DEVICE_CLASS
/* Buffer to hold the response to Activation request(i.e. ATS) */
extern uint8_t gPhDta_ActResponse[];

/* Digital Protocol Tag Test Variables */
extern uint8_t *pgPhDta_TagTest_NdefData;
extern uint32_t dwgPhDta_TagTest_NdefDataLength;

/* Request Code, used in the SENS F Request command */
extern uint8_t gbPhDta_SensFReq_RC;
#endif /* NXPBUILD_DTA_READER_DEVICE_CLASS */

#ifdef NXPBUILD__PHHAL_HW_TARGET
/* Answer to RATS in Listen Mode */
extern const uint8_t gaPhDta_AtsResp[5U];
#endif /* NXPBUILD__PHHAL_HW_TARGET */

/* *****************************************************************************************************************
 * Function Prototypes
 * ***************************************************************************************************************** */
/* Initialize/Reset Memory Management mechanism of DTA Application */
extern void phDta_InitAppMemory (void);

/* Memory allocation API for DTA Tests */
extern phStatus_t phDta_AllocateMemory (uint32_t NumBytes, uint8_t **pBufferPointer);

#endif /* PHDTA_RESOURCES_H_ */
