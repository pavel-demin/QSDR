#ifndef TX_USB_H
#define TX_USB_H

#include <gnuradio/hier_block2.h>
#include "tx.h"
#include "gnuradio.h"


class Tx_usb;

typedef boost::shared_ptr<Tx_usb> Tx_usb_sptr;
Tx_usb_sptr make_tx_usb(int sampleRate, int fMin, int fMax);

class Tx_usb : public Tx 
{

public:
    Tx_usb(int,int,int);
    ~Tx_usb();

private:
	int sampleRate;
	gr::filter::fir_filter_fcc::sptr firFilter_fcc; 
	gr::filter::firdes *firdes;

};

#endif // TX_USB_H
