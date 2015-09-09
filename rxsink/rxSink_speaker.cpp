#include <gnuradio/io_signature.h>
#include "rxSink_speaker.h"
#include "config.h"

RxSink_speaker_sptr make_rx_sink_speaker(char *devName) {
	return gnuradio::get_initial_sptr(new RxSink_speaker(devName));
}

RxSink_speaker::RxSink_speaker(char *devName) : RxSink("RXSINK-SPEAKER") {

	try {
		audioSink = gr::audio::sink::make  (AUDIO_RATE, devName);
	} catch (...) {
		//qDebug() << "audiodev setting to default";
		audioSink = gr::audio::sink::make (AUDIO_RATE, "default");
	}

	connect(self(), 0, audioSink, 0);
	connect(self(), 1, audioSink, 1);
}

RxSink_speaker::~RxSink_speaker()
{

}

