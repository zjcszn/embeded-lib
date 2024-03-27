

PROJECT(NxpRdLib_alMfdfEVx)

FILE(GLOB NxpRdLib_alMfdfEVx_Sources
    ./src/phalMfdfEVx.c
    ./src/phalMfdfEVx_Int.c
    ./src/phalMfdfEVx_Int.h
    ./src/Sw/phalMfdfEVx_Sw.c
    ./src/Sw/phalMfdfEVx_Sw.h
    ./src/Sw/phalMfdfEVx_Sw_Int.c
    ./src/Sw/phalMfdfEVx_Sw_Int.h
)
ADD_LIBRARY(${PROJECT_NAME}
    ${${PROJECT_NAME}_Sources}
)


TARGET_LINK_LIBRARIES(
    NxpRdLib_alMfdfEVx
    NxpRdLib_TMIUtils
)
