#ifndef RX_FM_H
#define RX_FM_H

#include <gnuradio/hier_block2.h>
#include "rx.h"
#include "gnuradio.h"

class Rx_fm;

typedef boost::shared_ptr<Rx_fm> Rx_fm_sptr;
Rx_fm_sptr make_rx_fm(int sampleRate);

class Rx_fm : public Rx 
{

public:
    Rx_fm(int);
    ~Rx_fm();

	void setAGC(bool on, double att, double dec, double ref);
	void setFreq(int f);
	void setFilter(int, int, int);
	void setFilterNotch(int, int);
	void setNotch(bool);
	void setSampleRate(int f);
	void setSquelch(int n);
	float getSignal();

private:
	gr::filter::freq_xlating_fir_filter_ccf::sptr filter_if, filter_fm;
	gr::analog::probe_avg_mag_sqrd_c::sptr smeter;
	gr::filter::firdes *firdes;
	gr::analog::simple_squelch_cc::sptr squelch;
	gr::analog::quadrature_demod_cf::sptr quadDemodFM;
	gr::filter::fir_filter_fff::sptr fmAudioFilter;

	int sampleRate;

};

#endif // RX_AM_H
