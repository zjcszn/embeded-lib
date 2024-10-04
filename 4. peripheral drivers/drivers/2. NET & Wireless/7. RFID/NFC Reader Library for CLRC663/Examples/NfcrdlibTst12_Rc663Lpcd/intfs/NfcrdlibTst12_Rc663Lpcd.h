/*
*         Copyright (c), NXP Semiconductors Gratkorn / Austria
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
* particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

/** \file
* Rc663 Lpcd Test Application.
* $Author: Ankur Srivastava (nxp79569) $
* $Revision: 6170 $ (v07.09.00)
* $Date: 2016-09-23 20:23:49 +0530 (Fri, 23 Sep 2016) $
*/

#ifndef NFCRDLIBTST12_RC663LPCD
#define NFCRDLIBTST12_RC663LPCD

/**
* Reader Library Headers
*/
#include <ph_Status.h>

/*******************************************************************************
**  Macros
*******************************************************************************/

/* Print system critical errors (with which execution can't proceed further)
 * and halt the execution. */

#ifdef PH_OSAL_FREERTOS

#define EX11_LPCD_TASK_STACK              (2000/4)
#define EX11_LPCD_TASK_PRIO                4

#elif defined PH_OSAL_LINUX

#define EX11_LPCD_TASK_STACK              0x20000
#define EX11_LPCD_TASK_PRIO                0

#endif /* PH_OSAL_FREERTOS || PH_OSAL_LINUX */

/* Different Options. */
/* Test Case Options. */

/* All test cases will be ran one after the other. */
#define EX11_LPCD_TC_OPTION_DEFAULT     0

/* There are 6 scenarios in Functional Test Cases */
#define EX11_LPCD_TC_FUNCTIONAL         1

/* 3 scenarios of functional test cases are used with varying Rf On and StandBy Time for Rf/Power Test Cases.
 * Scenarios 2, 4 and 5 are used for Rf/Power Test Cases. */
#define EX11_LPCD_TC_TIMING             2

/* Test Case Scenarios. */

/* All test cases scenarios will be ran for the selected option one after the other. */
#define EX11_LPCD_TC_SCENARIO_DEFAULT   0

/* Scenario 1 ==>
 *  DIGITAL FILTER  |   CHARGE PUMP   |   DETECTION OPTION
 *   disabled(0)    |   disabled(0)   |         na(x)
 */
#define EX11_LPCD_TC_SCENARIO_1         1

/* Scenario 2 ==>
 *  DIGITAL FILTER  |   CHARGE PUMP   |   DETECTION OPTION
 *   disabled(0)    |    enabled(1)   |         na(x)
 */
#define EX11_LPCD_TC_SCENARIO_2         2

/* Scenario 3 ==>
 *  DIGITAL FILTER  |   CHARGE PUMP   |   DETECTION OPTION
 *    enabled(1)    |   disabled(0)   |     Option 1
 */
#define EX11_LPCD_TC_SCENARIO_3         3

/* Scenario 4 ==>
 *  DIGITAL FILTER  |   CHARGE PUMP   |   DETECTION OPTION
 *    enabled(1)    |   disabled(0)   |     Option 2
 */
#define EX11_LPCD_TC_SCENARIO_4         4

/* Scenario 5 ==>
 *  DIGITAL FILTER  |   CHARGE PUMP   |   DETECTION OPTION
 *    enabled(1)    |    enabled(1)   |     Option 1
 */
#define EX11_LPCD_TC_SCENARIO_5         5

/* Scenario 6 ==>
 *  DIGITAL FILTER  |   CHARGE PUMP   |   DETECTION OPTION
 *    enabled(1)    |    enabled(1)   |     Option 2
 */
#define EX11_LPCD_TC_SCENARIO_6         6

/* Scenario 7 ==>
 * This scenario covers the test scenario of Calibration performed with load on the antennae and the load is unloaded during Lpcd Loop from the antennae.
 * The configuration of this scenario is as follows :-
 *  DIGITAL FILTER  |   CHARGE PUMP   |   DETECTION OPTION
 *   disabled(1)    |   disabled(1)   |         na(x)
 */
#define EX11_LPCD_TC_SCENARIO_7         7

/**
 * Reader library thread. This is the main thread which configures the Hal for LPCD and performs LPCD.
 * */
void Tst12_LpcdThread(void * pParam);

#endif /* NFCRDLIBTST12_RC663LPCD */
