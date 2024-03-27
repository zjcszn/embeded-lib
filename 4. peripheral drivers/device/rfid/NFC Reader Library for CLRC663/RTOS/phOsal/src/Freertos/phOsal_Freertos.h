/*----------------------------------------------------------------------------*/
/* Copyright 2016,2017,2019,2021 NXP                                          */
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
* Interface file of Operating System Abstraction Layer for FreeRTOS.
* $Author: NXP $
* $Revision: $
* $Date: $
*/

#ifndef PHOSAL_FREERTOS_H
#define PHOSAL_FREERTOS_H

#include <FreeRTOS.h>

#define PHOSAL_FREERTOS_ALL_EVENTS      0x00FFFFFF

#define PHOSAL_MAX_DELAY      portMAX_DELAY

#if( configSUPPORT_STATIC_ALLOCATION == 1 )
    #define PHOSAL_FREERTOS_STATIC_MEM_ALLOCATION
#endif /* ( configSUPPORT_STATIC_ALLOCATION == 1 ) */


/**
 * xPortIsInsideInterrupt is required by FreeRTOS OSAL wrapper. FreeRTOS port provides xPortIsInsideInterrupt definition for
 * Cortex-M3, M4 only. For cortex-M0 xPortIsInsideInterrupt is implemented here.
 */
#if (defined(CORE_M0) || defined(__PN74XXXX__) || defined (__PN73XXXX__))

/*Interrupt Control and State Register*/
#define PHOSAL_NVIC_INT_CTRL        ( (volatile uint32_t *) 0xe000ed04 )
#define PHOSAL_NVIC_VECTACTIVE      (0x0000003FU)

#define xPortIsInsideInterrupt()    (((*(PHOSAL_NVIC_INT_CTRL) & PHOSAL_NVIC_VECTACTIVE ) == 0)? pdFALSE : pdTRUE)

#endif /* (defined(CORE_M0) || defined(__PN74XXXX__) || defined (__PN73XXXX__)) */



#endif /* PHOSAL_FREERTOS_H */
