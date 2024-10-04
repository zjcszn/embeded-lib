#ifndef TYPE_A_H
#define TYPE_A_H

extern unsigned char PICC_ATQA[2],PICC_UID[15],PICC_SAK[3];

extern unsigned char TypeA_Request(unsigned char *pTagType);
extern unsigned char TypeA_WakeUp(unsigned char *pTagType);
extern unsigned char TypeA_Anticollision(unsigned char selcode,unsigned char *pSnr);
extern unsigned char TypeA_Select(unsigned char selcode,unsigned char *pSnr,unsigned char *pSak);
extern unsigned char TypeA_Halt(void);
extern unsigned char TypeA_CardActivate(unsigned char *pTagType,unsigned char *pSnr,unsigned char *pSak);
#endif