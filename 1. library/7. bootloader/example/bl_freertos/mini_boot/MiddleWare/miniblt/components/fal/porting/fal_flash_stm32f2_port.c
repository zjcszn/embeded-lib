/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include "boot_osal.h"
#include "fal.h"

#include "stm32f4xx.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"

#define FLASH_FLAG_ALL           (FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |   \
                                  FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR)

/* base address of the flash sectors */
#define ADDR_FLASH_SECTOR_0      ((uint32_t)0x08000000) /* Base address of Sector 0, 16 K bytes   */
#define ADDR_FLASH_SECTOR_1      ((uint32_t)0x08004000) /* Base address of Sector 1, 16 K bytes   */
#define ADDR_FLASH_SECTOR_2      ((uint32_t)0x08008000) /* Base address of Sector 2, 16 K bytes   */
#define ADDR_FLASH_SECTOR_3      ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 K bytes   */
#define ADDR_FLASH_SECTOR_4      ((uint32_t)0x08010000) /* Base address of Sector 4, 64 K bytes   */
#define ADDR_FLASH_SECTOR_5      ((uint32_t)0x08020000) /* Base address of Sector 5, 128 K bytes  */
#define ADDR_FLASH_SECTOR_6      ((uint32_t)0x08040000) /* Base address of Sector 6, 128 K bytes  */
#define ADDR_FLASH_SECTOR_7      ((uint32_t)0x08060000) /* Base address of Sector 7, 128 K bytes  */
#define ADDR_FLASH_SECTOR_8      ((uint32_t)0x08080000) /* Base address of Sector 8, 128 K bytes  */
#define ADDR_FLASH_SECTOR_9      ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 K bytes  */
#define ADDR_FLASH_SECTOR_10     ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 K bytes */
#define ADDR_FLASH_SECTOR_11     ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 K bytes */

/**
 * Get the sector of a given address
 *
 * @param address flash address
 *
 * @return The sector of a given address
 */
static uint32_t stm32_get_sector(uint32_t address) {
    uint32_t sector = 0;

    if ((address < ADDR_FLASH_SECTOR_1) && (address >= ADDR_FLASH_SECTOR_0)) {
        sector = FLASH_SECTOR_0;
    } else if ((address < ADDR_FLASH_SECTOR_2) && (address >= ADDR_FLASH_SECTOR_1)) {
        sector = FLASH_SECTOR_1;
    } else if ((address < ADDR_FLASH_SECTOR_3) && (address >= ADDR_FLASH_SECTOR_2)) {
        sector = FLASH_SECTOR_2;
    } else if ((address < ADDR_FLASH_SECTOR_4) && (address >= ADDR_FLASH_SECTOR_3)) {
        sector = FLASH_SECTOR_3;
    } else if ((address < ADDR_FLASH_SECTOR_5) && (address >= ADDR_FLASH_SECTOR_4)) {
        sector = FLASH_SECTOR_4;
    } else if ((address < ADDR_FLASH_SECTOR_6) && (address >= ADDR_FLASH_SECTOR_5)) {
        sector = FLASH_SECTOR_5;
    } else if ((address < ADDR_FLASH_SECTOR_7) && (address >= ADDR_FLASH_SECTOR_6)) {
        sector = FLASH_SECTOR_6;
    } else if ((address < ADDR_FLASH_SECTOR_8) && (address >= ADDR_FLASH_SECTOR_7)) {
        sector = FLASH_SECTOR_7;
    } else if ((address < ADDR_FLASH_SECTOR_9) && (address >= ADDR_FLASH_SECTOR_8)) {
        sector = FLASH_SECTOR_8;
    } else if ((address < ADDR_FLASH_SECTOR_10) && (address >= ADDR_FLASH_SECTOR_9)) {
        sector = FLASH_SECTOR_9;
    } else if ((address < ADDR_FLASH_SECTOR_11) && (address >= ADDR_FLASH_SECTOR_10)) {
        sector = FLASH_SECTOR_10;
    } else {
        sector = FLASH_SECTOR_11;
    }

    return sector;
}

/**
 * Get the sector size
 *
 * @param sector sector
 *
 * @return sector size
 */
static uint32_t stm32_get_sector_size(uint32_t sector) {
    assert(IS_FLASH_SECTOR(sector));

    switch (sector) {
        case FLASH_SECTOR_0:
            return 16 * 1024;
        case FLASH_SECTOR_1:
            return 16 * 1024;
        case FLASH_SECTOR_2:
            return 16 * 1024;
        case FLASH_SECTOR_3:
            return 16 * 1024;
        case FLASH_SECTOR_4:
            return 64 * 1024;
        case FLASH_SECTOR_5:
            return 128 * 1024;
        case FLASH_SECTOR_6:
            return 128 * 1024;
        case FLASH_SECTOR_7:
            return 128 * 1024;
        case FLASH_SECTOR_8:
            return 128 * 1024;
        case FLASH_SECTOR_9:
            return 128 * 1024;
        case FLASH_SECTOR_10:
            return 128 * 1024;
        case FLASH_SECTOR_11:
            return 128 * 1024;
        default :
            return 128 * 1024;
    }
}

static int init(void) {
    /* do nothing now */
    return 1;
}

static int read(long offset, uint8_t *buf, size_t size) {
    size_t i;
    uint32_t addr = stm32f2_onchip_flash.addr + offset;
    for (i = 0; i < size; i++, addr++, buf++) {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size) {
    size_t i;
    uint32_t read_data;
    uint32_t addr = stm32f2_onchip_flash.addr + offset;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL);
    for (i = 0; i < size; i++, buf++, addr++) {
        /* write data */
        if ((i & 0x3FFF) == 0) {
            rtos_delayms(10);
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, *buf);
        read_data = *(uint8_t *) addr;
        /* check data */
        if (read_data != *buf) {
            return -1;
        }
    }
    HAL_FLASH_Lock();

    return size;
}

static int erase(long offset, size_t size) {
    HAL_StatusTypeDef flash_status;
    FLASH_EraseInitTypeDef erase_init;
    size_t erased_size = 0;
    uint32_t sector_error = 0;
    uint32_t addr = stm32f2_onchip_flash.addr + offset;

    erase_init.TypeErase = TYPEERASE_SECTORS;
    erase_init.VoltageRange = VOLTAGE_RANGE_3;
    erase_init.NbSectors = 1;

    /* start erase */
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL);
    /* it will stop when erased size is greater than setting size */
    while (erased_size < size) {
        rtos_delayms(10);
        erase_init.Sector = stm32_get_sector(addr + erased_size);
        flash_status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
        if (flash_status != HAL_OK) {
            return -1;
        }
        erased_size += stm32_get_sector_size(erase_init.Sector);
    }
    HAL_FLASH_Lock();

    return size;
}

const struct fal_flash_dev stm32f2_onchip_flash =
{
    .name       = "stm32_onchip",
    .addr       = 0x08000000,
    .len        = 1024*1024,
    .blk_size   = 128*1024,
    .ops        = {init, read, write, erase},
    .write_gran = 8
};

