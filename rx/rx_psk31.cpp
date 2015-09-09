#include "rx_psk31.h"
#include "config.h"

Rx_psk31_sptr make_rx_psk31(int sampleRate, Parent *p, cbF f) {
	return gnuradio::get_initial_sptr(new Rx_psk31(sampleRate, p ,f));
}

Rx_psk31::Rx_psk31(int sRate, Parent *parent, cbF appendText) : Rx("PSK-RX") {
	sampleRate = sRate;
	agcVal = false;

	firdes = new gr::filter::firdes();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	filter_c = gr::filter::freq_xlating_fir_filter_ccc::make (IF_RATE / AUDIO_RATE, firdes->complex_band_pass(1, IF_RATE, -3000, -300, 300), 0, IF_RATE);
	smeter = gr::analog::probe_avg_mag_sqrd_c::make (0, 0.0001);
	adder = gr::blocks::add_ff::make (1);
	c2f = gr::blocks::complex_to_float::make (1);
	agc = gr::analog::agc2_ff::make (0.01, 0.1, 0.1);
	agc2 = gr::analog::agc2_cc::make (1, 0.1, 0.1, 1);
	costa = gr::digital::costas_loop_cc::make (M_PI/1000, 2);
	strToVect = gr::blocks::stream_to_vector::make (sizeof(gr_complex), 1024);
	psk31Decoder = gr_make_psk31_decoder(1024, AUDIO_RATE);
	readerI = gr_make_reader_i(parent, appendText);

	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, filter_c, 0);
	connect(filter_c, 0, smeter, 0);
	connect(filter_c, 0, c2f, 0);
	connect(c2f, 0, adder, 0);
	connect(c2f, 1, adder, 1);
	connect(adder, 0, self(), 0);
	connect(adder, 0, self(), 1);

	connect(filter_c, 0, agc2, 0);
	connect(agc2, 0, costa, 0);
	connect(costa, 0, strToVect, 0);
	connect(strToVect, 0, psk31Decoder, 0);
	connect(psk31Decoder, 0, readerI, 0);
}

Rx_psk31::~Rx_psk31()
{

}

void Rx_psk31::setAGC(bool on, double att, double dec, double ref) {
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

void Rx_psk31::setFreq(int f) {
	//filter_if->set_center_freq(f - CW_TONE);
	filter_if->set_center_freq(f + 600);
}

void Rx_psk31::setFilter(int filterLo, int filterHi, int filterCut) {
	if (filterHi > IF_RATE/2)
		filterHi = IF_RATE/2;
	filter_c->set_taps(firdes->complex_band_pass(1, IF_RATE, -filterHi, -filterLo, filterCut)); 
}

void Rx_psk31::setFilterNotch(int freq, int bw) {
}

void Rx_psk31::setNotch(bool v) {
}

void Rx_psk31::setSampleRate(int f) {
	sampleRate = f;
	disconnect(self(), 0, filter_if, 0);
	disconnect(filter_if, 0, filter_c, 0);
	filter_if.reset();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, filter_c, 0);
}

void Rx_psk31::setSquelch(int n) {
}

float Rx_psk31::getSignal() {
	return 10*log10(smeter->level());
}
