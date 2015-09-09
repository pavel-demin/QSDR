#include <gnuradio/io_signature.h>
#include "trxDev_null.h"

TrxDev_null_sptr make_trxdev_null(int sampleRate) {
	return gnuradio::get_initial_sptr(new TrxDev_null(sampleRate));
}

TrxDev_null::TrxDev_null(int sRate) : TrxDev("NULL") {
	sampleRate = sRate;

	throttle = gr::blocks::throttle::make(sizeof(gr_complex), sampleRate);
	nullSource = gr::blocks::null_source::make (sizeof(gr_complex));
	nullSink = gr::blocks::null_sink::make (sizeof(gr_complex));

	connect (nullSource, 0, throttle, 0);
	connect (throttle, 0, self(), 0);
	connect (self(), 0, nullSink, 0);
}

TrxDev_null::~TrxDev_null()
{

}

void TrxDev_null::setSampleRate(int f) {
	sampleRate = f;
	lock();
	disconnect (nullSource, 0, throttle, 0);
	disconnect (throttle, 0, self(), 0);
	throttle.reset();
	throttle = gr::blocks::throttle::make(sizeof(gr_complex), sampleRate);
	connect (nullSource, 0, throttle, 0);
	connect (throttle, 0, self(), 0);
	unlock();
}

void TrxDev_null::setCenterFreq(qint64 f) { }
void TrxDev_null::setPreamp(int n) { }
void TrxDev_null::setAntenne(int) { }
int TrxDev_null::getRXLowFreq() { return 0; }
int TrxDev_null::getRXHighFreq() { return 100; }
void TrxDev_null::setPTT(int) { }
void TrxDev_null::setTXPower(int) { }
int TrxDev_null::getTXLowFreq() { return 0; }
int TrxDev_null::getTXHighFreq() { return 0; }
void TrxDev_null::setTXFreq(qint64) { }
void TrxDev_null::setTXRate(int) { }
void TrxDev_null::setPresel(int) { }
void TrxDev_null::setCWMode(bool cw) { }
void TrxDev_null::setFreqCorr(int k) { }
void TrxDev_null::setDCFilter(bool) { }
void TrxDev_null::setPosition(int pos) {}
int TrxDev_null::getPosition() { return 0;}
