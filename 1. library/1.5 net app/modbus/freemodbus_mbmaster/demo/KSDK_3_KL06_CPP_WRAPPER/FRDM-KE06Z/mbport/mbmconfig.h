/* 
 * MODBUS Library: KL06 KDS/PE/FreeRTOS port
 * Copyright (c)  2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
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
#define MBM_ASCII_BACKOF_TIME_MS                ( 0 )
#define MBM_CALLBACK_ENABLE_CONTEXT				( 1 )
#ifdef __cplusplus
PR_END_EXTERN_C
#endif

#endif
