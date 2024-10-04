#ifndef _DRIVER_MISC_H
#define _DRIVER_MISC_H

#define UART1_RX_PORT_GROUP 		GPIOA
#define UART1_TX_PORT_GROUP 		GPIOA
#define UART1_TX_PIN 				GPIO_Pin_9
#define UART1_RX_PIN 				GPIO_Pin_10
#define UART1_TX_PIN_SOURSE		GPIO_PinSource9
#define UART1_RX_PIN_SOURSE		GPIO_PinSource10

#define UART3_RX_PORT_GROUP 		GPIOB
#define UART3_TX_PORT_GROUP 		GPIOB
#define UART3_TX_PIN 				GPIO_Pin_10
#define UART3_RX_PIN 				GPIO_Pin_11
#define UART3_TX_PIN_SOURSE		GPIO_PinSource10
#define UART3_RX_PIN_SOURSE		GPIO_PinSource11


//define DCMI PORT
#define DCMI_HS_PORT_GROUP 	GPIOA
#define DCMI_HS_PIN 				GPIO_Pin_4
#define DCMI_HS_SOURCE 		GPIO_PinSource4

#define DCMI_VS_PORT_GROUP 	GPIOB
#define DCMI_VS_PIN 				GPIO_Pin_7
#define DCMI_VS_SOURCE 		GPIO_PinSource7

#define DCMI_PCLK_PORT_GROUP GPIOA
#define DCMI_PCLK_PIN 			GPIO_Pin_6
#define DCMI_PCLK_SOURCE 		GPIO_PinSource6

#define DCMI_D0_PORT_GROUP 	GPIOC
#define DCMI_D0_PIN 				GPIO_Pin_6
#define DCMI_D0_SOURCE 			GPIO_PinSource6

#define DCMI_D1_PORT_GROUP 	GPIOC
#define DCMI_D1_PIN 				GPIO_Pin_7
#define DCMI_D1_SOURCE 			GPIO_PinSource7

#define DCMI_D2_PORT_GROUP 	GPIOC
#define DCMI_D2_PIN 				GPIO_Pin_8
#define DCMI_D2_SOURCE 			GPIO_PinSource8

#define DCMI_D3_PORT_GROUP 	GPIOC
#define DCMI_D3_PIN 				GPIO_Pin_9
#define DCMI_D3_SOURCE 			GPIO_PinSource9

#define DCMI_D4_PORT_GROUP 	GPIOC
#define DCMI_D4_PIN 				GPIO_Pin_11
#define DCMI_D4_SOURCE 			GPIO_PinSource11

#define DCMI_D5_PORT_GROUP 	GPIOB
#define DCMI_D5_PIN 				GPIO_Pin_6
#define DCMI_D5_SOURCE 			GPIO_PinSource6

#define DCMI_D6_PORT_GROUP 	GPIOB
#define DCMI_D6_PIN 				GPIO_Pin_8
#define DCMI_D6_SOURCE 			GPIO_PinSource8

#define DCMI_D7_PORT_GROUP 	GPIOB
#define DCMI_D7_PIN 				GPIO_Pin_9
#define DCMI_D7_SOURCE 			GPIO_PinSource9
/*
#define DCMI_D8_PORT_GROUP 	GPIOC
#define DCMI_D8_PIN 				GPIO_Pin_10
#define DCMI_D8_SOURCE 			GPIO_PinSource10

#define DCMI_D9_PORT_GROUP 	GPIOC
#define DCMI_D9_PIN 				GPIO_Pin_12
#define DCMI_D9_SOURCE 			GPIO_PinSource12
*/
#define MCO_PORT_GROUP 		GPIOA
#define MCO_PIN 					GPIO_Pin_8

#define DCMI_RST_PORT_GROUP 	GPIOA
#define DCMI_RST_PIN 			GPIO_Pin_12

#define DCMI_PWD_PORT_GROUP 	GPIOA
#define DCMI_PWD_PIN 			GPIO_Pin_11

#define SIO_C_PORT_GROUP 		GPIOC
#define SIO_C_PIN 				GPIO_Pin_14

#define SIO_D_PORT_GROUP 		GPIOC
#define SIO_D_PIN 				GPIO_Pin_13

#define SIO_D_OUT		gpio_cfg((uint32_t)SIO_D_PORT_GROUP, SIO_D_PIN, GPIO_Mode_Out_OD);
#define SIO_D_IN			gpio_cfg((uint32_t)SIO_D_PORT_GROUP, SIO_D_PIN, GPIO_Mode_IPU);

//end define DCMI PORT

#define GPIO_SET(port,pin) (port->BSRRL = pin)
#define GPIO_CLR(port,pin) (port->BSRRH = pin)
#define GPIO_STAT(port,pin)   (!!(port->IDR & pin))


void gpio_cfg(uint32_t group, uint32_t pin, uint8_t mode);
void usr_gpio_init(void);
int check_rst_stat(void);
void driver_misc_init(void);
uint32_t get_random(void);
void driver_gpio_init(void);
void _mem_init(void);

#endif
