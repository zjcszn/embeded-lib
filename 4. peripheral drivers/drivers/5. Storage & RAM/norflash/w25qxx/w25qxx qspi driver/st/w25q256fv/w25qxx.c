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
#include "stdint.h"
#include "bsp_qspi.h"
#include "main.h"
#include "elog.h"

/*----------------------------------------------------------------------------*/
#define W25QXX_MODE_SPI 0
#define W25QXX_MODE_QPI 1

#define QE_MASK 0x02
#define BUSY_MASK 0x01
/*----------------------------------------------------------------------------*/
uint8_t w25qxx_mode = W25QXX_MODE_SPI;
uint8_t w25qxx_uid[8];
uint8_t w25qxx_buf[4096];
uint16_t w25qxx_mid = W25Q64;

extern QSPI_HandleTypeDef qspi_handle;

/*----------------------------------------------------------------------------*/

void w25qxx_hardware_unprotected(int val) {
    if (val) {
        LL_GPIO_SetPinMode(FLASH_IOX_PORT, FLASH_IO2_PIN, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetOutputPin(FLASH_IOX_PORT, FLASH_IO2_PIN);
    } else {
        LL_GPIO_SetPinMode(FLASH_IOX_PORT, FLASH_IO2_PIN, LL_GPIO_MODE_ALTERNATE);

    }
}

w25x_status_t w25qxx_init(void) {
    uint8_t sr1, sr2, sr3;

    /* STEP1: 初始化qspi */
    bsp_qspi_init();

    /* 复位 w25qxx， 进入SPI mode */
    w25qxx_reset();

    /* STEP2: 检查地址模式，设置4字节地址模式 */

    if (w25qxx_read_sr(1, &sr1) || w25qxx_read_sr(2, &sr2) || w25qxx_read_sr(3, &sr3)) {
        return W25X_ERR;
    }

    if ((sr1 & 0xFC) != 0) {
        w25qxx_hardware_unprotected(1);
        w25qxx_write_enable(1);
        w25qxx_write_sr(1, 0);  /* 清除 SRP0, TB, BP0~3 */
        w25qxx_hardware_unprotected(0);
    }

    if ((sr3 & 0x06) != 02) {
        
        uint8_t sr3_new = (sr3 & ~((uint8_t)0x04)) | (uint8_t)0x02; /* 清除WPS位，置位ADP */
        
        w25qxx_write_enable(1);
        w25qxx_write_sr(3, sr3_new); /* 置位 ADP, 上电进入4byte地址模式 */
    }

    w25qxx_enter_4_byte_address_mode();

    /* STEP3: 进入QPI模式*/

    if(((sr2 & 0x02) == 0) || ((sr2 & 0x40) == 0x40)) {
        sr2 |= 0x02;    /* 置位 QE */
        sr2 &= 0xbf;    /* 清除 cmp 位*/
        w25qxx_write_enable(1);
        w25qxx_write_sr(2, sr2);       
    }

    w25qxx_enter_qpi_mode();

    return W25X_OK;
}


/*
* @note Not Test Code
*
*/
w25x_status_t w25qxx_global_unlock(void) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = (w25qxx_mode == W25QXX_MODE_QPI) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_GLOBAL_UNLOCK;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
    log_e("w25qxx global unlock failed, HAL Status = %u", status);
    return W25X_ERR;

}




w25x_status_t w25qxx_enter_4_byte_address_mode(void) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = (w25qxx_mode == W25QXX_MODE_QPI) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_Enter4ByteAddress;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
    log_e("w25qxx enter 4 byte address mode failed, HAL Status = %u", status);
    return W25X_ERR;
}


w25x_status_t w25qxx_exit_qpi_mode(void) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.Instruction = W25X_ExitQPIMode;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE; 
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status != HAL_OK) {
        log_e("w25qxx exit qpi mode failed, HAL Status = %u", status);
        return W25X_ERR;
    }
    
    w25qxx_mode = W25QXX_MODE_SPI;
    return W25X_OK;
}

w25x_status_t w25qxx_enter_qpi_mode(void) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t dat, srx;
    
    QSPI_CommandTypeDef cmd;

    /* enter qpi mode */
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_EnterQPIMode;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;   
    cmd.DataMode = QSPI_DATA_NONE;   
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status != HAL_OK)
        goto error;
    
    /* set read parameters */
    w25qxx_mode = W25QXX_MODE_QPI;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.Instruction = W25X_SetReadParameters;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.NbData = 1;
    dat = 0x03 << 4;    /* 8 dummy clocks */
    
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status != HAL_OK) {
        goto error;
    }
    
    status = HAL_QSPI_Transmit(&qspi_handle, &dat, 100);
    if (status != HAL_OK) {
        goto error;
    }

    return W25X_OK;

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
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.DataMode = QSPI_DATA_1_LINE;
    }
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.NbData = 1;    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;

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
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status != HAL_OK) {
        goto error;
    }
    status = HAL_QSPI_Receive(&qspi_handle, buf, 100);
    if (status != HAL_OK) {
        goto error;
    }

    return W25X_OK;
    
