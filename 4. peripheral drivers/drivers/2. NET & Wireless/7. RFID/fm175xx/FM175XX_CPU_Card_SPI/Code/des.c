//2012年4月23日修改，增加MAC运算函数

#include <string.h>
#include "des.h"
#include "main.h"
 
/*****************************************************************************************************/
//函 数 名：Tdes
//功    能：对一个8字节的字符串进行3DES加解密计算
//输    入：
//		    Mode=0:加密 ，Mode=1：解密
//			MsgIn ：8字节输入数组，明文（加密）或密文（解密）
//			Key :16字节密钥数组
//		  		
//输	  出：MsgOut：8字节输出数组，密文（加密）或明文（解密）
//返 回 值：OK
/**************************************************************************************/
unsigned char Tdes( unsigned char Mode,unsigned char *MsgIn, unsigned char *Key, unsigned char *MsgOut)
//Mode=0:加密 Mode=1：解密
{
unsigned char data des_key_temp[8];
unsigned char data Min[8];
unsigned char data Mout[8];

if(Mode ==ENCRY)//加密
{
	memcpy(des_key_temp,Key,8);
	memcpy(Min,MsgIn,8);
	
	Des(ENCRY,Min,des_key_temp,Mout);
	
	memcpy(des_key_temp,Key+8,8);
	memcpy(Min,Mout,8);
	
	Des(DECRY,Min,des_key_temp,Mout);
	
	memcpy(des_key_temp,Key,8);
	memcpy(Min,Mout,8);
	
	Des(ENCRY,Min,des_key_temp,Mout);
}
if(Mode==DECRY)
{
	memcpy(des_key_temp,Key,8);
	memcpy(Min,MsgIn,8);
	
	Des(DECRY,Min,des_key_temp,Mout);
	
	memcpy(	des_key_temp,Key+8,8);
	memcpy(	Min,Mout,8);
	
	Des(ENCRY,Min,des_key_temp,Mout);
	
	memcpy(des_key_temp,Key,8);
	memcpy(Min,Mout,8);
		
	Des(DECRY,Min,des_key_temp,Mout);
}
	memcpy(	MsgOut,Mout,8);
   	return OK;
}
/**************************************************************************************/
//TripleMAC 计算
//输入：
//init_data: 8字节数组，初始化数据
//mac_key: 16字节密钥数组
//data_len: 数据长度
//in_data: 输入数据数组，长度由data_len指定
//输出：
//mac_data: 8字节数组，MAC计算结果
//返 回 值：OK
/**************************************************************************************/
unsigned char TripleMAC(unsigned char *init_data,unsigned char *mac_key,unsigned char data_len,unsigned char *in_data,unsigned char *mac_data)

