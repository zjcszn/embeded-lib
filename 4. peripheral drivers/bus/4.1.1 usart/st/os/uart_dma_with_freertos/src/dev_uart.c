#include "dev_uart.h"


/* uart fifo & dma buffer*/
static uint8_t uart_rx_fifo[UART_RX_FIFO_BUFSZ];
static uint8_t uart_dmarx_buf[UART_RX_DMA_BUFSZ];
static uint8_t uart_tx_fifo[UART_TX_FIFO_BUFSZ];
static uint8_t uart_dmatx_buf[UART_TX_DMA_BUFSZ];

/* uart dev */
static dev_uart_t  dev_uart = {0};

/* static function declaration */
static void uart_dmatx_poll(void *args);

void dev_uart_init(void) {
    /* software init */
    fifo_queue_init(&dev_uart.rx_fifo, uart_rx_fifo, UART_RX_FIFO_BUFSZ);
    fifo_queue_init(&dev_uart.tx_fifo, uart_tx_fifo, UART_TX_FIFO_BUFSZ);
    dev_uart.dmarx_buf   = uart_dmarx_buf;
    dev_uart.dmarx_bufsz = UART_RX_DMA_BUFSZ;
    dev_uart.dmatx_buf   = uart_dmatx_buf;
    dev_uart.dmatx_bufsz = UART_TX_DMA_BUFSZ;
    dev_uart.dmarx_read  = 0;
    dev_uart.rx_notice = NULL;
    dev_uart.tx_fifo_lock = xSemaphoreCreateRecursiveMutex();
    dev_uart.rx_fifo_lock = xSemaphoreCreateRecursiveMutex();
    dev_uart.tx_notice = xSemaphoreCreateBinary();
    dev_uart.tx_dma_notice = xSemaphoreCreateBinary();
    /* hardware init */
    bsp_uart_init(uart_dmarx_buf, UART_RX_DMA_BUFSZ);
    /* create tx poll task */
    xTaskCreate(uart_dmatx_poll, "uart tx", 128, NULL, configMAX_PRIORITIES - 1, NULL);
}

void dev_uart_deinit(void) {

    /* 1.hardware deinit */

    /* 2.semaphore delete */

    /* 3.fifo deinit */

    /* 4.dev handle clear */

}

/**
 * @brief send data to uart serial port
 * 
 * @param src data to send
 * @param len data length
 * @return uint32_t real length of send data.
 */
uint32_t uart_write(const void *src, uint32_t len) {
    if (!len) return 0;

    if (xSemaphoreTakeRecursive(dev_uart.tx_fifo_lock, UART_BLOCKTIME) != pdTRUE) {
        return 0;
    }

    uint32_t wr = 0;
    uint32_t bw;

    while (len) {
        if (len > 1) {
            bw = fifo_queue_write(&dev_uart.tx_fifo, src + wr, len);
        }
        else {
            bw = fifo_queue_putc(&dev_uart.tx_fifo, *((char *)(src + wr)));
        }
        if (bw) {
            wr  += bw;
            len -= bw;
            xSemaphoreGive(dev_uart.tx_notice);
        }
    }
    xSemaphoreGiveRecursive(dev_uart.tx_fifo_lock);
    return wr;
}

/**
 * @brief read data from uart serial port
 * 
 * @param dst buffer of read data
 * @param len data length
 * @return uint32_t real length of read data.
 */
uint32_t uart_read(void *dst, uint32_t len) {
    if (!len) return 0;

    if (xSemaphoreTakeRecursive(dev_uart.rx_fifo_lock, UART_BLOCKTIME) != pdTRUE) {
        return 0;
    }

    uint32_t rd;

    if (len > 1) {
        rd = fifo_queue_read(&dev_uart.rx_fifo, dst, len);
    }
    else {
        rd = fifo_queue_getc(&dev_uart.rx_fifo, dst);
    }
    
    xSemaphoreGiveRecursive(dev_uart.rx_fifo_lock);
    return rd;
}

/**
 * @brief lock uart write interface
 * 
 * @return true on success, false on failed.
 */
bool uart_write_lock(void) {
    if (xSemaphoreTakeRecursive(dev_uart.tx_fifo_lock, UART_BLOCKTIME) != pdTRUE) {
        return false;
    }
    return true;
}

/**
 * @brief unlock uart write interface
 * 
 * @return true on success, false on failed.
 */
bool uart_write_unlock(void) {
    if (xSemaphoreGiveRecursive(dev_uart.tx_fifo_lock) != pdTRUE) {
        return false;
    }
    return true;
}

/**
 * @brief lock uart read interface
 * 
 * @return true on success, false on failed.
 */
bool uart_read_lock(void) {
    if (xSemaphoreTakeRecursive(dev_uart.rx_fifo_lock, UART_BLOCKTIME) != pdTRUE) {
        return false;
    }
    return true;
}

/**
 * @brief unlock uart read interface
 * 
 * @return true on success, false on failed.
 */
bool uart_read_unlock(void) {
    if (xSemaphoreGiveRecursive(dev_uart.rx_fifo_lock) != pdTRUE) {
        return false;
    }
    return true;
}

/**
 * @brief clear uart send buffer
 * 
 */
void uart_clear_rx_buffer(void) {
    taskENTER_CRITICAL();
    fifo_queue_clear(&(dev_uart.rx_fifo));
    taskEXIT_CRITICAL();
}

