
#include "lwkey.h"

/******************************** macro definition ********************************/

#define TICKS_INTERVAL          ( 10U)        // ticks interval
#define TICKS_FILTER            (  2U)        // ticks of button filter
#define TICKS_PRESS_REPEAT      ( 20U)        // ticks of repeat press
#define TICKS_LONG_PRESS        (100U)        // ticks of long press
#define TICKS_LONG_PRESS_HOLD   (  5U)        // ticks of long press hold

#define IDLE_LEVEL_L            (  0U)        // low  level when button actived
#define IDLE_LEVEL_H            (  1U)        // high level when button actived

#define HKEY_MASK(i)            (1U << (i))
#define HKEY_ALL_MASK           (~((~((hkey_status_t)0ULL)) << HKEY_COUNT))
#define HKEY_GET_STATUS(i)      ((hkey_status >> (i)) & 1U)

/**************************** key msg fifo configure ******************************/

#define KEY_MSG_FIFO_SIZE       (16U)
#define KEY_MSG_FIFO_MASK       (KEY_MSG_FIFO_SIZE - 1)

static struct {
  key_msg_t msg_buf[KEY_MSG_FIFO_SIZE];      // buffer
  volatile uint8_t r;                        // pointer of read
  volatile uint8_t w;                        // pointer of write
} key_msg_fifo = {0};

static inline uint8_t fifo_used(void) {
    return (key_msg_fifo.w - key_msg_fifo.r);
}

static inline int fifo_is_empty(void) {
    return (key_msg_fifo.w == key_msg_fifo.r);
}

static inline int fifo_is_full(void) {
    return (fifo_used() == KEY_MSG_FIFO_SIZE);
}

/**
 * @brief write button event to fifo buffer
 *
 * @param btn_id  button id
 * @param btn_event button event
 * @return int success on 1 | failed on 0
 */
bool lwkey_message_post(uint8_t id, uint8_t event) {
    if (fifo_is_full())
        return false;
    key_msg_t *msg = &key_msg_fifo.msg_buf[key_msg_fifo.w & KEY_MSG_FIFO_MASK];
    msg->id = id;
    msg->event = event;
    key_msg_fifo.w++;
    return true;
}

/**
 * @brief read button event from fifo buffer
 *
 * @param buf pointer of recv buffer
 * @return int success on 1 | failed on 0
 */
bool lwkey_message_fetch(key_msg_t *buf) {
    if (fifo_is_empty())
        return false;
    key_msg_t *msg = &key_msg_fifo.msg_buf[key_msg_fifo.r & KEY_MSG_FIFO_MASK];
    buf->id = msg->id;
    buf->event = msg->event;
    key_msg_fifo.r++;
    return true;
}

/****************************** hardware configure ******************************/

static hardkey_t hkey_list[HKEY_COUNT] = {
    // [hkey id]   = {filter ticks， action level}
    [KEY0] = {0, IDLE_LEVEL_H},
    [KEY1] = {0, IDLE_LEVEL_H},
    [KEY2] = {0, IDLE_LEVEL_H},
    [WKUP] = {0, IDLE_LEVEL_L},
};

// hkey count <= sizeof(hkey_status_t) * 8
static volatile hkey_status_t hkey_status;
static uint8_t (*hkey_input_read)(hardkey_id_t hkey_id);

/****************************** software configure ******************************/

// button list
static softkey_t skey_list[SKEY_COUNT] = {
    // [button id]  = {button id， button type， initval of key state ， hkey 1， hkey 2， initval of ticks}
    [SKEY_KEY0] = {SKEY_KEY0, KEY_TYPE_NORMAL, KEY_STATE_IDLE, KEY0, NULL, 0},
    [SKEY_KEY1] = {SKEY_KEY1, KEY_TYPE_NORMAL, KEY_STATE_IDLE, KEY1, NULL, 0},
    [SKEY_KEY2] = {SKEY_KEY2, KEY_TYPE_NORMAL, KEY_STATE_IDLE, KEY2, NULL, 0},
    [SKEY_WKUP] = {SKEY_WKUP, KEY_TYPE_NORMAL, KEY_STATE_IDLE, WKUP, NULL, 0},
    
    [SKEY_COM1] = {SKEY_COM1, KEY_TYPE_COMPOSITE, KEY_STATE_IDLE, WKUP, KEY0, 0},
    [SKEY_COM2] = {SKEY_COM2, KEY_TYPE_COMPOSITE, KEY_STATE_IDLE, WKUP, KEY2, 0},
};


/************************** function implementatio ******************************/

/**
 * @brief check hkey status
 *
 * @param hkey_id hkey id
 * @return int status changed on true | status unchanged on false
 */
