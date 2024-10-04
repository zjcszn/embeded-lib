#ifndef	SPI_H
#define SPI_H
extern unsigned char SPIRead(unsigned char addr);				//SPI¶Áº¯Êý
extern void SPIWrite(unsigned char add,unsigned char wrdata);	//SPIÐ´º¯Êý
extern void SPIRead_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value);
extern void SPIWrite_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value);

extern unsigned char SPI_Init();

#endif