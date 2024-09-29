
#include "bsp_lcd.h"
#include "systick.h"

#include "main.h"
#include "stdio.h"

#define  LCD_RST_L()             gpio_bit_reset(GPIOB, GPIO_PIN_11)
#define  LCD_RST_H()             gpio_bit_set(GPIOB, GPIO_PIN_11)

static volatile uint8_t st7789_dmatc_flag = 0;

void bsp_exmc_init(void)
{
    exmc_norsram_parameter_struct lcd_init_struct;
    exmc_norsram_timing_parameter_struct lcd_timing_init_struct;
    dma_parameter_struct dma_init_struct;

    /* EXMC clock enable */
    rcu_periph_clock_enable(RCU_EXMC);
    rcu_periph_clock_enable(RCU_DMA0);

    /* GPIO clock enable */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE);

    /* backlight */
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_1);

    /* configure EXMC_D[0~15]*/
    /* PD14(EXMC_D0), PD15(EXMC_D1),PD0(EXMC_D2), PD1(EXMC_D3), PD8(EXMC_D13), PD9(EXMC_D14), PD10(EXMC_D15) */
    gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1| GPIO_PIN_8 | GPIO_PIN_9 |
                                                         GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15);

    /* PE7(EXMC_D4), PE8(EXMC_D5), PE9(EXMC_D6), PE10(EXMC_D7), PE11(EXMC_D8), PE12(EXMC_D9), 
       PE13(EXMC_D10), PE14(EXMC_D11), PE15(EXMC_D12) */
    gpio_init(GPIOE, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | 
                                                         GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | 
                                                         GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    /* configure PE2(EXMC_A23),  LCD_RS */
    gpio_init(GPIOE, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    
    /* configure NOE and NWE */
    gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5);

    /* configure EXMC NE0, LCD_CS */
    gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);

    /* configure LCD_RST, PB11 */
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    gpio_bit_set(GPIOB, GPIO_PIN_11);

    lcd_timing_init_struct.asyn_access_mode = EXMC_ACCESS_MODE_A;
    lcd_timing_init_struct.syn_data_latency = EXMC_DATALAT_2_CLK;
    lcd_timing_init_struct.syn_clk_division = EXMC_SYN_CLOCK_RATIO_DISABLE;
    lcd_timing_init_struct.bus_latency = 2;
    lcd_timing_init_struct.asyn_data_setuptime = 10;
    lcd_timing_init_struct.asyn_address_holdtime = 2;
    lcd_timing_init_struct.asyn_address_setuptime = 5;

    lcd_init_struct.norsram_region = EXMC_BANK0_NORSRAM_REGION0;
    lcd_init_struct.write_mode = EXMC_ASYN_WRITE;
    lcd_init_struct.extended_mode = DISABLE;
    lcd_init_struct.asyn_wait = DISABLE;
    lcd_init_struct.nwait_signal = DISABLE;
    lcd_init_struct.memory_write = ENABLE;
    lcd_init_struct.nwait_config = EXMC_NWAIT_CONFIG_BEFORE;
    lcd_init_struct.wrap_burst_mode = DISABLE;
    lcd_init_struct.nwait_polarity = EXMC_NWAIT_POLARITY_LOW;
    lcd_init_struct.burst_mode = DISABLE;
    lcd_init_struct.databus_width = EXMC_NOR_DATABUS_WIDTH_16B;
    lcd_init_struct.memory_type = EXMC_MEMORY_TYPE_SRAM;
    lcd_init_struct.address_data_mux = DISABLE;
    lcd_init_struct.read_write_timing = &lcd_timing_init_struct;
    lcd_init_struct.write_timing = &lcd_timing_init_struct;

    exmc_norsram_init(&lcd_init_struct);

    exmc_norsram_enable(EXMC_BANK0_NORSRAM_REGION0);

    dma_deinit(DMA0, DMA_CH0);
    
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.periph_addr = (uint32_t)&(ST7789->RAM);
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH0, &dma_init_struct);

    dma_circulation_disable(DMA0, DMA_CH0);
    dma_memory_to_memory_enable(DMA0, DMA_CH0);
    
    nvic_irq_enable(DMA0_Channel0_IRQn, 0, 0);
}


