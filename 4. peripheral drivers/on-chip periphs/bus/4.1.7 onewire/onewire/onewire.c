#include "onewire.h"

#include <string.h>

/* Critical section macros must be provided by the user if needed.
   Typically defined in a config header or passed via compiler flags.
   If not defined, default to empty. */
#ifndef ONEWIRE_ENTER_CRITICAL
#define ONEWIRE_ENTER_CRITICAL() (void) 0
#endif

#ifndef ONEWIRE_EXIT_CRITICAL
#define ONEWIRE_EXIT_CRITICAL() (void) 0
#endif

#ifndef ONEWIRE_DELAY_US
#error "ONEWIRE_DELAY_US not defined"
#endif

void onewire_init(onewire_bus_t *bus, const onewire_ops_t *ops, void *port_data) {
    if (bus) {
        bus->ops = ops;
        bus->port_data = port_data;
    }
}

static void onewire_write_bit(const onewire_bus_t *bus, uint8_t bit) {
    if (!bus || !bus->ops) {
        return;
    }

    ONEWIRE_ENTER_CRITICAL();

    bus->ops->set_pin(bus->port_data, 0);

    if (bit) {
        ONEWIRE_DELAY_US(5);  // Write 1 low time
        bus->ops->set_pin(bus->port_data, 1);
        ONEWIRE_DELAY_US(65);  // Write 1 high time
    } else {
        ONEWIRE_DELAY_US(65);  // Write 0 low time
        bus->ops->set_pin(bus->port_data, 1);
        ONEWIRE_DELAY_US(5);  // Write 0 recovery
    }

    ONEWIRE_EXIT_CRITICAL();
}

static uint8_t onewire_read_bit(const onewire_bus_t *bus) {
    uint8_t bit = 0;

    if (!bus || !bus->ops) {
        return 0;
    }

    ONEWIRE_ENTER_CRITICAL();

    bus->ops->set_pin(bus->port_data, 0);
    ONEWIRE_DELAY_US(2);  // Read start

    bus->ops->set_pin(bus->port_data, 1);
    ONEWIRE_DELAY_US(8);  // Wait for sample

    if (bus->ops->get_pin(bus->port_data)) {
        bit = 1;
    }

    ONEWIRE_DELAY_US(50);  // Slot end recovery

    ONEWIRE_EXIT_CRITICAL();

    return bit;
}

bool onewire_reset(onewire_bus_t *bus) {
    uint8_t ret;

    if (!bus || !bus->ops) {
        return false;
    }

    /* Reset pulse is timing critical and long, but usually we don't disable ITs for 500us+ */
    /* If strict timing is needed, one might consider CRITICAL here too, but it blocks system too
     * long. */
    /* Standard practice is usually not to block for reset pulse unless strictly necessary. */

    bus->ops->set_pin(bus->port_data, 0);
    ONEWIRE_DELAY_US(500);

    bus->ops->set_pin(bus->port_data, 1);
    ONEWIRE_DELAY_US(70);

    ret = bus->ops->get_pin(bus->port_data);

    ONEWIRE_DELAY_US(430);

    return (ret == 0);  // 0 means device pulled low (presence detected)
}

void onewire_write_byte(const onewire_bus_t *bus, uint8_t byte) {
    uint8_t i;

    if (!bus || !bus->ops) {
        return;
    }

    for (i = 0; i < 8; i++) {
        onewire_write_bit(bus, byte & 0x01);
        byte >>= 1;
    }
}

uint8_t onewire_read_byte(const onewire_bus_t *bus) {
    uint8_t i;
    uint8_t byte = 0;

    if (!bus || !bus->ops) {
        return 0;
    }

    for (i = 0; i < 8; i++) {
        if (onewire_read_bit(bus)) {
            byte |= (1 << i);
        }
    }

    return byte;
}

void onewire_skip_rom(const onewire_bus_t *bus) {
    onewire_write_byte(bus, ONEWIRE_CMD_SKIPROM);
}

void onewire_match_rom(const onewire_bus_t *bus, const uint8_t *rom_id) {
    uint8_t i;

    onewire_write_byte(bus, ONEWIRE_CMD_MATCHROM);

    for (i = 0; i < 8; i++) {
        onewire_write_byte(bus, rom_id[i]);
    }
}

uint8_t onewire_crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0, inbyte, i, mix;

    while (len--) {
        inbyte = *data++;
        for (i = 8; i > 0; i--) {
            mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) {
                crc ^= 0x8C;
            }
            inbyte >>= 1;
        }
    }
    return crc;
}

void onewire_search_start(onewire_search_state_t *state) {
    if (state) {
        memset(state, 0, sizeof(onewire_search_state_t));
        state->last_discrepancy = -1;
        state->last_family_discrepancy = -1;
        state->last_device_flag = false;
    }
}

bool onewire_search_next(onewire_bus_t *bus, onewire_search_state_t *state) {
    int8_t id_bit_number;
    int8_t last_zero, rom_byte_number;
    uint8_t id_bit, cmp_id_bit;
    uint8_t rom_byte_mask, search_direction;

    /* Initialize for search */
    id_bit_number = 1;
    last_zero = -1;
    rom_byte_number = 0;
    rom_byte_mask = 1;

    if (!state || state->last_device_flag) {
        return false;
    }

    /* 1-Wire reset */
    if (!onewire_reset(bus)) {
        state->last_discrepancy = -1;
        state->last_family_discrepancy = -1;
        state->last_device_flag = false;
        return false;
    }

    /* Issue search command */
    onewire_write_byte(bus, ONEWIRE_CMD_SEARCHROM);

    /* Loop to do the search */
    do {
        /* Read a bit and its complement */
        id_bit = onewire_read_bit(bus);
        cmp_id_bit = onewire_read_bit(bus);

        /* Check for no devices on 1-wire */
        if ((id_bit == 1) && (cmp_id_bit == 1)) {
            break;
        } else {
            /* All devices coupled have 0 or 1 */
            if (id_bit != cmp_id_bit) {
                search_direction = id_bit;  // Bit write value for search
            } else {
                /* Discrepancy detected */
                if (id_bit_number < state->last_discrepancy) {
                    search_direction = ((state->rom_id[rom_byte_number] & rom_byte_mask) > 0);
                } else {
                    search_direction = (id_bit_number == state->last_discrepancy);
                }

                if (search_direction == 0) {
                    last_zero = id_bit_number;

                    /* Check for Last Family Discrepancy */
                    if (last_zero < 9) {
                        state->last_family_discrepancy = last_zero;
                    }
                }
            }

            /* Set or clear the bit in the ROM byte rom_byte_number with mask rom_byte_mask */
            if (search_direction == 1) {
                state->rom_id[rom_byte_number] |= rom_byte_mask;
            } else {
                state->rom_id[rom_byte_number] &= ~rom_byte_mask;
            }

            /* Serial number search direction write bit */
            onewire_write_bit(bus, search_direction);

            /* Increment the byte counter id_bit_number and shift the mask rom_byte_mask */
            id_bit_number++;
            rom_byte_mask <<= 1;

            /* If the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask */
            if (rom_byte_mask == 0) {
                rom_byte_number++;
                rom_byte_mask = 1;
            }
        }
    } while (rom_byte_number < 8);  // Loop until through all ROM bytes 0-7

    /* If the search was successful then */
    if (!((id_bit_number < 65) || (onewire_crc8(state->rom_id, 8) != 0))) {
        state->last_discrepancy = last_zero;

        /* Check for last device */
        if (state->last_discrepancy == -1) {
            state->last_device_flag = true;
        }

        return true;
    }

    return false;
}
