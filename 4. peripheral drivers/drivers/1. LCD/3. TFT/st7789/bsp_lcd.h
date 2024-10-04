
#ifndef __BSP_LCD_H__
#define __BSP_LCD_H__

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    volatile uint16_t REG;
    volatile uint16_t RAM;
} st7789_address_t;


#define ST7789_ADDR_POS             23UL
#define ST7789_BASE                 (0x60000000UL | ((1UL << (ST7789_ADDR_POS + 1UL))) -  2UL)
#define ST7789                      ((st7789_address_t *)ST7789_BASE)


#define LCD_HEIGHT                  320             
#define LCD_WIDTH                   240


/* LCD Color -----------------------------------------------------------------*/

#define LCD_COLOR_BLUE                0x001F
#define LCD_COLOR_GREEN               0x07E0
#define LCD_COLOR_RED                 0xF800
#define LCD_COLOR_CYAN                0x07FF
#define LCD_COLOR_MAGENTA             0xF81F
#define LCD_COLOR_YELLOW              0xFFE0
#define LCD_COLOR_LIGHTBLUE           0x841F
#define LCD_COLOR_LIGHTGREEN          0x87F0
#define LCD_COLOR_LIGHTRED            0xFC10
#define LCD_COLOR_LIGHTMAGENTA        0xFC1F
#define LCD_COLOR_LIGHTYELLOW         0xFFF0
#define LCD_COLOR_DARKBLUE            0x0010
#define LCD_COLOR_DARKGREEN           0x0400
#define LCD_COLOR_DARKRED             0x8000
#define LCD_COLOR_DARKCYAN            0x0410
#define LCD_COLOR_DARKMAGENTA         0x8010
#define LCD_COLOR_DARKYELLOW          0x8400
#define LCD_COLOR_WHITE               0xFFFF
#define LCD_COLOR_LIGHTGRAY           0xD69A
#define LCD_COLOR_GRAY                0x8410
#define LCD_COLOR_DARKGRAY            0x4208
#define LCD_COLOR_BLACK               0x0000
#define LCD_COLOR_BROWN               0xA145
#define LCD_COLOR_ORANGE              0xFD20

#define LCD_DEFAULT_COLOR             LCD_COLOR_BLACK


static inline void st7789_write_cmd(uint16_t cmd)
{
    ST7789->REG = cmd;
}

static inline void st7789_write_data(uint16_t data)
{
    ST7789->RAM = data;
}

static inline uint16_t st7789_read_data(void) {
    return ST7789->RAM;
}


static inline void st7789_write_register(uint16_t reg,uint16_t reg_val)
{
    ST7789->REG = reg;
    ST7789->RAM = reg_val;
}

static inline uint16_t st7789_read_register(uint16_t reg) {
    ST7789->REG = reg;
    return ST7789->RAM;
}

/**
 * @brief ST7789V Command Table 1
 * 
 */
#define ST7789_CMD_NOP               0x00  // No operation
#define ST7789_CMD_SWRESET           0x01  // Software reset
#define ST7789_CMD_RDDID             0x04  // Read display ID
#define ST7789_CMD_RDDST             0x09  // Read display status
#define ST7789_CMD_RDDPM             0x0a  // Read display power
#define ST7789_CMD_RDDMADCTL         0x0b  // Read display
#define ST7789_CMD_RDDCOLMOD         0x0c  // Read display pixel
#define ST7789_CMD_RDDIM             0x0d  // Read display image
#define ST7789_CMD_RDDSM             0x0e  // Read display signal
#define ST7789_CMD_RDDSDR            0x0f  // Read display self-diagnostic result
#define ST7789_CMD_SLPIN             0x10  // Sleep in
#define ST7789_CMD_SLPOUT            0x11  // Sleep out
#define ST7789_CMD_PTLON             0x12  // Partial mode on
#define ST7789_CMD_NORON             0x13  // Partial off (Normal)
#define ST7789_CMD_INVOFF            0x20  // Display inversion off
#define ST7789_CMD_INVON             0x21  // Display inversion on
#define ST7789_CMD_GAMSET            0x26  // Gamma set
#define ST7789_CMD_DISPOFF           0x28  // Display off
#define ST7789_CMD_DISPON            0x29  // Display on
#define ST7789_CMD_CASET             0x2a  // Column address set
#define ST7789_CMD_RASET             0x2b  // Row address set
#define ST7789_CMD_RAMWR             0x2c  // Memory write
#define ST7789_CMD_RAMRD             0x2e  // Memory read
#define ST7789_CMD_PTLAR             0x30  // Partial start/end address set
#define ST7789_CMD_VSCRDEF           0x33  // Vertical scrolling definition
#define ST7789_CMD_TEOFF             0x34  // Tearing line effect off
#define ST7789_CMD_TEON              0x35  // Tearing line effect on
#define ST7789_CMD_MADCTL            0x36  // Memory data access control
#define ST7789_CMD_VSCRSADD          0x37  // Vertical address scrolling
#define ST7789_CMD_IDMOFF            0x38  // Idle mode off
#define ST7789_CMD_IDMON             0x39  // Idle mode on
#define ST7789_CMD_COLMOD            0x3a  // Interface pixel format
#define ST7789_CMD_RAMWRC            0x3c  // Memory write continue
#define ST7789_CMD_RAMRDC            0x3e  // Memory read continue
#define ST7789_CMD_TESCAN            0x44  // Set tear scanline
#define ST7789_CMD_RDTESCAN          0x45  // Get scanline
#define ST7789_CMD_WRDISBV           0x51  // Write display brightness
#define ST7789_CMD_RDDISBV           0x52  // Read display brightness value
#define ST7789_CMD_WRCTRLD           0x53  // Write CTRL display
#define ST7789_CMD_RDCTRLD           0x54  // Read CTRL value display
#define ST7789_CMD_WRCACE            0x55  // Write content adaptive brightness control and Color enhancemnet
#define ST7789_CMD_RDCABC            0x56  // Read content adaptive brightness control
#define ST7789_CMD_WRCABCMB          0x5e  // Write CABC minimum brightness
#define ST7789_CMD_RDCABCMB          0x5f  // Read CABC minimum brightness
#define ST7789_CMD_RDABCSDR          0x68  // Read Automatic Brightness Control Self-Diagnostic Result
#define ST7789_CMD_RDID1             0xda  // Read ID1
#define ST7789_CMD_RDID2             0xdb  // Read ID2
#define ST7789_CMD_RDID3             0xdc  // Read ID3

