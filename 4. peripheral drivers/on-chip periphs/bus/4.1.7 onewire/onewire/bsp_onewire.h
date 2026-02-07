#ifndef __BSP_ONEWIRE_H__
#define __BSP_ONEWIRE_H__

#include "onewire.h"

/**
 * @brief Initialize the default onewire bus
 *
 * @param gpio GPIO port
 * @param pin GPIO pin
 * @return onewire_bus_t* Pointer to the initialized bus handle
 */
onewire_bus_t *bsp_onewire_init(uint32_t gpio, uint32_t pin);

/*
 * Legacy API wrappers (optional, for backward compatibility if needed)
 * It is recommended to use the new API directly:
 * onewire_reset(bus);
 * onewire_write_byte(bus, byte);
 * onewire_read_byte(bus);
 */

#endif /* __BSP_ONEWIRE_H__ */