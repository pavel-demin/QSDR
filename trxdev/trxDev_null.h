#ifndef TRXDEV_NULL_SOURCE_H
#define TRXDEV_NULL_SOURCE_H

#include <gnuradio/hier_block2.h>
#include "gnuradio.h"
#include "trxDev.h"

class TrxDev_null;

typedef boost::shared_ptr<TrxDev_null> TrxDev_null_sptr;

TrxDev_null_sptr make_trxdev_null(int sampleRate);

class TrxDev_null : public TrxDev
{

public:
    TrxDev_null(int sampleFreq);
    ~TrxDev_null();

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
	gr::blocks::null_source::sptr nullSource;
	gr::blocks::throttle::sptr throttle;
	int sampleRate, centerFreq;

};

#endif 
