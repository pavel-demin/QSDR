#ifndef TXSRC_MIC_H
#define TXSRC_MIC_H

#include <gnuradio/hier_block2.h>
#include "txSrc.h"
#include "gnuradio.h"
#include "gr_limit_ff.h"


class TxSrc_mic;

typedef boost::shared_ptr<TxSrc_mic> TxSrc_mic_sptr;
TxSrc_mic_sptr make_txsrc_mic(char *devName);

class TxSrc_mic : public TxSrc 
{

public:
    TxSrc_mic(char*);
    ~TxSrc_mic();


private:
	gr::audio::source::sptr  audioSourceMic;
	gr_limit_ff_sptr limiter; 

};

#endif
