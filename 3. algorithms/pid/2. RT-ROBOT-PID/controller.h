/*
 * Copyright (c) 2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-26     sogwms       The first version
 */

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <stdint.h>


#define GET_SYSTEM_MS()     get_system_ms() 

struct controller_pid_param
{
    float kp;
    float ki;
    float kd;
};

struct controller_param
{
    union
    {
        struct controller_pid_param pid;
    } data;
};

typedef struct controller_param *controller_param_t;

struct controller
{
    float           target;
    float           output;
    uint16_t        sample_time;    // unit:ms
    int             enable;

    int             (*update)(void *controller, float current_point);
    int             (*reset)(void *controller);
    int             (*destroy)(void *controller);
    int             (*set_param)(void *controller, controller_param_t param);
    int             (*get_param)(void *controller, controller_param_t param);
};

typedef struct controller *controller_t;

controller_t        controller_create(uint32_t size, uint16_t sample_time);
int            controller_update(controller_t controller, float current_point);
int            controller_destroy(controller_t controller);
int            controller_enable(controller_t controller);
int            controller_disable(controller_t controller);
int            controller_reset(controller_t controller);
int            controller_set_target(controller_t controller, int16_t target);
int            controller_set_sample_time(controller_t controller, uint16_t sample_time);
int            controller_set_param(controller_t controller, controller_param_t param);
int            controller_get_param(controller_t controller, controller_param_t param);

#endif // __CONTROLLER_H__
