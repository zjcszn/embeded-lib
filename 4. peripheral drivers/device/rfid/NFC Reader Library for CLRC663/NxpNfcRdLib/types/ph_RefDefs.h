/*----------------------------------------------------------------------------*/
/* Copyright 2009-2020 NXP                                                    */
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
* Platform definitions for Reader Library Framework.
* $Author$
* $Revision$ (v07.09.00)
* $Date$
*
*/

#ifndef PH_REFDEFS_H
#define PH_REFDEFS_H

#include <ph_TypeDefs.h>

/** \defgroup ph_RefDefs Platform Definitions
* \brief Platform depending definitions for memory location of variables.
*
* We distinguish between:
*   - BUFFERs   : Location of storage, usually mid-speed but max. RAM consumption,
*   - COUNTERs  : loop counters or frequently accessed variables, fast access,
*   - REMINDERs : variables holding status values or serving flow control purposes.
*
* If RCL is used on a microcontroller, an own REFDEF.H file must be generated where the
* RCL_MEMLOC_... definition must get according values. Default definitions are empty.
*
* Assignment Example: XDATA for #PH_MEMLOC_BUF,
*                     IDATA for #PH_MEMLOC_COUNT,
*                     EDATA for #PH_MEMLOC_REM.
* @{
*/

/**
*  Space used for storage (e.g. buffers).
*/
#define PH_MEMLOC_BUF   /* */

/**
* Space used for fast and frequent access e.g. counters, loop variables).
*/
#define PH_MEMLOC_COUNT /* */

/**
* Space used for control variables, these could be e.g. status values.
*/
#define PH_MEMLOC_REM   /* */

/**
* Space used for constants, these could be e.g. lookup tables.
*/
#define PH_MEMLOC_CONST_ROM /* */

/** @}
* end of ph_RefDefs group
*/

#endif /*  PH_REFDEFS_H */
