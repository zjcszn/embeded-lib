#include "soft_i2c.h"

/* Private macro definition --------------------------------------------------*/

#if SOFT_I2C_USE_ASSERT
#include "assert.h"
#define   SOFT_I2C_ASSERT(exp)      assert(exp)
#else     
#define   SOFT_I2C_ASSERT(exp)      ((void)0U)
#endif

#if SOFT_I2C_USE_LOG
#include "stdio.h"
#define   SOFT_I2C_LOG(...)         printf(__VA_ARGS__)
#else     
#define   SOFT_I2C_LOG(...)         ((void)0U)
#endif


/* Private inline function definition ----------------------------------------*/

static inline void I2C_SCL_H(soft_i2c_dev_t *i2c_dev) {
  LL_GPIO_SetOutputPin(i2c_dev->scl.port, i2c_dev->scl.pin);
}

static inline void I2C_SCL_L(soft_i2c_dev_t *i2c_dev) {
  LL_GPIO_ResetOutputPin(i2c_dev->scl.port, i2c_dev->scl.pin);
}

static inline void I2C_SDA_H(soft_i2c_dev_t *i2c_dev) {
  LL_GPIO_SetOutputPin(i2c_dev->sda.port, i2c_dev->sda.pin);
}

static inline void I2C_SDA_L(soft_i2c_dev_t *i2c_dev) {
  LL_GPIO_ResetOutputPin(i2c_dev->sda.port, i2c_dev->sda.pin);
}

static inline uint8_t I2C_SDA_READ(soft_i2c_dev_t *i2c_dev) {
  return LL_GPIO_IsInputPinSet(i2c_dev->sda.port, i2c_dev->sda.pin);
}

static inline void I2C_DELAY(soft_i2c_dev_t *i2c_dev) {
  SOFT_I2C_DELAY(i2c_dev->ck_delay);
}


/* Private function declaration ----------------------------------------------*/

static void    soft_i2c_gpio_clk_enable(GPIO_TypeDef *gpio_x);
static void    soft_i2c_master_start(soft_i2c_dev_t *i2c_dev);
static void    soft_i2c_master_stop(soft_i2c_dev_t *i2c_dev);
static int     soft_i2c_master_send_byte(soft_i2c_dev_t *i2c_dev, uint8_t byte);
static int     soft_i2c_master_wait_ack(soft_i2c_dev_t *i2c_dev);
static void    soft_i2c_master_send_ack(soft_i2c_dev_t *i2c_dev);
static void    soft_i2c_master_send_nack(soft_i2c_dev_t *i2c_dev);
static uint8_t soft_i2c_master_recv_byte(soft_i2c_dev_t *i2c_dev);
static void    soft_i2c_set_mem_addr(soft_i2c_dev_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len);


/* Function definition -------------------------------------------------------*/

/**
 * @brief Enable gpio clock 
 * 
 * @param gpio_x gpio port
 */
static void soft_i2c_gpio_clk_enable(GPIO_TypeDef *gpio_x) {
  if (gpio_x == GPIOA) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  else if (gpio_x == GPIOB) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  else if (gpio_x == GPIOC) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  else if (gpio_x == GPIOD) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  else if (gpio_x == GPIOE) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
  else if (gpio_x == GPIOF) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  else if (gpio_x == GPIOG) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
  else if (gpio_x == GPIOH) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  else if (gpio_x == GPIOI) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOI);
}

/**
 * @brief init i2c device
 * 
 */
void soft_i2c_init(soft_i2c_dev_t *i2c_dev, uint16_t i2c_freq) {
  SOFT_I2C_ASSERT(i2c_dev);
  SOFT_I2C_ASSERT(i2c_dev->scl.port && i2c_dev->sda.port);

  uint8_t ck_delay  =  500U / i2c_freq;
  i2c_dev->ck_delay = ck_delay ? ck_delay : 1;                  // calculate i2c clock delay 

  soft_i2c_gpio_clk_enable(i2c_dev->scl.port);                  // enable scl gpio clock
  soft_i2c_gpio_clk_enable(i2c_dev->sda.port);                  // enable sda gpio clock
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;                   // set gpio mode: output
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;        // set gpio output type: open drain
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                       // set gpio no pull
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;         // set gpio speed: very high
  GPIO_InitStruct.Pin = i2c_dev->scl.pin;                       // 
  LL_GPIO_Init(i2c_dev->scl.port, &GPIO_InitStruct);            // init scl pin
  GPIO_InitStruct.Pin = i2c_dev->sda.pin;                       // 
  LL_GPIO_Init(i2c_dev->sda.port, &GPIO_InitStruct);            // init sda pin

  I2C_SDA_H(i2c_dev);                                           // set sda pin output high level
  I2C_SCL_H(i2c_dev);                                           // set scl pin output high level

  SOFT_I2C_LOG("Soft I2C Freq Set: %u Khz\r\n", 500 / i2c_dev->ck_delay);
  SOFT_I2C_LOG("Soft I2C Initialize Success\r\n");

}

