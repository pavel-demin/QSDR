#include "rx_am.h"
#include "config.h"

Rx_am_sptr make_rx_am(int sampleRate) {
	return gnuradio::get_initial_sptr(new Rx_am(sampleRate));
}

Rx_am::Rx_am(int sRate) : Rx("AM-RX") {
	sampleRate = sRate;
	agcVal = false;

	firdes = new gr::filter::firdes();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	filter_f =   gr::filter::freq_xlating_fir_filter_ccf::make (IF_RATE / AUDIO_RATE, firdes->low_pass(1, IF_RATE, IF_RATE/4, IF_RATE/8), 0, IF_RATE);
	smeter = gr::analog::probe_avg_mag_sqrd_c::make (0, 0.0001);
	c2mag = gr::blocks::complex_to_mag::make (1);
	agc = gr::analog::agc2_ff::make (0.01, 0.1, 0.1);

	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, filter_f, 0);
	connect(filter_f, 0, smeter, 0);
	connect(filter_f, 0, c2mag, 0);
	connect(c2mag, 0, self(), 0);
	connect(c2mag, 0, self(), 1);

}

Rx_am::~Rx_am()
{

}

void Rx_am::setAGC(bool on, double att, double dec, double ref) {
	if (agcVal) {
		disconnect(c2mag, 0, agc, 0);
		disconnect(agc, 0, self(), 0);
		disconnect(agc, 0, self(), 1);
	} else {
		disconnect(c2mag, 0, self(), 0);
		disconnect(c2mag, 0, self(), 1);
	}
	agcVal = on;
	if (agcVal) {
		agc.reset();
		agc = gr::analog::agc2_ff::make (att, dec, ref);
		connect(c2mag, 0, agc, 0);
		connect(agc, 0, self(), 0);
		connect(agc, 0, self(), 1);
	} else {
		connect(c2mag, 0, self(), 0);
		connect(c2mag, 0, self(), 1);
	}
}

void Rx_am::setFreq(int f) {
	filter_if->set_center_freq(f);
}

void Rx_am::setFilter(int filterLo, int filterHi, int filterCut) {
	if (filterHi > IF_RATE/2)
		filterHi = IF_RATE/2;
	filter_f->set_taps(firdes->low_pass(1, IF_RATE, filterHi, filterCut)); 
}

void Rx_am::setFilterNotch(int freq, int bw) {
}

void Rx_am::setNotch(bool v) {
}

void Rx_am::setSampleRate(int f) {
	sampleRate = f;
	disconnect(self(), 0, filter_if, 0);
	disconnect(filter_if, 0, filter_f, 0);
	filter_if.reset();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, filter_f, 0);
}

void Rx_am::setSquelch(int n) {
}

float Rx_am::getSignal() {
	return 10*log10(smeter->level());
}
