#ifndef __SHARED_PARAMS_H__
#define __SHARED_PARAMS_H__

#include <stdint.h>
#include <stdbool.h>


/** 
 * @brief Constant parameter buffer identifier. This value is always located as the
 *        start of the buffer to validate the the RAM contains valid shared parameters.
 */
#define SHARED_PARAMS_MAGIC_WORD                (0xCE42E7A2U)


/** 
 * @brief Configuration macro for specifying the size of the data inside the parameter
 *        buffer. This is the length in bytes of the actual parameter data, so
 *        excluding the bufferId and checksum.
 */
#define SHARED_PARAMS_DATA_LENGTH               (7U)

#define SHARED_PARAMS_CHECKSUM_CALC_LENGTH      (((SHARED_PARAMS_DATA_LENGTH + 1U) >> 1U) + 2U)



struct _shared_params
{
    uint32_t magic_word;
    uint8_t  data[SHARED_PARAMS_DATA_LENGTH];
    uint16_t checksum;
};
typedef struct _shared_params shared_params_t;


bool shared_params_init(void);
void shared_params_reset(void);
bool shared_params_read_by_index(uint32_t idx, uint8_t *value);
bool shared_params_write_by_index(uint32_t idx, uint8_t value);


#endif