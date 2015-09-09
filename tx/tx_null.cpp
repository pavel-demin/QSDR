#include "tx_null.h"

Tx_null_sptr make_tx_null(int sampleRate) {
	return gnuradio::get_initial_sptr(new Tx_null(sampleRate));
}

Tx_null::Tx_null(int sRate) : Tx("TX-NULL") {
	sampleRate = sRate;
	f2c = gr::blocks::float_to_complex::make (1);
	multi = gr::blocks::multiply_const_ff::make (0);

	connect(self(), 0, multi, 0);
	connect(multi, 0, f2c, 0);
	connect(f2c, 0, self(), 0);
}

Tx_null::~Tx_null()
{
}
