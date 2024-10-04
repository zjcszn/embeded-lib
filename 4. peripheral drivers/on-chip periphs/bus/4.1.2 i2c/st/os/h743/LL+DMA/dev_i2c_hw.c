//
// Created by zjcszn on 2024/3/11.
//

#include "dev_i2c_hw.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32h7xx_ll_i2c.h"
#include "stm32h7xx_ll_dma.h"

#define I2C_LOCK()                                      \
    do {                                                \
        xSemaphoreTake(hi2c.lock, portMAX_DELAY);    \
    } while (0U)

#define I2C_UNLOCK()                                    \
    do {                                                \
        xSemaphoreGive(hi2c.lock);                   \
    } while (0U)

#define I2C_IT_ERR_NACK_TC_STOP_MASK       (I2C_CR1_ERRIE | I2C_CR1_NACKIE | I2C_CR1_TCIE | I2C_CR1_STOPIE)
#define I2C_IT_FLAG_CLEAR()                 I2C1->ICR = 0x00003F38
#define I2C_IT_ENABLE_ERR_NACK_TC_STOP()    MODIFY_REG(I2C1->CR1, I2C_IT_ERR_NACK_TC_STOP_MASK, I2C_IT_ERR_NACK_TC_STOP_MASK)
#define I2C_IT_DISABLE_ERR_NACK_TC_STOP()   MODIFY_REG(I2C1->CR1, I2C_IT_ERR_NACK_TC_STOP_MASK, 0UL)
#define I2C_IT_ENABLE_TXE()                 MODIFY_REG(I2C1->CR1, I2C_CR1_TXIE, I2C_CR1_TXIE)
#define I2C_IT_DISABLE_TXE()                MODIFY_REG(I2C1->CR1, I2C_CR1_TXIE, 0UL)
#define I2C_IT_ENABLE_TC()                  MODIFY_REG(I2C1->CR1, I2C_CR1_TCIE, I2C_CR1_TCIE)
#define I2C_IT_DISABLE_TC()                 MODIFY_REG(I2C1->CR1, I2C_CR1_TCIE, 0UL)

static struct {
    __IO i2c_state_t state;
    __IO uint8_t saddr;
    __IO uint32_t raddr;
    __IO uint8_t raddr_len;
    __IO uint8_t xfer_size;
    __IO uint16_t xfer_count;
    __IO uint32_t xfer_mode;
    __IO uint32_t err;
    SemaphoreHandle_t lock;
    SemaphoreHandle_t xfer_tc;
} hi2c;

uint32_t dev_i2c_init(void) {
    hi2c.lock = xSemaphoreCreateMutex();
    hi2c.xfer_tc = xSemaphoreCreateBinary();
    if (!hi2c.lock || !hi2c.xfer_tc) {
        return I2C_ERR_NOMEM;
    }
    bsp_i2c_init();
    return I2C_OK;
}

static void i2c_wait_transfer(void) {
    if (xSemaphoreTake(hi2c.xfer_tc, pdMS_TO_TICKS(2000)) != pdTRUE) {
        I2C_IT_DISABLE_ERR_NACK_TC_STOP();
        hi2c.err |= I2C_ERR_TIMEOUT;
        hi2c.state = I2C_STATE_TRANSFER_ERROR;
    }
    if ((hi2c.state == I2C_STATE_TRANSFER_ERROR) || (hi2c.state == I2C_STATE_TRANSFER_COMPLETE)) {
        if (hi2c.xfer_mode == I2C_MODE_WRITE) {
            LL_I2C_DisableDMAReq_TX(I2C1);
            LL_DMA_DisableStream(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);
            LL_DMA_DisableIT_TE(I2C1_TX_DMA, I2C1_TX_DMA_STREAM);
        } else {
            LL_I2C_DisableDMAReq_RX(I2C1);
            LL_DMA_DisableStream(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);
            LL_DMA_DisableIT_TE(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);
        }
        LL_I2C_ClearFlag_STOP(I2C1);
    }
}

