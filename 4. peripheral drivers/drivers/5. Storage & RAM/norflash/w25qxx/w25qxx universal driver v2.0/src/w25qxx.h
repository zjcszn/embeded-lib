#ifndef W25QXX_H
#define W25QXX_H 200

//#include <stdint.h> // debug only

//#define W25QXX_DYNAMIC_OVERWRITE_BUFFER // uncomment to enable. Lowers static RAM usage by 4+kb at an expense of lower performance due to alloc/free calls during over-writing operations.

/* Features

CHIP    | STATUS REG|SFDP|
ID      | 1 | 2 | 3 |    |
--------+---+---+---+----+
W25Q10  |   |   |   |    |
W25Q20  |   |   |   |    |
W25Q40  |   |   |   |    |
W25Q80  |   |   |   |    |
W25Q16  |   |   |   |    |
W25Q32  |   |   |   |    |
W25Q64  | o | o | x |  x |
W25Q128 |   |   |   |    |
W25Q256 | o | o | o | 256|
W25Q512 |   |   |   |    |
--------+---+---+---+----+
Legend: o = available; x = not available;
*/


#define W25QXX_SECTOR_SIZE					4096	// bytes
#define W25QXX_SFDP_BLOCK_SIZE				256
#define W25QXX_SECURITY_BLOCK_SIZE			256
#define W25QXX_WRITE_BLOCK_SIZE				256
#define W25QXX_ERASE_MIN_BLOCK_SIZE			W25QXX_SECTOR_SIZE
#define W25QXX_MEM_DEFAULT_VAL				0xFF
#define W25QXX_ADDR_INVALID_VAL				0xFFFFFFFF
#define W25QXX_UID_LEN						8

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup w25qxx driver
 * @brief    w25qxx driver functions
 * @{
 */

/**
 * @brief w25qxx type enumeration definition
 */
enum W25Qxx_e {
	// NOTE: type = 0x10 + x;
	//W25Q00,
	W25Q10,
	W25Q20,
	W25Q40,
	W25Q80,
	W25Q16,
	W25Q32,
	W25Q64,		// Real W25Q64  returns '0x16'
	W25Q128,
	W25Q256,	// Real W25Q256 returns '0x18'
	W25Q512,
  W25Qxx_max,
  W25Qxx_base = 0x10,
  W25Qxx_mask = 0x0F,
  W25Qxx_invalid_dev_id_val = W25Qxx_mask,
};
_Static_assert((W25Q256 == 8), "ERROR: Drunig calculations, 'W25Q256' will not be equal to real value of '0x18'! Check enumerators integrity");

/**
 * @brief w25qxx functions return codes enumeration definition
 */
typedef enum W25QXX_RET_e {
	W25QXX_RET_SUCCESS,
	W25QXX_RET_ERR_NONE = W25QXX_RET_SUCCESS,
	W25QXX_RET_ERR_IO_FAIL,
	W25QXX_RET_ERR_NULL_PTR,
	W25QXX_RET_ERR_UNINITIALIZED,
	W25QXX_RET_ERR_INVALID_DATA,
	W25QXX_RET_ERR_TIMEOUT,
	W25QXX_RET_ERR_INVALID_MODE,
	W25QXX_RET_ERR_FORBIDDEN,
	W25QXX_RET_ERR_INVALID_STATE,
	W25QXX_RET_ERR_OVERFLOW,
	W25QXX_RET_ERR_NO_MEM,
	W25QXX_RET_ERR_DATA_CORRUPTED,
} W25QXX_RET_t;

/**
 * @brief w25qxx memory block sizes enumeration definition
 */
enum W25QXX_BLOCK_SIZE_e {
	W25QXX_BLOCK_SIZE_4K  =  4,
	W25QXX_BLOCK_SIZE_32K = 32,
	W25QXX_BLOCK_SIZE_64K = 64,
};

/**
 * @brief w25qxx chip memory block security locking states enumeration definition
 */
enum W25QXX_LOCK_STATE_e {
	W25QXX_UNLOCKED,
	W25QXX_LOCKED,
};

/**
 * @brief w25qxx interface enumeration definition
 */
enum W25QXX_INTF_e {
	W25QXX_INTF_SPI = 0x00,																			///< spi/dspi/qspi interface
	W25QXX_INTF_QPI = 0x01,																			///< qpi interface
};

/**
 * @brief w25qxx address mode enumeration definition
 */
enum W25QXX_ADDR_MODE_e {
	W25QXX_ADDR_MODE_3_BYTE = 0x00,
	W25QXX_ADDR_MODE_4_BYTE = 0x01,
};

/**
 * @brief w25qxx qspi read dummy enumeration definition
 */
enum W25QXX_QSPI_READ_DUMMY_e {
	W25QXX_QSPI_READ_DUMMY_2_33MHZ = 0x00,															///< qspi read dummy 2 max 33 MHz
	W25QXX_QSPI_READ_DUMMY_4_55MHZ = 0x01,															///< qspi read dummy 4 max 55 MHz
	W25QXX_QSPI_READ_DUMMY_6_80MHZ = 0x02,															///< qspi read dummy 6 max 80 MHz
	W25QXX_QSPI_READ_DUMMY_8_80MHZ = 0x03,															///< qspi read dummy 8 max 80 MHz
};

/**
 * @brief w25qxx qspi read wrap length enumeration definition
 */
enum W25QXX_QSPI_READ_WRAP_LENGTH_e {
	W25QXX_QSPI_READ_WRAP_LENGTH_8_BYTE  = 0x00,													///< read wrap length 8 byte
	W25QXX_QSPI_READ_WRAP_LENGTH_16_BYTE = 0x01,													///< read wrap length 16 byte
	W25QXX_QSPI_READ_WRAP_LENGTH_32_BYTE = 0x02,													///< read wrap length 32 byte
	W25QXX_QSPI_READ_WRAP_LENGTH_64_BYTE = 0x03,													///< read wrap length 64 byte
};

/**
 * @brief w25qxx security register enumeration definition
 */
enum W25QXX_SECURITY_REG_e {
	W25QXX_SECURITY_REG_1 = 0x1000,																	///< security register 1
	W25QXX_SECURITY_REG_2 = 0x2000,																	///< security register 2
	W25QXX_SECURITY_REG_3 = 0x3000,																	///< security register 3
};

/**
 * @brief w25qxx burst wrap enumeration definition
 */
enum W25QXX_BURST_WRAP_e {
	W25QXX_BURST_WRAP_NONE    = 0x10,																///< no burst wrap
	W25QXX_BURST_WRAP_8_BYTE  = 0x00,																///< 8 byte burst wrap
	W25QXX_BURST_WRAP_16_BYTE = 0x20,																///< 16 byte burst wrap
	W25QXX_BURST_WRAP_32_BYTE = 0x40,																///< 32 byte burst wrap
	W25QXX_BURST_WRAP_64_BYTE = 0x60,																///< 64 byte burst wrap
};

/**
 * @brief w25qxx status enumeration definition
 */
enum W25QXX_STATUS_e {
	W25QXX_STATUS1_STATUS_REGISTER_PROTECT_0				= (1 << 7),								///< status register protect 0
	W25QXX_STATUS1_SECTOR_PROTECT_OR_TOP_BOTTOM_PROTECT		= (1 << 6),								///< sector protect bit or top / bottom protect bit
	W25QXX_STATUS1_TOP_BOTTOM_PROTECT_OR_BLOCK_PROTECT_3	= (1 << 5),								///< top / bottom protect bit or block 3 protect bit
	W25QXX_STATUS1_BLOCK_PROTECT_2							= (1 << 4),								///< block 2 protect bit
	W25QXX_STATUS1_BLOCK_PROTECT_1							= (1 << 3),								///< block 1 protect bit
	W25QXX_STATUS1_BLOCK_PROTECT_0							= (1 << 2),								///< block 0 protect bit
	W25QXX_STATUS1_WRITE_ENABLE_LATCH						= (1 << 1),								///< write enable latch
	W25QXX_STATUS1_ERASE_WRITE_BUSY						= (1 << 0),								///< erase / write in progress

	W25QXX_STATUS2_SUSPEND_STATUS							= (1 << 7),								///< suspend status
	W25QXX_STATUS2_COMPLEMENT_PROTECT						= (1 << 6),								///< complement protect
	W25QXX_STATUS2_SECURITY_REGISTER_3_LOCK_BITS			= (1 << 5),								///< security register 3 lock bits
	W25QXX_STATUS2_SECURITY_REGISTER_2_LOCK_BITS			= (1 << 4),								///< security register 2 lock bits
	W25QXX_STATUS2_SECURITY_REGISTER_1_LOCK_BITS			= (1 << 3),								///< security register 1 lock bits
	W25QXX_STATUS2_QUAD_ENABLE								= (1 << 1),								///< quad enable
	W25QXX_STATUS2_STATUS_REGISTER_PROTECT_1				= (1 << 0),								///< status register protect 1

	W25QXX_STATUS3_HOLD_RESET_FUNCTION						= (1 << 7),								///< HOLD or RESET function
	W25QXX_STATUS3_OUTPUT_DRIVER_STRENGTH_100				= (0 << 5),								///< output driver strength 100%
	W25QXX_STATUS3_OUTPUT_DRIVER_STRENGTH_75				= (1 << 5),								///< output driver strength 75%
	W25QXX_STATUS3_OUTPUT_DRIVER_STRENGTH_50				= (2 << 5),								///< output driver strength 50%
	W25QXX_STATUS3_OUTPUT_DRIVER_STRENGTH_25				= (3 << 5),								///< output driver strength 25%
	W25QXX_STATUS3_WRITE_PROTECT_SELECTION					= (1 << 2),								///< write protect selection
	W25QXX_STATUS3_POWER_UP_ADDRESS_MODE					= (1 << 1),								///< power up address mode
	W25QXX_STATUS3_CURRENT_ADDRESS_MODE						= (1 << 0),								///< current address mode
};

/**
 * @brief w25qxx device definition
 */
typedef struct w25qxx_dev_s {

	union {
		W25QXX_RET_t (*spi_data_write_read)(const void *buf_out, uint32_t len_out, void *data_io, uint32_t data_io_len);
		//W25QXX_RET_t (*qspi_write_read)(uint8_t qspi_lines, const uint8_t *buf_out, uint32_t len_out, uint8_t *data_io, uint32_t data_io_len);
		W25QXX_RET_t (*qspi_write_read_ext)(uint8_t instruction, uint8_t instruction_line, uint32_t address, uint8_t address_line, uint8_t address_len, uint32_t alternate, uint8_t alternate_line, uint8_t alternate_len, uint8_t dummy, uint8_t *in_buf, uint32_t in_len, const uint8_t *out_buf, uint32_t out_len, uint8_t data_line);
	};
	union {
		W25QXX_RET_t (*spi_data_write)(const void *cmd_buf, uint32_t cmd_len, const void *data_out, uint32_t data_out_len);
	};
	#ifndef W25QXX_DYNAMIC_OVERWRITE_BUFFER
	uint8_t *ow_buf; // must point to array, size of 'W25QXX_SECTOR_SIZE'
	#endif

	union {
		struct {
			uint8_t type:4;																			///< chip type
			uint8_t intf:1;																			///< interface type: spi / qspi
			uint8_t dual_quad_spi_enabled:1;														///< dual spi and quad spi enable
			uint8_t initialized:1;																	///< initialized flag
			uint8_t addr_mode:1;																	///< address mode: 3-Byte / 4-Byte
		};
		uint8_t flags;
	};
	uint8_t ext_addr_msb;

	uint8_t param;																					///< param
	uint8_t dummy;																					///< dummy byte to send during read
} w25qxx_dev_t;

/**
 * @brief w25qxx information structure definition
 */
typedef struct w25qxx_info_s {
	const char *chip;																				///< chip name
	const char *manufacturer;																		///< manufacturer name
	const char *interface;																			///< chip interface name
	const char *intf_mode;																			///< chip spi mode

	float supply_volt_min;																			///< chip min supply voltage

	float supply_volt_max;																			///< chip max supply voltage

	char addr_mode[2];																				///< chip address mode
	uint8_t driver_version;																			///< driver version
	uint8_t max_current_ma;																			///< chip max current, mA

	int8_t temperature_min;																			///< chip min operating temperature, °C
	int8_t temperature_max;																			///< chip max operating temperature, °C
	//uint8_t _unused_yet;																			// reserved by alignment
	uint16_t vol_size_kb;																			///< avilable volube size
} w25qxx_info_t;

/**
 * @defgroup w25qxx_link_driver w25qxx link driver function
 * @brief    w25qxx link driver modules
 * @ingroup  w25qxx_driver
 * @{
 */

/**
 * @brief      Returns available volume of memory in bytes
 * @param[in]  *dev points to a w25qxx dev structure
 * @return     operation status code
 */
uint32_t     w25qxx_get_storage_capacity_bytes(const w25qxx_dev_t *const dev);

/**
 * @brief      get chip's information
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *info points to a w25qxx info structure
 * @return     operation status code
 * @example:
	w25qxx_info_t info;
	w25qxx_get_info(&w25qxx_dev1, &info);
	printf(
		"Manufacturer:\t%s\n"
		"Dev_type:\t%s\n"
		"Interface:\t%s\n"
		"Intf_mode:\t%s\n"
		"Addr_mode:\t%.2s\n"
		"Supply volt:\t%.1f..%.1f\n"
		"Supply curr:\t%u ma\n"
		"Temp_range:\t%d°C..%d°C\n"
		"Driver vers:\t%X.%X\n",
		info.manufacturer,
		info.chip,
		info.interface,
		info.intf_mode,
		info.addr_mode,
		info.supply_volt_min, info.supply_volt_max,
		info.max_current_ma,
		info.temperature_max, info.temperature_min,
		(info.driver_version >> 4) & 0xF, info.driver_version & 0xF
	);
 */
W25QXX_RET_t w25qxx_get_info(const w25qxx_dev_t *dev, w25qxx_info_t *info);

/**
 * @brief     enable or disable the dual quad spi
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] enable is a bool value
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_set_dual_quad_spi(w25qxx_dev_t *dev, uint8_t enable) __attribute__((nonnull));

/**
 * @brief      get the dual quad spi status
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] enable points to a bool value buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_dual_quad_spi(const w25qxx_dev_t *dev, uint8_t *enable) __attribute__((nonnull));

/**
 * @brief     set the chip type
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] type is the chip type
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_set_type(w25qxx_dev_t *dev, enum W25Qxx_e type) __attribute__((nonnull));

/**
 * @brief      get the chip type
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *type points to a chip type buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_type(const w25qxx_dev_t *dev, enum W25Qxx_e *type) __attribute__((nonnull));

/**
 * @brief     set the chip interface
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] interface is the chip interface
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_set_interface(w25qxx_dev_t *dev, enum W25QXX_INTF_e interface) __attribute__((nonnull));

/**
 * @brief      get the chip interface
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *interface points to a chip interface buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_interface(const w25qxx_dev_t *dev, enum W25QXX_INTF_e *interface) __attribute__((nonnull));

/**
 * @brief     set the chip address mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] mode is the address mode
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_set_address_mode(w25qxx_dev_t *dev, enum W25QXX_ADDR_MODE_e mode) __attribute__((nonnull));

/**
 * @brief      get the chip address mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *mode points to a address mode buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_address_mode(const w25qxx_dev_t *dev, enum W25QXX_ADDR_MODE_e *mode) __attribute__((nonnull));

/**
 * @brief     initialize the chip
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code:
 *            - 0 success
 *            - 1 spi or qspi initialization failed
 *            - 2 dev is NULL
 *            - 3 linked functions is NULL
 *            - 4 get manufacturer device id failed
 *            - 5 enter qspi failed
 *            - 6 id is invalid
 *            - 7 reset failed
 *            - 8 set address mode failed
 */
W25QXX_RET_t w25qxx_init(w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief     close the chip
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_deinit(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief      read data
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read(w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief      read only in the spi interface
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_spi_only(w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief      read in the fast mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_fast(w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief     write any amount of data (< 65 kB) into unaligned address without any verifications
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] addr is the programming address
 * @param[in] *data points to a data buffer
 * @param[in] len is the data length
 * @return    operation status code
 * @note      len <= 256
 */
W25QXX_RET_t w25qxx_write(w25qxx_dev_t *dev, uint32_t addr, const void *data, uint16_t len) __attribute__((nonnull));

/**
 * @brief     write some data [< 256 B] into aligned address without any verifications
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] addr is the programming address
 * @param[in] *data points to a data buffer
 * @param[in] len is the data length
 * @return    operation status code
 * @note      len <= 256
 */
W25QXX_RET_t w25qxx_write_block(w25qxx_dev_t *dev, uint32_t addr, const void *data, uint16_t len) __attribute__((nonnull));

/**
 * @brief     quad page program with quad input
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] addr is the programming address
 * @param[in] *data points to a data buffer
 * @param[in] len is the data length
 * @return    operation status code
 * @note      len <= 256
 */
W25QXX_RET_t w25qxx_write_block_quad_input(const w25qxx_dev_t *dev, uint32_t addr, const void *data, uint16_t len) __attribute__((nonnull));

/**
 * @brief     write data safely.
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] addr is the write address
 * @param[in] *data points to a data buffer
 * @param[in] len is the data length
 * @details   The driver first reads whole sector that block of data supposed to be writen to into a buffer.
              It verifies that block which supposed to overwritten is empty. If it is not, erases whole sector and re-writes data with overwritten block.
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_overwrite_verify(w25qxx_dev_t *dev, uint32_t addr, const void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief     erase the block
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] addr is the erase address
 * @param[in] size of the block to erase (4k/32k/64k)
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_block_erase(w25qxx_dev_t *const dev, uint32_t addr, enum W25QXX_BLOCK_SIZE_e block_size_kb) __attribute__((nonnull));

/**
 * @brief     erase the chip
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_chip_erase(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief     suspend erase or program
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_erase_write_suspend(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief     resume erase or program
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_erase_write_resume(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief     power down
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_power_down(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief     release power down
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_release_power_down(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief      read the 'manufacturer' and 'device_id' information
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *manufacturer points to a manufacturer buffer
 * @param[out] *device_id points to a device id buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_manufacturer_device_id(const w25qxx_dev_t *dev, uint8_t *manufacturer, uint8_t *device_id) __attribute__((nonnull(1)));

/**
 * @brief      get the manufacturer && device id information with dual io
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *manufacturer points to a manufacturer buffer
 * @param[out] *device_id points to a device id buffer
 * @return     status code
 */
W25QXX_RET_t w25qxx_get_manufacturer_device_id_dual_io(const w25qxx_dev_t *dev, uint8_t *manufacturer, uint8_t *device_id) __attribute__((nonnull(1)));

/**
 * @brief      get the manufacturer && device id information with quad io
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *manufacturer points to a manufacturer buffer
 * @param[out] *device_id points to a device id buffer
 * @return     status code
 */
W25QXX_RET_t w25qxx_get_manufacturer_device_id_quad_io(const w25qxx_dev_t *dev, uint8_t *manufacturer, uint8_t *device_id) __attribute__((nonnull(1)));

/**
 * @brief      read with dual output in the fast mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_fast_dual_output(const w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief      read with quad output in the fast mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_fast_quad_output(const w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief      read with dual io in the fast mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_fast_dual_io(const w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief      read with quad io in the fast mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_fast_quad_io(const w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief      word read with quad io
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_word_quad_io(const w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief      octal word read with quad io
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the read address
 * @param[out] *data points to a data buffer
 * @param[in]  len is the data length
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_octal_word_quad_io(const w25qxx_dev_t *dev, uint32_t addr, void *data, uint32_t len) __attribute__((nonnull));

/**
 * @brief     enable writing
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_writing_set_enabled(const w25qxx_dev_t *dev, enum W25QXX_LOCK_STATE_e new_state) __attribute__((nonnull));

/**
 * @brief     enable volatile sr writing
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_writing_set_enabled_for_volatile_sr(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief      Returns content of 'Status Register X' as 'status'
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *status points to a status buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_status_reg(const w25qxx_dev_t *const dev, uint8_t reg_id, uint8_t *status) __attribute__((nonnull));

/**
 * @brief     Sets content of 'Status Register X'
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] status is the set status
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_set_status_reg(const w25qxx_dev_t *const dev, uint8_t reg_id, uint8_t status) __attribute__((nonnull));

/**
 * @brief      get the jedec id information
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *manufacturer points to a manufacturer buffer
 * @param[out] *device_id points to a device id buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_jedec_id(const w25qxx_dev_t *dev, uint8_t *manufacturer, uint8_t *device_id) __attribute__((nonnull(1)));

/**
 * @brief     set the read parameters
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] dummy is the qspi read dummy
 * @param[in] length is the qspi read wrap length
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_set_read_parameters(w25qxx_dev_t *dev, enum W25QXX_QSPI_READ_DUMMY_e dummy, enum W25QXX_QSPI_READ_WRAP_LENGTH_e length) __attribute__((nonnull));

/**
 * @brief     enter the qspi mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_enter_qspi_mode(w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief     exit the qspi mode
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_exit_qspi_mode(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief     reset the device
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_reset_device(const w25qxx_dev_t *dev) __attribute__((nonnull));

/**
 * @brief      get the unique id
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *id points to a id buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_unique_id(const w25qxx_dev_t *dev, uint8_t id[8]) __attribute__((nonnull));

/**
 * @brief      get the sfdp register (Serial Flash Discoverable Parameters)
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *sfdp points to a sfdp buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_get_sfdp(const w25qxx_dev_t *dev, uint8_t sfdp[256]) __attribute__((nonnull));

/**
 * @brief     erase the security register
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] num is the security register number
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_erase_security_register(const w25qxx_dev_t *dev, enum W25QXX_SECURITY_REG_e num) __attribute__((nonnull));

/**
 * @brief     program the security register
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] num is the security register number
 * @param[in] *data points to a data buffer
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_write_security_register(const w25qxx_dev_t *dev, enum W25QXX_SECURITY_REG_e num, const uint8_t data[256]) __attribute__((nonnull));

/**
 * @brief      read the security register
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  num is the security register number
 * @param[out] *data points to a data buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_read_security_register(const w25qxx_dev_t *dev, enum W25QXX_SECURITY_REG_e num, uint8_t data[256]) __attribute__((nonnull));

/**
 * @brief     lock the individual block
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] addr is the block address
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_block_lock_set_state(w25qxx_dev_t *dev, uint32_t addr, enum W25QXX_LOCK_STATE_e new_state) __attribute__((nonnull));

/**
 * @brief      read the block lock
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  addr is the block address
 * @param[out] *value points to a value buffer
 * @return     operation status code
 */
W25QXX_RET_t w25qxx_block_lock_get_state(w25qxx_dev_t *dev, uint32_t addr, uint8_t *value) __attribute__((nonnull));

/**
 * @brief     lock the whole block
 * @param[in]  *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_block_lock_set_state_global(const w25qxx_dev_t *dev, enum W25QXX_LOCK_STATE_e new_state) __attribute__((nonnull));

/**
 * @brief     set the burst with wrap
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in] wrap is the burst wrap
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_set_burst_with_wrap(const w25qxx_dev_t *dev, enum W25QXX_BURST_WRAP_e wrap) __attribute__((nonnull));

/**
 * @brief     Returns text description of return code
 * @param[in] code as return code
 * @return    pointer to null-terminated const string
 */
const char *w25qxx_ret2str(W25QXX_RET_t code);

/**
 * @brief     Attempts to reset and read device id. On success, assigns correct associated internal value to 'dev->type'
 * @param[in] *dev points to a w25qxx dev structure
 * @return    operation status code
 */
W25QXX_RET_t w25qxx_chip_detect(w25qxx_dev_t *const dev);

/**
 * @brief      Reads chip memory sector by sector and verifies that all bytes have default value (0xFF)
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[out] *non_empty_addr (if chip memory is not empty, returns address of the first non-default value)
 * @return     operation status code:
 * 'W25QXX_RET_SUCCESS' = specified memory region is empty
 * 'W25QXX_RET_ERR_INVALID_STATE' = specified memory region is not empty
 * other values = error code, operation failed.
 */
W25QXX_RET_t w25qxx_chip_check_empty(w25qxx_dev_t *const dev, uint32_t *non_empty_addr) __attribute__((nonnull(1)));

/**
 * @brief      Reads chip memory sector by sector and verifies that all bytes have default value (0xFF)
 * @param[in]  *dev points to a w25qxx dev structure
 * @param[in]  *addr_beg - where to start searching
 * @param[in]  *addr_end - where to end searching (if 0, search till the end of memory on the chip)
 * @param[out] *non_empty_addr (if chip memory is not empty, returns address of the first non-default value)
 * @return     operation status code:
 * 'W25QXX_RET_SUCCESS' = specified memory region is empty
 * 'W25QXX_RET_ERR_INVALID_STATE' = specified memory region is not empty
 * other values = error code, operation failed.
 */
W25QXX_RET_t w25qxx_mem_check_empty(w25qxx_dev_t *const dev, uint32_t addr_beg, const uint32_t addr_end, uint32_t *non_empty_addr) __attribute__((nonnull(1)));


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // W25QXX_H
