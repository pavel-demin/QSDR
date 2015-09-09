#ifndef RxSINK_H
#define RxSINK_H

#include <gnuradio/hier_block2.h>


class RxSink;

typedef boost::shared_ptr<RxSink> RxSink_sptr;


class RxSink : public gr::hier_block2
{

public:
    RxSink(std::string sink_name);
    ~RxSink();

};

#endif 