uint32_t i2c_mem_write_dma(uint8_t saddr, uint32_t raddr, uint16_t raddr_len, uint8_t *pbuf, uint16_t nbytes) {
    if (!pbuf || (raddr_len > 4) || (bsp_i2c_bus_check() != 0))
        return I2C_ERR_PARAM;
    
    I2C_LOCK();
    
    hi2c.saddr = (saddr & 0x7F) << 1;
    hi2c.state = I2C_STATE_SEND_REG_ADDR;
    hi2c.raddr = raddr;
    hi2c.raddr_len = raddr_len;
    hi2c.xfer_count = nbytes;
    hi2c.xfer_mode = I2C_MODE_WRITE;
    hi2c.xfer_size = (nbytes > 0xFF) ? 0xFF : nbytes;
    hi2c.err = I2C_OK;
    
    /* 1. send reg address */
    I2C_IT_FLAG_CLEAR();
    I2C_IT_ENABLE_ERR_NACK_TC_STOP();
    LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, raddr_len, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_START_WRITE);
    I2C_IT_ENABLE_TXE();
    i2c_wait_transfer();
    if (hi2c.err != I2C_OK) {
        goto exit;
    }
    
    /* 2. send data */
    bsp_i2c_dmatx_config(pbuf, nbytes);
    if (hi2c.xfer_size == nbytes) {
        LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, hi2c.xfer_size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_NOSTARTSTOP);
    } else {
        LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, hi2c.xfer_size, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_NOSTARTSTOP);
        I2C_IT_ENABLE_TC();
    }
    i2c_wait_transfer();

exit:
    I2C_UNLOCK();
    return hi2c.err;
}

uint32_t i2c_mem_read_dma(uint8_t saddr, uint32_t raddr, uint16_t raddr_len, uint8_t *pbuf, uint16_t nbytes) {
    if (!pbuf || (raddr_len > 4) || (bsp_i2c_bus_check() != 0))
        return I2C_ERR_PARAM;
    
    I2C_LOCK();
    
    hi2c.saddr = (saddr & 0x7F) << 1;
    hi2c.state = I2C_STATE_SEND_REG_ADDR;
    hi2c.raddr = raddr;
    hi2c.raddr_len = raddr_len;
    hi2c.xfer_count = nbytes;
    hi2c.xfer_mode = I2C_MODE_READ;
    hi2c.xfer_size = (nbytes > 0xFF) ? 0xFF : nbytes;
    hi2c.err = I2C_OK;
    
    /* 1. send reg address */
    I2C_IT_FLAG_CLEAR();
    I2C_IT_ENABLE_ERR_NACK_TC_STOP();
    LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, raddr_len, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);
    I2C_IT_ENABLE_TXE();
    i2c_wait_transfer();
    if (hi2c.err != I2C_OK) {
        goto exit;
    }
    
    /* 2. recv data */
    bsp_i2c_dmarx_config(pbuf, nbytes);
    if (hi2c.xfer_size == nbytes) {
        LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, hi2c.xfer_size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);
    } else {
        LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, hi2c.xfer_size, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_RESTART_7BIT_READ);
        I2C_IT_ENABLE_TC();
    }
    i2c_wait_transfer();

exit:
    I2C_UNLOCK();
    return hi2c.err;
}

