#include "bsd.h"

// =================================================================================================
// GPIO
// =================================================================================================

#define GPIOX_BASE_MASK			0xFFFFFF00
#define GPIO_PINX_MASK			0x0000000F // max #15
#define GET_HW_PIN(x)			(1<<((x) & GPIO_PINX_MASK)) // (uint16_t)
#define GET_HW_PIN_NUM(x)		((x) & GPIO_PINX_MASK)
#define GET_HW_PORT(x)			((GPIO_TypeDef *) (GPIOX_BASE_MASK & (x)))
#define GET_HW_PORT_NUM(x)		((uint8_t) (((x)>>10)-2))

enum GPIO_AF_e {
	GPIO_PF				= 0x00, // use primary function mapping
	GPIO_AF_SPI1		= 0x05,
	GPIO_AF_USART1		= 0x07,
};
enum GPIO_TYPE_e {
	GPIO_TYPE_IN,	// general Purpose Input
	GPIO_TYPE_OUT,	// general Purpose Output
	GPIO_TYPE_AF,	// Alternate Function
	GPIO_TYPE_AN,	// Analog
};
enum GPIO_CLK_e {
	GPIO_CLK_2MHZ,
	GPIO_CLK_25MHZ,
	GPIO_CLK_50MHZ,
	GPIO_CLK_100MHZ,

	GPIO_CLK_LOW  = GPIO_CLK_2MHZ,
	GPIO_CLK_MED  = GPIO_CLK_25MHZ,
	GPIO_CLK_HIGH = GPIO_CLK_50MHZ,
	GPIO_CLK_MAX  = GPIO_CLK_100MHZ,
};
enum GPIO_PT_e {
	GPIO_PT_NP,		// Pull Type = No Pull
	GPIO_PT_PU,		// Pull Type = Pull Up
	GPIO_PT_PD,		// Pull Type = Pull Down
};
enum GPIO_OT_e {
	GPIO_OT_PP,		// Output Type = Push/Pull
	GPIO_OT_OD,		// Output Type = Open Drain
};
enum GPIO_MODE_e {
	GPIO_MODE_AIN		= (GPIO_TYPE_AN <<2),														// INPUT, Analog
	GPIO_MODE_IN_FL 	= (GPIO_TYPE_IN <<2),														// INPUT, Floating (saves more power compare to ipd or ipu - use if appropriate)
	GPIO_MODE_IN_PU 	= (GPIO_TYPE_IN <<2) | (GPIO_PT_PU<<5),										// INPUT, pulled down
	GPIO_MODE_IN_PD 	= (GPIO_TYPE_IN <<2) | (GPIO_PT_PD<<5),										// INPUT, pulled up
	GPIO_MODE_OUT_OD	= (GPIO_TYPE_OUT<<2) | (GPIO_OT_OD<<4),										// OUTPUT open drain
	GPIO_MODE_OUT_PP	= (GPIO_TYPE_OUT<<2) | (GPIO_OT_PP<<4),										// OUTPUT push-pull
	GPIO_MODE_AF_OD		= (GPIO_TYPE_AF <<2) | (GPIO_OT_OD<<4),										// Alternate Function, open drain
	GPIO_MODE_AF_PP		= (GPIO_TYPE_AF <<2) | (GPIO_OT_PP<<4),										// Alternate Function, push-pull
//	GPIO_MODE_AF		= (GPIO_TYPE_AF <<2),														// Alternate Function, push-pull
};

typedef union GPIO_Mode_Struct_t {
	struct {
		uint8_t SPD:2,TYPE:2,OTYPE:1,PUPD:2;
	};
	uint8_t DATA;
} GPIO_Mode_Struct_t;

