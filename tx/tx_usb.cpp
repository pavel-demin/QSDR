#include "tx_usb.h"
#include "config.h"

Tx_usb_sptr make_tx_usb(int sampleRate, int fMin, int fMax) {
	return gnuradio::get_initial_sptr(new Tx_usb(sampleRate, fMin, fMax));
}

Tx_usb::Tx_usb(int sRate, int fMin, int fMax) : Tx("USB-TX") {
	sampleRate = sRate;
	firdes = new gr::filter::firdes();
	firFilter_fcc = gr::filter::fir_filter_fcc::make (1, firdes->complex_band_pass(1, sampleRate, fMin, fMax, 300));
	connect(self(), 0, firFilter_fcc, 0);
	connect(firFilter_fcc, 0, self(), 0);
}

Tx_usb::~Tx_usb()
{

}

