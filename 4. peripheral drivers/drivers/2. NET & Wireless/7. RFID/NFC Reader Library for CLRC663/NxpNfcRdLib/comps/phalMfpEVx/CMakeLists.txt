

PROJECT(NxpRdLib_alMfpEVx)

FILE(GLOB NxpRdLib_alMfpEVx_Sources
    ./src/phalMfpEVx.c
    ./src/phalMfpEVx_Int.c
    ./src/phalMfpEVx_Int.h
    ./src/Sw/phalMfpEVx_Sw.c
    ./src/Sw/phalMfpEVx_Sw.h
    ./src/Sw/phalMfpEVx_Sw_Int.h
)
ADD_LIBRARY(${PROJECT_NAME}
    ${${PROJECT_NAME}_Sources}
)


TARGET_LINK_LIBRARIES(
    NxpRdLib_alMfpEVx
    NxpRdLib_TMIUtils
)
