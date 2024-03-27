	   __________________________________________________

	    NXPNFCRDLIB EXAMPLE : NFCRDLIBEX1_DISCOVERYLOOP
			      (V07.09.00)
	   __________________________________________________


Table of Contents
_________________

1 Document Purpose
2 Description of the NfcrdlibEx1_DiscoveryLoop
3 Restrictions on NfcrdlibEx1_DiscoveryLoop
4 Configurations of NfcrdlibEx1_DiscoveryLoop
5 Package Contents
6 Mandatory materials (not included)
7 Hardware Configuration
8 Software Configuration
9 Steps to build NfcrdlibEx1_DiscoveryLoop for LPC1769 with PN5190 using MCUXpresso
10 Steps to build NfcrdlibEx1_DiscoveryLoop for LPC1769 with PN5180 using MCUXpresso
11 Steps to build NfcrdlibEx1_DiscoveryLoop for LPC1769 with RC663 using MCUXpresso
12 Steps to build NfcrdlibEx1_DiscoveryLoop for PN7462AU using MCUXpresso
13 Steps to build NfcrdlibEx1_DiscoveryLoop for FRDM-K82F using MCUXpresso
14 Selection of Board / OSAL / Frontend (MCUXpresso)
15 Steps to follow for PI / Linux / CMake
16 Running NfcrdlibEx1_DiscoveryLoop
17 List of supported NFC Reader Boards/ICs
18 Reference Documents





1 Document Purpose
==================

  This document describes the steps to be followed to execute
  NfcrdlibEx1_DiscoveryLoop example as well as it's known problems and
  restrictions.


2 Description of the NfcrdlibEx1_DiscoveryLoop
==============================================

  - NfcrdlibEx1_DiscoveryLoop can be run with CLEV663B, PN5180, PN5190
    and PN7462AU.  (See Section-17 for supported versions)
  - This example will use both POLL and LISTEN modes of discovery loop.
  - It displays/prints detected tag information (like UID, SAK, Product
    Type for MIfare Cards) and prints information when it gets activated
    as a target by an external Initiator/reader.
  - This example will load/configure Discovery loop with default values
    based on the interested profile (Nfc, Emvco) via SetConfig.
  - Whenever multiple technologies are detected, example will select
    first detected technology to resolve.
  - This example will activate device at index zero whenever multiple
    devices are detected.
  - For EMVCo profile, this example provide full EMVCo digital
    demonstration.


3 Restrictions on NfcrdlibEx1_DiscoveryLoop
===========================================

  - NfcrdlibEx1_DiscoveryLoop is restricted to be run on NFC Reader
    Boards/ICs mentioned in Section-17.
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


4 Configurations of NfcrdlibEx1_DiscoveryLoop
=============================================

  - The example application can be run with default configuratin Or can
    be configured by enabling ENABLE_DISC_CONFIG to different settings
    Or can be configure to work as EMVCo digital loopback application,
    by enabling both ENABLE_DISC_CONFIG and ENABLE_EMVCO_PROF
  - The Default configuration example application is written to detect
    and activate one tag of supported technologies.
  - Listen mode is supported only for PN5180 and PN7462AU and this
    example application receives ATR_REQ or RATS from a NFC Peer/Reader.
    Additional functionality is not supported in this example.
  - With CLRC663, owing to the hardware limitations of CLRC663, it only
    supports poll without Active mode.


5 Package Contents
==================

  - Readme.txt
    + This readme file
  - NfcrdlibEx1_DiscoveryLoop.c
    + Main example file.
  - intfs/NfcrdlibEx1_DiscoveryLoop.h
    + Interfaces/Defines specific to the example
  - intfs/ph_NxpBuild_App.h
    + Reader library build configuration file
  - src/phApp_Helper.c
  - src/phApp_PN5180_Init.c
  - src/phApp_PN5190_Init.c
  - src/phApp_PN7462AU_Init.c
  - src/phApp_RC663_Init.c
  - src/phApp_Init.c and intfs/phApp_Init.h
    + Common utility functions (common across all examples)
  - src/NfcrdlibEx1_EmvcoProfile.c
    + Emvco profile funtions specific to the example
  - mcux/.cproject and mcux/.project
    + MCUXpresso project configuration file
  - cr_startup_lpc175x_6x.c
    + Startup required to compile example for LPC1769 Micro Controller.


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
    listed in Section-17 (Note: PN7462AU (Section-17 Bullet-3) does not
    need any other other microcontroller).


