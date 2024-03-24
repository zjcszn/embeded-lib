

#ifndef __LWIP_HTTPD_COOKIES_H__
#define __LWIP_HTTPD_COOKIES_H__

#include "stdbool.h"

#if LWIP_HTTPD_SUPPORT_AUTH_COOKIE

enum {
    HTTP_AUTH_OK,
    HTTP_AUTH_DENIED,
    HTTP_AUTH_SET_COOKIE,
};

void httpd_cookie_init(void);
bool httpd_cookie_verify(const char *cookie_string);
const char *httpd_cookie_generate(void);

extern int httpd_authorized_for_uri(void *connection, const char *uri, u32_t uri_len);

struct _sys_tim {
    void *tim;
};
typedef struct _sys_tim sys_timer_t;
typedef void (*sys_timer_cb_t)(sys_timer_t);

err_t sys_timer_new(sys_timer_t *timer, const char *name, u32_t period, sys_timer_cb_t func);

#endif /* LWIP_HTTPD_SUPPORT_AUTH_COOKIE */

#endif //__LWIP_HTTPD_COOKIES_H__
