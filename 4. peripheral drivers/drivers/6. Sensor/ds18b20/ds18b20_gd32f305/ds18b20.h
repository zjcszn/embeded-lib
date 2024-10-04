
#ifndef _DS18B20_H
#define _DS18B20_H

#include <stdbool.h>


/* Every onewire chip has different ROM code, but all the same chips has same family code */
/* in case of DS18B20 this is 0x28 and this is first byte of ROM address */
#define DS18B20_FAMILY_CODE 		0x28
#define DS18B20_CMD_ALARMSEARCH 	0xEC

/* DS18B20 read temperature command */
#define DS18B20_CMD_CONVERTTEMP 	0x44 /* Convert temperature */
#define DS18B20_DECIMAL_STEPS_12BIT 0.0625f
#define DS18B20_DECIMAL_STEPS_11BIT 0.125f
#define DS18B20_DECIMAL_STEPS_10BIT 0.25f
#define DS18B20_DECIMAL_STEPS_9BIT 	0.5f



typedef enum {
	DS18B20_Resolution_9bits,   /*!< DS18B20 9 bits resolution */
	DS18B20_Resolution_10bits,  /*!< DS18B20 10 bits resolution */
	DS18B20_Resolution_11bits,  /*!< DS18B20 11 bits resolution */
	DS18B20_Resolution_12bits   /*!< DS18B20 12 bits resolution */
} ds18b20_resolution_t;



bool ds18b20_init(void);
bool ds18b20_mesure_start(void);
bool ds18b20_read(float *temp);
bool ds18b20_set_resolution(ds18b20_resolution_t resolution);


#endif