

PROJECT(NfcrdlibEx1_DiscoveryLoop)


FILE(GLOB NfcrdlibEx1_DiscoveryLoop_Sources
    *.c
    src/*.c
)

include_directories(
    intfs
    types
)

ADD_EXECUTABLE(NfcrdlibEx1_DiscoveryLoop
    ${NfcrdlibEx1_DiscoveryLoop_Sources}
)

TARGET_LINK_LIBRARIES(
    NfcrdlibEx1_DiscoveryLoop
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
    NxpRdLib_palI18092mT
    NxpRdLib_CryptoSym
    NxpRdLib_CryptoRng
    NxpRdLib_palI14443p4mC
    NxpRdLib_ceT4T
    NxpRdLib_palEpcUid
    NxpRdLib_alMfdfEVx
    NxpRdLib_alMfNtag42XDna
    NxpRdLib_phNfcLib
)