/**
 * @brief ST7789V Command Table 2
 * 
 */
#define ST7789_CMD_RAMCTRL           0xb0  // RAM Control
#define ST7789_CMD_RGBCTRL           0xb1  // RGB Control
#define ST7789_CMD_PORCTRL           0xb2  // Porch control
#define ST7789_CMD_FRCTRL1           0xb3  // Frame Rate Control 1
#define ST7789_CMD_GCTRL             0xb7  // Gate control
#define ST7789_CMD_DGMEN             0xba  // Digital Gamma Enable
#define ST7789_CMD_VCOMS             0xbb  // VCOM Setting
#define ST7789_CMD_LCMCTRL           0xc0  // LCM Control
#define ST7789_CMD_IDSET             0xc1  // ID Setting
#define ST7789_CMD_VDVVRHEN          0xc2  // VDV and VRH Command enable
#define ST7789_CMD_VRHS              0xc3  // VRH Set
#define ST7789_CMD_VDVSET            0xc4  // VDV Setting
#define ST7789_CMD_VCMOFSET          0xc5  // VCOM Offset Set
#define ST7789_CMD_FRCTR2            0xc6  // FR Control 2
#define ST7789_CMD_CABCCTRL          0xc7  // CABC Control
#define ST7789_CMD_REGSEL1           0xc8  // Register value selection 1
#define ST7789_CMD_REGSEL2           0xca  // Register value selection 2
#define ST7789_CMD_PWMFRSEL          0xcc  // PWM Frequency Selection
#define ST7789_CMD_PWCTRL1           0xd0  // Power Control 1
#define ST7789_CMD_VAPVANEN          0xd2  // Enable VAP/VAN signal output
#define ST7789_CMD_CMD2EN            0xdf  // Command 2 Enable
#define ST7789_CMD_PVGAMCTRL         0xe0  // Positive Voltage Gamma Control
#define ST7789_CMD_NVGAMCTRL         0xe1  // Negative voltage Gamma Control
#define ST7789_CMD_DGMLUTR           0xe2  // Digital Gamma Look-up Table for Red
#define ST7789_CMD_DGMLUTB           0xe3  // Digital Gamma Look-up Table for Blue
#define ST7789_CMD_GATECTRL          0xe4  // Gate control
#define ST7789_CMD_PWCTRL2           0xe8  // Power Control 2
#define ST7789_CMD_EQCTRL            0xe9  // Equalize Time Control
#define ST7789_CMD_PROMCTRL          0xec  // Program Control
#define ST7789_CMD_PROMEN            0xfa  // Program Mode Enable
#define ST7789_CMD_NVMSET            0xfc  // NVM Setting
#define ST7789_CMD_PROMACT           0xfe  // Program Action








void bsp_lcd_init(void);

void st7789_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void st7789_draw_point(uint16_t x, uint16_t y, uint16_t color);

void st7789_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *color_buf);

void st7789_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

void st7789_set_window_xy(uint16_t x, uint16_t y, uint16_t xe, uint16_t ye);

void st7789_dma_write(uint16_t *pbuf, uint16_t len);

bool st7789_wait_dma_complete(void);

void st7789_read_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *color_buf);


#endif /* GD32F303C_LCD_EVAL_H */



