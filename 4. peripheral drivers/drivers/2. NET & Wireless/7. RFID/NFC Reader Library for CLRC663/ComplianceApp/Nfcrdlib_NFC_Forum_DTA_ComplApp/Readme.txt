	   __________________________________________________

			 NXPNFCRDLIB EXAMPLE :
		    NFCRDLIB_NFC_FORUM_DTA_COMPLAPP
			      (V07.09.00)
	   __________________________________________________


Table of Contents
_________________

1 Document Purpose
2 Description of the Nfcrdlib_NFC_Forum_DTA_ComplApp
3 Restrictions on Nfcrdlib_NFC_Forum_DTA_ComplApp
4 Configurations of Nfcrdlib_NFC_Forum_DTA_ComplApp
5 Package Contents
6 Mandatory materials (not included)
7 Hardware Configuration
8 Software Configuration
9 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for LPC1769 with PN5190 using MCUXpresso
10 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for LPC1769 with PN5180 using MCUXpresso
11 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for LPC1769 with RC663 using MCUXpresso
12 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for PN7462AU using MCUXpresso
13 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for FRDM-K82F using MCUXpresso
14 Selection of Board / OSAL / Frontend (MCUXpresso)
15 Running Nfcrdlib_NFC_Forum_DTA_ComplApp
16 List of supported NFC Reader Boards/ICs
17 Reference Documents





1 Document Purpose
==================

  This document describes the steps to be followed to execute
  Nfcrdlib_NFC_Forum_DTA_ComplApp application as well as it's known
  problems and restrictions.


2 Description of the Nfcrdlib_NFC_Forum_DTA_ComplApp
====================================================

  - Nfcrdlib_NFC_Forum_DTA_ComplApp can be run with CLEV663B, PN5180,
    PN5190 and PN7462AU.  (See Section-16 for supported versions)
  This application is used to perform NFC Forum Certification Release 13
  Digital/Analog compliance validation.


3 Restrictions on Nfcrdlib_NFC_Forum_DTA_ComplApp
=================================================

  - Nfcrdlib_NFC_Forum_DTA_ComplApp is restricted to be run on NFC
    Reader Boards/ICs mentioned in Section-16.
  - The APIs are intended for NXP contact-less reader ICs only, as such
    the APIs are not to be ported to any technology from any other
    vendor.
  - NXP will not support porting to any other vendor platform.
  - This software project requires NxpNfcRdLib v07.09.00 or later.
  - **IMPORTANT** The default used MCU Type is K82. When executing
    example on LPC1769 or PN7462AU, the MCU Type has to be changed in
    the MCUXpresso IDE.
  - For switching to MCU Type to PN7462AU, see Section-12
  - For switching to MCU Type to LPC1769, see Section-9, Section-10 and
    Section-11.


4 Configurations of Nfcrdlib_NFC_Forum_DTA_ComplApp
===================================================

  - This application can be used for NFC Forum digital/analog compliance
    validation.
  - This application has been verified using the NI/Micropross MP500
    TCL3 Test Tool.


5 Package Contents
==================

  - Readme.txt
    + This readme file
  - Nfcrdlib_NFC_Forum_DTA_ComplApp.c
    + Main example file.
  - intfs/Nfcrdlib_NFC_Forum_DTA_ComplApp.h
    + Interfaces/Defines specific to the example
  - intfs/ph_NxpBuild_App.h
    + Reader library build configuration file
  - src/phDTA_Input.c
  - src/phDTA_ListenMode.c
  - src/phDTA_Output.c
  - src/phDTA_PollMode.c
  - src/phDTA_Resources.c
  - src/phDTA_T2T.c
  - src/phDTA_T3T.c
  - src/phDTA_T4T.c
  - src/phDTA_T5T.c
  - inc/phDTA_Input.h
  - inc/phDTA_Output.h
  - inc/phDTA_Resources.h
  - src/phApp_Helper.c
  - src/phApp_PN5180_Init.c
  - src/phApp_PN5190_Init.c
  - src/phApp_PN7462AU_Init.c
  - src/phApp_RC663_Init.c
  - src/phApp_Init.c and intfs/phApp_Init.h
    + Common utility functions (common across all examples)
  - mcux/.cproject and mcux/.project
    + MCUXpresso project configuration file
  - cr_startup_lpc175x_6x.c
    + Startup required to compile application for LPC1769 Micro
      Controller.


