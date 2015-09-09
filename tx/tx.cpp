#include <gnuradio/io_signature.h>
#include "tx.h"


static const int MIN_IN = 1;  /* Mininum number of input streams. */
static const int MAX_IN = 1;  /* Maximum number of input streams. */
static const int MIN_OUT = 1; /* Minimum number of output streams. */
static const int MAX_OUT = 1; /* Maximum number of output streams. */

Tx::Tx(std::string src_name) : gr::hier_block2 (src_name,
		gr::io_signature::make (MIN_IN, MAX_IN, sizeof(float)),
		gr::io_signature::make (MIN_OUT, MAX_OUT, sizeof(gr_complex)))
{

}

Tx::~Tx()
{

}
