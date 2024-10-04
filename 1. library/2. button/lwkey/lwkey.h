#ifndef __LW_KEY_H__
#define __LW_KEY_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************** hardware layer ***********************/

typedef enum {
  /* key id */
  KEY0,
  KEY1,
  KEY2,
  WKUP,
  /* hard key count */
  HKEY_COUNT,
} hardkey_id_t;

typedef struct {
  uint8_t filter_cnt;      // debounce times
  uint8_t idle_level;      // electric level when button is idle
} hardkey_t;

typedef uint32_t hkey_status_t;
typedef uint8_t (*hkey_read_func_t)(hardkey_id_t hkey_id);

/*********************** software layer ***********************/

typedef enum {
  /* key id */
  SKEY_KEY0,
  SKEY_KEY1,
  SKEY_KEY2,
  SKEY_WKUP,
  SKEY_COM1,
  SKEY_COM2,
  /* soft key count */
  SKEY_COUNT,
} softkey_id_t;

typedef enum {
  KEY_UP = 0,
  KEY_DOWN = 1,
  KEY_BREAK = 2,
} softkey_status_t;

typedef enum {
  KEY_TYPE_NORMAL,
  KEY_TYPE_COMPOSITE,
} softkey_type_t;

typedef enum {
  KEY_STATE_BLOCK,
  KEY_STATE_IDLE,
  KEY_STATE_PRESS_DOWN,
  KEY_STATE_PRESS_BREAK,
  KEY_STATE_LONG_PRESS,
} softkey_state_t;

typedef struct {
  uint8_t id;
  uint8_t type;
  uint8_t state;
  uint8_t hkey1;
  uint8_t hkey2;
  uint8_t ticks;
} softkey_t;

/************************ key event ************************/

typedef enum {
  KEY_EVENT_NULL,
  KEY_EVENT_PRESS_DOWN,
  KEY_EVENT_LONG_PRESS_START,
  KEY_EVENT_LONG_PRESS_HOLD,
  KEY_EVENT_RELEASE,
} key_event_t;

typedef struct {
  uint8_t id: 4;
  uint8_t event: 4;
} key_msg_t;

/******************* function declaration *********************/

void lwkey_init(hkey_read_func_t hkey_read);
void lwkey_task(void *args);
bool lwkey_message_fetch(key_msg_t *buf);

#ifdef __cplusplus
}
#endif

#endif