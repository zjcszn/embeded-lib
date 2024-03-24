# ARM Nucleo L053r8 support

# check if ${TGT} exists
if((NOT DEFINED TGT) OR (NOT TGT))
    message(FATAL_ERROR "set TGT to your cmake target to which the files shall be added")
endif()

string(TOLOWER ${CMAKE_C_COMPILER_ID} _compiler_)
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_compiler_})
    message(FATAL_ERROR "No support for compiler ${CMAKE_C_COMPILER_ID}")
endif()

target_sources(${TGT} PRIVATE
    system_stm32l0xx.c
    ${_compiler_}/startup_stm32l053xx.c
)
if(DEFINED QPC_SDK_PATH)
    set(CMSIS_BASE "${QPC_SDK_PATH}/3rd_party/CMSIS")
elseif(DEFINED QPCPP_SDK_PATH)
    set(CMSIS_BASE "${QPCPP_SDK_PATH}/3rd_party/CMSIS")
else()
    message(FATAL_ERROR "Neither 'QPC_SDK_PATH' nor 'QPCPP_SDK_PATH' is defined!")
endif()

target_include_directories(${TGT} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMSIS_BASE}/Include
)
