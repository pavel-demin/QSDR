#ifndef TXSRC_MESSAGE_H
#define TXSRC_MESSAGE_H

#include <gnuradio/hier_block2.h>
#include "txSrc.h"
#include "gr_limit_ff.h"
#include "gnuradio.h"

class TxSrc_message;

typedef boost::shared_ptr<TxSrc_message> TxSrc_message_sptr;
TxSrc_message_sptr make_txsrc_message(gr::msg_queue::sptr msgq);

class TxSrc_message : public TxSrc 
{

public:
    TxSrc_message(gr::msg_queue::sptr msgq);
    ~TxSrc_message();

private:
	gr::blocks::message_source::sptr msgSource;
	gr::filter::rational_resampler_base_fff::sptr resampler;
	gr::filter::firdes *firdes;
	gr::vocoder::ulaw_decode_bs::sptr ulawDecoder;
	gr::blocks::short_to_float::sptr short2float;
	gr::blocks::file_sink::sptr fileSink;
	gr_limit_ff_sptr limiter; 

};

#endif
