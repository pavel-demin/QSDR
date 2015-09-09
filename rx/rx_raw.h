#ifndef RX_RAW_H
#define RX_RAW_H

#include <gnuradio/hier_block2.h>
#include "rx.h"
#include "gnuradio.h"

class Rx_raw;

typedef boost::shared_ptr<Rx_raw> Rx_raw_sptr;
Rx_raw_sptr make_rx_raw(int sampleRate);

class Rx_raw : public Rx 
{

public:
    Rx_raw(int);
    ~Rx_raw();

	void setAGC(bool on, double att, double dec, double ref);
	void setFreq(int f);
	void setFilter(int, int, int);
	void setFilterNotch(int, int);
	void setNotch(bool);
	void setSampleRate(int f);
	void setSquelch(int n);
	float getSignal();

private:
	gr::filter::freq_xlating_fir_filter_ccf::sptr filter_if;
	gr::analog::probe_avg_mag_sqrd_c::sptr smeter;
	gr::filter::firdes *firdes;
	gr::blocks::complex_to_float::sptr c2f;
	gr::analog::agc2_cc::sptr agc;

	int sampleRate;
	bool agcVal;

};

#endif // RX_RAW_H
