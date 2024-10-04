/*
 * MODBUS Library: WIN32
 * Copyright (c) 2008 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: demo-tcp.c,v 1.2 2008-11-02 17:12:07 cwalter Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>
#include <stdlib.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mbm.h"
#include "common/mbportlayer.h"

/* ----------------------- Defines ------------------------------------------*/

#define CLIENT_RETRIES  10
#define CLIENT_HOSTNAME "213.129.231.174"
#define CLIENT_PORT     502

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Static variables ---------------------------------*/

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/

int
main( int argc, char *argv[] )
{
    eMBErrorCode eStatus;
    xMBHandle    xMBMMaster;
    USHORT       usRegCnt = 0;
    int          iNConnects;
    int          iPolls = 2;

    vMBPOtherDLLInit();

    do
    {
        if( MB_ENOERR == ( eStatus = eMBMTCPInit( &xMBMMaster ) ) )
        {
            iNConnects = 5;
            do
            {
                if( MB_ENOERR == eMBMTCPConnect( xMBMMaster, CLIENT_HOSTNAME, CLIENT_PORT ) )
                {
                    _ftprintf( stderr, _T( "connected to TCP client %s:%hu\r\n"), _T( CLIENT_HOSTNAME ), CLIENT_PORT );

                    eStatus = eMBMWriteSingleRegister( xMBMMaster, 1, 0, usRegCnt++ );
                    _ftprintf( stderr, _T( "  write register: %d\r\n"), eStatus );

                    /* Now disconnect. */
                    eStatus = eMBMTCPDisconnect( xMBMMaster );
                    _ftprintf( stderr, _T( "  disconnected: %d\r\n"), eStatus );
                }
                else
                {
                    _ftprintf( stderr, _T( "failed to connect to %s:%hu!\r\n" ), _T( CLIENT_HOSTNAME ), CLIENT_PORT );
                }
            } while( iNConnects-- > 0 );
        }
        else
        {
            MBP_ASSERT( 0 );
        }

        if( MB_ENOERR != ( eStatus = eMBMClose( xMBMMaster ) ) )
        {
            MBP_ASSERT( 0 );
        }
    } while( iPolls-- > 0 );

    vMBPOtherDLLClose();
    return 0;
}
