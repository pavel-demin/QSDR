#include "tx_am.h"
#include "config.h"

Tx_am_sptr make_tx_am(int sampleRate, int fMax) {
	return gnuradio::get_initial_sptr(new Tx_am(sampleRate, fMax));
}

Tx_am::Tx_am(int sRate, int fMax) : Tx("AM-TX") {
	sampleRate = sRate;
	firdes = new gr::filter::firdes();
	addConst = gr::blocks::add_const_ff::make(0.2);
	firFilter_fcc = gr::filter::fir_filter_fcc::make (1, firdes->complex_band_pass(1, sampleRate, -fMax, fMax, 300));
	connect(self(), 0, addConst, 0);
	connect(addConst, 0, firFilter_fcc, 0);
	connect(firFilter_fcc, 0, self(), 0);
}


Tx_am::~Tx_am()
{

}

