#ifndef _LIB_H_
#define _LIB_H_

#define ERR1 1
#define	MSG1 2
#define	MSG2 3
#define	MSG3 4

#define CHECKRANGE(v,from,to) { if (v<from) v=from; if (v>to) v=to; }

#include <QStringList>

void PDEBUG  (int Level, const char *Format, ... );
void PDEBUGBUF (int Level, const char *comment, unsigned char *buf, unsigned int len);
int ParityBit(unsigned int in);
extern int debugLevel; 
QStringList getAlsaAudioDevices();
int min(int,int);
unsigned char min(unsigned char a, unsigned char b);
unsigned char max(unsigned char a, unsigned char b);


#endif
