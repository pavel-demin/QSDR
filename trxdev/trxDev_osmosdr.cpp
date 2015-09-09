#include <gnuradio/io_signature.h>
#include "trxDev_osmosdr.h"

TrxDev_osmosdr_sptr make_trxdev_osmosdr(int sampleRate, int freqCorr) {
	return gnuradio::get_initial_sptr(new TrxDev_osmosdr(sampleRate, freqCorr));
}

TrxDev_osmosdr::TrxDev_osmosdr(int sRate, int freqCorr) : TrxDev("OSMOSDR") {
	qDebug() << "IN OSMO";
	sampleRate = sRate;

	osmoSdrSource = osmosdr::source::make();	
	nullSink = gr::blocks::null_sink::make (sizeof(gr_complex));
	multi = gr::blocks::multiply_const_cc::make (1/256.0);

	osmoSdrSource->set_sample_rate(sampleRate);
	osmoSdrSource->set_gain_mode(false);

	osmoSdrSource->set_freq_corr(freqCorr);

	connect (osmoSdrSource, 0, multi, 0);
	connect (multi, 0, self(), 0);
	connect (self(), 0, nullSink, 0);
}

TrxDev_osmosdr::~TrxDev_osmosdr()
{

}

void TrxDev_osmosdr::setSampleRate(int f) {
	sampleRate = f;
	osmoSdrSource->set_sample_rate(sampleRate);
}

void TrxDev_osmosdr::setCenterFreq(qint64 f) {
	qDebug() << "OSMO" << f;
	centerFreq = f;
	osmoSdrSource->set_center_freq(centerFreq);
}

void TrxDev_osmosdr::setPreamp(int n) {
	osmoSdrSource->set_gain_mode(n>0);
	if (n<=0)
		osmoSdrSource->set_if_gain(40+n);
}

void TrxDev_osmosdr::setAntenne(int) { }

int TrxDev_osmosdr::getRXLowFreq() {
	return 53;
}

int TrxDev_osmosdr::getRXHighFreq() {
	return 2100;
}

void TrxDev_osmosdr::setPTT(int) { }
void TrxDev_osmosdr::setTXPower(int) { }
int TrxDev_osmosdr::getTXLowFreq() { return 0; }
int TrxDev_osmosdr::getTXHighFreq() { return 0; }
void TrxDev_osmosdr::setTXFreq(qint64) { }
void TrxDev_osmosdr::setTXRate(int) { }
void TrxDev_osmosdr::setPresel(int) { }
void TrxDev_osmosdr::setCWMode(bool cw) { }

void TrxDev_osmosdr::setFreqCorr(int k) {
	osmoSdrSource->set_freq_corr(k);
}

void TrxDev_osmosdr::setDCFilter(bool) { }
void TrxDev_osmosdr::setPosition(int pos) {}
int TrxDev_osmosdr::getPosition() { return 0;}
