#include <string.h>
#include "lwipopts.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/def.h"
#include "lwip/apps/cookies.h"
#include "timers.h"
#include "lwip/apps/httpd.h"

#if LWIP_HTTPD_SUPPORT_AUTH_COOKIE

#if !defined(LWIP_HTTPD_AUTH_COOKIE_LENGTH) || !LWIP_HTTPD_AUTH_COOKIE_LENGTH || LWIP_HTTPD_AUTH_COOKIE_LENGTH % 8 != 0
#error "LWIP_HTTPD_AUTH_COOKIE_LENGTH must be a multiple of 8"
#endif

#if !defined(configUSE_TIMERS) || !configUSE_TIMERS
#error "configUSE_TIMERS must be enabled"
#endif

struct http_cookie {
    char string[LWIP_HTTPD_AUTH_COOKIE_LENGTH + 1];
    u32_t ctime;
    struct http_cookie *next;
};

struct http_cookie_list {
    struct http_cookie *head;
    u32_t count;
    sys_mutex_t mux;
    sys_timer_t timer;
    volatile u32_t ticks;
};

static struct http_cookie_list cookie_list;

/**
 * @brief destory cookie of list head (oldest)
 */
static void httpd_cookie_destory_head(void) {
    sys_mutex_lock(&cookie_list.mux);
    struct http_cookie *del_cookie = cookie_list.head;
    if (del_cookie != NULL) {
        cookie_list.head = del_cookie->next;
        cookie_list.count--;
        LWIP_DEBUGF(COOKIES_DEBUG, ("cookie[%s]: destory, left:%u\r\n", del_cookie->string, cookie_list.count));
        mem_free(del_cookie);
    }
    sys_mutex_unlock(&cookie_list.mux);
}

/**
 * @brief destory a cookie for cookie_list
 * @param cookie_string value of the cookie that want to destory
 */
err_t httpd_cookie_destory_by_value(const char *cookie_string) {
    err_t ret = ERR_VAL;
    sys_mutex_lock(&cookie_list.mux);
    if (cookie_list.head != NULL && cookie_string != NULL) {
        struct http_cookie **target, *del_cookie;

        for (target = &cookie_list.head; *target != NULL; target = &(*target)->next) {
            if (memcmp((*target)->string, cookie_string, LWIP_HTTPD_AUTH_COOKIE_LENGTH) == 0) {
                del_cookie = *target;
                *target = del_cookie->next;
                cookie_list.count--;
                LWIP_DEBUGF(COOKIES_DEBUG, ("cookie[%s]: destory, left:%u\r\n", del_cookie->string, cookie_list.count));
                mem_free(del_cookie);
                ret = ERR_OK;
                break;
            }
        }
    }
    sys_mutex_unlock(&cookie_list.mux);
    return ret;
}

/**
 * @brief insert new cookie to the cookie list
 * @param new_cookie pointer of the new cookie
 */
static void httpd_cookie_insert(struct http_cookie *new_cookie) {
    sys_mutex_lock(&cookie_list.mux);
    if (cookie_list.head == NULL) {
        cookie_list.head = new_cookie;
    } else {
        if (cookie_list.count >= LWIP_HTTPD_AUTH_SESSION_MAXNUM) {
            httpd_cookie_destory_head();
        }
        struct http_cookie *pre_cookie = cookie_list.head;
        while (pre_cookie->next != NULL) {
            pre_cookie = pre_cookie->next;
        }
        pre_cookie->next = new_cookie;
    }
    new_cookie->next = NULL;
    cookie_list.count++;
    LWIP_DEBUGF(COOKIES_DEBUG, ("cookie[%s]: insert , left:%u\r\n", new_cookie->string, cookie_list.count));
    sys_mutex_unlock(&cookie_list.mux);
}

/**
 * @brief handler of the httpd cookies timer, used to check cookie expried
 * @param xTimer freertos soft timer handle, not used
 */
