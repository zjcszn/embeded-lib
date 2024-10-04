

PROJECT(NxpRdLib_alMfp)

FILE(GLOB NxpRdLib_alMfp_Sources
    ./src/phalMfp.c
    ./src/phalMfp_Int.c
    ./src/phalMfp_Int.h
    ./src/SamAV2/phalMfp_SamAV2.c
    ./src/SamAV2/phalMfp_SamAV2.h
    ./src/SamAV2/phalMfp_SamAV2_Int.h
    ./src/SamAV2_X/phalMfp_SamAV2_X.c
    ./src/SamAV2_X/phalMfp_SamAV2_X.h
    ./src/SamAV2_X/phalMfp_SamAV2_X_Int.h
    ./src/Sw/phalMfp_Sw.c
    ./src/Sw/phalMfp_Sw.h
    ./src/Sw/phalMfp_Sw_Int.h
)
ADD_LIBRARY(${PROJECT_NAME}
    ${${PROJECT_NAME}_Sources}
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    NxpRdLib_palMifare
    NxpRdLib_CryptoSym
    NxpRdLib_CryptoRng
)