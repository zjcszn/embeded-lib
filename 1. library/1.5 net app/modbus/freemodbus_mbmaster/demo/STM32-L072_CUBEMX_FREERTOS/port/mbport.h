/* 
 * MODBUS Library: STM32 CubeMX, FreeRTOS port
 * Copyright (c)  2016 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbport.h 1068 2016-08-26 16:05:09Z cwalter $
 */

#ifndef _MB_PORT_H
#define _MB_PORT_H

#ifdef __cplusplus
extern          "C" {
#endif

/* ----------------------- Defines ------------------------------------------*/

#define INLINE                              
#define STATIC                              static

#define PR_BEGIN_EXTERN_C                   extern "C" {
#define	PR_END_EXTERN_C                     }

#define MBP_ASSERT( x )                     do { if( !( x ) ) { vMBPAssert( __FILE__, __LINE__ ); } } while( 0 )

#define MBP_ENTER_CRITICAL_SECTION( )       vMBPEnterCritical( )
#define MBP_EXIT_CRITICAL_SECTION( )        vMBPExitCritical( )
#define MBP_SERIAL_PORT_DETECTS_TIMEOUT			( 1 )
#define MBP_FORCE_SERV2PROTOTYPES           ( 1 )

#ifndef TRUE
#define TRUE                                ( BOOL )1
#endif

#ifndef FALSE
#define FALSE                               ( BOOL )0
#endif

#define MBP_EVENTHDL_INVALID                NULL
#define MBP_TIMERHDL_INVALID                NULL
#define MBP_SERIALHDL_INVALID               NULL
#define MBP_TCPHDL_INVALID                  NULL
#define MBP_TCPHDL_CLIENT_INVALID           NULL
 
#define MBP_TASK_PRIORITY                   ( tskIDLE_PRIORITY + 1 )  

/* ----------------------- Type definitions ---------------------------------*/
typedef void      *xMBPEventHandle;
typedef void      *xMBPTimerHandle;
typedef void      *xMBPSerialHandle;
typedef void      *xMBPTCPHandle;
typedef void      *xMBPTCPClientHandle;

typedef int       BOOL;
typedef unsigned char BYTE;
typedef unsigned char UBYTE;

typedef unsigned char UCHAR;
typedef char      CHAR;

typedef unsigned short USHORT;
typedef short     SHORT;

typedef unsigned long ULONG;
typedef long      LONG;

/* ----------------------- Function prototypes ------------------------------*/
void              vMBPInit( void );
void              vMBPEnterCritical( void );
void              vMBPExitCritical( void );
void              vMBPAssert( const char *file, int line );

#ifdef __cplusplus
}
#endif

#endif
