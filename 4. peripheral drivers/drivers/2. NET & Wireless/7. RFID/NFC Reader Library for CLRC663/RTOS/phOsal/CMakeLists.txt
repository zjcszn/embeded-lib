

PROJECT(phOsal)

include_directories(
    inc
)

FILE(GLOB phOsal_Sources
    ./src/Linux/phOsal_Linux.c
    ./src/Linux/phOsal_Linux.h
)

ADD_LIBRARY(${PROJECT_NAME}
    ${${PROJECT_NAME}_Sources}
)

IF(${BOARD_RaspberryPi})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        -lpthread
        -lrt
    )
ENDIF(${BOARD_RaspberryPi})

IF(${BOARD_RaspberryPi})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        -lpthread
        -lrt
    )
ENDIF(${BOARD_RaspberryPi})

IF(BOARD_LPC1769)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        LibOpenRTOS
    )
ENDIF(BOARD_LPC1769)
