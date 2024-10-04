/**
  **************************************************************************
  * @file     eeprom.c
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    the driver library of the flash eeprom
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
  
#include "bsp_emul_ee.h"

#define EE_VALID_PAGE0                  ((uint16_t)0x0000)  /*!< the effective page is page 0 */ 
#define EE_VALID_PAGE1                  ((uint16_t)0x0001)  /*!< the effective page is page 1 */ 
#define EE_VALID_PAGE_NONE              ((uint16_t)0x0002)  /*!< no valid page found */
                                                              
#define EE_PAGE_ERASED                  ((uint16_t)0xFFFF)  /*!< page is in erased state */ 
#define EE_PAGE_TRANSFER                ((uint16_t)0xCCCC)  /*!< page is in transfer state */ 
#define EE_PAGE_VALID                   ((uint16_t)0x0000)  /*!< page is in valid state */ 

/**
  * @brief  flash eeprom valid page get mode
  */
typedef enum
{
  EE_VALID_PAGE_READ                = 0x01, /*!< get valid page in read mode */
  EE_VALID_PAGE_WRITE               = 0x02, /*!< get valid page in write mode */
} ee_valid_page_type;


static inline void flash_lock(void) {
    fmc_bank1_lock();
}

static inline void flash_unlock(void) {
    fmc_bank1_unlock();
}

static inline flash_status_t flash_sector_erase(uint32_t erase_address) {
    fmc_state_enum fmc_state;

    if ((erase_address >= EE_BASE_ADDRESS) && (erase_address < (EE_BASE_ADDRESS + 2 * EE_PAGE_SIZE))) {
        /* erase the flash pages */
        fmc_state = fmc_page_erase(erase_address);

        return (fmc_state == FMC_READY) ? FLASH_OPERATE_DONE : FLASH_EPP_ERROR;
    }

    return FLASH_EPP_ERROR;
}

static flash_status_t flash_halfword_program(uint32_t address, uint16_t data) {
    fmc_state_enum fmc_state;

    if ((address >= EE_BASE_ADDRESS) && (address < (EE_BASE_ADDRESS + 2 * EE_PAGE_SIZE))) {
        /* program flash */
        fmc_state = fmc_halfword_program(address, data);
        return (fmc_state == FMC_READY) ? FLASH_OPERATE_DONE : FLASH_PROGRAM_ERROR;
    }


    return FLASH_PROGRAM_ERROR;
}

/** 
  * @brief  erase eeprom page, one page can contain one or more sectors.
  * @param  page_address:
  *         this parameter can be one of the following values:  
  *         - EE_PAGE0_ADDRESS: erase page 0
  *         - EE_PAGE1_ADDRESS: erase page 1
  * @retval flash_status
  */
flash_status_t flash_ee_page_erase(uint32_t page_address)
{
  uint16_t i;
  uint32_t erase_address;
  flash_status_t flash_status;
  
  /* erase one or more sectors */ 
  for(i = 0; i < EE_SECTOR_NUM; i++)
  {
    /* calculate the erase address */ 
    erase_address = page_address + i * EE_SECTOR_SIZE;
    
    /* erase sector */ 
    if ((flash_status = flash_sector_erase(erase_address)) != FLASH_OPERATE_DONE)
    {
      return flash_status;
    }
  }
  
  return FLASH_OPERATE_DONE;  
}

/** 
  * @brief  get the valid eeprom page.
  * @param  mode
  *         - EE_VALID_PAGE_READ: get valid page in read mode 
  *         - EE_VALID_PAGE_WRITE: get valid page in write mode
  * @retval valid page
  *         - EE_VALID_PAGE0: page 0 is valid
  *         - EE_VALID_PAGE1: page 1 is valid
  *         - EE_VALID_PAGE_NONE: no valid page
  */
