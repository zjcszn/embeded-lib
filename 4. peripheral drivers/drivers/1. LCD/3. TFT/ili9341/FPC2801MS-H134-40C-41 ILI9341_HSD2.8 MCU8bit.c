
void WriteComm(unsigned char i)
{	

		RS=0;	
		CS0=0;
		RD0=1;
		WR0=0;  
		DBL=i;	
		WR0=1; 	
		CS0=1; 

}

void WriteData(unsigned char i)
{
	
		RS=1;		
		CS0=0;
		RD0=1;
		WR0=0;  	
		DBL=i; 	
		WR0=1; 
		CS0=1; 

	
}


void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend) reentrant
{
	WriteComm(0x2a);   
	WriteData(Xstart>>8);
	WriteData(Xstart&0xff);
	WriteData(Xend>>8);
	WriteData(Xend&0xff);

	WriteComm(0x2b);   
	WriteData(Ystart>>8);
	WriteData(Ystart&0xff);
	WriteData(Yend>>8);
	WriteData(Yend&0xff);

	WriteComm(0x2c);
}



void LCD_Init(void)
{

	
	RST=1;  
	Delay(5);

	RST=0;
	Delay(10);

	RST=1;
	Delay(120);



//------------------------------------LCD_CODE------------------------------------------------------------------------------------------------------------------------//
//************* Start Initial Sequence **********// 
WriteComm(0xCF);  
WriteData(0x00); 
WriteData(0xC1); 
WriteData(0X30); 
 
WriteComm(0xED);  
WriteData(0x64); 
WriteData(0x03); 
WriteData(0X12); 
WriteData(0X81); 
 
WriteComm(0xE8);  
WriteData(0x85); 
WriteData(0x00); 
WriteData(0x78); 
 
WriteComm(0xCB);  
WriteData(0x39); 
WriteData(0x2C); 
WriteData(0x00); 
WriteData(0x34); 
WriteData(0x02); 
 
WriteComm(0xF7);  
WriteData(0x20); 
 
WriteComm(0xEA);  
WriteData(0x00); 
WriteData(0x00); 
 
WriteComm(0xC0);    //Power control 
WriteData(0x22);   //VRH[5:0] 
 
WriteComm(0xC1);    //Power control 
WriteData(0x11);   //SAP[2:0];BT[3:0] 
 
WriteComm(0xC5);    //VCM control 
WriteData(0x3F); 
WriteData(0x3C); 
 
WriteComm(0xC7);    //VCM control2 
WriteData(0X90); 
 
WriteComm(0x36);    // Memory Access Control 
WriteData(0x08); 
 
WriteComm(0x3A);   
WriteData(0x55); 

WriteComm(0xB1);   
WriteData(0x00);   
WriteData(0x18); 
 
WriteComm(0xB6);    // Display Function Control 
WriteData(0x0A); 
WriteData(0xA2); 

WriteComm(0xF2);    // 3Gamma Function Disable 
WriteData(0x00); 
 
WriteComm(0x26);    //Gamma curve selected 
WriteData(0x01); 
 
WriteComm(0xE0);    //Set Gamma 
WriteData(0x0F); 
WriteData(0x0B); 
WriteData(0x17); 
WriteData(0x07); 
WriteData(0x0D); 
WriteData(0x05); 
WriteData(0X45); 
WriteData(0xB5); 
WriteData(0x39); 
WriteData(0x0A); 
WriteData(0x15); 
WriteData(0x07); 
WriteData(0x09); 
WriteData(0x02); 
WriteData(0x00); 

WriteComm(0XE1);    //Set Gamma 
WriteData(0x00); 
WriteData(0x0E); 
WriteData(0x1A); 
WriteData(0x02); 
WriteData(0x0E); 
WriteData(0x04); 
WriteData(0x2F); 
WriteData(0x34); 
WriteData(0x45); 
WriteData(0x03); 
WriteData(0x0E); 
WriteData(0x0D); 
WriteData(0x33); 
WriteData(0x36); 
WriteData(0x0F); 
 
WriteComm(0x11);    //Exit Sleep 
Delay(120); 
WriteComm(0x29);    //Display on 


}

