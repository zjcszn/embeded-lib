#include "HAL_Config.h"
#include "W5x00RelFunctions.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

void W5x00Initialze(void)
{
	intr_kind temp;
	int8_t phy_link =0;
#if _WIZCHIP_ < W5200
	unsigned char W5x00_AdrSet[2][4] = {{2,2,2,2},{2,2,2,2}};
#else
	unsigned char W5x00_AdrSet[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
#endif
	/*
	 */
#if _WIZCHIP_ == W5200
	temp = 0;
#else
	temp = IK_DEST_UNREACH;
#endif

	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)W5x00_AdrSet) == -1)
	{
		printf("W5x00 initialized fail.\r\n");
	}

	if(ctlwizchip(CW_SET_INTRMASK,&temp) == -1)
	{
		printf("W5x00 interrupt\r\n");
	}
	do{//check phy status.
		if(ctlwizchip(CW_GET_PHYLINK,&phy_link) == -1){
			printf("Unknown PHY link status.\r\n");
			delay(10);
		}

	}while(phy_link == PHY_LINK_OFF);
}

uint8_t spiReadByte(void)
{
	while (SPI_I2S_GetFlagStatus(W5x00_SPI, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(W5x00_SPI, 0xff);
	while (SPI_I2S_GetFlagStatus(W5x00_SPI, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(W5x00_SPI);
}

void spiWriteByte(uint8_t byte)
{
	while (SPI_I2S_GetFlagStatus(W5x00_SPI, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(W5x00_SPI, byte);
	while (SPI_I2S_GetFlagStatus(W5x00_SPI, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(W5x00_SPI);
}

//(*bus_wb)(uint32_t addr, iodata_t wb);
void busWriteByte(uint32_t addr, iodata_t data)
{
	(*((volatile uint8_t*)(W5x00Address+1))) = (uint8_t)((addr &0xFF00)>>8);
	(*((volatile uint8_t*)(W5x00Address+2))) = (uint8_t)((addr) & 0x00FF);
	(*((volatile uint8_t*)(W5x00Address+3))) = data;
}

//iodata_t (*bus_rb)(uint32_t addr);
iodata_t busReadByte(uint32_t addr)
{
	(*((volatile uint8_t*)(W5x00Address+1))) = (uint8_t)((addr &0xFF00)>>8);
	(*((volatile uint8_t*)(W5x00Address+2))) = (uint8_t)((addr) & 0x00FF);
	return  (*((volatile uint8_t*)(W5x00Address+3)));
}

inline void csEnable(void)
{
	GPIO_ResetBits(W5x00_CS_PORT, W5x00_CS_PIN);
}

inline void csDisable(void)
{
	GPIO_SetBits(W5x00_CS_PORT, W5x00_CS_PIN);
}

inline void resetAssert(void)
{
	GPIO_ResetBits(W5x00_RESET_PORT, W5x00_RESET_PIN);
}

inline void resetDeassert(void)
{
	GPIO_SetBits(W5x00_RESET_PORT, W5x00_RESET_PIN);
}

void W5x00Reset(void)
{
	int i,j,k;
	k=0;
	GPIO_ResetBits(W5x00_RESET_PORT,W5x00_RESET_PIN);
	CoTickDelay(10);
	GPIO_SetBits(W5x00_RESET_PORT,W5x00_RESET_PIN);
}
