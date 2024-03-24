#include "mbmaster.h"

#include "mbport.h"
#include "mbm.h"

#define MBM_SERIAL_PORT             ( 0 )
#define MBM_SERIAL_BAUDRATE         ( 38400 )
#define MBM_PARITY                  ( MB_PAR_ODD )

/* MODBUS master entry function */
void mbmaster_entry(void)
{
    eMBErrorCode    eStatus, eStatus2;
    xMBHandle       xMBMMaster;
    USHORT          usNRegs[10];
    bsp_leds_t leds;

    R_BSP_LedsGet(&leds);

    /* Initialize the porting layer - DO NOT REMOVE */
    vMBPInit( );

    /* This demo opens a serial master instance and polls a MODBUS slave. Then the instance
     * is closed and reopened again for testing startup/shutdown behavior. Opening/closing
     * the stack is only necessary in case there is a low-level error with the UART or
     * the stacks needs to be reconfigured to different baudrate, parity, etc.
     */

    g_ioport.p_api->pinDirectionSet( IOPORT_PORT_06_PIN_03, IOPORT_DIRECTION_OUTPUT );
    g_ioport.p_api->pinWrite( IOPORT_PORT_06_PIN_03, IOPORT_LEVEL_LOW );
    g_ioport.p_api->pinDirectionSet( IOPORT_PORT_06_PIN_04, IOPORT_DIRECTION_OUTPUT );
    g_ioport.p_api->pinWrite( IOPORT_PORT_06_PIN_04, IOPORT_LEVEL_LOW );
    while (1)
    {
        if( MB_ENOERR == ( eStatus = eMBMSerialInit( &xMBMMaster, MB_RTU, MBM_SERIAL_PORT, MBM_SERIAL_BAUDRATE, MBM_PARITY ) ) )
        {
            for( unsigned cnt = 0; cnt < 100; cnt++ )
            {
                eStatus = MB_ENOERR;
                if( MB_ENOERR != ( eStatus2 = eMBMReadHoldingRegisters( xMBMMaster, 1, 0, 10, usNRegs ) ) )
                {
                    eStatus = eStatus2;
                }
                usNRegs[0]++;
                if( MB_ENOERR != ( eStatus2 = eMBMWriteMultipleRegisters( xMBMMaster, 1, 0, 10, usNRegs ) ) )
                {
                    eStatus = eStatus2;
                }

                tx_thread_sleep( 10 );
                g_ioport.p_api->pinWrite(leds.p_leds[0], MB_ENOERR == eStatus ? IOPORT_LEVEL_LOW: IOPORT_LEVEL_HIGH );
                g_ioport.p_api->pinWrite(leds.p_leds[1], MB_ENOERR == eStatus ? IOPORT_LEVEL_HIGH: IOPORT_LEVEL_LOW );
            }
            if( MB_ENOERR != ( eStatus = eMBMClose( xMBMMaster ) ) )
            {
                MBP_ASSERT( 0 );
            }
        }
        else
        {
            tx_thread_sleep (100);
        }
    }
}
