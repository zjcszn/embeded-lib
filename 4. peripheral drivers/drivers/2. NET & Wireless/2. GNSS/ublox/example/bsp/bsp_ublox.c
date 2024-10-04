//
// Created by zjcszn on 2024/1/26.
//

#include "bsp_ublox.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "dev_uart.h"
#include "minmea/minmea.h"

#define UBLOX_PRINTF(...)   \
do {                        \
    uint32_t now = xTaskGetTickCount();     \
    printf("[%07u.%03u] UBLOX > ", now / 1000, now % 1000); \
    printf(__VA_ARGS__);      \
} while(0)

#define UBLOX_SERIAL_SEND(src, len)     uart1_write(src, len)

#define UBLOX_SERIAL_RECV(dst)          \
do{                                     \
    while(uart1_read(dst, 1) == 0) {    \
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000));  \
    }   \
}while(0)

#define UBLOX_ARRAY_SIZE(arr)           (sizeof(arr) / sizeof(arr[0]))

/* --------------------- ublox interface configure command------------------------ */

const uint8_t ublox_cfg_msgout[][17] = {
    /* $xxGSV disable */
    {
        0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0xC5, 0x00, 0x91, 0x20, 0x00, 0x11, 0x06
    },
    /* $xxGSA disable */
    {
        0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0xC0, 0x00, 0x91, 0x20, 0x00, 0x0C, 0xED
    },
    /* $xxZDA disable */
    {
        0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0xD9, 0x00, 0x91, 0x20, 0x00, 0x25, 0x6A
    },
    /* $xxVTG disable */
    {
        0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0xB1, 0x00, 0x91, 0x20, 0x00, 0xFD, 0xA2
    },
    /* $xxGLL disable */
    {
        0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0xCA, 0x00, 0x91, 0x20, 0x00, 0x16, 0x1F
    },
    /* $xxRMC enable 1hz */
    {
        0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0xAC, 0x00, 0x91, 0x20, 0x01, 0xF9, 0x8A
    },
    /* $xxGGA enable 1hz */
    {
        0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0xBB, 0x00, 0x91, 0x20, 0x01, 0x08, 0xD5
    }
};

/* reset : hot start  */
const uint8_t ublox_cfg_reset[] = {
    0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x66
};

/* --------------------- ublox message buffer ------------------------ */

static uint8_t ublox_buffer[UBLOX_BUFFER_SIZE];
static TaskHandle_t ublox_task_handle;

/* ------------------------ ublox core api --------------------------- */

static uint32_t prv_ublox_sentence_read(void) {
    uint8_t c;
    uint32_t nread = 0;

    /* find start character '$ ' */
    do {
        UBLOX_SERIAL_RECV(&c);
        if (c == '$')
            break;
    } while (1);

    ublox_buffer[nread++] = c;

    /* read remaining character of sentence */
    do {
        UBLOX_SERIAL_RECV(&c);

        if (c == '$')
            nread = 0;

        ublox_buffer[nread++] = c;

        if (c == '\n' && ublox_buffer[nread - 2] == '\r') {
            ublox_buffer[nread] = '\0';
            return nread;
        }
    } while (nread < (UBLOX_BUFFER_SIZE - 1));

    return 0;
}

static void prv_ublox_message_notify(void) {
    if (ublox_task_handle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(ublox_task_handle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            taskYIELD();
        }
    }
}

static void prv_ublox_interface_init(void) {
    dev_uart1_init();
    uart1_set_rx_indi(prv_ublox_message_notify);
}

void prv_ublox_soft_reset(void) {
    UBLOX_SERIAL_SEND(&ublox_cfg_reset, sizeof(ublox_cfg_reset));
    vTaskDelay(pdMS_TO_TICKS(300));
}

void prv_ublox_checksum_calc(uint8_t *src, uint16_t len, uint8_t *ck_a, uint8_t *ck_b) {
    uint8_t cka = 0, ckb = 0;
    for (int i = 0; i < len; i++) {
        cka += src[i];
        ckb += cka;
    }
    *ck_a = cka;
    *ck_b = ckb;
}

void bsp_ublox_rate_set(uint8_t freq) {
    uint16_t period;
    freq = (freq == 0) ? 1 : (freq > 10) ? 10 : freq;
    period = 1000 / freq;

    uint8_t ublox_cfg_rate[18] = {
        0xB5, 0x62, 0x06, 0x8A, 0x0A, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x21, 0x30};
    /* set nominal time between GNSS measurements -- E.g. 100 ms results in 10 Hz */
    ublox_cfg_rate[14] = period & 0xFF;
    ublox_cfg_rate[15] = period >> 8;
    /* caculate checksum */
    prv_ublox_checksum_calc(&ublox_cfg_rate[2], 14, &ublox_cfg_rate[16], &ublox_cfg_rate[17]);

    UBLOX_SERIAL_SEND(ublox_cfg_rate, sizeof(ublox_cfg_rate));
    vTaskDelay(pdMS_TO_TICKS(10));
}

void prv_ublox_msgout_config(void) {
    for (int i = 0; i < UBLOX_ARRAY_SIZE(ublox_cfg_msgout); i++) {
        UBLOX_SERIAL_SEND(&ublox_cfg_msgout[i], sizeof(ublox_cfg_msgout[0]));
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void bsp_ublox_reset(void) {
    prv_ublox_soft_reset();
    prv_ublox_msgout_config();
    bsp_ublox_rate_set(1);
}

void ublox_task(void *args) {
    (void)args;

    char talker_id[3];
    uint32_t nread, err_cnt;

    prv_ublox_interface_init();
    bsp_ublox_reset();

    while (1) {
        nread = prv_ublox_sentence_read();
        if (nread == 0) {
            if (++err_cnt > 5) {
                err_cnt = 0;
                bsp_ublox_reset();
            }
            continue;
        } else {
            UBLOX_PRINTF("raw data: %s", ublox_buffer);
        }
        if (minmea_sentence_id((const char *)ublox_buffer) == MINMEA_SENTENCE_RMC) {
            struct minmea_sentence_rmc frame;
            if (minmea_talker_id(talker_id, (const char *)ublox_buffer)) {
                UBLOX_PRINTF("Talker ID: %s\n", talker_id);
            }
            if (minmea_parse_rmc(&frame, (const char *)ublox_buffer)) {
                if (frame.valid) {
                    UBLOX_PRINTF("$xxRMC coordinates and speed: (%f,%f) %f\n",
                                 minmea_tocoord(&frame.latitude),
                                 minmea_tocoord(&frame.longitude),
                                 minmea_tofloat(&frame.speed));
                } else {
                    UBLOX_PRINTF("$xxRMC data is invalid\n");
                }
            }
        }
    }
}

void bsp_ublox_init(void) {
    BaseType_t xReturn;
    xReturn = xTaskCreate(ublox_task, "ublox", 512, NULL, 10, &ublox_task_handle);
    configASSERT(xReturn == pdTRUE);
}
