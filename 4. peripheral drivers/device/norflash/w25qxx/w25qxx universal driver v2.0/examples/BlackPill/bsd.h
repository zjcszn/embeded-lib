//#include "core_cm4.h"
#include "stm32f4xx.h"
#include "w25qxx.h"

#define PA(x)					(GPIOA_BASE | x)
#define PC(x)					(GPIOC_BASE | x)

#define LED_BLUE				PC(13)	// BlackPill

#define USART1_BR				115200 // bps
#define USART1_PIN_RX			PA(10)
#define USART1_PIN_TX			PA( 9)

#define SPI1_PIN_MOSI			PA( 7)
#define SPI1_PIN_MISO			PA( 6)
#define SPI1_PIN_SCK			PA( 5)

#define W25QXX_PIN_DI_IO0		SPI1_PIN_MOSI
#define W25QXX_PIN_DO_IO1		SPI1_PIN_MISO
#define W25QXX_PIN_SCK			SPI1_PIN_SCK
#define W25QXX_PIN_CS			PA( 4)
#define W25QXX_PIN_WP_IO2		0 // (+3V3)
#define W25QXX_PIN_HOLD_IO3		0 // (+3V3)

#define LedOn(led)				PinHi(led)
#define LedOff(led)				PinLo(led)
#define LedTog(led)				PinTog(led)

void Init_GPIO_AF(const uint32_t pin, const uint8_t config, const uint8_t alt_func);

void Init_USART1_DBG(uint32_t br);
void USART_DBG_DataTxGeneric(const void * const USARTx, const char *data, unsigned short msg_len);

void SPI1_Init(const uint8_t prescaler);
uint8_t SPI1_ByteRx(void);
void SPI1_ByteTx(uint8_t data);
uint8_t SPI1_ByteTRx(uint8_t data);

void PinHi(const uint32_t pin);
void PinLo(const uint32_t pin);
void PinTog(const uint32_t pin);

void delay_us(uint32_t microsec);
void delay_ms(uint32_t millisec);

uint8_t board_init(void);



