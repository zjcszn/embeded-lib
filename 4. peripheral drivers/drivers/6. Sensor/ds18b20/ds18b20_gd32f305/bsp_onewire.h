#ifndef __BSP_ONEWIRE_H__
#define __BSP_ONEWIRE_H__

#include "stdint.h"
#include "stdbool.h"

/* OneWire commands */
#define ONEWIRE_CMD_RSCRATCHPAD     0xBE
#define ONEWIRE_CMD_WSCRATCHPAD     0x4E
#define ONEWIRE_CMD_CPYSCRATCHPAD   0x48
#define ONEWIRE_CMD_RECEEPROM       0xB8
#define ONEWIRE_CMD_RPWRSUPPLY      0xB4
#define ONEWIRE_CMD_SEARCHROM       0xF0
#define ONEWIRE_CMD_READROM         0x33
#define ONEWIRE_CMD_MATCHROM        0x55
#define ONEWIRE_CMD_SKIPROM         0xCC

void    bsp_onewire_init(uint32_t gpio, uint32_t pin);
bool    onewire_reset(void);
void    onewire_write_byte(uint8_t byte);
uint8_t onewire_read_byte(void);
uint8_t onewire_crc8(uint8_t *addr, uint8_t len);


#endif