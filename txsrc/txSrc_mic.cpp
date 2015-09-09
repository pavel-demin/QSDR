#include "txSrc_mic.h"
#include "config.h"

TxSrc_mic_sptr make_txsrc_mic(char *devName) {
	return gnuradio::get_initial_sptr(new TxSrc_mic(devName));
}

TxSrc_mic::TxSrc_mic(char *devName) : TxSrc("TXSRC-MIC") {
	try {
		audioSourceMic   = gr::audio::source::make(AUDIO_RATE, devName);
	} catch (...) {
		audioSourceMic   = gr::audio::source::make(AUDIO_RATE, "default");
	}
	limiter = gr_make_limit_ff(0.8);

	connect(audioSourceMic, 0, limiter, 0);
	connect(limiter, 0, self(), 0);
}

TxSrc_mic::~TxSrc_mic()
{

}


