#include "gt9xx.h"
#include <stdio.h>
#include <string.h>


/* Private macro -------------------------------------------------------------*/


#if       GOODIX_USE_LOG
#include  <stdio.h>
#define   GOODIX_LOG(...)            printf(__VA_ARGS__)
#else
#define   GOODIX_LOG(...)            ((void)0U)
#endif

#define   MIN(a, b)                  (((a) < (b)) ? (a) : (b)) 


/* Private variables ---------------------------------------------------------*/

// goodix i2c handle
static soft_i2c_dev_t Goodix_I2C = {0};

// goodix config register value buffer
#if GOODIX_CFG_USE_CFG_GROUP
static uint8_t  Goodix_CFG[GOODIX_CFG_LENGTH] = GOODIX_CFG_GROUP;
#else
static uint8_t  Goodix_CFG[GOODIX_CFG_LENGTH] = {0};
#endif

// goodix i2c rx buffer
static uint8_t  Goodix_RxBuffer[10] = {0};

// goodix i2c tx buffer
static uint8_t  Goodix_TxBuffer[ 2] = {0};

/* I2C Operate port -----------------------------------------------------------*/

/**
 * @brief I2C init
 * 
 */
static void Goodix_I2C_Init(void) {
  // add user init code
  Goodix_I2C.dev_addr = GOODIX_ADDRESS;
  Goodix_I2C.scl.port = CTP_SCL_PORT;
  Goodix_I2C.scl.pin  = CTP_SCL_PIN;
  Goodix_I2C.sda.port = CTP_SDA_PORT;
  Goodix_I2C.sda.pin  = CTP_SDA_PIN;
  soft_i2c_init(&Goodix_I2C, CTP_I2C_FREQ);
}

/**
 * @brief Write data to goodix ic
 * 
 * @param wr_data pointer of data to write
 * @param len length of write data
 * @return int GOODIX_OK on write success | GOODIX_ERROR on write error
 */
static int Goodix_I2C_Write(uint8_t *wr_data, int len) {
  if (soft_i2c_master_write(&Goodix_I2C, wr_data, len)) {
    return GOODIX_ERROR;
  }
  return GOODIX_OK;
}

/**
 * @brief Read data from goodix ic
 * 
 * @param rd_data pointer of read buffer
 * @param len length of read data
 * @return int GOODIX_OK on write success | GOODIX_ERROR on write error
 */
static int Goodix_I2C_Read(uint8_t *rd_data, int len) {
  if (soft_i2c_master_read(&Goodix_I2C, rd_data, len)) {
    return GOODIX_ERROR;
  }
  return GOODIX_OK;
}

/**
 * @brief Write data to goodix register
 * 
 * @param reg_addr register address
 * @param wr_data  data to write
 * @param data_len length of read data
 * @return int GOODIX_OK on write success | GOODIX_ERROR on write error
 */
static int Goodix_I2C_MemWrite(uint16_t reg_addr, uint8_t *wr_data, int data_len) {
  if (soft_i2c_master_mem_write(&Goodix_I2C, reg_addr, 2, wr_data, data_len)) {
    return GOODIX_ERROR;
  }
  return GOODIX_OK;
}

/**
 * @brief Read data from goodix register
 * 
 * @param reg_addr register address
 * @param rd_data  pointer of read buffer
 * @param data_len length of read data
 * @return int GOODIX_OK on write success | GOODIX_ERROR on write error
 */
static int Goodix_I2C_MemRead(uint16_t reg_addr, uint8_t *rd_data, int data_len) {
  if (soft_i2c_master_mem_read(&Goodix_I2C, reg_addr, 2, rd_data, data_len)) {
    return GOODIX_ERROR;
  }
  return GOODIX_OK;
}

/* IO Operate port -----------------------------------------------------------*/

/**
 * @brief Set GPIO Input mode
 * 
 * @param gpio GPIO port
 * @param pin GPIO Pin
 */
static void Goodix_Pin_Input(GPIO_TypeDef *gpio, uint32_t pin) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin   = pin;
  GPIO_InitStruct.Mode  = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  LL_GPIO_Init(gpio, &GPIO_InitStruct); 
}

