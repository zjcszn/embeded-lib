#设置CMake最低版本要求
cmake_minimum_required(VERSION 3.21)

#交叉编译：设置目标机器类型
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Cortex-M4)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

#交叉编译：添加mcpu编译选项
add_compile_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR})

#交叉编译：设置编译器
set(CMAKE_C_COMPILER armclang.exe CACHE STRING "c compiler" FORCE)
set(CMAKE_CXX_COMPILER armclang.exe CACHE STRING "c++ compiler" FORCE)
set(CMAKE_ASM_COMPILER armclang.exe CACHE STRING "asm compiler" FORCE)

#交叉编译：设置链接器
set(CMAKE_C_LINK_EXECUTABLE armlink.exe CACHE STRING "c linker" FORCE)
set(CMAKE_CXX_LINK_EXECUTABLE armlink.exe CACHE STRING "c++ linker" FORCE)
set(CMAKE_ASM_LINK_EXECUTABLE armlink.exe CACHE STRING "asm linker" FORCE)

#交叉编译：设置调试选项
add_compile_options("-gdwarf-4")

#设置用户变量
set(FPU TRUE CACHE BOOL "fpu switch")

#设置C/C++标准
set(C_STANDARD_FLAG "-xc -std=gnu11" CACHE STRING "c standard flag")
set(CXX_STANDARD_FLAG "-xc++ -std=gnu++11" CACHE STRING "c++ standard flag")

#设置debug/release编译参数
set(CMAKE_C_FLAGS_DEBUG "-O0 -fno-lto -Wall" CACHE STRING "c debug flag" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} CACHE STRING "c++ debug flag" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "--no_lto" CACHE STRING "link debug flag" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O3 -flto -Wall -DNDEBUG" CACHE STRING "c release flag" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_DEBUG} CACHE STRING "c++ release flag" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "--lto" CACHE STRING "link release flag" FORCE)

#接口: 配置预定义宏
function(CMSIS_SetPreDefine ...)
    set(INDEX 0)
    while(INDEX LESS ${ARGC})
        set(C_COMPILE_PREDEFINES "${C_COMPILE_PREDEFINES} -D${ARGV${INDEX}}" CACHE STRING "pre-defines" FORCE)
        math(EXPR INDEX "${INDEX} + 1")
    endwhile()
endfunction()

#接口: FPU使能控制
function(CMSIS_SetFPU isEnabled)
    set(FPU ${isEnabled} CACHE BOOL "fpu switch" FORCE)
endfunction()

#接口: 设置工程名称
function(CMSIS_SetProjectName projectName)
    set(CMAKE_PROJECT_NAME ${projectName} CACHE STRING "project name" FORCE)
endfunction()

#接口：链接脚本配置
function(CMSIS_SetSectionsScriptPath path)
    set(SECTIONS_SCRIPT_PATH ${path} CACHE STRING "link script path" FORCE)
endfunction()

#接口：交叉编译配置
function(CMSIS_CrossCompilingConfiguration)

    #设置变量
    set(C_TARGET_FLAG "--target=arm-arm-none-eabi")

    #设置FPU参数
    if(FPU)
        set(FPU_FLAG "-mfpu=fpv4-sp-d16 -mfloat-abi=hard")
        set(ASM_TARGET_FLAG "--target=arm-arm-none-eabi")
        set(LINK_TARGET_FLAG "--cpu=${CMAKE_SYSTEM_PROCESSOR}.fp.sp")
    else()
        set(FPU_FLAG "-mfloat-abi=soft -mfpu=none")
        set(ASM_TARGET_FLAG "--target=armv7em-arm-none-eabi")
        set(LINK_TARGET_FLAG "--cpu=${CMAKE_SYSTEM_PROCESSOR} --fpu=SoftVFP")
    endif()


    #设置C/C++通用编译选项
    set(COMPILE_COMMON_FLAG "\
        ${C_TARGET_FLAG} ${FPU_FLAG} ${C_COMPILE_PREDEFINES} \
        -c -fno-rtti -funsigned-char -fshort-enums -fshort-wchar -ffunction-sections -MD")

    #设置C编译器选项
    set(CMAKE_C_FLAGS_INIT "${C_STANDARD_FLAG} ${COMPILE_COMMON_FLAG}" CACHE STRING "c compile flag" FORCE)

    #设置C++编译器选项
    set(CMAKE_CXX_FLAGS_INIT "${CXX_STANDARD_FLAG} ${COMPILE_COMMON_FLAG}" CACHE STRING "c++ compile flag" FORCE)

    #设置ASM编译器选项
    set(CMAKE_ASM_FLAGS_INIT "${ASM_TARGET_FLAG} ${FPU_FLAG} -masm=auto -c" CACHE STRING "asm compile flag" FORCE)

    #判断链接脚本是否存在
    if(NOT SECTIONS_SCRIPT_PATH)
        message(FATAL_ERROR "You not set SECTIONS_SCRIPT_PATH!")
    endif ()

    #设置链接选项
    set(CMAKE_EXE_LINKER_FLAGS_INIT "\
        ${LINKE_TARGET_FLAG} \
        --strict \
        --scatter \"${SECTIONS_SCRIPT_PATH}\" \
        --summary_stderr \
        --info summarysizes \
        --map --load_addr_map_info --xref --callgraph --symbols \
        --info sizes --info totals --info unused --info veneers \
        " CACHE STRING "link flag" FORCE)

    #使能汇编
    enable_language(C ASM)

endfunction()