6 Mandatory materials (not included)
====================================

  - MCUXpresso IDE. It can be downloaded from
    [http://www.nxp.com/products/:MCUXpresso-IDE]
  - SDK for Freedom K82 Board
    ([http://www.nxp.com/products/:FRDM-K82F]).  The pre-build SDK can
    be downloaded from
    [https://mcuxpresso.nxp.com/en/license?hash=9897a8c19a6bc569c3fade7141f0f405&hash_download=true&to_vault=true]
    See MCUXpresso User Manual for steps needed to install an SDK.
  - Plugin to extend MCUXpresso to support PN7462AU. (Required for
    MCUXpresso versions before MCUXpressoIDE_10.0.2)

  - LPCXpresso LPC1769 / Freedom K82 development boards For NFC ICs
    listed in Section-16 (Note: PN7462AU (Section-16 Bullet-3) does not
    need any other other microcontroller).


7 Hardware Configuration
========================

  Before starting this application, HW Changes may be required for the
  used board.  Refer to the following User Manuals / Application notes
  before starting with this application.

  - AN11211: Quick Start Up Guide RC663 Blueboard
  - AN11744: PN5180 Evaluation board quick start guide
  - AN12550: PNEV5190B Evaluation board quick start guide


8 Software Configuration
========================

  - The Software can be compiled for Cortex M3 LPC1769 micro-controller,
    Cortex M4 Freedom K82 and Cortex M0 based PN7462AU from NXP.
  - Since this application can be configured to run on various MCU ICs
    and various NFC ICs, appropriate changes are required as mentioned
    in Section-9, Section-10, Section-11, Section-12 and Section-13.


9 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for LPC1769 with PN5190 using MCUXpresso
=========================================================================================

  See "PNEV5190B Evaluation board quick start guide" in AN12550 (See
  Section-17, Bullet-5 below)


10 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for LPC1769 with PN5180 using MCUXpresso
==========================================================================================

  See "Importing provided SW example projects" in AN11908 (See
  Section-17, Bullet-2 below)


11 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for LPC1769 with RC663 using MCUXpresso
=========================================================================================

  See "Importing provided SW example projects" in AN11022 (See
  Section-17, Bullet-3 below)


12 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for PN7462AU using MCUXpresso
===============================================================================

  - For MCUXpresso versions before MCUXpressoIDE_10.0.2, See "Adding
    PN7462AU Plugin" in UM10883. (See Section-17, Bullet-4 below)
  - See "Importing provided SW example projects" in UM10883.


13 Steps to build Nfcrdlib_NFC_Forum_DTA_ComplApp for FRDM-K82F using MCUXpresso
================================================================================

  See "Import projects to the MCUXpresso" in AN11908. (See Section-17,
  Bullet-2 below)


14 Selection of Board / OSAL / Frontend (MCUXpresso)
====================================================

  For MCUXpresso, the selection of Board / OSAL / Frontend has to be
  done via -D (Preprocessor defines).

  1) To select the board go to "Project Properties" --> "C/C++ Build"
     --> "Settings" --> "Preprocessor" --> "Defined symbols (-D)", and
     define the relevant PHDRIVER_<BoardNFCCombination>_BOARD macro.

     e.g. For using LPC1769 with Pn5180 use
     PHDRIVER_LPC1769PN5180_BOARD.  For list of supported boards refer
     to Platform\DAL\cfg\BoardSelection.h.

  2) To select the osal/os type, go to "Project Properties" --> "C/C++
     Build" --> "Settings" --> "Preprocessor" --> "Defined symbols
     (-D)".

     e.g. For using FreeRTOS use PH_OSAL_FREERTOS. For other options
     refer to RTOS\phOsal\inc\phOsal_Config.h.


15 Running Nfcrdlib_NFC_Forum_DTA_ComplApp
==========================================

  The application can be used with NFC Forum CR13 Test suite for
  validation.
  - For any other application execution related information, please
    refer DTA_QuickStart (Path:
    ..\Nfcrdlib_NFC_Forum_DTA_ComplApp\docs).
  - For DTA pattern number information, please refer
    DTA_PatternNumber_Guide.xlsx (Path:
    ..\Nfcrdlib_NFC_Forum_DTA_ComplApp\docs).


16 List of supported NFC Reader Boards/ICs
==========================================

  1) CLEV6630B 2.0 Customer Evaluation Board
  2) PNEV5180B v2.0 Customer Evaluation Board
  3) PN7462AU v2.1 Customer Evaluation Board
  4) PNEV5190B v1.0 Customer Evaluation Board


17 Reference Documents
======================

  1) UM10954 : PN5180 SW Quick start guide
     [http://www.nxp.com/docs/en/user-guide/UM10954.pdf]

  2) AN11908 : NFC Reader Library for FRDM-K82F Board Installation
     guidelines
     [http://www.nxp.com/docs/en/application-note/AN11908.pdf]

  3) AN11022 : CLRC663 Evaluation board quick start guide
     [http://www.nxp.com/docs/en/application-note/AN11022.pdf]

  4) UM10883 : PN7462AU Quick Start Guide - Development Kit
     [http://www.nxp.com/docs/en/user-guide/UM10883.pdf]

  5) AN12550 : PNEV5190B Evaluation board quick start guide

  ----------------------------------------------------------------------

  For updates of this example, see
  [http://www.nxp.com/pages/:NFC-READER-LIBRARY]
