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

/** \file
* Example Source Header for NFC Type 4A Card Emulation application.
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*/


#ifndef EX8_HCE_T4T_H
#define EX8_HCE_T4T_H

/* Reader library layers required for this example */
#if (defined(NXPBUILD__PHHAL_HW_PN5180)                     \
    || defined(NXPBUILD__PHHAL_HW_PN5190)                   \
    || defined(NXPBUILD__PHHAL_HW_PN76XX)                   \
    || defined(NXPBUILD__PHHAL_HW_PN7462AU))                \
    && defined(NXPBUILD__PHHAL_HW)                          \
    && defined(NXPBUILD__PHPAL_I14443P4MC_SW)               \
    && defined(NXPBUILD__PHAC_DISCLOOP_SW)                  \
    && defined(NXPBUILD__PHAC_DISCLOOP_TARGET)              \
    && (defined(PH_OSAL_FREERTOS)                           \
    || defined (PH_OSAL_LINUX))                             \
    && defined(NXPBUILD__PHCE_T4T_SW)

    #define NXPBUILD_EX8_MANDATORY_LAYERS      1U
#else
    #define NXPBUILD_EX8_MANDATORY_LAYERS      0U
#endif

/* Check if all required reader library layers are enabled. */
#if NXPBUILD_EX8_MANDATORY_LAYERS

/*******************************************************************************
**  Macros
*******************************************************************************/
/* Set this to 1 for running ISO 10373-6 PICC test bench. When set to 1, code
 * for creating WTX (for TEST_COMMAND3) in ISO test bench will be enabled. */
#define ISO_10373_6_PICC_TEST_BENCH 0

/* Use application callback for processing UPDATE BINARY and proprietary
 * commands */
#define USE_APP_CALLBACK 1

/* Default CET4T NDEF message mapping version configured to 2.0
 * Note: User need to update this macro to PHCE_T4T_NDEF_SUPPORTED_VNO to configure NDEF message mapping version to 3.0
 * */
#define NXPBUILD_EX8_CET4T_NDEF_VERSION  PHCE_T4T_NDEF_SUPPORTED_VNO_20

/* Get content length from CC file. Initial content length is 0. */
#define GET_CCFILE_CONTENT_LEN(x) ((((uint16_t)(x[0]) << 8) & 0xFF00) | (x[1]))

/* Get content length specified in file (first 2 bytes of file) for NDEF and
 * proprietary files */

#define GET_EXT_FILE_CONTENT_LEN(x) ((((((uint32_t)(x[0]) << 24) & 0xFF000000) | (((uint32_t)(x[1]) << 16) & 0xFF0000) | (((uint32_t)(x[2]) << 8) & 0xFF00) | (x[3])) > 0)?   \
                                 (((((uint32_t)(x[0]) << 24) & 0xFF000000) | (((uint32_t)(x[1]) << 16) & 0xFF0000) | (((uint32_t)(x[2]) << 8) & 0xFF00) | (x[3])) + 4) : 0)

#define GET_FILE_CONTENT_LEN(x) ((((((uint16_t)(x[0]) << 8) & 0xFF00) | (x[1])) > 0)?   \
                                 (((((uint16_t)(x[0]) << 8) & 0xFF00) | (x[1])) + 2) : 0)

/* Enable(1) / Disable(0) printing error/info */
#define DETECT_ERROR 0

#if DETECT_ERROR
    #define DEBUG_ERROR_PRINT CHECK_STATUS
    #define PRINT_INFO(...) DEBUG_PRINTF(__VA_ARGS__)
#else /* DETECT_ERROR */
    #define DEBUG_ERROR_PRINT(x)
    #define PRINT_INFO(...)
#endif /* DETECT_ERROR */

/* Print system critical errors (with which execution can't proceed further)
 * and halt the execution. */
#define ERROR_CRITICAL(...) DEBUG_PRINTF(__VA_ARGS__); while(1) { /* Nothing to do */ }

#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define RDLIB_TASK_STACK                (1800/4)
        #define APPLICATION_TASK_STACK          (600/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if defined( __PN74XXXX__) || defined(__PN76XX__)
            #define RDLIB_TASK_STACK            (1600/4)
            #define APPLICATION_TASK_STACK      (600/4)
        #else /* #if  defined( __PN74XXXX__) || defined(__PN76XX__) */
            #define RDLIB_TASK_STACK            (1650)
            #define APPLICATION_TASK_STACK      (600)
        #endif /* #if  defined( __PN74XXXX__) || defined(__PN76XX__) */
    #endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
    #define RDLIB_TASK_PRIO                     2
    #define APPLICATION_TASK_PRIO               1
#endif /* PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
    #define RDLIB_TASK_STACK            0x20000
    #define APPLICATION_TASK_STACK      0x20000
    #define RDLIB_TASK_PRIO             0
#define APPLICATION_TASK_PRIO           0

#endif /* PH_OSAL_LINUX */


/*******************************************************************************
**  Forward declarations
*******************************************************************************/
/*
 * WTX Callback called from WTX timer of 14443p3mC PAL.
 */
void pWtoxCallBck(void);

#if USE_APP_CALLBACK
/**
 * Application callback to handle UPDATE BINARY and proprietary commands.
 * Refer phceT4T_AppCallback_t in phceT4T.h for more info.
 * */
phStatus_t AppProcessCmdCallback(
    uint8_t bState,        /* [in] Tag State indicating received command type */
    uint16_t wRxOption,    /* [in] Indicating received data is partial or not */
    uint8_t *pRxData,      /* [in] Received Data */
    uint16_t wRxDataLen,   /* [in] Length of received data */
    uint16_t *pStatusWord, /* [out] Status Word to be sent (part of R-APDU) */
    uint8_t **ppTxData,    /* [out] Data to be transmitted (part of R-APDU) */
    uint16_t *pTxDataLen   /* [out] Length of data to be transmitted */
    );
#endif /* USE_APP_CALLBACK */

/**
 * Application thread. This calls phceT4T_AppProcessCmd() to handle update
 * binary and proprietary commands.
 * Refer phceT4T_AppProcessCmd in phceT4T.h for more info.
 * APIs which does PAL or HAL exchange should not be called from application
 * thread.
 * */
void TApplication(
    void *pParams);        /* [in] Input parameter to thread (not used here) */

/**
 * Do card emulation by starting discovery loop in listen mode and activating
 * ISO 14443p4mC.
 * Refer phceT4T_Activate() in phceT4T.h for info.
 * */
void T4TCardEmulation(void);

/**
 * Reader library thread. This is the main thread which starts discovery loop
 * in listen mode.
 * */
void TReaderLibrary(
    void *pParams);        /* [in] Input parameter to thread (not used here) */

/**
 * Initialize/Configure components required by this Example.
 * */
void phApp_Initialize(void);

/**
 * Configure discovery loop parameters.
 * */
phStatus_t ConfigureCardEmulation(void);

#endif /* NXPBUILD_EX8_MANDATORY_LAYERS */

#endif /* EX8_HCE_T4T_H */
