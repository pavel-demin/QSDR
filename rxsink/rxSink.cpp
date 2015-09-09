#include <gnuradio/io_signature.h>
#include "rxSink.h"


RxSink::RxSink(std::string src_name) : gr::hier_block2 (src_name,
		gr::io_signature::make (2, 2, sizeof(float)),
		gr::io_signature::make (0,0,0))
{

}

RxSink::~RxSink()
{

}

