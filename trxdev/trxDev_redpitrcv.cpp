#include <gnuradio/io_signature.h>
#include <redpitaya/rcv_source.h>
#include "trxDev_redpitrcv.h"

TrxDev_redpitrcv_sptr make_trxdev_redpitrcv(int sampleRate, char *ip, int port) {
	printf("NEW make_redpitrcv %d %s %d\n",sampleRate,ip,port);
	return gnuradio::get_initial_sptr(new TrxDev_redpitrcv(sampleRate, ip, port));
}

TrxDev_redpitrcv::TrxDev_redpitrcv(int sRate, char *ip, int port) : TrxDev("REDPITRCV") {
	sampleRate = sRate;

	printf("NEW trxDev_redpitrcv %d %s %d\n",sampleRate,ip,port);
	redpitrcvSource = gr::redpitrcv::source::make (sizeof(gr_complex), ip, port, sampleRate);
	redpitrcvSink = gr::blocks::null_sink::make (sizeof(gr_complex));

	connect (redpitrcvSource, 0, self(), 0);
	connect (self(), 0, redpitrcvSink, 0);
}

TrxDev_redpitrcv::~TrxDev_redpitrcv()
{
}

void TrxDev_redpitrcv::setSampleRate(int f) {
	sampleRate = f;
	printf("trxDev_redpitrcv samplerate %d\n",sampleRate);
	redpitrcvSource->setSampleRate(f);
}

void TrxDev_redpitrcv::setCenterFreq(qint64 f) {
	printf("trxDev_redpitrcv freq %d\n",(int)f);
	redpitrcvSource->setRXFreq((int) f);
}

void TrxDev_redpitrcv::setPreamp(int n) { }
void TrxDev_redpitrcv::setAntenne(int) { }
int TrxDev_redpitrcv::getRXLowFreq() { return 0; }
int TrxDev_redpitrcv::getRXHighFreq() { return 100; }
void TrxDev_redpitrcv::setPTT(int) { }
void TrxDev_redpitrcv::setTXPower(int) { }
int TrxDev_redpitrcv::getTXLowFreq() { return 0; }
int TrxDev_redpitrcv::getTXHighFreq() { return 0; }
void TrxDev_redpitrcv::setTXFreq(qint64 f) {printf("setTXRate %d\n",(int)f); }
void TrxDev_redpitrcv::setTXRate(int rate) {printf("setTXRate %d\n",rate); }
void TrxDev_redpitrcv::setPresel(int) { }
void TrxDev_redpitrcv::setCWMode(bool cw) { }
void TrxDev_redpitrcv::setFreqCorr(int k) { }
void TrxDev_redpitrcv::setDCFilter(bool) { }
void TrxDev_redpitrcv::setPosition(int pos) {}
int TrxDev_redpitrcv::getPosition() { return 0;}

TrxDev_redpittrx_sptr make_trxdev_redpittrx(int sampleRate, char *ip, int port) {
	printf("NEW make_redpittrx %d %s %d %d\n",sampleRate,ip,port);
	return gnuradio::get_initial_sptr(new TrxDev_redpittrx(sampleRate, ip, port));
}

TrxDev_redpittrx::TrxDev_redpittrx(int sRate, char *ip, int port) : TrxDev("REDPITTRX") {
	sampleRate = sRate;

	printf("NEW trxDev_redpittrx %d %s %d %d\n",sampleRate,ip,port);
	redpittrxSource = gr::redpittrx::source::make (sizeof(gr_complex), ip, port, sampleRate);
	redpittrxSink = gr::redpittrx::sink::make (sizeof(gr_complex), ip, port, 50000);
	multi = gr::blocks::multiply_const_cc::make(5.0);

	connect (redpittrxSource, 0, self(), 0);
	connect (self(), 0, multi, 0);
	connect (multi, 0, redpittrxSink, 0);
}

TrxDev_redpittrx::~TrxDev_redpittrx()
{
}

void TrxDev_redpittrx::setSampleRate(int f) {
	sampleRate = f;
	printf("trxDev_redpittrx samplerate %d\n",sampleRate);
	redpittrxSource->setRXRate(sampleRate);
}

void TrxDev_redpittrx::setCenterFreq(qint64 f) {
	printf("trxDev_redpittrx freq %d\n",(int)f);
	redpittrxSource->setRXFreq((int) f);
	redpittrxSink->setTXFreq((int) f);
}

void TrxDev_redpittrx::setPTT(int on) {
	redpittrxSink->setPtt((bool) on);
}

void TrxDev_redpittrx::setPreamp(int n) { }
void TrxDev_redpittrx::setAntenne(int) { }
int TrxDev_redpittrx::getRXLowFreq() { return 0; }
int TrxDev_redpittrx::getRXHighFreq() { return 100; }
void TrxDev_redpittrx::setTXPower(int) { }
int TrxDev_redpittrx::getTXLowFreq() { return 0; }
int TrxDev_redpittrx::getTXHighFreq() { return 0; }
void TrxDev_redpittrx::setTXFreq(qint64) { }
void TrxDev_redpittrx::setTXRate(int) { }
void TrxDev_redpittrx::setPresel(int) { }
void TrxDev_redpittrx::setCWMode(bool cw) { }
void TrxDev_redpittrx::setFreqCorr(int k) { }
void TrxDev_redpittrx::setDCFilter(bool) { }
void TrxDev_redpittrx::setPosition(int pos) {}
int TrxDev_redpittrx::getPosition() { return 0;}
