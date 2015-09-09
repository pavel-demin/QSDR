#ifndef RXSINK_SPEAKER_H
#define RXSINK_SPEAKER_H

#include <gnuradio/hier_block2.h>
#include "rxSink.h"
#include "gnuradio.h"


class RxSink_speaker;

typedef boost::shared_ptr<RxSink_speaker> RxSink_speaker_sptr;
RxSink_speaker_sptr make_rx_sink_speaker(char *devName);


class RxSink_speaker : public RxSink
{

public:
    RxSink_speaker(char *name);
    ~RxSink_speaker();

private:
	gr::audio::sink::sptr audioSink;

};

#endif 

