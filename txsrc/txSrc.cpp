#include <gnuradio/io_signature.h>
#include "txSrc.h"


TxSrc::TxSrc(std::string src_name) : gr::hier_block2 (src_name,
		gr::io_signature::make (0, 0, 0),
		gr::io_signature::make (1, 1, sizeof(float)))
{

}

TxSrc::~TxSrc()
{

}

