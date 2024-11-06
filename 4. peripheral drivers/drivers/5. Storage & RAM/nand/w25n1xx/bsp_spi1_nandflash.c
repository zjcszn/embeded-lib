#include "main.h"
#include "bsp_spi1_nandflash.h"


typedef enum {
    SPI_IDLE,
    SPI_BUSY,
    SPI_COMPLETE,
    SPI_ERR,
} spi_status_t;

static uint32_t dummy;
static volatile spi_status_t spi_status;



void bsp_spi_nandflash_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);  
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);    /* 80Mhz */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    /* (1) Enables GPIO clock and configures the SPI1 pins ********************/
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pin = NANDFLASH_MISO_PIN | NANDFLASH_MOSI_PIN | NANDFLASH_SCK_PIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    LL_GPIO_Init(NANDFLASH_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pin = NANDFLASH_CS_PIN;
    LL_GPIO_Init(NANDFLASH_CS_PORT, &GPIO_InitStruct);

    /* (2) Configure the LL_DMA_CHANNEL_2 functional parameters */
    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_2,
                            LL_DMA_DIRECTION_PERIPH_TO_MEMORY | LL_DMA_PRIORITY_LOW | LL_DMA_MODE_NORMAL |
                            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
                            LL_DMA_PDATAALIGN_BYTE | LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2, LL_SPI_DMA_GetRegAddr(SPI1), (uint32_t)NULL,
                            LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, 0);
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMA_REQUEST_1);

    /* (3) Configure the LL_DMA_CHANNEL_3 functional parameters */
    LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_3,
                            LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_PRIORITY_LOW | LL_DMA_MODE_NORMAL |
                            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
                            LL_DMA_PDATAALIGN_BYTE | LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3, (uint32_t)NULL, LL_SPI_DMA_GetRegAddr(SPI1),
                            LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, 0);
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMA_REQUEST_1);

    /* (4) Enable DMA interrupts complete/error */
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    /* (5) Configure NVIC for SPI1 transfer complete/error interrupts **********/
    NVIC_SetPriority(DMA1_Channel2_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    /* (6) Configure SPI1 functional parameters ********************************/

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
    LL_SPI_SetBaudRatePrescaler(NANDFLASH_SPI, LL_SPI_BAUDRATEPRESCALER_DIV2);
    LL_SPI_SetTransferDirection(NANDFLASH_SPI, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetClockPhase(NANDFLASH_SPI, LL_SPI_PHASE_2EDGE);     /* W25N01仅支持SPI <0.0> <1.1>模式, MSB */
    LL_SPI_SetClockPolarity(NANDFLASH_SPI, LL_SPI_POLARITY_HIGH);
    LL_SPI_SetTransferBitOrder(NANDFLASH_SPI, LL_SPI_MSB_FIRST);
    LL_SPI_SetDataWidth(NANDFLASH_SPI, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetNSSMode(NANDFLASH_SPI, LL_SPI_NSS_SOFT);
    LL_SPI_SetRxFIFOThreshold(NANDFLASH_SPI, LL_SPI_RX_FIFO_TH_QUARTER);
    LL_SPI_SetMode(NANDFLASH_SPI, LL_SPI_MODE_MASTER);
    LL_SPI_SetStandard(NANDFLASH_SPI, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_Enable(NANDFLASH_SPI);
}


uint8_t bsp_spi_nandflash_read_write_byte(uint8_t c) {
    uint8_t tmp;
    while(!LL_SPI_IsActiveFlag_TXE(NANDFLASH_SPI));
    LL_SPI_TransmitData8(NANDFLASH_SPI, c);
    while(!LL_SPI_IsActiveFlag_RXNE(NANDFLASH_SPI));
    tmp = LL_SPI_ReceiveData8(NANDFLASH_SPI);
    return tmp;
}


bool bsp_spi_nandflash_read_buffer(uint8_t * buf, uint32_t len) {
    bool ret;

    if (spi_status != SPI_IDLE) {
        return false;
    }

    spi_status = SPI_BUSY;
    dummy = ~0UL;

    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_NOINCREMENT);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)&dummy);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, len);

    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)buf);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, len);

    LL_DMA_ClearFlag_TC2(DMA1);
    LL_DMA_ClearFlag_TE2(DMA1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    LL_SPI_EnableDMAReq_RX(SPI1);
    LL_SPI_EnableDMAReq_TX(SPI1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

    while (spi_status < SPI_COMPLETE);

    ret = (spi_status == SPI_COMPLETE);
    spi_status = SPI_IDLE;

    return  ret;
}

bool bsp_spi_nandflash_write_buffer(uint8_t * buf, uint32_t len) {
    bool ret;
    
    if (spi_status != SPI_IDLE) {
        return false;
    }

    spi_status = SPI_BUSY;

    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_3, (uint32_t)buf);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, len);

    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_NOINCREMENT);
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)&dummy);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, len);

    LL_DMA_ClearFlag_TC2(DMA1);
    LL_DMA_ClearFlag_TE2(DMA1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    LL_SPI_EnableDMAReq_RX(SPI1);
    LL_SPI_EnableDMAReq_TX(SPI1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

    while (spi_status < SPI_COMPLETE);
    
    ret = (spi_status == SPI_COMPLETE);
    spi_status = SPI_IDLE;

    return  ret;
}

void DMA1_Channel2_IRQHandler(void) {
    if (LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_2) && LL_DMA_IsActiveFlag_TC2(DMA1)) {
        LL_DMA_ClearFlag_TC2(DMA1);
        spi_status = SPI_COMPLETE;
    }

    if (LL_DMA_IsEnabledIT_TE(DMA1, LL_DMA_CHANNEL_2) && LL_DMA_IsActiveFlag_TE2(DMA1)) {
        LL_DMA_ClearFlag_TE2(DMA1);
        spi_status = SPI_ERR;
    }

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_SPI_DisableDMAReq_TX(SPI1);
    LL_SPI_DisableDMAReq_RX(SPI1);
    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_DisableIT_TE(DMA1, LL_DMA_CHANNEL_2);
}