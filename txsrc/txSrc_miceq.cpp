#include "txSrc_miceq.h"
#include "config.h"

TxSrc_miceq_sptr make_txsrc_miceq(char *devName) {
	return gnuradio::get_initial_sptr(new TxSrc_miceq(devName));
}

TxSrc_miceq::TxSrc_miceq(char *devName) : TxSrc("TXSRC-MIC") {
	audioSourceMic   = gr::audio::source::make(AUDIO_RATE, devName);
	firdes = new gr::filter::firdes();
	filter_eq_lo =  gr::filter::fir_filter_fff::make (1, firdes->band_pass(1, AUDIO_RATE, 300,800,300));
	filter_eq_mi =  gr::filter::fir_filter_fff::make (1, firdes->band_pass(1, AUDIO_RATE, 800,2000,300));
	filter_eq_hi =  gr::filter::fir_filter_fff::make (1, firdes->band_pass(1, AUDIO_RATE, 2000,3000,300));
	adder_eq = gr::blocks::add_ff::make (1);
	limiter = gr_make_limit_ff(0.8);

	connect(audioSourceMic, 0, filter_eq_lo, 0);
	connect(audioSourceMic, 0, filter_eq_mi, 0);
	connect(audioSourceMic, 0, filter_eq_hi, 0);
	connect(filter_eq_lo, 0, adder_eq, 0);
	connect(filter_eq_mi, 0, adder_eq, 1);
	connect(filter_eq_hi, 0, adder_eq, 2);
	connect(adder_eq, 0, limiter, 0);
	connect(limiter, 0, self(), 0);
}

TxSrc_miceq::~TxSrc_miceq()
{

}


