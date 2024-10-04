#ifndef _24CXX_H_
#define _24CXX_H_

#include <stdint.h>


/* Export Macro Definetion ---------------------------------------------------*/


/* if define 1 enable assert macro */
#define _24CXX_USE_ASSERT               0

/* if define 1 enable output error log */
#define _24CXX_USE_LOG                  1

/* if define 1 enable check validity of param */
#define _24CXX_CHECK_PARAM              1

/* if define 1 enable check overflow of write or read operation */
#define _24CXX_CHECK_CHIP_OVERFLOW      1
#define _24CXX_CHECK_PAGE_OVERFLOW      0

/* if define 1 enable test demo */
/* WARNING: Test Function Will Corrupted EEPROM Old Data!!! */
#define _24CXX_USE_TEST                 0

#if     _24CXX_USE_TEST

/* eeprom device definition */
#define _24CXX_DEMO_ADDR                0x50
#define _24CXX_DEMO_TYPE                _24CXX_EEPROM
#define _24CXX_DEMO_MODEL               _24C256
#define _24CXX_WRITE_DELAY              5

/* eeprom i2c interface definition */
#define _24CXX_DEMO_SCL_PIN             LL_GPIO_PIN_7
#define _24CXX_DEMO_SCL_PORT            GPIOG
#define _24CXX_DEMO_SDA_PIN             LL_GPIO_PIN_8
#define _24CXX_DEMO_SDA_PORT            GPIOG

#endif

/* Export Type Definetion ----------------------------------------------------*/


/* 24cxx type */
typedef enum _24cxx_type {
  _24CXX_EEPROM,
  _24CXX_FRAM,
} _24cxx_type_t;


/* 24cxx error code */
typedef enum _24cxx_error
{
  _24CXX_OK                  = 0x00,
  _24CXX_ERR_INIT_FAILED     = 0x01,
  _24CXX_ERR_WRITE_FAILED    = 0x02,
  _24CXX_ERR_READ_FAILED     = 0x03,
  _24CXX_ERR_ERASE_FAILED    = 0x04,
  _24CXX_ERR_COMPARE_FAILED  = 0x05,
  _24CXX_ERR_TRANS_FAILED    = 0x06,
  _24CXX_ERR_PARAM_INVALID   = 0x20,
  _24CXX_ERR_PARAM_DEVICE    = 0x21,
  _24CXX_ERR_PARAM_BUFFER    = 0x22,
  _24CXX_ERR_PARAM_MEMADDR   = 0x23,
  _24CXX_ERR_PARAM_LENGTH    = 0x24,
  _24CXX_ERR_PAGE_OVERFLOW   = 0x30,
  _24CXX_ERR_CHIP_OVERFLOW   = 0x31,
} _24cxx_error_t;

/* 24cxx model */
typedef enum _24cxx_model
{
  _24C01,
  _24C02,
  _24C04,
  _24C08,
  _24C16,
  _24C32,
  _24C64,
  _24C128,
  _24C256,
  _24C512,
  _24C1024,
  _24C2048,
} _24cxx_model_t;

/*24cxx devcie struct*/

typedef struct 
{
  _24cxx_type_t  type;             /* 24cxx type */
  _24cxx_model_t model;            /* 24cxx model */
  uint8_t dev_addr;                /* 24cxx i2c addr */
  int (*i2c_mem_read)(uint8_t dev_addr, uint16_t mem_addr,    /* i2c bus read function */
        uint16_t mem_addr_bytes, void *recv_buff, uint32_t recv_size);
  int (*i2c_mem_write)(uint8_t dev_addr, uint16_t mem_addr,   /* i2c bus write function */
        uint16_t mem_addr_bytes, const void *send_buff, uint32_t send_size);
  void (*wp)(uint8_t ctrl);        /* protect of write function */
  void (*page_write_delay)(void);  /* there is a delay in continuous writin for EEPROM,FRAM not need */
} _24cxx_dev_t;



/* 24CXX Export Function Declaration -----------------------------------------*/


extern int16_t _24cxx_read(_24cxx_dev_t *pdev,uint32_t addr, uint8_t *pbuf, uint32_t size);
extern int16_t _24cxx_write(_24cxx_dev_t *pdev,uint32_t addr,uint8_t *pbuf,uint32_t size);
extern int16_t _24cxx_erase(_24cxx_dev_t *pdev,uint32_t addr,uint8_t data, uint32_t size);
extern int16_t _24cxx_compare(_24cxx_dev_t *pdev, uint32_t addr, uint8_t *pbuf, uint32_t size);

#if _24CXX_USE_TEST
extern int16_t _24cxx_test(void);
#endif

#endif
