#ifndef TRXDEV_OSMOSDR_SOURCE_H
#define TRXDEV_OSMOSDR_SOURCE_H

#include <gnuradio/hier_block2.h>
#include <osmosdr/source.h>
#include <gnuradio/blocks/null_sink.h>
#include "trxDev.h"
#include "config.h"

class TrxDev_osmosdr;

typedef boost::shared_ptr<TrxDev_osmosdr> TrxDev_osmosdr_sptr;

TrxDev_osmosdr_sptr make_trxdev_osmosdr(int sampleRate, int freqCorr);

class TrxDev_osmosdr : public TrxDev
{

public:
    TrxDev_osmosdr(int sampleFreq, int freqCorr);
    ~TrxDev_osmosdr();

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
	osmosdr::source::sptr osmoSdrSource;
	gr::blocks::null_sink::sptr nullSink;
	gr::blocks::multiply_const_cc::sptr multi;
	int sampleRate, centerFreq;

};

#endif 
