#include <gnuradio/io_signature.h>
#include "uhd/usrp/multi_usrp.hpp"
#include "trxDev_usrp.h"
#include "config.h"

static bool is_ready = false;

TrxDev_usrp_sptr make_trxdev_usrp(int sampleRate) {
	return gnuradio::get_initial_sptr(new TrxDev_usrp(sampleRate));
}

TrxDev_usrp::TrxDev_usrp(int sRate ) : TrxDev("USRP") {
	is_ready = false;
	sampleRate = sRate;
	//uhd::device_addr_t addr("master_clock_rate=30.72e6"); 
	uhd::device_addr_t addr("master_clock_rate=15.36e6"); 

	usrpSource = gr::uhd::usrp_source::make (addr, uhd::stream_args_t("fc32"));
	usrpSink = gr::uhd::usrp_sink::make (addr, uhd::stream_args_t("fc32"));
	usrpSource->set_samp_rate(sampleRate);
	usrpSink->set_samp_rate(48000);
	connect (usrpSource, 0, self(), 0);
	connect (self(), 0, usrpSink, 0);
	//nullSink = gr::blocks::null_sink::make (sizeof(gr_complex));
	//connect (self(), 0, nullSink, 0);

	is_ready = true;
	qDebug() << "READY";
}

TrxDev_usrp::~TrxDev_usrp()
{
	is_ready = false;

}

void TrxDev_usrp::setSampleRate(int f) {
	sampleRate = f;
	usrpSource->set_samp_rate(sampleRate);
}

void TrxDev_usrp::setCenterFreq(qint64 f) {
	centerFreq = f;
	if (is_ready)
		usrpSource->set_center_freq(centerFreq);
}

void TrxDev_usrp::setPreamp(int preamp) {
	if (is_ready)
		usrpSource->set_gain(preamp+44);
}

void TrxDev_usrp::setPresel(int pre) {
}

void TrxDev_usrp::setAntenne(int ant) {
}

int TrxDev_usrp::getRXLowFreq() {
	return 50;
}

int TrxDev_usrp::getRXHighFreq() {
	return 6000;
}

void TrxDev_usrp::setPTT(int on) {
	if (on) {
		usrpSink->set_start_time(usrpSink->get_time_now() + uhd::time_spec_t(0,0.3));
	}
}

void TrxDev_usrp::setTXPower(int txPower) {
	if (is_ready)
		usrpSink->set_gain(txPower);
}

int TrxDev_usrp::getTXLowFreq() {
	return 50;
}

int TrxDev_usrp::getTXHighFreq() {
	return 6000;
}

void TrxDev_usrp::setTXFreq(qint64 f) {
	if (is_ready)
		usrpSink->set_center_freq(f);
}

void TrxDev_usrp::setTXRate(int f) {
	if (is_ready)
		usrpSink->set_samp_rate(f);
}

void TrxDev_usrp::setCWMode(bool cw) {
}

void TrxDev_usrp::setDCFilter(bool dc) {
}

void TrxDev_usrp::setFreqCorr(int k) {
}
void TrxDev_usrp::setPosition(int pos) {}
int TrxDev_usrp::getPosition() { return 0;}
