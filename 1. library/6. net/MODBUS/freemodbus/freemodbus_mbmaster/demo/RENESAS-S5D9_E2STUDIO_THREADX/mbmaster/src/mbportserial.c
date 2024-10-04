/* 
 * MODBUS Library: THREADX port
 * Copyright (c) 2019 Christian Walter <wolti@sil.at>
 * All rights reserved.
 *
 * $Id: mbportserial.c,v 1.1 2008-04-06 07:47:26 cwalter Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"
#include <mbportssp.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbportlayer.h"
#include "common/mbframe.h"
#include "common/mbutils.h"

/* ----------------------- Defines ------------------------------------------*/
#define IDX_INVALID				( 255 )
#define UART_BAUDRATE_MIN		( 300 )
#define UART_BAUDRATE_MAX		( 115200 )
#define TIMEOUT_INVALID         ( 255 )

#define HDL_RESET( x ) do { \
	( x )->ubIdx = IDX_INVALID; \
	( x )->pbMBPTransmitterEmptyFN = NULL; \
	( x )->pvMBPReceiveFN = NULL; \
	( x )->xMBMHdl = MB_HDL_INVALID; \
	( x )->pbMBPTimerExpiredFN = NULL; \
	( x )->eSerialMode = MB_RTU; \
	memset( ( x )->rxBuffers, 0, sizeof( ( x )->rxBuffers ) ); \
	( x )->ubTimeoutCnt = TIMEOUT_INVALID; \
	( x )->ubTimeoutReloadCnt = TIMEOUT_INVALID; \
	( x )->ubCurrentRcvBuffer = 0; \
	( x )->bHasData = 0; \
	( x )->bHasTimeout = 0; \
	memset( &( x )->txBuffer, 0, sizeof( ( x )->txBuffer ) ); \
	( x )->bCanTx = 0; \
} while( 0 );

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE           arubBuffer[32];
    UBYTE           arubBufferPos;
} xSerialBuf;

typedef struct
{
    UBYTE           ubIdx;
    pbMBPSerialTransmitterEmptyAPIV2CB pbMBPTransmitterEmptyFN;
    pvMBPSerialReceiverAPIV2CB pvMBPReceiveFN;
    xMBHandle       xMBMHdl;
    pbMBPTimerExpiredCB pbMBPTimerExpiredFN;
    eMBSerialMode   eSerialMode;

    /* RX needs to be double buffered */
    xSerialBuf      rxBuffers[2];
    UBYTE           ubTimeoutCnt;
    UBYTE           ubTimeoutReloadCnt;
    UBYTE           ubCurrentRcvBuffer;
    BOOL            bHasData;
    BOOL            bHasTimeout;

    /* TX */
    xSerialBuf      txBuffer;
    BOOL            bCanTx;
} xSerialHandle;

typedef struct
{
    const uart_instance_t *xUART;
} xSerialHWHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC xSerialHandle xSerialHdls[1];
STATIC xSerialHWHandle xSerialHWHdls[] = { {&g_uart0} };

STATIC BOOL     bIsInitalized = FALSE;

/* ----------------------- Static functions ---------------------------------*/
STATIC void     vMBPSerialSwitchBuffer( xSerialHandle * hdl, BOOL hasTimeout );

/* ----------------------- Start implementation -----------------------------*/
#include "../synergy/ssp/src/driver/r_sci_uart/hw/hw_sci_uart_private.h"
static void     r_sci_uart_config_change( const uart_instance_t * const p_uart, uart_data_bits_t data_bits, uart_parity_t parity, uart_stop_bits_t stop_bits );

