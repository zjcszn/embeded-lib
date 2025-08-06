#include "bsp_onewire.h"

#include "stdint.h"
#include "gd32f30x_gpio.h"
#include "dwt_delay.h"
#include "stdio.h"

typedef struct {
    uint32_t gpio;
    uint32_t pin;
} onewire_handle_t;

static onewire_handle_t onewire_handle;

static uint8_t onewire_timing[][2] = {{65, 5}, {5, 65}};

void bsp_onewire_init(uint32_t gpio, uint32_t pin) {
    dwt_init();

    onewire_handle.gpio = gpio;
    onewire_handle.pin = pin;
    
    gpio_init(onewire_handle.gpio, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, onewire_handle.pin);
    gpio_bit_set(onewire_handle.gpio, onewire_handle.pin);
    dwt_delay_ms(10);
}

bool onewire_reset(void) {
    FlagStatus ret;

    gpio_bit_reset(onewire_handle.gpio, onewire_handle.pin);
    dwt_delay_us(500);
    gpio_bit_set(onewire_handle.gpio, onewire_handle.pin);
    gpio_init(onewire_handle.gpio, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, onewire_handle.pin);
    dwt_delay_us(70);
    ret = gpio_input_bit_get(onewire_handle.gpio, onewire_handle.pin);
    dwt_delay_us(430);
    gpio_init(onewire_handle.gpio, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, onewire_handle.pin);

    if (ret == SET) {
        printf("ds18b20 init failed");
        return false;
    }
    return true;
}

void onewire_write_byte(uint8_t byte) {
    uint8_t bit;

    for (int i = 0; i < 8; i++) {
        bit = byte & 0x01;
        gpio_bit_reset(onewire_handle.gpio, onewire_handle.pin);
        dwt_delay_us(onewire_timing[bit][0]);
        gpio_bit_set(onewire_handle.gpio, onewire_handle.pin);
        dwt_delay_us(onewire_timing[bit][1]);
        byte >>= 1;
    }

}

uint8_t onewire_read_byte(void) {
    uint8_t byte;

    for (int i = 0; i < 8; i++) {
        byte >>= 1;
        gpio_bit_reset(onewire_handle.gpio, onewire_handle.pin);
        dwt_delay_us(2);
        gpio_init(onewire_handle.gpio, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, onewire_handle.pin);
        dwt_delay_us(10);
        byte |= (gpio_input_bit_get(onewire_handle.gpio, onewire_handle.pin) << 7);
        gpio_init(onewire_handle.gpio, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, onewire_handle.pin);
		gpio_bit_set(onewire_handle.gpio, onewire_handle.pin);
        dwt_delay_us(50);
    }
    return byte;
}

uint8_t onewire_crc8(uint8_t *addr, uint8_t len) {
	uint8_t crc = 0, inbyte, i, mix;

	while (len--) {
		inbyte = *addr++;
		for (i = 8; i; i--) {
			mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) {
				crc ^= 0x8C;
			}
			inbyte >>= 1;
		}
	}

	/* Return calculated CRC */
	return crc;
}