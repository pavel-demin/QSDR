#include "rx_null.h"
#include "config.h"

Rx_null_sptr make_rx_null(int sampleRate) {
	return gnuradio::get_initial_sptr(new Rx_null(sampleRate));
}

Rx_null::Rx_null(int sRate) : Rx("NULL-RX") {
	nullSinkc = gr::blocks::null_sink::make (sizeof(gr_complex));
	nullSource = gr::blocks::null_source::make (sizeof(float));
	throttle = gr::blocks::throttle::make(sizeof(float), AUDIO_RATE);

	connect(self(), 0, nullSinkc, 0);
	connect(nullSource, 0, throttle, 0);
	connect(throttle, 0, self(), 0);
	connect(throttle, 0, self(), 1);
}

Rx_null::~Rx_null()
{

}

void Rx_null::setAGC(bool on, double att, double dec, double ref) {
}

void Rx_null::setFreq(int f) {
}

void Rx_null::setFilter(int filterLo, int filterHi, int filterCut) {
}

void Rx_null::setFilterNotch(int freq, int bw) {
}

void Rx_null::setNotch(bool v) {
}

void Rx_null::setSampleRate(int f) {
}

void Rx_null::setSquelch(int n) {
}

float Rx_null::getSignal() {
	return 0;
}
