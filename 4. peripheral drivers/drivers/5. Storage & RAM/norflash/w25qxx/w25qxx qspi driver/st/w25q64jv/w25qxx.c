/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-15     whj4674672   first version
 */

#define LOG_TAG "W25QXX"
#define LOG_LVL ELOG_LVL_DEBUG

#include "w25qxx.h"

#include "bsp_qspi.h"
#include "main.h"
// #include "elog.h"
// #include "perf_counter.h"

/*----------------------------------------------------------------------------*/
#define W25QXX_MODE_SPI 0
#define W25QXX_MODE_QPI 1

#define QE_MASK   0x02
#define BUSY_MASK 0x01
#define SUS_MASK  0x80
/*----------------------------------------------------------------------------*/

extern QSPI_HandleTypeDef qspi_handle;

static __attribute__ ((aligned (32)))  uint8_t w25qxx_buf[4096] __attribute__((section(".AXI")));

/*----------------------------------------------------------------------------*/

w25x_status_t w25qxx_init(void) {
    uint32_t id;
    uint64_t unique_id;
    
    bsp_qspi_init();

    w25qxx_reset();
    
    id = w25qxx_read_jedec_id();
    if (id != FLASH_ID_W25Q64) {
        log_e("w25qxx init failed, flash id: %06X", id);
        return W25X_ERR;
    }
    log_i("w25qxx init success, flash id: %06X", id);
    
    unique_id = w25qxx_read_unique_id();
    log_i("w25qxx unique id: %016llX", unique_id);
    
    return W25X_OK;
}

w25x_status_t w25qxx_read_sr(uint8_t srx, uint8_t *buf) {
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    switch (srx) {
        case 1:
            cmd.Instruction = W25X_ReadStatusReg1;
            break;
        case 2:
            cmd.Instruction = W25X_ReadStatusReg2;
            break;
        case 3:
            cmd.Instruction = W25X_ReadStatusReg3;
            break;
        default:
            goto error;
    }
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;
    if (HAL_QSPI_Receive(&qspi_handle, buf, 100) != HAL_OK)
        goto error;
    
    return W25X_OK;

error:
    log_e("w25qxx read sr[%u] failed", srx);
    return W25X_ERR;
}

w25x_status_t w25qxx_write_sr(uint8_t srx, uint8_t dat) {
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    switch (srx) {
        case 1:
            cmd.Instruction = W25X_WriteStatusReg1;
            break;
        case 2:
            cmd.Instruction = W25X_WriteStatusReg2;
            break;
        case 3:
            cmd.Instruction = W25X_WriteStatusReg3;
            break;
        default:
            goto error;
    }
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;
    if (HAL_QSPI_Transmit(&qspi_handle, &dat, 100) != HAL_OK)
        goto error;
    
    return W25X_OK;

error:
    log_e("w25qxx write sr[%u] failed", srx);
    return W25X_ERR;
}

w25x_status_t w25qxx_write_enable(void) {
    QSPI_CommandTypeDef s_command;
    QSPI_AutoPollingTypeDef s_config;
    
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = W25X_WriteEnable;
    
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DummyCycles = 0;
    s_command.DataMode = QSPI_DATA_NONE;
    
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (HAL_QSPI_Command(&qspi_handle, &s_command, 100) != HAL_OK)
        goto error;
    
    s_config.Match = W25X_SR1_MASK_WEL;
    s_config.Mask = W25X_SR1_MASK_WEL;
    s_config.MatchMode = QSPI_MATCH_MODE_AND;
    s_config.Interval = 0x10;
    s_config.StatusBytesSize = 1;
    s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;
    
    s_command.Instruction = W25X_ReadStatusReg1;
    s_command.DataMode = QSPI_DATA_1_LINE;
    
    if (HAL_QSPI_AutoPolling_IT(&qspi_handle, &s_command, &s_config) != HAL_OK)
        goto error;
    
    bsp_qspi_wait_status_match();
    return W25X_OK;

error:
    log_e("w25qxx write enable failed");
    return W25X_ERR;
}

uint32_t w25qxx_read_jedec_id(void) {
    uint8_t pData[3];
    uint32_t jedec_id = 0;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_JedecID;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 3;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) == HAL_OK) {
        if (HAL_QSPI_Receive(&qspi_handle, pData, 100) == HAL_OK) {
            jedec_id = (pData[0] << 16) | (pData[1] << 8) | pData[2];
        }
    }
    return jedec_id;
}