{
	unsigned char mac_data0[8],mac_data1[8];
    unsigned char data len;
    unsigned char data i,j;    
	unsigned char L[8],R[8];

	memcpy(	L,mac_key,8);
	memcpy(	R,mac_key+8,8);
	memcpy(	mac_data0,init_data,8);
                 
	len=data_len/8;
	for (i=0;i<	(8*(len+1)-data_len);i++)
		{
		if (i==0)
			in_data[data_len+i]=0x80;
		if (i>0)
			in_data[data_len+i]=0x00;
		}
	data_len=data_len+i;
				       
	len=data_len/8;
	for(j=0;j<len;j++)

    {
    	for(i=0;i<8;i++)      
			mac_data0[i] ^= in_data[i+j*8];
		Des(ENCRY,mac_data0,L,mac_data1);     
	    memcpy(mac_data0,mac_data1,8);
	}
					
	Des(DECRY,mac_data0,R,mac_data1);
    memcpy(mac_data0,mac_data1,8);
	Des(ENCRY,mac_data0,L,mac_data1);
	
	memcpy(	mac_data,mac_data1,8);
    return OK;    
}
/**************************************************************************************/
//MAC 计算
//输入：
//init_data: 8字节数组，初始化数据
//mac_key: 16字节密钥数组
//data_len: 数据长度
//in_data: 输入数组，长度由data_len指定
//输出：
//mac_data: 8字节数组，MAC计算结果
//返 回 值：OK
/**************************************************************************************/
unsigned char MAC(unsigned char *init_data,unsigned char *mac_key,unsigned char data_len,unsigned char *in_data,unsigned char *mac_data)
{
	unsigned char mac_data0[8],mac_data1[8];
    unsigned char data len;
    unsigned char data i,j;    

	memcpy(	mac_data0,init_data,8);
							                  
	len=data_len/8;
	for (i=0;i<	(8*(len+1)-data_len);i++)
		{
		if (i==0)
			in_data[data_len+i]=0x80;
		if (i>0)
			in_data[data_len+i]=0x00;
		}
	data_len=data_len+i;
				       
	len=data_len/8;
	for(j=0;j<len;j++)

    {
    	for(i=0;i<8;i++)      
			mac_data0[i] ^= in_data[i+j*8];
		Des(ENCRY,mac_data0,mac_key,mac_data1);     
	    memcpy(mac_data0,mac_data1,8);
	}
				
 	memcpy(mac_data,mac_data1,8);
    return OK;  
   
}
/***************************************************************************************/
//函 数 名：Des
//功    能：对一个8字节的字符串进行单DES加解密计算
//输    入：
//		    Mode=0:加密 ，Mode=1：解密
//			MsgIn ：输入8字节数组 ，明文（加密）或密文（解密）
//			Key：8字节密钥数组
//输	  出：MsgOut：输出8字节数组 ，密文（加密）或明文（解密）
//返 回 值：OK
/**************************************************************************************/
unsigned char Des( unsigned char Mode,unsigned char *MsgIn, unsigned char *Key, unsigned char *MsgOut)
{
	
	unsigned char data i;
	unsigned char data temp1,temp2,temp3,temp4;
	unsigned char data C0[4], D0[4];
	unsigned char data Ki[8];


const unsigned char SBox1[64]=
{
	14,  4, 13,  1,  2,  15, 11,  8,  3,  10,  6,  12,  5,  9,  0,  7, 
	 0, 15,  7,  4,  14, 2,  13,  1,  10, 6 ,  12, 11,  9,  5,  3,  8,	 
	4,  1,  14,  8,  13, 6,  2,  11, 15,  12, 9,   7,  3,  10, 5,  0,
	15, 12, 8,   2,  4,  9,  1,   7,  5,  11,  3,  14, 10, 0,  6,  13 
};
const unsigned char SBox2[64]=
{
	15,  1,   8,  14,  6, 11,   3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	3,  13,   4,   7, 15,  2,   8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	0,  14,   7,  11, 10,  4,  13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8,  10,   1,  3, 15,   4,  2, 11,  6,  7, 12,  0,  5, 14,  9
};
const unsigned char SBox3[64]=
{
	10,  0,   9,  14,  6,  3,  15,  5,  1, 13, 12,  7, 11,  4,  2,  8, 
	13,  7,   0,   9,  3,  4,   6, 10,  2,  8,  5, 14, 12, 11, 15,  1, 
	13,  6,   4 ,  9,  8, 15,   3,  0, 11,  1,  2, 12,  5, 10, 14,  7, 
	 1, 10,  13,   0,  6,  9,   8,  7,  4, 15, 14,  3, 11,  5,  2, 12
};
const unsigned char SBox4[64]=
{
	 7, 13,  14,   3,  0,  6,   9, 10,  1,  2,  8,  5, 11, 12,  4, 15, 
    13,  8,  11,   5,  6, 15,   0,  3,  4,  7,  2, 12,  1, 10, 14,  9, 
    10,  6,   9,   0, 12, 11,   7, 13, 15,  1,  3, 14,  5,  2,  8,  4, 
     3, 15,   0,   6, 10,  1,  13,  8,  9,  4,  5, 11, 12,  7,  2, 14 
};
const unsigned char SBox5[64]=
{
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9, 
    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6, 
     4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14, 
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3 
};
const unsigned char SBox6[64]=
{
	12,  1, 10, 15, 9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11, 
    10, 15,  4,  2, 7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8, 
     9, 14, 15,  5, 2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6, 
     4,  3,  2, 12, 9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13 
};
const unsigned char SBox7[64]=
{
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1, 
    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6, 
     1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2, 
     6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12 
};
const unsigned char SBox8[64]=
{
	13,  2,  8, 4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7, 
     1, 15, 13, 8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2, 
     7, 11,  4, 1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8, 
     2,  1, 14, 7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};
	
const unsigned char ShiftValue[16]={1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };					 	
		
	for(i=0; i<8; i++)
	{
		MsgOut[0]>>=1;
		MsgOut[1]>>=1;
		MsgOut[2]>>=1;
		MsgOut[3]>>=1;
		MsgOut[4]>>=1;
		MsgOut[5]>>=1;
		MsgOut[6]>>=1;
		MsgOut[7]>>=1;
		
		temp1=MsgIn[i];

		MsgOut[4] |= (temp1&0x80);
		temp1<<=1;
		MsgOut[0] |= (temp1&0x80);
		temp1<<=1;
		MsgOut[5] |= (temp1&0x80);
		temp1<<=1;
		MsgOut[1] |= (temp1&0x80);
		temp1<<=1;
		MsgOut[6] |= (temp1&0x80);
		temp1<<=1;
		MsgOut[2] |= (temp1&0x80);
		temp1<<=1;
		MsgOut[7] |= (temp1&0x80);
		temp1<<=1;
		MsgOut[3] |= (temp1&0x80);
	}
	//密钥处理
	//序列变换１（PC-1）
	//输出：C0, D0,各28bit
	for(i=0; i<8; i++)
	{
		C0[0]>>=1;
		C0[1]>>=1;
		C0[2]>>=1;
		C0[3]>>=1;
		D0[0]>>=1;
		D0[1]>>=1;
		D0[2]>>=1;

		temp1=Key[i];
		C0[0] |= (temp1&0x80);
		temp1<<=1;
		C0[1] |= (temp1&0x80);
		temp1<<=1;
		C0[2] |= (temp1&0x80);
		temp1<<=1;
		C0[3] |= (temp1&0x80);
		temp1<<=1;
		D0[2] |= (temp1&0x80);
		temp1<<=1;
		D0[1] |= (temp1&0x80);
		temp1<<=1;
		D0[0] |= (temp1&0x80);
			
	}
	
	D0[3]=(C0[3]&0x0f)|(D0[2]<<4);
	D0[2]=(D0[2]>>4)|(D0[1]<<4);
	D0[1]=(D0[1]>>4)|(D0[0]<<4);
	D0[0]=(D0[0]>>4)&0x0F;
	C0[3]&=0xf0;
	////////////////////////////////////////
	
	for(i=0; i<16; i++)
	{
		//计算子密钥,输出Ki
		if(Mode == ENCRY)
		{
		
			for(temp1=0; temp1<ShiftValue[i]; temp1++)
			{
				
				if((C0[0]&bit1)!=0) C0[3]|=bit5;
				C0[0]<<=1;
				if((C0[1]&bit1)!=0) C0[0]|=bit8;
				C0[1]<<=1;
				if((C0[2]&bit1)!=0) C0[1]|=bit8;
				C0[2]<<=1;
				if((C0[3]&bit1)!=0) C0[2]|=bit8;
				C0[3]<<=1;
				
			//	D0
				D0[0]<<=1;
				if((D0[1]&bit1)!=0) D0[0]|=bit8;
				D0[1]<<=1;
				if((D0[2]&bit1)!=0) D0[1]|=bit8;
				D0[2]<<=1;
				if((D0[3]&bit1)!=0) D0[2]|=bit8;
				D0[3]<<=1;
				if((D0[0]&bit4)!=0)
				{
					 D0[3]|=bit8;
					 D0[0]&=0x0f;//(~bit4);
				}
						
			}
		}
		//把C[i]D[i]串联起来，变成Ki,共有56bit
		
		Ki[0]=C0[0]; 
		Ki[1]=C0[1]; 
		Ki[2]=C0[2]; 
		Ki[3]=C0[3]|D0[0]; 
		Ki[4]=D0[1]; 
		Ki[5]=D0[2]; 
		Ki[6]=D0[3]; 
		
		//序列变换２(PC-2)，变换后存入Ki中，共有48bit
		temp1=0;
		temp2=0;
		temp3=0;
		
		if((Ki[0]&bit1)!=0) temp1|=bit5;  //1
		if((Ki[0]&bit2)!=0) temp3|=bit8;  //2
		if((Ki[0]&bit3)!=0) temp1|=bit7;  //3
		if((Ki[0]&bit4)!=0) temp2|=bit8;  //4
		if((Ki[0]&bit5)!=0) temp1|=bit6;  //5
		if((Ki[0]&bit6)!=0) temp2|=bit2;  //6
		if((Ki[0]&bit7)!=0) temp3|=bit4;  //7
		if((Ki[0]&bit8)!=0) temp3|=bit2;  //8
		                                  
//		if((Ki[1]&bit1)!=0) temp1|=bit5;  // 9
		if((Ki[1]&bit2)!=0) temp2|=bit4;  //10
		if((Ki[1]&bit3)!=0) temp1|=bit3;  //11
		if((Ki[1]&bit4)!=0) temp2|=bit7;  //12
		if((Ki[1]&bit5)!=0) temp3|=bit7;  //13
		if((Ki[1]&bit6)!=0) temp1|=bit1;  //14
		if((Ki[1]&bit7)!=0) temp2|=bit1;  //15
		if((Ki[1]&bit8)!=0) temp3|=bit3;  //16
	                                      //
		if((Ki[2]&bit1)!=0) temp1|=bit2;  //17
//		if((Ki[2]&bit2)!=0) temp3|=bit8;  //18
		if((Ki[2]&bit3)!=0) temp2|=bit6;  //19
		if((Ki[2]&bit4)!=0) temp3|=bit6;  //20
		if((Ki[2]&bit5)!=0) temp2|=bit3;  //21
//		if((Ki[2]&bit6)!=0) temp2|=bit2;  //22
		if((Ki[2]&bit7)!=0) temp2|=bit5;  //23
		if((Ki[2]&bit8)!=0) temp1|=bit4;  //24
		
//		if((Ki[3]&bit1)!=0) temp1|=bit5;  //25
		if((Ki[3]&bit2)!=0) temp3|=bit1;  //26
		if((Ki[3]&bit3)!=0) temp3|=bit5;  //27
		if((Ki[3]&bit4)!=0) temp1|=bit8;  //28
    	
		Ki[0]=temp1;
		Ki[1]=temp2;
		Ki[2]=temp3;
	
		temp1=0;
		temp2=0;
		temp3=0;
		
		if((Ki[3]&bit5)!=0) temp3|=bit7;
		if((Ki[3]&bit6)!=0) temp1|=bit7;
		if((Ki[3]&bit7)!=0) temp1|=bit3;
		if((Ki[3]&bit8)!=0) temp3|=bit8;
		
		if((Ki[4]&bit1)!=0) temp2|=bit3;
		if((Ki[4]&bit2)!=0) temp3|=bit1;
//		if((Ki[4]&bit3)!=0) temp1|=bit7;
		if((Ki[4]&bit4)!=0) temp3|=bit6;
		if((Ki[4]&bit5)!=0) temp1|=bit4;
//		if((Ki[4]&bit6)!=0) temp2|=bit2;
		if((Ki[4]&bit7)!=0) temp2|=bit7;
		if((Ki[4]&bit8)!=0) temp1|=bit8;
	
		if((Ki[5]&bit1)!=0) temp1|=bit1;
		if((Ki[5]&bit2)!=0) temp3|=bit4;
//		if((Ki[5]&bit3)!=0) temp1|=bit7;
		if((Ki[5]&bit4)!=0) temp2|=bit5;
		if((Ki[5]&bit5)!=0) temp2|=bit2;
		if((Ki[5]&bit6)!=0) temp3|=bit3;
		if((Ki[5]&bit7)!=0) temp1|=bit5;
		if((Ki[5]&bit8)!=0) temp2|=bit4;
		
		if((Ki[6]&bit1)!=0) temp2|=bit6;
		if((Ki[6]&bit2)!=0) temp3|=bit5;
		if((Ki[6]&bit3)!=0) temp2|=bit1;
		if((Ki[6]&bit4)!=0) temp1|=bit2;
		if((Ki[6]&bit5)!=0) temp3|=bit2;
//		if((Ki[6]&bit6)!=0) temp2|=bit2;
		if((Ki[6]&bit7)!=0) temp1|=bit6;
		if((Ki[6]&bit8)!=0) temp2|=bit8;
		
		Ki[3]=temp1;
		Ki[4]=temp2;
		Ki[5]=temp3;
		
		//此时已计算出Ki
		//把32位R扩展成48位,即把MsgOut[4]、MsgOut[５]、
		//MsgOut[６]、MsgOut[７]扩展,取得然后与Ki异或
		     
		//1
		temp1=((MsgOut[4]>>1)&0x7c);
		if((MsgOut[7]&bit8)!=0) temp1|=bit1;
		if((MsgOut[4]&bit4)!=0) temp1|=bit7;
		if((MsgOut[4]&bit5)!=0) temp1|=bit8;
			
		//扩展后R与Ki异或
	 	Ki[0]^=temp1;
	 	
	 	//2
	 	temp1=((MsgOut[4]<<5)&0xE0);
		if((MsgOut[5]&bit1)!=0) temp1|=bit4;
		if((MsgOut[4]&bit8)!=0) temp1|=bit5;
		temp1|=((MsgOut[5]>>5)&0x07);	
	 	
	 	//扩展后R与Ki异或
	 	Ki[1]^=temp1;
	 	
	 	//3
	 	temp1=((MsgOut[5]<<3)&0xc0)|((MsgOut[5]<<1)&0x3e);	
		if((MsgOut[6]&bit1)!=0) temp1|=bit8;
	 	Ki[2]^=temp1;
	 	
	 	//4
	 	temp1=((MsgOut[6]>>1)&0x7c);
		if((MsgOut[5]&bit8)!=0) temp1|=bit1;
		temp1|=((MsgOut[6]>>3)&0x03);	
		Ki[3]^=temp1;
	 	
	 	//5
	 	temp1=((MsgOut[6]<<5)&0xE0);
		if((MsgOut[7]&bit1)!=0) temp1|=bit4;
		if((MsgOut[6]&bit8)!=0) temp1|=bit5;
		temp1|=((MsgOut[7]>>5)&0x07);	
	 	
	 	Ki[4]^=temp1;
	 	
	 	//6
	 	temp1=((MsgOut[7]<<1)&0x3E);
		if((MsgOut[7]&bit4)!=0) temp1|=(bit1);
		if((MsgOut[7]&bit5)!=0) temp1|=(bit2);
		if((MsgOut[4]&bit1)!=0) temp1|=bit8;
		//扩展后R与Ki异或
		Ki[5]^=temp1;
			 	
//	 	for(i=0; i<6; i++)
//			Ki[i]^=tempR[i];
		//准备S盒的行列坐标
/*		Ki[7]= Ki[5]&0x3F;
		Ki[6]= ((Ki[5]>>6))|((Ki[4]<<2)&0x3C);
		Ki[5]= (Ki[4]>>4)|((Ki[3]<<4)&0x30);
		Ki[4]= (Ki[3]>>2);
		Ki[3]= Ki[2]&0x3F;
		Ki[2]= ((Ki[2]>>6))|((Ki[1]<<2)&0x3C);
		Ki[1]= (Ki[1]>>4)|((Ki[0]<<4)&0x30);
		Ki[0]= (Ki[0]>>2);*/
		
		//选择SBox的值来替代，替代后的值存入Ki
		
/*		i=((Ki[0]&0x20)>>4)|(Ki[0]&0x01);
		j=(Ki[0]&0x1E)>>1;
		Ki[0]=SBox1[i][j];
		
		i=((Ki[1]&0x20)>>4)|(Ki[1]&0x01);
		j=(Ki[1]&0x1E)>>1;
		Ki[1]=SBox2[i][j];
		
		i=((Ki[2]&0x20)>>4)|(Ki[2]&0x01);
		j=(Ki[2]&0x1E)>>1;
		Ki[2]=SBox3[i][j];
		
		i=((Ki[3]&0x20)>>4)|(Ki[3]&0x01);
		j=(Ki[3]&0x1E)>>1;
		Ki[3]=SBox4[i][j];
				
		i=((Ki[4]&0x20)>>4)|(Ki[4]&0x01);
		j=(Ki[4]&0x1E)>>1;
		Ki[4]=SBox5[i][j];
		
		i=((Ki[5]&0x20)>>4)|(Ki[5]&0x01);
		j=(Ki[5]&0x1E)>>1;
		Ki[5]=SBox6[i][j];
	
		i=((Ki[6]&0x20)>>4)|(Ki[6]&0x01);
		j=(Ki[6]&0x1E)>>1;
		Ki[6]=SBox7[i][j];
		
		i=((Ki[7]&0x20)>>4)|(Ki[7]&0x01);
		j=(Ki[7]&0x1E)>>1;
		Ki[7]=SBox8[i][j];*/
		
		temp1=0;
		
		if((Ki[5]&bit8)!=0) temp1|=bit4;
		if((Ki[5]&bit7)!=0) temp1|=bit8;
		if((Ki[5]&bit6)!=0) temp1|=bit7;
		if((Ki[5]&bit5)!=0) temp1|=bit6;
		if((Ki[5]&bit4)!=0) temp1|=bit5;
		if((Ki[5]&bit3)!=0) temp1|=bit3;
		Ki[7]=SBox8[temp1]; 

		temp1=0;
		if((Ki[5]&bit2)!=0) temp1|=bit4;
		if((Ki[5]&bit1)!=0) temp1|=bit8;
		if((Ki[4]&bit8)!=0) temp1|=bit7;
		if((Ki[4]&bit7)!=0) temp1|=bit6;
		if((Ki[4]&bit6)!=0) temp1|=bit5;
		if((Ki[4]&bit5)!=0) temp1|=bit3;
		Ki[6]=SBox7[temp1];
 		
		temp1=0;
		if((Ki[4]&bit4)!=0) temp1|=bit4;
		if((Ki[4]&bit3)!=0) temp1|=bit8;
		if((Ki[4]&bit2)!=0) temp1|=bit7;
		if((Ki[4]&bit1)!=0) temp1|=bit6;
		if((Ki[3]&bit8)!=0) temp1|=bit5;
		if((Ki[3]&bit7)!=0) temp1|=bit3;
		Ki[5]=SBox6[temp1];
 		
		temp1=0;
		if((Ki[3]&bit6)!=0) temp1|=bit4;
		if((Ki[3]&bit5)!=0) temp1|=bit8;
		if((Ki[3]&bit4)!=0) temp1|=bit7;
		if((Ki[3]&bit3)!=0) temp1|=bit6;
		if((Ki[3]&bit2)!=0) temp1|=bit5;
		if((Ki[3]&bit1)!=0) temp1|=bit3;
		Ki[4]=SBox5[temp1];
		
		temp1=0;
		if((Ki[2]&bit8)!=0) temp1|=bit4;
		if((Ki[2]&bit7)!=0) temp1|=bit8;
		if((Ki[2]&bit6)!=0) temp1|=bit7;
		if((Ki[2]&bit5)!=0) temp1|=bit6;
		if((Ki[2]&bit4)!=0) temp1|=bit5;
		if((Ki[2]&bit3)!=0) temp1|=bit3;
		Ki[3]=SBox4[temp1];
		
		temp1=0;
		if((Ki[2]&bit2)!=0) temp1|=bit4;
		if((Ki[2]&bit1)!=0) temp1|=bit8;
		if((Ki[1]&bit8)!=0) temp1|=bit7;
		if((Ki[1]&bit7)!=0) temp1|=bit6;
		if((Ki[1]&bit6)!=0) temp1|=bit5;
		if((Ki[1]&bit5)!=0) temp1|=bit3;
		Ki[2]=SBox3[temp1];
		
		temp1=0;
		if((Ki[1]&bit4)!=0) temp1|=bit4;
		if((Ki[1]&bit3)!=0) temp1|=bit8;
		if((Ki[1]&bit2)!=0) temp1|=bit7;
		if((Ki[1]&bit1)!=0) temp1|=bit6;
		if((Ki[0]&bit8)!=0) temp1|=bit5;
		if((Ki[0]&bit7)!=0) temp1|=bit3;
		Ki[1]=SBox2[temp1];
		
		temp1=0;
		if((Ki[0]&bit6)!=0) temp1|=bit4;
		if((Ki[0]&bit5)!=0) temp1|=bit8;
		if((Ki[0]&bit4)!=0) temp1|=bit7;
		if((Ki[0]&bit3)!=0) temp1|=bit6;
		if((Ki[0]&bit2)!=0) temp1|=bit5;
		if((Ki[0]&bit1)!=0) temp1|=bit3;
		Ki[0]=SBox1[temp1];
		
		//把上面的结果进行P置换
		temp1=0;
		temp2=0;
		temp3=0;
		temp4=0;
		
		if(Ki[0]&bit5)        //1
			temp2|=bit1;      //
		if(Ki[0]&bit6)        //2
			temp3|=bit1;      //
		if(Ki[0]&bit7)        //3
			temp3|=bit7;      //
		if(Ki[0]&bit8)        //4
			temp4|=bit7;      //
			                  //
		if(Ki[1]&bit5)        //5
			temp2|=bit5;      //
		if(Ki[1]&bit6)        //6
			temp4|=bit4;      //
		if(Ki[1]&bit7)        //7
			temp1|=bit2;      //
		if(Ki[1]&bit8)        //8
			temp3|=bit2;      //
			                  //
		if(Ki[2]&bit5)        //9
			temp3|=bit8;      //
		if(Ki[2]&bit6)        //10
			temp2|=bit8;      //
		if(Ki[2]&bit7)        //11
			temp4|=bit6;      //
		if(Ki[2]&bit8)        //12
			temp1|=bit6;      //
			                  //
		if(Ki[3]&bit5)        //13
			temp4|=bit2;      //
		if(Ki[3]&bit6)        //14
			temp3|=bit4;      //
		if(Ki[3]&bit7)        //15
			temp2|=bit2;      //
		if(Ki[3]&bit8)        //16
			temp1|=bit1;      //
		                      //
		if(Ki[4]&bit5)        //17
			temp1|=bit8;      //
		if(Ki[4]&bit6)        //18
			temp2|=bit6;      //
		if(Ki[4]&bit7)        //19
			temp4|=bit1;      //
		if(Ki[4]&bit8)        //20
			temp1|=bit3;      //
			                  //
		if(Ki[5]&bit5)        //21
			temp1|=bit4;      //
		if(Ki[5]&bit6)        //22
			temp4|=bit5;      //
		if(Ki[5]&bit7)        //23
			temp2|=bit3;      //
		if(Ki[5]&bit8)        //24
		temp3|=bit3;          //
		                      //
		if(Ki[6]&bit5)        //25
			temp4|=bit8;      //
		if(Ki[6]&bit6)        //26
			temp2|=bit4;      //
		if(Ki[6]&bit7)        //27
			temp3|=bit6;      //
		if(Ki[6]&bit8)        //28
			temp1|=bit7;      //
			                  //
		if(Ki[7]&bit5)        //29
			temp1|=bit5;      //
		if(Ki[7]&bit6)        //30
			temp4|=bit3;      //
		if(Ki[7]&bit7)        //31
			temp2|=bit7;      //
		if(Ki[7]&bit8)        //32
			temp3|=bit5;      //
		                      //
		//保存R               //
		Ki[0]=MsgOut[4];      //
		Ki[1]=MsgOut[5];      //
		Ki[2]=MsgOut[6];      //
		Ki[3]=MsgOut[7];      //
			
		//计算新的R	
		MsgOut[4]=temp1^MsgOut[0];	
		MsgOut[5]=temp2^MsgOut[1];	
		MsgOut[6]=temp3^MsgOut[2];	
		MsgOut[7]=temp4^MsgOut[3];
		//用上一次的R作为这一次的L
		
		MsgOut[0]=Ki[0];
		MsgOut[1]=Ki[1];
		MsgOut[2]=Ki[2];
		MsgOut[3]=Ki[3];
		
		if(Mode != ENCRY)
		{
		 
		 	for(temp1=0; temp1<ShiftValue[15-i]; temp1++)
			{

				C0[3]>>=1;
				if((C0[2]&bit8)!=0) C0[3]|=bit1;
				C0[2]>>=1;
				if((C0[1]&bit8)!=0) C0[2]|=bit1;
				C0[1]>>=1;
				if((C0[0]&bit8)!=0) C0[1]|=bit1;
				C0[0]>>=1;
				if((C0[3]&bit5)!=0)
				{
					C0[3]&=0xF0;//(~bit5);
					C0[0]|=bit1;
					
				}
				
				if((D0[3]&bit8)!=0) D0[0]|=bit4;
				D0[3]>>=1;
				if((D0[2]&bit8)!=0) D0[3]|=bit1;
				D0[2]>>=1;
				if((D0[1]&bit8)!=0) D0[2]|=bit1;
				D0[1]>>=1;
				if((D0[0]&bit8)!=0) D0[1]|=bit1;
				D0[0]>>=1;
					
			}
	
		}
				
	}	
	//对R[16]L[16]进行置换，Final Permutation (IP**-1)
	Ki[0]=MsgOut[0];
	Ki[1]=MsgOut[4];
	Ki[2]=MsgOut[1];
	Ki[3]=MsgOut[5];
		
	Ki[4]=MsgOut[2];
	Ki[5]=MsgOut[6];
	Ki[6]=MsgOut[3];
	Ki[7]=MsgOut[7];
		
	for(i=0; i<8; i++)
	{
		MsgOut[0]<<=1;
		MsgOut[1]<<=1;
		MsgOut[2]<<=1;
		MsgOut[3]<<=1;
		MsgOut[4]<<=1;
		MsgOut[5]<<=1;
		MsgOut[6]<<=1;
		MsgOut[7]<<=1;
		
		MsgOut[0] |= (Ki[i]&0x01);
		Ki[i]>>=1;
		MsgOut[1] |= (Ki[i]&0x01);
		Ki[i]>>=1;
		MsgOut[2] |= (Ki[i]&0x01);
		Ki[i]>>=1;
		MsgOut[3] |= (Ki[i]&0x01);
		Ki[i]>>=1;
		MsgOut[4] |= (Ki[i]&0x01);
		Ki[i]>>=1;
		MsgOut[5] |= (Ki[i]&0x01);
		Ki[i]>>=1;
		MsgOut[6] |= (Ki[i]&0x01);
		Ki[i]>>=1;
		MsgOut[7] |= (Ki[i]&0x01);
	}
	 return OK;
	/*
	40 8 48 16 56 24 64 32 
	39 7 47 15 55 23 63 31 
	38 6 46 14 54 22 62 30 
	37 5 45 13 53 21 61 29 
	36 4 44 12 52 20 60 28 
	35 3 43 11 51 19 59 27 
	34 2 42 10 50 18 58 26 
	33 1 41 9  49 17 57 25 
	*/
}
 

