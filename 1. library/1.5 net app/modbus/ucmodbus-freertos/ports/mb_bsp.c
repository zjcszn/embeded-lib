/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                    Copyright 2003-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            uC/Modbus
*
*                                      Board Support Package
*                                        ST STM32F407ZGT6
*
* Filename : mb_bsp.c
* Version  : V2.14.00
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include    "mb.h"
#include    "mb_os.h"
#include    "mb_bsp.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

extern void Error_Handler(void);


/*
*********************************************************************************************************
*                                             LOCAL VARIABLES
*********************************************************************************************************
*/

static  CPU_INT32U          MB_Tmr_ReloadCnts;
static  SemaphoreHandle_t   MB_TxDMA_SemTbl[MODBUS_CFG_MAX_CH];

 
/*
*********************************************************************************************************
*                                             MB_CommExit()
*
* Description : Terminates Modbus communications.  All Modbus channels are closed.
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommExit (void)
{
    CPU_INT08U   ch;
    MODBUS_CH   *pch;


    pch = &MB_ChTbl[0];
    for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {
#if    (MODBUS_CFG_USE_DMATX != DEF_ENABLED)
        MB_CommTxIntDis(pch);
#endif
        MB_CommRxIntDis(pch);
        pch++;
    }
}

/*
*********************************************************************************************************
*                                           MB_PortToUsart()
*
* Description : Convert port number to usart handle
*
* Argument(s) : port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a specific Modbus channel.
*
* Return(s)   : USART handle.
*
* Caller(s)   : ....
*
* Note(s)     : none.
*********************************************************************************************************
*/

USART_TypeDef* MB_PortToUsart(CPU_INT08U  port_nbr)
{
    switch (port_nbr) {
        case MB_BSP_UART_00:
            return USART1;

        case MB_BSP_UART_01:
            return USART2;

        case MB_BSP_UART_02:
            return USART3;

        default:
            MB_LOG("mb_bsp.c -> MB_PortToUsart() Incomplete\r\n");
            Error_Handler();
            break;  
    }
    return NULL;
}

