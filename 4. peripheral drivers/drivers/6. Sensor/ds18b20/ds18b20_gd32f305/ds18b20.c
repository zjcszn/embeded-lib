

#include "ds18b20.h"
#include "gd32f30x_gpio.h"
#include "bsp_onewire.h"
#include "bsp_sensor.h"


bool ds18b20_init(void) {

	bsp_onewire_init(SENSOR_TEMP_PORT, SENSOR_TEMP_PIN);	/* 初始化单总线 */

	if (onewire_reset() == true) {
		ds18b20_set_resolution(DS18B20_Resolution_12bits);
		return true;
	}

  	return false;
}


bool ds18b20_mesure_start(void) {
	if (onewire_reset() == false)
		return false;

	onewire_write_byte(ONEWIRE_CMD_SKIPROM);
	onewire_write_byte(DS18B20_CMD_CONVERTTEMP);
	return true;
}


bool ds18b20_read(float *temp) {
	uint8_t  minus = 0, res;
	uint8_t  temp_int;
	float    temp_decimal;
	uint16_t temp_raw;
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
	if (onewire_crc8(data, 8) != data[8])
		return false;

	temp_raw = data[0] | (data[1] << 8);

	/* 判断温度值正负 */
	if (temp_raw & 0x8000) {
		temp_raw = ~temp_raw + 1;
		minus = 1;
	}

	/* Get sensor resolution */
	res = (data[4] >> 5) & 0x03;

	/* Store temperature integer digits and decimal digits */
	static const uint8_t decimal_shift[] = {3, 2, 1, 0};
	static const float   decimal_resolution[] = {0.5f, 0.25f, 0.125f, 0.0625f};

	temp_decimal =  ((temp_raw & 0xF) >> decimal_shift[res]) * decimal_resolution[res];
	temp_int = (temp_raw >> 4) & 0x7F;
	temp_decimal += temp_int;

	if (minus) {temp_decimal = 0 - temp_decimal;}

	*temp = temp_decimal;

	return true;
}


bool ds18b20_set_resolution(ds18b20_resolution_t resolution) {
	uint8_t th, tl, conf;

	
	if (onewire_reset() == false) 
		return false;

	onewire_write_byte(ONEWIRE_CMD_SKIPROM);
	onewire_write_byte(ONEWIRE_CMD_WSCRATCHPAD);

	th = (uint8_t)125;
	tl = (uint8_t)-55;
	conf = 0x1F + resolution;

	onewire_write_byte(th);
	onewire_write_byte(tl);
	onewire_write_byte(conf);

	if (onewire_reset() == false) 
		return false;

	onewire_write_byte(ONEWIRE_CMD_SKIPROM);
	onewire_write_byte(ONEWIRE_CMD_CPYSCRATCHPAD);

	return true;
}

