#ifndef TX_LSB_H
#define TX_LSB_H

#include <gnuradio/hier_block2.h>
#include "tx.h"
#include "gnuradio.h"


class Tx_lsb;

typedef boost::shared_ptr<Tx_lsb> Tx_lsb_sptr;
Tx_lsb_sptr make_tx_lsb(int sampleRate, int fMin, int fMax);

class Tx_lsb : public Tx 
{

public:
    Tx_lsb(int,int,int);
    ~Tx_lsb();

private:
	int sampleRate;
	gr::filter::fir_filter_fcc::sptr firFilter_fcc; 
	gr::filter::firdes *firdes;

};

#endif // TX_LSB_H
