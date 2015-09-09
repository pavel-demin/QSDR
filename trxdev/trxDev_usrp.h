#ifndef TRXDEV_USRP_SOURCE_H
#define TRXDEV_USRP_SOURCE_H

#include <gnuradio/hier_block2.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/uhd/usrp_source.h>
#include <gnuradio/uhd/usrp_sink.h>
#include "trxDev.h"


class TrxDev_usrp;

typedef boost::shared_ptr<TrxDev_usrp> TrxDev_usrp_sptr;

TrxDev_usrp_sptr make_trxdev_usrp(int sampleRate);

class TrxDev_usrp : public TrxDev
{

public:
    TrxDev_usrp(int sampleFreq);
    ~TrxDev_usrp();

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
	gr::uhd::usrp_sink::sptr usrpSink;
	gr::uhd::usrp_source::sptr usrpSource;
	gr::blocks::null_sink::sptr nullSink;
	int sampleRate; 
	qint64 centerFreq;
	int presel;

};

#endif 
