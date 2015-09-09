#ifndef TXSRC_H
#define TXSRC_H

#include <gnuradio/hier_block2.h>


class TxSrc;

typedef boost::shared_ptr<TxSrc> TxSrc_sptr;


class TxSrc : public gr::hier_block2
{

public:
    TxSrc(std::string src_name);
    ~TxSrc();

};

#endif 
