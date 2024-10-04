/* 
 * MODBUS Library: STM32 CubeMX, FreeRTOS port
 * Copyright (c)  2016 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbsconfig.h 1068 2016-08-26 16:05:09Z cwalter $
 */


#ifndef _MBMCONFIG_H
#define _MBMCONFIG_H

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

/* ----------------------- Defines ------------------------------------------*/
#define MBM_SERIAL_API_VERSION                  ( 2 )
#define MBM_ASCII_ENABLED                       ( 1 )
#define MBM_RTU_ENABLED                         ( 1 )
#define MBM_TCP_ENABLED                         ( 0 )
#define MBM_ASCII_BACKOF_TIME_MS								( 0 )

/* ----------------------- Functions  ---------------------------------------*/

#endif
