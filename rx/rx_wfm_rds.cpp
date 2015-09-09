#include "rx_wfm_rds.h"
#include "config.h"

Rx_wfmrds_sptr make_rx_wfmrds(int sampleRate, gr::msg_queue::sptr msgq) {
	return gnuradio::get_initial_sptr(new Rx_wfmrds(sampleRate, msgq));
}

Rx_wfmrds::Rx_wfmrds(int sRate, gr::msg_queue::sptr msgq) : Rx("WFM-RDS-RX") {
	int audio_decim = IF_STEREO_RATE/AUDIO_RATE;
	sampleRate = sRate;
	if (sRate<IF_STEREO_RATE)
		sampleRate = IF_STEREO_RATE;
	firdes = new gr::filter::firdes();
	filter_wfm = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_STEREO_RATE, firdes->low_pass(1, sampleRate, 120000, 20000), 0, sampleRate);
	gr_diff_decoder_bb_0 = gr::digital::diff_decoder_bb::make(2);
	gr_multiply_xx_0 = gr::blocks::multiply_ff::make(1);
	gr_multiply_xx_1 = gr::blocks::multiply_ff::make(1);
	gr_multiply_xx_2 = gr::blocks::multiply_ff::make(1);
	gr_pll_freqdet_cf_0 = gr::analog::pll_freqdet_cf::make(0.7,2,-2);
	gr_rds_bpsk_demod_0 = gr_rds_make_bpsk_demod(AUDIO_RATE);
	gr_rds_data_decoder_0 = gr_rds_make_data_decoder(msgq);
	gr_rds_freq_divider_0 = gr_rds_make_freq_divider(2);
	left = gr::blocks::add_ff::make(1);
	lmr_bb_filter = gr::filter::fir_filter_fff::make(audio_decim, firdes->low_pass( 1, IF_STEREO_RATE, 15000, 1000));
	lmr_filter = gr::filter::fir_filter_fff::make(1, firdes->band_pass( 1, IF_STEREO_RATE, 23000, 53000, 1000));
	lpr_filter = gr::filter::fir_filter_fff::make(audio_decim, firdes->low_pass( 1, IF_STEREO_RATE, 15000, 1000));
	rds_bb_filter = gr::filter::fir_filter_fff::make(audio_decim, firdes->low_pass( 1, AUDIO_RATE, 2400, 200));
	rds_filter = gr::filter::fir_filter_fff::make(1, firdes->band_pass( 1, IF_STEREO_RATE, 54000, 60000, 3000));
	right = gr::blocks::sub_ff::make(1) ;
	smeter = gr::analog::probe_avg_mag_sqrd_c::make (0, 0.0001);
	gr_rds_clock_filter = gr::filter::fir_filter_fff::make(1, firdes->band_pass( 1, AUDIO_RATE, 19000/8-50, 19000/8+50, 100));

	filterPilotTone = gr::filter::fir_filter_fcc::make (1, firdes->complex_band_pass(1, IF_STEREO_RATE, 18500, 19500, 1000));
	pllPilotTone = gr::analog::pll_refout_cc::make(M_PI/200, 18990.0*2*M_PI/IF_STEREO_RATE, 19010.0*2*M_PI/IF_STEREO_RATE);
	c2imag = gr::blocks::complex_to_imag::make(1);
	c2imag1 = gr::blocks::complex_to_imag::make(1);
	multic = gr::blocks::multiply_cc::make(1);

	connect(self(), 0, filter_wfm, 0);
	connect(filter_wfm, 0, smeter, 0);
	connect(filter_wfm, 0, gr_pll_freqdet_cf_0, 0);
	connect(gr_pll_freqdet_cf_0, 0, lmr_filter, 0);
	connect(gr_pll_freqdet_cf_0, 0, filterPilotTone, 0);
	connect(filterPilotTone, 0, pllPilotTone, 0);
	connect(pllPilotTone, 0, multic, 0);
	connect(pllPilotTone, 0, multic, 1);
	connect(pllPilotTone, 0, multic, 2);
	connect(multic, 0, c2imag, 0);
	connect(c2imag, 0, gr_multiply_xx_0, 1);

	connect(gr_pll_freqdet_cf_0, 0, rds_filter, 0);
	connect(rds_filter, 0, gr_multiply_xx_0, 0);
	connect(gr_multiply_xx_0, 0, rds_bb_filter, 0);
