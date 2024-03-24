# This is a copy of <QPC_SDK_PATH>/3rd-party/cmake/qpc_import.cmake

# This can be dropped into an external project to help locate this SDK
# It should be include()ed prior to project()
message(STATUS "qpc_import.cmake starting...")

if (DEFINED ENV{QPC_SDK_PATH} AND (NOT QPC_SDK_PATH))
    set(QPC_SDK_PATH $ENV{QPC_SDK_PATH})
    message("Using QPC_SDK_PATH from environment ('${QPC_SDK_PATH}')")
endif ()

if (DEFINED ENV{QPC_FETCH_FROM_GIT} AND (NOT QPC_FETCH_FROM_GIT))
    set(QPC_FETCH_FROM_GIT $ENV{QPC_FETCH_FROM_GIT})
    message("Using QPC_FETCH_FROM_GIT from environment ('${QPC_FETCH_FROM_GIT}')")
endif ()

if (DEFINED ENV{QPC_FETCH_FROM_GIT_PATH} AND (NOT QPC_FETCH_FROM_GIT_PATH))
    set(QPC_FETCH_FROM_GIT_PATH $ENV{QPC_FETCH_FROM_GIT_PATH})
    message("Using QPC_FETCH_FROM_GIT_PATH from environment ('${QPC_FETCH_FROM_GIT_PATH}')")
endif ()

if (QPC_FETCH_FROM_GIT AND DEFINED ENV{QPC_URL} AND (NOT QPC_URL))
    set(QPC_URL $ENV{QPC_URL})
    message("Using QPC_URL from environment ('${QPC_URL}')")
endif ()

if(QPC_FETCH_FROM_GIT AND (NOT QPC_URL))
    set(QPC_URL "https://github.com/QuantumLeaps/qpc.git")
    message(STATUS "using QPC_URL ('${QPC_URL}') since not specified")
endif()

set(QPC_URL "${QPC_URL}" CACHE PATH "URL to retrieve the QPC SDK from git")
set(QPC_FETCH_FROM_GIT "${QPC_FETCH_FROM_GIT}" CACHE BOOL "Set to ON to fetch copy of SDK from git if not otherwise locatable")
set(QPC_FETCH_FROM_GIT_PATH "${QPC_FETCH_FROM_GIT_PATH}" CACHE FILEPATH "location to download SDK")

if (NOT QPC_SDK_PATH)
    if (QPC_FETCH_FROM_GIT)
        include(FetchContent)
        set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
        if (QPC_FETCH_FROM_GIT_PATH)
            get_filename_component(FETCHCONTENT_BASE_DIR "${QPC_FETCH_FROM_GIT_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
        endif ()
        # GIT_SUBMODULES_RECURSE was added in 3.17
        FetchContent_Declare(
                qpc
                GIT_REPOSITORY "${QPC_URL}"
                GIT_TAG master
                GIT_SUBMODULES_RECURSE FALSE
                GIT_SHALLOW ON
        )

        if (NOT qpc)
            message("Downloading QPC SDK")
            FetchContent_Populate(qpc)
            message("QPC source dir = '${qpc_SOURCE_DIR}', QPC binary dir = '${qpc_BINARY_DIR}'")
            set(QPC_SDK_PATH ${qpc_SOURCE_DIR})
        endif ()
        set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})
    else ()
        message(FATAL_ERROR
                "SDK location was not specified. Please set QPC_SDK_PATH or set QPC_FETCH_FROM_GIT to on to fetch from git."
                )
    endif ()
endif ()

file(REAL_PATH "${QPC_SDK_PATH}" QPC_SDK_PATH BASE_DIRECTORY "${CMAKE_BINARY_DIR}")
if (NOT EXISTS ${QPC_SDK_PATH})
    message(FATAL_ERROR "Directory '${QPC_SDK_PATH}' not found")
endif ()

find_file(QPC_INIT_CMAKE_FILE qpc_sdk_init.cmake
    HINTS ${QPC_SDK_PATH}
    PATH_SUFFIXES qpc-src
)
if (NOT EXISTS ${QPC_INIT_CMAKE_FILE})
    message(FATAL_ERROR "Directory '${QPC_SDK_PATH}' does not appear to contain the QPC SDK")
else()
    cmake_path(GET QPC_INIT_CMAKE_FILE PARENT_PATH QPC_SDK_PATH)
endif ()

set(QPC_SDK_PATH "${QPC_SDK_PATH}" CACHE PATH "Path to the QPC SDK" FORCE)

include(${QPC_INIT_CMAKE_FILE})
