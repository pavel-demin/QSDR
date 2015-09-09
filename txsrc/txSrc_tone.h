#ifndef TXSRC_TONE_H
#define TXSRC_TONE_H

#include <gnuradio/hier_block2.h>
#include "txSrc.h"
#include "gnuradio.h"

class TxSrc_tone;

typedef boost::shared_ptr<TxSrc_tone> TxSrc_tone_sptr;
TxSrc_tone_sptr make_txsrc_tone(int t1, float v1, int t2, float v2, char *devName);

class TxSrc_tone : public TxSrc 
{

public:
    TxSrc_tone(int t1, float v1, int t2, float v2, char *devName);
    ~TxSrc_tone();


private:
	gr::audio::source::sptr  audioSourceMic;
	gr::blocks::add_ff::sptr adder;
	gr::analog::sig_source_f::sptr twoToneTest0,twoToneTest1;
	gr::blocks::multiply_const_ff::sptr multi; 
	gr::analog::noise_source_f::sptr noiceTest;

};

#endif
