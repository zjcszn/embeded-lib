#ifndef _TCP_H
#define _TCP_H

#include "api.h"

struct ret_connect_stat
{
	uint32_t local_addr;
	uint32_t remote_addr;
	uint16_t local_port;
	uint16_t remote_port;
	uint8_t socket_num;
	uint8_t type; //tcp udp
};

int my_lwip_init(void);
int netif_addr_init(void);
int init_lwip(void);
int socket_link(char *ip, uint16_t port);
int auto_get_ip(void);
int get_host_by_name(char *hostname, uint32_t *addr);
int tcp_link(char *ip, uint16_t port);
int udp_link(char *ip, uint16_t port);
int udp_add_membership(char *ip, uint16_t port);
int udp_create_server(uint16_t port);
int udp_data_send(int socket, void *data, int len, uint16_t remote_port, uint32_t remote_addr);
extern struct netif *p_netif;

#endif
