

PROJECT(NfcrdlibEx6_LPCD)


FILE(GLOB NfcrdlibEx6_LPCD_Sources
    *.c
    src/*.c
)

include_directories(
    intfs
    types
)

ADD_EXECUTABLE(NfcrdlibEx6_LPCD
    ${NfcrdlibEx6_LPCD_Sources}
)

TARGET_LINK_LIBRARIES(
    NfcrdlibEx6_LPCD
    phOsal
    DAL
    NxpRdLib_acDiscLoop
    NxpRdLib_halHw
    NxpRdLib_KeyStore
    NxpRdLib_phTools
    NxpRdLib_palFelica
    NxpRdLib_palI18092mPI
    NxpRdLib_palI14443p4a
    NxpRdLib_palI14443p4
    NxpRdLib_palI14443p4
    NxpRdLib_palMifare
    NxpRdLib_alFelica
    NxpRdLib_alMful
    NxpRdLib_alMfdf
    NxpRdLib_alT1T
    NxpRdLib_alTop
    NxpRdLib_alMfc
    NxpRdLib_lnLlcp
    NxpRdLib_phTools
    NxpRdLib_palI18092mT
    NxpRdLib_npSnep
    NxpRdLib_palI14443p4mC
    NxpRdLib_ceT4T
    NxpRdLib_palEpcUid
    NxpRdLib_alMfNtag42XDna
    NxpRdLib_phNfcLib
)

