#ifndef TRXDEV_FILE_SOURCE_H
#define TRXDEV_FILE_SOURCE_H

#include <gnuradio/hier_block2.h>
#include "gnuradio.h"
#include "gr_sdrfile_source.h"
#include "trxDev.h"

class TrxDev_file;

typedef boost::shared_ptr<TrxDev_file> TrxDev_file_sptr;

TrxDev_file_sptr make_trxdev_file(int sampleRate, char *fileName);

class TrxDev_file : public TrxDev
{

public:
    TrxDev_file(int sampleFreq, char *fileName);
    ~TrxDev_file();

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
	gr::blocks::null_sink::sptr nullSink;
	gr::blocks::throttle::sptr throttle;
	gr_sdrfile_source_sptr fileSource;
	int sampleRate, centerFreq;
	char fName[128];

};

#endif 
