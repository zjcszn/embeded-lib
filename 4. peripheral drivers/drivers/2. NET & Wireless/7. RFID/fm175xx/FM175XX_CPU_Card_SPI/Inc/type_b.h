#ifndef TYPE_B_H
#define TYPE_B_H

extern unsigned char PICC_PUPI[4];

extern unsigned char TypeB_Init(void);			//TYPE B ≥ı ºªØ
extern unsigned char TypeB_Request(unsigned char *rece_len,unsigned char *buff,unsigned char *card_sn);
extern unsigned char TypeB_Select(unsigned char *card_sn,unsigned char *rece_len,unsigned char *buff);
extern unsigned char TypeB_GetUID(unsigned char *rece_len,unsigned char *buff);
extern unsigned char TypeB_Halt(unsigned char *card_sn);
extern unsigned char TypeB_WUP(unsigned char *rece_len,unsigned char *buff,unsigned char *card_sn);
#endif