uint32_t i2c_write_dma(uint8_t saddr, uint8_t *pbuf, uint16_t nbytes) {
    if (!pbuf || (bsp_i2c_bus_check() != 0))
        return I2C_ERR_PARAM;
    
    I2C_LOCK();
    
    hi2c.saddr = (saddr & 0x7F) << 1;
    hi2c.state = I2C_STATE_TRANSFER_START;
    hi2c.raddr = 0;
    hi2c.raddr_len = 0;
    hi2c.xfer_count = nbytes;
    hi2c.xfer_mode = I2C_MODE_WRITE;
    hi2c.err = I2C_OK;
    
    I2C_IT_FLAG_CLEAR();
    I2C_IT_ENABLE_ERR_NACK_TC_STOP();
    bsp_i2c_dmatx_config(pbuf, nbytes);
    
    uint32_t i2c_mode;
    if (nbytes > 0xFF) {
        hi2c.xfer_size = 0xFF;
        i2c_mode = LL_I2C_MODE_RELOAD;
    } else {
        hi2c.xfer_size = nbytes;
        i2c_mode = LL_I2C_MODE_AUTOEND;
    }
    LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, hi2c.xfer_size, i2c_mode, LL_I2C_GENERATE_START_WRITE);
    i2c_wait_transfer();
    
    I2C_UNLOCK();
    return hi2c.err;
}

uint32_t i2c_read_dma(uint8_t saddr, uint8_t *pbuf, uint16_t nbytes) {
    if (!pbuf || (bsp_i2c_bus_check() != 0))
        return I2C_ERR_PARAM;
    
    I2C_LOCK();
    
    hi2c.saddr = (saddr & 0x7F) << 1;
    hi2c.state = I2C_STATE_TRANSFER_START;
    hi2c.raddr = 0;
    hi2c.raddr_len = 0;
    hi2c.xfer_count = nbytes;
    hi2c.xfer_mode = I2C_MODE_READ;
    hi2c.err = I2C_OK;
    
    I2C_IT_FLAG_CLEAR();
    I2C_IT_ENABLE_ERR_NACK_TC_STOP();
    bsp_i2c_dmarx_config(pbuf, nbytes);
    
    uint32_t i2c_mode;
    if (nbytes > 0xFF) {
        hi2c.xfer_size = 0xFF;
        i2c_mode = LL_I2C_MODE_RELOAD;
    } else {
        hi2c.xfer_size = nbytes;
        i2c_mode = LL_I2C_MODE_AUTOEND;
    }
    LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, hi2c.xfer_size, i2c_mode, LL_I2C_GENERATE_START_READ);
    i2c_wait_transfer();
    
    I2C_UNLOCK();
    return hi2c.err;
}

