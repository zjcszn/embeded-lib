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
 * Interface file for the DTA one-time Input and Input-Engine Modules
 *
 * $Author: NXP $
 * $Revision: $ (v07.09.00)
 * $Date: $
 */

#ifndef PHDTA_INPUT_H_
#define PHDTA_INPUT_H_

/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */
#ifdef NXPBUILD__PHHAL_HW_PN7462AU
#include "ph_Datatypes.h"
#endif
#include "ph_Status.h"

/* *****************************************************************************************************************
 * MACROS/Defines
 * ***************************************************************************************************************** */
#define PHDTA_INPUTENGINE_GPIO_DEFSTATE    (1U)   /* GPIO default state */

#ifdef NXPBUILD__PHHAL_HW_PN7462AU
#define PHDTA_INPUTENGINE_GPIO_PIN         (1U)   /* GPIO1 */
#endif /* NXPBUILD__PHHAL_HW_PN7462AU */

#ifdef NXPBUILD__PHHAL_HW_PN76XX
/* Configured as per PNEV76XXFAMA Board.
 * User Button SW1(GPIO5) is configured for Pattern number change.
 */
#define PHDTA_INPUTENGINE_GPIO_PIN         (5U)   /* GPIO5 */
#endif /* NXPBUILD__PHHAL_HW_PN76XX */

#if !defined(__PN76XX__)
    #ifdef PHDRIVER_KINETIS_K82
        #if SDK_DEBUGCONSOLE==1
            #define  DTA_PRINT(...) DbgConsole_Printf(__VA_ARGS__)
            #define  DTA_SCAN(...)  DbgConsole_Scanf(__VA_ARGS__)
        #else
            #define  DTA_PRINT(...) printf(__VA_ARGS__);
            #define  DTA_SCAN(...)  scanf(__VA_ARGS__);
        #endif
    #else /* PHDRIVER_KINETIS_K82 */
        #include "stdio.h"
        #define DTA_PRINT(...)    printf(__VA_ARGS__);fflush(stdout)
        #define DTA_SCAN(...)     scanf(__VA_ARGS__);while ((getchar()) != '\n');
    #endif /* PHDRIVER_KINETIS_K82 */
#else /* __PN76XX__ */
    #if (SEGGER_RTT_ENABLE == 1)
        #include "SEGGER_RTT.h"
        #define DTA_PRINT(...) SEGGER_RTT_printf(0, __VA_ARGS__);
        #define DTA_GET_INPUT(data) *data = SEGGER_RTT_WaitKey();
   #elif defined MCUXPRESSO_SDK
		#include <fsl_debug_console.h>
		#define DTA_PRINT(...) PRINTF(__VA_ARGS__)
		#define DTA_SCAN(...) SCANF(__VA_ARGS__);
   #else
        #define DTA_PRINT(...)
        #define DTA_SCAN(...)
    #endif
#endif /* __PN76XX__ */

/* *****************************************************************************************************************
 * Function Prototypes
 * ***************************************************************************************************************** */
/* This function gets the user input and sets global variables and flags that determine the DTA test procedure. */
extern phStatus_t phDta_GetUserInput (void);

/* API to Initialize the Input Engine */
extern phStatus_t phDta_InputEngineInit (void);

/* Input Engine API for checking if User wants to feed DTA input parameters, and get the parameters */
extern void phDta_InputEngine (void);

#endif /* PHDTA_INPUT_H_ */
