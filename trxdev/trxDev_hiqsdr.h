#ifndef TRXDEV_HIQSDR_SOURCE_H
#define TRXDEV_HIQSDR_SOURCE_H

#include <gnuradio/hier_block2.h>
#include <gnuradio/blocks/null_sink.h>
#include <hiqsdr/source.h>
#include <hiqsdr/sink.h>
#include "trxDev.h"


class TrxDev_hiqsdr;

typedef boost::shared_ptr<TrxDev_hiqsdr> TrxDev_hiqsdr_sptr;

TrxDev_hiqsdr_sptr make_trxdev_hiqsdr(int sampleRate, const char* ip);

class TrxDev_hiqsdr : public TrxDev
{

public:
    TrxDev_hiqsdr(int sampleFreq, const char* ip);
    ~TrxDev_hiqsdr();

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
	void setTXRate(int f);
	void setCWMode(bool);

private:
	gr::hiqsdr::sink::sptr hiqsdrSink;
	gr::hiqsdr::source::sptr hiqsdrSource;
	gr::blocks::moving_average_cc::sptr movingAverage;
	gr::blocks::add_cc::sptr adder;

	gr::filter::firdes *firdes;
	int sampleRate, centerFreq;
	int presel;
	bool dcFilter;

};

#endif 
