/* generated thread header file - do not edit */
#ifndef MBPORTSSP_H_
#define MBPORTSSP_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void mbportssp_entry(void);
#else
extern void mbportssp_entry(void);
#endif
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_sci_uart.h"
#include "r_uart_api.h"
#ifdef __cplusplus
extern "C" {
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer0;
#ifndef mbport_timer_ms
void mbport_timer_ms(timer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer0;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t g_uart0;
#ifdef NULL
#else
extern void NULL(uint32_t channel, uint32_t level);
#endif
#ifndef user_uart_callback
void user_uart_callback(uart_callback_args_t *p_args);
#endif
extern TX_MUTEX g_mbport_mutex0;
extern TX_SEMAPHORE g_mbport_sem;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* MBPORTSSP_H_ */
