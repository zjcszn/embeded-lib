# ==============================================================================
# lwkey - 轻量级按键管理模块 CMake 配置
# ==============================================================================
cmake_minimum_required(VERSION 3.14)
project(lwkey
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Lightweight key management module for embedded systems"
)

# ==============================================================================
# 编译选项
# ==============================================================================
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 编译警告选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==============================================================================
# 测试配置
# ==============================================================================
option(LWKEY_BUILD_TESTS "Build lwkey unit tests" ON)

if(LWKEY_BUILD_TESTS)
    enable_testing()

    # --------------------------------------------------------------------------
    # 测试用库 (定义 LWKEY_TEST_MODE 以禁用嵌入式相关代码)
    # --------------------------------------------------------------------------
    add_library(lwkey_testlib STATIC
        lwkey.c
    )

    target_compile_definitions(lwkey_testlib
        PUBLIC
            LWKEY_TEST_MODE  # 启用测试模式，禁用 __disable_irq 等
    )

    target_include_directories(lwkey_testlib
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # --------------------------------------------------------------------------
    # 独立测试 (自包含的 lwkey_test.c)
    # --------------------------------------------------------------------------
    add_executable(lwkey_test
        test/lwkey_test.c
    )

    target_link_libraries(lwkey_test
        PRIVATE
            lwkey_testlib
    )

    # --------------------------------------------------------------------------
    # 集成测试 (可选 - 链接真正的库进行测试)
    # --------------------------------------------------------------------------
    # 如果将来需要测试真正的 lwkey.c 实现，可以创建:
    # add_executable(lwkey_integration_test ...)
    # target_link_libraries(lwkey_integration_test PRIVATE lwkey_testlib)

    # --------------------------------------------------------------------------
    # CTest 注册
    # --------------------------------------------------------------------------
    add_test(
        NAME lwkey_unit_tests
        COMMAND lwkey_test
    )

    set_tests_properties(lwkey_unit_tests PROPERTIES
        TIMEOUT 30
        LABELS "unit"
    )

    # 便捷测试目标
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS lwkey_test
        COMMENT "Running lwkey unit tests..."
    )
endif()

# ==============================================================================
# 生产库目标 (嵌入式环境使用)
# ==============================================================================
# 注意: 此目标使用根目录的 lwkey_conf.h，需要用户配置嵌入式平台相关的宏
# 在 PC 上编译时会失败（因为 __disable_irq 等宏不存在）
# 这是预期行为 - 生产库专为嵌入式目标设计
option(LWKEY_BUILD_PRODUCTION_LIB "Build production library (requires embedded toolchain)" OFF)

if(LWKEY_BUILD_PRODUCTION_LIB)
    add_library(lwkey STATIC
        lwkey.c
    )

    target_include_directories(lwkey
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# ==============================================================================
# 安装配置 (可选)
# ==============================================================================
option(LWKEY_INSTALL "Install lwkey library" OFF)

if(LWKEY_INSTALL)
    include(GNUInstallDirs)

    install(TARGETS lwkey
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(FILES
        lwkey.h
        lwkey_conf_template.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lwkey
    )
endif()

# ==============================================================================
# 打印配置摘要
# ==============================================================================
message(STATUS "")
message(STATUS "=== lwkey Configuration ===")
message(STATUS "  Version:      ${PROJECT_VERSION}")
message(STATUS "  C Standard:   C${CMAKE_C_STANDARD}")
message(STATUS "  Build Tests:  ${LWKEY_BUILD_TESTS}")
message(STATUS "  Install:      ${LWKEY_INSTALL}")
message(STATUS "")
