#ifndef RX_AM_H
#define RX_AM_H

#include <gnuradio/hier_block2.h>
#include "rx.h"
#include "gnuradio.h"

class Rx_am;

typedef boost::shared_ptr<Rx_am> Rx_am_sptr;
Rx_am_sptr make_rx_am(int sampleRate);

class Rx_am : public Rx 
{

public:
    Rx_am(int);
    ~Rx_am();

	void setAGC(bool on, double att, double dec, double ref);
	void setFreq(int f);
	void setFilter(int, int, int);
	void setFilterNotch(int, int);
	void setNotch(bool);
	void setSampleRate(int f);
	void setSquelch(int n);
	float getSignal();

private:
	gr::filter::freq_xlating_fir_filter_ccf::sptr filter_f;
	gr::filter::freq_xlating_fir_filter_ccf::sptr filter_if;
	gr::analog::probe_avg_mag_sqrd_c::sptr smeter;
	gr::filter::firdes *firdes;
	gr::analog::agc2_ff::sptr agc;
	gr::blocks::complex_to_mag::sptr c2mag;

	int sampleRate;
	bool agcVal;

};

#endif // RX_AM_H
