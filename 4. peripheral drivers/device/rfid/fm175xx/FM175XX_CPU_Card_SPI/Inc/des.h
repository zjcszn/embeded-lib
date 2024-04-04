#ifndef DES_H
#define DES_H

#define ENCRY	0x0
#define DECRY	0x1

#define bit1    0x80
#define bit2    0x40
#define bit3    0x20
#define bit4    0x10
#define bit5    0x08
#define bit6    0x04
#define bit7    0x02
#define bit8    0x01

extern unsigned char Tdes(unsigned char Mode,unsigned char *MsgIn,unsigned char *Key, unsigned char *MsgOut);
extern unsigned char Des (unsigned char Mode,unsigned char *MsgIn,unsigned char *Key, unsigned char *MsgOut);
extern unsigned char TripleMAC(unsigned char *init_data,unsigned char *mac_key,unsigned char data_len,unsigned char *in_data,unsigned char *mac_data);
extern unsigned char MAC(unsigned char *init_data,unsigned char *mac_key,unsigned char data_len,unsigned char *in_data,unsigned char *mac_data);
#endif