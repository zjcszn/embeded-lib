#ifndef __NT35510_H__
#define __NT35510_H__

#include <stdint.h>
#include "fonts.h"
#include "stm32f4xx_ll_gpio.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __IO
#define __IO volatile
#endif

/* Export Type Definetion ----------------------------------------------------*/

typedef enum {
    L2R_U2D = 0,
    L2R_D2U = 1,
    R2L_U2D = 2,
    R2L_D2U = 3,
    U2D_L2R = 4,
    U2D_R2L = 5,
    D2U_L2R = 6,
    D2U_R2L = 7,
} NT35510_ScanDirEnum;



typedef enum {
    NT35510_DISP_PORTRAIT  = 0,
    NT35510_DISP_LANDSCAPE = 1,
} NT35510_DispDirEnum;



typedef struct {
  __IO uint16_t REG;
  __IO uint16_t RAM;
} NT35510_AddressTypedef;



typedef struct {
  uint16_t  width;
  uint16_t  height;
  uint16_t  id;
  uint8_t   direction;
  uint16_t  textcolor;
  uint16_t  backcolor;
} NT35510_ParamsTypedef;


/* Export Macro Definetion ---------------------------------------------------*/

// Use log
#define NT35510_USE_LOG               1

// If define 1, nt35510 init function will update gamma config
#define NT35510_UPDATE_GAMMA          1

// Gamma config group
#define NT35510_GAMMA_CFG_GROUP       1

/* Macro definetion of NT35510 Memory Address*/
/* FSMC Memory address
  - Bank1 (NE1) 0x60000000
  - Bank2 (NE2) 0x64000000
  - Bank3 (NE3) 0x68000000
  - Bank4 (NE4) 0x6C000000
  - REGSELECT_BIT: if example A18 pin -> 18 
*/
#define NT35510_ADDR_BASE             0x6C000000UL
#define NT35510_REGSELECT_BIT         6                     
#define NT35510_ADDR                  (NT35510_ADDR_BASE | ((1UL << (NT35510_REGSELECT_BIT + 1UL))) -  2UL)
#define NT35510                       ((NT35510_AddressTypedef *)NT35510_ADDR)

/* Width and Height in Portrait mode */
#define NT35510_WIDTH                 ((uint16_t)480)     /* LCD PIXEL WIDTH   */
#define NT35510_HEIGHT                ((uint16_t)800)     /* LCD PIXEL HEIGHT  */

#define NT35510_BACKLIGHT_PIN         LL_GPIO_PIN_15
#define NT35510_BACKLIGHT_PORT        GPIOB

/**
 * @brief lcd display backlight power on
 * 
 */
static inline void NT35510_BacklightON(void) {
  LL_GPIO_SetOutputPin(NT35510_BACKLIGHT_PORT, NT35510_BACKLIGHT_PIN);
}

/**
 * @brief lcd display backlight power off
 * 
 */
static inline void NT35510_BacklightOFF(void) {
  LL_GPIO_ResetOutputPin(NT35510_BACKLIGHT_PORT, NT35510_BACKLIGHT_PIN);
}

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


/* NT35510 Export Function Declaration ---------------------------------------*/

uint16_t NT35510_ReadID(void);
uint16_t NT35510_ReadPoint(uint16_t x, uint16_t y);

void NT35510_Init(NT35510_DispDirEnum disp_dir);
void NT35510_DisplayON(void);
void NT35510_DisplayOFF(void);

void NT35510_SetWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void NT35510_SetCursor(uint16_t x, uint16_t y);
void NT35510_SetCursorEnd(uint16_t x, uint16_t y);
void NT35510_SetTextColor(uint16_t text_color, uint16_t back_color);
void NT35510_SetScanDirection(NT35510_ScanDirEnum scan_dir);
void NT35510_SetDispDirection(NT35510_DispDirEnum disp_dir);


void NT35510_Clear(uint16_t color);
void NT35510_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void NT35510_DrawLine(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);
void NT35510_DrawCircle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);
void NT35510_DrawCircleFill(uint16_t x, uint16_t y, uint16_t r, uint16_t color);
void NT35510_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void NT35510_DrawRectangleFill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void NT35510_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font);
void NT35510_WriteString(uint16_t x, uint16_t y, char *str, FontDef font);
void NT35510_DrawRGBImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *buffer);
void NT35510_ColorFill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *color_p);
void NT35510_BacklightSet(uint8_t pwm);


#ifdef __cplusplus
}
#endif

#endif /* __NT35510_H */