error:
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
    cmd.NbData = 1;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
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
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status != HAL_OK) {
        goto error;
    }
    status = HAL_QSPI_Transmit(&qspi_handle, &dat, 100);
    if (status != HAL_OK) {
        goto error;
    }
    return W25X_OK;

error:
    log_e("w25qxx write sr[%u] failed, HAL Status = %u", srx, status);
    return W25X_ERR;
}


w25x_status_t w25qxx_write_enable(uint8_t en) {
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = (w25qxx_mode == W25QXX_MODE_QPI) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = en ? W25X_WriteEnable : W25X_WriteDisable;

    cmd.AddressMode = QSPI_ADDRESS_NONE;    
    cmd.DataMode = QSPI_DATA_NONE;    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
    log_e("w25qxx enable write failed, HAL Status = %u", status);
    return W25X_ERR;
}

w25x_status_t w25qxx_read_jedec_id(uint32_t *id) {
    uint8_t  p_data[3];
    
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode == W25QXX_MODE_QPI) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.DataMode = QSPI_DATA_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.DataMode = QSPI_DATA_1_LINE;
    }
    
    cmd.Instruction = W25X_JedecID;
    cmd.AddressMode = QSPI_ADDRESS_NONE;    
    cmd.NbData = 3;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK) {
        goto error;
    }
    if (HAL_QSPI_Receive(&qspi_handle, (uint8_t *)&p_data, 100) != HAL_OK) {
        goto error;
    }

    *id = ((uint32_t)p_data[0] << 16) | ((uint32_t)p_data[1] << 8) | p_data[2];

    log_d("w25qxx jedec id %X", *id);
    return W25X_OK;

error:
    log_e("w25qxx read jedec id failed");
    return W25X_ERR;
}


w25x_status_t w25qxx_read_unique_id(void) {
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode == W25QXX_MODE_QPI ) {
        goto error;
    }
    
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_ReadUniqueID;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;   
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 8;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;    
    cmd.DummyCycles = 5 * 8;    /* 5 dummy bytes */
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if (HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK) {
        goto error;
    }
    if (HAL_QSPI_Receive(&qspi_handle, w25qxx_uid, 100) != HAL_OK) {
        goto error;
    }

    log_d("w25qxx unique id: %02X%02X%02X%02X%02X%02X%02X%02X", w25qxx_uid[0], w25qxx_uid[1], w25qxx_uid[2], 
            w25qxx_uid[3], w25qxx_uid[4], w25qxx_uid[5], w25qxx_uid[6], w25qxx_uid[7]);  
    return W25X_OK;

error:
    log_e("w25qxx read unique id failed");
    return W25X_ERR;

}

w25x_status_t w25qxx_read(uint8_t *pbuf, uint32_t addr, uint32_t size) {
    HAL_StatusTypeDef status;
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

    cmd.Instruction = W25X_FastRead;
    cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;    
    cmd.DummyCycles = 8;
    
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
    log_e("w25qxx read failed, HAL status = %u, addr:%06X", status, cmd.Address);
    return W25X_ERR;
}


w25x_status_t w25qxx_page_program(uint8_t *pbuf, uint32_t addr, uint16_t size) {
    HAL_StatusTypeDef status = HAL_OK;
    QSPI_CommandTypeDef cmd;
    
    if (size > 256)
        goto error;
    
    if (w25qxx_mode == W25QXX_MODE_QPI) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
        cmd.DataMode = QSPI_DATA_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
        cmd.DataMode = QSPI_DATA_1_LINE;
    }

    cmd.Instruction = W25X_PageProgram;
    cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    cmd.Address = addr;
    cmd.NbData = size;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    w25qxx_write_enable(1);
    
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status != HAL_OK) {
        goto error;
    }

#if !W25X_USE_DMA
    status = HAL_QSPI_Transmit(&qspi_handle, pbuf, 1000);
    if (status != HAL_OK)
#else
    status = HAL_QSPI_Transmit_DMA(&qspi_handle, pbuf);
    if ((status != HAL_OK) || (bsp_qspi_wait_tc() != RET_OK))
#endif
    {
        goto error;
    }

    if (w25qxx_wait_busy() != W25X_OK) {
        goto error;
    }

    return W25X_OK;

error:
    log_e("w25qxx page program failed, HAL status = %u", status);
    return W25X_ERR;
}


