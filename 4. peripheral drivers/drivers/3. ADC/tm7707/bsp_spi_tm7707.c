#include "bsp_spi_tm7707.h"
#include "main.h"


void bsp_spi_tm7707_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_SPI_InitTypeDef SPI_InitStruct = {0};

    /* SPI3 SCK GPIO pin configuration*/
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    
    GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
    GPIO_InitStruct.Pin = SPI_TM7707_CLK_PIN;
    LL_GPIO_Init(SPI_TM7707_CLK_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
    GPIO_InitStruct.Pin = SPI_TM7707_MISO_PIN;
    LL_GPIO_Init(SPI_TM7707_MISO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    GPIO_InitStruct.Pin = SPI_TM7707_MOSI_PIN;
    LL_GPIO_Init(SPI_TM7707_MOSI_PORT, &GPIO_InitStruct);

    /* Configure SPI MASTER ****************************************************/
    /* Enable SPI3 Clock */
    LL_RCC_SetClockSource(LL_RCC_SPI123_CLKSOURCE_CLKP);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);     /* APB2 peri clk: 120MHZ max*/

    /* Configure the SPI1 parameters */
    SPI_InitStruct.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV32;  /* 120MHZ/32 = 3.75MHZ */
    SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
    SPI_InitStruct.ClockPhase        = LL_SPI_PHASE_2EDGE;
    SPI_InitStruct.ClockPolarity     = LL_SPI_POLARITY_HIGH;
    SPI_InitStruct.BitOrder          = LL_SPI_MSB_FIRST;
    SPI_InitStruct.DataWidth         = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.NSS               = LL_SPI_NSS_SOFT;
    SPI_InitStruct.CRCCalculation    = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.Mode              = LL_SPI_MODE_MASTER;

    LL_SPI_Init(SPI3, &SPI_InitStruct);
    LL_I2S_Disable(SPI3);

    /* Set number of date to transmit */
    LL_SPI_SetTransferSize(SPI3, 0);

    /* Lock GPIO for master to avoid glitches on the clock output */
    // LL_SPI_EnableGPIOControl(SPI3);
    LL_SPI_DisableMasterRxAutoSuspend(SPI3);
    LL_SPI_SetFIFOThreshold(SPI3, LL_SPI_FIFO_TH_01DATA);

    LL_SPI_EnableIOLock(SPI3);
    
    /* Enable SPI1 */
    LL_SPI_SetStandard(SPI3, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_Enable(SPI3);
    LL_SPI_StartMasterTransfer(SPI3);
}


static inline uint8_t bsp_spi_tm7707_write_read(uint8_t data) {
    while (!LL_SPI_IsActiveFlag_TXP(SPI3));
    LL_SPI_TransmitData8(SPI3, data);
    while (!LL_SPI_IsActiveFlag_RXP(SPI3));
    return LL_SPI_ReceiveData8(SPI3);
}


void bsp_spi_tm7707_write_then_stop(uint8_t *data, uint16_t size) {
    for (int i = 0; i < size; i++) {
        (void) bsp_spi_tm7707_write_read(data[i]);
    }
}

void bsp_spi_tm7707_write_then_read(uint8_t *tx_data, uint16_t tx_size, uint8_t *rx_data, uint16_t rx_size) {
    for (int i = 0; i < tx_size; i++) {
        (void) bsp_spi_tm7707_write_read(tx_data[i]); 
    }

    for (int i = 0; i < rx_size; i++) {
        rx_data[i] = bsp_spi_tm7707_write_read(0x00);
    }
}