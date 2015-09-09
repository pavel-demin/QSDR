#include "txSrc_message.h"
#include "config.h"

TxSrc_message_sptr make_txsrc_message(gr::msg_queue::sptr msgq) {
	return gnuradio::get_initial_sptr(new TxSrc_message(msgq));
}

TxSrc_message::TxSrc_message(gr::msg_queue::sptr msgq) : TxSrc("TXSRC-MESSAGE") {

	msgSource = gr::blocks::message_source::make(sizeof(char), msgq);
	ulawDecoder = gr::vocoder::ulaw_decode_bs::make();
	firdes = new gr::filter::firdes();
	resampler = gr::filter::rational_resampler_base_fff::make (AUDIO_RATE/REMOTE_AUDIO_RATE, 1, firdes->low_pass(1, AUDIO_RATE, REMOTE_AUDIO_RATE*7/16, REMOTE_AUDIO_RATE/16));
	short2float = gr::blocks::short_to_float::make (1, 8000);
	limiter = gr_make_limit_ff(0.8);

	connect(msgSource, 0, ulawDecoder, 0);
	connect(ulawDecoder, 0, short2float, 0);
	connect(short2float, 0, resampler, 0);
	connect(resampler, 0, limiter, 0);
	connect(limiter, 0, self(), 0);
}

TxSrc_message::~TxSrc_message()
{

}


