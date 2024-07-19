/**
  **************************************************************************
  * @file     eeprom.h
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    flash eeprom libray header file
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to 
  * download from Artery official website is the copyrighted work of Artery. 
  * Artery authorizes customers to use, copy, and distribute the BSP 
  * software and its related documentation for the purpose of design and 
  * development in conjunction with Artery microcontrollers. Use of the 
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

/*!< define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes ------------------------------------------------------------------*/
#include "gd32f30x.h"

/*
  +-----------------------------------------------------+
  |                        EEPROM                       |
  +--------------------------+--------------------------+
  |          PAGE 0          |          PAGE 1          |
  +--------+--------+--------+--------+--------+--------+
  |        |        |        |        |        |        |
  | sector | sector | sector | sector | sector | sector |
  |   0    |  ...   |   N    | N + 1  |  ...   |  N + N |
  |        |        |        |        |        |        |
  +--------+--------+--------+--------+--------+--------+
*/

/**
  * @brief  flash status type
  */
typedef enum
{
  FLASH_OPERATE_BUSY                     = 0x00, /*!< flash status is operate busy */
  FLASH_PROGRAM_ERROR                    = 0x01, /*!< flash status is program error */
  FLASH_EPP_ERROR                        = 0x02, /*!< flash status is epp error */
  FLASH_OPERATE_DONE                     = 0x03, /*!< flash status is operate done */
  FLASH_OPERATE_TIMEOUT                  = 0x04  /*!< flash status is operate timeout */
} flash_status_t;


/*!< user defined */ 
#define EE_SECTOR_NUM            ((uint32_t)1)                                 /*!< sector number, support multiple sectors to from 1 page */
#define EE_SECTOR_SIZE           ((uint32_t)(1024 * 4))                        /*!< sector size */

/*!< user do not need to care */ 
#define EE_FLASH_SIZE            (*(uint16_t *)0x1FFFF7E0)	                   /*!< at32 flash size information */ 

#define EE_PAGE_SIZE             ((uint32_t)(EE_SECTOR_NUM * EE_SECTOR_SIZE))  /*!< page size */

#define EE_BASE_ADDRESS          (0x08000000 + 1024 * EE_FLASH_SIZE - EE_PAGE_SIZE * 2)   /*!< eeprom base address */    
#define EE_PAGE0_ADDRESS         ((uint32_t)(EE_BASE_ADDRESS))                 /*!< eeprom page 0 base address */
#define EE_PAGE1_ADDRESS         ((uint32_t)(EE_PAGE0_ADDRESS + EE_PAGE_SIZE)) /*!< eeprom page 1 base address */

#define EE_PARA_MAX_NUMBER       ((uint16_t)(EE_PAGE_SIZE / 4 - 1))            /*!< maximum number of variables that can be stored */ 

flash_status_t flash_ee_init       (void);
uint16_t       flash_ee_data_read  (uint16_t address, uint16_t* pdata);
flash_status_t flash_ee_data_write (uint16_t address, uint16_t data);

#ifdef __cplusplus
}
#endif

#endif






