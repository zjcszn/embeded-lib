/*
 * MODBUS Library: KL06 KDS/PE/FreeRTOS port
 * Copyright (c)  2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 */

#include <exception>
#include <stdexcept>
#include <vector>
#include <iterator>

#include "mbport.h"
#include "mbm.h"
#include "mbmaster.hpp"

using namespace sila;

mbmaster::mbmaster( )
{
	if( !m_port_initialized )
	{
		vMBPInit(  );
		m_port_initialized = true;
	}
}

mbcommon::status mbmaster::set_slave_timeout( uint16_t ms )
{
	eMBErrorCode eStatus = eMBMSetSlaveTimeout( m_hdl, ms );
	return static_cast< mbcommon::status >( eStatus );
}

mbcommon::status mbmaster::read_holding( uint8_t addr, uint16_t start, std::vector<uint16_t> &regs , uint16_t nregs )
{
	if( regs.size( ) < nregs )
	{
		regs.resize( nregs );
	}
	eMBErrorCode eStatus = eMBMReadHoldingRegisters( m_hdl, addr, start, nregs, regs.data( ) );
	return static_cast< mbcommon::status >( eStatus );
}

mbcommon::status mbmaster::mask_write_register( uint8_t addr, uint16_t start, uint16_t and_mask, uint16_t or_mask )
{
	eMBErrorCode eStatus = eMBMMaskWriteRegister( m_hdl, addr, start, and_mask, or_mask );
	return static_cast< mbcommon::status >( eStatus );
}

mbcommon::status mbmaster::write_single_register( uint8_t addr, uint16_t address, uint16_t value )
{
	eMBErrorCode eStatus = eMBMWriteSingleRegister( m_hdl, addr, address, value );
	return static_cast< mbcommon::status >( eStatus );
}

mbcommon::status mbmaster::read_input_register( uint8_t addr, uint16_t address, std::vector<uint16_t> &regs, uint16_t nregs )
{
	if( regs.size( ) < nregs )
	{
		regs.resize( nregs );
	}
	eMBErrorCode eStatus = eMBMReadInputRegisters( m_hdl, addr, address, nregs, regs.data( ) );
	return static_cast< mbcommon::status >( eStatus );
}

mbcommon::status mbmaster::write_multiple_registers( uint8_t addr, uint16_t address, std::vector<uint16_t> &regs, uint16_t nregs )
{
	if( regs.size( ) >= nregs )
	{
		eMBErrorCode eStatus = eMBMWriteMultipleRegisters( m_hdl, addr, address, nregs, regs.data( ) );
		return static_cast< mbcommon::status >( eStatus );
	}
	else
	{
		return mbcommon::MB_EINVAL;
	}
}

mbcommon::status mbmaster::read_write_multiple_registers( uint8_t addr,
											uint16_t write_address, const std::vector<uint16_t> &write_regs, uint8_t nwrite_regs,
											uint16_t read_address, std::vector<uint16_t> &read_regs, uint8_t nread_regs )
{
	if( write_regs.size( ) >= nwrite_regs )
	{
		if( read_regs.size( ) < nread_regs )
		{
			read_regs.resize( nread_regs );
		}

		eMBErrorCode eStatus = eMBMReadWriteMultipleRegisters( m_hdl, addr, write_address, nwrite_regs, write_regs.data(),
				read_address, nread_regs, read_regs.data( ) );
		return static_cast< mbcommon::status >( eStatus );
	}
	else
	{
		return mbcommon::MB_EINVAL;
	}
}

mbcommon::status mbmaster::read_discrete_inputs( uint8_t addr, uint16_t start_address, std::vector<uint8_t> &inputs, uint16_t ninputs )
{
	size_t required = ( ninputs + 7 ) / 8;
	if( inputs.size( ) < required )
	{
		inputs.resize( required );
	}
	eMBErrorCode eStatus = eMBMReadDiscreteInputs( m_hdl, addr, start_address, ninputs, inputs.data( ) );
	return static_cast< mbcommon::status >( eStatus );
}

mbcommon::status mbmaster::read_coils( uint8_t addr, uint16_t start_address, std::vector<uint8_t> &coils, uint16_t ncoils )
{
	size_t required = ( ncoils + 7 ) / 8;
	if( coils.size( ) < required )
	{
		coils.resize( required );
	}
	eMBErrorCode eStatus = eMBMReadCoils( m_hdl, addr, start_address, ncoils, coils.data( ) );
	return static_cast< mbcommon::status >( eStatus );
}

mbcommon::status mbmaster::write_single_coil( uint8_t addr, uint16_t address, bool on )
{
	eMBErrorCode eStatus = eMBMWriteSingleCoil( m_hdl, addr, address, on );
	return static_cast< mbcommon::status >( eStatus );
}

mbcommon::status mbmaster::write_coils( uint8_t addr, uint16_t address, const std::vector<uint8_t> &coils_in, uint16_t ncoils )
{
	size_t required = ( ncoils + 7 ) / 8;
	if( coils_in.size( ) >= required )
	{
		eMBErrorCode eStatus = eMBMWriteCoils( m_hdl, addr, address, ncoils, coils_in.data( ) );
		return static_cast< mbcommon::status >( eStatus );
	}
	else
	{
		return mbcommon::MB_EINVAL;
	}
}

mbcommon::status mbmaster::write_raw_pdu( uint8_t addr, uint8_t function_code,
		 const std::vector<uint8_t > &in, uint8_t in_length,
		 std::vector<uint8_t> &out, uint8_t &out_length )
{
	if( in.size( ) >= in_length )
	{
		if( out.size( ) < mbcommon::MB_MAX_PDU_LENGTH )
		{
			out.reserve( mbcommon::MB_MAX_PDU_LENGTH  );
		}
		eMBErrorCode eStatus = eMBMReadWriteRAWPDU( m_hdl, addr, function_code, in.data(), in_length, out.data(), mbcommon::MB_MAX_PDU_LENGTH, &out_length );
		return static_cast< mbcommon::status >( eStatus );
	}
	else
	{
		return mbcommon::MB_EINVAL;
	}
}

mbcommon::status mbmaster::report_slave_id( uint8_t addr, std::vector<uint8_t> &out, uint8_t &out_length )
{
	if( out.size( ) < mbcommon::MB_MAX_PDU_LENGTH )
	{
		out.reserve( mbcommon::MB_MAX_PDU_LENGTH  );
	}
	eMBErrorCode eStatus = eMBMReportSlaveID( m_hdl, addr, out.data( ), mbcommon::MB_MAX_PDU_LENGTH, &out_length );
	return static_cast< mbcommon::status >( eStatus );
}

mbrtumaster::mbrtumaster( mbcommon::mode mode, uint8_t port, uint32_t baudrate, mbcommon::parity parity, uint8_t stopbits )
{
	eMBErrorCode eStatus = eMBMSerialInitExt( &m_hdl, (eMBSerialMode)mode, port, baudrate, (eMBSerialParity)parity, stopbits );
	if( MB_ENOERR != eStatus )
	{
		throw std::runtime_error( "MODBUS RTU init failed" );
	}
}

mbrtumaster::mbrtumaster( mbcommon::mode mode, uint8_t port, uint32_t baudrate, mbcommon::parity parity ) :
		mbrtumaster(mode, port, baudrate, parity, parity == mbcommon::MB_PAR_NONE ? 2 : 1 )
{

}