uint64_t w25qxx_read_unique_id(void) {
    uint64_t unique_id = 0;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_ReadUniqueID;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    
    cmd.DummyCycles = 31;
    
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 8;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) == HAL_OK) {
        HAL_QSPI_Receive(&qspi_handle, (uint8_t *)&unique_id, 100);
    }
    return unique_id;
}

w25x_status_t w25qxx_read(uint8_t *pbuf, uint32_t addr, uint32_t size) {

    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_FastReadQuadIO;
    
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    cmd.AlternateBytes = 0xFF;
    
    cmd.DummyCycles = 4;
    
    cmd.DataMode = QSPI_DATA_4_LINES;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

#if W25X_USE_DMA == 0
    /* polling mode */
    cmd.Address = addr;
    cmd.NbData = size;
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) == HAL_OK) {
        if (HAL_QSPI_Receive(&qspi_handle, pbuf, 1000) == HAL_OK) {
            return W25X_OK;
        }
    }
#else
    uint32_t cnt = size;

    /* dma mode */
    for (uint32_t btr = 0, pos = 0; cnt != 0; cnt -= btr, pos += btr) {
        
        btr = (cnt > 65536) ? 65536 : cnt;
        cmd.Address = addr + pos;
        cmd.NbData = btr;
        
        if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
            goto error;
        
        if (HAL_QSPI_Receive_DMA(&qspi_handle, pbuf + pos) != HAL_OK)
            goto error;
        
        if (bsp_qspi_wait_tc() != RET_OK) 
            goto error;
    }

    SCB_InvalidateDCache_by_Addr((uint32_t *)pbuf, (int32_t)size);
    return W25X_OK;
#endif

error:
    log_e("w25qxx read failed, addr:%06X", cmd.Address);
    return W25X_ERR;
}

w25x_status_t w25qxx_page_program(uint8_t *pbuf, uint32_t addr, uint16_t size) {
    QSPI_CommandTypeDef cmd;
    
    if (size > 256)
        goto error;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_QuadPageProgram;
    
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.NbData = size;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable() != W25X_OK)
        goto error;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;

#if !W25X_USE_DMA
    if (HAL_QSPI_Transmit(&qspi_handle, pbuf, 1000) != HAL_OK)
#else
    if (HAL_QSPI_Transmit_DMA(&qspi_handle, pbuf) != HAL_OK)
#endif
    {
        goto error;
    }

#if W25X_USE_DMA
    bsp_qspi_wait_tc();
#endif
    
    if (w25qxx_wait_busy() != W25X_OK)
        goto error;

    return W25X_OK;

error:
    log_e("w25qxx page program failed");
    return W25X_ERR;
}

w25x_status_t w25qxx_write_nocheck(uint8_t *pbuf, uint32_t addr, uint32_t size) {
    uint32_t page_rem;
    page_rem = 256 - addr % 256;
    if (size <= page_rem) {
        page_rem = size;
    }
    while (1) {
        if (w25qxx_page_program(pbuf, addr, page_rem) != W25X_OK)
            return W25X_ERR;
        if (size == page_rem) {
            break;
        } else {
            pbuf += page_rem;
            addr += page_rem;
            
            size -= page_rem;
            if (size > 256) {
                page_rem = 256;
            } else {
                page_rem = size;
            }
        }
    }
    return W25X_OK;
}

w25x_status_t w25qxx_write(uint8_t *pbuf, uint32_t addr, uint32_t size) {
    uint32_t sec_pos;
    uint32_t sec_off;
    uint32_t sec_rem;
    uint32_t i;
    uint8_t *W25QXX_BUF;
    
    W25QXX_BUF = w25qxx_buf;
    sec_pos = addr / 4096;
    sec_off = addr % 4096;
    sec_rem = 4096 - sec_off;
    
    if (size <= sec_rem) {
        sec_rem = size;
    }
    while (1) {
        if (w25qxx_read(W25QXX_BUF, sec_pos * 4096, 4096) != W25X_OK)
            return W25X_ERR;

#if W25X_USE_DMA
        /* clean cache */
        SCB_InvalidateDCache_by_Addr((uint32_t *)W25QXX_BUF, 4096);
#endif
        
        for (i = 0; i < sec_rem; i++) {
            if (W25QXX_BUF[sec_off + i] != 0xFF)
                break;
        }
        if (i < sec_rem) {
            /* debug: sector erase must use 24bit address(sec_pos * 4096), not sector id */
            if (w25qxx_sector_erase(sec_pos * 4096) != W25X_OK)
                return W25X_ERR;
            
            for (i = 0; i < sec_rem; i++) {
                W25QXX_BUF[sec_off + i] = pbuf[i];
            }
            
            if (w25qxx_write_nocheck(W25QXX_BUF, sec_pos * 4096, 4096) != W25X_OK)
                return W25X_ERR;
        } else {
            if (w25qxx_write_nocheck(pbuf, addr, sec_rem) != W25X_OK)
                return W25X_ERR;
        }
        if (size == sec_rem) {
            break;
        } else {
            sec_pos++;
            sec_off = 0;
            
            pbuf += sec_rem;
            addr += sec_rem;
            size -= sec_rem;
            if (size > 4096) {
                sec_rem = 4096;
            } else {
                sec_rem = size;
            }
        }
    }
    return W25X_OK;
}

