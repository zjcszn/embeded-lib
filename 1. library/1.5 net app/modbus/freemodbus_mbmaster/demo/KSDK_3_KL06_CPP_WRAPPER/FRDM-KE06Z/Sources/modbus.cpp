/*
 * MODBUS Library: KL06 KDS/PE/FreeRTOS port
 * Copyright (c)  2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 */

#include <cmath>
#include <cstdlib>

#include "mbmaster.hpp"
#include "freertos.h"
#include "task.h"

using namespace sila;

extern "C" void
AppTask(void *pvParameters)
{
	mbrtumaster master( mbcommon::MB_RTU,  0, 38400, mbcommon::MB_PAR_ODD );
	std::vector<uint16_t> regs;
	for( ;; )
	{
		mbcommon::status status = master.read_holding( 1, 0, regs, 10 );
		if( status == mbcommon::MB_ENOERR )
		{
			regs[0]++;
			master.write_multiple_registers( 1, 0, regs, 5 );
		}
	}
}
