/**
 * \file            lwwdg.c
 * \brief           LwWDG main file
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LWWDG - Lightweight watchdog for RTOS in embedded systems.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.0.1
 */

#include "lwwdg.h"
#include "components.h"

#include <string.h>

/**
 * \brief           Get system time in milliseconds.
 *
 * It is required to keep reload time
 */
#define LWWDG_GET_TIME()    xTaskGetTickCount()

/**
 * \brief           Lock the critical section.
 *
 * Critical section should prevent other tasks or interrupt,
 * to access to the same core.
 *
 * Easiest is to simply disable the interrupt, since task is normally pretty quick.
 *
 * \note            Default implementation is for *Cortex-M*
 */
#define LWWDG_CRITICAL_SECTION_LOCK()   taskENTER_CRITICAL()


/**
 * \brief           Unlock the critical section.
 *
 * \note            Default implementation is for *Cortex-M*
 */
#define LWWDG_CRITICAL_SECTION_UNLOCK() taskEXIT_CRITICAL()


static lwwdg_wdg_t* wdgs;

/**
 * \brief           Initialize watchdog module
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwwdg_init(void) {
    wdgs = NULL;
    return 1;
}

/**
 * \brief           Add new watchdog timer instance to internal linked list.
 * 
 * \param           wdg: Watchdog handle. Must not be local variable
 * \param           timeout: Max allowed timeout in milliseconds
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwwdg_add(lwwdg_wdg_t* wdg, uint32_t timeout) {
    uint8_t ret = 1;

    wdg->timeout = timeout;
    wdg->last_reload_time = LWWDG_GET_TIME();
    LWWDG_CRITICAL_SECTION_LOCK();
    /* Check if already on a list -> we don't want that */
    for (lwwdg_wdg_t* w = wdgs; w != NULL; w = w->next) {
        if (w == wdg) {
            ret = 0;
        }
    }
    if (ret) {
        /* Add to beginning of a list */
        wdg->next = wdgs;
        wdgs = wdg;
    }
    LWWDG_CRITICAL_SECTION_UNLOCK();
    return ret;
}

/**
 * \brief           Remove watchdog from the list
 * 
 * This function is typically used if a task
 * is killed by the scheduler. A user must manually
 * call the function and can later clean wdg memory
 * 
 * \param           wdg: Watchdog handle to remove from list
 * \return          `1` if removed, `0` otherwise
 */
uint8_t lwwdg_remove(lwwdg_wdg_t* wdg) {
    lwwdg_wdg_t** ind;
    uint8_t ret = 0;

    LWWDG_CRITICAL_SECTION_LOCK();
    /* Check if watchdog exists on a list */
    ind = &wdgs; /* Get address of the first entry */
    while (*ind != NULL && *ind != wdg) {
        ind = &(*ind)->next;
    }

    /* ind should not be NULL if entry exists on linked list */
    if (ind != NULL) {
        *ind = wdg->next; /* Remove entry from the list */
    }
    LWWDG_CRITICAL_SECTION_UNLOCK();

    return ret;
}

/**
 * \brief           Reload thread watchdog
 * 
 * \note            Reload will not be successful, if there was a timeout before.
 *                  This will ensure that main thread won't reload hardware watchdog,
 *                  resulting system to reset
 * 
 * \param           wdg: Watchdog handle to reload
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwwdg_reload(lwwdg_wdg_t* wdg) {
    uint8_t ret = 0;
    uint32_t time = LWWDG_GET_TIME();

    LWWDG_CRITICAL_SECTION_LOCK();
    if ((time - wdg->last_reload_time) < wdg->timeout) {
        wdg->last_reload_time = time;
        ret = 1;
    }
    LWWDG_CRITICAL_SECTION_UNLOCK();
    return ret;
}

/**
 * \brief           Process and check system timers
 * 
 * Function will check all timers and will return OK,
 * if all timers are within max timeout state
 * 
 * \return          `1` if hardware watchdog can be reset,
 *                      `0` if at least one timer hasn't been reloaded
 *                      within maximum timeout
 */
uint8_t lwwdg_process(void) {
    uint8_t ret = 1;
    uint32_t time = LWWDG_GET_TIME();

    LWWDG_CRITICAL_SECTION_LOCK();
    for (lwwdg_wdg_t* wdg = wdgs; wdg != NULL; wdg = wdg->next) {
        if ((time - wdg->last_reload_time) > wdg->timeout) {
            ret = 0;
            break;
        }
    }
    LWWDG_CRITICAL_SECTION_UNLOCK();
    return ret;
}
