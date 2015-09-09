#ifndef RX_H
#define RX_H

#include <gnuradio/hier_block2.h>

class Rx;

typedef boost::shared_ptr<Rx> Rx_sptr;

class Rx : public gr::hier_block2
{

public:
    Rx(std::string src_name);
    ~Rx();

	virtual void setAGC(bool on, double att=1, double dec=1, double ref=0.1)=0;
	virtual void setFreq(int f)=0;
	virtual void setFilter(int, int, int)=0;
	virtual void setFilterNotch(int, int)=0;
	virtual void setNotch(bool)=0;
	virtual void setSampleRate(int f)=0;
	virtual void setSquelch(int n)=0;
	virtual float getSignal()=0;

};

#endif // RX_H
