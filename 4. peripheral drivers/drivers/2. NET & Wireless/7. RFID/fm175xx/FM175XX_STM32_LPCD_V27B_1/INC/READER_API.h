#ifndef _READERAPI_H_
#define _READERAPI_H_


static const unsigned char RF_CMD_REQA = 0x26;
static const unsigned char RF_CMD_WUPA	= 0x52;
static const unsigned char RF_CMD_ANTICOL[3] = {0x93,0x95,0x97} ;
static const unsigned char RF_CMD_SELECT[3] = {0x93,0x95,0x97} ;

//发射参数设置
#define MODWIDTH 0x26 //默认设置为 0x26 ，0x26 = 106K，0x13 = 212K， 0x09 = 424K ，0x04 = 848K

//接收参数配置
#define RXGAIN	4		//设置范围0~7
#define GSNON	15			//设置范围0~15
#define MODGSNON 8 	//设置范围0~15
#define GSP 31			//设置范围0~63
#define MODGSP 31		//设置范围0~63
#define COLLLEVEL 4	//设置范围0~7
#define MINLEVEL  8	//设置范围0~15
#define RXWAIT 4		//设置范围0~63
#define UARTSEL 2		//默认设置为2  设置范围0~3 0:固定低电平 1:TIN包络信号 2:内部接收信号 3:TIN调制信号

struct type_b_struct
{
unsigned char ATQB[12];
unsigned char	PUPI[4];
unsigned char	APPLICATION_DATA[4];
unsigned char	PROTOCOL_INF[3];
unsigned char ATTRIB[10];
unsigned char UID[8];
unsigned char LEN_ATTRIB;	
};

extern struct type_b_struct Type_B; 

struct type_a_struct
{
unsigned char ATQA[2];
unsigned char CASCADE_LEVEL;
unsigned char UID[15];
unsigned char BCC[3];
unsigned char SAK[3];
};

extern struct type_a_struct Type_A; 

#define FM175XX_SUCCESS				0x00
#define FM175XX_RESET_ERR			0xF1
#define FM175XX_PARAM_ERR 		0xF2	//输入参数错误
#define FM175XX_TIMER_ERR			0xF3	//接收超时
#define FM175XX_COMM_ERR			0xF4	//
#define FM175XX_COLL_ERR			0xF5	//冲突错误
#define FM175XX_FIFO_ERR			0xF6	//FIFO
#define FM175XX_CRC_ERR				0xF7
#define FM175XX_PARITY_ERR		0xF8
#define FM175XX_PROTOCOL_ERR	0xF9

#define FM175XX_AUTH_ERR	0xE1


#define FM175XX_RATS_ERR 	0xD1
#define FM175XX_PPS_ERR 	0xD2
#define FM175XX_PCB_ERR 	0xD3

typedef struct
{
	unsigned char Cmd;                 	// 命令代码
	unsigned char nBitsToSend;					//准备发送的位数	
	unsigned char nBytesToSend;        	//准备发送的字节数
	unsigned char nBytesToReceive;			//准备接收的字节数	
	unsigned char nBytesReceived;      	// 已接收的字节数
	unsigned char nBitsReceived;       	// 已接收的位数
	unsigned char *pSendBuf;						//发送数据缓冲区
	unsigned char *pReceiveBuf;					//接收数据缓冲区
	unsigned char CollPos;             	// 碰撞位置
	unsigned char Error;								// 错误状态
	unsigned char Timeout;							//超时时间
} command_struct;

extern unsigned char SetTimeOut(unsigned int microseconds);
extern unsigned char Command_Execute(command_struct * comm_status);
extern void FM175XX_Initial_ReaderA(void);
extern void FM175XX_Initial_ReaderB(void);
extern unsigned char ReaderA_Halt(void);
extern unsigned char ReaderA_Request(void);
extern unsigned char ReaderA_Wakeup(void);

extern unsigned char ReaderA_AntiColl(unsigned char size);
extern unsigned char ReaderA_Select(unsigned char size);
extern unsigned char ReaderA_CardActivate(void);
extern void SetRf(unsigned char mode);
extern void SetTxCRC(unsigned char mode);
extern void SetRxCRC(unsigned char mode);
extern unsigned char ReaderB_Request(void);
unsigned char ReaderB_Wakeup(void);
extern unsigned char ReaderB_Attrib(void);
extern unsigned char ReaderB_GetUID(void);
extern unsigned char FM175XX_Polling(unsigned char *polling_card);
#endif

