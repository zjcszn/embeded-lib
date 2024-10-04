#ifndef _GOODIX_GT9XX_H_
#define _GOODIX_GT9XX_H_


/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_gpio.h"
#include "bsp_i2c_sw.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported macro ------------------------------------------------------------*/

// Use Log
#define GOODIX_USE_LOG                  1

// If define 1, output touch point info by log
#define GOODIX_LOG_TOUCHPOINT            1

// If define 1, goodix init function will overwrite chip config registers value
#define GOODIX_CFG_OVERWRITE            0

// If define 1, will overwrite all config registers by GOODIX_CFG_GROUP
// If define 0, will overwrite only x_resolution, y_esolution, touch_number and module_switch1 register 
#define GOODIX_CFG_USE_CFG_GROUP        0

#define GOODIX_CFG_GROUP                GOODIX_CFG_GROUP1


// If define 1, goodix init function will print old chip config registers value
#define GOODIX_CFG_PRINT                0

// Goodix Chip ID
#define GOODIX_PRODUCT_ID                GOODIX_PRODUCT_ID_911
#define GOODIX_PRODUCT_ID_911            0x39313100

// Goodix config register number
#define GOODIX_CFG_LENGTH                GOODIX_CFG_LENGTH_911
#define GOODIX_CFG_LENGTH_911            186
#define GOODIX_CFG_LENGTH_967            228

// Goodix ms delay function
#define Goodix_Delay(ms)                HAL_Delay(ms)

// Goodix I2C config
#define CTP_I2C_FREQ                     400    // i2c freq

// Goodix I2C sda pin definition
#define CTP_SDA_PORT                    GPIOF
#define CTP_SDA_PIN                      LL_GPIO_PIN_11

// Goodix I2C scl pin definition
#define CTP_SCL_PORT                    GPIOB
#define CTP_SCL_PIN                      LL_GPIO_PIN_0

// Goodix INT pin definition
#define CTP_INT_PORT                    GPIOB
#define CTP_INT_PIN                      LL_GPIO_PIN_1

// Goodix RST pin definition
#define CTP_RST_PORT                    GPIOC
#define CTP_RST_PIN                      LL_GPIO_PIN_13

// INT triggering mechanism
#define GOODIX_INT_TRIGGER              0    // 0 rising edge | 1 falling edge | 2 low level | 3 high level

// Software noise reduction
#define GOODIX_SOFT_NOISE_REDUCTION      1

// Goodix register address
#define GOODIX_ADDRESS                  0x5D

// Write only registers
#define GOODIX_REG_COMMAND              0x8040
#define GOODIX_REG_ESD_CHECK            0x8041
#define GOODIX_REG_PROXIMITY_EN         0x8042

// Read / write registers

// The version number of the configuration file
#define GOODIX_REG_CONFIG_DATA          0x8047

// X output maximum value (LSB 2 bytes)
#define GOODIX_REG_MAX_X                0x8048

// Y output maximum value (LSB 2 bytes)
#define GOODIX_REG_MAX_Y                0x804A

// Maximum number of output contacts: 1~5 (4 bit value 3:0, 7:4 is reserved)
#define GOODIX_REG_MAX_TOUCH            0x804C

// Module switch 1
// 7:6 Reserved, 5:4 Stretch rank, 3 X2Y, 2 SITO (Single sided ITO touch screen), 1:0 INT Trigger mode
#define GOODIX_REG_MOD_SW1              0x804D

// Module switch 2
// 7:1 Reserved, 0 Touch key
#define GOODIX_REG_MOD_SW2              0x804E

// Number of debuffs fingers press/release
#define GOODIX_REG_SHAKE_CNT            0x804F

// X threshold
#define GOODIX_REG_X_THRESHOLD          0x8057

// Configuration update fresh
#define GOODIX_REG_CONFIG_FRESH         0x8100

// ReadOnly registers (device and coordinates info)

// Product ID (LSB 4 bytes, GT9110: 0x06 0x00 0x00 0x09)
#define GOODIX_REG_ID                   0x8140

// Firmware version (LSB 2 bytes)
#define GOODIX_REG_FW_VER               0x8144

// Current output X resolution (LSB 2 bytes)
#define GOODIX_READ_X_RES               0x8146

// Current output Y resolution (LSB 2 bytes)
#define GOODIX_READ_Y_RES               0x8148

