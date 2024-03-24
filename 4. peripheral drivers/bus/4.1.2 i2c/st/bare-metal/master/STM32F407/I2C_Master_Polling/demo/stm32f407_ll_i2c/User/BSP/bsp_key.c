#include "bsp_key.h"
#include "bsp_lcd1602.h"

static uint8_t bsp_key_gpio_read(uint8_t key_id);


// 按键初始化程序
void bsp_key_init(void) {
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;                 
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;                   
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;           
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_4;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);     
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;   
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0;               
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);     
  button_init(bsp_key_gpio_read);
}

// 按键轮询程序
void bsp_key_poll(void *args) {
  button_scan();
}

// 按键事件回调函数
void bsp_key_callback(void) {
  ButtonEvent_T key_event;
  if (button_event_get(&key_event)){
    switch (key_event.button_id) {
      case BUTTON_KEY0:
        if (key_event.button_event == EVENT_PRESS_UP) {
          lcd1602_show_str(1, 1, "HELLO WORLD!");
        }
        break;

      case BUTTON_KEY1:
        if (key_event.button_event == EVENT_PRESS_UP) {
          lcd1602_show_str(2, 1, "THIS IS A DEMO!");
        }
        break;

      case BUTTON_KEY2:
        break;

      case BUTTON_WKUP:
        if (key_event.button_event == EVENT_PRESS_UP) {
          lcd1602_clear();
        }
        break;
      case BUTTON_COMBO1:
        break;

      case BUTTON_COMBO2:
        break;
    }
  }
}

static uint8_t bsp_key_gpio_read(uint8_t key_id) {
  switch (key_id) {
    case HBUTTON_KEY0:
      return LL_GPIO_IsInputPinSet(GPIOE, LL_GPIO_PIN_4);
    case HBUTTON_KEY1:
      return LL_GPIO_IsInputPinSet(GPIOE, LL_GPIO_PIN_3);
    case HBUTTON_KEY2:
      return LL_GPIO_IsInputPinSet(GPIOE, LL_GPIO_PIN_2);
    case HBUTTON_WKUP:
      return LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0);
    default:
      return 0;
  }
}