void PinHi(const uint32_t pin)	{ if (pin) { GET_HW_PORT(pin)->BSRRL = GET_HW_PIN(pin); }; };
void PinLo(const uint32_t pin)	{ if (pin) { GET_HW_PORT(pin)->BSRRH = GET_HW_PIN(pin); }; };
void PinTog(const uint32_t pin)	{ if (pin) { GET_HW_PORT(pin)->ODR  ^= GET_HW_PIN(pin); }; };
void Init_GPIO_AF(const uint32_t pin, const uint8_t config, const uint8_t alt_func) {
  GPIO_Mode_Struct_t* const cfg = (GPIO_Mode_Struct_t *)&config;
  GPIO_TypeDef* const GPIOx = GET_HW_PORT(pin);
  const uint8_t pin_num = GET_HW_PIN_NUM(pin);

	if ((GPIOx == 0) || (GET_HW_PIN(pin) == 0)) { return; };

	// Reset all config registers
	GPIOx->OTYPER  &= ~(GPIO_OTYPER_OT_0       << (pin_num    ));
	GPIOx->MODER   &= ~(GPIO_MODER_MODER0      << (pin_num * 2));
	GPIOx->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << (pin_num * 2));
	GPIOx->PUPDR   &= ~(GPIO_PUPDR_PUPDR0      << (pin_num * 2));

	// Set new config
	GPIOx->MODER |= (((uint32_t)cfg->TYPE) << (pin_num * 2));										// Pin Mode Config
	GPIOx->PUPDR |= (((uint32_t)cfg->PUPD) << (pin_num * 2));										// Pull-up Pull down resistor configuration
	if ((config & (GPIO_TYPE_OUT<<2)) || (config & (GPIO_TYPE_AF<<2))) {
		GPIOx->OSPEEDR |= ((uint32_t)(cfg->SPD) << (pin_num * 2));									// Speed mode configuration
		GPIOx->OTYPER  |= (((uint16_t)cfg->OTYPE) << (pin_num));									// Output mode configuration
		//if (config & (GPIO_TYPE_AF<<2)) { };//if(AF)
	};//if(OUT/AF)
	GPIOx->AFR[pin_num >> 3] &= ~((uint32_t)0xF      << ((pin_num & 0x07) * 4));					// clear AF config | AFR[0] is for pins 0..7
	GPIOx->AFR[pin_num >> 3] |=  ((uint32_t)alt_func << ((pin_num & 0x07) * 4));					// set   AF config | AFR[1] is for pins 8..15

};

// =================================================================================================
// USART
// =================================================================================================

#define USART_RXNE(USARTx)								((USARTx)->SR & USART_SR_RXNE)
#define USART_WAIT_TXE(USARTx)							while (!(((USART_TypeDef *)(USARTx))->SR & USART_SR_TXE))
#define USART_WAIT_TC(USARTx)							while (!(((USART_TypeDef *)(USARTx))->SR &  USART_SR_TC))
#define USART_DATA_TX(USARTx, data)						(((USART_TypeDef *)(USARTx))->DR = data)

typedef union USART_BR_Div_t { struct { uint16_t fraction:4,mantissa:11; }; uint16_t val; } USART_BR_Div_t;
static void USART_Set_BR(void *USARTx, uint32_t br) {
  uint32_t divisor_x100 = 0;
  USART_BR_Div_t br_div;

	if ((USARTx == USART1) || (USARTx == USART6)) { // on STM32F407 USART1 and USART6 are the only usarts clocked by high-speed APB2
		if (RCC->CFGR & RCC_CFGR_PPRE2_2) { // if (PCLK2 is divided)
			divisor_x100 = ((RCC->CFGR & (RCC_CFGR_PPRE2_0|RCC_CFGR_PPRE2_1)) >> 11) + 1;
		};
	} else { // USARTs 2,3,4,5
		if (RCC->CFGR & RCC_CFGR_PPRE1_2) { // if (PCLK1 is divided)
			divisor_x100 = ((RCC->CFGR & (RCC_CFGR_PPRE1_0|RCC_CFGR_PPRE1_1)) >> 8) + 1;
		};
	};

	divisor_x100 = SystemCoreClock >> divisor_x100;
	divisor_x100 = ((25 * divisor_x100) / (4*br));							// 450
	br_div.mantissa = divisor_x100 / 100;									// m = 4
	divisor_x100 = (divisor_x100 % 100) * 16;								// d = 800
	if ((divisor_x100 % 100) > 50) { divisor_x100 += 100; }; // rounding	// d = 800
	divisor_x100 /= 100;													// d = 8
	br_div.fraction = divisor_x100 & 0xF;									// f = 8
	br_div.mantissa += divisor_x100 / 16; // in case if fraction overflows	// m += 0;

	((USART_TypeDef *)USARTx)->BRR = br_div.val;

};
void Init_USART1_DBG(uint32_t br) {
	RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST; RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	USART_Set_BR(USART1, br);
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
};
void USART_DBG_DataTxGeneric(const void * const USARTx, const char *data, unsigned short msg_len) {
	while (msg_len--) {
		USART_WAIT_TXE(USARTx);
		USART_DATA_TX(USARTx, *data++);
	};//while(*data)
	USART_WAIT_TC(USARTx);
};