uint16_t flash_ee_valid_page_get(ee_valid_page_type mode)
{
  uint16_t page0_status;
  uint16_t page1_status;

  /* get page 0 status */ 
  page0_status = (*(__IO uint16_t*)EE_PAGE0_ADDRESS);

  /* get page 1 status */ 
  page1_status = (*(__IO uint16_t*)EE_PAGE1_ADDRESS);

  if (mode == EE_VALID_PAGE_READ)
  {
    if (page0_status == EE_PAGE_VALID)
    {
      /* page 0 is valid */ 
      return EE_VALID_PAGE0;
    }
    else if (page1_status == EE_PAGE_VALID)
    {
      /* page 1 is valid */     
      return EE_VALID_PAGE1;     
    }  
  }
  else
  {
    if (page0_status == EE_PAGE_TRANSFER)
    {
      /* page 0 is valid */ 
      return EE_VALID_PAGE0;
    }
    else if (page1_status == EE_PAGE_TRANSFER)
    {
      /* page 1 is valid */ 
      return EE_VALID_PAGE1;
    }
    else if (page0_status == EE_PAGE_VALID)
    {
      /* page 0 is valid */ 
      return EE_VALID_PAGE0;
    }
    else if (page1_status == EE_PAGE_VALID)
    {
      /* page 1 is valid */     
      return EE_VALID_PAGE1;     
    }  
  }

  /* no valid page found */
  return EE_VALID_PAGE_NONE; 
}

/** 
  * @brief  write data to the eeprom.
  * @param  address: variable address.
  * @param  data: data.
  * @retval flash_status.
  */
flash_status_t flash_ee_write_no_check(uint16_t address, uint16_t data)
{
  uint16_t valid_page;
  uint32_t find_address; 
  uint32_t end_address;
  flash_status_t flash_status = FLASH_OPERATE_DONE;
  
  /* get the valid page */
  valid_page = flash_ee_valid_page_get(EE_VALID_PAGE_WRITE);

  if (valid_page == EE_VALID_PAGE_NONE)
  {
    return  FLASH_PROGRAM_ERROR;
  }

  /* find address calculation */ 
  find_address = EE_BASE_ADDRESS + valid_page * EE_PAGE_SIZE;

  /* end address calculation */
  end_address  = EE_BASE_ADDRESS + valid_page * EE_PAGE_SIZE + EE_PAGE_SIZE - 2;  
  
  while (find_address < end_address)
  {
    /* find addresses without data */ 
    if ((*(__IO uint32_t*)find_address) == 0xFFFFFFFF)
    {
      /* write data to flash */ 
      if ((flash_status = flash_halfword_program(find_address, data)) != FLASH_OPERATE_DONE)
      {
        return flash_status;
      }
      
      /* write variable address to flash */
      if ((flash_status = flash_halfword_program(find_address + 2, address)) != FLASH_OPERATE_DONE)
      {
        return flash_status;
      }
      
      return FLASH_OPERATE_DONE;
    }
    else
    {
      /* find address + 4 */ 
      find_address += 4;
    }
  }

  return FLASH_PROGRAM_ERROR;
}

/** 
  * @brief  transfer full page data to empty page.
  * @param  none
  * @retval flash_status
  */
flash_status_t flash_ee_copy_to_new_page(void)
{
  uint16_t data;
  uint16_t idx, valid_page;
  uint32_t full_page_address;
  uint32_t empty_page_address;
  flash_status_t flash_status = FLASH_OPERATE_DONE;
  
  /* get valid page */
  valid_page = flash_ee_valid_page_get(EE_VALID_PAGE_READ);

  if (valid_page == EE_VALID_PAGE0) 
  {
    /* empty page is page 1 */
    empty_page_address = EE_PAGE1_ADDRESS;

    /* full page is pgae 0 */ 
    full_page_address = EE_PAGE0_ADDRESS;
  }
  else if (valid_page == EE_VALID_PAGE1) 
  {
    /* empty page is page 0 */
    empty_page_address = EE_PAGE0_ADDRESS;

    /* full page is pgae 1 */
    full_page_address = EE_PAGE1_ADDRESS;
  }
  else
  {
    /* page status flag error */ 
    return FLASH_PROGRAM_ERROR; 
  }

  /* change the status of the empty page to TRANSFER */ 
  if ((flash_status = flash_halfword_program(empty_page_address, EE_PAGE_TRANSFER)) != FLASH_OPERATE_DONE)
  {
    return flash_status;
  }

  for (idx = 0; idx < EE_PARA_MAX_NUMBER; idx++)
  {
    /* find valid variables */
    if (flash_ee_data_read(idx, &data) == 0)
    {
      /* store variable to new page  */ 
      if ((flash_status = flash_ee_write_no_check(idx, data)) != FLASH_OPERATE_DONE)
      {
        return flash_status;
      }
    }
  }

  /* erase old page */
  if ((flash_status = flash_ee_page_erase(full_page_address)) != FLASH_OPERATE_DONE)
  {
    return flash_status;
  }

  /* change the status of the empty page to VALID */ 
  if ((flash_status = flash_halfword_program(empty_page_address, EE_PAGE_VALID)) != FLASH_OPERATE_DONE)
  {
    return flash_status;
  }

  return FLASH_OPERATE_DONE;
}

