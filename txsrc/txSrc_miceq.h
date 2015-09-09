#ifndef TXSRC_MICEQ_H
#define TXSRC_MICEQ_H

#include <gnuradio/hier_block2.h>
#include "txSrc.h"
#include "gnuradio.h"
#include "gr_limit_ff.h"


class TxSrc_miceq;

typedef boost::shared_ptr<TxSrc_miceq> TxSrc_miceq_sptr;
TxSrc_miceq_sptr make_txsrc_miceq(char *devName);

class TxSrc_miceq : public TxSrc 
{

public:
    TxSrc_miceq(char*);
    ~TxSrc_miceq();


private:
	gr::audio::source::sptr  audioSourceMic;
	gr_limit_ff_sptr limiter; 
	gr::filter::fir_filter_fff::sptr filter_eq_lo, filter_eq_mi, filter_eq_hi;
	gr::blocks::add_ff::sptr adder_eq;
	gr::filter::firdes *firdes;
};

#endif
