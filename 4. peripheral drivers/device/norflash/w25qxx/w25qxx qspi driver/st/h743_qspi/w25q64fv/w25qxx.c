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
#define LOG_LVL ELOG_LVL_INFO

#include "w25qxx.h"

#include "bsp_qspi.h"
#include "main.h"
#include "elog.h"

/*----------------------------------------------------------------------------*/
#define W25QXX_MODE_SPI 0
#define W25QXX_MODE_QPI 1

#define QE_MASK 0x02
#define BUSY_MASK 0x01
/*----------------------------------------------------------------------------*/
uint8_t w25qxx_mode = W25QXX_MODE_QPI;
uint8_t w25qxx_uid[8];
uint8_t w25qxx_buf[4096];
uint16_t w25qxx_mid = W25Q64;

/*----------------------------------------------------------------------------*/

void w25qxx_init(void) {
    w25qxx_exit_qpi_mode();
    w25qxx_reset();
    w25qxx_enter_qpi_mode();
}

w25x_status_t w25qxx_exit_qpi_mode(void) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.Instruction = W25X_ExitQPIMode;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0x00;
    
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status != HAL_OK) {
        log_e("w25qxx exit qpi mode failed, HAL Status = %u", status);
        return W25X_ERR;
    }
    
    w25qxx_mode = W25QXX_MODE_SPI;
    return W25X_OK;
}

w25x_status_t w25qxx_enter_qpi_mode(void) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t dat;
    
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_read_sr(2, &dat) != W25X_OK)
        goto error;
    
    if ((dat & QE_MASK) == 0x00) {
        dat |= QE_MASK;
        if (w25qxx_write_enable(1) != W25X_OK)
            goto error;
        if (w25qxx_write_sr(2, dat) != W25X_OK)
            goto error;
    }
    
    /* enter qpi mode */
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_EnterQPIMode;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0x00;
    
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status != HAL_OK)
        goto error;
    
    /* set read parameters */
    w25qxx_mode = W25QXX_MODE_QPI;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.Instruction = W25X_SetReadParameters;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.NbData = 1;
    dat = 0x03 << 4;
    
    if (w25qxx_write_enable(1) != W25X_OK)
        goto error;
    
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status == HAL_OK) {
        status = HAL_QSPI_Transmit(&dev_qspi.hqspi, &dat, 100);
        if (status == HAL_OK)
            return W25X_OK;
    }

error:
    log_e("w25qxx enter qpi mode failed, HAL status = %u", status);
    return W25X_ERR;
}

w25x_status_t w25qxx_read_sr(uint8_t srx, uint8_t *buf) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.DataMode = QSPI_DATA_4_LINES;
        cmd.DummyCycles = 2;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.DataMode = QSPI_DATA_1_LINE;
        cmd.DummyCycles = 0;
    }
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0x00;
    
    cmd.NbData = 1;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
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
            cmd.Instruction = W25X_ReadStatusReg1;
            break;
    }
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status == HAL_OK) {
        status = HAL_QSPI_Receive(&dev_qspi.hqspi, buf, 100);
        if (status == HAL_OK) {
            return W25X_OK;
        }
    }
    
    log_e("w25qxx read sr[%u] failed, HAL Status = %u", srx, status);
    return W25X_ERR;
}

w25x_status_t w25qxx_write_sr(uint8_t srx, uint8_t dat) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.DataMode = QSPI_DATA_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.DataMode = QSPI_DATA_1_LINE;
    }
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0x00;
    
    cmd.NbData = 1;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
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
            cmd.Instruction = W25X_WriteStatusReg1;
            break;
    }
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status == HAL_OK) {
        status = HAL_QSPI_Transmit(&dev_qspi.hqspi, &dat, 100);
        if (status == HAL_OK) {
            return W25X_OK;
        }
    }
    
    log_e("w25qxx write sr[%u] failed, HAL Status = %u", srx, status);
    return W25X_ERR;
}

