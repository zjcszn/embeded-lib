#include "bsp_beep.h"

#include "gd32f30x_gpio.h"


typedef enum {
    BEEP_IDLE,
    BEEP_START,
    BEEP_STOP,
} beep_state_t;

typedef struct {
    uint8_t  state;
    uint8_t  cycle;  
    uint32_t beep_time;
    uint32_t stop_time;
    uint16_t pwm_reload;
    uint16_t pwm_counter;
    uint32_t ticks;
} beep_handle_t;

static volatile beep_handle_t beep_handle;


void bsp_beep_init(void) {
    gpio_init(BEEP_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_MAX, BEEP_PIN);
}


/**
 * @brief 启动蜂鸣器
 * 
 * @param beep_time 蜂鸣持续时间(ms)，0则不会启动蜂鸣器
 * @param stop_time 蜂鸣停止时间(ms)，0表示持续鸣叫
 * @param cycle 循环次数，0表示一直循环
 * @param freq 蜂鸣器pwm频率
 */
void bsp_beep_start(uint16_t beep_time, uint16_t stop_time, uint8_t cycle, uint16_t freq) {
    __disable_irq();
    if (beep_time && (beep_handle.state == BEEP_IDLE)) {
        uint16_t tmp = (uint16_t)(((1.0f * 1000.0f * 1000.0f) / (float)freq) / 100.0f / 2.0f); /* 计算pwm脉宽 */
        beep_handle.pwm_reload = tmp ? tmp : 1;
        beep_handle.pwm_counter = beep_handle.pwm_reload;
        beep_handle.beep_time = beep_time * 10; /* 单位转换，参数 ms, beep 时基 100us */
        beep_handle.stop_time = stop_time * 10;
        beep_handle.ticks = beep_handle.beep_time;
        beep_handle.cycle = cycle;
        beep_handle.state = BEEP_START;
    }
    __enable_irq();
}

/**
 * @brief 停止蜂鸣器
 * 
 */
void bsp_beep_stop(void) {
    __disable_irq();
    beep_handle.state = BEEP_IDLE;
    __enable_irq();

}


void bsp_beep_task(void) {
    static volatile uint8_t beep_on;

    switch (beep_handle.state) {
        case BEEP_IDLE:
            break;

        case BEEP_START:
            if (beep_handle.ticks == 0) {
                if (beep_handle.stop_time == 0) {
                    if ((beep_handle.cycle != 0) && (--beep_handle.cycle == 0)) {
                        beep_handle.state = BEEP_IDLE;
                        break;
                    }
                    beep_handle.ticks = beep_handle.beep_time;
                } else {
                    beep_handle.ticks = beep_handle.stop_time;
                    beep_handle.state = BEEP_STOP;
                    break;
                }
            }

            if (--beep_handle.pwm_counter == 0) {
                if (beep_on) {
                    gpio_bit_set(BEEP_PORT, BEEP_PIN);
                } else {
                    gpio_bit_reset(BEEP_PORT, BEEP_PIN);
                }
                beep_on = !beep_on;
                beep_handle.pwm_counter = beep_handle.pwm_reload;
            }

            beep_handle.ticks--;
            break;

        case BEEP_STOP:
            if (beep_handle.ticks) {
                beep_handle.ticks--;
            } else {
                if ((beep_handle.cycle != 0) && (--beep_handle.cycle == 0)) {
                    beep_handle.state = BEEP_IDLE;
                    break;
                } else {
                    beep_handle.ticks = beep_handle.beep_time;
                    beep_handle.state = BEEP_START;
                    break;
                }
            }
            break;

        default:
            break;
    }
}