w25x_status_t w25qxx_write_nocheck (uint8_t *pbuf, uint32_t addr, uint32_t size) {
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
    
    cmd.InstructionMode = (w25qxx_mode == W25QXX_MODE_QPI) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = W25X_ChipErase;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if ((w25qxx_write_enable(1) != W25X_OK) || (w25qxx_wait_busy() != W25X_OK))
        goto error;
        
    if ((HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK) || (w25qxx_wait_busy() != W25X_OK)) {
        goto error;
    }
       
    return W25X_OK;

error:
    log_e("w25qxx chip erase failed");
    return W25X_ERR;
}


w25x_status_t w25qxx_sector_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode == W25QXX_MODE_QPI) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    }

    cmd.Instruction = W25X_SectorErase;
    cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_NONE;    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if ((w25qxx_write_enable(1) != W25X_OK) || (w25qxx_wait_busy() != W25X_OK))
        goto error;
        
    if ((HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK) || (w25qxx_wait_busy() != W25X_OK)) {
        goto error;
    }

    return W25X_OK;

error:
    log_e("w25qxx sector erase failed, addr: %08X", addr);
    return W25X_ERR;
}


w25x_status_t w25qxx_block32k_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode == W25QXX_MODE_QPI) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    }

    cmd.Instruction = W25X_BlockErase32;
    cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_NONE;    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if ((w25qxx_write_enable(1) != W25X_OK) || (w25qxx_wait_busy() != W25X_OK))
        goto error;
        
    if ((HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK) || (w25qxx_wait_busy() != W25X_OK)) {
        goto error;
    }
       
    return W25X_OK;

error:
    log_e("w25qxx block32 erase failed, addr: %08X", addr);
    return W25X_ERR;
}

w25x_status_t w25qxx_block64k_erase(uint32_t addr) {
    QSPI_CommandTypeDef cmd;
    
    if (w25qxx_mode == W25QXX_MODE_QPI) {
        cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    } else {
        cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    }

    cmd.Instruction = W25X_BlockErase64;
    cmd.AddressSize = QSPI_ADDRESS_32_BITS;
    cmd.Address = addr;
    cmd.DataMode = QSPI_DATA_NONE;    
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;    
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    if ((w25qxx_write_enable(1) != W25X_OK) || (w25qxx_wait_busy() != W25X_OK))
        goto error;
        
    if ((HAL_QSPI_Command(&qspi_handle, &cmd, 100) != HAL_OK) || (w25qxx_wait_busy() != W25X_OK)) {
        goto error;
    }
       
    return W25X_OK;

error:
    log_e("w25qxx block64 erase failed, addr: %08X", addr);
    return W25X_ERR;
}

w25x_status_t w25qxx_wait_busy(void) {
    QSPI_CommandTypeDef s_command = {0};
    QSPI_AutoPollingTypeDef s_config = {0};
    
    /**
     * ref:https://community.st.com/t5/stm32-mcus-products/hal-qspi-autopolling-fail-when-polling-the-sr-of-nor-flash-in/td-p/216569
     * 为确保数据信号从输出模式转变为输入模式有足够的周转时间，使用双线/四线模式时，必须至少设置1个dummy cycles
     */
    if (w25qxx_mode == W25QXX_MODE_QPI) {
        s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        s_command.DataMode = QSPI_DATA_4_LINES;
    } else {
        s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        s_command.DataMode = QSPI_DATA_1_LINE;
    }
    
    s_command.Instruction = W25X_ReadStatusReg1;
    s_command.DummyCycles = 2;
    s_command.AddressMode = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;

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
    HAL_StatusTypeDef status;
    QSPI_CommandTypeDef cmd;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.Instruction = W25X_EnableReset;
    
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.DataMode = QSPI_DATA_NONE;   
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DummyCycles = 0;
    
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status != HAL_OK) {
        goto error;
    }
    cmd.Instruction = W25X_ResetDevice;
    status = HAL_QSPI_Command(&qspi_handle, &cmd, 100);
    if (status != HAL_OK) {
        goto error;
    }

    w25qxx_wait_busy();     /* fixed by zjcszn, 20241122, reset, then wait bsy flag clear */

    w25qxx_mode = W25QXX_MODE_SPI;
    return W25X_OK;

error:    
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
    s_command.AddressSize = QSPI_ADDRESS_32_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode = QSPI_DATA_4_LINES;
    s_command.DummyCycles = 8;
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    
    /* Configure the memory mapped mode */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod = 0;
    
    status = HAL_QSPI_MemoryMapped(&qspi_handle, &s_command, &s_mem_mapped_cfg);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
    log_e("w25qxx memory mapped failed, HAL status = %u", status);
    return W25X_ERR;
}
