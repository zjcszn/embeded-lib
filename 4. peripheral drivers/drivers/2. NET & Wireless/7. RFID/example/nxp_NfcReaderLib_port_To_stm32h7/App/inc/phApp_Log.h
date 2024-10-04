//
// Created by zjcszn on 2024/4/6.
//

#ifndef PHAPP_LOG_H_
#define PHAPP_LOG_H_

#if defined(NXPBUILD__PH_LOG)
void phApp_Log(void *pDataParams, uint8_t bOption, phLog_LogEntry_t *pLogEntries, uint16_t wEntryCount);
#endif

#endif //PHAPP_LOG_H_
