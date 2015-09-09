#ifndef TX_AM_H
#define TX_AM_H

#include <gnuradio/hier_block2.h>
#include "tx.h"
#include "gnuradio.h"


class Tx_am;

typedef boost::shared_ptr<Tx_am> Tx_am_sptr;
Tx_am_sptr make_tx_am(int sampleRate, int fMax);

class Tx_am : public Tx 
{

public:
    Tx_am(int,int);
    ~Tx_am();

private:
	int sampleRate;
	gr::filter::fir_filter_fcc::sptr firFilter_fcc; 
	gr::filter::firdes *firdes;
	gr::blocks::add_const_ff::sptr addConst;

};

#endif // TX_AM_H