/*
*********************************************************************************************************
*                                           MB_CommPortInit()
*
* Description : Initializes the serial port to the desired baud rate and the UART will be
*               configured for N, 8, 1 (No parity, 8 bits, 1 stop).
*
* Argument(s) : port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a specific Modbus channel.
*               usart_init initialization structure of usart peripheral.
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommPortCfg()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommPortInit(CPU_INT08U port_nbr, LL_USART_InitTypeDef *usart_init)
{
    USART_TypeDef           *usart_x    = MB_PortToUsart(port_nbr);
    LL_GPIO_InitTypeDef      gpio_init  = {0};

    gpio_init.Mode          =   LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed         =   LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType    =   LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull          =   LL_GPIO_PULL_UP;
    gpio_init.Alternate     =   LL_GPIO_AF_7;

    switch (port_nbr) {                                               /* Enable clock & Init GPIO  & Init interrupt   */
        case MB_BSP_UART_00:
            USART1_ENABLE_GPIO_CLOCK();
            USART1_ENABLE_USART_CLOCK();
            gpio_init.Pin = USART1_RX_GPIO_PIN | USART1_TX_GPIO_PIN;
            LL_GPIO_Init(USART1_GPIO_PORT, &gpio_init);
            NVIC_SetPriority(USART1_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
            NVIC_EnableIRQ(USART1_IRQN);       
            break;

        case MB_BSP_UART_01:
            USART2_ENABLE_GPIO_CLOCK();
            USART2_ENABLE_USART_CLOCK();
            gpio_init.Pin = USART2_RX_GPIO_PIN | USART2_TX_GPIO_PIN;
            LL_GPIO_Init(USART2_GPIO_PORT, &gpio_init);
            NVIC_SetPriority(USART2_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
            NVIC_EnableIRQ(USART2_IRQN);       
            break;

        case MB_BSP_UART_02:
            USART3_ENABLE_GPIO_CLOCK();
            USART3_ENABLE_USART_CLOCK();
            gpio_init.Pin = USART3_RX_GPIO_PIN | USART3_TX_GPIO_PIN;
            LL_GPIO_Init(USART3_GPIO_PORT, &gpio_init);
            NVIC_SetPriority(USART3_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
            NVIC_EnableIRQ(USART3_IRQN);       
            break;

        default:
            MB_LOG("mb_bsp.c -> MB_CommPortInit() Incomplete\r\n");
            Error_Handler();
            break;  
    }

    LL_USART_DeInit(usart_x);
    LL_USART_Init(usart_x, usart_init);
    LL_USART_ConfigAsyncMode(usart_x);
    LL_USART_EnableIT_RXNE(usart_x);
    LL_USART_Enable(usart_x);
}


/*
*********************************************************************************************************
*                                           MB_CommTxDMA_Init()
*
* Description : Initializes the serial port to the desired baud rate and the UART will be
*               configured for N, 8, 1 (No parity, 8 bits, 1 stop).
*
* Argument(s) : port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a specific Modbus channel.
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommPortCfg()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_USE_DMATX == DEF_ENABLED)

void MB_CommTxDMA_Init(CPU_INT08U port_nbr) 
{
    USART_TypeDef   *usart_x;
    DMA_TypeDef     *dmatx_port;
    CPU_INT32U       dmatx_stream;
    CPU_INT32U       dmatx_channel;
    CPU_INT32U       dmatx_irqn;

    usart_x = MB_PortToUsart(port_nbr);
    switch (port_nbr) {
        case MB_BSP_UART_00:
            USART1_ENABLE_DMA_CLOCK();
            dmatx_port     = USART1_TX_DMA;
            dmatx_stream   = USART1_TX_DMA_STREAM;
            dmatx_channel  = USART1_TX_DMA_CHANNEL;
            dmatx_irqn     = USART1_TX_DMA_IRQN;
            break;

        case MB_BSP_UART_01:
            USART2_ENABLE_DMA_CLOCK();
            dmatx_port     = USART2_TX_DMA;
            dmatx_stream   = USART2_TX_DMA_STREAM;
            dmatx_channel  = USART2_TX_DMA_CHANNEL;
            dmatx_irqn     = USART2_TX_DMA_IRQN;
            break;

        case MB_BSP_UART_02:
            USART3_ENABLE_DMA_CLOCK();
            dmatx_port     = USART3_TX_DMA;
            dmatx_stream   = USART3_TX_DMA_STREAM;
            dmatx_channel  = USART3_TX_DMA_CHANNEL;
            dmatx_irqn     = USART3_TX_DMA_IRQN;
            break;

        default:
            MB_LOG("mb_bsp.c -> MB_CommTxDMA_Init() Incomplete\r\n");
            Error_Handler();
            break;
    }

    NVIC_SetPriority(dmatx_irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(dmatx_irqn);       

    LL_DMA_DeInit(dmatx_port, dmatx_stream);	
    LL_DMA_SetChannelSelection(dmatx_port, dmatx_stream, dmatx_channel);
    LL_DMA_SetDataTransferDirection(dmatx_port, dmatx_stream, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);  // DMA 数据传输方向：内存->外设
    LL_DMA_SetStreamPriorityLevel(dmatx_port, dmatx_stream, LL_DMA_PRIORITY_MEDIUM);               // DMA 数据流优先级：MEDIUM
    LL_DMA_SetMode(dmatx_port, dmatx_stream, LL_DMA_MODE_NORMAL);                                  // DMA 模式：正常模式
    LL_DMA_SetPeriphIncMode(dmatx_port, dmatx_stream, LL_DMA_PERIPH_NOINCREMENT);                  // DMA 外设地址自增：否
    LL_DMA_SetMemoryIncMode(dmatx_port, dmatx_stream, LL_DMA_MEMORY_INCREMENT);                    // DMA 内存地址自增：是
    LL_DMA_SetPeriphSize(dmatx_port, dmatx_stream, LL_DMA_PDATAALIGN_BYTE);                        // DMA 外设数据宽度：BYTE
    LL_DMA_SetMemorySize(dmatx_port, dmatx_stream, LL_DMA_MDATAALIGN_BYTE);                        // DMA 内存数据宽度：BYTE
    LL_DMA_DisableFifoMode(dmatx_port, dmatx_stream);                                              // DMA 硬件FIFO配置：关闭
    LL_DMA_SetPeriphAddress(dmatx_port, dmatx_stream, LL_USART_DMA_GetRegAddr(usart_x));           // 设置外设地址
}

#endif


/*
*********************************************************************************************************
*                                         MB_CommTxDMA()
*
* Description : Transmit data by DMA channel.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_Tx()
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (   MODBUS_CFG_USE_DMATX == DEF_ENABLED)

void  MB_CommTxDMA (MODBUS_CH  *pch)
{  
    xSemaphoreTake(MB_TxDMA_SemTbl[pch->Ch], MB_BSP_DMA_TIMEOUT);

    switch (pch->PortNbr) {
        case MB_BSP_UART_00:
            LL_DMA_SetMemoryAddress(USART1_TX_DMA, USART1_TX_DMA_STREAM, (uint32_t)pch->TxBufPtr);    // 设置内存地址
            LL_DMA_SetDataLength(USART1_TX_DMA, USART1_TX_DMA_STREAM, pch->TxBufByteCtr);             // 设置DMA传输数据长度
            DMA_CLEAR_IT_FLAG(USART1_TX_DMA_STREAM_ID, TC,  USART1_TX_DMA);         // 清除发送完成标志      
            LL_DMA_EnableIT_TC(USART1_TX_DMA, USART1_TX_DMA_STREAM);                // 使能发送完成中断
            LL_DMA_EnableStream(USART1_TX_DMA, USART1_TX_DMA_STREAM);               // 使能DMA数据流
            LL_USART_EnableDMAReq_TX(USART1);                                       // 使能串口TX DMA传输
            break;

        case MB_BSP_UART_01:
            LL_DMA_SetMemoryAddress(USART2_TX_DMA, USART2_TX_DMA_STREAM, (uint32_t)pch->TxBufPtr);    // 设置内存地址
            LL_DMA_SetDataLength(USART2_TX_DMA, USART2_TX_DMA_STREAM, pch->TxBufByteCtr);             // 设置DMA传输数据长度
            DMA_CLEAR_IT_FLAG(USART2_TX_DMA_STREAM_ID, TC,  USART2_TX_DMA);         // 清除发送完成标志      
            LL_DMA_EnableIT_TC(USART2_TX_DMA, USART2_TX_DMA_STREAM);                // 使能发送完成中断
            LL_DMA_EnableStream(USART2_TX_DMA, USART2_TX_DMA_STREAM);               // 使能DMA数据流
            LL_USART_EnableDMAReq_TX(USART2);                                       // 使能串口TX DMA传输
            break;

        case MB_BSP_UART_02:
            LL_DMA_SetMemoryAddress(USART3_TX_DMA, USART3_TX_DMA_STREAM, (uint32_t)pch->TxBufPtr);    // 设置内存地址
            LL_DMA_SetDataLength(USART3_TX_DMA, USART3_TX_DMA_STREAM, pch->TxBufByteCtr);             // 设置DMA传输数据长度
            DMA_CLEAR_IT_FLAG(USART3_TX_DMA_STREAM_ID, TC,  USART3_TX_DMA);         // 清除发送完成标志      
            LL_DMA_EnableIT_TC(USART3_TX_DMA, USART3_TX_DMA_STREAM);                // 使能发送完成中断
            LL_DMA_EnableStream(USART3_TX_DMA, USART3_TX_DMA_STREAM);               // 使能DMA数据流
            LL_USART_EnableDMAReq_TX(USART3);                                       // 使能串口TX DMA传输
            break;

        default:
            MB_LOG("mb_bsp.c -> MB_CommTxDMA() Incomplete\r\n");
            Error_Handler();
            break;  
    }
}

#endif

/*
*********************************************************************************************************
*                                       MB_CommTxDMA_ISR_Handler()
*
* Description : ISR for DMA transmit complete.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is a ISR
*
* Note(s)     : none. 
*
*********************************************************************************************************
*/

#if (   MODBUS_CFG_USE_DMATX == DEF_ENABLED)

void  MB_CommTxDMA_ISR_Handler (CPU_INT08U  port_nbr)
{
    switch (port_nbr) {
        case MB_BSP_UART_00:
            MB_CommTxDMA_ISR_0_Handler();
            break;

        case MB_BSP_UART_01:
            MB_CommTxDMA_ISR_1_Handler();
            break;

        case MB_BSP_UART_02:
            MB_CommTxDMA_ISR_2_Handler();
            break;
    }
}

void  MB_CommTxDMA_ISR_0_Handler (void) 
{
    if (LL_DMA_IsEnabledIT_TC(USART1_TX_DMA, USART1_TX_DMA_STREAM) && 
            DMA_IS_ACTIVE_IT_FLAG(USART1_TX_DMA_STREAM_ID, TC, USART1_TX_DMA)) {

        LL_DMA_DisableIT_TC(USART1_TX_DMA, USART1_TX_DMA_STREAM);
        LL_USART_ClearFlag_TC(USART1);
        LL_USART_EnableIT_TC(USART1);
    }
}

void  MB_CommTxDMA_ISR_1_Handler (void) 
{
    if (LL_DMA_IsEnabledIT_TC(USART2_TX_DMA, USART2_TX_DMA_STREAM) && 
            DMA_IS_ACTIVE_IT_FLAG(USART2_TX_DMA_STREAM_ID, TC, USART2_TX_DMA)) {

        LL_DMA_DisableIT_TC(USART2_TX_DMA, USART2_TX_DMA_STREAM);
        LL_USART_ClearFlag_TC(USART2);
        LL_USART_EnableIT_TC(USART2);
    }
}

void  MB_CommTxDMA_ISR_2_Handler (void) 
{
    if (LL_DMA_IsEnabledIT_TC(USART3_TX_DMA, USART3_TX_DMA_STREAM) && 
            DMA_IS_ACTIVE_IT_FLAG(USART3_TX_DMA_STREAM_ID, TC, USART3_TX_DMA)) {

        LL_DMA_DisableIT_TC(USART3_TX_DMA, USART3_TX_DMA_STREAM);
        LL_USART_ClearFlag_TC(USART3);
        LL_USART_EnableIT_TC(USART3);
    }
}

#endif

/*
*********************************************************************************************************
*                                           MB_CommPortCfg()
*
* Description : Initializes the serial port to the desired baud rate and the UART will be
*               configured for N, 8, 1 (No parity, 8 bits, 1 stop).
*
* Argument(s) : pch        is a pointer to the Modbus channel
*               port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a specific Modbus channel.
*               baud       is the desired baud rate for the serial port.
*               parity     is the desired parity and can be either:
*
*                          MODBUS_PARITY_NONE
*                          MODBUS_PARITY_ODD
*                          MODBUS_PARITY_EVEN
*
*               bits       specifies the number of bit and can be either 7 or 8.
*               stops      specifies the number of stop bits and can either be 1 or 2
*
* Return(s)   : none.
*
* Caller(s)   : MB_CfgCh()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommPortCfg (MODBUS_CH  *pch,
                      CPU_INT08U  port_nbr,
                      CPU_INT32U  baud,
                      CPU_INT08U  bits,
                      CPU_INT08U  parity,
                      CPU_INT08U  stops)
{
    if (pch != (MODBUS_CH *)0){
        pch->PortNbr        = port_nbr;                               /* Store configuration in channel              */
        pch->BaudRate       = baud;
        pch->Parity         = parity;
        pch->Bits           = bits;
        pch->Stops          = stops;    
        
        /**************************************************************************************************************/

        LL_USART_InitTypeDef    usart_init = {0};                     /* ------ USART CONFIGURATION SETUP ----------  */

        switch (parity) {                                             /* (1) Setup USART parity                       */
            case MODBUS_PARITY_ODD:
                 usart_init.Parity = LL_USART_PARITY_ODD;
                 break;
                 
            case MODBUS_PARITY_EVEN:            
                 usart_init.Parity = LL_USART_PARITY_EVEN;
                 break;

            case MODBUS_PARITY_NONE:
            default:
                 usart_init.Parity = LL_USART_PARITY_NONE;
                 break;
        }
        
        if (pch->Bits == 7) {                                              /* (2) Setup #bits                               */
            usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
        } else {
            if (parity == MODBUS_PARITY_NONE) {
                usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
            } else {
                usart_init.DataWidth = LL_USART_DATAWIDTH_9B;
            }
        }
            
        if (pch->Stops == 1) {                                             /* (3) Setup # stops bits                        */
            usart_init.StopBits = LL_USART_STOPBITS_1;
        } else {
            usart_init.StopBits = LL_USART_STOPBITS_2;
        }
          
        usart_init.BaudRate = baud;                                        /* (4) Setup USART baud rate                     */
        usart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;
        
        MB_CommPortInit(port_nbr, &usart_init);

#if (   MODBUS_CFG_USE_DMATX == DEF_ENABLED)
        vSemaphoreCreateBinary(MB_TxDMA_SemTbl[pch->Ch]);
        MB_CommTxDMA_Init(port_nbr);
#endif

    }
}


