#ifndef UART_H
#define UART_H

struct status
{
 unsigned char send_finish;		//串口完成发送一帧数据标志位
 unsigned char rece_finish;		//串口完成接收一帧数据标志位
 unsigned char rece_error;			//串口接收出错标志
 unsigned char send_error;			//串口发送出错标志
};
extern struct status data uart_status;

extern unsigned char uart_buffer[255];	//串口缓冲区

extern unsigned char data uart_point;		//数据指针


extern void Timer_Uart_Init();
extern void Uart_Send_Msg(unsigned char *msg);
extern void Uart_Rece_Msg(unsigned char recr_len);
extern void Uart_Send_Hex(unsigned char *input_byte,unsigned int input_len);
//extern void Hex_to_BCD(unsigned long hex_input,unsigned char *BCD_output);
#endif