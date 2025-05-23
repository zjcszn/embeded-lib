
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

OPTION(WITH_NDA              "With Components/Examples Under NDA" OFF)

OPTION(BOARD_RaspberryPi  "Platform is RaspberryPi"            ON)
OPTION(BOARD_UdooNeo      "Platform is UdooNeo"                OFF)
OPTION(BOARD_LPC1769      "Platform is LPC1769"                OFF)

OPTION(FRONTEND_PN5190       "IC Frontend is PN5190"              ON)
OPTION(FRONTEND_PN5180       "IC Frontend is PN5180"              OFF)
OPTION(FRONTEND_RC663        "IC Frontend is RC663"               OFF)


OPTION(BUILD_EXAMPLES        "Build examples and compliance apps"  ON)

OPTION(BUS_SPIUserMode       "Connected to FRONTEND over SPI in User Mode"     OFF)
OPTION(BUS_SPIKernelMode     "Connected to FRONTEND over SPI in Kernal Mode"   OFF)

IF(BUS_SPIKernelMode AND BUS_SPIUserMode)
    MESSAGE(FATAL_ERROR "Can not select both SPI Kernel Mode and SPI User Mode")
ENDIF()

IF(BUS_SPIKernelMode AND FRONTEND_RC663)
    MESSAGE(FATAL_ERROR "SPI Kernel Mode is not supported for RC663")
ENDIF()

IF(BOARD_RaspberryPi AND BOARD_UdooNeo)
    MESSAGE(FATAL_ERROR "RaspberryPi and UdooNeo can not be compiled at the same time. ")
ENDIF()

IF (FRONTEND_PN5180 AND FRONTEND_RC663)
    MESSAGE(FATAL_ERROR "PN5180 and RC663 can not be compiled at the same time. ")
ENDIF (FRONTEND_PN5180 AND FRONTEND_RC663)

IF (FRONTEND_PN5190 AND FRONTEND_RC663)
    MESSAGE(FATAL_ERROR "PN5190 and RC663 can not be compiled at the same time. ")
ENDIF (FRONTEND_PN5190 AND FRONTEND_RC663)

IF (FRONTEND_PN5180 AND FRONTEND_PN5190)
    MESSAGE(FATAL_ERROR "PN5180 and PN5190 can not be compiled at the same time. ")
ENDIF (FRONTEND_PN5180 AND FRONTEND_PN5190)


IF(IS_ABSOLUTE ${CMAKE_TOOLCHAIN_FILE})
    SET(INCLUDING_TOOLCHAIN_FILE_FOR_FLAGS ON)
    INCLUDE(${CMAKE_TOOLCHAIN_FILE})
    SET(INCLUDING_TOOLCHAIN_FILE_FOR_FLAGS OFF)
ENDIF()

# Generate folders for IDE targets (e.g., VisualStudio solutions)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(IDE_FOLDER "")

PROJECT(NxpNfcInfraSw)

IF (BUS_SPIKernelMode OR BUS_SPIUserMode)
    ADD_DEFINITIONS(-DNXPBUILD__PHBAL_SELECTED)
ENDIF ()

IF(BUS_SPIKernelMode)
    ADD_DEFINITIONS(-DNXPBUILD__PHBAL_REG_LINUX_KERNEL_SPI)
ENDIF()

IF(BUS_SPIUserMode)
    ADD_DEFINITIONS(-DNXPBUILD__PHBAL_REG_LINUX_USER_SPI)
ENDIF()

IF (FRONTEND_PN5180 OR FRONTEND_RC663 OR FRONTEND_PN5190)
    ADD_DEFINITIONS(-DNXPBUILD__PHHAL_SELECTED)
ELSE (FRONTEND_PN5180 OR FRONTEND_RC663 OR FRONTEND_PN5190)
    MESSAGE(FATAL_ERROR "Either of PN5180 or PN5190 or RC663 should be selected. ")
ENDIF (FRONTEND_PN5180 OR FRONTEND_RC663 OR FRONTEND_PN5190)


IF(FRONTEND_PN5180)
    ADD_DEFINITIONS(-DNXPBUILD__PHHAL_HW_PN5180)
ENDIF()

IF(FRONTEND_PN5190)
    ADD_DEFINITIONS(-DNXPBUILD__PHHAL_HW_PN5190)
ENDIF()

IF(FRONTEND_RC663)
    ADD_DEFINITIONS(-DNXPBUILD__PHHAL_HW_RC663)
ENDIF()

INCLUDE_DIRECTORIES(
    Platform/DAL/inc
    Platform/DAL/boards
    Platform/DAL/cfg
    RTOS/phOsal/inc
)

IF(BOARD_RaspberryPi OR BOARD_UdooNeo)
    ADD_DEFINITIONS(-pthread)
    ADD_DEFINITIONS(-DDEBUG)
    ADD_DEFINITIONS(-g)
ENDIF()

IF(BOARD_RaspberryPi)
    IF(FRONTEND_PN5180)
        ADD_DEFINITIONS(-DPHDRIVER_PIPN5180_BOARD)
    ENDIF(FRONTEND_PN5180)
    IF(FRONTEND_PN5190)
        ADD_DEFINITIONS(-DPHDRIVER_PIPN5190_BOARD)
        ADD_DEFINITIONS(-DPH_NXPNFCRDLIB_CONFIG_EMVCO_FIELD_OFF_DELAY_US=2500)
    ENDIF(FRONTEND_PN5190)
    IF(FRONTEND_RC663)
        ADD_DEFINITIONS(-DPHDRIVER_PIRC663_BOARD)
    ENDIF(FRONTEND_RC663)
    ADD_DEFINITIONS(-DNXPBUILD__PH_RASPBERRY_PI)
    ADD_DEFINITIONS(-DPH_OSAL_LINUX)
    INCLUDE_DIRECTORIES(
        "Platform/DAL/src/Linux/"
    )
ENDIF(BOARD_RaspberryPi)

IF(BOARD_UdooNeo)
    ADD_DEFINITIONS(-DNXPBUILD__PH_UDOO_NEO)
ENDIF(BOARD_UdooNeo)

IF(BOARD_LPC1769)
    ADD_SUBDIRECTORY(RTOS)
    ADD_SUBDIRECTORY(Platform)

    INCLUDE_DIRECTORIES(
        "RTOS/FreeRTOS/include"
        "RTOS/FreeRTOS/portable/GCC/ARM_CM3/"
    )

    IF(IS_DIRECTORY ${lpc_chip_175x_6x_SOURCE_DIR}/libs)
        LINK_DIRECTORIES(${lpc_chip_175x_6x_SOURCE_DIR}/libs)
    ENDIF()
ENDIF(BOARD_LPC1769)

ADD_SUBDIRECTORY(Platform/DAL)
ADD_SUBDIRECTORY(RTOS/phOsal)
ADD_SUBDIRECTORY(NxpNfcRdLib)

INCLUDE_DIRECTORIES(
    ${NxpNfcRdLib_SOURCE_DIR}/intfs
    ${NxpNfcRdLib_SOURCE_DIR}/types
)

IF(BUILD_EXAMPLES)
    # ADD_SUBDIRECTORY(PlatformIntegartionTestBench)
    ADD_SUBDIRECTORY(Examples)
    ADD_SUBDIRECTORY(ComplianceApp)
    IF(FRONTEND_PN5190)
        ADD_SUBDIRECTORY(ExamplesPN5190)
    ENDIF()
ENDIF(BUILD_EXAMPLES)