/*
*********************************************************************************************************
*                                         MB_CommRxIntDis()
*
* Description : Disables Rx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommExit()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommRxIntDis (MODBUS_CH  *pch)
{  
    USART_TypeDef *usart_x = MB_PortToUsart(pch->PortNbr);
    LL_USART_DisableIT_RXNE(usart_x);
}

 
/*
*********************************************************************************************************
*                                          MB_CommRxIntEn()
*
* Description : Enables Rx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommRxIntEn (MODBUS_CH  *pch)
{   
    USART_TypeDef *usart_x = MB_PortToUsart(pch->PortNbr);
    LL_USART_ClearFlag_RXNE(usart_x);
    LL_USART_EnableIT_RXNE(usart_x);
}

 
/*
*********************************************************************************************************
*                                       MB_CommRxTxISR_Handler()
*
* Description : ISR for either a received or transmitted character.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is a ISR
*
* Note(s)     : (1) The pseudo-code for this function should be:  
*
*               if (Rx Byte has been received) {
*                  c = get byte from serial port;
*                  Clear receive interrupt;
*                  pch->RxCtr++;                      Increment the number of bytes received
*                  MB_RxByte(pch, c);                 Pass character to Modbus to process
*              }
*
*              if (Byte has been transmitted) {
*                  pch->TxCtr++;                      Increment the number of bytes transmitted
*                  MB_TxByte(pch);                    Send next byte in response
*                  Clear transmit interrupt           Clear Transmit Interrupt flag
*              }
*********************************************************************************************************
*/


