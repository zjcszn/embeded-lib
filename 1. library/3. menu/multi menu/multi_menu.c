/***********************************************************************************
 * 
 * author: zjcszn
 * date: 2022/12/17
 * version: 1.0
 * 
 * MIT License
 * 
 * Copyright (c) 2022 zjcszn
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * ********************************************************************************/

#include "multi_menu.h"
#include <assert.h>
#include <string.h>

/******************************** Macro Definition ********************************/

#define MENU_DISP_MAX_ITEMS       (2U)          // max number of display items
#define MENU_TREE_DEPTH           (3U)          // max level  of menu tree

#define menu_assert(exp)          assert(exp)   // menu assert macro

/******************************** Private Variable ********************************/

static MenuList         menu_list;              // pointer of menu list
static uint32_t         menu_count;             // count of menu node

static MenuConfig       menu_config;            // config of menu display
static uint32_t         menu_refresh;           // menu ui refresh flag

static MenuUI_Handler   menu_ui_handler;

/******************************* Menu Config Stack ********************************/

#define MENU_CFG_STACK_SIZE       (MENU_TREE_DEPTH - 1)

static struct {
  MenuConfig stack[MENU_CFG_STACK_SIZE];
  __IO uint8_t sp;
} menu_cfg_stack = {0};

/******************************* Menu Message Queue *******************************/

#define MENU_MSG_FIFO_SIZE        (4U)
#define MENU_MSG_FIFO_MASK        (MENU_MSG_FIFO_SIZE - 1)

static struct {
  uint8_t msg_buf[MENU_MSG_FIFO_SIZE];
  __IO uint8_t r;
  __IO uint8_t w;
}menu_msg_fifo = {0};

/**
 * @brief clear message queue
 * 
 */
static inline void menu_msg_clear(void) {
  menu_msg_fifo.r = menu_msg_fifo.w = 0;
}

/**
 * @brief get count of message in the queue
 * 
 * @return uint8_t message count
 */
static inline uint8_t menu_msg_count(void) {
  return (menu_msg_fifo.w - menu_msg_fifo.r);
}

/**
 * @brief check if queue is empty
 * 
 * @return int empty on 1 | not empty on 0
 */
static inline int check_menu_msg_empty(void) {
  return (menu_msg_fifo.w == menu_msg_fifo.r);
}

/**
 * @brief check if queue is full
 * 
 * @return int full on 1 | not full on 0
 */
static inline int check_menu_msg_full(void) {
  return (menu_msg_count() == MENU_MSG_FIFO_SIZE);
}

/************************** Private Function Declaration **************************/

static void menu_display_init(MenuNode *_cur_page, uint32_t _max_disp);
static void menu_msg_handler(void);
static int  menu_msg_dequeue(uint8_t *_menu_msg);
static int  menu_config_push(void);
static int  menu_config_pop(void);
static MenuNode *find_menu_node(uint32_t target_id);

/****************************** Function Definition *******************************/



/**
 * @brief init menu tree
 * 
 * @param menu_list  pointer of menu list  
 * @param menu_count  count of menu item
 */
void menu_init(MenuList _menu_list, uint32_t _menu_count, MenuUI_Handler ui_callback) {
  menu_assert(_menu_list && ui_callback);
  menu_list  = _menu_list;
  menu_count = _menu_count;
  MenuNode *target;
  for (int i = 0; i < _menu_count; i++) {
    target = &menu_list[i];
    target->child  = find_menu_node((target->id << 4U) + 1);
    if (target->id) {
      target->left   = find_menu_node(target->id - 1);
      target->right  = find_menu_node(target->id + 1);
    }
    else {
      target->left = target->right = NULL;
    }
  }
  menu_ui_handler = ui_callback;
  menu_display_init(&menu_list[0], MENU_DISP_MAX_ITEMS);
}

/**
 * @brief menu loop function
 * 
 */
void menu_loop(void) {
  menu_assert(menu_ui_handler);
  menu_msg_handler();
  if (menu_refresh != MENU_REFRESH_NULL) {
    menu_ui_handler(&menu_config);
    menu_refresh = MENU_REFRESH_NULL;
  }
}

/**
 * @brief init menu display
 * 
 * @param _cur_page current menu page
 * @param _max_disp max number of menu item in the display list
 */
