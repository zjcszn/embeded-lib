#include <mbportssp.h>
#include <mbport.h>

void
user_uart_callback(uart_callback_args_t * p_args)
{
    switch( p_args->event )
    {
        case UART_EVENT_RX_CHAR:
            prvMBPSerialReceiveChar( 0, ( UBYTE )p_args->data );
            break;
        case UART_EVENT_TX_COMPLETE:
            prvMBPSerialCanTransmit( 0 );
            break;
        default:
            break;
    }
}

void
mbport_timer_ms(timer_callback_args_t *p_args)
{
    if( TIMER_EVENT_EXPIRED == p_args->event )
    {
        prvMBPTimerTickMS( );
        prvMBPSerialTimeoutDetect( );
    }
}

/* MODBUS port entry function */
void mbportssp_entry(void)
{
    g_timer0.p_api->open( g_timer0.p_ctrl, g_timer0.p_cfg );
    g_timer0.p_api->start( g_timer0.p_ctrl );
    while (1)
    {
        UINT status = tx_semaphore_get( &g_mbport_sem, 100 );
        if( TX_SUCCESS == status )
        {
            prvMBPTimerPoll( );
            prvMBPSerialPoll( );
        }
    }
}
