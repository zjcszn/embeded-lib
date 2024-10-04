/* 
 * MODBUS Library: THREADX port
 * Copyright (c) 2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 */

#ifndef _MB_PORT_H
#define _MB_PORT_H

#ifdef __cplusplus
extern          "C"
{
#endif

/* ----------------------- Defines ------------------------------------------*/
#define MBP_SERIAL_PORT_DETECTS_TIMEOUT     ( 1 )
#define MBP_FORCE_SERV2PROTOTYPES           ( 2 )

#define INLINE                              inline
#define STATIC                              static

#define PR_BEGIN_EXTERN_C                   extern "C" {
#define	PR_END_EXTERN_C                     }

#define MBP_ASSERT( x )                     \
    ( ( x ) ? ( void )0 : vMBPAssert( ) )

#define MBP_ENTER_CRITICAL_SECTION( )       vMBPEnterCritical( )
#define MBP_EXIT_CRITICAL_SECTION( )        vMBPExitCritical( )

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

/* ----------------------- Function prototypes ------------------------------*/


/* ----------------------- Type definitions ---------------------------------*/
typedef void      *xMBPEventHandle;
typedef void      *xMBPTimerHandle;
typedef void      *xMBPSerialHandle;
typedef void      *xMBPTCPHandle;
typedef void      *xMBPTCPClientHandle;

typedef char      BOOL;

typedef char      BYTE;
typedef unsigned char UBYTE;

typedef unsigned char UCHAR;
typedef char      CHAR;

typedef unsigned short USHORT;
typedef short     SHORT;

typedef unsigned long ULONG;
typedef long      LONG;

/* ----------------------- Function prototypes ------------------------------*/
void              vMBPEnterCritical( void );
void              vMBPExitCritical( void );
void              vMBPAssert( void );
void              vMBPInit( void );
void              prvMBPTimerTickMS( void );
void              prvMBPTimerPoll( void );
void              prvMBPSerialTimeoutDetect( void );
void              prvMBPSerialReceiveChar( UBYTE ubIdx, UBYTE ubChar );
void              prvMBPSerialCanTransmit( UBYTE ubIdx );
void              prvMBPSerialPoll( void );

#ifdef __cplusplus
}
#endif

#endif
