#ifndef _SCCB_H_
#define _SCCB_H_

void SCCB_INIT(void);
void startSCCB(void);
void stopSCCB(void);
void noAck(void);
unsigned char SCCBwriteByte(unsigned char m_data);
unsigned char SCCBreadByte(void);

#endif

