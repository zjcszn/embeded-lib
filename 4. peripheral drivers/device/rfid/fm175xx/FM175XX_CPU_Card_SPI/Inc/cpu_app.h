#ifndef CPU_APP_H
#define CPU_APP_H

extern unsigned char CPU_Send_Buff[255],CPU_Rece_Buff[255],CPU_Send_Len,CPU_Rece_Len;

extern unsigned char CPU_APP(void);
extern unsigned char APDU_Exchange(unsigned char send_len,unsigned char *send_buff,unsigned char *rece_len,unsigned char *rece_buff);

#endif