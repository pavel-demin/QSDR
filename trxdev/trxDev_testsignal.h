#ifndef TRXDEV_TESTSIGNAL_SOURCE_H
#define TRXDEV_TESTSIGNAL_SOURCE_H

#include <gnuradio/hier_block2.h>
#include "gnuradio.h"
#include "trxDev.h"

class TrxDev_testsignal;

typedef boost::shared_ptr<TrxDev_testsignal> TrxDev_testsignal_sptr;

TrxDev_testsignal_sptr make_trxdev_testsignal(int sampleRate);

class TrxDev_testsignal : public TrxDev
{

public:
    TrxDev_testsignal(int sampleFreq);
    ~TrxDev_testsignal();

	void setSampleRate(int f);
	void setCenterFreq(qint64 f);
	void setPreamp(int n);
	void setPresel(int n);
	void setAntenne(int);
	int getRXLowFreq();
	int getRXHighFreq();
	void setFreqCorr(int);
	void setDCFilter(bool);
	void setPosition(int sec);
	int getPosition();

	void setPTT(int);
	void setTXPower(int);
	int getTXLowFreq();
	int getTXHighFreq();
	void setTXFreq(qint64);
	void setTXRate(int);
	void setCWMode(bool);

private:
	gr::blocks::null_sink::sptr nullSink;
	gr::blocks::throttle::sptr throttle;
	gr::blocks::add_cc::sptr adder;
	gr::analog::sig_source_c::sptr twoToneTest0,twoToneTest1;
	gr::analog::noise_source_c::sptr noiceSource;
	int sampleRate, centerFreq;

};

#endif 
