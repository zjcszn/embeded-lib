/*
 * Copyright (c) 2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-26     sogwms       The first version
 */

#include "controller.h"

#define DBG_SECTION_NAME  "controller"
#define DBG_LEVEL         DBG_LOG

// single intput and single output system

controller_t controller_create(uint32_t size, uint16_t sample_time)
{
    // Malloc memory and initialize PID
    controller_t new_controller = (controller_t)malloc(size);
    if(new_controller == NULL)
    {
        LOG_E("Failed to malloc memory for new controller\n");
        return NULL;
    }

    new_controller->sample_time = sample_time;
    new_controller->enable = 0;

    return new_controller;
}

int controller_update(controller_t controller, float current_point)
{
    assert(controller != NULL);

    if (controller->enable)
    {
        return controller->update(controller, current_point);
    }
    else
    {
        controller->output = controller->target;
    }
    
    return 0;
}

int controller_destroy(controller_t controller)
{
    assert(controller != NULL);
    
    LOG_D("Free controller");

    return controller->destroy(controller);
}

int controller_enable(controller_t controller)
{
    assert(controller != NULL);

    LOG_D("Enabling controller");
    controller->enable = 1;

    return 0;
}

int controller_disable(controller_t controller)
{
    assert(controller != NULL);

    LOG_D("Disabling controller");
    controller->enable = 0;

    return 0;
}

int controller_reset(controller_t controller)
{
    assert(controller != NULL);

    return controller->reset(controller);
}

int controller_set_target(controller_t controller, uint16_t target)
{
    assert(controller != NULL);

    controller->target = target;
    return 0;
}

int controller_set_sample_time(controller_t controller, uint16_t sample_time)
{
    assert(controller != NULL);

    controller->sample_time = sample_time;
    return 0;
}

int controller_set_param(controller_t controller, controller_param_t param)
{
    assert(controller != NULL);

    return controller->set_param(controller, param);
}

int controller_get_param(controller_t controller, controller_param_t param)
{
    assert(controller != NULL);

    return controller->get_param(controller, param);
}

