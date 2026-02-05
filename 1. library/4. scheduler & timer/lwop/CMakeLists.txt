# Copyright 2024 zjcszn. All rights reserved.
#
# CMakeLists.txt for Operation Module Test Suite
#
# Usage:
#   mkdir build && cd build
#   cmake ..
#   cmake --build .
#   ctest --output-on-failure
#
# Build specific tests:
#   cmake --build . --target operation_test           # Bare-metal single-slot mode
#   cmake --build . --target operation_test_fifo      # Bare-metal FIFO mode
#   cmake --build . --target operation_test_rtos      # RTOS mode simulation
#
# Run all tests:
#   ctest --output-on-failure

cmake_minimum_required(VERSION 3.14)

project(OperationModuleTest
    VERSION 1.0.0
    DESCRIPTION "Operation Module Test Suite"
    LANGUAGES C
)

# =============================================================================
# Options
# =============================================================================

option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(BUILD_RTOS_TEST "Build RTOS simulation test (requires pthreads)" ON)

# =============================================================================
# C Standard
# =============================================================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =============================================================================
# Platform Detection
# =============================================================================

if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
else()
    set(PLATFORM_WINDOWS FALSE)
endif()

# =============================================================================
# Compiler Warnings
# =============================================================================

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror=implicit-function-declaration
        -Werror=return-type
        -Wno-unused-function
    )
elseif(MSVC)
    add_compile_options(
        /W4
        /WX
        /wd4100  # Unreferenced formal parameter.
        /wd4996  # Deprecated functions.
    )
endif()

# =============================================================================
# Sanitizers (Optional)
# =============================================================================

if(ENABLE_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# =============================================================================
# Coverage (Optional)
# =============================================================================

if(ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

# =============================================================================
# Test 1: Bare-metal Single-Slot Mode (Default)
# =============================================================================

add_executable(operation_test
    operation_test.c
)

target_include_directories(operation_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# =============================================================================
# Test 2: Bare-metal FIFO Mode
# =============================================================================

add_executable(operation_test_fifo
    operation_test_fifo.c
)

target_include_directories(operation_test_fifo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# =============================================================================
# Test 3: RTOS Mode Simulation (requires pthreads)
# =============================================================================

if(BUILD_RTOS_TEST)
    add_executable(operation_test_rtos
        operation_test_rtos.c
    )
    
    target_include_directories(operation_test_rtos PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Link pthread library on non-Windows platforms.
    if(NOT PLATFORM_WINDOWS)
        find_package(Threads REQUIRED)
        target_link_libraries(operation_test_rtos PRIVATE Threads::Threads)
    endif()
    
    # On Windows, the test uses Windows native threads (no extra libs needed).
endif()

# =============================================================================
# CTest Integration
# =============================================================================

enable_testing()

# Test 1: Bare-metal single-slot mode.
add_test(
    NAME BareMetal_SingleSlot
    COMMAND operation_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(BareMetal_SingleSlot PROPERTIES
    LABELS "unit;bare-metal;single-slot"
    TIMEOUT 60
)

# Test 2: Bare-metal FIFO mode.
add_test(
    NAME BareMetal_FIFO
    COMMAND operation_test_fifo
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(BareMetal_FIFO PROPERTIES
    LABELS "unit;bare-metal;fifo"
    TIMEOUT 60
)

# Test 3: RTOS mode simulation.
if(BUILD_RTOS_TEST)
    add_test(
        NAME RTOS_Simulation
        COMMAND operation_test_rtos
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(RTOS_Simulation PROPERTIES
        LABELS "unit;rtos;threading"
        TIMEOUT 120
    )
endif()

# =============================================================================
# Custom Targets
# =============================================================================

# Run all tests with verbose output.
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS operation_test operation_test_fifo
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running all tests..."
)

if(BUILD_RTOS_TEST)
    add_dependencies(run_tests operation_test_rtos)
endif()

# Run only bare-metal tests.
add_custom_target(run_bare_metal_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L bare-metal
    DEPENDS operation_test operation_test_fifo
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running bare-metal tests..."
)

# Run only RTOS test.
if(BUILD_RTOS_TEST)
    add_custom_target(run_rtos_test
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L rtos
        DEPENDS operation_test_rtos
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running RTOS simulation test..."
    )
endif()

# Clean and rebuild.
add_custom_target(rebuild
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --clean-first
    COMMENT "Clean rebuild..."
)

# =============================================================================
# Coverage Report (if enabled)
# =============================================================================

if(ENABLE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)

    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${LCOV} --capture --directory . --output-file coverage.info
            COMMAND ${LCOV} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${GENHTML} coverage.info --output-directory coverage_report
            DEPENDS run_tests
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating coverage report..."
        )
    else()
        message(WARNING "lcov/genhtml not found, coverage report target disabled")
    endif()
endif()

# =============================================================================
# Installation (Optional)
# =============================================================================

install(TARGETS operation_test operation_test_fifo
    RUNTIME DESTINATION bin
)

if(BUILD_RTOS_TEST)
    install(TARGETS operation_test_rtos
        RUNTIME DESTINATION bin
    )
endif()

# =============================================================================
# Print Configuration Summary
# =============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Operation Module Test Configuration")
message(STATUS "========================================")
message(STATUS "  CMake Version:    ${CMAKE_VERSION}")
message(STATUS "  C Compiler:       ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform:         ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Coverage:         ${ENABLE_COVERAGE}")
message(STATUS "  Sanitizers:       ${ENABLE_SANITIZERS}")
message(STATUS "  Build RTOS Test:  ${BUILD_RTOS_TEST}")
message(STATUS "")
message(STATUS "Test Targets:")
message(STATUS "  - operation_test       (Bare-metal, Single-Slot)")
message(STATUS "  - operation_test_fifo  (Bare-metal, FIFO)")
if(BUILD_RTOS_TEST)
message(STATUS "  - operation_test_rtos  (RTOS Simulation)")
endif()
message(STATUS "========================================")
message(STATUS "")
