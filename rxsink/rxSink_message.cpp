#include <gnuradio/io_signature.h>
#include "rxSink_message.h"
#include "config.h"

RxSink_message_sptr make_rx_sink_message(gr::msg_queue::sptr msgq) {
	return gnuradio::get_initial_sptr(new RxSink_message(msgq));
}

RxSink_message::RxSink_message(gr::msg_queue::sptr msgq) : RxSink("RXSINK-MESSAGE") {

	msgSink = gr::blocks::message_sink::make(sizeof(char), msgq, true);
	adder = gr::blocks::add_ff::make (1);
	firdes = new gr::filter::firdes();
	resampler = gr::filter::rational_resampler_base_fff::make (1, AUDIO_RATE/REMOTE_AUDIO_RATE, firdes->low_pass(1, AUDIO_RATE, REMOTE_AUDIO_RATE*7/16, REMOTE_AUDIO_RATE/16));
	ulawEncoder = gr::vocoder::ulaw_encode_sb::make();
	float2short = gr::blocks::float_to_short::make (1, 32768);
	nullSink = gr::blocks::null_sink::make (sizeof(float));

	connect(self(), 0, adder, 0);
	connect(self(), 1, nullSink, 0);
	connect(adder, 0, resampler, 0);
	connect(resampler, 0, float2short, 0);
	connect(float2short, 0, ulawEncoder, 0);
	connect(ulawEncoder, 0, msgSink, 0);
}

RxSink_message::~RxSink_message()
{

}

