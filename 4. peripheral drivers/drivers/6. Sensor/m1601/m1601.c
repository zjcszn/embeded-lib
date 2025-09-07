

#include "m1601.h"
#include "gd32f30x_gpio.h"
#include "bsp_onewire.h"

#include "multi_task.h"
#include "log.h"



__attribute__((noinline))
extern int64_t get_system_ms(void);

typedef enum {
	M1601_STATE_START_MEASURE,
	M1601_STATE_WAIT,
	M1601_STATE_READ,
} m1601_state_t;




bool m1601_init(void) {

	bsp_onewire_init(GPIOB, GPIO_PIN_10);	/* 初始化单总线 */

	if (onewire_reset() == true) {
		m1601_set_resolution(M1601_Resolution_HIGH);
		m1601_mesure_start();
		return true;
	}

	m1601_state = M1601_STATE_START_MEASURE;

  	return false;
}


bool m1601_mesure_start(void) {
	if (onewire_reset() == false)
		return false;

	onewire_write_byte(ONEWIRE_CMD_SKIPROM);
	onewire_write_byte(M1601_CMD_CONVERTTEMP);
	return true;
}

bool m1601_read(float *temp) {
	uint8_t  minus = 0, res;
	uint8_t  temp_int;
	float    temp_f;
	int16_t  temp_raw;
	uint8_t  data[9];


	if (onewire_reset() == false)
		return false;

	onewire_write_byte(ONEWIRE_CMD_SKIPROM);
	onewire_write_byte(ONEWIRE_CMD_RSCRATCHPAD);

	/* Get data */
	for (int i = 0; i < 9; i++) {
		data[i] = onewire_read_byte();
	}

	/* Check if CRC is ok */
	uint8_t crc = onewire_crc8(data, 8);
	if (crc != data[8]){
		LOG("m1601 CRC error, crc: 0x%02X, data: 0x%02X\r\n", crc, data[8]);
		return false;
	}

	temp_raw = (int16_t)(((uint16_t)(data[0])) | (((uint16_t)data[1]) << 8));

	temp_f = 40.0 + ((float)temp_raw / 256.0);
	*temp = temp_f;

	LOG("m1601 temp: %.1f\r\n", temp_f);

	return true;
}


bool m1601_set_resolution(m1601_resolution_t resolution) {
	uint8_t th, tl, conf;

	
	if (onewire_reset() == false) 
		return false;

	onewire_write_byte(ONEWIRE_CMD_SKIPROM);
	onewire_write_byte(ONEWIRE_CMD_WSCRATCHPAD);

	th = (uint8_t)0;
	tl = (uint8_t)0;
	conf = (uint8_t)(0x00 + resolution);

	onewire_write_byte(th);
	onewire_write_byte(tl);
	onewire_write_byte(conf);

	if (onewire_reset() == false) 
		return false;

	onewire_write_byte(ONEWIRE_CMD_SKIPROM);
	onewire_write_byte(ONEWIRE_CMD_CPYSCRATCHPAD);

	return true;
}

