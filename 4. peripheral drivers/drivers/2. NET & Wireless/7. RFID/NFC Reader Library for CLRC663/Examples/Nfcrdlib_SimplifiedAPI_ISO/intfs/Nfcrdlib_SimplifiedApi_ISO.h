/*----------------------------------------------------------------------------*/
/* Copyright 2017-2021 NXP                                                    */
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
* Example Source Header for Nfcrdlib_SimplifiedApi_ISO application.
*
* $Author: NXP $
* $Revision: $ (v07.09.00)
* $Date: $
*/

#ifndef INTFS_NFCRDLIB_SIMPLIFIEDAPI_ISO_H_
#define INTFS_NFCRDLIB_SIMPLIFIEDAPI_ISO_H_
#include <phApp_Init.h>
#include <Nfcrdlib_SimplifiedApi_ISO_15693.h>
#include <Nfcrdlib_SimplifiedApi_ISO_18000.h>
#include <Nfcrdlib_SimplifiedApi_ISO_MFUL.h>
#include <Nfcrdlib_SimplifiedApi_ISO_MFC.h>

uint32_t NfcLib_MifareClassic_1k_Reference_app(void);
uint32_t NfcLib_ISO18000p3m3_Reference_app(void);
uint32_t NfcLib_ISO15693_Reference_app(void);
uint32_t NfcLib_MifareDesfire_Reference_app(void);
uint32_t NfcLib_MifareUltralight_Reference_app(void);
uint32_t NfcLib_TypeB_Reference_app(void);
uint32_t NfcLib_Layer4TypeA_Reference_app(void);


#ifdef PH_OSAL_FREERTOS
    #ifdef PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
        #define SIMPLIFIED_ISO_STACK        (2200/4)
    #else /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION */
        #if defined( __PN74XXXX__) || defined(__PN76XX__)
            #define SIMPLIFIED_ISO_STACK    (2000/4)
        #else /* defined( __PN74XXXX__) || defined(__PN76XX__) */
            #define SIMPLIFIED_ISO_STACK    (2050)
        #endif /* defined( __PN74XXXX__) || defined(__PN76XX__) */
    #endif /* PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION*/
    #define SIMPLIFIED_ISO_PRIO             4
#endif /* PH_OSAL_FREERTOS */

#ifdef PH_OSAL_LINUX
#define SIMPLIFIED_ISO_PRIO     0
#define SIMPLIFIED_ISO_STACK    0x20000
#endif /* PH_OSAL_LINUX*/


#endif /* INTFS_NFCRDLIB_SIMPLIFIEDAPI_ISO_H_ */
