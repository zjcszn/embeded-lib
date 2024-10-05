#设置CMake最低版本要求
cmake_minimum_required(VERSION 3.21)

#引用ARMClang_Cortex-M4.cmake
include(ARMClang_Cortex-M4.cmake)

#设置工程名称
CMSIS_SetProjectName(mini_boot)

#使能FPU
CMSIS_SetFPU(TRUE)

#添加全局宏定义
CMSIS_SetPreDefine(
        USE_FULL_LL_DRIVER
        STM32F407xx
        HSE_VALUE=8000000
        HSE_STARTUP_TIMEOUT=100
        LSE_STARTUP_TIMEOUT=5000
        LSE_VALUE=32768
        EXTERNAL_CLOCK_VALUE=12288000
        HSI_VALUE=16000000
        LSI_VALUE=32000
        VDD_VALUE=3300
        PREFETCH_ENABLE=1
        INSTRUCTION_CACHE_ENABLE=1
        DATA_CACHE_ENABLE=1
        )

#设置Sections脚本路径
CMSIS_SetSectionsScriptPath("${CMAKE_HOME_DIRECTORY}/mini_boot.sct")

#配置交叉编译
CMSIS_CrossCompilingConfiguration()

#配置工程属性
project(${CMAKE_PROJECT_NAME} LANGUAGES C ASM)

#添加头文件引用目录
include_directories(
        ./
        Core/Inc
        Drivers/CMSIS/Include
        Drivers/CMSIS/Device/ST/STM32F4xx/Include
        Drivers/STM32F4xx_HAL_Driver/Inc
        Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
        MiddleWare/miniblt/inc
        MiddleWare/miniblt/components
        MiddleWare/miniblt/components/elog
        MiddleWare/miniblt/components/fal/inc
        MiddleWare/miniblt/components/fal/porting
        MiddleWare/miniblt/components/fatfs
        MiddleWare/miniblt/components/freertos/include
        MiddleWare/miniblt/components/mbedtls
        MiddleWare/miniblt/components/mbedtls/include
        MiddleWare/miniblt/components/mbedtls/include/mbedtls
        MiddleWare/miniblt/components/mbedtls/include/psa
        MiddleWare/miniblt/components/shell
        MiddleWare/miniblt/components/ymodem
        MiddleWare/miniblt/utils
        MiddleWare/miniblt/port
        MiddleWare/miniblt/components/freertos/portable
        )

#添加源文件
file(GLOB_RECURSE SOURCES
        Core/Src/*.c
        Drivers/STM32F4xx_HAL_Driver/Src/*.c
        MDK-ARM/*.s
        MiddleWare/*.c
        User/*.c
        )

#设置排除列表
file(GLOB EXCLUDE_SRCS

        )

#从源文件列表(SOURCES)中剔除干扰文件(EXCLUDE_SRCS)
list(REMOVE_ITEM SOURCES ${EXCLUDE_SRCS})

#添加生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(ELF_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND cd /d "${PROJECT_BINARY_DIR}"
        COMMAND del /q /s "${PROJECT_NAME}.*" >nul
        COMMENT "Delete Old Output File..."
        )

#生成编译后显示固件大小的powershell脚本
file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/showinfo.ps1" CONTENT "\
        $esc = ([char]27)
        $elf_size = \"{0,8:D}\"  -f (Get-Item \"${ELF_FILE}\").Length
        $elf_size_kb = \"{0,8:N2}\" -f ($elf_size / 1KB)
        $hex_size = \"{0,8:D}\"  -f (Get-Item \"${HEX_FILE}\").Length
        $hex_size_kb = \"{0,8:N2}\" -f ($hex_size / 1KB)
        $bin_size = \"{0,8:D}\"  -f (Get-Item \"${BIN_FILE}\").Length
        $bin_size_kb = \"{0,8:N2}\" -f ($bin_size / 1KB)
        Write-Output \"$esc[32mELF Size: $elf_size ($elf_size_kb KB)$esc[0m\"
        Write-Output \"$esc[32mHEX Size: $hex_size ($hex_size_kb KB)$esc[0m\"
        Write-Output \"$esc[32mBIN Size: $bin_size ($bin_size_kb KB)$esc[0m\""
        )

set(FROM_ELF fromelf.exe)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${FROM_ELF} --i32combined --output="${HEX_FILE}" "${ELF_FILE}"
        COMMAND ${FROM_ELF} --bincombined --output="${BIN_FILE}" "${ELF_FILE}"
        COMMAND powershell.exe -File "${PROJECT_BINARY_DIR}/showinfo.ps1"
        COMMENT "Building Hex & Bin Firmware..."
        )
