#include <gnuradio/io_signature.h>
#include "trxDev_testsignal.h"
#include "stdio.h"


#define SIG1_FREQ	(sampleRate/4)
#define SIG2_FREQ	(sampleRate/3)
#define SIG1_AMPL	(1e-4)
#define SIG2_AMPL	(2e-4)
#define NOICE_AMPL	(1e-5)

TrxDev_testsignal_sptr make_trxdev_testsignal(int sampleRate) {
	return gnuradio::get_initial_sptr(new TrxDev_testsignal(sampleRate));
}

TrxDev_testsignal::TrxDev_testsignal(int sRate) : TrxDev("NULL") {
	sampleRate = sRate;

	twoToneTest0 = gr::analog::sig_source_c::make(sampleRate, gr::analog::GR_SIN_WAVE, SIG1_FREQ, SIG1_AMPL);
	twoToneTest1 = gr::analog::sig_source_c::make(sampleRate, gr::analog::GR_SIN_WAVE, SIG2_FREQ, SIG2_AMPL);
	throttle = gr::blocks::throttle::make(sizeof(gr_complex), sampleRate);
	adder = gr::blocks::add_cc::make (1);
	nullSink = gr::blocks::null_sink::make (sizeof(gr_complex));
	noiceSource = gr::analog::noise_source_c::make (gr::analog::GR_GAUSSIAN, NOICE_AMPL);

	connect (twoToneTest0, 0, adder, 0);
	connect (twoToneTest1, 0, adder, 1);
	connect (noiceSource, 0, adder, 2);
	connect (adder, 0, throttle, 0);
	connect (throttle, 0, self(), 0);
	connect (self(), 0, nullSink, 0);
}

TrxDev_testsignal::~TrxDev_testsignal()
{
}

void TrxDev_testsignal::setSampleRate(int f) {
	sampleRate = f;
	lock();
	disconnect(twoToneTest0, 0, adder, 0);
	disconnect(twoToneTest1, 0, adder, 1);
	disconnect (noiceSource, 0, adder, 2);
	disconnect (adder, 0, throttle, 0);
	disconnect (throttle, 0, self(), 0);
	throttle.reset();
	throttle = gr::blocks::throttle::make(sizeof(gr_complex), sampleRate);
	twoToneTest0.reset();
	twoToneTest1.reset();
	twoToneTest0 = gr::analog::sig_source_c::make(sampleRate, gr::analog::GR_SIN_WAVE, SIG1_FREQ, SIG1_AMPL);
	twoToneTest1 = gr::analog::sig_source_c::make(sampleRate, gr::analog::GR_SIN_WAVE, SIG2_FREQ, SIG2_AMPL);
	connect(twoToneTest0, 0, adder, 0);
	connect(twoToneTest1, 0, adder, 1);
	connect (noiceSource, 0, adder, 2);
	connect (adder, 0, throttle, 0);
	connect (throttle, 0, self(), 0);
	unlock();
}

void TrxDev_testsignal::setCenterFreq(qint64 f) { }
void TrxDev_testsignal::setPreamp(int n) { }
void TrxDev_testsignal::setAntenne(int) { }
int TrxDev_testsignal::getRXLowFreq() { return 0; }
int TrxDev_testsignal::getRXHighFreq() { return 100; }
void TrxDev_testsignal::setPTT(int) { }
void TrxDev_testsignal::setTXPower(int) { } 
int TrxDev_testsignal::getTXLowFreq() { return 0; }
int TrxDev_testsignal::getTXHighFreq() { return 0; }
void TrxDev_testsignal::setTXFreq(qint64) { }
void TrxDev_testsignal::setTXRate(int) { }
void TrxDev_testsignal::setPresel(int) { }
void TrxDev_testsignal::setCWMode(bool cw) { }
void TrxDev_testsignal::setFreqCorr(int) { }
void TrxDev_testsignal::setDCFilter(bool) { }
void TrxDev_testsignal::setPosition(int pos) {}
int TrxDev_testsignal::getPosition() { return 0;}