/**
 * @brief I2C master send data function (polling mode)
 * 
 * @param i2c_dev  I2C handle
 * @param wr_data  pointer of write data
 * @param data_len length  of write data
 * @return int 0 on success | others on failed
 */
int soft_i2c_master_write(soft_i2c_dev_t *i2c_dev, const uint8_t *wr_data, uint16_t data_len) {
  // start condition
  soft_i2c_master_start(i2c_dev);
  
  // send slave addr (write mode)
  if (soft_i2c_master_send_byte(i2c_dev, i2c_dev->dev_addr << 1)) {
    SOFT_I2C_LOG("Soft I2C ERROR -> WR Addr No Response: 0x%02X\r\n", i2c_dev->dev_addr);
    return SOFT_I2C_ERR_ADDR;
  }

  // send data
  for (int i = 0; i < data_len; i++) {
    if (soft_i2c_master_send_byte(i2c_dev, *wr_data++)) {
      return SOFT_I2C_ERR_NACK;
    }
  }

  // stop condition
  soft_i2c_master_stop(i2c_dev);
  return SOFT_I2C_OK;
}

/**
 * @brief I2C master write register function (polling mode)
 * 
 * @param i2c_dev  I2C handle
 * @param mem_addr register address
 * @param addr_len bytes of register address
 * @param wr_data  pointer of write data
 * @param data_len length  of write data
 * @return int 0 on success | others on failed
 */
int soft_i2c_master_mem_write(soft_i2c_dev_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len, const uint8_t *wr_data, uint16_t data_len) {

  // set target memory address
  soft_i2c_set_mem_addr(i2c_dev, mem_addr, addr_len);

  // start condition
  soft_i2c_master_start(i2c_dev);

  // send slave address
  if (soft_i2c_master_send_byte(i2c_dev, i2c_dev->dev_addr << 1)) {
    SOFT_I2C_LOG("Soft I2C ERROR -> WR Addr No Response: 0x%02X\r\n", i2c_dev->dev_addr);
    return SOFT_I2C_ERR_ADDR;
  }

  // send target memory address
  for (int i = 0; i < addr_len; i++) {
    if (soft_i2c_master_send_byte(i2c_dev, i2c_dev->mem_addr[i])) {
      return SOFT_I2C_ERR_NACK;
    }
  }

  // send data
  for (int i = 0; i < data_len; i++) {
    if (soft_i2c_master_send_byte(i2c_dev, *wr_data++)) {
      return SOFT_I2C_ERR_NACK;
    }
  }

  // stop condition
  soft_i2c_master_stop(i2c_dev);
  return SOFT_I2C_OK;
}


/**
 * @brief I2C master read data function
 * 
 * @param i2c_dev  I2C handle
 * @param rd_data  pointer of read data buffer  
 * @param data_len length  of read data
 * @return int 0 on success | others on failed
 */
int soft_i2c_master_read(soft_i2c_dev_t *i2c_dev, uint8_t *rd_data, uint16_t data_len) {

  // start condition
  soft_i2c_master_start(i2c_dev);

  // send slave address (read mode)
  if (soft_i2c_master_send_byte(i2c_dev, (i2c_dev->dev_addr << 1 ) | 1)) {
    SOFT_I2C_LOG("Soft I2C ERROR -> RD Addr No Response: 0x%02X\r\n", i2c_dev->dev_addr);
    return SOFT_I2C_ERR_ADDR;
  }

  // read data
  for (int i = 0; i < data_len - 1; i++) {
    *rd_data++ = soft_i2c_master_recv_byte(i2c_dev);
    soft_i2c_master_send_ack(i2c_dev);
  }
  *rd_data = soft_i2c_master_recv_byte(i2c_dev);
  soft_i2c_master_send_nack(i2c_dev);

  // stop condition
  soft_i2c_master_stop(i2c_dev);
  return SOFT_I2C_OK;
}

/**
 * @brief I2C read data from register of target device
 * 
 * @param i2c_dev I2C handle
 * @param mem_addr register address
 * @param addr_len bytes of register address
 * @param rd_data  pointer of read data buffer
 * @param data_len length  of read data
 * @return int  0 on success | others on failed
 */
