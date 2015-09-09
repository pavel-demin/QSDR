#ifndef TX_FM_H
#define TX_FM_H

#include <gnuradio/hier_block2.h>
#include "tx.h"
#include "gnuradio.h"


class Tx_fm;

typedef boost::shared_ptr<Tx_fm> Tx_fm_sptr;
Tx_fm_sptr make_tx_fm(int sampleRate, int fMax);

class Tx_fm : public Tx 
{

public:
    Tx_fm(int,int);
    ~Tx_fm();

private:
	int sampleRate;
	gr::filter::fir_filter_ccc::sptr firFilter_ccc; 
	gr::filter::firdes *firdes;
	gr::analog::frequency_modulator_fc::sptr fmMod;
	gr::blocks::multiply_const_cc::sptr multi;

};

#endif // TX_FM_H
