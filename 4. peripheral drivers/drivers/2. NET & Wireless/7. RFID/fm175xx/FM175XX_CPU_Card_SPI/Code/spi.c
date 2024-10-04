/*************************************************************/
//2014.07.15修改版
/*************************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "pin.h"
#include "spi.h"
#include "main.h"


unsigned char SPI_Init()
{
	SPDAT=0;
	SPSTAT=SPIF|WCOL;//清除SPIF与WCOL标志
	SPCTL=0xD0;//SSIG=1,SPEN=1,DORD=0,MSTR=1,CPOL=0,CPHA=0,SPR1=0,SPR0=0 (SPI_CLK=CPU_CLK/4)
	AUXR1=0x00;//PCA_P4=0,SPI_P4=0(SPI在P1 Port),S2_P4=0,GF2=0,ADRJ=0,DPS=0
	return OK;
}
/*******************************************************************************************************/
/*名称：																							   */
/*功能：SPI接口读取数据	SPI read function															   */
/*输入:																								   */
/*		N/A																							   */
/*输出：																							   */
/*		addr:	读取FM175XX内的寄存器地址[0x00~0x3f]	reg_address									   */
/*		rddata:	读取的数据							reg_data										   */
/*******************************************************************************************************/
unsigned char SPIRead(unsigned char addr)	//SPI读函数
{
 unsigned char data reg_value,send_data;
	SPI_CS=0;

   	send_data=(addr<<1)|0x80;
	SPSTAT=SPIF|WCOL;//清除SPIF与WCOL标志
	SPDAT=send_data;
	while(!(SPSTAT&SPIF));

	SPSTAT=SPIF|WCOL;//清除SPIF与WCOL标志
	SPDAT=0x00;
	while(!(SPSTAT&SPIF));
	reg_value=SPDAT;
	SPI_CS=1;
	return(reg_value);
}

void SPIRead_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value)	
//SPI连续读函数,用于READ FIFO函数
{
 unsigned char data i,send_data;
 	if (sequence_length==0)
		return;

	SPI_CS=0;

   	send_data=(addr<<1)|0x80;
	SPSTAT=SPIF|WCOL;//清除SPIF与WCOL标志
	SPDAT=send_data;
	while(!(SPSTAT&SPIF));

	for (i=0;i<sequence_length;i++)
	{
		SPSTAT=SPIF|WCOL;//清除SPIF与WCOL标志
		if (i==sequence_length-1)
			SPDAT=0x00;//最后一次读取时，地址填入0x00
		else
			SPDAT=send_data;

		while(!(SPSTAT&SPIF));
		*(reg_value+i)=SPDAT;
 	}
	SPI_CS=1;
	return;
}	
/*******************************************************************************************************/
/*名称：SPIWrite																					   */
/*功能：SPI接口写入数据	  SPI write function														   */
/*输入:																								   */
/*		add:	写入FM17XX内的寄存器地址[0x00~0x3f]	  reg_address									   */
/*		wrdata:   要写入的数据						  reg_data										   */
/*输出：																							   */
/*		N/A																							   */
/*******************************************************************************************************/
void SPIWrite(unsigned char addr,unsigned char wrdata)	//SPI写函数
{
unsigned char data send_data;
	SPI_CS=0;				   

	send_data=(addr<<1)&0x7e;
	
  	SPSTAT=SPIF|WCOL;//清除SPIF与WCOL标志
	SPDAT=send_data;
	while(!(SPSTAT&SPIF));

	SPSTAT=0xC0;
	SPDAT=wrdata;
	while(!(SPSTAT&SPIF));

	SPI_CS=1;
	return ;	
}

void SPIWrite_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value)
//SPI连续写函数,用于WRITE FIFO函数
{
unsigned char data send_data,i;
	if(sequence_length==0)
		return;

	SPI_CS=0;

	send_data=(addr<<1)&0x7e;
	
  	SPSTAT=SPIF|WCOL;//清除SPIF与WCOL标志
	SPDAT=send_data;
	while(!(SPSTAT&SPIF));

	for (i=0;i<sequence_length;i++)
	{
		SPSTAT=0xC0;
		SPDAT=*(reg_value+i);
		while(!(SPSTAT&SPIF));
	}

	SPI_CS=1;
	return ;	
}