#ifndef UART_H
#define UART_H
extern unsigned char data timer_L,timer_H;
//extern unsigned char xdata uart_buffer[255];	//串口缓冲区
extern unsigned char data uart_data_length;		//串口数据长度
extern unsigned char data uart_verify;			//串口数据校验
extern unsigned char data uart_point;		//数据指针
extern bit data uart_receive_finish_flag;		//串口完成接收一帧有效数据标志位

extern bit data uart_send_finish_flag;			//串口完成发送一帧数据标志位
extern bit data uart_receive_error_flag;			//串口接收出错标志
extern bit data uart_send_error_flag;			//串口发送出错标志

extern void Timer_Uart_Init();
extern void Uart_Send_Msg(unsigned char *msg);
extern void Uart_Rece_Msg(unsigned char recr_len);
extern void Uart_Send_Hex(unsigned char *input_byte,unsigned int input_len);
extern void Hex_to_BCD(unsigned long hex_input,unsigned char *BCD_output);
#endif