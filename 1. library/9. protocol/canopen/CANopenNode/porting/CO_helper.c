
#include "CANopen.h"
#include "CO_platform.h"

#if ((CO_CONFIG_TIME)&CO_CONFIG_TIME_ENABLE) != 0

extern CO_t* CO;

/**
 * @brief Constant: Number of days from 1984-01-01 to 2024-01-01.
 * 
 * This constant is used to calculate the number of days since 1984-01-01 for a given civil date.
 */
static const int32_t DAYS_1984_01_01 = 5113;


/**
 * @brief Convert a civil date (year, month, day) to the number of days since 1984-01-01.
 * 
 * @param y Year (e.g., 2024).
 * @param m Month (1-12).
 * @param d Day (1-31).
 * @return int32_t Number of days since 1984-01-01.
 */
static int32_t days_from_civil(const int32_t y, const int32_t m, const int32_t d)
{
    int32_t year = y - (m <= 2 ? 1 : 0);
    int32_t era = (year >= 0) ? (year / 400) : ((year - 399) / 400);
    int32_t yoe = year - era * 400;               /* [0,399] */
    int32_t mp = m + (m > 2 ? -3 : 9);            /* [0,11] */
    int32_t doy = ( (153 * mp) + 2 ) / 5 + d - 1; /* [0,365] */
    int32_t doe = yoe * 365 + (yoe / 4) - (yoe / 100) + doy; /* [0,146096] */
    int32_t days = era * 146097 + doe - 719468;
    return days;
}

/**
 * @brief Calculate the number of days since 1984-01-01 for a given civil date.
 * 
 * @param year Year (e.g., 2024).
 * @param month Month (1-12).
 * @param day Day (1-31).
 * @return int32_t Number of days since 1984-01-01.
 */
static int32_t days_since_1984(const uint16_t year, const uint8_t month, const uint8_t day)
{
    int32_t days_now = days_from_civil((int32_t)year, (int32_t)month, (int32_t)day);
    int32_t days_1984 = days_from_civil(1984, 1, 1);
    return (int32_t)(days_now - days_1984);
}

/**
 * @brief Validate the components of a UTC date and time.
 * 
 * @param year Year (e.g., 2024).
 * @param month Month (1-12).
 * @param day Day (1-31).
 * @param hour Hour (0-23).
 * @param minute Minute (0-59).
 * @param second Second (0-60, support leap second).
 * @param millisecond Millisecond (0-999).
 * @return true If the components are valid.
 * @return false If the components are invalid.
 */
static bool validate_utc_components(const uint16_t year, const uint8_t month, const uint8_t day, const uint8_t hour, const uint8_t minute, const uint8_t second, const uint16_t millisecond)
{
    if ((year < 1970U) || (month < 1U || month > 12U) ||  (day < 1U || day > 31U) || 
        (hour > 23U) || (minute > 59U) || (second > 60U) || (millisecond > 999U)) {
        return false;
    }

    return true;
}



bool CO_Platform_SetTimestamp(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,uint8_t minute, uint8_t second, uint16_t millisecond, uint32_t producerInterval_ms)
{
    uint32_t ms_of_day;
    int32_t  days_from_1984;
    uint16_t days16;

    if (CO == NULL || CO->TIME == NULL) {
        return false;
    }

    if (validate_utc_components(year, month, day, hour, minute, second, millisecond) == false) {
        return false;
    }

    ms_of_day  = (uint32_t)hour * (uint32_t)3600000U;
    ms_of_day += (uint32_t)minute * (uint32_t)60000U;
    ms_of_day += (uint32_t)second * (uint32_t)1000U;
    ms_of_day += (uint32_t)millisecond;

    /* calculate days from 1984-01-01 */
    days_from_1984 = days_since_1984(year, month, day);

    /* check if days_from_1984 is within uint16_t range */
    if ((days_from_1984 < 0) || (days_from_1984 > 0xFFFF)) {
        return false;
    }

    days16 = (uint16_t)days_from_1984;

    /* set TIME object */
    CO_TIME_set(CO->TIME, ms_of_day, days16, producerInterval_ms);

    return true;
}

#endif   // CO_CONFIG_TIME_ENABLE




/**
 * @brief Report an emergency error.
 * 
 * @param infoCode Error code to report.
 */
void CO_Helper_ReportEmergency(uint32_t infoCode)
{
    CO_errorReport(CO->em, CO_EM_MANUFACTURER_CRITICAL, CO_EMC_DEVICE_SPECIFIC, infoCode);
}


/**
 * @brief Reset an emergency error.
 * 
 * @param infoCode Error code to reset.
 */