// =================================================================================================
// SPI
// =================================================================================================

#define SPI_SCLK_DIV_2			(0<<3)

#define DMA_CHAN(x)				(DMA_SxCR_CHSEL & ((x)<<25)) // 0x0E000000
#define SPI1_DMA_CHAN			DMA_CHAN(3) // Ref.: STM32F401 Reference manual, sect.9.3.3, p.170
#define SPI1_TX_GUARD_EN 1

void SPI1_Init(const uint8_t prescaler) {

	RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST; RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	SPI1->CR1 = SPI_CR1_MSTR |	// BIT_SET => Master configuration
				SPI_CR1_SSI |	// BIT_SET => internal slave mgmt (required for master)
				SPI_CR1_SSM |	// BIT_SET => When the SSM bit is set, the NSS pin input is replaced with the value from the SSI bit. (required for master)
				SPI_CR1_SPE |	// BIT_SET => SPI enabled
				prescaler;
};
uint8_t SPI1_ByteRx(void)      {
	while(SPI1_TX_GUARD_EN && !(SPI1->SR&SPI_SR_TXE)) { __NOP(); };
	SPI1->DR = 0;
	while((SPI1->SR&SPI_SR_RXNE) == 0) { __NOP(); }; // NOTE: while(SPI1->SR&SPI_SR_BSY) doesn't really work here as expected, since it takes some time to shift data to the register, so immediate reading of this reg returns immediatelly resulting in previous data to be returned.
	//printf_dbg("SPI>%02X\r\n", SPI1->DR);
  return SPI1->DR;
};
void SPI1_ByteTx(uint8_t data) {
	//printf_dbg("SPI<%02X\r\n", data);
	while(SPI1_TX_GUARD_EN && !(SPI1->SR&SPI_SR_TXE)) { __NOP(); };
	SPI1->DR = data;
	while((SPI1->SR&SPI_SR_RXNE) == 0) { __NOP(); };
	(void)SPI1->DR;
//	printf_dbg("SPI>%02X\r\n", SPI1->DR);
};
uint8_t SPI1_ByteTRx(uint8_t data) {
	//printf_dbg("SPI«%02X\r\n", data);
	while(SPI1_TX_GUARD_EN && !(SPI1->SR & SPI_SR_TXE)) { __NOP(); };
	SPI1->DR = data;
	while((SPI1->SR & SPI_SR_RXNE) == 0) { __NOP(); };
	data = SPI1->DR;
	//printf_dbg("SPI»%02X\r\n", data);
	return data;
};

// =================================================================================================
// BSD
// =================================================================================================

uint8_t board_init(void) {

	SystemCoreClockUpdate(); // optional
	NVIC_SetPriorityGrouping(3); // optional

	// Init GPIOs clocking
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN);

	Init_GPIO_AF(LED_BLUE, (GPIO_MODE_OUT_PP | GPIO_CLK_LOW), 0);

	Init_USART1_DBG(USART1_BR);
	Init_GPIO_AF(USART1_PIN_TX, (GPIO_MODE_AF_PP | GPIO_CLK_MAX),	GPIO_AF_USART1);
	Init_GPIO_AF(USART1_PIN_RX, (GPIO_MODE_AF_OD | GPIO_CLK_MAX),	GPIO_AF_USART1);

	SPI1_Init(SPI_SCLK_DIV_2);
	Init_GPIO_AF(W25QXX_PIN_DI_IO0, (GPIO_MODE_AF_PP | GPIO_CLK_MAX), GPIO_AF_SPI1);
	Init_GPIO_AF(W25QXX_PIN_DO_IO1, (GPIO_MODE_AF_OD | GPIO_CLK_MAX), GPIO_AF_SPI1);
	Init_GPIO_AF(W25QXX_PIN_SCK,    (GPIO_MODE_AF_PP | GPIO_CLK_MAX), GPIO_AF_SPI1);
	Init_GPIO_AF(W25QXX_PIN_CS,     (GPIO_MODE_OUT_PP| GPIO_CLK_MAX), GPIO_PF);

//	w25qxx_init_io();

	return W25QXX_RET_SUCCESS;
};
