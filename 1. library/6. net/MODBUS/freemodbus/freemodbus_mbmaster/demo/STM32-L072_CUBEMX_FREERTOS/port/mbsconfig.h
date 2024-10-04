/* 
 * MODBUS Library: STM32 CubeMX, FreeRTOS port
 * Copyright (c)  2016 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbsconfig.h 1068 2016-08-26 16:05:09Z cwalter $
 */

#ifndef _MBSCONFIG_H
#define _MBSCONFIG_H

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

/* ----------------------- Defines ------------------------------------------*/
#define MBS_SERIAL_API_VERSION                  ( 2 )
#define MBS_ASCII_ENABLED                       ( 1 )
#define MBS_RTU_ENABLED                         ( 1 )
#define MBS_TCP_ENABLED                         ( 0 )
#define MBS_ASCII_BACKOF_TIME_MS								( 0 )

#ifdef __cplusplus
PR_END_EXTERN_C
#endif

#endif
