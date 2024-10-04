#ifndef __AT_UTILS_H__
#define __AT_UTILS_H__

#include <stdint.h>

extern void at_print_raw_cmd(const char *name, const char *buf, uint32_t size);
extern const char *at_get_last_cmd(uint32_t *cmd_size);
extern uint32_t at_utils_send(uint32_t uart_idx, uint32_t pos, const void *buffer, uint32_t size);
extern uint32_t at_vprintf(uint32_t uart_idx, const char *format, va_list args);
extern uint32_t at_vprintfln(uint32_t uart_idx, const char *format, va_list args);


#endif