w25x_status_t w25qxx_write_enable(uint8_t en) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    }
    
    if (en) {
        cmd.Instruction = W25X_WriteEnable;
    } else {
        cmd.Instruction = W25X_WriteDisable;
    }
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0x00;
    
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
    log_e("w25qxx enable write failed, HAL Status = %u", status);
    return W25X_ERR;
}

uint16_t w25qxx_read_mftr_id(void) {
    uint8_t pData[2];
    uint16_t MftrID = 0xEEEE;
    
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode == W25QXX_MODE_QPI) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
        cmd.DataMode = QSPI_DATA_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
        cmd.DataMode = QSPI_DATA_1_LINE;
    }
    
    cmd.Instruction = W25X_ManufacturerDeviceID;
    
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0x00;
    
    cmd.NbData = 2;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100) != HAL_OK) {
        return MftrID;
    }
    if (HAL_QSPI_Receive(&dev_qspi.hqspi, pData, 100) != HAL_OK) {
        return MftrID;
    }
    MftrID = (pData[0] << 8) | pData[1];
    
    return MftrID;
}

uint8_t w25qxx_read_unique_id(void) {
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        return 1;
    }
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_ReadUniqueID;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0x00;
    
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 8;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 32 - 1;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100) != HAL_OK) {
        return 1;
    }
    if (HAL_QSPI_Receive(&dev_qspi.hqspi, w25qxx_uid, 100) != HAL_OK) {
        return 1;
    }
    
    return 0;
}

w25x_status_t w25qxx_read(uint8_t *pbuf, uint32_t addr, uint32_t size) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
        cmd.DataMode = QSPI_DATA_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
        cmd.DataMode = QSPI_DATA_1_LINE;
    }
    cmd.Instruction = W25X_FastRead;
    
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 8;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

#if W25X_USE_DMA == 0
    /* polling mode */
    cmd.Address = addr;
    cmd.NbData = size;
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status == HAL_OK) {
        status = HAL_QSPI_Receive(&dev_qspi.hqspi, pbuf, 1000);
        if (status == HAL_OK) {
            return W25X_OK;
        }
    }
#else
    /* dma mode */
    for (uint32_t btr = 0, pos = 0; size != 0; size -= btr, pos += btr) {
        
        btr = (size > 65536) ? 65536 : size;
        cmd.Address = addr + pos;
        cmd.NbData = btr;
        
        status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
        if (status != HAL_OK)
            goto error;
        
        status = HAL_QSPI_Receive_DMA(&dev_qspi.hqspi, pbuf + pos);
        if (status != HAL_OK)
            goto error;
        
        bsp_qspi_wait_tc();
    }
    return W25X_OK;
#endif

error:
    log_e("w25qxx read failed, HAL status = %u, addr:%06X", status, cmd.Address);
    return W25X_ERR;
}

w25x_status_t w25qxx_page_program(uint8_t *pbuf, uint32_t addr, uint16_t size) {
    HAL_StatusTypeDef status = HAL_OK;
    QSPI_CommandTypeDef cmd;
    
    if (size > 256)
        goto error;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
        cmd.DataMode = QSPI_DATA_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
        cmd.DataMode = QSPI_DATA_1_LINE;
    }
    cmd.Instruction = W25X_PageProgram;
    
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    
    cmd.NbData = size;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    w25qxx_write_enable(1);
    
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status == HAL_OK) {

#if W25X_USE_DMA == 0
        status = HAL_QSPI_Transmit(&dev_qspi.hqspi, pbuf, 1000);
#else
        status = HAL_QSPI_Transmit_DMA(&dev_qspi.hqspi, pbuf);
#endif
        if (status == HAL_OK) {
#if W25X_USE_DMA == 1
            bsp_qspi_wait_tc();
#endif
            w25qxx_wait_busy();
            return W25X_OK;
        }
    }

error:
    log_e("w25qxx page program failed, HAL status = %u", status);
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
        for (i = 0; i < sec_rem; i++) {
            if (W25QXX_BUF[sec_off + i] != 0xFF)
                break;
        }
        if (i < sec_rem) {
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
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    }
    cmd.Instruction = W25X_ChipErase;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0x00;
    
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable(1) != W25X_OK)
        return W25X_ERR;
    
    w25qxx_wait_busy();
    
    if (HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100) != HAL_OK) {
        return W25X_ERR;
    }
    
    w25qxx_wait_busy();
    
    return W25X_OK;
}

