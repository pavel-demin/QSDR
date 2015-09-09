#include <gnuradio/io_signature.h>
#include "trxDev_file.h"

TrxDev_file_sptr make_trxdev_file(int sampleRate, char *fileName) {
	return gnuradio::get_initial_sptr(new TrxDev_file(sampleRate, fileName));
}

TrxDev_file::TrxDev_file(int sRate, char* fileName) : TrxDev("FILE") {
	sampleRate = sRate;
	strncpy(fName, fileName, sizeof(fName)); 

	throttle = gr::blocks::throttle::make(sizeof(gr_complex), sampleRate);
	fileSource = gr_make_sdrfile_source (sizeof(gr_complex), fileName, true);
	nullSink = gr::blocks::null_sink::make (sizeof(gr_complex));

	connect (fileSource, 0, throttle, 0);
	connect (throttle, 0, self(), 0);
	connect (self(), 0, nullSink, 0);
}

TrxDev_file::~TrxDev_file()
{
}

void TrxDev_file::setSampleRate(int f) {
	sampleRate = f;
	lock();
	disconnect (fileSource, 0, throttle, 0);
	disconnect (throttle, 0, self(), 0);
	throttle.reset();
	throttle = gr::blocks::throttle::make(sizeof(gr_complex), sampleRate);
	connect (fileSource, 0, throttle, 0);
	connect (throttle, 0, self(), 0);
	unlock();
}

void TrxDev_file::setCenterFreq(qint64 f) { }
void TrxDev_file::setPreamp(int n) { }
void TrxDev_file::setAntenne(int) { }
int TrxDev_file::getRXLowFreq() { return 0; }
int TrxDev_file::getRXHighFreq() { return 100; }
void TrxDev_file::setPTT(int) { }
void TrxDev_file::setTXPower(int) { }
int TrxDev_file::getTXLowFreq() { return 0; }
int TrxDev_file::getTXHighFreq() { return 0; }
void TrxDev_file::setTXFreq(qint64) { } 
void TrxDev_file::setTXRate(int) { }
void TrxDev_file::setPresel(int) { }
void TrxDev_file::setCWMode(bool) { }
void TrxDev_file::setFreqCorr(int k) { }
void TrxDev_file::setDCFilter(bool) { } 

void TrxDev_file::setPosition(int pos) {
	lock();
	fileSource->seek(pos*sampleRate, SEEK_SET);
	unlock();
}
int TrxDev_file::getPosition() { 
	return fileSource->pos()/sampleRate;
}