void  MB_CommRxTxISR_Handler (CPU_INT08U  port_nbr)
{
              MODBUS_CH          *pch;
              CPU_INT08U          ch;
              CPU_INT08U          rx_data;

    
    pch                = &MB_ChTbl[0];
    
    for (ch = 0; ch < MODBUS_CFG_MAX_CH; ch++) {       /* Find the channel assigned to this port       */
        if (pch->PortNbr == port_nbr) {
            ch = MODBUS_CFG_MAX_CH;
        } else {
            pch++;
        }
    }

    USART_TypeDef *usart_x = MB_PortToUsart(port_nbr);

    if (LL_USART_IsEnabledIT_RXNE(usart_x) && LL_USART_IsActiveFlag_RXNE(usart_x)) {
        pch->RxCtr++;                                           /* Increment the Rx counter                                  */
        rx_data = LL_USART_ReceiveData8(usart_x);
        MB_RxByte(pch, rx_data);
    }

    if (LL_USART_IsEnabledIT_TC(usart_x) && LL_USART_IsActiveFlag_TC(usart_x)) {
        LL_USART_DisableIT_TC(usart_x);

#if    (MODBUS_CFG_USE_DMATX == DEF_ENABLED)
        xSemaphoreGiveFromISR(MB_TxDMA_SemTbl[pch->Ch], NULL);
#endif

#if    (MODBUS_CFG_RS485_EN  == DEF_ENABLED)
        MB_CommRS485_Recv(port_nbr);
#endif

        MB_CommRxIntEn(pch);                                    /* Re-enable the receiver for the next packet         */
    }

#if    (MODBUS_CFG_USE_DMATX  != DEF_ENABLED)
    if (LL_USART_IsEnabledIT_TXE(usart_x) && LL_USART_IsActiveFlag_TXE(usart_x)) {
        pch->TxCtr++;                                           
        MB_TxByte(pch);                                         /* Send next byte                                    */
    }
#endif
}