w25x_status_t w25qxx_sector_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    }
    cmd.Instruction = W25X_SectorErase;
    
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable(1) != W25X_OK)
        return W25X_ERR;
    
    w25qxx_wait_busy();
    
    if (HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100) != HAL_OK)
        return W25X_ERR;
    
    w25qxx_wait_busy();
    
    return W25X_OK;
}

w25x_status_t w25qxx_block32k_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    }
    cmd.Instruction = W25X_BlockErase32;
    
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable(1) != W25X_OK)
        return W25X_ERR;
    
    w25qxx_wait_busy();
    
    if (HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100) != HAL_OK)
        return W25X_ERR;
    
    w25qxx_wait_busy();
    
    return W25X_OK;
}

w25x_status_t w25qxx_block64k_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    }
    cmd.Instruction = W25X_BlockErase64;
    
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (w25qxx_write_enable(1) != W25X_OK)
        return W25X_ERR;
    
    w25qxx_wait_busy();
    
    if (HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100) != HAL_OK)
        return W25X_ERR;
    
    w25qxx_wait_busy();
    
    return W25X_OK;
}

void w25qxx_wait_busy(void) {
    QSPI_CommandTypeDef s_command = {0};
    QSPI_AutoPollingTypeDef s_config = {0};
    
    /**
     * ref:https://community.st.com/t5/stm32-mcus-products/hal-qspi-autopolling-fail-when-polling-the-sr-of-nor-flash-in/td-p/216569
     * 为确保数据信号从输出模式转变为输入模式有足够的周转时间，使用双线/四线模式时，必须至少设置1个dummy cycles
     */
    if (w25qxx_mode) {
        s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        s_command.DataMode = QSPI_DATA_4_LINES;
        s_command.DummyCycles = 2;
    } else {
        s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        s_command.DataMode = QSPI_DATA_1_LINE;
        s_command.DummyCycles = 0;
    }
    
    s_command.Instruction = W25X_ReadStatusReg1;
    
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.Address = 0x00;
    
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.AlternateBytesSize = 0;
    s_command.AlternateBytes = 0x00;
    
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    s_config.Mask = BUSY_MASK;
    s_config.Match = 0;
    s_config.MatchMode = QSPI_MATCH_MODE_AND;
    s_config.StatusBytesSize = 1;
    s_config.Interval = 0x10;
    s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;
    
    if (HAL_QSPI_AutoPolling_IT(&dev_qspi.hqspi, &s_command, &s_config) != HAL_OK) {
        Error_Handler(__FILE__, __LINE__);
    }
    bsp_qspi_wait_busy();
}

w25x_status_t w25qxx_reset(void) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    }
    cmd.Instruction = W25X_EnableReset;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = 0;
    
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytesSize = 0;
    cmd.AlternateBytes = 0x00;
    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    w25qxx_wait_busy();
    status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
    if (status == HAL_OK) {
        cmd.Instruction = W25X_ResetDevice;
        status = HAL_QSPI_Command(&dev_qspi.hqspi, &cmd, 100);
        if (status == HAL_OK) {
            return W25X_OK;
        }
    }
    
    log_e("w25qxx reset failed, HAL status = %u", status);
    return W25X_ERR;
}

w25x_status_t w25qxx_memory_mapped_enable(void) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef s_command;
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
    
    /* Configure the command for the read instruction */
    s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    s_command.Instruction = W25X_FastReadQuadIO;
    s_command.AddressMode = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_4_LINES;
    s_command.DummyCycles = 8;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    /* Configure the memory mapped mode */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod = 0;
    
    status = HAL_QSPI_MemoryMapped(&dev_qspi.hqspi, &s_command, &s_mem_mapped_cfg);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
    log_e("w25qxx memory mapped failed, HAL status = %u", status);
    return W25X_ERR;
}