static bool check_hkey_status_update(hardkey_id_t hkey_id) {
    int newstat;
    newstat = (hkey_input_read(hkey_id) != hkey_list[hkey_id].idle_level);
    return (HKEY_GET_STATUS(hkey_id) != newstat);
}

/**
 * @brief update hardware layer button status
 * 
 * @param 
 */
static void hardkey_handler(void) {
    for (int i = 0; i < HKEY_COUNT; i++) {
        if (check_hkey_status_update(i) == true) {
            if (++(hkey_list[i].filter_cnt) >= TICKS_FILTER) {
                // reverse hkey status
                hkey_status ^= HKEY_MASK(i);
                hkey_list[i].filter_cnt = 0;
            }
        } else {
            hkey_list[i].filter_cnt = 0;
        }
    }
    hkey_status &= HKEY_ALL_MASK;
}

/**
 * @brief get button action
 * 
 * @param  
 * @return int press up on 0 | press down on 1 | press break on 2
 */
static softkey_status_t get_softkey_status(softkey_t *key) {
    if (key->type == KEY_TYPE_NORMAL) {
        // single button: if other button press down, will break target button action
        if (hkey_status & ~(HKEY_MASK(key->hkey1))) {
            return KEY_BREAK;
        } else {
            return HKEY_GET_STATUS(key->hkey1);
        }
    } else {
        // composite button: only when hkey 1 press down, can trigger combo button action
        if (hkey_status & ~(HKEY_MASK(key->hkey1) | HKEY_MASK(key->hkey2))) {
            return KEY_BREAK;
        } else {
            if (HKEY_GET_STATUS(key->hkey1)) {
                return HKEY_GET_STATUS(key->hkey2);
            }
            if (HKEY_GET_STATUS(key->hkey2)) {
                return KEY_BREAK;
            }
            return KEY_UP;
        }
    }
}

/**
 * @brief update state of button fsm
 * 
 * @param btn pointer of button
 */
static void softkey_state_handler(softkey_t *key) {
    int key_stat = get_softkey_status(key);

    switch (key->state) {

        case KEY_STATE_BLOCK:
            if (key_stat == KEY_UP) {
                key->state = KEY_STATE_IDLE;
            }
            break;

        case KEY_STATE_IDLE:
            if (key_stat == KEY_BREAK) {
                key->state = KEY_STATE_BLOCK;
            } else if (key_stat == KEY_DOWN) {
                lwkey_message_post(key->id, KEY_EVENT_PRESS_DOWN);
                key->ticks = 0;
                key->state = KEY_STATE_PRESS_DOWN;
            }
            break;
        
        case KEY_STATE_PRESS_DOWN:
            if (key_stat == KEY_BREAK) {
                key->state = KEY_STATE_PRESS_BREAK;
            } else if (key_stat == KEY_DOWN) {
                if (++key->ticks >= TICKS_LONG_PRESS) {
                    lwkey_message_post(key->id, KEY_EVENT_LONG_PRESS_START);
                    key->ticks = 0;
                    key->state = KEY_STATE_LONG_PRESS;
                }
            } else {
                lwkey_message_post(key->id, KEY_EVENT_RELEASE);
                key->state = KEY_STATE_IDLE;
            }
            break;
        
        case KEY_STATE_LONG_PRESS:
            if (key_stat == KEY_BREAK) {
                key->state = KEY_STATE_PRESS_BREAK;
            } else if (key_stat == KEY_DOWN) {
                if (++key->ticks >= TICKS_LONG_PRESS_HOLD) {
                    lwkey_message_post(key->id, KEY_EVENT_LONG_PRESS_HOLD);
                    key->ticks = 0;
                }
            } else {
                lwkey_message_post(key->id, KEY_EVENT_RELEASE);
                key->state = KEY_STATE_IDLE;
            }
            break;
        
        case KEY_STATE_PRESS_BREAK:
            if (key_stat == KEY_UP) {
                lwkey_message_post(key->id, KEY_EVENT_RELEASE);
                key->state = KEY_STATE_IDLE;
            }
            break;
        
        default:
            break;
    }
}

static void softkey_handler(void) {
    for (int i = 0; i < SKEY_COUNT; i++) {
        softkey_state_handler(&skey_list[i]);
    }
}

/**
 * @brief init function
 *
 * @param hkey_read pointer of callback function
 */
void lwkey_init(hkey_read_func_t hkey_read) {

    hkey_input_read = hkey_read;
    
    // To Do: init key message queue for rtos.
}

/**
 * @brief lwkey task function
 *
 */
void lwkey_task(void* args) {
    (void)args;

    // TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        hardkey_handler();
        softkey_handler();

        // vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TICKS_INTERVAL));
    }
}