// Module vendor ID
#define GOODIX_READ_VENDOR_ID           0x814A

// Read coordinates
// 7 Buffer status(1 is ready for read), 6 Large detect, 4 HaveKey, 3:0 num of touch points
#define GOODIX_READ_COORD_ADDR          0x814E

#define GOODIX_POINT1_X_ADDR             0x8150
#define GOODIX_POINT1_Y_ADDR             0x8152

/* Commands for REG_COMMAND */

// 0: read coordinate state
#define GOODIX_CMD_READ                 0x00
// 1: difference value original value
#define GOODIX_CMD_DIFFVAL              0x01
// 2: software reset
#define GOODIX_CMD_SOFTRESET            0x02
// 3: Baseline update
#define GOODIX_CMD_BASEUPDATE           0x03
// 4: Benchmark calibration
#define GOODIX_CMD_CALIBRATE            0x04
// 5: Off screen (send other invalid)
#define GOODIX_CMD_SCREEN_OFF           0x05

/* When data needs to be sent, the host sends command 0x21 to GT9x,
 * enabling GT911 to enter "Approach mode" and work as a transmitting terminal */
#define GOODIX_CMD_HOTKNOT_TX           0x21

// Goodix default Params
#define GOODIX_DEFAULT_MAX_X            480    /* default coordinate max values */
#define GOODIX_DEFAULT_MAX_Y            800



#define GOODIX_CFG_GROUP1  {                                  \
0x82, 0xE0, 0x01, 0x20, 0x03, 0x05, 0xF5, 0x00, 0x01, 0x08, \
0x1E, 0x05, 0x3C, 0x3C, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x1A, 0x1C, 0x1E, 0x14, 0x87, 0x29, 0x0A, \
0x2A, 0x28, 0xEB, 0x04, 0x00, 0x00, 0x01, 0x61, 0x03, 0x2C, \
0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x14, 0x3C, 0x94, 0xC5, 0x02, 0x08, 0x00, 0x00, 0x04, \
0xB7, 0x16, 0x00, 0x9F, 0x1B, 0x00, 0x8B, 0x22, 0x00, 0x7B, \
0x2B, 0x00, 0x70, 0x36, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, \
0x12, 0x14, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x1D, \
0x1E, 0x1F, 0x20, 0x21, 0x22, 0x24, 0x26, 0x28, 0xFF, 0xFF, \
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x7F, 0x00,                          \
}                          

#define GOODIX_CFG_GROUP2 {                                  \
0x41, 0xE0, 0x01, 0x20, 0x03, 0x01, 0xC4, 0x00, 0x01, 0x08, \
0x28, 0x08, 0x5A, 0x46, 0x03, 0x05, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x18, 0x1A, 0x1E, 0x14, 0x89, 0x2A, 0x09, \
0x57, 0x5C, 0xB5, 0x06, 0x00, 0x00, 0x00, 0x02, 0x01, 0x1D, \
0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x46, 0x82, 0x94, 0xC5, 0x02, 0x07, 0x00, 0x00, 0x04, \
0x96, 0x4A, 0x00, 0x85, 0x54, 0x00, 0x77, 0x5F, 0x00, 0x6A, \
0x6C, 0x00, 0x5F, 0x7A, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x10, 0x12, \
0x14, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0F, 0x10, \
0x12, 0x16, 0x18, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, \
0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0xCD, 0x00,                          \
}

/* Exported types ------------------------------------------------------------*/

typedef enum{
  GOODIX_OK, 
  GOODIX_ERROR,
  GOODIX_NOACK,
  GOODIX_NTOUCH,
} Goodix_ErrorTypedef;

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t size;
} Goodix_PointTypedef;

typedef struct{
  uint16_t X_Resolution;
  uint16_t Y_Resolution;
  uint8_t  Num_Of_Touch;
  uint8_t  ReverseX;
  uint8_t  ReverseY;
  uint8_t  SwithX2Y;
} Goodix_ConfigTypedef;


/* Exported function declaration ---------------------------------------------*/

int Goodix_Init(Goodix_ConfigTypedef *config);
int Goodix_ReadTouch(Goodix_PointTypedef *cordinate, uint8_t *number_of_cordinate);


#ifdef __cplusplus
}
#endif


#endif