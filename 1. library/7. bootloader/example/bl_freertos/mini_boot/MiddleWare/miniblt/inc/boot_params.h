#ifndef __BOOT_PARAMS_H__
#define __BOOT_PARAMS_H__

#include <stdint.h>
#include <stdbool.h>


/** 
 * @brief Constant parameter buffer identifier. This value is always located as the
 *        start of the buffer to validate the the RAM contains valid shared parameters.
 */
#define BOOT_PARAMS_MAGIC_WORD                (0xCE42E7A2U)


/** 
 * @brief Configuration macro for specifying the size of the data inside the parameter
 *        buffer. This is the length in bytes of the actual parameter data, so
 *        excluding the bufferId and checksum.
 */
#define BOOT_PARAMS_DATA_LENGTH               (3U)

#define BOOT_PARAMS_CHECKSUM_CALC_LENGTH      ((BOOT_PARAMS_DATA_LENGTH + 1U) << 1U)



struct _boot_flags
{
    uint32_t magic_word;
    uint32_t data[BOOT_PARAMS_DATA_LENGTH];
    uint16_t checksum;
};
typedef struct _boot_flags boot_params_t;


bool boot_params_init(void);
void boot_params_reset(void);
bool boot_params_read(uint32_t idx, uint32_t *value);
bool boot_params_write(uint32_t idx, uint32_t value);


#endif