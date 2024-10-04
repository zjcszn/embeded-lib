#ifndef _MACHTALK_REMOTE_CONTROL_H
#define _MACHTALK_REMOTE_CONTROL_H


#include "app_cfg.h"
#if MACHTALK_ENABLE

int is_machtalk_socket(int socket);
void handle_machtalk_rcv(int socket, uint8_t *buff, int size);
int test_connect_to_machtalk_server(void);
#endif
#endif