static void
r_sci_uart_config_change( const uart_instance_t * const p_uart, uart_data_bits_t data_bits, uart_parity_t parity, uart_stop_bits_t stop_bits )
{
    sci_uart_instance_ctrl_t *const p_ctrl = ( sci_uart_instance_ctrl_t * ) p_uart->p_ctrl;
    R_SCI0_Type    *p_sci_reg = ( R_SCI0_Type * ) p_ctrl->p_reg;

    /** Configure parity. */
    HW_SCI_ParitySet( p_sci_reg, parity );

    /** Configure data size. */
    if( UART_DATA_BITS_7 == data_bits )
    {
        HW_SCI_DataBits7bitsSelect( p_sci_reg );
    }
    else if( UART_DATA_BITS_8 == data_bits )
    {
        /* Set the SMR.CHR bit & SCMR.CHR1 bit as selected (Character Length)
         *  Character Length
         *  (CHR1,CHR)
         *  (1, 0) Transmit/receive in 8-bit data length
         */
        p_sci_reg->SMR_b.CHR = 0U;
        p_sci_reg->SCMR_b.CHR1 = 1U;
    }
    else if( UART_DATA_BITS_9 == data_bits )
    {
        HW_SCI_DataBits9bitsSelect( p_sci_reg );
    }

    /** Configure stop bits. */
    HW_SCI_StopBitsSelect( p_sci_reg, stop_bits );
}

