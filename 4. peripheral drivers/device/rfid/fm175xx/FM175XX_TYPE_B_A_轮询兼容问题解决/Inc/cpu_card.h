#ifndef	CPU_CARD_H
#define CPU_CARD_H
extern unsigned char CPU_PCB,CPU_CID;

extern void CPU_PCB_CONVER(void);
extern unsigned char CPU_BaudRate_Set(unsigned char mode);
extern unsigned char CPU_TPDU(unsigned char send_len,unsigned char *send_buff,unsigned char *rece_len,unsigned char *rece_buff);
extern unsigned char CPU_Rats(unsigned char fsdi,unsigned char cid,unsigned char *rece_len,unsigned char *buff);
extern unsigned char CPU_PPS(unsigned char cid,unsigned char mode,unsigned char *rece_len,unsigned char *buff);
//extern unsigned char CPU_Random(unsigned char *rece_len,unsigned char *buff);
extern unsigned char CPU_I_Block(unsigned char send_len,unsigned char *send_buff,unsigned char *rece_len,unsigned char *rece_buff);
extern unsigned char CPU_R_Block(unsigned char *rece_len,unsigned char *rece_buff);
extern unsigned char CPU_S_Block(unsigned char pcb,unsigned char wtxm,unsigned char *rece_len,unsigned char *rece_buff);
//extern unsigned char CPU_auth(unsigned char *key_buff,unsigned char *rece_len,unsigned char *buff);
#endif