/** 
  * @brief  erase all pages to format eeprom.
  * @param  none
  * @retval flash_status
  */
flash_status_t flash_ee_format(void)
{
  flash_status_t flash_status = FLASH_OPERATE_DONE;

  /* erase page 0 */
  if ((flash_status = flash_ee_page_erase(EE_PAGE0_ADDRESS)) != FLASH_OPERATE_DONE)
  {
    return flash_status;
  }

  /* erase page 1 */
  if ((flash_status = flash_ee_page_erase(EE_PAGE1_ADDRESS)) != FLASH_OPERATE_DONE)
  {
    return flash_status;
  }
  
  /* mark the status of page 0 as VALID */
  return flash_halfword_program(EE_PAGE0_ADDRESS, EE_PAGE_VALID);
}

/** 
  * @brief  eeprom format validity check.
  * @param  page0_status: page0 status
  * @param  page1_status: page1 status
  * @retval format status:
  *         - 0: the format is correct
  *         - 1: the format is incorrect
  */
uint16_t flash_ee_format_check(uint16_t page0_status, uint16_t page1_status)
{
  /* the euqal status is invalid */
  if (page0_status == page1_status)
  {
    return 1;
  }
  
  /* status is not known status is invalid status */ 
  if ((page0_status != EE_PAGE_ERASED) && (page0_status != EE_PAGE_TRANSFER) && (page0_status != EE_PAGE_VALID))
  {
    return 1;
  }
  
  /* status is not known status is invalid status */ 
  if ((page1_status != EE_PAGE_ERASED) && (page1_status != EE_PAGE_TRANSFER) && (page1_status != EE_PAGE_VALID))
  {
    return 1;
  }
  
  /* the format is correct */
  return 0;
}

/** 
  * @brief  check if eeprom page is full, when the page is full, transfer the data to erase page.
  * @param  none
  * @retval flash_status
  */
flash_status_t flash_ee_full_check(void)
{
  uint16_t valid_page;
  uint32_t end_address;
  flash_status_t flash_status = FLASH_OPERATE_DONE;
  
  /* get the valid page */
  valid_page = flash_ee_valid_page_get(EE_VALID_PAGE_READ);

  if (valid_page == EE_VALID_PAGE_NONE)
  {
    return  FLASH_PROGRAM_ERROR;
  }

  /* end address calculation */
  end_address  = EE_BASE_ADDRESS + valid_page * EE_PAGE_SIZE + EE_PAGE_SIZE - 2;  

  /* check if the page is full */ 
  if ((*(__IO uint32_t*)(end_address - 2)) != 0xFFFFFFFF)
  {
    /* when the page is full, transfer the data to erase page */ 
    if ((flash_status = flash_ee_copy_to_new_page()) != FLASH_OPERATE_DONE)
    {
      return flash_status;
    }
  }
  
  /* page full check ok */
  return FLASH_OPERATE_DONE;
}

/** 
  * @brief  transition state processing, a page state is ERASE, a page state is TRANSFER, 
  *         and the TRANSFER state is changed to VALID.
  * @param  page0_status: page0 status
  * @param  page1_status: page1 status
  * @retval format status:
  *         - 0: the format is correct
  *         - 1: the format is incorrect
  */
flash_status_t flash_ee_erase_transfer(uint16_t page0_status, uint16_t page1_status)
{
  if (page0_status == EE_PAGE_TRANSFER)
  {
    /* mark the status of page 0 as VALID */
    return flash_halfword_program(EE_PAGE0_ADDRESS, EE_PAGE_VALID);
  }
  else
  {
    /* mark the status of page 1 as VALID */
    return flash_halfword_program(EE_PAGE1_ADDRESS, EE_PAGE_VALID);
  }
}

