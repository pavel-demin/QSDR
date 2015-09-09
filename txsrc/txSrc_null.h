#ifndef TXSRC_NULL_H
#define TXSRC_NULL_H

#include <gnuradio/hier_block2.h>
#include "txSrc.h"
#include "gnuradio.h"
#include "gr_limit_ff.h"


class TxSrc_null;

typedef boost::shared_ptr<TxSrc_null> TxSrc_null_sptr;
TxSrc_null_sptr make_txsrc_null();

class TxSrc_null : public TxSrc 
{

public:
    TxSrc_null();
    ~TxSrc_null();

private:
	gr::blocks::null_source::sptr nullSource;
	gr::blocks::throttle::sptr throttle;

};

#endif
