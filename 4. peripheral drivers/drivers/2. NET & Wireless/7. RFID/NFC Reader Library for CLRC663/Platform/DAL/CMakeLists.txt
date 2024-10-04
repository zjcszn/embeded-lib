

PROJECT(DAL)

include_directories(
    inc
    boards
    cfg
)

FILE(GLOB DAL_Sources
    ./src/Linux/*_Linux.c
    ./src/Linux/*_Linux_*.c
    ./src/Linux/*_Linux_*.h
)


ADD_LIBRARY(${PROJECT_NAME}
    ${${PROJECT_NAME}_Sources}
)

IF(BUILD_COMPILER_armnoneeabigcc AND BOARD_LPC1769)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        lpc_board_nxp_lpcxpresso_1769
        lpc_chip_175x_6x
        c
    )
ENDIF()