void I2C1_EVENT_IRQ_HANDLER(void) {
    uint32_t cr1 = I2C1->CR1;
    uint32_t isr = I2C1->ISR;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    /* check TXE flag, to send register address */
    if ((cr1 & I2C_CR1_TXIE) && (isr & I2C_ISR_TXE)) {
        if (hi2c.raddr_len > 0) {
            hi2c.raddr_len--;
            I2C1->TXDR = (hi2c.raddr >> ((hi2c.raddr_len) * 8)) & 0xFF;
        } else {
            I2C_IT_DISABLE_TXE();
        }
    }
    /* check TC & TCR flag */
    if ((cr1 & I2C_CR1_TCIE) && (isr & (I2C_ISR_TCR | I2C_ISR_TC))) {
        if (hi2c.state == I2C_STATE_SEND_REG_ADDR) {
            I2C_IT_DISABLE_TC();
            xSemaphoreGiveFromISR(hi2c.xfer_tc, &xHigherPriorityTaskWoken);
            hi2c.state = I2C_STATE_TRANSFER_START;
        } else if (hi2c.state == I2C_STATE_TRANSFER_START) {
            hi2c.xfer_count -= hi2c.xfer_size;
            if (hi2c.xfer_count > 0xFF) {
                hi2c.xfer_size = 0xFF;
                LL_I2C_SetTransferSize(I2C1, hi2c.xfer_size);
            } else {
                hi2c.xfer_size = hi2c.xfer_count;
                LL_I2C_HandleTransfer(I2C1, hi2c.saddr, LL_I2C_ADDRSLAVE_7BIT, hi2c.xfer_size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_NOSTARTSTOP);
            }
        }
    }
    /* check NACK flag */
    if ((cr1 & I2C_CR1_NACKIE) && (isr & I2C_ISR_NACKF)) {
        hi2c.err |= I2C_ERR_NACK;
        /* if nack, hardware will auto genarate stop condition */
        I2C1->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;   /* clear nack & stop flag */
        xSemaphoreGiveFromISR(hi2c.xfer_tc, &xHigherPriorityTaskWoken);
        hi2c.state = I2C_STATE_TRANSFER_ERROR;
    }
    /* check STOP flag */
    if ((cr1 & I2C_CR1_STOPIE) && (isr & I2C_ISR_STOPF)) {
        I2C1->ICR = I2C_ISR_STOPF;  /* clear stop flag */
        /* if transfer is in write mode, release semaphore to notify user. if in read mode, wait dma tranfer complete */
        if (hi2c.xfer_mode == I2C_MODE_WRITE) {
            xSemaphoreGiveFromISR(hi2c.xfer_tc, &xHigherPriorityTaskWoken);
        }
        hi2c.state = I2C_STATE_TRANSFER_COMPLETE;
    }
    
    if ((hi2c.state == I2C_STATE_TRANSFER_ERROR) || (hi2c.state == I2C_STATE_TRANSFER_COMPLETE)) {
        I2C_IT_DISABLE_ERR_NACK_TC_STOP();  /* disable nack err tc stop interrupt */
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void I2C1_ERROR_IRQ_HANDLER(void) {
    uint32_t isr = I2C1->ISR;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    hi2c.state = I2C_STATE_TRANSFER_ERROR;
    
    hi2c.err |= (isr & I2C_ISR_ARLO) ? I2C_ERR_ARLO : 0UL;
    hi2c.err |= (isr & I2C_ISR_BERR) ? I2C_ERR_BERR : 0UL;
    hi2c.err |= (isr & I2C_ISR_OVR) ? I2C_ERR_OVR : 0UL;
    
    I2C_IT_DISABLE_ERR_NACK_TC_STOP();
    xSemaphoreGiveFromISR(hi2c.xfer_tc, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void I2C1_RX_DMA_IRQ_HANDLER(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if (LL_DMA_IsEnabledIT_TE(I2C1_RX_DMA, I2C1_RX_DMA_STREAM) && LL_DMA_IsActiveFlag_TE5(I2C1_RX_DMA)) {
        LL_DMA_ClearFlag_TE5(I2C1_RX_DMA);
        hi2c.err |= I2C_ERR_DMA;
        hi2c.state = I2C_STATE_TRANSFER_ERROR;
        I2C_IT_DISABLE_ERR_NACK_TC_STOP();
        xSemaphoreGiveFromISR(hi2c.xfer_tc, &xHigherPriorityTaskWoken);
    }
    if (LL_DMA_IsEnabledIT_TC(I2C1_RX_DMA, I2C1_RX_DMA_STREAM) && LL_DMA_IsActiveFlag_TC5(I2C1_RX_DMA)) {
        LL_DMA_ClearFlag_TC5(I2C1_RX_DMA);
        LL_DMA_DisableIT_TC(I2C1_RX_DMA, I2C1_RX_DMA_STREAM);
        hi2c.state = I2C_STATE_TRANSFER_COMPLETE;
        xSemaphoreGiveFromISR(hi2c.xfer_tc, &xHigherPriorityTaskWoken);
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void I2C1_TX_DMA_IRQ_HANDLER(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if (LL_DMA_IsEnabledIT_TE(I2C1_TX_DMA, I2C1_TX_DMA_STREAM) && LL_DMA_IsActiveFlag_TE4(I2C1_TX_DMA)) {
        LL_DMA_ClearFlag_TE4(I2C1_TX_DMA);
        hi2c.err |= I2C_ERR_DMA;
        hi2c.state = I2C_STATE_TRANSFER_ERROR;
        I2C_IT_DISABLE_ERR_NACK_TC_STOP();
        xSemaphoreGiveFromISR(hi2c.xfer_tc, &xHigherPriorityTaskWoken);
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

