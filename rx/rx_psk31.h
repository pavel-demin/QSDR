#ifndef RX_PSK31_H
#define RX_PSK31_H

#include <gnuradio/hier_block2.h>
#include "rx.h"
#include "gr_psk31_decoder.h"
#include "gr_reader_i.h"
#include "gnuradio.h"

class Rx_psk31;

typedef boost::shared_ptr<Rx_psk31> Rx_psk31_sptr;
Rx_psk31_sptr make_rx_psk31(int sampleRate, Parent *p, cbF f);

class Rx_psk31 : public Rx 
{

public:
    Rx_psk31(int, Parent *p, cbF f);
    ~Rx_psk31();

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
	gr::filter::firdes *firdes;
	gr::blocks::add_ff::sptr adder;
	gr::blocks::complex_to_float::sptr c2f;
	gr::analog::agc2_ff::sptr agc;
	gr::analog::agc2_cc::sptr agc2;
	gr::digital::costas_loop_cc::sptr costa; 
	gr::blocks::stream_to_vector::sptr strToVect;
	gr_psk31_decoder_sptr psk31Decoder;
	gr_reader_i_sptr readerI;

	int sampleRate;
	bool agcVal;

};

#endif // RX_PSK31_H
