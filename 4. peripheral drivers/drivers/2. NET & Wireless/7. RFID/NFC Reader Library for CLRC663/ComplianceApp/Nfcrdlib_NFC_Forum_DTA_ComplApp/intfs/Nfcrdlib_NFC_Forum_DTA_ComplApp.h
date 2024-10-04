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
 * Interface file for the DTA Test Application
 *
 * $Author: NXP $
 * $Revision: $ (v07.09.00)
 * $Date: $
 */

#ifndef NFCRDLIB_NFC_FORUM_DTA_COMPLAPP_H
#define NFCRDLIB_NFC_FORUM_DTA_COMPLAPP_H

/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */
#ifdef NXPBUILD__PHHAL_HW_PN7462AU
#include "ph_Datatypes.h"
#endif
#include "ph_Status.h"

#include "phDTA_Resources.h"
#include "phDTA_Input.h"
#include "phDTA_Output.h"

/* Parameters for creating the Main Loop Task */
#ifdef NXPBUILD__PHHAL_HW_PN7462AU
    #define PHDTA_MAINLOOP_STACKSIZE   (250U)
    #define aDTAMainTaskBuffer         NULL
#elif defined (NXPBUILD__PHHAL_HW_PN76XX)
    #define PHDTA_MAINLOOP_STACKSIZE   (1600/4)
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        uint32_t aDTAMainTaskBuffer[PHDTA_MAINLOOP_STACKSIZE];
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #define aDTAMainTaskBuffer     NULL
    #endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
#else
    #define PHDTA_MAINLOOP_STACKSIZE   (2000U)
    #define aDTAMainTaskBuffer         NULL
#endif

#define PHDTA_MAINLOOP_TASKPRIORITY    (3U)

/* *****************************************************************************************************************
 * MACROS/Defines
 * ***************************************************************************************************************** */
#define PHDTA_CON_DEVICES_LIMIT    (2U)      /* Number of NFCIDx Device identifiers that need to be resolved when a collision
											    is detected and pending. */
#define PHDTA_TYPEA_I3P4_RATS_FSDI (0x08U)   /* FSDI for 14443P4A tags. */
#define PHDTA_TYPEA_I3P4_RATS_CID  (0x00)    /* Card Identifier for 14443P4A tags. */
#define PHDTA_TYPEB_ATTRIB_FSDI    (0x08U)   /* FSDI for Type B tags. FSD set to 256 bytes. */
#define PHDTA_TYPEB_ATTRIB_CID     (0x00)    /* Card Identifier (CID/DID) for Type B tags. */

/* *****************************************************************************************************************
 * Function Prototypes
 * ***************************************************************************************************************** */
/* DTA Main Loop implementation API */
extern void phDta_MainLoop (void *pTaskParam);

/* This function performs the Discovery Loop configurations necessary to run the DTA Test profiles. */
extern phStatus_t phDta_DiscLoopConfig (void);

/* DTA Poll Mode Test Module API */
extern void phDta_PollMode (void);

/* T2T Test Module API */
extern void phDta_T2T (void);

/* T3T Test Module API */
extern void phDta_T3T (void);

/* T4T Test Module API */
extern void phDta_T4T (void);

/* DTA Poll Mode T4T Loop Back Test Functionality module */
extern void phDta_PollT4TLoopBack (void);

/* DTA Listen Mode T4T Loop Back Test Functionality module */
extern void phDta_ListenT4TLoopBack (void);

/* T5T Test Module API */
extern void phDta_T5T (phStatus_t dwDiscLoopStatus);

#ifdef NXPBUILD__PHHAL_HW_TARGET
/* DTA Listen Mode Test Module API */
extern void phDta_ListenMode (void);
#endif /* NXPBUILD__PHHAL_HW_TARGET */

#endif /* NFCRDLIB_NFC_FORUM_DTA_COMPLAPP_H */
