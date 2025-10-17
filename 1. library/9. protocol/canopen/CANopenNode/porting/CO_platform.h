#ifndef __CO_PLATFORM_H__
#define __CO_PLATFORM_H__

#include <stdint.h>
#include <stdbool.h>

#include "CANopen.h"

/* =============== CANopen Platform Typedef =============== */
typedef uint64_t     CO_time_t;

struct  CAN_Module;
typedef CAN_Module   CO_CAN_Handle_t;




/*================== CANopen Platform Log ================= */

#define CO_LOG_ENABLE                   1 



#if (CO_LOG_ENABLE == 1)

#define  CO_EMERGENCY_LOG_ENABLE        1
#define  CO_NMT_LOG_ENABLE              1
#define  CO_SDO_LOG_ENABLE              1
#define  CO_PDO_LOG_ENABLE              1
#define  CO_EMERGENCY_LOG_ENABLE        1
#define  CO_OD_LOG_ENABLE               1
#define  CO_DRIVER_LOG_ENABLE           1


#define  LOG_TAG    "CANOPEN"
#define  LOG_LVL     ELOG_LVL_DEBUG
#include "elog.h"

#define  CO_LOG_Debug(...)    log_d(__VA_ARGS__)
#define  CO_LOG_Info(...)     log_i(__VA_ARGS__)
#define  CO_LOG_Error(...)    log_e(__VA_ARGS__)

#else

#define  CO_EMERGENCY_LOG_ENABLE        0
#define  CO_NMT_LOG_ENABLE              0
#define  CO_SDO_LOG_ENABLE              0
#define  CO_PDO_LOG_ENABLE              0

#define  CO_LOG_Debug(...)    ((void)0U)
#define  CO_LOG_Info(...)     ((void)0U)
#define  CO_LOG_Error(...)    ((void)0U)
#endif  // CO_LOG_ENABLE



#ifndef CO_USE_GLOBALS

#define CO_alloc(num, size)     CO_Platform_Calloc((num), (size))
#define CO_free(ptr)            CO_Platform_Free((ptr))

#endif




uint32_t  CO_Platform_Lock(void);
void      CO_Platform_Unlock(uint32_t primask);
void*     CO_Platform_Calloc(uint32_t num, uint32_t size);
void      CO_Platform_Free(void* ptr);
CO_time_t CO_Platform_GetMs(void);
bool      CO_Platform_SetTimestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,uint8_t minute, uint8_t second, uint16_t millisecond, uint32_t producerInterval_ms);
void      CO_Platform_SleepMs(CO_time_t ms);

CO_SDO_abortCode_t CO_Helper_Setup_SDOclient(uint8_t nodeId);
CO_SDO_abortCode_t CO_Helper_Read_SDO(uint16_t index, uint8_t subIndex, uint8_t* buf, size_t bufSize, size_t* readSize);
CO_SDO_abortCode_t CO_Helper_Write_SDO(uint16_t index, uint8_t subIndex, uint8_t* data, size_t dataSize);


#endif