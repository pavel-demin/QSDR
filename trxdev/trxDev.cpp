#include <gnuradio/io_signature.h>
#include "trxDev.h"


TrxDev::TrxDev(std::string src_name) : gr::hier_block2 (src_name,
		gr::io_signature::make (1, 1, sizeof(gr_complex)),
		gr::io_signature::make (1, 1, sizeof(gr_complex)))
{

}

TrxDev::~TrxDev()
{

}

