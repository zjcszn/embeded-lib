//
// Created by zjcszn on 2024/4/6.
//

#include <stdio.h>
#include "phLog.h"
#include "phApp_Log.h"

#ifdef NXPBUILD__PH_LOG

void phApp_Log(void *pDataParams, uint8_t bOption, phLog_LogEntry_t *pLogEntries, uint16_t wEntryCount) {
    printf("\r\n");
    for (int i = 0; i < wEntryCount; i++) {
        if (pLogEntries[i].pString) {
            printf("%s: ", (char *)pLogEntries[i].pString);
        }
        if (pLogEntries[i].wDataLen) {
            printf("0x");
            for (int offset = 0; offset < pLogEntries[i].wDataLen; offset++) {
                printf("%02X", ((char *)pLogEntries[i].pData)[offset]);
            }
        }
        printf("\r\n");
    }
    printf("\r\n");
}

#endif