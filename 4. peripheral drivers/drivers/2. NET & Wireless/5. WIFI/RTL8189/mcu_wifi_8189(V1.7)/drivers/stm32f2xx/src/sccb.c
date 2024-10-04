#include "drivers.h"
#include "app.h"
#include "api.h"


#define SCCB_SID_H()	GPIO_SET(SIO_D_PORT_GROUP, SIO_D_PIN)
#define SCCB_SID_L()	GPIO_CLR(SIO_D_PORT_GROUP, SIO_D_PIN)

#define SCCB_SIC_H()	GPIO_SET(SIO_C_PORT_GROUP, SIO_C_PIN)
#define SCCB_SIC_L()	GPIO_CLR(SIO_C_PORT_GROUP, SIO_C_PIN)

#define SCCB_SID_IN		SIO_D_IN
#define SCCB_SID_OUT		SIO_D_OUT
#define SCCB_SID_STATE	GPIO_STAT(SIO_D_PORT_GROUP, SIO_D_PIN)

void SCCB_INIT(void)
{
	gpio_cfg((uint32_t)SIO_C_PORT_GROUP, SIO_C_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)SIO_D_PORT_GROUP, SIO_D_PIN, GPIO_Mode_Out_OD);
}

/*
-----------------------------------------------
   功能: start命令,SCCB的起始信号
   参数: 无
 返回值: 无
-----------------------------------------------
*/
void startSCCB(void)
{
    SCCB_SID_H();     //数据线高电平
    delay_us(15);

    SCCB_SIC_H();	   //在时钟线高的时候数据线由高至低
    delay_us(15);
 
    SCCB_SID_L();
    delay_us(15);

    SCCB_SIC_L();	 //数据线恢复低电平，单操作函数必要
    delay_us(15);


}
/*
-----------------------------------------------
   功能: stop命令,SCCB的停止信号
   参数: 无
 返回值: 无
-----------------------------------------------
*/
void stopSCCB(void)
{
    SCCB_SID_L();
    delay_us(15);
 
    SCCB_SIC_H();	
    delay_us(15);
  

    SCCB_SID_H();	
    delay_us(15);
   
}

/*
-----------------------------------------------
   功能: noAck,用于连续读取中的最后一个结束周期
   参数: 无
 返回值: 无
-----------------------------------------------
*/
void noAck(void)
{
	
	SCCB_SID_H();	
	delay_us(15);
	
	SCCB_SIC_H();	
	delay_us(15);
	
	SCCB_SIC_L();	
	delay_us(15);
	
	SCCB_SID_L();	
	delay_us(15);

}

/*
-----------------------------------------------
   功能: 写入一个字节的数据到SCCB
   参数: 写入数据
 返回值: 发送成功返回1，发送失败返回0
-----------------------------------------------
*/
unsigned char SCCBwriteByte(unsigned char m_data)
{
	unsigned char j,tem;

	for(j=0;j<8;j++) //循环8次发送数据
	{
		if((m_data<<j)&0x80)
		{
			SCCB_SID_H();	
		}
		else
		{
			SCCB_SID_L();	
		}
		delay_us(15);
		SCCB_SIC_H();	
		delay_us(15);
		SCCB_SIC_L();	
		delay_us(15);

	}
	delay_us(15);
	SCCB_SID_IN;/*设置SDA为输入*/
	delay_us(15);
	SCCB_SIC_H();	
	delay_us(15);
	if(SCCB_SID_STATE){tem=0;}   //SDA=1发送失败，返回0}
	else {tem=1;}   //SDA=0发送成功，返回1
	SCCB_SIC_L();	
	delay_us(15);	
  	SCCB_SID_OUT;/*设置SDA为输出*/

	return (tem);  
}

/*
-----------------------------------------------
   功能: 一个字节数据读取并且返回
   参数: 无
 返回值: 读取到的数据
-----------------------------------------------
*/
unsigned char SCCBreadByte(void)
{
	unsigned char read,j;
	read=0x00;
	
	SCCB_SID_IN;/*设置SDA为输入*/
	delay_us(15);
	for(j=8;j>0;j--) //循环8次接收数据
	{		     
		delay_us(15);
		SCCB_SIC_H();
		delay_us(15);
		read=read<<1;
		if(SCCB_SID_STATE) 
		{
			read=read+1;
		}
		SCCB_SIC_L();
		delay_us(15);
	}	
	return(read);
}