/*
*********************************************************************************************************
*                                UART #0 Rx/Tx Communication handler for Modbus
*********************************************************************************************************
*/

#ifdef  MB_BSP_UART_00 

void  MB_CommRxTxISR_0_Handler (void)
{
    MB_CommRxTxISR_Handler(MB_BSP_UART_00);
}

#endif

#ifdef  MB_BSP_UART_01
/*
*********************************************************************************************************
*                                UART #1 Rx/Tx Communication handler for Modbus
*********************************************************************************************************
*/

void  MB_CommRxTxISR_1_Handler (void)
{
    MB_CommRxTxISR_Handler(MB_BSP_UART_01);
}

#endif

/*
*********************************************************************************************************
*                                UART #2 Rx/Tx Communication handler for Modbus
*********************************************************************************************************
*/

#ifdef  MB_BSP_UART_02

void  MB_CommRxTxISR_2_Handler (void)
{
    MB_CommRxTxISR_Handler(MB_BSP_UART_02);
}

#endif


/*
*********************************************************************************************************
*                                UART #3 Rx/Tx Communication handler for Modbus
*********************************************************************************************************
*/

#ifdef  MB_BSP_UART_03

void  MB_CommRxTxISR_3_Handler (void)
{
    MB_CommRxTxISR_Handler(MB_BSP_UART_03);
}

