#ifndef _WEBSEVER_H__
#define _WEBSEVER_H__

#include "app_cfg.h"

#if SUPPORT_WEBSERVER

typedef enum _WEB_SM
{
	WEB_SM_IDLE,
		
	//must together
	WEB_SM_GET_GETMORE,
	WEB_SM_RESPONE_GET,

	//must together
	WEB_SM_POST_GETMORE,
	WEB_SM_POST_GETMORE1,
	WEB_SM_RESPONE_POST,

	WEB_SM_SEND_DATA,
	
}WEB_SM;

#define GET_NAME_LEN 32


struct WEB_CFG_LINK{
	char	mode;
	char	channel;
	char	essid[33];
	char	key[14];
}__packed;

struct WEB_CFG_IP{
	unsigned int	ip;
	unsigned int	gw;
	unsigned int	msk;
	unsigned int	dns;
	unsigned int	auto_get;
}__packed;

struct WEB_CFG_SERVER{
	unsigned int	tcpudp;
	unsigned int	port;
	unsigned int	open;
}__packed;

struct WEB_CFG_DTU{
	char			server_name[64 + 1];
	unsigned int	tcpudp;
	unsigned int	server_ip;
	unsigned int	port;
	unsigned int	baud;
	unsigned int	open;
}__packed;


typedef struct  _WEB_CFG
{
	struct WEB_CFG_LINK		link;
	struct WEB_CFG_IP		ip;
	struct WEB_CFG_SERVER	server;
	struct WEB_CFG_DTU		dtu;
	char	password[32];

	//struct WEB_GPIO 		gpio;
	WEB_SM web_sm;
	void (*cur_routing)(UCHAR s, CHAR _step);
	int (*assemble_routing)(UCHAR s, CHAR _step);
	char get_name[GET_NAME_LEN];
	char str_end_len;
	char is_update_post;
	char update_data_timer;
	char need_reboot;
	int update_data_cnt;
	int post_len;
	uint32_t update_addr;
	uint32_t update_remain_size;
} WEB_CFG;

int handle_web_sm(UCHAR s, PUCHAR packet ,uint32_t packetlength);

void web_server_init(void) ;
int wifi_link_page_assemble(UCHAR s, CHAR _sub_step) ;
int web_body_add_input_str(char *buff, const char *tag, const char *str) ;
int web_body_add_option_str(char *buff, const char *tag, const char *str) ;
void sub_page_send(UCHAR s, CHAR _step) ;
char *get_head_tag(const char *tag, const char *buff, int *len) ;
char *get_name(char *head, char *buff, int *len) ;
char *get_body_tag(const char *tag, const char *buff, int *len) ;
int get_post_tag_value(const char *tag, const char *buff, char *str_val) ;
int ip_addr_page_assemble(UCHAR s, CHAR _sub_step) ;
int tcp_server_page_assemble(UCHAR s, CHAR _sub_step) ;
int tcp_client_page_assemble(UCHAR s, CHAR _sub_step) ;
int remote_page_assemble(UCHAR s, CHAR _sub_step) ;
int reset_page_assemble(UCHAR s, CHAR _sub_step) ;
int uart_page_assemble(UCHAR s, CHAR _sub_step) ;
int io_page_assemble(UCHAR s, CHAR _sub_step) ;
int update_page_assemble(UCHAR s, CHAR _sub_step) ;
void web_update_timer(void) ;


int handle_wifilink_post(int s, char *buff) ;
int handle_ip_addr_post(int s, char *buff) ;
int handle_tcp_server_post(int s, char *buff) ;
int handle_uart_post(int s, char *buff) ;
int handle_remote_ctl_post(int s, char *buff) ;
int handle_login_post(int s, char *buff) ;
int handle_reset_post(int s, char *buff) ;
int handle_cust_post(int s, char *buff) ;
int handle_io_post(int s, char *buff) ;
int handle_tcp_client_post(int s, char *buff) ;
void handle_web_get(UCHAR s) ;
void handle_web_post(int s) ;
	
extern WEB_CFG web_cfg;

#endif
#endif

