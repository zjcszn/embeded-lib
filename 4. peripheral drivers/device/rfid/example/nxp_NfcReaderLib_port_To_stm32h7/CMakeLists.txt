
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

OPTION(WITH_NDA "With Components/Examples Under NDA" OFF)
OPTION(FRONTEND_RC663 "IC Frontend is RC663" ON)
OPTION(BUILD_EXAMPLES "Build examples and compliance apps" OFF)


IF (IS_ABSOLUTE ${CMAKE_TOOLCHAIN_FILE})
    SET(INCLUDING_TOOLCHAIN_FILE_FOR_FLAGS ON)
    INCLUDE(${CMAKE_TOOLCHAIN_FILE})
    SET(INCLUDING_TOOLCHAIN_FILE_FOR_FLAGS OFF)
ENDIF ()

# Generate folders for IDE targets (e.g., VisualStudio solutions)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(IDE_FOLDER "")

PROJECT(NxpNfcInfraSw)


IF (FRONTEND_RC663)
    ADD_DEFINITIONS(-DNXPBUILD__PHHAL_SELECTED)
    ADD_DEFINITIONS(-DNXPBUILD__PHHAL_HW_RC663)
ENDIF ()

ADD_DEFINITIONS(-DNXPBUILD_CUSTOMER_HEADER_INCLUDED)
ADD_DEFINITIONS(-DPH_OSAL_FREERTOS)

INCLUDE_DIRECTORIES(
        Platform/DAL/inc
        Platform/DAL/boards
        Platform/DAL/cfg
        RTOS/phOsal/inc
        App/inc
        NxpNfcRdLib/intfs
        NxpNfcRdLib/types
)

ADD_SUBDIRECTORY(Platform/DAL)
ADD_SUBDIRECTORY(RTOS/phOsal)
ADD_SUBDIRECTORY(NxpNfcRdLib)
#ADD_SUBDIRECTORY(App)

add_library(${PROJECT_NAME} STATIC
        App/src/phApp_DiscoveryLoop.c
        App/src/phApp_RC663_Init.c
        App/src/phApp_Helper.c
        App/src/phApp_Init.c)

target_link_libraries(${PROJECT_NAME} PUBLIC
        NxpRdLib_CryptoRng
        NxpRdLib_alI18000p3m3
        NxpRdLib_CidManager
        NxpRdLib_CryptoSym
        NxpRdLib_KeyStore
        NxpRdLib_phTools
        NxpRdLib_acDiscLoop
        NxpRdLib_alFelica
        NxpRdLib_alICode
        NxpRdLib_alMfc
        NxpRdLib_alMfdf
        NxpRdLib_alMfdfEVx
        NxpRdLib_alMfNtag42XDna
        NxpRdLib_alMfp
        NxpRdLib_alMful
        NxpRdLib_alT1T
        NxpRdLib_alTop
        NxpRdLib_palI14443p4a
        NxpRdLib_halHw
        NxpRdLib_palEpcUid
        NxpRdLib_palFelica
        NxpRdLib_palI14443p3a
        NxpRdLib_palI14443p3b
        NxpRdLib_palI14443p4
        NxpRdLib_palI14443p4mC
        NxpRdLib_palI18000p3m3
        NxpRdLib_palI18092mPI
        NxpRdLib_palI18092mT
        NxpRdLib_palMifare
        NxpRdLib_palSli15693
        NxpRdLib_phNfcLib
        phOsal
        NxpRdLib_Log
        DAL
)


IF (BUILD_EXAMPLES)
    # ADD_SUBDIRECTORY(PlatformIntegartionTestBench)
    ADD_SUBDIRECTORY(Examples)
    ADD_SUBDIRECTORY(ComplianceApp)
ENDIF (BUILD_EXAMPLES)