w25x_status_t w25qxx_chip_erase(void) {
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_ChipErase;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DataMode = QSPI_DATA_NONE;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable() != W25X_OK)
        goto error;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;
    
    if (w25qxx_wait_busy() != W25X_OK)
        goto error;

    return W25X_OK;

error:
    log_e("w25qxx chip erase failed");
    return W25X_ERR;
}

w25x_status_t w25qxx_sector_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_SectorErase;
    
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    cmd.DataMode = QSPI_DATA_NONE;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable() != W25X_OK)
        goto error;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;
    
    if (w25qxx_wait_busy() != W25X_OK)
        goto error;

    return W25X_OK;

error:
    log_e("w25qxx sector erase failed");
    return W25X_ERR;
}

w25x_status_t w25qxx_block32k_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_BlockErase32;
    
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    cmd.DataMode = QSPI_DATA_NONE;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable() != W25X_OK)
        goto error;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;
    
    if (w25qxx_wait_busy() != W25X_OK)
        goto error;

    return W25X_OK;

error:
    log_e("w25qxx sector 32k erase failed");
    return W25X_ERR;
}

w25x_status_t w25qxx_block64k_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_BlockErase64;
    
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    cmd.DataMode = QSPI_DATA_NONE;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable() != W25X_OK)
        goto error;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;
    
    if (w25qxx_wait_busy() != W25X_OK)
        goto error;

    return W25X_OK;

error:
    log_e("w25qxx sector 64k erase failed");
    return W25X_ERR;
}

w25x_status_t w25qxx_wait_busy(void) {
    QSPI_CommandTypeDef s_command = {0};
    QSPI_AutoPollingTypeDef s_config = {0};
    
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = W25X_ReadStatusReg1;
    
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DummyCycles = 0;
    
    s_command.DataMode = QSPI_DATA_1_LINE;
    
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    s_config.Mask = BUSY_MASK;
    s_config.Match = 0;
    s_config.MatchMode = QSPI_MATCH_MODE_AND;
    s_config.StatusBytesSize = 1;
    s_config.Interval = 0x10;
    s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;
    
    if (HAL_QSPI_AutoPolling_IT(&qspi_handle, &s_command, &s_config) != HAL_OK) {
        return W25X_ERR;
    }
    if (bsp_qspi_wait_status_match() != RET_OK){
        return W25X_ERR;
    }

    return W25X_OK;
}

w25x_status_t w25qxx_reset(void) {
    uint8_t sr1, sr2;
    uint32_t now;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    /* check busy & sus status before reset */
    now = HAL_GetTick();
    do {
        w25qxx_read_sr(1, &sr1);
        w25qxx_read_sr(2, &sr2);
        if ((HAL_GetTick() - now) > HAL_QSPI_TIMEOUT_DEFAULT_VALUE) {
            log_e("w25qxx wait busy & sus status timeout");
            break;
        }
    } while ((sr1 & BUSY_MASK) || (sr2 & SUS_MASK));
    
    cmd.Instruction = W25X_EnableReset;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;
    
    cmd.Instruction = W25X_ResetDevice;
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK)
        goto error;
    
    /* delay at least 30us after reset */
    HAL_Delay(1);
    
    log_i("w25qxx reset success");
    return W25X_OK;

error:
    log_e("w25qxx reset failed");
    return W25X_ERR;
}

