	   __________________________________________________

		     NXPNFCRDLIB TEST APPLICATION :
			NFCRDLIBTST12_RC663LPCD
			      (V07.09.00)
	   __________________________________________________


Table of Contents
_________________

1 Document Purpose
2 Description of the NfcrdlibTst12_Rc663Lpcd
3 Restrictions on NfcrdlibTst12_Rc663Lpcd
4 Configurations of NfcrdlibTst12_Rc663Lpcd
5 Package Contents
6 Mandatory materials (not included)
7 Hardware Configuration
8 Software Configuration
9 Steps to build NfcrdlibTst12_Rc663Lpcd for LPC1769 with RC663 using MCUXpresso
10 Selection of Board / OSAL / Frontend (MCUXpresso)
11 Running NfcrdlibTst12_Rc663Lpcd
12 List of supported NFC Reader Boards/ICs
13 Reference Documents





1 Document Purpose
==================

  This document describes the steps to be followed to execute
  NfcrdlibTst12_Rc663Lpcd test application as well as it's known
  problems and restrictions.


2 Description of the NfcrdlibTst12_Rc663Lpcd
============================================

  - NfcrdlibTst12_Rc663Lpcd can be run with CLEV663B.  (See Section-12
    for supported versions)
  - This test application will execute the test case as choosen by the
    user which will set the Digital Filter, Charge Pump and Detection
    Option as per the test case.
  - This test application 7 Functional Scenarios. 3 out of these
    scenarios are reused for Timing Test Cases.
  - For Scenario 1 to 6, the Calibration Process is performed without
    any load on the antenna. And the presence of any card in the
    proximity is detected by the Lpcd Loop.
  - For Scenario 1 to 6, if any load (Cards : Mifare Desfire EV1 (Type
    A), ezlink Type B, Type F(Felica) RCS888, ICode SliX (SLI 15693),
    Stackit ICode ILT (18000p3m3); Phone or any metal) are brought in
    proximity then the test application detect the load the exits Lpcd
    loop.
  - For Scenario 7, the calibration process is performed with the
    load(Cards : Mifare Desfire EV1 (Type A), ezlink Type B, Type
    F(Felica) RCS888, ICode SliX (SLI 15693), Stackit ICode ILT
    (18000p3m3); Phone or any metal) on any antenna. When the load is
    removed during the Lpcd Loop, then this test application detect the
    unloading on the antenna and exits Lpcd loop.


3 Restrictions on NfcrdlibTst12_Rc663Lpcd
=========================================

  - NfcrdlibTst12_Rc663Lpcd is restricted to be run on NFC Reader
    Boards/ICs mentioned in Section-12.
  - The APIs are intended for NXP contact-less reader ICs only, as such
    the APIs are not to be ported to any technology from any other
    vendor.
  - NXP will not support porting to any other vendor platform.
  - This software project requires NxpNfcRdLib v07.09.00 or later.


4 Configurations of NfcrdlibTst12_Rc663Lpcd
===========================================

  - The test application is written to perform lpcd with the
    configuration as provided by the user.
  - After the LPCD loop returns success, the test application wait for
    the new input from user.


5 Package Contents
==================

  - Readme.txt
    + This readme file
  - NfcrdlibTst12_Rc663Lpcd.c
    + Main test application.
  - intfs/NfcrdlibTst12_Rc663Lpcd.h
    + Interfaces/Defines specific to the example
  - intfs/ph_NxpBuild_App.h
    + Reader library build configuration file
  - mcux/.cproject and mcux/.project
    + MCUXpresso project configuration file
  - cr_startup_lpc175x_6x.c
    + Startup required to compile test application LPC1769 Micro
      Controller.


6 Mandatory materials (not included)
====================================

  - MCUXpresso IDE. It can be downloaded from
    [http://www.nxp.com/products/:MCUXpresso-IDE]


7 Hardware Configuration
========================

  Before starting this application, HW Changes may be required for the
  used board.  Refer to the following User Manuals / Application notes
  before starting with this example.

  - AN11211: Quick Start Up Guide RC663 Blueboard


8 Software Configuration
========================

  - The Software can be compiled for Cortex M3 LPC1769 micro-controller
    from NXP.
  - Since this test application be configured to run on various MCU ICs
    and various NFC ICs, appropriate changes are required as mentioned
    in Section-.


9 Steps to build NfcrdlibTst12_Rc663Lpcd for LPC1769 with RC663 using MCUXpresso
================================================================================

  See "Importing provided SW example projects" in AN11022 (See
  Section-13, Bullet-1 below)


10 Selection of Board / OSAL / Frontend (MCUXpresso)
====================================================

  For MCUXpresso, the selection of Board / OSAL / Frontend has to be
  done via -D (Preprocessor defines).

  1) To select the board go to "Project Properties" --> "C/C++ Build"
     --> "Settings" --> "Preprocessor" --> "Defined symbols (-D)", and
     define the relevant PHDRIVER_<BoardNFCCombination>_BOARD macro.

     e.g. For using LPC1769 with RC663 use PHDRIVER_LPC1769RC663_BOARD.
     For list of supported boards refer to
     Platform\DAL\cfg\BoardSelection.h.

  2) To select the osal/os type, go to "Project Properties" --> "C/C++
     Build" --> "Settings" --> "Preprocessor" --> "Defined symbols
     (-D)".

     e.g. For using FreeRTOS use PH_OSAL_FREERTOS. For other options
     refer to RTOS\phOsal\inc\phOsal_Config.h.


11 Running NfcrdlibTst12_Rc663Lpcd
==================================

  When this application is running on the target MCU setup and when you
  bring the NFC cards or phone in proximity, the test application will
  detect and reports the NFC technology types detected.


12 List of supported NFC Reader Boards/ICs
==========================================

  1) CLEV6630B v2.0 Customer Evaluation Board


13 Reference Documents
======================

  1) AN11022 : CLRC663 Evaluation board quick start guide
     [http://www.nxp.com/docs/en/application-note/AN11022.pdf]

  ----------------------------------------------------------------------

  For updates of this example, see
  [http://www.nxp.com/products/:NFC-READER-LIBRARY]
