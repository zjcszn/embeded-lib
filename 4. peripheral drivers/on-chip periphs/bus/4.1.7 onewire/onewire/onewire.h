#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "onewire_config.h"

/**
 * @brief OneWire hardware operations interface
 */
typedef struct {
    void (*set_pin)(void *arg, uint8_t level); /**< Set pin level (0 or 1) */
    uint8_t (*get_pin)(void *arg);             /**< Get pin level */
} onewire_ops_t;

/**
 * @brief OneWire bus handle
 */
typedef struct {
    const onewire_ops_t *ops; /**< Hardware operations */
    void *port_data;          /**< User-defined data passed to ops callbacks */
} onewire_bus_t;

/* OneWire Standard Rom Commands */
#define ONEWIRE_CMD_RSCRATCHPAD   0xBE
#define ONEWIRE_CMD_WSCRATCHPAD   0x4E
#define ONEWIRE_CMD_CPYSCRATCHPAD 0x48
#define ONEWIRE_CMD_RECEEPROM     0xB8
#define ONEWIRE_CMD_RPWRSUPPLY    0xB4
#define ONEWIRE_CMD_SEARCHROM     0xF0
#define ONEWIRE_CMD_READROM       0x33
#define ONEWIRE_CMD_MATCHROM      0x55
#define ONEWIRE_CMD_SKIPROM       0xCC

/**
 * @brief Initialize OneWire bus handle
 *
 * @param bus Pointer to bus handle
 * @param ops Pointer to operations structure (must be persistent)
 * @param port_data Pointer to user data passed to ops
 */
void onewire_init(onewire_bus_t *bus, const onewire_ops_t *ops, void *port_data);

/**
 * @brief Reset bus and check for presence
 *
 * @param bus Pointer to bus handle
 * @return true Device present
 * @return false No device detected
 */
bool onewire_reset(onewire_bus_t *bus);

/**
 * @brief Write a single byte to the bus
 *
 * @param bus Pointer to bus handle
 * @param data Byte to write
 */
void onewire_write_byte(const onewire_bus_t *bus, uint8_t data);

/**
 * @brief Read a single byte from the bus
 *
 * @param bus Pointer to bus handle
 * @return uint8_t Read byte
 */
uint8_t onewire_read_byte(const onewire_bus_t *bus);

/**
 * @brief Issue Skip ROM command (0xCC)
 *
 * @param bus Pointer to bus handle
 */
void onewire_skip_rom(const onewire_bus_t *bus);

/**
 * @brief Issue Match ROM command (0x55) followed by ROM ID
 *
 * @param bus Pointer to bus handle
 * @param rom_id Pointer to 8-byte ROM ID
 */
void onewire_match_rom(const onewire_bus_t *bus, const uint8_t *rom_id);

/* Search ROM state */
typedef struct {
    uint8_t rom_id[8];              /**< Detected ROM ID (Family Code + Serial + CRC) */
    int8_t last_discrepancy;        /**< Last discrepancy position */
    int8_t last_family_discrepancy; /**< Last family discrepancy position */
    bool last_device_flag;          /**< Flag indicating if the last device was found */
} onewire_search_state_t;

/**
 * @brief Calculate CRC8 for 1-Wire data
 *
 * @param data Pointer to data
 * @param len Length of data
 * @return uint8_t Calculated CRC
 */
uint8_t onewire_crc8(const uint8_t *data, uint8_t len);

/**
 * @brief Initialize/Reset search state
 *
 * @param state Pointer to search state structure
 */
void onewire_search_start(onewire_search_state_t *state);

/**
 * @brief Search for the next device on the bus
 *
 * @param bus Pointer to bus handle
 * @param state Pointer to search state structure
 * @return true New device found (ID in state->rom_id)
 * @return false Search finished or error
 */
bool onewire_search_next(onewire_bus_t *bus, onewire_search_state_t *state);

#ifdef __cplusplus
}
#endif

#endif /* __ONEWIRE_H__ */