w25x_status_t w25qxx_memory_mapped_enable(void) {
    QSPI_CommandTypeDef s_command;
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
    
    /* Configure the command for the read instruction */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction = W25X_FastReadQuadIO;
    s_command.AddressMode = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    s_command.AlternateBytes = 0xFF;
    s_command.DummyCycles = 4;
    s_command.DataMode = QSPI_DATA_4_LINES;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    /* Configure the memory mapped mode */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod = 0;
    
    if (HAL_QSPI_MemoryMapped(&qspi_handle, &s_command, &s_mem_mapped_cfg) != HAL_OK) {
        log_e("w25qxx memory mapped failed");
        return W25X_ERR;
    }
    
    return W25X_OK;
}





#if W25X_TEST

#include <stdlib.h>

uint8_t w25_test_buf[W25X_TEST_BUFSZ];
uint8_t w25_read_buf[W25X_TEST_BUFSZ];


void w25qxx_test(void) {
    uint64_t start;
    uint64_t spend_time;
    bool test_ok = true;

    log_d("------- W25QXX TEST START -------");

    srand(NULL);

    for (int i = 0; i < W25X_TEST_BUFSZ; i++) {
        w25_test_buf[i] = rand() & 0xFF;
    }

    start = get_system_ms();
    w25qxx_chip_erase();
    spend_time = get_system_ms() - start;
    log_d("chip erase spend %llu ms", spend_time);

    start = get_system_us();
    w25qxx_write(w25_test_buf, W25X_TEST_ADDR, W25X_TEST_BUFSZ);
    spend_time = (uint64_t)get_system_us() - start;
    log_d("[pre-erase] write 128k spend %llu us, speed: %.2f KB/S", spend_time, ((double)W25X_TEST_BUFSZ / 1024.0) / ((double) spend_time / 1000000.0));

    start = get_system_us();
    w25qxx_read(w25_read_buf, W25X_TEST_ADDR, W25X_TEST_BUFSZ);
    spend_time = (uint64_t)get_system_us() - start;
    log_d("[pre-erase] read  128k spend %llu us, speed: %.2f KB/S", spend_time, ((double)W25X_TEST_BUFSZ / 1024.0) / ((double) spend_time / 1000000.0));

    for (int i = 0; i < W25X_TEST_BUFSZ; i++) {
        if (w25_test_buf[i] != w25_read_buf[i]) {
            test_ok = false;
            log_e("[pre-erase] w25qxx write read test: [failed], idx: %u, w: %02X, r: %02X", i, w25_test_buf[i], w25_read_buf[i]);
            break;
        }
    }

    if (test_ok) log_d("[pre-erase] w25qxx write read test: [PASS]");

    srand(get_system_ms());
    for (int i = 0; i < W25X_TEST_BUFSZ; i++) {
        w25_test_buf[i] = rand() & 0xFF;
    }

    start = get_system_us(); 
    w25qxx_write(w25_test_buf, W25X_TEST_ADDR, W25X_TEST_BUFSZ);
    spend_time = (uint64_t)get_system_us() - start;
    log_d("[no pre-erase] write 128k spend %llu us, speed: %.2f KB/S", spend_time, ((double)W25X_TEST_BUFSZ / 1024.0) / ((double) spend_time / 1000000.0));
    w25qxx_read(w25_read_buf, W25X_TEST_ADDR, W25X_TEST_BUFSZ);

    test_ok = true;
    for (int i = 0; i < W25X_TEST_BUFSZ; i++) {
        if (w25_test_buf[i] != w25_read_buf[i]) {
            test_ok = false;
            log_e("[no pre-erase] w25qxx write read test: [failed], idx: %u, w: %02X, r: %02X", i, w25_test_buf[i], w25_read_buf[i]);
            break;
        }
    }

    if (test_ok) log_d("[no pre-erase] w25qxx write read test: [PASS]");

    w25qxx_memory_mapped_enable();
    volatile uint32_t tmp;
    uint32_t *flash_addr = (uint32_t *)0x90000000;
    start = get_system_ms(); 
    for (int i = 0; i < W25Q_CAPACITY / sizeof(uint32_t) / 32; i++) {
        tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++;
        tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++;
        tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++;
        tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++;

        tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++;
        tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++;
        tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++;
        tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++; tmp = *flash_addr++;
    }
    spend_time = (uint64_t)get_system_ms() - start;
    log_d("[memory-mapped] read %u MB spend %llu ms, speed: %.2f MB/S", W25Q_CAPACITY / 1024 / 1024, spend_time, ((double)(W25Q_CAPACITY / 1024 / 1024)) / ((double) spend_time / 1000.0));
   

    log_d("----- W25QXX TEST COMPLETE ------");
}


#endif

