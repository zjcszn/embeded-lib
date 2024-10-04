#include "bsp_uart.h"


static void bsp_uart_gpio_init(void);
static void bsp_uart_uart_init(void);
static void bsp_uart_nvic_init(void);
static void bsp_uart_dmarx_config(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size);
static void bsp_uart_dmatx_config(void);

void bsp_uart_init(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size) {

    UART_ENABLE_CLOCK();

    bsp_uart_gpio_init();
    bsp_uart_dmarx_config(dmarx_buf_addr, dmarx_buf_size);
    bsp_uart_dmatx_config();
    bsp_uart_nvic_init();
    bsp_uart_uart_init();

    LL_USART_EnableDMAReq_RX(UART); 
    LL_USART_ClearFlag_IDLE(USART1);
    LL_USART_EnableIT_IDLE(UART); 
    LL_USART_Enable(UART); 
}


static void bsp_uart_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = UART_RX_GPIO_PIN | UART_TX_GPIO_PIN;   
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                 
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;                   
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;             
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;                              
    GPIO_InitStruct.Alternate = UART_GPIO_AF;                           
    LL_GPIO_Init(UART_GPIO_PORT, &GPIO_InitStruct);                   
}


static void bsp_uart_uart_init(void) {
    LL_USART_InitTypeDef UART_InitStruct = {0};
    UART_InitStruct.BaudRate = UART_BAUD;                      
    UART_InitStruct.DataWidth = UART_DATAWIDTH;
    UART_InitStruct.StopBits = UART_STOP; 
    UART_InitStruct.Parity = UART_PARITY;
    UART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;        
    UART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;    
    UART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;  
    LL_USART_Init(UART, &UART_InitStruct);
    LL_USART_ConfigAsyncMode(UART);
}


static void bsp_uart_nvic_init(void) {
    NVIC_SetPriority(UART_RX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), UART_NVIC_PRIOR, 0));    
    NVIC_EnableIRQ(UART_RX_DMA_IRQN);                                                              
    NVIC_SetPriority(UART_TX_DMA_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), UART_NVIC_PRIOR, 0));     
    NVIC_EnableIRQ(UART_TX_DMA_IRQN);                                                              
    NVIC_SetPriority(UART_IRQN, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), UART_NVIC_PRIOR, 0));           
    NVIC_EnableIRQ(UART_IRQN);                                                                        
}


static void bsp_uart_dmarx_config(uint8_t *dmarx_buf_addr, uint32_t dmarx_buf_size) {

    LL_DMA_DeInit(UART_RX_DMA, UART_RX_DMA_STREAM);                                                      
    LL_DMA_SetChannelSelection(UART_RX_DMA, UART_RX_DMA_STREAM, UART_RX_DMA_CHANNAL);                    
    LL_DMA_SetDataTransferDirection(UART_RX_DMA, UART_RX_DMA_STREAM, LL_DMA_DIRECTION_PERIPH_TO_MEMORY); 
    LL_DMA_SetStreamPriorityLevel(UART_RX_DMA, UART_RX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);              
    LL_DMA_SetMode(UART_RX_DMA, UART_RX_DMA_STREAM, LL_DMA_MODE_CIRCULAR);                                
    LL_DMA_SetPeriphIncMode(UART_RX_DMA, UART_RX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  
    LL_DMA_SetMemoryIncMode(UART_RX_DMA, UART_RX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                   
    LL_DMA_SetPeriphSize(UART_RX_DMA, UART_RX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);                       
    LL_DMA_SetMemorySize(UART_RX_DMA, UART_RX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);                        
    LL_DMA_DisableFifoMode(UART_RX_DMA, UART_RX_DMA_STREAM);                                            

    LL_DMA_SetPeriphAddress(UART_RX_DMA, UART_RX_DMA_STREAM, LL_USART_DMA_GetRegAddr(UART));            
    LL_DMA_SetMemoryAddress(UART_RX_DMA, UART_RX_DMA_STREAM, (uint32_t)dmarx_buf_addr);                  
    LL_DMA_SetDataLength(UART_RX_DMA, UART_RX_DMA_STREAM, dmarx_buf_size);                                

    DMA_CLEAR_IT_FLAG(UART_RX_DMA_STREAM_ID, HT, UART_RX_DMA);
    DMA_CLEAR_IT_FLAG(UART_RX_DMA_STREAM_ID, TC, UART_RX_DMA);
    LL_DMA_EnableIT_HT(UART_RX_DMA, UART_RX_DMA_STREAM);                                                  
    LL_DMA_EnableIT_TC(UART_RX_DMA, UART_RX_DMA_STREAM);                                                  
    LL_DMA_EnableStream(UART_RX_DMA, UART_RX_DMA_STREAM);                                                 
}


static void bsp_uart_dmatx_config(void) {

    LL_DMA_DeInit(UART_TX_DMA, UART_TX_DMA_STREAM);                                                      
    LL_DMA_SetChannelSelection(UART_TX_DMA, UART_TX_DMA_STREAM, UART_TX_DMA_CHANNAL);                 
    LL_DMA_SetDataTransferDirection(UART_TX_DMA, UART_TX_DMA_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);  
    LL_DMA_SetStreamPriorityLevel(UART_TX_DMA, UART_TX_DMA_STREAM, LL_DMA_PRIORITY_MEDIUM);              
    LL_DMA_SetMode(UART_TX_DMA, UART_TX_DMA_STREAM, LL_DMA_MODE_NORMAL);                                
    LL_DMA_SetPeriphIncMode(UART_TX_DMA, UART_TX_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  
    LL_DMA_SetMemoryIncMode(UART_TX_DMA, UART_TX_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                 
    LL_DMA_SetPeriphSize(UART_TX_DMA, UART_TX_DMA_STREAM, LL_DMA_PDATAALIGN_BYTE);                      
    LL_DMA_SetMemorySize(UART_TX_DMA, UART_TX_DMA_STREAM, LL_DMA_MDATAALIGN_BYTE);                       
    LL_DMA_DisableFifoMode(UART_TX_DMA, UART_TX_DMA_STREAM);                                              
    LL_DMA_SetPeriphAddress(UART_TX_DMA, UART_TX_DMA_STREAM, LL_USART_DMA_GetRegAddr(UART));             

 } 

void bsp_uart_dmatx_enable(uint8_t *dmatx_buf_addr, uint32_t data_length) {
    LL_DMA_SetMemoryAddress(UART_TX_DMA, UART_TX_DMA_STREAM, (uint32_t)dmatx_buf_addr);                   
    LL_DMA_SetDataLength(UART_TX_DMA, UART_TX_DMA_STREAM, data_length);                                   

    DMA_CLEAR_IT_FLAG(UART_TX_DMA_STREAM_ID, TC,  UART_TX_DMA);               
    LL_DMA_EnableIT_TC(UART_TX_DMA, UART_TX_DMA_STREAM);              
    LL_DMA_EnableStream(UART_TX_DMA, UART_TX_DMA_STREAM);               
    LL_USART_EnableDMAReq_TX(UART);                                    
}

uint32_t bsp_uart_dmarx_buf_remain_size(void) {
    return LL_DMA_GetDataLength(UART_RX_DMA, UART_RX_DMA_STREAM);
}