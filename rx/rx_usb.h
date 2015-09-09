#ifndef RX_LSB_H
#define RX_LSB_H

#include <gnuradio/hier_block2.h>
#include "rx.h"
#include "gnuradio.h"

class Rx_usb;

typedef boost::shared_ptr<Rx_usb> Rx_usb_sptr;
Rx_usb_sptr make_rx_usb(int sampleRate);

class Rx_usb : public Rx 
{

public:
    Rx_usb(int);
    ~Rx_usb();

	void setAGC(bool on, double att, double dec, double ref);
	void setFreq(int f);
	void setFilter(int, int, int);
	void setFilterNotch(int, int);
	void setNotch(bool);
	void setSampleRate(int f);
	void setSquelch(int n);
	float getSignal();

private:
	gr::filter::freq_xlating_fir_filter_ccc::sptr filter_c;
	gr::filter::freq_xlating_fir_filter_ccf::sptr filter_if;
	gr::analog::probe_avg_mag_sqrd_c::sptr smeter;
	gr::filter::fir_filter_ccf::sptr firFilter_ccf; 
	gr::filter::firdes *firdes;
	gr::blocks::add_ff::sptr adder;
	gr::blocks::complex_to_float::sptr c2f;
	gr::analog::agc2_ff::sptr agc;

	int sampleRate;
	bool notchOn;
	bool agcVal;

};

#endif // RX_LSB_H
