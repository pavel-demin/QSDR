#ifndef RXSINK_MESSAGE_H
#define RXSINK_MESSAGE_H

#include <gnuradio/hier_block2.h>
#include "rxSink.h"
#include "gnuradio.h"


class RxSink_message;

typedef boost::shared_ptr<RxSink_message> RxSink_message_sptr;
RxSink_message_sptr make_rx_sink_message(gr::msg_queue::sptr msgq);


class RxSink_message : public RxSink
{

public:
    RxSink_message(gr::msg_queue::sptr msgq);
    ~RxSink_message();

private:
	gr::blocks::message_sink::sptr msgSink;
	gr::blocks::add_ff::sptr adder;
	gr::filter::rational_resampler_base_fff::sptr resampler;
	gr::filter::firdes *firdes;
	gr::vocoder::ulaw_encode_sb::sptr ulawEncoder;
	gr::blocks::float_to_short::sptr float2short;
	gr::blocks::null_sink::sptr nullSink;
};

#endif 

