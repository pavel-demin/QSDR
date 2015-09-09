#ifndef TX_H
#define TX_H

#include <gnuradio/hier_block2.h>

class Tx;

typedef boost::shared_ptr<Tx> Tx_sptr;


class Tx : public gr::hier_block2
{

public:
    Tx(std::string src_name);
    ~Tx();

};

#endif // RECEIVER_BASE_H
