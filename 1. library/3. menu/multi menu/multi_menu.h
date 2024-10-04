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

#ifndef __MULTI_MENU_H__
#define __MULTI_MENU_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************** Macro Definition ********************************/

#ifndef __IO
#define __IO volatile
#endif

#ifndef NULL
#define NULL ((void*)0U)
#endif

/******************************** Type Definition *********************************/

typedef struct menu_node    MenuNode;
typedef struct menu_node   *MenuList;
typedef struct menu_config  MenuConfig;
typedef void (*MenuUI_Handler)(MenuConfig *);
typedef void (*MenuFunc)(void *);

enum ENUM_MenuRefresh {
  MENU_REFRESH_NULL,
  MENU_REFRESH_PART,
  MENU_REFRESH_FULL,
};

enum ENUM_MenuMessage {
  MENU_MSG_NULL,
  MENU_MSG_UP,  
  MENU_MSG_DOWN,    
  MENU_MSG_ENTER,   
  MENU_MSG_ESC,     
};

struct menu_node{
  uint32_t  id;        // menu id
  uint8_t  *text;      // menu text
  MenuFunc  op;        // operate function
  MenuNode *child;     // child item
  MenuNode *left;      // left  item
  MenuNode *right;     // right item
};

struct menu_config{
  MenuNode *cur_page;  // current page
  MenuNode *cur_item;  // current selected item
  MenuNode *head;      // head of display list
  MenuNode *tail;      // tail of display list
//uint32_t  max_disp;  // max number of item in the display list       
};

/************************** Extern Function Declaration ***************************/

extern void menu_init(MenuList _menu_list, uint32_t _menu_count, MenuUI_Handler ui_callback);

extern void menu_loop(void);
extern int  menu_msg_enqueue(uint8_t _menu_msg);


#ifdef __cplusplus
}
#endif

#endif