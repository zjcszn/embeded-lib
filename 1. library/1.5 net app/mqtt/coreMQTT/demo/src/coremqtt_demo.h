//
// Created by zjcszn on 2023/6/18.
//

#ifndef __COREMQTT_DEMO_H__
#define __COREMQTT_DEMO_H__

#include "FreeRTOS.h"

void vStartSimpleSubscribePublishTask(uint32_t ulNumberToCreate,
                                      configSTACK_DEPTH_TYPE uxStackSize,
                                      UBaseType_t uxPriority);
#endif //__COREMQTT_DEMO_H__
