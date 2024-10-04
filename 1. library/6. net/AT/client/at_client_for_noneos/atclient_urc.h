#ifndef __ATCLIENT_URC_H__
#define __ATCLIENT_URC_H__

#include "stdint.h"

typedef struct {
    const char *str;
    uint16_t    len;
    void (*cb)(void *args);
} at_urc_t;


extern const at_urc_t urc_prefix[];

#endif