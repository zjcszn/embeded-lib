cmake_minimum_required(VERSION 3.10)

project(MultiTaskScheduler
        VERSION 1.0
        DESCRIPTION "Lightweight cooperative task scheduler test suite"
        LANGUAGES C)

# ==========================================
# 编译器设置
# ==========================================

# 指定 C 标准 (C99 是嵌入式开发的常用基准)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 开启常用的编译警告
# 这对于嵌入式代码非常重要，有助于发现潜在的溢出或类型问题
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion      # 警告隐式类型转换 (对 tick 计算很重要)
            -Wsign-conversion # 警告符号转换 (对 tick 溢出处理很重要)
            -g                # 生成调试信息，方便 GDB 调试
    )
endif()

# ==========================================
# 目标定义
# ==========================================

# 定义源文件列表
set(SOURCES
        multi_task.c
        multi_task_testing.c
)

# 创建可执行文件
add_executable(multi_task_test ${SOURCES})

# ==========================================
# 包含路径
# ==========================================

# 将当前目录添加到头文件搜索路径
# 这样 multi_task.c 和 test_multi_task.c 才能找到 multi_task.h
target_include_directories(multi_task_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ==========================================
# 辅助信息
# ==========================================
message(STATUS "Configuring build for: ${PROJECT_NAME}")
message(STATUS "Source files: ${SOURCES}")