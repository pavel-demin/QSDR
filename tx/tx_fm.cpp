#include "tx_fm.h"
#include "config.h"

Tx_fm_sptr make_tx_fm(int sampleRate, int fMax) {
	return gnuradio::get_initial_sptr(new Tx_fm(sampleRate, fMax));
}

Tx_fm::Tx_fm(int sRate, int fMax) : Tx("FM-TX") {
	sampleRate = sRate;
	multi = gr::blocks::multiply_const_cc::make(0.4);
	firdes = new gr::filter::firdes();
	firFilter_ccc = gr::filter::fir_filter_ccc::make (1, firdes->complex_band_pass(1, sampleRate, -fMax, fMax, 300));
	fmMod = gr::analog::frequency_modulator_fc::make(0.1);
	connect(self(), 0, fmMod, 0);
	connect(fmMod, 0, multi, 0);
	connect(multi, 0, firFilter_ccc, 0);
	connect(firFilter_ccc, 0, self(), 0);
}


Tx_fm::~Tx_fm()
{

}

