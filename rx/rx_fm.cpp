#include "rx_fm.h"
#include "config.h"

Rx_fm_sptr make_rx_fm(int sampleRate) {
	return gnuradio::get_initial_sptr(new Rx_fm(sampleRate));
}

Rx_fm::Rx_fm(int sRate) : Rx("FM-RX") {
	sampleRate = sRate;

	firdes = new gr::filter::firdes();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	smeter = gr::analog::probe_avg_mag_sqrd_c::make (0, 0.0001);
	
	filter_fm =  gr::filter::freq_xlating_fir_filter_ccf::make (1, firdes->low_pass(1, IF_RATE, IF_RATE/4, IF_RATE/8), 0, IF_RATE);
	squelch = gr::analog::simple_squelch_cc::make (1, 0.001);
	quadDemodFM = gr::analog::quadrature_demod_cf::make (IF_RATE / (2*M_PI*10e3));
	fmAudioFilter  = gr::filter::fir_filter_fff::make (IF_RATE/AUDIO_RATE, firdes->low_pass(1, IF_RATE, 3000, 1000));

	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, filter_fm, 0);
	connect(filter_fm, 0, smeter, 0);
	connect(filter_fm, 0, squelch, 0);
	connect(squelch, 0, quadDemodFM, 0);
	connect(quadDemodFM, 0, fmAudioFilter, 0);
	connect(fmAudioFilter, 0, self(), 0);
	connect(fmAudioFilter, 0, self(), 1);
}

Rx_fm::~Rx_fm()
{

}

void Rx_fm::setAGC(bool on, double att, double dec, double ref) {
}

void Rx_fm::setFreq(int f) {
	filter_if->set_center_freq(f);
}

void Rx_fm::setFilter(int filterLo, int filterHi, int filterCut) {
	if (filterHi > IF_RATE/2)
		filterHi = IF_RATE/2;
	filter_fm->set_taps(firdes->low_pass(1, IF_RATE, filterHi, filterCut)); 
}

void Rx_fm::setFilterNotch(int freq, int bw) {
}

void Rx_fm::setNotch(bool v) {
}

void Rx_fm::setSampleRate(int f) {
	sampleRate = f;
	disconnect(self(), 0, filter_if, 0);
	disconnect(filter_if, 0, filter_fm, 0);
	disconnect(filter_fm, 0, smeter, 0);
	filter_if.reset();
	filter_if = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_RATE, firdes->low_pass(1, sampleRate, IF_RATE/2, IF_RATE/4), 0, sampleRate);
	connect(self(), 0, filter_if, 0);
	connect(filter_if, 0, filter_fm, 0);
	connect(filter_fm, 0, smeter, 0);
}

void Rx_fm::setSquelch(int n) {
	squelch->set_threshold(n);
}

float Rx_fm::getSignal() {
	return 10*log10(smeter->level());
}