static void httpd_cookie_timer(sys_timer_t timer) {
    LWIP_UNUSED_ARG(timer);
    sys_mutex_lock(&cookie_list.mux);

    /* increase ticks */
    cookie_list.ticks++;

    /* check cookie expried */
    struct http_cookie *target = cookie_list.head;
    while (target != NULL) {
        if ((cookie_list.ticks - target->ctime) < LWIP_HTTPD_AUTH_COOKIE_MAX_AGE)
            break;

        httpd_cookie_destory_head();
        target = cookie_list.head;
    }
    sys_mutex_unlock(&cookie_list.mux);
}

/**
 * @brief check the generated cookie is exists in cookie list
 * @param cookie_string value of the generated cookie
 * @return true on exists, false on other wise
 */
static int httpd_cookie_check_exists(const char *cookie_string) {
    int ret = false;
    sys_mutex_lock(&cookie_list.mux);
    for (struct http_cookie *target = cookie_list.head; target != NULL; target = target->next) {
        if (memcmp(target->string, cookie_string, LWIP_HTTPD_AUTH_COOKIE_LENGTH) == 0) {
            ret = true;
            break;
        }
    }
    sys_mutex_unlock(&cookie_list.mux);
    return ret;
}

/**
 * @brief generated new cookie
 * @return value of the new cookie
 */
const char *httpd_cookie_generate(void) {
    /* generate session id */
    struct http_cookie *new_cookie;

    new_cookie = mem_malloc(sizeof(struct http_cookie));
    LWIP_ASSERT("can't create new http cookie, no memory", new_cookie != NULL);

    /* generate cookie */
    do {
        int offset = 0;
        int count = LWIP_HTTPD_AUTH_COOKIE_LENGTH >> 3;
        int buf_len = LWIP_HTTPD_AUTH_COOKIE_LENGTH + 1;
        for (int i = 0; i < count; i++, offset += 8, buf_len -= 8) {
            snprintf(new_cookie->string + offset, buf_len, "%08lX", (u32_t)LWIP_RAND());
        }
    } while (httpd_cookie_check_exists(new_cookie->string) == true);

    new_cookie->ctime = cookie_list.ticks;
    /* insert to cookies list */
    httpd_cookie_insert(new_cookie);
    return (const char *)(new_cookie->string);
}

/**
 * @brief httpd cookies handler init
 */
void httpd_cookie_init(void) {
    err_t ret;
    ret = sys_mutex_new(&(cookie_list.mux));
    LWIP_ASSERT("can't create cookies mutex", ret == ERR_OK);
    ret = sys_timer_new(&cookie_list.timer, "cookie timer", 1, httpd_cookie_timer);
    LWIP_ASSERT("can't create cookie timer", ret == ERR_OK);
}

/**
 * @brief check valid of cookie
 * @param cookie_string value of cookie
 * @return true on valid, false on otherwise
 */
bool httpd_cookie_verify(const char *cookie_string) {
    return httpd_cookie_check_exists(cookie_string);
}

int httpd_authorized_for_uri(void *connection, const char *uri, u32_t uri_len) {
    if (httpd_login_auth(connection) == 1) {
        return HTTP_AUTH_SET_COOKIE;
    }
    if ((lwip_strnistr(uri, "html", uri_len) == NULL) || (httpd_cookie_auth(connection) == 1)) {
        return HTTP_AUTH_OK;
    }
    return HTTP_AUTH_DENIED;
}

err_t sys_timer_new(sys_timer_t *timer, const char *name, u32_t period, sys_timer_cb_t func) {
    timer->tim = xTimerCreate(name, pdMS_TO_TICKS(period * 1000), pdTRUE, NULL, (TimerCallbackFunction_t)func);
    if (timer->tim != NULL && xTimerStart(timer->tim, 0) == pdPASS) {
        return ERR_OK;
    }
    return ERR_MEM;
}

#endif /* LWIP_HTTPD_SUPPORT_AUTH_COOKIE */
