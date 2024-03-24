# This is a copy of <QPCPP_SDK_PATH>/3rd-party/cmake/qpcpp_import.cmake

# This can be dropped into an external project to help locate this SDK
# It should be include()ed prior to project()
message(STATUS "qpcpp_import.cmake starting...")

if (DEFINED ENV{QPCPP_SDK_PATH} AND (NOT QPCPP_SDK_PATH))
    set(QPCPP_SDK_PATH $ENV{QPCPP_SDK_PATH})
    message("Using QPCPP_SDK_PATH from environment ('${QPCPP_SDK_PATH}')")
endif ()

if (DEFINED ENV{QPCPP_FETCH_FROM_GIT} AND (NOT QPCPP_FETCH_FROM_GIT))
    set(QPCPP_FETCH_FROM_GIT $ENV{QPCPP_FETCH_FROM_GIT})
    message("Using QPCPP_FETCH_FROM_GIT from environment ('${QPCPP_FETCH_FROM_GIT}')")
endif ()

if (DEFINED ENV{QPCPP_FETCH_FROM_GIT_PATH} AND (NOT QPCPP_FETCH_FROM_GIT_PATH))
    set(QPCPP_FETCH_FROM_GIT_PATH $ENV{QPCPP_FETCH_FROM_GIT_PATH})
    message("Using QPCPP_FETCH_FROM_GIT_PATH from environment ('${QPCPP_FETCH_FROM_GIT_PATH}')")
endif ()

if (QPCPP_FETCH_FROM_GIT AND DEFINED ENV{QPCPP_URL} AND (NOT QPCPP_URL))
    set(QPCPP_URL $ENV{QPCPP_URL})
    message("Using QPCPP_URL from environment ('${QPCPP_URL}')")
endif ()

if(QPCPP_FETCH_FROM_GIT AND (NOT QPCPP_URL))
    set(QPCPP_URL "https://github.com/QuantumLeaps/qpcpp.git")
    message(STATUS "using QPCPP_URL ('${QPCPP_URL}') since not specified")
endif()

set(QPCPP_URL "${QPCPP_URL}" CACHE PATH "URL to retrieve the QPC SDK from git")
set(QPCPP_FETCH_FROM_GIT "${QPCPP_FETCH_FROM_GIT}" CACHE BOOL "Set to ON to fetch copy of SDK from git if not otherwise locatable")
set(QPCPP_FETCH_FROM_GIT_PATH "${QPCPP_FETCH_FROM_GIT_PATH}" CACHE FILEPATH "location to download SDK")

if (NOT QPCPP_SDK_PATH)
    if (QPCPP_FETCH_FROM_GIT)
        include(FetchContent)
        set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
        if (QPCPP_FETCH_FROM_GIT_PATH)
            get_filename_component(FETCHCONTENT_BASE_DIR "${QPCPP_FETCH_FROM_GIT_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
        endif ()
        # GIT_SUBMODULES_RECURSE was added in 3.17
        FetchContent_Declare(
                qpcpp
                GIT_REPOSITORY "${QPCPP_URL}"
                GIT_TAG master
                GIT_SUBMODULES_RECURSE FALSE
                GIT_SHALLOW ON
        )

        if (NOT qpcpp)
            message("Downloading QPCPP SDK")
            FetchContent_Populate(qpcpp)
            message("QPCPP source dir = '${qpcpp_SOURCE_DIR}', QPCPP binary dir = '${qpcpp_BINARY_DIR}'")
            set(QPCPP_SDK_PATH ${qpcpp_SOURCE_DIR})
        endif ()
        set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})
    else ()
        message(FATAL_ERROR
                "SDK location was not specified. Please set QPCPP_SDK_PATH or set QPCPP_FETCH_FROM_GIT to on to fetch from git."
                )
    endif ()
endif ()

file(REAL_PATH "${QPCPP_SDK_PATH}" QPCPP_SDK_PATH BASE_DIRECTORY "${CMAKE_BINARY_DIR}")
if (NOT EXISTS ${QPCPP_SDK_PATH})
    message(FATAL_ERROR "Directory '${QPCPP_SDK_PATH}' not found")
endif ()

find_file(QPCPP_INIT_CMAKE_FILE qpcpp_sdk_init.cmake
    HINTS ${QPCPP_SDK_PATH}
    PATH_SUFFIXES qpcpp-src
)
if (NOT EXISTS ${QPCPP_INIT_CMAKE_FILE})
    message(FATAL_ERROR "Directory '${QPCPP_SDK_PATH}' does not appear to contain the QPCPP SDK")
else()
    cmake_path(GET QPCPP_INIT_CMAKE_FILE PARENT_PATH QPCPP_SDK_PATH)
endif ()

set(QPCPP_SDK_PATH "${QPCPP_SDK_PATH}" CACHE PATH "Path to the QPCPP SDK" FORCE)

include(${QPCPP_INIT_CMAKE_FILE})