#endif

/*
*********************************************************************************************************
*                                             MB_CommTx1()
*
* Description : This function is called to obtain the next byte to send from the transmit buffer.  When
*               all bytes in the reply have been sent, transmit interrupts are disabled and the receiver
*               is enabled to accept the next Modbus request.
*
* Argument(s) : c     is the byte to send to the serial port
*
* Return(s)   : none.
*
* Caller(s)   : MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if    (MODBUS_CFG_USE_DMATX  != DEF_ENABLED)

void  MB_CommTx1 (MODBUS_CH  *pch,
                  CPU_INT08U  c)

{    
    USART_TypeDef *usart_x = MB_PortToUsart(pch->PortNbr);
    LL_USART_TransmitData8(usart_x, c);
}

#endif

/*
*********************************************************************************************************
*                                         MB_CommTxIntDis()
*
* Description : Disables Tx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommExit()
*               MB_TxByte()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if    (MODBUS_CFG_USE_DMATX  != DEF_ENABLED)

void  MB_CommTxIntDis (MODBUS_CH  *pch)
{
    USART_TypeDef *usart_x = MB_PortToUsart(pch->PortNbr);
    LL_USART_DisableIT_TXE(usart_x);
}

#endif
 
/*
*********************************************************************************************************
*                                         MB_CommTxIntEn()
*
* Description : Enables Tx interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_Tx()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if    (MODBUS_CFG_USE_DMATX  != DEF_ENABLED)

void  MB_CommTxIntEn (MODBUS_CH  *pch)
{
    USART_TypeDef *usart_x = MB_PortToUsart(pch->PortNbr);
    LL_USART_EnableIT_TXE(usart_x);
}

#endif

/*
*********************************************************************************************************
*                                         MB_CommTxTcEn()
*
* Description : Enables Tx complete interrupts.
*
* Argument(s) : pch        is a pointer to the Modbus channel
*
* Return(s)   : none.
*
* Caller(s)   : MB_Tx()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommTxTcEn (MODBUS_CH  *pch)
{
    USART_TypeDef *usart_x = MB_PortToUsart(pch->PortNbr);
    LL_USART_ClearFlag_TC(usart_x);
    LL_USART_EnableIT_TC(usart_x);
}

/*
*********************************************************************************************************
*                                         MB_CommRS485_Send()
*
* Description : Change RS485 direction to send data.
*
* Argument(s) : port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a specific Modbus channel.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Tx()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommRS485_Send (CPU_INT08U  port_nbr)
{
    /* user code */
}

