#ifndef TRXDEV_REDPITRCV_SOURCE_H
#define TRXDEV_REDPITRCV_SOURCE_H

#include <gnuradio/hier_block2.h>
#include "gnuradio.h"
#include <redpitaya/rcv_source.h>
#include <redpitaya/trx_source.h>
#include <redpitaya/trx_sink.h>
#include "trxDev.h"

class TrxDev_redpitrcv;

typedef boost::shared_ptr<TrxDev_redpitrcv> TrxDev_redpitrcv_sptr;

TrxDev_redpitrcv_sptr make_trxdev_redpitrcv(int sampleRate, char *ip, int port);

class TrxDev_redpitrcv : public TrxDev
{

public:
    TrxDev_redpitrcv(int sampleFreq, char *ip, int port);
    ~TrxDev_redpitrcv();

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
	gr::blocks::null_sink::sptr redpitrcvSink;
	gr::redpitrcv::source::sptr redpitrcvSource;
	int sampleRate, centerFreq;

};

class TrxDev_redpittrx;

typedef boost::shared_ptr<TrxDev_redpittrx> TrxDev_redpittrx_sptr;

TrxDev_redpittrx_sptr make_trxdev_redpittrx(int sampleRate, char *ip, int rxport, int txport);

class TrxDev_redpittrx : public TrxDev
{

public:
    TrxDev_redpittrx(int sampleFreq, char *ip, int rxport, int txport);
    ~TrxDev_redpittrx();

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
	gr::redpittrx::sink::sptr redpittrxSink;
	gr::redpittrx::source::sptr redpittrxSource;
	int sampleRate, centerFreq;

};

#endif
