//
// Created by zjcszn on 2024/2/26.
//

#ifndef _W5500_PORT_H_
#define _W5500_PORT_H_

#define W5500_GPIO_PORT GPIOA

#define W5500_PIN_RST   LL_GPIO_PIN_3
#define W5500_PIN_SCS   LL_GPIO_PIN_4
#define W5500_PIN_SCLK  LL_GPIO_PIN_5
#define W5500_PIN_MISO  LL_GPIO_PIN_6
#define W5500_PIN_MOSI  LL_GPIO_PIN_7

#define DEFAULT_MAC_ADDR    {0x00,0xf1,0xbe,0xc4,0xa1,0x05}
#define DEFAULT_IP_ADDR     {192,168,137,136}
#define DEFAULT_SUB_MASK    {255,255,255,0}
#define DEFAULT_GW_ADDR     {192,168,137,1}
#define DEFAULT_DNS_ADDR    {8,8,8,8}

int w5500_init(void);

#endif //_W5500_PORT_H_
