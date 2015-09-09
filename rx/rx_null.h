#ifndef RX_NULL_H
#define RX_NULL_H

#include <gnuradio/hier_block2.h>
#include "rx.h"
#include "gnuradio.h"

class Rx_null;

typedef boost::shared_ptr<Rx_null> Rx_null_sptr;
Rx_null_sptr make_rx_null(int sampleRate);

class Rx_null : public Rx 
{

public:
    Rx_null(int);
    ~Rx_null();

	void setAGC(bool on, double att, double dec, double ref);
	void setFreq(int f);
	void setFilter(int, int, int);
	void setFilterNotch(int, int);
	void setNotch(bool);
	void setSampleRate(int f);
	void setSquelch(int n);
	float getSignal();

private:

	gr::blocks::null_sink::sptr nullSinkc;
	gr::blocks::null_source::sptr nullSource;
	gr::blocks::throttle::sptr throttle;
};

#endif // RX_NULL_H
