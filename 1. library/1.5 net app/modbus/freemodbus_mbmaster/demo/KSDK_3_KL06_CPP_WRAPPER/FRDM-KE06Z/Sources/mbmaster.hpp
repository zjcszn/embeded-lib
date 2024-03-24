/*
 * MODBUS Library: KL06 KDS/PE/FreeRTOS port
 * Copyright (c)  2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 */

#ifndef MBMASTER_H_
#define MBMASTER_H_

#include <cstdint>
#include <vector>
#include <bitset>

#include "mbcommon.hpp"

namespace sila
{

class mbmaster
{
private:
	bool m_port_initialized;

protected:
	void *m_hdl;
	mbmaster( );

public:
	/*! \brief set slave timeout for the the next request
	 *
	 * \return true if new slave timeout was set successfully.
	 */
	mbcommon::status set_slave_timeout( uint16_t ms );

	/*! \brief <em>Read Holding Registers</em> from a slave using the MODBUS
	 *   function code <b>0x03</b>
	 *
	 * \param addr Slave address. Note that a broadcast address is not
	 *   allowed for a function which expects a response.
	 * \param start The first holding register to be read.
	 * \param regs A caller allocated array of size nregs.
	 * \param nregs Number of registers to read.
	 */
	mbcommon::status read_holding( uint8_t addr, uint16_t start, std::vector<uint16_t> &regs , uint16_t nregs );

	/*! \brief <em>Mask Single Register</em> in a slave using the MODBUS function
	 *   code <b>0x16</b>.
	 *
	 *  After a write to a register the content of the register is modified
	 *  as following. New Content = (Current AND and_mask) OR (or_mask AND (NOT and_mask))
	 *
	 * \param addr Slave address. Note that a broadcast address is not
	 *   allowed for a function which expects a response.
	 * \param start The first holding register to be read.
	 * \param usANDMask AND mask
	 * \param usORMask OR mask
	 */
	mbcommon::status mask_write_register( uint8_t addr, uint16_t start, uint16_t and_mask, uint16_t or_mask );

	/*! \brief <em>Write Single Register</em> in a slave using the MODBUS function
	 *   code <b>0x06</b>.
	 *
	 * \param addr Slave address. Note that a broadcast address is not
	 *   allowed for a function which expects a response.
	 * \param address The register address to write.
	 * \param value The value which should be written to the register.
	 */
	mbcommon::status write_single_register( uint8_t addr, uint16_t address, uint16_t value );

	/*! \brief <em>Read Input Registers</em> from a slave using the MODBUS function
	 *   code <b>0x04</b>.
	 *
	 * \param addr Slave address. Not that a broadcast address is not
	 *    allowed.
	 * \param address First register to read.
	 * \param nregs Numer of registers to read.
	 * \param regs A caller allocated array of size nregs.
	 */
	mbcommon::status read_input_register( uint8_t addr, uint16_t address, std::vector<uint16_t> &regs, uint16_t nregs );

	/*! \brief <em>Write Multiple Registers</em> from a slave using the MODBUS
	 *   function code <b>0x10</b>.
	 *
	 * \param xHdl A valid MODBUS master handle.
	 * \param ucSlaveAddress Slave address.
	 * \param usRegStartAddress First register to write to.
	 * \param ubNRegs Number of registers to write.
	 */
	mbcommon::status write_multiple_registers( uint8_t addr, uint16_t address, std::vector<uint16_t> &regs, uint16_t nregs );

	/*! \brief <em>Read/Write Multiple Registers</em> from a slave using the MODBUS
	 *   function code <b>0x17</b>
	 *
	 * \note The pointers arusBufferIn and arusBufferOut can point to the same
	 *   memory location. In this case the old contents is overwritten with
	 *   the returned registers values.
	 *
	 * \param addr Slave address.
	 * \param write_address First register to write on the slave.
	 * \param write_regs A caller allocated array of size nwrite_regs which should be written to the slave.
	 * \param nwrite_regs Number of registers to write.
	 * \param read_address First register to read from the slave.
	 * \param read_regs A caller allocated array of size nread_regs which is filled by registers read from the slave.
	 * \param nread_regs Number of registers to read.

	 *   values returned by the slave should be stored.
	 */
	mbcommon::status read_write_multiple_registers( uint8_t addr,
												uint16_t write_address, const std::vector<uint16_t> &write_regs, uint8_t nwrite_regs,
												uint16_t read_address, std::vector<uint16_t> &read_regs, uint8_t nread_regs );

	/*! \brief <em>Read Discrete Inputs</em> from a slave using the MODBUS
	 *   function code <b>0x02</b>
	 *
	 * The discrete inputs are packed as one input per bit. Statis is 1=ON and 0=OFF.
	 * The LSB of the first data byte contains the input addressed in the query. The
	 * other inputs follow toward the high order of the byte. If the input quantity
	 * is not a multiple of eight the final data byte is padded with zeros.
	 *
	 * \param addr Slave address.
	 * \param start_address Address of first discrete input.
	 * \param inputs An caller allocated array with a size of at least ninputs/8.
	 *   The size must be rounded up to next integer.
	 * \param ninputs Number of discrete inputs to read.
	 */
	mbcommon::status read_discrete_inputs( uint8_t addr, uint16_t start_address, std::vector<uint8_t> &inputs, uint16_t ninputs );


