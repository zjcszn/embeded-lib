#ifndef gb2312_unicode_h
#define gb2312_unicode_h

extern unsigned int StrA2W(const char* mbs, unsigned int lenA, unsigned short* wcs, unsigned int lenW);
extern unsigned int StrW2A(const unsigned short* wcs, unsigned int lenW, char* mbs, unsigned int lenA);
extern int ToUprA(int ch);
extern int ToUprW(unsigned short ch);

#endif /* gb2312_unicode_h */