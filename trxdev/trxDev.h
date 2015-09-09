#ifndef TRXDEV_SOURCE_H
#define TRXDEV_SOURCE_H

#include <gnuradio/hier_block2.h>
#include "settings.h"
#include "gnuradio.h"


class TrxDev;

typedef boost::shared_ptr<TrxDev> TrxDev_sptr;


class TrxDev : public gr::hier_block2
{

public:
    TrxDev(std::string src_name);
    ~TrxDev();

	virtual void setSampleRate(int f)=0;
	virtual void setCenterFreq(qint64 f)=0;
	virtual void setPreamp(int n)=0;
	virtual void setPresel(int n)=0;
	virtual void setAntenne(int)=0;
	virtual int getRXLowFreq()=0;
	virtual int getRXHighFreq()=0;
	virtual void setDCFilter(bool)=0;
	virtual void setFreqCorr(int)=0;
	virtual void setPosition(int sec)=0;
	virtual int getPosition()=0;

	virtual void setPTT(int)=0;
	virtual void setTXPower(int)=0;
	virtual int getTXLowFreq()=0;
	virtual int getTXHighFreq()=0;
	virtual void setTXFreq(qint64)=0;
	virtual void setTXRate(int f)=0;
	virtual void setCWMode(bool)=0;
};

#endif 
