#include "shared_params.h"


#include <string.h>


#define   LOG_LVL      ELOG_LVL_DEBUG 
#define   LOG_TAG     "BOOT_SHARERAM"

#include "elog.h"


/**
 * @brief Declaration of the actual parameter buffer that this module manages.
 * 
 * @warning For the shared RAM parameters to work properly for sharing information
 *          between the bootloader and user program, it is important that this variable
 *          is linked to the exact same RAM address in both the bootloader and the user 
 *          program. Additionally, it should be configured such that the C-startup code
 *          does NOT zero its contents during system initialization. This is the code 
 *          that runs in the reset event handler, before function main() is called.
 * 
 * @remark  Here are a few links to get you started:
 *          IAR EWARM:
 *          https://www.iar.com/support/tech-notes/compiler/linker-error-for-absolute-located-variable/
 *          Keil MDK:
 *          http://www.keil.com/support/docs/3480.htm
 */
shared_params_t shared_params __attribute__((section(".bss.NoInit")));  /* this is demo for armcc v6 */



static uint16_t shared_params_calc_checksum(void);
static bool     shared_params_is_valid(void);


/**
 * @brief Initializes the shared RAM parameters module.
 * 
 * @return True if is a cold boot, false otherwise. 
 */
bool shared_params_init(void)
{
    log_d("shared params address: %p", &shared_params);
    bool result = false;
    if (!shared_params_is_valid())
    {
        shared_params_reset();
        result = true;
        log_i("shared params module init success");
    }
    return result;
}

/**
 * @brief Reset the shared RAM parameters module.
 * 
 */
void shared_params_reset(void)
{
    memset(&shared_params, 0x00, sizeof(shared_params_t));
    shared_params.magic_word = SHARED_PARAMS_MAGIC_WORD;
    shared_params.checksum   = shared_params_calc_checksum();
}


/**
 * @brief Reads a data byte from the shared parameter buffer at the specified index.
 * 
 * @param idx Index into the parameter data array. A valid value is between 0 and
 *            (SHARED_PARAMS_CFG_BUFFER_DATA_LEN - 1).
 * 
 * @param value Pointer to where the read data value is stored.
 * 
 * @return True if successful, false otherwise.
 */
bool shared_params_read_by_index(uint32_t idx, uint8_t *value)
{
    bool result = false;
    if ( shared_params_is_valid() && (idx < SHARED_PARAMS_DATA_LENGTH) )
    {
        *value = shared_params.data[idx];
        result = true;
    }

    log_d("shared params read  %s, [%2u]: %02X, checksum: %04X", result ? "success" : " failed",
            idx, *value, shared_params.checksum);

    return result;
} 

/**
 * @brief Writes a data byte to the shared parameter buffer at the specified index.
 * 
 * @param idx Index into the parameter data array. A valid value is between 0 and
 *            (SHARED_PARAMS_CFG_BUFFER_DATA_LEN - 1).
 * 
 * @param value Value to write.
 * 
 * @return True if successful, false otherwise.
 */
bool shared_params_write_by_index(uint32_t idx, uint8_t value)
{
    bool result = false;

    if ( shared_params_is_valid() && (idx < SHARED_PARAMS_DATA_LENGTH))
    {
        shared_params.data[idx] = value;
        shared_params.checksum = shared_params_calc_checksum();
        result = true;
    }
    
    log_d("shared params write %s, [%2u]: %02X, checksum: %04X", result ? "success" : " failed",
            idx, value, shared_params.checksum);

    return result;
}


/**
 * @brief Validates the shared parameter buffer contents by looking at the table
 *        identifier and verifying its checksum.
 * 
 * @return True if successful, false otherwise.
 */
static bool shared_params_is_valid(void)
{
    bool result = false;

    if ((shared_params.magic_word == SHARED_PARAMS_MAGIC_WORD) &&
        (shared_params.checksum == shared_params_calc_checksum()))
    {
        result = true;
    }
    return result;
}

/**
 * @brief Calculates and returns the checksum value for the current contents in the
 *        buffer. The checksum is calculated by taking the sum of all bytes in the
 *        parameter buffer (excluding the checksum at the end) and them taking the
 *        two's complement value of it.
 * 
 * @return The calculated checksum value.
 */
static uint16_t shared_params_calc_checksum(void)
{
    uint32_t checksum = 0;
    uint16_t *data = (uint16_t *)(&shared_params);

    for (int i = 0; i < SHARED_PARAMS_CHECKSUM_CALC_LENGTH; i++) 
    {
        checksum += data[i];
    }
    
    while (checksum >> 16)
    {
        checksum = (checksum >> 16) + (checksum & 0x0000FFFF);
    }
    checksum = ~checksum;
    return (uint16_t)(checksum & 0x0000FFFF);
} 