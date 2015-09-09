#include "txSrc_null.h"
#include "config.h"

TxSrc_null_sptr make_txsrc_null() {
	return gnuradio::get_initial_sptr(new TxSrc_null());
}

TxSrc_null::TxSrc_null() : TxSrc("TXSRC-NULL") {
	throttle = gr::blocks::throttle::make(sizeof(float), AUDIO_RATE);
	nullSource = gr::blocks::null_source::make (sizeof(float));

	connect (nullSource, 0, throttle, 0);
	connect (throttle, 0, self(), 0);
}

TxSrc_null::~TxSrc_null()
{

}