//	connect(c2imag1, 0, gr_rds_freq_divider_0, 0);
//	connect(gr_rds_freq_divider_0, 0, rds_clk_filter, 0);
	connect(pllPilotTone, 0, c2imag1, 0);
	connect(c2imag1, 0, gr_multiply_xx_1, 0);
	connect(c2imag1, 0, gr_multiply_xx_1, 1);
	connect(lmr_filter, 0, gr_multiply_xx_1, 2);
	connect(gr_multiply_xx_1, 0, lmr_bb_filter, 0);
	connect(rds_bb_filter, 0, gr_multiply_xx_2, 0);
	connect(rds_bb_filter, 0, gr_multiply_xx_2, 1);
	connect(gr_multiply_xx_2, 0, gr_rds_clock_filter, 0);
	connect(gr_rds_clock_filter, 0, gr_rds_freq_divider_0, 0);
	connect(left, 0, self(), 0);
	connect(right, 0, self(), 1);
	connect(lmr_bb_filter, 0, left, 0);
	connect(lpr_filter, 0, left, 1);
	connect(lpr_filter, 0, right, 1);
	connect(lmr_bb_filter, 0, right, 0);
	connect(gr_pll_freqdet_cf_0, 0, lpr_filter, 0);
	connect(gr_diff_decoder_bb_0, 0, gr_rds_data_decoder_0, 0);
	connect(gr_rds_bpsk_demod_0, 0, gr_diff_decoder_bb_0, 0);
	connect(gr_rds_freq_divider_0, 0, gr_rds_bpsk_demod_0, 1);
	connect(rds_bb_filter, 0, gr_rds_bpsk_demod_0, 0);

}

Rx_wfmrds::~Rx_wfmrds()
{

}

void Rx_wfmrds::setAGC(bool on, double att, double dec, double ref) {
}

void Rx_wfmrds::setFreq(int f) {
	filter_wfm->set_center_freq(f);
	gr_rds_bpsk_demod_0->reset(0);
	gr_rds_data_decoder_0->reset(0);
}

void Rx_wfmrds::setFilter(int filterLo, int filterHi, int filterCut) {
	if (filterHi > sampleRate/2)
		filterHi = sampleRate/2;

	filter_wfm->set_taps(firdes->low_pass(1, sampleRate, filterHi, filterCut)); 
}

void Rx_wfmrds::setFilterNotch(int freq, int bw) {
}

void Rx_wfmrds::setNotch(bool v) {
}

void Rx_wfmrds::setSampleRate(int f) {
	sampleRate = f;
	if (f<IF_STEREO_RATE)
		sampleRate = IF_STEREO_RATE;
	disconnect(self(), 0, filter_wfm, 0);
	disconnect(filter_wfm, 0, smeter, 0);
	disconnect(filter_wfm, 0, gr_pll_freqdet_cf_0, 0);
	filter_wfm.reset();
	filter_wfm = gr::filter::freq_xlating_fir_filter_ccf::make (sampleRate/IF_STEREO_RATE, firdes->low_pass(1, sampleRate, 120000, 20000), 0, sampleRate);
	connect(self(), 0, filter_wfm, 0);
	connect(filter_wfm, 0, smeter, 0);
	connect(filter_wfm, 0, gr_pll_freqdet_cf_0, 0);
}

void Rx_wfmrds::setSquelch(int n) {
}

float Rx_wfmrds::getSignal() {
	return 10*log10(smeter->level());
}