/**
 * @brief clear uart recive buffer
 * 
 */
void uart_clear_tx_buffer(void) {
    taskENTER_CRITICAL();
    fifo_queue_clear(&(dev_uart.tx_fifo));
    taskEXIT_CRITICAL();
}

/**
 * @brief set callback function of uart recive notify
 * 
 * @param rx_indi pointer of new callback function
 * @return void* pointer of old callback function
 */
void* uart_set_rx_notice(uart_cbf_t rx_indi) {
    
    taskENTER_CRITICAL();
    void *old_indi = dev_uart.rx_notice;
    dev_uart.rx_notice = rx_indi;
    taskEXIT_CRITICAL();

    return old_indi;
}


/*-------------------------------------- static function -------------------------------------*/


static void uart_rx_fifo_write(const void* src, uint32_t len) {

    if (len) {
        uint32_t bw;

        if (len > 1) {
            bw = fifo_queue_write(&dev_uart.rx_fifo, src, len);
        }
        else {
            bw = fifo_queue_putc(&dev_uart.rx_fifo, *((char *)src));
        }

        if (bw && dev_uart.rx_notice) {
            dev_uart.rx_notice();
        }    
    }
}

static void uart_dmarx_ht_callback(void) {
    uint32_t recv_len;
    recv_len = dev_uart.dmarx_bufsz - (bsp_uart_dmarx_buf_remain_size() + dev_uart.dmarx_read);
    uart_rx_fifo_write(dev_uart.dmarx_buf + dev_uart.dmarx_read, recv_len);
    dev_uart.dmarx_read += recv_len;
}


static void uart_dmarx_tc_callback(void) {
    uint32_t recv_len;
    recv_len = dev_uart.dmarx_bufsz - dev_uart.dmarx_read;
    uart_rx_fifo_write(dev_uart.dmarx_buf + dev_uart.dmarx_read, recv_len);
    dev_uart.dmarx_read = 0;
}


static void uart_idle_callback(void) {
    uint32_t recv_len;
    recv_len = dev_uart.dmarx_bufsz - (bsp_uart_dmarx_buf_remain_size() + dev_uart.dmarx_read);
    uart_rx_fifo_write(dev_uart.dmarx_buf + dev_uart.dmarx_read, recv_len);
    dev_uart.dmarx_read += recv_len;
}


static void uart_dmatx_tc_callback(void) {
    LL_DMA_DisableIT_TC(UART_TX_DMA, UART_TX_DMA_STREAM);
    LL_USART_ClearFlag_TC(UART);
    LL_USART_EnableIT_TC(UART);
}

static void uart_tc_callback(void) {
    LL_USART_DisableDMAReq_TX(UART);
    LL_USART_DisableIT_TC(UART);
    xSemaphoreGiveFromISR(dev_uart.tx_dma_notice, NULL);
}

static void uart_dmatx_poll(void *args) {
    (void) args;
    uint32_t tx_bytes;

    while (1) {
        xSemaphoreTake(dev_uart.tx_notice, portMAX_DELAY);
        while (1) {
            tx_bytes = fifo_queue_read(&dev_uart.tx_fifo, dev_uart.dmatx_buf, dev_uart.dmatx_bufsz);
            if (tx_bytes == 0)
                break;
            bsp_uart_dmatx_enable(dev_uart.dmatx_buf, tx_bytes);
            xSemaphoreTake(dev_uart.tx_dma_notice, portMAX_DELAY);
        }   
    }
}

/*---------------------------------- uart & dma IRQ handler ----------------------------------*/

void BSP_UART_TX_DMA_ISR(void) {

    if (LL_DMA_IsEnabledIT_TC(UART_TX_DMA, UART_TX_DMA_STREAM) && LL_DMA_IsActiveFlag_TC7(UART_TX_DMA)) {
        uart_dmatx_tc_callback();
        DMA_CLEAR_IT_FLAG(UART_TX_DMA_STREAM_ID, TC, UART_TX_DMA);
    }
}

void BSP_UART_RX_DMA_ISR(void) {
    
    if (LL_DMA_IsEnabledIT_HT(UART_RX_DMA, UART_RX_DMA_STREAM) && LL_DMA_IsActiveFlag_HT2(UART_RX_DMA)) {
        uart_dmarx_ht_callback();
        DMA_CLEAR_IT_FLAG(UART_RX_DMA_STREAM_ID, HT, UART_RX_DMA);
    }
    if (LL_DMA_IsEnabledIT_TC(UART_RX_DMA, UART_RX_DMA_STREAM) && LL_DMA_IsActiveFlag_TC2(UART_RX_DMA)) {
        uart_dmarx_tc_callback();
        DMA_CLEAR_IT_FLAG(UART_RX_DMA_STREAM_ID, TC, UART_RX_DMA);
    }
}

void BSP_UART_ISR(void) {

    if (LL_USART_IsEnabledIT_TC(UART) && LL_USART_IsActiveFlag_TC(UART)) {
        uart_tc_callback();
        LL_USART_ClearFlag_TC(UART);
    }
    if (LL_USART_IsEnabledIT_IDLE(UART) && LL_USART_IsActiveFlag_IDLE(UART)) {
        uart_idle_callback();
        LL_USART_ClearFlag_IDLE(UART);
    }
}