/**
 * @brief Set GPIO Output mode
 * 
 * @param gpio GPIO port
 * @param pin GPIO Pin
 */
static void Goodix_Pin_Output(GPIO_TypeDef *gpio, uint32_t pin) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin   = pin;
  GPIO_InitStruct.Mode  = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  LL_GPIO_Init(gpio, &GPIO_InitStruct); 
}

/**
 * @brief Set GPIO output high level
 * 
 * @param gpio GPIO port
 * @param pin GPIO pin
 */
static inline void Goodix_Pin_SetHigh(GPIO_TypeDef *gpio, uint32_t pin) {
  LL_GPIO_SetOutputPin(gpio, pin);
}

/**
 * @brief Set GPIO output low level
 * 
 * @param gpio GPIO port
 * @param pin GPIO pin
 */
static inline void Goodix_Pin_SetLow(GPIO_TypeDef *gpio, uint32_t pin) {
  LL_GPIO_ResetOutputPin(gpio, pin);
}

/**
 * @brief Set GPIO clock enable
 * 
 * @param gpio GPIO port
 */
static void Goodix_PinClock_Enable(GPIO_TypeDef *gpio) {
  if (gpio == GPIOA) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  else if (gpio == GPIOB) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  else if (gpio == GPIOC) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  else if (gpio == GPIOD) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  else if (gpio == GPIOE) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
  else if (gpio == GPIOF) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  else if (gpio == GPIOG) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
  else if (gpio == GPIOH) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  else if (gpio == GPIOI) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOI);
}


/* Private function declaration ----------------------------------------------*/

static void    Goodix_Reset(void);
static void    Goodix_Hardware_Init(void);
static int     Goodix_Software_Init(Goodix_ConfigTypedef *config);
static int     Goodix_ReadConfig(void);
static int     Goodix_WriteConfig(void);
static int     Goodix_GetProductID(uint32_t *id);
static int     Goodix_WriteCommand(uint8_t cmd);
static int     Goodix_GetStatus(uint8_t *status);
static int     Goodix_SetStatus(uint8_t  status);
static uint8_t Goodix_CalculateCheckSum(void);


/* Private function definition -----------------------------------------------*/

/**
 * @brief Goodix chip init function
 * 
 * @param config pointer of goodix config value
 * @return int GOODIX_OK on init success | GOODIX_ERROR on init error
 */
int Goodix_Init(Goodix_ConfigTypedef *config) {
  Goodix_Hardware_Init();
  return Goodix_Software_Init(config);
}

/**
 * @brief Goodix read touch point function 
 * 
 * @param tp_buf pointer of touch point buffer
 * @param tp_num number  of touch point
 * @return int GOODIX_OK on read success | GOODIX_ERROR on read error
 */
int Goodix_ReadTouch(Goodix_PointTypedef *tp_buf, uint8_t *tp_num) {
  uint8_t status, tpcnt_read;
  
  if (Goodix_GetStatus(&status) != GOODIX_OK) {
    return GOODIX_ERROR;
  }

  if ((status & 0x80) == 0) {
    return GOODIX_NTOUCH;
  }

  tpcnt_read = status & 0x0F;
  *tp_num = MIN(*tp_num, tpcnt_read);

  if (*tp_num) {
    for (int i = 0; i < *tp_num; i++) {
      if (Goodix_I2C_MemRead(GOODIX_POINT1_X_ADDR + (i << 3), Goodix_RxBuffer, 6) != GOODIX_OK) {
        return GOODIX_ERROR;
      }
      tp_buf[i].x    = (Goodix_RxBuffer[1] << 8) | Goodix_RxBuffer[0];
      tp_buf[i].y    = (Goodix_RxBuffer[3] << 8) | Goodix_RxBuffer[2];
      tp_buf[i].size = (Goodix_RxBuffer[5] << 8) | Goodix_RxBuffer[4]; 

      #if GOODIX_LOG_TOUCHPOINT
      GOODIX_LOG("TP: X->%4u   Y->%4u   Size->%4u\r\n", tp_buf[i].x, tp_buf[i].y, tp_buf[i].size);
      #endif
    
    }
  }
  Goodix_SetStatus(0);
  return GOODIX_OK;
}

