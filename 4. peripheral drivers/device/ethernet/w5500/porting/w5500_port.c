//
// Created by zjcszn on 2024/2/26.
//
#define LOG_TAG "ETH"
#define LOG_LVL ELOG_LVL_INFO

#include <string.h>
#include <stdio.h>
#include "w5500_port.h"
#include "elog.h"
#include "stm32h7xx.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_bus.h"
#include "perf_counter.h"

#include "bsp_spi.h"

#include "w5500.h"

static uint8_t w5500_spi_readbyte(void) {
    uint8_t tmp = 0;
    spi_master_read(&tmp, 1);
    return tmp;
}

static void w5500_spi_writebyte(uint8_t wb) {
    spi_master_write(&wb, 1);
}

static void w5500_spi_readburst(uint8_t *pBuf, uint16_t len) {
    spi_master_read(pBuf, len);
}

static void w5500_spi_writeburst(uint8_t *pBuf, uint16_t len) {
    spi_master_write(pBuf, len);
}

static void w5500_cs_select(void) {
    LL_GPIO_ResetOutputPin(W5500_GPIO_PORT, W5500_PIN_SCS);
}

static void w5500_cs_deselect(void) {
    LL_GPIO_SetOutputPin(W5500_GPIO_PORT, W5500_PIN_SCS);
}

static void w5500_io_init(void) {
    LL_GPIO_InitTypeDef gpio_init = {0};
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Speed = LL_GPIO_SPEED_LOW;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    gpio_init.Pin = W5500_PIN_RST;
    LL_GPIO_Init(W5500_GPIO_PORT, &gpio_init);
}

static void w5500_hard_reset(void) {
    LL_GPIO_ResetOutputPin(W5500_GPIO_PORT, W5500_PIN_RST);
    delay_ms(50);
    LL_GPIO_SetOutputPin(W5500_GPIO_PORT, W5500_PIN_RST);
    delay_ms(10);
}

static int w5500_chip_init(void) {
    /* default size is 2KB */
    
    return wizchip_init(NULL, NULL);
}

/**
 * @brief   set phy config if autonego is disable
 * @param   none
 * @return  none
 */
static void w5500_phy_init(void) {
#ifdef USE_AUTONEGO
    // no thing to do
#else
    wiz_PhyConf conf = {
        .by = PHY_CONFBY_SW,
        .mode = PHY_MODE_MANUAL,
        .speed = PHY_SPEED_100,
        .duplex = PHY_DUPLEX_FULL,
    };
    
    wizphy_setphyconf(&conf);
#endif
}

/**
 * @brief   initializes the network infomation
 * @param   none
 * @return  none
 */
static void w5500_network_info_init(void) {
    wiz_NetInfo info = {
        .mac =  DEFAULT_MAC_ADDR,
        .ip = DEFAULT_IP_ADDR,
        .sn = DEFAULT_SUB_MASK,
        .gw = DEFAULT_GW_ADDR,
        .dns = DEFAULT_DNS_ADDR,
    };

#ifdef USE_DHCP
    info.dhcp = NETINFO_DHCP;
#else
    info.dhcp = NETINFO_STATIC;
#endif
    
    wizchip_setnetinfo(&info);
}

/**
 * @brief   read and show the network infomation
 * @param   none
 * @return  none
 */
void w5500_network_info_show(void) {
    wiz_NetInfo info;
    
    wizchip_getnetinfo(&info);
    
    printf("w5500 network infomation:\r\n");
    printf("  -mac:%02X:%02X:%02X:%02X:%02X:%02X\r\n", info.mac[0], info.mac[1], info.mac[2],
           info.mac[3], info.mac[4], info.mac[5]);
    printf("  -ip:%d.%d.%d.%d\r\n", info.ip[0], info.ip[1], info.ip[2], info.ip[3]);
    printf("  -sn:%d.%d.%d.%d\r\n", info.sn[0], info.sn[1], info.sn[2], info.sn[3]);
    printf("  -gw:%d.%d.%d.%d\r\n", info.gw[0], info.gw[1], info.gw[2], info.gw[3]);
    printf("  -dns:%d.%d.%d.%d\r\n", info.dns[0], info.dns[1], info.dns[2], info.dns[3]);
    printf("  -dhcp_mode: %s\r\n", info.dhcp == NETINFO_DHCP ? "dhcp" : "static");
}

/**
 * @brief   w5500 init
 * @param   none
 * @return  errcode
 * @retval  0   success
 * @retval  -1  chip init fail
 */
int w5500_init(void) {
    /* W5500 hard reset */
    w5500_io_init();
    w5500_hard_reset();
    
    /* Register spi driver function */
//    reg_wizchip_cris_cbfunc(w5500_cris_enter, w5500_cris_exit);
    reg_wizchip_cs_cbfunc(w5500_cs_select, w5500_cs_deselect);
    reg_wizchip_spi_cbfunc(w5500_spi_readbyte, w5500_spi_writebyte);
    reg_wizchip_spiburst_cbfunc(w5500_spi_readburst, w5500_spi_writeburst);
    
    /* socket buffer size init */
    if (w5500_chip_init() != 0) {
        return -1;
    }
    
    /* phy init */
    w5500_phy_init();
    
    /* network infomation init */
    w5500_network_info_init();
    
    /* show network infomation */
    w5500_network_info_show();
    
    return 0;
}



