#ifndef __CUSTOM_BUTTON_H__
#define __CUSTOM_BUTTON_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************** 硬件层 ***********************/

// 依据设计原理图定义

// 硬件按键编号
enum ENUM_HButtonID {
  HBUTTON_KEY0,             // KEY_0
  HBUTTON_KEY1,             // KEY_1
  HBUTTON_KEY2,             // KEY_2
  HBUTTON_WKUP,             // KEY_WKUP
  HBUTTON_COUNT,            // 硬件按键数量
  HBUTTON_NULL,             // 
};

// 硬件按键结构体
typedef struct {
  uint8_t filter_cnt;       // 消抖计数器
  uint8_t act_level;        // 动作电平
}HButton_T;

/*********************** 应用层 ***********************/

// 按键编号
enum soft_button_id {
  BUTTON_KEY0,              // KEY_0
  BUTTON_KEY1,              // KEY_1
  BUTTON_KEY2,              // KEY_2
  BUTTON_WKUP,              // KEY_WKUP
  BUTTON_COMBO1,            // 组合键 1：KEY_WKUP + KEY_0
  BUTTON_COMBO2,            // 组合键 2：KEY_WKUP + KEY_2
  BUTTON_COUNT,             // 自定义按键数量
};

// 按键动作事件
enum soft_button_action {
  BUTTON_UP   = 0,   // 按键释放
  BUTTON_DOWN = 1,   // 按键按下
  BUTTON_BREAK  = 2,   // 按键打断
};

// 按键类型
enum soft_button_type {
  BUTTON_TYPE_SINGLE,       // 单键类型
  BUTTON_TYPE_COMBO,        // 组合类型
};

// 按键状态机
enum softkey_state {
  STATE_IDLE,               // 空闲状态
  STATE_PRESS_DOWN,         // 按下状态
  STATE_PRESS_BRK,          // 打断状态
  STATE_PRESS_LONG,         // 长按状态
};

// 按键结构体
typedef struct {
  uint8_t id;               // 按键ID
  uint8_t type;             // 按键类型
  uint8_t state;            // FSM状态
  uint8_t hbtn_1;           // 组合键1
  uint8_t hbtn_2;           // 组合键2
  uint8_t ticks;            // 计数器
}softkey_t;



/********************** 按键事件 **********************/

// 事件类型
enum skey_event {
  EVENT_NULL,               // 空事件
  EVENT_PRESS_DOWN,         // 按键按下事件
  EVENT_PRESS_UP,           // 按键抬起事件
  EVENT_LONG_PRESS_START,   // 按键长按开始事件
  EVENT_LONG_PRESS_HOLD,    // 按键长按保持事件
  EVENT_LONG_PRESS_UP,      // 按键长按抬起事件
};

// 事件结构体
typedef struct {
  uint8_t button_id;        // 按键ID
  uint8_t button_event;     // 按键事件
}ButtonEvent_T;

/********************** 函数声明 **********************/

void button_init (uint8_t(*cb)(uint8_t hbtn_id));
void button_scan(void);
int  button_event_get(ButtonEvent_T *buf);

#ifdef __cplusplus
}
#endif

#endif