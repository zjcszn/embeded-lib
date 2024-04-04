#ifndef ULTRALIGHT_H
#define ULTRALIGHT_H

//extern unsigned char ultra_request(unsigned char mode,unsigned char *atqa);
//extern unsigned char ultra_anti(unsigned char *card_uid,unsigned char *card_sak);
extern unsigned char Ultra_Read(unsigned char block_address,unsigned char *buff);
extern unsigned char Ultra_Write(unsigned char block_address,unsigned char *buff);
//extern unsigned char ultra_halt(void);
#endif
