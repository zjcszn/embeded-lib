/*----------------------------------------------------------------------------*/
/* Copyright 2017-2022 NXP                                                    */
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
* Generic phDriver Component of Reader Library Framework.
* $Author$
* $Revision$
* $Date$
*
*/

#ifndef BOARDSELECTION_H
#define BOARDSELECTION_H

/* Select one of the boards through defined symbols (-D).
 * Board Header files. */

#ifdef PHDRIVER_LPC1769PN5180_BOARD
#    include <Board_Lpc1769Pn5180.h>
#endif

#ifdef PHDRIVER_PIPN5180_BOARD
#    include <Board_PiPn5180.h>
#endif

#ifdef PHDRIVER_FRDM_K82FPN5180_BOARD
#    include <Board_FRDM_K82FPn5180.h>
#endif

#ifdef PHDRIVER_LPC1769RC663_BOARD
#    include <Board_Lpc1769Rc663.h>
#endif

#ifdef PHDRIVER_FRDM_K82FRC663_BOARD
#    include <Board_FRDM_K82FRc663.h>
#endif

#ifdef PHDRIVER_PIRC663_BOARD
#    include <Board_PiRc663.h>
#endif

#ifdef PHDRIVER_LPC1769PN5190_BOARD
#    include <Board_Lpc1769Pn5190.h>
#endif

#ifdef PHDRIVER_LPC1769_MBDB_BOARD
#    include <Board_Lpc1769_MBDB.h>
#endif

#ifdef PHDRIVER_K82F_PNEV5190B_BOARD
#    include <Board_K82F_PNEV5190B.h>
#endif

#ifdef PHDRIVER_PIPN5190_BOARD
#    include <Board_PiPn5190.h>
#endif
#endif /* BOARDSELECTION_H */

