#include "rx_usb.h"
#include "config.h"

Rx_usb_sptr make_rx_usb(int sampleRate) {
	return gnuradio::get_initial_sptr(new Rx_usb(sampleRate));
}

Rx_usb::Rx_usb(int sRate) : Rx("USB-RX") {
	sampleRate = sRate;
	notchOn = false;
	agcVal = false;

	firdes = new gr::filter::firdes();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	filter_c = gr::filter::freq_xlating_fir_filter_ccc::make (IF_RATE / AUDIO_RATE, firdes->complex_band_pass(1, IF_RATE, 300, 3000, 300), 0, IF_RATE);
	firFilter_ccf = gr::filter::fir_filter_ccf::make (1, firdes->band_reject(1, AUDIO_RATE, 5000, 5200, 500));
	smeter = gr::analog::probe_avg_mag_sqrd_c::make (0, 0.0001);
	adder = gr::blocks::add_ff::make (1);
	c2f = gr::blocks::complex_to_float::make (1);
	agc = gr::analog::agc2_ff::make (0.01, 0.1, 0.1);

	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, filter_c, 0);
	connect(filter_c, 0, smeter, 0);
	if (notchOn) {
		connect(filter_c, 0, firFilter_ccf, 0);
		connect(firFilter_ccf, 0, c2f, 0);
	} else {
		connect(filter_c, 0, c2f, 0);
	}
	connect(c2f, 0, adder, 0);
	connect(c2f, 1, adder, 1);
	connect(adder, 0, self(), 0);
	connect(adder, 0, self(), 1);
}

Rx_usb::~Rx_usb()
{

}

void Rx_usb::setAGC(bool on, double att, double dec, double ref) {
	if (agcVal) {
		disconnect(adder, 0, agc, 0);
		disconnect(agc, 0, self(), 0);
		disconnect(agc, 0, self(), 1);
	} else {
		disconnect(adder, 0, self(), 0);
		disconnect(adder, 0, self(), 1);
	}
	agcVal = on;
	if (agcVal) {
		agc.reset();
		agc = gr::analog::agc2_ff::make (att, dec, ref);
		connect(adder, 0, agc, 0);
		connect(agc, 0, self(), 0);
		connect(agc, 0, self(), 1);
	} else {
		connect(adder, 0, self(), 0);
		connect(adder, 0, self(), 1);
	}
}

void Rx_usb::setFreq(int f) {
	filter_if->set_center_freq(f);
}

void Rx_usb::setFilter(int filterLo, int filterHi, int filterCut) {
	if (filterHi > IF_RATE/2)
		filterHi = IF_RATE/2;
	filter_c->set_taps(firdes->complex_band_pass(1, IF_RATE, filterLo, filterHi, filterCut)); 
}

void Rx_usb::setFilterNotch(int freq, int bw) {
	if (freq<bw) 
		freq=bw;
	firFilter_ccf->set_taps(firdes->band_reject(1, AUDIO_RATE, freq-bw/2, freq+bw/2, bw/2)); 
}

void Rx_usb::setNotch(bool v) {
	if (notchOn) {
		disconnect(filter_c, 0, firFilter_ccf, 0);
		disconnect(firFilter_ccf, 0, c2f, 0);
	} else {
		disconnect(filter_c, 0, c2f, 0);
	}
	notchOn = v;
	if (notchOn) {
		connect(filter_c, 0, firFilter_ccf, 0);
		connect(firFilter_ccf, 0, c2f, 0);
	} else {
		connect(filter_c, 0, c2f, 0);
	}
}

void Rx_usb::setSampleRate(int f) {
	sampleRate = f;
	disconnect(self(), 0, filter_if, 0);
	disconnect(filter_if, 0, filter_c, 0);
	filter_if.reset();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, filter_c, 0);
}

void Rx_usb::setSquelch(int n) {
}

float Rx_usb::getSignal() {
	return 10*log10(smeter->level());
}
