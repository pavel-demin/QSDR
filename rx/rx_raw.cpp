#include "rx_raw.h"
#include "config.h"

Rx_raw_sptr make_rx_raw(int sampleRate) {
	return gnuradio::get_initial_sptr(new Rx_raw(sampleRate));
}

Rx_raw::Rx_raw(int sRate) : Rx("RAW-RX") {
	sampleRate = sRate;
	agcVal = false;

	firdes = new gr::filter::firdes();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	smeter = gr::analog::probe_avg_mag_sqrd_c::make (0, 0.0001);
	c2f = gr::blocks::complex_to_float::make (1);
	agc = gr::analog::agc2_cc::make (0.01, 0.1, 0.1);

	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, smeter, 0);
	connect(filter_if, 0, c2f, 0);
	connect(c2f, 0, self(), 0);
	connect(c2f, 1, self(), 1);
}

Rx_raw::~Rx_raw()
{

}

void Rx_raw::setAGC(bool on, double att, double dec, double ref) {
	if (agcVal) {
		disconnect(filter_if, 0, agc, 0);
		disconnect(agc, 0, c2f, 0);
	} else {
		disconnect(filter_if, 0, c2f, 0);
	}
	agcVal = on;
	if (agcVal) {
		agc.reset();
		agc = gr::analog::agc2_cc::make (att, dec, ref);
		connect(filter_if, 0, agc, 0);
		connect(agc, 0, c2f, 0);
	} else {
		connect(filter_if, 0, c2f, 0);
	}
}

void Rx_raw::setFreq(int f) {
	filter_if->set_center_freq(f);
}

void Rx_raw::setFilter(int filterLo, int filterHi, int filterCut) {
}

void Rx_raw::setFilterNotch(int freq, int bw) {
}

void Rx_raw::setNotch(bool v) {
}

void Rx_raw::setSampleRate(int f) {
	sampleRate = f;
	disconnect(self(), 0, filter_if, 0);
	disconnect(filter_if, 0, smeter, 0);
	if (agcVal) {
		disconnect(filter_if, 0, agc, 0);
		disconnect(agc, 0, c2f, 0);
	} else {
		disconnect(filter_if, 0, c2f, 0);
	}
	filter_if.reset();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, smeter, 0);
	if (agcVal) {
		connect(filter_if, 0, agc, 0);
		connect(agc, 0, c2f, 0);
	} else {
		connect(filter_if, 0, c2f, 0);
	}
}

void Rx_raw::setSquelch(int n) {
}

float Rx_raw::getSignal() {
	return 10*log10(smeter->level());
}
