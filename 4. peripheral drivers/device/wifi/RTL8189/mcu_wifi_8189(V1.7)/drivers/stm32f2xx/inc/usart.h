#ifndef __USART_H
#define	__USART_H

#include "api.h"

#define UART_PKG_SIZE  			256

#define UART_RECV_BUFF_SIZE 	(UART_PKG_SIZE + 32)

#define UART1_DEFAULT_BAUD  115200
#define UART3_DEFAULT_BAUD  115200

typedef struct _UART_PKG
{
 	atomic		pending;
	u32_t   	size;
  u8_t 			buff[UART_PKG_SIZE];    //buff包含格式:0xaa +CMD +LENL+LENH+SEQ +DATA0+…+DATAN+ SUM + 0x55
}UART_PKG;

typedef struct _UART_RECV
{	 
	u32_t 		over_err_num;   
	u32_t 		ne_err_num;  
	u32_t 		fe_err_num; 
	u32_t 		pkg_over_err;  
	u32_t 		recv_num;
	uint32_t 	pos;
	u8_t 		c_buff[UART_RECV_BUFF_SIZE]; 		//循环缓冲区
	UART_PKG 	pkg;  //保存一帧数据	 
}UART_RECV;

typedef struct _UART_SEND
{
	wait_event_t 	wait;
	u32_t 		send_num;
	u32_t 		cnt;
	u32_t 		size;
	atomic		sending;
	u8_t 		err;
	u8_t*		buff; 
}UART_SEND;

extern UART_RECV u1_recv;
extern UART_SEND u3_send;
extern UART_SEND u1_send;

void uart_interface_off(int onoff);
int set_baud(int uart_num,uint32_t baud);
void enable_rcvdata(void);
void uart1_init(void);
void uart3_init(void);
int uart1_send(u8_t *buff,u32_t size);
int uart2_send(u8_t *buff,u32_t size);
int handle_uart1_rcv(void);
void send_cmd_to_self(char cmd);


#endif /* __USART1_H */