/*
*********************************************************************************************************
*                                         MB_CommRS485_Recv()
*
* Description : Change RS485 direction to receive data.
*
* Argument(s) : port_nbr   is the desired serial port number.  This argument allows you to assign a
*                          specific serial port to a specific Modbus channel.
*
* Return(s)   : none.
*
* Caller(s)   : MB_CommRxTxISR_Handler()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_CommRS485_Recv (CPU_INT08U  port_nbr)
{
    /* user code */
}
/*
*********************************************************************************************************
*                                           MB_RTU_TmrInit()
*
* Description : Initializes RTU timeout timer.
*
* Argument(s) : freq          Is the frequency of the modbus RTU timer interrupt.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrInit (void)
{
    LL_TIM_InitTypeDef      tim_init     = {0};
    CPU_INT32U              tim_clk_freq =  0;
    LL_RCC_ClocksTypeDef    rcc_clk_info = {0};

    LL_RCC_GetSystemClocksFreq(&rcc_clk_info);
    tim_clk_freq = LL_RCC_GetAPB2Prescaler() ? (rcc_clk_info.PCLK2_Frequency << 1) : rcc_clk_info.PCLK2_Frequency;    
    MB_Tmr_ReloadCnts    = (tim_clk_freq / 1000) / MB_RTU_Freq - 1;

    NVIC_SetPriority(TIM1_UP_TIM10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

    tim_init.Prescaler = 999;
    tim_init.Autoreload = MB_Tmr_ReloadCnts;

    LL_TIM_Init(TIM1, &tim_init);
    LL_TIM_DisableARRPreload(TIM1);
    LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM1);
    LL_TIM_EnableIT_UPDATE(TIM1);
    LL_TIM_EnableCounter(TIM1);

    MB_RTU_TmrResetAll();                                     /* Reset all the RTU timers.                                   */
}
#endif


 
/*
*********************************************************************************************************
*                                           MB_RTU_TmrExit()
*
* Description : Disables RTU timeout timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrExit (void)
{
    LL_TIM_DeInit(TIM1);
}
#endif

 
/*
*********************************************************************************************************
*                                       MB_RTU_TmrISR_Handler()
*
* Description : Handles the case when the RTU timeout timer expires.
*
* Arguments   : none.
*
* Returns     : none.
*
* Caller(s)   : This is a ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_RTU_EN == DEF_ENABLED)
void  MB_RTU_TmrISR_Handler (void)
{
    MB_RTU_TmrCtr++;                                          /* Indicate that we had activities on this interrupt.          */
    MB_RTU_TmrUpdate();                                       /* Check for RTU timers that have expired                      */
}

#endif