/** 
  * @brief  transition state processing, one page state is VALID, one page state is TRANSFER,
  *         and the data is transferred to the TRANSFER state page.
  * @param  page0_status: page0 status
  * @param  page1_status: page1 status
  * @retval format status:
  *         - 0: the format is correct
  *         - 1: the format is incorrect
  */
flash_status_t flash_ee_valid_transfer(uint16_t page0_status, uint16_t page1_status)
{                                  
  uint32_t erase_page_address;  
  flash_status_t  flash_status;
  
  /* find the page in the transfer state, erase the page, and retransmit the data */ 
  if (page0_status == EE_PAGE_TRANSFER)
  {
    erase_page_address = EE_PAGE0_ADDRESS;
  }
  else
  {
    erase_page_address = EE_PAGE1_ADDRESS;
  }
  
  /* erase the transfer state page */ 
  if ((flash_status = flash_ee_page_erase(erase_page_address)) != FLASH_OPERATE_DONE)
  {
    return flash_status;
  }
  
  /* retransmit data */ 
  return flash_ee_copy_to_new_page();
}

/** 
  * @brief  eeprom init.
  +-------------------+---------------------------------------------------------------------------------------------+
  |                   |                                      PAGE 1 status                                          |
  |                   +-----------------------+----------------------------------+----------------------------------+
  |                   |         ERASE         |              TRANSFER            |              VALID               |
  +--------+----------+-----------------------+----------------------------------+----------------------------------+
  |        |          |  erase page 0         |  erase page 0                    |                                  |
  |        | ERASE    |  erase page 1         |  mark  page 1 VALID              |   erase page 0                   |
  |        |          |  mark  page 0 VALID   |                                  |                                  |
  |        +----------+-----------------------+----------------------------------+----------------------------------+
  |        |          |  erase page 1         |  erase page 0                    |  erase page 0                    |
  | PAGE 0 | TRANSFER |  mark  page 0 VALID   |  erase page 1                    |  copy data from page 1 to page 0 |
  | status |          |                       |  mark  page 0 VALID              |  erase page 1                    |
  |        |          |                       |                                  |  mark  page 0 VALID              |
  |        +----------+-----------------------+----------------------------------+----------------------------------+
  |        |          |                       |  erase page 1                    |  erase page 0                    |
  |        | VALID    |  erase page 1         |  copy data from page 0 to page 1 |  erase page 1                    |
  |        |          |                       |  erase page 0                    |  mark  page 0 VALID              |
  |        |          |                       |  mark  page 1 VALID              |                                  |
  +--------+----------+-----------------------+----------------------------------+----------------------------------+
  * @param  none
  * @retval flash status.
  */