void bsp_lcd_init(void)
{ 
    bsp_exmc_init();

    LCD_RST_L();
    delay_1ms(20);
    LCD_RST_H();
    delay_1ms(150);

    /* Gate Control */    
    st7789_write_cmd(0XB7);                     
    st7789_write_data(0x35);     

    /* VCOM setting: 0.725v (default 0.9v for 0x20) */
    st7789_write_cmd(0xBB);         
    st7789_write_data(0x19);     

    /* LCMCTRL:  MY | BGR  */
    st7789_write_cmd(0xC0);         
    st7789_write_data (0x60);       

    /* VDV and VRH command Enable */
    st7789_write_cmd (0xC2);        
    st7789_write_data (0x01);       
    st7789_write_data (0xFF);       

    /* VRH set */
    st7789_write_cmd (0xC3);        
    st7789_write_data (0x12);  

    /* VDV set */
    st7789_write_cmd (0xC4);        
    st7789_write_data (0x20);       

    /* Frame rate control in normal mode, 105Hz */
    st7789_write_cmd (0xC6);        
    st7789_write_data (0x02);      

    /* Power control */
    st7789_write_cmd (0xD0);        
    st7789_write_data (0xA4);      
    st7789_write_data (0x81);      

    /* Gamma Set (26h) */
    st7789_write_cmd(0x26);
    st7789_write_data(0x01);

    /* Interface Pixel Format: RGB565 */
    st7789_write_cmd(0x3A);
    st7789_write_data(0x55);

    /* Positive Gamma Correction */
    st7789_write_cmd(0xE0);     //Set Gamma
    st7789_write_data(0x0F);
    st7789_write_data(0x20);
    st7789_write_data(0x1E);
    st7789_write_data(0x09);
    st7789_write_data(0x12);
    st7789_write_data(0x0B);
    st7789_write_data(0x50);
    st7789_write_data(0XBA);
    st7789_write_data(0x44);
    st7789_write_data(0x09);
    st7789_write_data(0x14);
    st7789_write_data(0x05);
    st7789_write_data(0x23);
    st7789_write_data(0x21);
    st7789_write_data(0x00);

    /* Negative Gamma Correction (E1h) */
    st7789_write_cmd(0XE1);  
    st7789_write_data(0x00);
    st7789_write_data(0x19);
    st7789_write_data(0x19);
    st7789_write_data(0x00);
    st7789_write_data(0x12);
    st7789_write_data(0x07);
    st7789_write_data(0x2D);
    st7789_write_data(0x28);
    st7789_write_data(0x3F);
    st7789_write_data(0x02);
    st7789_write_data(0x0A);
    st7789_write_data(0x08);
    st7789_write_data(0x25);
    st7789_write_data(0x2D);
    st7789_write_data(0x0F);

    /* sleep out */
    st7789_write_cmd(0x11);
 
    delay_1ms(120);

    st7789_fill(0, 0, LCD_WIDTH, LCD_HEIGHT, LCD_DEFAULT_COLOR);

    /* display on */
    st7789_write_cmd(ST7789_CMD_DISPON);  

    delay_1ms(100);

    gpio_bit_set(GPIOB, GPIO_PIN_1);
}

void st7789_set_window_xy(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye) {
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT))
        return;

    st7789_write_cmd(ST7789_CMD_CASET);
    st7789_write_data(x >> 8);
    st7789_write_data(x & 0xFF);
    st7789_write_data(xe >> 8);
    st7789_write_data(xe & 0xFF);

    st7789_write_cmd(ST7789_CMD_RASET);
    st7789_write_data(y >> 8);
    st7789_write_data(y & 0xFF);
    st7789_write_data(ye >> 8);
    st7789_write_data(ye & 0xFF);
}

void st7789_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT))
        return;

    uint16_t xe = x + w - 1;
    uint16_t ye = y + h - 1;

    st7789_write_cmd(ST7789_CMD_CASET);
    st7789_write_data(x >> 8);
    st7789_write_data(x & 0xFF);
    st7789_write_data(xe >> 8);
    st7789_write_data(xe & 0xFF);

    st7789_write_cmd(ST7789_CMD_RASET);
    st7789_write_data(y >> 8);
    st7789_write_data(y & 0xFF);
    st7789_write_data(ye >> 8);
    st7789_write_data(ye & 0xFF);
}


void st7789_draw_point(uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT))
        return;

    st7789_write_cmd(ST7789_CMD_CASET);
    st7789_write_data(x >> 8);
    st7789_write_data(x & 0xFF);
    st7789_write_cmd(ST7789_CMD_RASET);
    st7789_write_data(y >> 8);
    st7789_write_data(y & 0xFF);
    st7789_write_cmd(ST7789_CMD_RAMWR);
    st7789_write_data(color);

}

void st7789_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *color_buf) {
    st7789_set_window(x, y, w, h);

    st7789_write_cmd(ST7789_CMD_RAMWR);
    for (int i = 0; i < w * h; i++) {
        st7789_write_data(*color_buf++);
    }
}

void st7789_read_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *color_buf) {
    uint16_t d1, d2, d3;
    st7789_set_window(x, y, w, h);

    st7789_write_cmd(ST7789_CMD_RAMRD);
    st7789_read_data();
    for (int i = 0; i < w * h; i += 2) {
        d1 = st7789_read_data();
        d2 = st7789_read_data();
        d3 = st7789_read_data();

        *color_buf++ =  (d1 & 0xF800) | ((d1 & 0x00FC) << 3) | ((d2 & 0xF800) >> 11);
        *color_buf++ =  ((d2 & 0x00F8) << 8) | ((d3 & 0xFC00) >> 5) | ((d3 & 0x00F8) >> 3);
    }    
}

void st7789_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    st7789_set_window(x, y, w, h);

    st7789_write_cmd(ST7789_CMD_RAMWR);
    for (int i = 0; i < w * h; i++) {
        st7789_write_data(color);
    }
}

void st7789_dma_write(uint16_t *pbuf, uint16_t len) {
    dma_memory_address_config(DMA0, DMA_CH0, (uint32_t)pbuf);
    dma_transfer_number_config(DMA0, DMA_CH0, len);
    dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_FTF);
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_FTF);
    st7789_dmatc_flag = 0;
    dma_channel_enable(DMA0, DMA_CH0);
}

void DMA0_Channel0_IRQHandler(void) {
    if (dma_interrupt_flag_get(DMA0, DMA_CH0, DMA_INT_FLAG_FTF) == SET) {
        dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_FTF);
        // printf("dma complete\n");
        st7789_dmatc_flag = 1;
    }
    dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_G);
    dma_channel_disable(DMA0, DMA_CH0);
}

bool st7789_wait_dma_complete(void) {
    return st7789_dmatc_flag ? true : false;
}