void CO_Helper_ResetEmergency(uint32_t infoCode)
{
    CO_errorReset(CO->em, CO_EM_MANUFACTURER_CRITICAL, infoCode);
}




#if ((CO_CONFIG_SDO_CLI)&CO_CONFIG_SDO_CLI_ENABLE) != 0



/**
 * @brief Setup an SDO client for a given node ID.
 * 
 * @param nodeId Node ID of the remote device.
 * @return CO_SDO_abortCode_t Abort code indicating success or failure.
 */
CO_SDO_abortCode_t CO_Helper_Setup_SDOclient(uint8_t nodeId)
{
    CO_SDO_return_t SDO_ret;
    CO_SDOclient_t* SDO_C = &CO->SDOclient[0];

    /* check if SDO client is initialized */
    if (SDO_C == NULL) {
        return CO_SDO_AB_GENERAL;
    }

    /* check if nodeId is valid */
    if (nodeId == 0) {
        nodeId = SDO_DEFAULT_SERVER_ID;
    }

    // setup client (this can be skipped, if remote device is the same)
    SDO_ret = CO_SDOclient_setup(SDO_C, CO_CAN_ID_SDO_CLI + nodeId, CO_CAN_ID_SDO_SRV + nodeId, nodeId);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd) {
        return CO_SDO_AB_GENERAL;
    }

    return CO_SDO_AB_NONE;
}


/**
 * @brief Read data from an SDO server.
 * 
 * @param index Index of the object to read.
 * @param subIndex Subindex of the object to read.
 * @param buf Buffer to store the read data.
 * @param bufSize Size of the buffer.
 * @param readSize Pointer to store the number of bytes read.
 * @return CO_SDO_abortCode_t Abort code indicating success or failure.
 */
CO_SDO_abortCode_t CO_Helper_Read_SDO(uint16_t index, uint8_t subIndex, uint8_t* buf, size_t bufSize, size_t* readSize) 
{
    CO_SDO_return_t SDO_ret;
    CO_SDOclient_t* SDO_C = &CO->SDOclient[0];


    // initiate upload
    SDO_ret = CO_SDOclientUploadInitiate(SDO_C, index, subIndex, 1000, false);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd) {
        return CO_SDO_AB_GENERAL;
    }

    // upload data
    do {
        uint32_t timeDifference_us = 10000;
        CO_SDO_abortCode_t abortCode = CO_SDO_AB_NONE;

        SDO_ret = CO_SDOclientUpload(SDO_C, timeDifference_us, false, &abortCode, NULL, NULL, NULL);
        if (SDO_ret < 0) {
            return abortCode;
        }

        CO_Platform_SleepMs(timeDifference_us / 1000);
    } while (SDO_ret > 0);

    // copy data to the user buffer (for long data function must be called several times inside the loop)
    *readSize = CO_SDOclientUploadBufRead(SDO_C, buf, bufSize);

    return CO_SDO_AB_NONE;
}



/**
 * @brief Write data to an SDO server.
 * 
 * @param index Index of the object to write.
 * @param subIndex Subindex of the object to write.
 * @param data Buffer containing the data to write.
 * @param dataSize Size of the data buffer.
 * @return CO_SDO_abortCode_t Abort code indicating success or failure.
 */
CO_SDO_abortCode_t CO_Helper_Write_SDO(uint16_t index, uint8_t subIndex, uint8_t* data, size_t dataSize) 
{
    CO_SDO_return_t SDO_ret;
    bool_t bufferPartial = false;
    CO_SDOclient_t* SDO_C = &CO->SDOclient[0];

    // initiate download
    SDO_ret = CO_SDOclientDownloadInitiate(SDO_C, index, subIndex, dataSize, 1000, false);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd) {
        return CO_SDO_AB_GENERAL;
    }

    // fill data
    size_t nWritten = CO_SDOclientDownloadBufWrite(SDO_C, data, dataSize);
    if (nWritten < dataSize) {
        bufferPartial = true;
        // If SDO Fifo buffer is too small, data can be refilled in the loop.
    }

    // download data
    do {
        uint32_t timeDifference_us = 10000;
        CO_SDO_abortCode_t abortCode = CO_SDO_AB_NONE;

        SDO_ret = CO_SDOclientDownload(SDO_C, timeDifference_us, false, bufferPartial, &abortCode, NULL, NULL);
        if (SDO_ret < 0) {
            return abortCode;
        }

        CO_Platform_SleepMs(timeDifference_us / 1000);
    } while (SDO_ret > 0);

    return CO_SDO_AB_NONE;
}

#endif   // CO_CONFIG_SDO_CLI_ENABLE