/**
 * @brief Print goodix config register value
 * 
 * @return int GOODIX_OK on success | GOODIX_ERROR on error
 */
int Goodix_PrintConfigValue(void) {
  uint8_t count = 0;
  uint8_t checksum = 0;

  printf("Goodix Controller Config Registers Value:\r\n");
  for (int i = 0; i < GOODIX_CFG_LENGTH - 1; i++, count++) {
    count %= 10;
    if (count == 0) {
      if (Goodix_I2C_MemRead(GOODIX_REG_CONFIG_DATA + i, Goodix_RxBuffer, 10) != GOODIX_OK) {
        return GOODIX_ERROR;
      }
    }
    checksum += Goodix_RxBuffer[i % 10];
    printf("0x%02X, ", Goodix_RxBuffer[i % 10]);
    if (count == 9) printf("\r\n");
  }
  printf("0x00,\r\n%s\r\n", checksum ? "CheckSum Verify Result: Error" : "CheckSum Verify Result: OK");
  return GOODIX_OK;
}

/**
 * @brief Goodix hardware init
 * 
 */
static void Goodix_Hardware_Init(void) {
  Goodix_PinClock_Enable(CTP_INT_PORT);
  Goodix_PinClock_Enable(CTP_RST_PORT);
  Goodix_Pin_Input(CTP_INT_PORT, CTP_INT_PIN);
  Goodix_Pin_Output(CTP_RST_PORT, CTP_RST_PIN);
  Goodix_I2C_Init();
  GOODIX_LOG("Goodix Controller Hardware Initialize Success\r\n");
}

/**
 * @brief Goodix software init
 * 
 * @return int GOODIX_OK on init success | GOODIX_ERROR on init error
 */
static int Goodix_Software_Init(Goodix_ConfigTypedef *config) {
  // Reset chip
  Goodix_Reset();

  // Reference capacitance calibration (Internal test)
  Goodix_WriteCommand(GOODIX_CMD_CALIBRATE);

  // Read product ID
  uint32_t goodix_id;
  if (Goodix_GetProductID(&goodix_id) != GOODIX_OK || goodix_id != GOODIX_PRODUCT_ID ) {
    return GOODIX_ERROR;
  }
  GOODIX_LOG("Goodix Read Product ID Success: 0x%04X\r\n", goodix_id);

  // Soft reset chip
  Goodix_WriteCommand(GOODIX_CMD_SOFTRESET);

  #if GOODIX_CFG_PRINT

  if (Goodix_PrintConfigValue() != GOODIX_OK) {
    return GOODIX_ERROR;
  }

  #endif

  #if GOODIX_CFG_OVERWRITE

  #if GOODIX_CFG_USE_CFG_GROUP == 0
  // Read config register value
  if (Goodix_ReadConfig() != GOODIX_OK) {
    return GOODIX_ERROR;
  }
  #endif

  // Write config register
  Goodix_CFG[0]  = 0;
  Goodix_CFG[1]  =  config->X_Resolution & 0x00FF;
  Goodix_CFG[2]  = (config->X_Resolution >> 8) & 0x00FF;
  Goodix_CFG[3]  =  config->Y_Resolution & 0x00FF;
  Goodix_CFG[4]  = (config->Y_Resolution >> 8) & 0x00FF;
  Goodix_CFG[5]  =  config->Num_Of_Touch ? config->Num_Of_Touch : 1;
  Goodix_CFG[6]  = 0;
  Goodix_CFG[6]  = ((config->ReverseX & 1) << 7) | ((config->ReverseY & 1) << 6) | ((config->SwithX2Y & 1) << 3);
  Goodix_CFG[6] |= (GOODIX_SOFT_NOISE_REDUCTION << 2) | GOODIX_INT_TRIGGER;
  if (Goodix_WriteConfig() != GOODIX_OK) {
    return GOODIX_ERROR;
  }

  #endif

  // Set read coordinates status
  Goodix_WriteCommand(GOODIX_CMD_READ);

  GOODIX_LOG("Goodix Controller Software Initialize Success\r\n");
  return GOODIX_OK;
}

/**
 * @brief Goodix reset chip function
 * 
 */