	/*! \brief <em>Read Coils</em> from a slave using the MODBUS
	 *   function code <b>0x01</b>
	 *
	 * The coils are packed as one coil per bit. Statis is 1=ON and 0=OFF. The LSB
	 * of the first data byte contains the coil addressed in the query. The other
	 * coils follow toward the high order of the byte. If the input quantity
	 * is not a multiple of eight the final data byte is padded with zeros.
	 *
	 * \param addr Slave address.
	 * \param start_address Address of first coil.
	 * \param coils An array with a size of at least usNCoils/8 bytes.
	 *   The size must be rounded up to next integer.
	 * \param ncoils Number of coils to read.
	 */
	mbcommon::status read_coils( uint8_t addr, uint16_t start_address, std::vector<uint8_t> &coils, uint16_t ncoils );

	/*! \brief <em>Write Coil</em> on slave using the MODBUS
	 *   function code <b>0x05</b>
	 *
	 * \param addr Slave address.
	 * \param address Address of coil.
	 * \param on Either \c TRUE or \c FALSE.
	 */
	mbcommon::status write_single_coil( uint8_t addr, uint16_t address, bool on );

	/*! \brief <em>Write Coils</em> from a slave using the MODBUS
	 *   function code <b>0x0F</b>
	 *
	 * The coils are packed as one coil per bit. Statis is 1=ON and 0=OFF. The LSB
	 * of the first data byte contains the coil addressed in the query. The other
	 * coils follow toward the high order of the byte.
	 *
	 * \param addr Slave address.
	 * \param address Address of first coil.
	 * \param coils_in An array with a size of at least usNCoils/8 bytes.
	 *   The size must be rounded up to next integer.
	 * \param ncoils Number of coils to read.
	 */
	mbcommon::status write_coils( uint8_t addr, uint16_t address, const std::vector<uint8_t> &coils_in, uint16_t ncoils );

	/*! \brief A function for sending raw MODBUS PDUs to a MODBUS slave. This
	 *   function can be used if you want to transport custom data over a
	 *   MODBUS connection or for briding other protocols. Please note that
	 *   of course most standard slaves won't know how to deal with these
	 *   custom data payload and therefore its use should be limited.
	 *
	 * \param addr Slave address.
	 * \param function_code MODBUS function code used for transport. Must
	 *   be between 1 and 127. Recommend values are between 65-72 and 100-110.
	 * \param in MODBUS request which will be sent to the slave.
	 * \param in_length Length of MODBUS request. Maximum is 252 bytes.
	 * \param out Holds the MODBUS response sent by the slave.
	 * \param pubPayloadOutLength Actual number of bytes returned by the
	 *   slave.
	 */
	mbcommon::status write_raw_pdu( uint8_t addr, uint8_t function_code,
			 const std::vector<uint8_t > &in, uint8_t in_length,
			 std::vector<uint8_t> &out, uint8_t &out_length );

	/*! \brief <em>Report slave ID</em> from a MODBUS slave with function
	 *    code <b>0x11</b>
	 *
	 * This function issues the report slave id command to a MODBUS slave.
	 * The response is then stored in the buffer provided by arubBufferOut
	 * where the caller has to allocate sufficient space. The maximum amount
	 * of space available is determined by ubBufferMax. If there is not enough
	 * space available the function returns eMBErrorCode::MB_ENORES.
	 *
	 * There is no chance for the MODBUS stack to tell something about the
	 * content since the content is vendor specific.
	 *
	 * \note There are some non vendor specific fields but most vendors do
	 * not implement them correctly. Therefore we have completely avoided interpreting
	 * the data.
	 *
	 * \param addr Slave address.
	 * \param out Buffer which can hold ubBufferMax bytes.
	 * \param out_length If the call succeeds contains the number of bytes
	 *   written to the buffer.
	 */
	mbcommon::status report_slave_id( uint8_t addr, std::vector<uint8_t> &out, uint8_t &out_length );
};

class mbrtumaster: public mbmaster
{
public:
	/*!
	 * \brief Create a new instances for a serial MODBUS master instance using
	 *   either ASCII or RTU transmission mode.
	 *
	 * The stopbits shall be decided internally by databits and operating mode. It is recommended
	 * to use the function without the stopbits argument.
	 *
	 * \param mode The serial transmission mode to use. Either MB_RTU or MB_ASCII.
	 * \param port The serial port to use. The meaning of this value depends on
	 *   the porting layer.
	 * \param baudrate The baudrate. For example 38400.
	 * \param parity The parity to use.
	 * \param stopbits Number of stopbits to use.
	 */
	mbrtumaster( mbcommon::mode mode, uint8_t port, uint32_t baudrate, mbcommon::parity parity, uint8_t stopbits );
	mbrtumaster( mbcommon::mode mode, uint8_t port, uint32_t baudrate, mbcommon::parity parity );
};

}

#endif /* MBMASTER_H_ */