int soft_i2c_master_mem_read(soft_i2c_dev_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len, uint8_t *rd_data, uint16_t data_len) {

  // set register address
  soft_i2c_set_mem_addr(i2c_dev, mem_addr, addr_len);

  // start condition
  soft_i2c_master_start(i2c_dev);

  // send slave address (write mode)
  if (soft_i2c_master_send_byte(i2c_dev, i2c_dev->dev_addr << 1)) {
    SOFT_I2C_LOG("Soft I2C ERROR -> WR Addr No Response: 0x%02X\r\n", i2c_dev->dev_addr);
    return SOFT_I2C_ERR_ADDR;
  }

  // send target register address
  for (int i = 0; i < addr_len; i++) {
    if (soft_i2c_master_send_byte(i2c_dev, i2c_dev->mem_addr[i])) {
      return SOFT_I2C_ERR_NACK;
    }
  }

  // restart condition
  soft_i2c_master_start(i2c_dev);

  // send slave address (read mode)
  if (soft_i2c_master_send_byte(i2c_dev, (i2c_dev->dev_addr << 1) | 0x01)) {
    SOFT_I2C_LOG("Soft I2C ERROR -> RD Addr No Response: 0x%02X\r\n", i2c_dev->dev_addr);
    return SOFT_I2C_ERR_ADDR;
  }

  // read data
  for (int i = 0; i < data_len - 1; i++) {
    *rd_data++ = soft_i2c_master_recv_byte(i2c_dev);
    soft_i2c_master_send_ack(i2c_dev);
  }
  *rd_data = soft_i2c_master_recv_byte(i2c_dev);
  soft_i2c_master_send_nack(i2c_dev);

  // stop condition
  soft_i2c_master_stop(i2c_dev);
  return SOFT_I2C_OK;
}

/**
 * @brief set register address of target device
 * 
 * @param i2c_dev I2C handle
 * @param mem_addr register address
 * @param addr_len bytes of register address
 */
static void soft_i2c_set_mem_addr(soft_i2c_dev_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len) {
  for (int i = addr_len - 1; i >= 0; i--, mem_addr >>= 8) {
    i2c_dev->mem_addr[i] = mem_addr & 0xFF;
  }
}

/**
 * @brief send i2c start condition
 * @param i2c_dev I2C handle
 */
static void soft_i2c_master_start(soft_i2c_dev_t *i2c_dev) {
  I2C_SCL_L(i2c_dev);
  I2C_SDA_H(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SCL_H(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SDA_L(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SCL_L(i2c_dev);
}

/**
 * @brief send stop condition
 * @param i2c_dev I2C handle
 */
static void soft_i2c_master_stop(soft_i2c_dev_t *i2c_dev) {
  I2C_SCL_L(i2c_dev);
  I2C_SDA_L(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SCL_H(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SDA_H(i2c_dev);
}

/**
 * @brief send byte data
 * @param i2c_dev I2C handle
 * @param byte byte data
 * @return int 0 on ack | 2 on noack
 */
static int soft_i2c_master_send_byte(soft_i2c_dev_t *i2c_dev, uint8_t byte) {
  for (int i = 0; i < 8; i++) {
    I2C_SCL_L(i2c_dev);
    byte & 0x80 ? I2C_SDA_H(i2c_dev) : I2C_SDA_L(i2c_dev);
    I2C_DELAY(i2c_dev);
    I2C_SCL_H(i2c_dev);
    I2C_DELAY(i2c_dev);
    byte <<= 1U;
  }
  if (soft_i2c_master_wait_ack(i2c_dev)) {
    soft_i2c_master_stop(i2c_dev);
    SOFT_I2C_LOG("Soft I2C ERROR -> No ACK\r\n");
    return SOFT_I2C_ERR_NACK;
  }
  return SOFT_I2C_OK;
}

/**
 * @brief i2c master wait for ack signal
 * @param i2c_dev I2C handle
 * @return int 0 on ack | 1 on nack
 */
static int soft_i2c_master_wait_ack(soft_i2c_dev_t *i2c_dev) {
  I2C_SCL_L(i2c_dev);
  I2C_SDA_H(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SCL_H(i2c_dev);
  I2C_DELAY(i2c_dev);
  return I2C_SDA_READ(i2c_dev);
}

/**
 * @brief i2c master send ack signal when after read a byte
 * @param i2c_dev I2C handle
 */
static void soft_i2c_master_send_ack(soft_i2c_dev_t *i2c_dev) {
  I2C_SCL_L(i2c_dev);
  I2C_SDA_L(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SCL_H(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SCL_L(i2c_dev);
  I2C_SDA_H(i2c_dev);
}

/**
 * @brief i2c master send noack signal when after read a byte
 * @param i2c_dev I2C handle
 */
static void soft_i2c_master_send_nack(soft_i2c_dev_t *i2c_dev) {
  I2C_SCL_L(i2c_dev);
  I2C_SDA_H(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SCL_H(i2c_dev);
  I2C_DELAY(i2c_dev);
  I2C_SCL_L(i2c_dev);
  I2C_SDA_H(i2c_dev);
}

/**
 * @brief i2c master read a byte from target device
 * @param i2c_dev I2C handle
 * @return uint8_t byte data 
 */
static uint8_t soft_i2c_master_recv_byte(soft_i2c_dev_t *i2c_dev) {
  uint8_t recv_tmp;
  for (int i = 0; i < 8; i++) {
    I2C_SCL_L(i2c_dev);
    I2C_DELAY(i2c_dev);
    I2C_SCL_H(i2c_dev);
    I2C_DELAY(i2c_dev);
    recv_tmp = (recv_tmp << 1) | I2C_SDA_READ(i2c_dev);
  }
  return recv_tmp;
}

