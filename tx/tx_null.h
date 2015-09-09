#ifndef TX_NULL_H
#define TX_NULL_H

#include <gnuradio/hier_block2.h>
#include "tx.h"
#include "gnuradio.h"


class Tx_null;

typedef boost::shared_ptr<Tx_null> Tx_null_sptr;
Tx_null_sptr make_tx_null(int sampleRate);

class Tx_null : public Tx 
{

public:
    Tx_null(int);
    ~Tx_null();

private:
	int sampleRate;
	gr::blocks::float_to_complex::sptr f2c;
	gr::blocks::multiply_const_ff::sptr multi;

};

#endif // TX_NULL_H