flash_status_t flash_ee_init(void)
{
  uint16_t page0_status; 
  uint16_t page1_status;
  flash_status_t flash_status = FLASH_OPERATE_DONE;

  /* flash unlock */
  flash_unlock();
  
  /* get page 0 status */ 
  page0_status = (*(__IO uint16_t*)EE_PAGE0_ADDRESS);

  /* get page 2 status */ 
  page1_status = (*(__IO uint16_t*)EE_PAGE1_ADDRESS);

  /* ensure that the sector data is completely erased */ 
  if (page0_status == EE_PAGE_ERASED)
  {
    /* erase page 0 */
    if ((flash_status = flash_ee_page_erase(EE_PAGE0_ADDRESS)) != FLASH_OPERATE_DONE)
    {
      /* flash lock */
      flash_lock();
          
      return flash_status;
    }
  }

  /* ensure that the sector data is completely erased */ 
  if (page1_status == EE_PAGE_ERASED)
  {
    /* erase page 1 */
    if ((flash_status = flash_ee_page_erase(EE_PAGE1_ADDRESS)) != FLASH_OPERATE_DONE)
    {
      /* flash lock */
      flash_lock();
      
      return flash_status;
    }
  }  
  
  /* format validity check */
  if (flash_ee_format_check(page0_status, page1_status) != 0)
  {
    /* if the format is invalid, reformat */
    if ((flash_status = flash_ee_format()) != FLASH_OPERATE_DONE)
    {
      /* flash lock */
      flash_lock();
      
      return flash_status;
    }
  }
  
  /* transition state processing, a page state is ERASE, a page state is TRANSFER, and the TRANSFER state is changed to VALID */
  if (((page0_status == EE_PAGE_ERASED) && (page1_status == EE_PAGE_TRANSFER)) || 
     ((page0_status == EE_PAGE_TRANSFER) && (page1_status == EE_PAGE_ERASED)))
  {
    if ((flash_status = flash_ee_erase_transfer(page0_status, page1_status)) != FLASH_OPERATE_DONE)
    {
      /* flash lock */
      flash_lock();
      
      return flash_status;
    }
  }

  /* transition state processing, one page state is VALID, one page state is TRANSFER, and the data is transferred to the TRANSFER state page */
  if (((page0_status == EE_PAGE_VALID) && (page1_status == EE_PAGE_TRANSFER)) || 
     ((page0_status == EE_PAGE_TRANSFER) && (page1_status == EE_PAGE_VALID)))
  {
    if ((flash_status = flash_ee_valid_transfer(page0_status, page1_status)) != FLASH_OPERATE_DONE)
    {
      /* flash lock */
      flash_lock();
      
      return flash_status;
    }
  }
  
  /* check if the page is full, when the page is full, transfer the data to erase page */ 
  if ((flash_status = flash_ee_full_check()) != FLASH_OPERATE_DONE)
  {
    /* flash lock */
    flash_lock();
    
    return flash_status;
  }

  /* flash lock */
  flash_lock();
  
  return FLASH_OPERATE_DONE;
}

/** 
  * @brief  write data to the eeprom.
  * @param  address: variable address.
  * @param  data: data.
  * @retval flash_status.
  */
flash_status_t flash_ee_data_write(uint16_t address, uint16_t data)
{
  flash_status_t flash_status = FLASH_OPERATE_DONE;
  
  /* flash unlock */
  flash_unlock();
  
  /* check if the page is full, when the page is full, transfer the data to erase page */ 
  if ((flash_status = flash_ee_full_check()) != FLASH_OPERATE_DONE)
  {
    /* flash lock */
    flash_lock();

    return flash_status;
  }
  
   /* write data to flash */ 
  if ((flash_status = flash_ee_write_no_check(address, data)) != FLASH_OPERATE_DONE)
  {
    /* flash lock */
    flash_lock();

    return flash_status;
  }
  
  /* check if the page is full, when the page is full, transfer the data to erase page */ 
  if ((flash_status = flash_ee_full_check()) != FLASH_OPERATE_DONE)
  {
    /* flash lock */
    flash_lock();
    
    return flash_status;
  }

  /* flash lock */
  flash_lock();
  
  return FLASH_OPERATE_DONE;
}

/** 
  * @brief  read data from the eeprom.
  * @param  address: variable address.
  * @param  pdata: data pointer.
  * @retval read status:
  *         - 0: data successfully read
  *         - 1: failed to read data
  */
uint16_t flash_ee_data_read(uint16_t address, uint16_t* pdata)
{
  uint16_t valid_page;
  uint16_t data_address;
  uint32_t find_address;
  uint32_t start_address;

  /* get the valid page */
  valid_page = flash_ee_valid_page_get(EE_VALID_PAGE_READ);

  if (valid_page == EE_VALID_PAGE_NONE)
  {
    return  EE_VALID_PAGE_NONE;
  }
  
  /* start address calculation */
  start_address = EE_BASE_ADDRESS + valid_page * EE_PAGE_SIZE + 4;
  
  /* find address calculation */
  find_address  = EE_BASE_ADDRESS + valid_page * EE_PAGE_SIZE + EE_PAGE_SIZE - 2;  
  
  while (find_address > start_address)
  {
    /* read variable address */
    data_address = (*(__IO uint16_t*)find_address);

    /* variable address matching */ 
    if (address == data_address)
    {
      /* read data */ 
      *pdata = (*(__IO uint16_t*)(find_address - 2));

      /* data successfully read */
      return 0;
    }

    /* find address - 4 */ 
    find_address -= 4;
  }

  /* failed to read data */
  return 1;
}

