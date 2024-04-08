
PROJECT(NxpApp)

add_library(${PROJECT_NAME} STATIC
        src/phApp_Helper.c
        src/phApp_Init.c
        src/phApp_RC663_Init.c
        src/phApp_Demo.c
)

include_directories(
        inc
)