7 Hardware Configuration
========================

  Before starting this application, HW Changes may be required for the
  used board.  Refer to the following User Manuals / Application notes
  before starting with this example.

  - AN11211: Quick Start Up Guide RC663 Blueboard
  - AN11744: PN5180 Evaluation board quick start guide
  - AN11802: NFC Reader Library for Linux Installation Guidelines
  - AN12550: PNEV5190B Evaluation board quick start guide


8 Software Configuration
========================

  - The Software can be compiled for Cortex M3 LPC1769 micro-controller,
    Cortex M4 Freedom K82 and Cortex M0 based PN7462AU from NXP.
  - Since this example can be configured to run on various MCU ICs and
    various NFC ICs, appropriate changes are required as mentioned in
    Section-9, Section-10, Section-11, Section-12 and Section-13.


9 Steps to build NfcrdlibEx1_DiscoveryLoop for LPC1769 with PN5190 using MCUXpresso
===================================================================================

  See "PNEV5190B Evaluation board quick start guide" in AN12550 (See
  Section-18, Bullet-6 below)


10 Steps to build NfcrdlibEx1_DiscoveryLoop for LPC1769 with PN5180 using MCUXpresso
====================================================================================

  See "Importing provided SW example projects" in AN11908 (See
  Section-18, Bullet-2 below)


11 Steps to build NfcrdlibEx1_DiscoveryLoop for LPC1769 with RC663 using MCUXpresso
===================================================================================

  See "Importing provided SW example projects" in AN11022 (See
  Section-18, Bullet-3 below)


12 Steps to build NfcrdlibEx1_DiscoveryLoop for PN7462AU using MCUXpresso
=========================================================================

  - For MCUXpresso versions before MCUXpressoIDE_10.0.2, See "Adding
    PN7462AU Plugin" in UM10883. (See Section-18, Bullet-4 below)
  - See "Importing provided SW example projects" in UM10883.


13 Steps to build NfcrdlibEx1_DiscoveryLoop for FRDM-K82F using MCUXpresso
==========================================================================

  See "Import projects to the MCUXpresso" in AN11908. (See Section-18,
  Bullet-2 below)


14 Selection of Board / OSAL / Frontend (MCUXpresso)
====================================================

  For MCUXpresso, the selection of Board / OSAL / Frontend has to be
  done via -D (Preprocessor defines).

  1) To select the board go to "Project Properties" --> "C/C++ Build"
     --> "Settings" --> "Preprocessor" --> "Defined symbols (-D)", and
     define the relevant PHDRIVER_<BoardNFCCombination>_BOARD macro.

     e.g. For using K82 with Pn5190 use PHDRIVER_K82F_PNEV5190B_BOARD
     and for LPC1769 with PN5190 use PHDRIVER_LPC1769PN5190_BOARD.  For
     list of supported boards refer to
     Platform\DAL\cfg\BoardSelection.h.

  2) To select the osal/os type, go to "Project Properties" --> "C/C++
     Build" --> "Settings" --> "Preprocessor" --> "Defined symbols
     (-D)".

     e.g. For using FreeRTOS use PH_OSAL_FREERTOS. For other options
     refer to RTOS\phOsal\inc\phOsal_Config.h.


15 Steps to follow for PI / Linux / CMake
=========================================

  The steps are described in AN11802. (See Section-18, Bullet-5 below)


16 Running NfcrdlibEx1_DiscoveryLoop
====================================

  When this application is running in default configuration on the
  target MCU setup and when you bring the NFC cards or phone in
  proximity, the example application will detect and reports the NFC
  technology types detected.  Upon using MACRO ENABLE_EMVCO_PROF, this
  application will work as EMVCo Digital Loopback Application


17 List of supported NFC Reader Boards/ICs
==========================================

  1) CLEV6630B v2.0 Customer Evaluation Board
  2) PNEV5180B v2.0 Customer Evaluation Board
  3) PN7462AU v2.1 Customer Evaluation Board
  4) PNEV5190B v1.0 Customer Evaluation Board


18 Reference Documents
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

  5) AN11802 : NFC Reader Library for Linux Installation Guidelines
     [http://www.nxp.com/docs/en/application-note/AN11802.pdf]

  6) AN12550 : PNEV5190B Evaluation board quick start guide

  ----------------------------------------------------------------------

  For updates of this example, see
  [http://www.nxp.com/products/:NFC-READER-LIBRARY]
