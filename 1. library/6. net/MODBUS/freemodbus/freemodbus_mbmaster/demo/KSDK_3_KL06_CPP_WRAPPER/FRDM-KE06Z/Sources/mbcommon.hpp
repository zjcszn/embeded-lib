/*
 * MODBUS Library: KL06 KDS/PE/FreeRTOS port
 * Copyright (c)  2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 */

#ifndef MBCOMMON_H_
#define MBCOMMON_H_

#include <bitset>

namespace sila
{

class mbcommon
{
public:
	static const unsigned MB_MAX_PDU_LENGTH = 253;
	typedef std::bitset<2000> bitfield;

	enum status
	{
	    MB_ENOERR = 0,              /*!< No error. */
	    MB_ENOREG = 1,              /*!< Illegal register address. */
	    MB_EINVAL = 2,              /*!< Illegal argument. */
	    MB_EPORTERR = 3,            /*!< Porting layer error. */
	    MB_ENORES = 4,              /*!< Insufficient resources. */
	    MB_EIO = 5,                 /*!< I/O error. */
	    MB_EILLSTATE = 6,           /*!< Protocol stack in illegal state. */
	    MB_EAGAIN = 7,              /*!< Retry I/O operation. */
	    MB_ETIMEDOUT = 8,           /*!< Timeout error occurred. */
	    MB_EX_ILLEGAL_FUNCTION = 10,        /*!< Illegal function exception. */
	    MB_EX_ILLEGAL_DATA_ADDRESS = 11,    /*!< Illegal data address. */
	    MB_EX_ILLEGAL_DATA_VALUE = 12,      /*!< Illegal data value. */
	    MB_EX_SLAVE_DEVICE_FAILURE = 13,    /*!< Slave device failure. */
	    MB_EX_ACKNOWLEDGE = 14,     /*!< Slave acknowledge. */
	    MB_EX_SLAVE_BUSY = 15,      /*!< Slave device busy. */
	    MB_EX_MEMORY_PARITY_ERROR = 16,     /*!< Memory parity error. */
	    MB_EX_GATEWAY_PATH_UNAVAILABLE = 17,        /*!< Gateway path unavailable. */
	    MB_EX_GATEWAY_TARGET_FAILED = 18    /*!< Gateway target device failed to respond. */
	};

	enum exceptions
	{
	    MB_PDU_EX_NONE = 0x00,
	    MB_PDU_EX_ILLEGAL_FUNCTION = 0x01,
	    MB_PDU_EX_ILLEGAL_DATA_ADDRESS = 0x02,
	    MB_PDU_EX_ILLEGAL_DATA_VALUE = 0x03,
	    MB_PDU_EX_SLAVE_DEVICE_FAILURE = 0x04,
	    MB_PDU_EX_ACKNOWLEDGE = 0x05,
	    MB_PDU_EX_SLAVE_BUSY = 0x06,
	    MB_PDU_EX_NOT_ACKNOWLEDGE = 0x07,
	    MB_PDU_EX_MEMORY_PARITY_ERROR = 0x08,
	    MB_PDU_EX_GATEWAY_PATH_UNAVAILABLE = 0x0A,
	    MB_PDU_EX_GATEWAY_TARGET_FAILED = 0x0B
	};

	enum mode
	{
		MB_RTU,                     /*!< RTU transmission mode. */
		MB_ASCII                    /*!< ASCII transmission mode. */
	};

	enum parity
	{
	    MB_PAR_ODD,                 /*!< ODD parity. */
	    MB_PAR_EVEN,                /*!< Even parity. */
	    MB_PAR_NONE                 /*!< No parity. */
	};

};

}



#endif /* MBCOMMON_H_ */