static void menu_display_init(MenuNode *_cur_page, uint32_t _max_disp) {
  menu_assert(_cur_page && _cur_page->child);
//menu_config.max_disp = _max_disp; 
  menu_config.cur_page = _cur_page;
  menu_config.cur_item = _cur_page->child;
  menu_config.head = menu_config.cur_item;
  menu_config.tail = menu_config.cur_item;
  for (int i = 1; i < _max_disp; i++) {
    if (menu_config.tail->right) {
      menu_config.tail = menu_config.tail->right;
    }
    else break;
  }
  menu_refresh = MENU_REFRESH_FULL;
}

/**
 * @brief Handling of message operations
 * 
 */
static void menu_msg_handler(void) {
  uint8_t msg;
  if (!menu_msg_dequeue(&msg)) return;

  switch (msg) {
    case MENU_MSG_UP:
      if (menu_config.cur_item->left != NULL) {
        menu_refresh = MENU_REFRESH_FULL;
        if (menu_config.cur_item == menu_config.head) {
          menu_config.head = menu_config.head->left;
          menu_config.tail = menu_config.tail->left;
        }
        menu_config.cur_item = menu_config.cur_item->left;
      }
      break;

    case MENU_MSG_DOWN:
      if (menu_config.cur_item->right != NULL) {
        menu_refresh = MENU_REFRESH_FULL;
        if (menu_config.cur_item == menu_config.tail) {
          menu_config.head = menu_config.head->right;
          menu_config.tail = menu_config.tail->right;
        }
        menu_config.cur_item = menu_config.cur_item->right;
      }
      break;

    case MENU_MSG_ENTER:
      if (menu_config.cur_item->child == NULL) {
        if (menu_config.cur_item->op != NULL) {
          menu_config.cur_item->op(NULL);
        }
      }
      else {
        if (menu_config_push()) {
          menu_display_init(menu_config.cur_item, MENU_DISP_MAX_ITEMS);
        }
        else {
          menu_assert(0);     // can not arrive here
        }
      }
      break;

    case MENU_MSG_ESC:
      if (menu_config_pop()) {
        menu_refresh = MENU_REFRESH_FULL;
      }
      break;
  }
}


/**
 * @brief read message from queue
 * 
 * @param _menu_msg pointer of read buffer 
 * @return int success on 1 | failed on 0
 */
static int menu_msg_dequeue(uint8_t *_menu_msg) {
  menu_assert(_menu_msg);
  if (check_menu_msg_empty()) return 0;
  *_menu_msg = menu_msg_fifo.msg_buf[menu_msg_fifo.r & MENU_MSG_FIFO_MASK];
  menu_msg_fifo.r++;
  return 1;
}

/**
 * @brief send message to queue
 * 
 * @param _menu_msg menu message
 * @return int success on 1 | failed on 0
 */
int menu_msg_enqueue(uint8_t _menu_msg) {
  if (check_menu_msg_full()) return 0;
  menu_msg_fifo.msg_buf[menu_msg_fifo.w & MENU_MSG_FIFO_MASK] = _menu_msg;
  menu_msg_fifo.w++;
  return 1;
}


/**
 * @brief push menu display config into stack
 * 
 * @return int success on 1 | failed on 0
 */
static int menu_config_push(void) {
  if (menu_cfg_stack.sp >= MENU_CFG_STACK_SIZE) return 0;
  memcpy(&menu_cfg_stack.stack[menu_cfg_stack.sp++], (void *)&menu_config, sizeof(MenuConfig));
  return 1;
}

/**
 * @brief pop menu display config from stack
 * 
 * @return int success on 1 | failed on 0
 */
static int menu_config_pop(void) {
  if (!menu_cfg_stack.sp) return 0;
  memcpy((void *)&menu_config, &menu_cfg_stack.stack[--menu_cfg_stack.sp], sizeof(MenuConfig));
  return 1;
}


/**
 * @brief find menu node
 * 
 * @param target_id target menu id
 * @return MenuNode *pointer of menu node
 */
static MenuNode *find_menu_node(uint32_t target_id) {
  menu_assert(menu_list);
  for(int i = 1; i < menu_count; i++) {
    if (menu_list[i].id == target_id) return &menu_list[i];
  }
  return NULL;
}