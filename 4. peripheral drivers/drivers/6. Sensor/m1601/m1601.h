#ifndef __M1601_H__
#define __M1601_H__

#include <stdbool.h>
#include <stdint.h>


/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of M1601 this is 0x28 and this is first byte of ROM address */
#define M1601_FAMILY_CODE 		0x28
#define M1601_CMD_ALARMSEARCH 	0xEC

/* M1601 read temperature command */
#define M1601_CMD_CONVERTTEMP 	0x44 /* Convert temperature */




typedef enum {
	M1601_Resolution_LOW = 0x00,
    M1601_Resolution_MID = 0x01,
	M1601_Resolution_HIGH = 0x02,
} m1601_resolution_t;



bool m1601_init(void);
bool m1601_mesure_start(void);
bool m1601_read(float *temp);
bool m1601_set_resolution(m1601_resolution_t resolution);
float m1601_get_temp(void);
uint8_t m1601_crc8(uint8_t *data, uint16_t len);

#endif