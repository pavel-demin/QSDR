#include <QDebug>
#include "config.h"
#include "lib.h"
#include <alsa/asoundlib.h>

int debugLevel = 0;

void PDEBUG (int Level, const char *Format, ... ) {
//#ifndef QT_NO_DEBUG
	va_list ap;
	char s[64*1024];
	va_start(ap, Format);
	if (debugLevel >= Level) {
		vsprintf(s,Format,ap);
		qDebug() << s;
	}
	va_end(ap);
//#endif
}

#ifndef QT_NO_DEBUG
static void bin2hex(const unsigned char *from, int len, char *to) {
	int i;
	for (i=0;i<len;i++)
		sprintf (to+2*i, "%02x", from[i]);
}
#endif


void PDEBUGBUF (int Level, const char *comment, unsigned char *buf, unsigned int len) {
#ifndef QT_NO_DEBUG
#define LINELEN 128 
	char s[LINELEN+1];
	int l=LINELEN/2;
	unsigned int pos=0;
	if (debugLevel >= Level) {
		qDebug(comment);
		while(pos<len) {
			if (pos+l>len)
				l=len-pos;
			bin2hex(buf+pos, l, s);
			s[l*2]=0;
			qDebug() << s;
			pos+=l;
		}
	}
#endif
}

int ParityBit(unsigned int in)
{
	qDebug() << "in:" << in;
	int bits=0;
	while(in) {
		if (in & 1)
			bits++;
		in/=2;
	}
	qDebug() << bits;
	return bits & 1;
}

QStringList getAlsaAudioDevices() {
	QStringList list;
	char **hints;
	
	int err = snd_device_name_hint(-1, "pcm", (void***)&hints);
	if (err != 0)
		return list;

	char** n = hints;
	while (*n != NULL) {
		char *name = snd_device_name_get_hint(*n, "NAME");

		if (name != NULL && 0 != strcmp("null", name)) {
			list +=  QString(name);
			free(name);
		}
		n++;
	}
	snd_device_name_free_hint((void**)hints);
	return list;
}

int min(int a, int b) {
	if (a>b)
		return b;
	return a;
}

unsigned char min(unsigned char a, unsigned char b) {
	if (a>b)
		return b;
	return a;
}

unsigned char max(unsigned char a, unsigned char b) {
	if (a<b)
		return b;
	return a;
}
