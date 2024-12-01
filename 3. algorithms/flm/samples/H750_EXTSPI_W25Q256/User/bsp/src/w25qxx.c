

#include "w25qxx.h"
#include "stdint.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_pwr.h"
#include "stm32h7xx_hal_qspi.h"
#include "bsp_qspi_w25q256.h"
#include "stm32h7xx_ll_gpio.h"

/*----------------------------------------------------------------------------*/
#define W25QXX_MODE_SPI 0
#define W25QXX_MODE_QPI 1

#define QE_MASK 0x02
#define BUSY_MASK 0x01
/*----------------------------------------------------------------------------*/
uint8_t w25qxx_mode = W25QXX_MODE_SPI;


extern QSPI_HandleTypeDef QSPIHandle;

/*----------------------------------------------------------------------------*/

void w25qxx_hardware_unprotected(int val) {
    if (val) {
        LL_GPIO_SetPinMode(GPIOF, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetOutputPin(GPIOF, LL_GPIO_PIN_7);
    } else {
        LL_GPIO_SetPinMode(GPIOF, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
    }
}

w25x_status_t w25qxx_init(void) {
    uint8_t sr1, sr2, sr3;


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

    if ((sr3 & 0x02) == 0) {
        w25qxx_write_enable(1);
        w25qxx_write_sr(3, sr3 | 0x02); /* 置位 ADP, 上电进入4byte地址模式 */
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
    
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
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
    
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status != HAL_OK) {
        return W25X_ERR;
    }
    
    w25qxx_mode = W25QXX_MODE_SPI;
    return W25X_OK;
}

w25x_status_t w25qxx_enter_qpi_mode(void) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t dat;
    
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
    
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status != HAL_OK)
        goto error;
    
    /* set read parameters */
    w25qxx_mode = W25QXX_MODE_QPI;
    
    cmd.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    cmd.Instruction = W25X_SetReadParameters;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.NbData = 1;
    dat = 0x03 << 4;    /* 8 dummy clocks */
    
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status != HAL_OK) {
        goto error;
    }
    
    status = HAL_QSPI_Transmit(&QSPIHandle, &dat, 0);
    if (status != HAL_OK) {
        goto error;
    }

    return W25X_OK;

error:
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
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status != HAL_OK) {
        goto error;
    }
    status = HAL_QSPI_Receive(&QSPIHandle, buf, 0);
    if (status != HAL_OK) {
        goto error;
    }

    return W25X_OK;
    
error:
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
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status != HAL_OK) {
        goto error;
    }
    status = HAL_QSPI_Transmit(&QSPIHandle, &dat, 0);
    if (status != HAL_OK) {
        goto error;
    }
    return W25X_OK;

error:
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
    
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
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
    
    if (HAL_QSPI_Command(&QSPIHandle, &cmd, 0) != HAL_OK) {
        goto error;
    }
    if (HAL_QSPI_Receive(&QSPIHandle, (uint8_t *)&p_data, 0) != HAL_OK) {
        goto error;
    }

    *id = ((uint32_t)p_data[0] << 16) | ((uint32_t)p_data[1] << 8) | p_data[2];

    return W25X_OK;

error:
    return W25X_ERR;
}



w25x_status_t w25qxx_read(uint8_t *pbuf, uint32_t addr, uint32_t size) {
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

    /* polling mode */
    cmd.Address = addr;
    cmd.NbData = size;
    if (HAL_QSPI_Command(&QSPIHandle, &cmd, 0) == HAL_OK) {
        if (HAL_QSPI_Receive(&QSPIHandle, pbuf, 0) == HAL_OK) {
            return W25X_OK;
        }
    }

    return W25X_ERR;
}


w25x_status_t w25qxx_page_program(uint8_t *pbuf, uint32_t addr, uint16_t size) {
    HAL_StatusTypeDef status = HAL_OK;
    QSPI_CommandTypeDef cmd;
    
    if (size > 256)
        goto error;
		
		w25qxx_write_enable(1);
    
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
    
    
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status != HAL_OK) {
        goto error;
    }

    status = HAL_QSPI_Transmit(&QSPIHandle, pbuf, 0);
    if (status != HAL_OK)

    {
        goto error;
    }

    if (w25qxx_wait_busy() != W25X_OK) {
        goto error;
    }

    return W25X_OK;

error:
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
        
    if ((HAL_QSPI_Command(&QSPIHandle, &cmd, 0) != HAL_OK) || (w25qxx_wait_busy() != W25X_OK)) {
        goto error;
    }
       
    return W25X_OK;

error:
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
        
    if ((HAL_QSPI_Command(&QSPIHandle, &cmd, 0) != HAL_OK) || (w25qxx_wait_busy() != W25X_OK)) {
        goto error;
    }

    return W25X_OK;

error:
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
        
    if ((HAL_QSPI_Command(&QSPIHandle, &cmd, 0) != HAL_OK) || (w25qxx_wait_busy() != W25X_OK)) {
        goto error;
    }
       
    return W25X_OK;

error:
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
    
    if (HAL_QSPI_AutoPolling(&QSPIHandle, &s_command, &s_config, 0) != HAL_OK) {
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
    
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status != HAL_OK) {
        goto error;
    }
    cmd.Instruction = W25X_ResetDevice;
    status = HAL_QSPI_Command(&QSPIHandle, &cmd, 0);
    if (status != HAL_OK) {
        goto error;
    }

    w25qxx_mode = W25QXX_MODE_SPI;
    
    w25qxx_wait_busy();
    return W25X_OK;

error:    
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
    
    status = HAL_QSPI_MemoryMapped(&QSPIHandle, &s_command, &s_mem_mapped_cfg);
    if (status == HAL_OK) {
        return W25X_OK;
    }
    
    return W25X_ERR;
}
