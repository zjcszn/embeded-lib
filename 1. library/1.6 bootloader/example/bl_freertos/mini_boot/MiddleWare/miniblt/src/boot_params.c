
#define   LOG_LVL      ELOG_LVL_ASSERT
#define   LOG_TAG     "BOOT"

#include "boot_params.h"
#include "elog.h"
#include <string.h>

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
boot_params_t boot_params __attribute__((section(".bss.NoInit")));  /* this is demo for armcc v6 */



static uint16_t boot_params_calc_checksum(void);

static bool boot_params_is_valid(void);


/**
 * @brief Initializes the shared RAM parameters module.
 * 
 * @return True if is a cold boot, false otherwise. 
 */
bool boot_params_init(void) {
    log_d("boot params address: %p", &boot_params);
    bool result = false;
    if (!boot_params_is_valid()) {
        boot_params_reset();
        result = true;
        log_d("boot params init success");
    }
    return result;
}

/**
 * @brief Reset the shared RAM parameters module.
 * 
 */
void boot_params_reset(void) {
    memset((void *) &boot_params, 0x00, sizeof(boot_params_t));
    boot_params.magic_word = BOOT_PARAMS_MAGIC_WORD;
    boot_params.checksum = boot_params_calc_checksum();
}


/**
 * @brief Reads a data byte from the shared parameter buffer at the specified index.
 * 
 * @param idx Index into the parameter data array. A valid value is between 0 and
 *            (BOOT_PARAMS_CFG_BUFFER_DATA_LEN - 1).
 * 
 * @param value Pointer to where the read data value is stored.
 * 
 * @return True if successful, false otherwise.
 */
bool boot_params_read(uint32_t idx, uint32_t *value) {
    bool result = false;
    if (boot_params_is_valid() && (idx < BOOT_PARAMS_DATA_LENGTH)) {
        *value = boot_params.data[idx];
        result = true;
    }

    log_d("boot params read  %s, [%2u]: %02X, checksum: %04X", result ? "success" : " failed",
          idx, *value, boot_params.checksum);

    return result;
}

/**
 * @brief Writes a data byte to the shared parameter buffer at the specified index.
 * 
 * @param idx Index into the parameter data array. A valid value is between 0 and
 *            (BOOT_PARAMS_CFG_BUFFER_DATA_LEN - 1).
 * 
 * @param value Value to write.
 * 
 * @return True if successful, false otherwise.
 */
bool boot_params_write(uint32_t idx, uint32_t value) {
    bool result = false;

    if (boot_params_is_valid() && (idx < BOOT_PARAMS_DATA_LENGTH)) {
        boot_params.data[idx] = value;
        boot_params.checksum = boot_params_calc_checksum();
        result = true;
    }

    log_d("boot params write %s, [%2u]: %02X, checksum: %04X", result ? "success" : " failed",
          idx, value, boot_params.checksum);

    return result;
}

/**
 * @brief Validates the shared parameter buffer contents by looking at the table
 *        identifier and verifying its checksum.
 * 
 * @return True if successful, false otherwise.
 */
static bool boot_params_is_valid(void) {
    if ((boot_params.magic_word == BOOT_PARAMS_MAGIC_WORD) &&
        (boot_params.checksum == boot_params_calc_checksum())) {
        return true;
    }
    return false;
}

/**
 * @brief Calculates and returns the checksum value for the current contents in the
 *        buffer. The checksum is calculated by taking the sum of all bytes in the
 *        parameter buffer (excluding the checksum at the end) and them taking the
 *        two's complement value of it.
 * 
 * @return The calculated checksum value.
 */
static uint16_t boot_params_calc_checksum(void) {
    uint32_t checksum = 0;

    volatile uint16_t *data = (volatile uint16_t *) (&boot_params);

    for (int i = 0; i < BOOT_PARAMS_CHECKSUM_CALC_LENGTH; i++) {
        checksum += data[i];
    }

    while (checksum & 0xFFFF0000) {
        checksum = (checksum >> 16) + (checksum & 0x0000FFFF);
    }
    checksum = ~checksum;
    return (uint16_t) (checksum & 0x0000FFFF);
} 