eMBErrorCode
eMBPSerialInit( xMBPSerialHandle * pxSerialHdl, UCHAR ucPort, ULONG ulBaudRate,
                UCHAR ucDataBits, eMBSerialParity eParity, UCHAR ucStopBits, xMBHandle xMBMHdl, pbMBPTimerExpiredCB pbFrameTimeoutFN, eMBSerialMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    UBYTE           ubIdx;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( !bIsInitalized )
    {
        for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
        {
            HDL_RESET( &xSerialHdls[ubIdx] );
        }
    }

    if( NULL == pxSerialHdl )
    {
        eStatus = MB_EINVAL;
    }
    else if( ( ucPort < MB_UTILS_NARRSIZE( xSerialHdls ) ) && ( IDX_INVALID == xSerialHdls[ucPort].ubIdx ) )
    {
        if( MB_HDL_INVALID != xMBMHdl )
        {

            const uart_instance_t *xUART = xSerialHWHdls[ucPort].xUART;
            uart_cfg_t      cfg;
            memcpy( &cfg, xUART->p_cfg, sizeof( uart_cfg_t ) );
            uart_data_bits_t data_bits;
            uart_parity_t   parity;
            uart_stop_bits_t stop_bits;
            switch ( eParity )
            {
            case MB_PAR_ODD:
                parity = UART_PARITY_ODD;

                break;
            case MB_PAR_EVEN:
                parity = UART_PARITY_EVEN;
                break;
            default:
                parity = UART_PARITY_OFF;
                break;
            }
            switch ( ucStopBits )
            {
            default:
            case 1:
                stop_bits = UART_STOP_BITS_1;
                break;
            case 2:
                stop_bits = UART_STOP_BITS_2;
                break;
            }
            switch ( ucDataBits )
            {
            default:
            case 8:
                data_bits = UART_DATA_BITS_8;
                break;
            case 7:
                data_bits = UART_DATA_BITS_7;
                break;
            }

            cfg.parity = parity;
            cfg.stop_bits = stop_bits;
            cfg.data_bits = data_bits;
            if( SSP_SUCCESS != xUART->p_api->open( xUART->p_ctrl, &cfg ) )
            {
                eStatus = MB_EIO;
            }
            else if( SSP_SUCCESS != xUART->p_api->baudSet( xUART->p_ctrl, ulBaudRate ) )
            {
                eStatus = MB_EIO;
            }
            else
            {
                r_sci_uart_config_change( xUART, data_bits, parity, stop_bits );
                xSerialHdls[ucPort].ubIdx = ucPort;
                xSerialHdls[ucPort].xMBMHdl = xMBMHdl;
                xSerialHdls[ucPort].pbMBPTimerExpiredFN = pbFrameTimeoutFN;
                xSerialHdls[ucPort].eSerialMode = eMode;
                xSerialHdls[ucPort].ubTimeoutReloadCnt = ( UBYTE ) ( 38500UL / ulBaudRate );
                if( xSerialHdls[ucPort].ubTimeoutReloadCnt < 2 )
                {
                    xSerialHdls[ucPort].ubTimeoutReloadCnt = 2;
                }
                xSerialHdls[ucPort].ubTimeoutCnt = TIMEOUT_INVALID;
                *pxSerialHdl = &xSerialHdls[ucPort];
                eStatus = MB_ENOERR;
            }
        }
        else
        {
            eStatus = MB_EINVAL;
        }
    }
    else
    {
        eStatus = MB_ENORES;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBPSerialClose( xMBPSerialHandle xSerialHdl )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        eStatus = MB_ENOERR;
        xSerialHWHdls[pxSerialIntHdl->ubIdx].xUART->p_api->close( xSerialHWHdls[pxSerialIntHdl->ubIdx].xUART->p_ctrl );
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBPSerialTxEnable( xMBPSerialHandle xSerialHdl, pbMBPSerialTransmitterEmptyCB pbMBPTransmitterEmptyFN )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        eStatus = MB_ENOERR;
        if( NULL != pbMBPTransmitterEmptyFN )
        {
            pxSerialIntHdl->pbMBPTransmitterEmptyFN = pbMBPTransmitterEmptyFN;
        }
        else
        {
            pxSerialIntHdl->pbMBPTransmitterEmptyFN = NULL;
        }
        pxSerialIntHdl->bCanTx = TRUE;
        UINT            status = tx_semaphore_put( &g_mbport_sem );
        MBP_ASSERT( TX_SUCCESS == status );
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBPSerialRxEnable( xMBPSerialHandle xSerialHdl, pvMBPSerialReceiverCB pvMBPReceiveFN )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        eStatus = MB_ENOERR;
        if( NULL != pvMBPReceiveFN )
        {
            MBP_ASSERT( NULL == pxSerialIntHdl->pvMBPReceiveFN );
            pxSerialIntHdl->pvMBPReceiveFN = pvMBPReceiveFN;
        }
        else
        {
            pxSerialIntHdl->pvMBPReceiveFN = NULL;
        }
        UINT            status = tx_semaphore_put( &g_mbport_sem );
        MBP_ASSERT( TX_SUCCESS == status );
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

STATIC xSerialBuf *
vMBPSerialGetReadBuffer( xSerialHandle * hdl )
{
    return 1 == hdl->ubCurrentRcvBuffer ? &hdl->rxBuffers[0] : &hdl->rxBuffers[1];
}

STATIC void
vMBPSerialSwitchBuffer( xSerialHandle * hdl, BOOL hasTimeout )
{
    /* set flags depending if the buffer has received some data and/or if a timeout has occurred */
    if( hdl->rxBuffers[hdl->ubCurrentRcvBuffer].arubBufferPos )
    {
        hdl->bHasData = TRUE;
    }
    if( hasTimeout )
    {
        hdl->ubTimeoutCnt = TIMEOUT_INVALID;
        hdl->bHasTimeout = TRUE;
    }

    /* switch the buffer such that the polling thread can read it and pass data to the stack
     * and the ISR callback can fill another buffer
     */
    hdl->ubCurrentRcvBuffer = hdl->ubCurrentRcvBuffer == 1 ? 0 : 1;
    hdl->rxBuffers[hdl->ubCurrentRcvBuffer].arubBufferPos = 0;

    /* signal polling thread that there is new data */
    UINT            status = tx_semaphore_put( &g_mbport_sem );
    MBP_ASSERT( TX_SUCCESS == status );
}

void
prvMBPSerialTimeoutDetect( void )
{
    for( UBYTE ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
    {
        if( ( IDX_INVALID != xSerialHdls[ubIdx].ubIdx ) && ( TIMEOUT_INVALID != xSerialHdls[ubIdx].ubTimeoutCnt ) )
        {
            xSerialHdls[ubIdx].ubTimeoutCnt--;
            if( 0 == xSerialHdls[ubIdx].ubTimeoutCnt )
            {
                g_ioport.p_api->pinWrite( IOPORT_PORT_06_PIN_04, IOPORT_LEVEL_HIGH );
                vMBPSerialSwitchBuffer( &xSerialHdls[ubIdx], TRUE );
                g_ioport.p_api->pinWrite( IOPORT_PORT_06_PIN_04, IOPORT_LEVEL_LOW );
            }
        }
    }
}

void
prvMBPSerialReceiveChar( UBYTE ubIdx, UBYTE ubChar )
{
    xSerialHandle  *hdl;
    if( ( ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ) ) && ( IDX_INVALID != xSerialHdls[ubIdx].ubIdx ) )
    {
        g_ioport.p_api->pinWrite( IOPORT_PORT_06_PIN_03, IOPORT_LEVEL_HIGH );
        hdl = &xSerialHdls[ubIdx];
        /* implement timeouts */
        hdl->ubTimeoutCnt = hdl->ubTimeoutReloadCnt;

        /* add data to the buffer */
        xSerialBuf     *buf = &hdl->rxBuffers[hdl->ubCurrentRcvBuffer];
        buf->arubBuffer[buf->arubBufferPos++] = ubChar;
        /* the buffer has filled up - let the stack process some data */
        if( buf->arubBufferPos >= sizeof( buf->arubBuffer ) )
        {
            vMBPSerialSwitchBuffer( hdl, FALSE );

        }
        g_ioport.p_api->pinWrite( IOPORT_PORT_06_PIN_03, IOPORT_LEVEL_LOW );
    }
}

void
prvMBPSerialCanTransmit( UBYTE ubIdx )
{
    if( ( ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ) ) && ( IDX_INVALID != xSerialHdls[ubIdx].ubIdx ) )
    {
        xSerialHdls[ubIdx].bCanTx = TRUE;
        UINT            status = tx_semaphore_put( &g_mbport_sem );
        MBP_ASSERT( TX_SUCCESS == status );
    }
}

void
prvMBPSerialPoll( void )
{
    for( UBYTE ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
    {
        if( IDX_INVALID != xSerialHdls[ubIdx].ubIdx )
        {
            xSerialHandle  *hdl = &xSerialHdls[ubIdx];
            if( hdl->bHasTimeout )
            {
                hdl->bHasTimeout = FALSE;
                MBP_ENTER_CRITICAL_SECTION(  );
                if( ( NULL != hdl->pvMBPReceiveFN ) && ( hdl->bHasData ) )
                {
                    hdl->bHasData = FALSE;
                    xSerialBuf     *buf = vMBPSerialGetReadBuffer( hdl );
                    ( void )hdl->pvMBPReceiveFN( hdl->xMBMHdl, buf->arubBuffer, buf->arubBufferPos );
                }
                if( ( NULL != hdl->pvMBPReceiveFN ) && ( NULL != hdl->pbMBPTimerExpiredFN ) )
                {
                    ( void )hdl->pbMBPTimerExpiredFN( hdl->xMBMHdl );
                }
                MBP_EXIT_CRITICAL_SECTION(  );
            }
            if( hdl->bHasData )
            {
                hdl->bHasData = FALSE;
                MBP_ENTER_CRITICAL_SECTION(  );
                if( NULL != hdl->pvMBPReceiveFN )
                {
                    xSerialBuf     *buf = vMBPSerialGetReadBuffer( hdl );
                    ( void )hdl->pvMBPReceiveFN( hdl->xMBMHdl, buf->arubBuffer, buf->arubBufferPos );
                }
                MBP_EXIT_CRITICAL_SECTION(  );
            }
            if( hdl->bCanTx )
            {
                hdl->bCanTx = FALSE;
                MBP_ENTER_CRITICAL_SECTION(  );
                if( NULL != hdl->pbMBPTransmitterEmptyFN )
                {
                    USHORT          usTxBytes;
                    BOOL            hasData =
                        hdl->pbMBPTransmitterEmptyFN( hdl->xMBMHdl, hdl->txBuffer.arubBuffer, MB_UTILS_NARRSIZE( hdl->txBuffer.arubBuffer ), &usTxBytes );
                    if( hasData )
                    {
                        ssp_err_t       status =
                            xSerialHWHdls[hdl->ubIdx].xUART->p_api->write( xSerialHWHdls[hdl->ubIdx].xUART->p_ctrl, hdl->txBuffer.arubBuffer, usTxBytes );
                        ( void )status;
                    }
                    else
                    {
                        hdl->pbMBPTransmitterEmptyFN = NULL;
                    }
                }
                MBP_EXIT_CRITICAL_SECTION(  );
            }
        }
    }
}
