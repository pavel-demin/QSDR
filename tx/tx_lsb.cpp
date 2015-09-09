#include "tx_lsb.h"
#include "config.h"

Tx_lsb_sptr make_tx_lsb(int sampleRate, int fMin, int  fMax) {
	return gnuradio::get_initial_sptr(new Tx_lsb(sampleRate, fMin, fMax));
}

Tx_lsb::Tx_lsb(int sRate, int fMin, int fMax) : Tx("LSB-TX") {
	sampleRate = sRate;
	firdes = new gr::filter::firdes();
	firFilter_fcc = gr::filter::fir_filter_fcc::make (1, firdes->complex_band_pass(1, sampleRate, -fMax, -fMin, 300));
	connect(self(), 0, firFilter_fcc, 0);
	connect(firFilter_fcc, 0, self(), 0);
}

Tx_lsb::~Tx_lsb()
{

}