static void Goodix_Reset(void) {
  Goodix_Pin_Output(CTP_INT_PORT, CTP_INT_PIN);
  Goodix_Pin_SetLow(CTP_RST_PORT, CTP_RST_PIN);
  Goodix_Pin_SetLow(CTP_INT_PORT, CTP_INT_PIN);
  Goodix_Delay(1);
  Goodix_Pin_SetHigh(CTP_RST_PORT, CTP_RST_PIN);
  Goodix_Delay(100);
  Goodix_Pin_Input(CTP_INT_PORT, CTP_INT_PIN);
  GOODIX_LOG("Goodix Controller Reset Success\r\n");
}

/**
 * @brief Read goodix config registers value
 * 
 * @return int GOODIX_OK on read success | GOODIX_ERROR on read error
 */
static int Goodix_ReadConfig(void) {
  if (Goodix_I2C_MemRead(GOODIX_REG_CONFIG_DATA, Goodix_CFG, GOODIX_CFG_LENGTH) == GOODIX_OK) {
    if (Goodix_CalculateCheckSum() == Goodix_CFG[GOODIX_CFG_LENGTH - 2]) {
      GOODIX_LOG("Goodix Read Config Registers Success\r\n");
      return GOODIX_OK;
    }
  }
  GOODIX_LOG("Goodix Read Config Registers Failed\r\n");
  return GOODIX_ERROR;
}

/**
 * @brief Write to goodix config registers
 * 
 * @return int GOODIX_OK on write success | GOODIX_ERROR on write error
 */
static int Goodix_WriteConfig(void) {
  Goodix_CFG[GOODIX_CFG_LENGTH - 2] = Goodix_CalculateCheckSum();
  Goodix_CFG[GOODIX_CFG_LENGTH - 1] = 0x01;
  if (Goodix_I2C_MemWrite(GOODIX_REG_CONFIG_DATA, Goodix_CFG, GOODIX_CFG_LENGTH) != GOODIX_OK) {
    GOODIX_LOG("Goodix Write Config Registers Failed\r\n");
    return GOODIX_ERROR;
  }
  GOODIX_LOG("Goodix Write Config Registers Success\r\n");
  return GOODIX_OK;
}

/**
 * @brief Read goodix product id
 * 
 * @param id pointer of id buffer
 * @return int 
 */
static int Goodix_GetProductID(uint32_t *id) {
  if (Goodix_I2C_MemRead(GOODIX_REG_ID, Goodix_RxBuffer, 4) != GOODIX_OK) {
    return GOODIX_ERROR;
  }
  *id = Goodix_RxBuffer[0] << 24 | Goodix_RxBuffer[1] << 16 | Goodix_RxBuffer[2] << 8 | Goodix_RxBuffer[3];
  return GOODIX_OK;
}

/**
 * @brief Calculate checksum function
 * 
 * @return uint8_t checksum
 */
static uint8_t Goodix_CalculateCheckSum(void){
  uint8_t checksum = 0;
  for (int i = 0 ; i < GOODIX_CFG_LENGTH - 2 ; i++){
    checksum += Goodix_CFG[i];
  }
  checksum = ~checksum + 1;
  return checksum;
}

/**
 * @brief Write command to goodix chip
 * 
 * @param cmd command
 * @return int GOODIX_OK on write success | GOODIX_ERROR on write error
 */
static int Goodix_WriteCommand(uint8_t cmd) {
  Goodix_TxBuffer[0] = cmd;
  return Goodix_I2C_MemWrite(GOODIX_REG_COMMAND, Goodix_TxBuffer, 1);
}

/**
 * @brief Get touch buffer status value
 * 
 * @param status value of touch buffer status
 * @return int GOODIX_OK on read success | GOODIX_ERROR on read error
 */
static int Goodix_GetStatus(uint8_t *status) {
  return Goodix_I2C_MemRead(GOODIX_READ_COORD_ADDR, status, 1);
}

/**
 * @brief Set touch buffer status value
 * 
 * @param status value to write
 * @return int GOODIX_OK on write success | GOODIX_ERROR on write error
 */
static int Goodix_SetStatus(uint8_t  status) {
  Goodix_TxBuffer[0] = status;
  return Goodix_I2C_MemWrite(GOODIX_READ_COORD_ADDR, Goodix_TxBuffer, 1);
}
