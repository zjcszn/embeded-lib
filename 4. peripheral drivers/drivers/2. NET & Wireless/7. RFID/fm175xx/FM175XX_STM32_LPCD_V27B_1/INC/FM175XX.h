
#ifndef _FM175XX_H
#define _FM175XX_H
#include "stm32f10x_map.h"


extern void FM175XX_HardReset(void);
extern unsigned char FM175XX_SoftReset(void);
extern void FM175XX_HPD(unsigned char mode);
extern void GetReg(unsigned char addr,unsigned char* regdata);
extern void SetReg(unsigned char addr,unsigned char regdata);
extern void ModifyReg(unsigned char addr,unsigned char mask,unsigned char set);
extern void SPI_Write_FIFO(unsigned char reglen,unsigned char* regbuf); //SPI接口连续写FIFO 
extern void SPI_Read_FIFO(unsigned char reglen,unsigned char* regbuf);  //SPI接口连续读FIFO

extern void Clear_FIFO(void);
extern void GetReg_Ext(unsigned char ExtRegAddr,unsigned char* ExtRegData);
extern void SetReg_Ext(unsigned char ExtRegAddr,unsigned char ExtRegData);
extern void ModifyReg_Ext(unsigned char ExtRegAddr,unsigned char mask,unsigned char set);
#endif





