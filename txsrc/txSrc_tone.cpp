#include "txSrc_tone.h"
#include "config.h"

TxSrc_tone_sptr make_txsrc_tone(int t1, float v1, int t2, float v2, char* devName) {
	return gnuradio::get_initial_sptr(new TxSrc_tone(t1,v1,t2,v2, devName));
}

TxSrc_tone::TxSrc_tone(int t1, float v1, int t2, float v2, char *devName) : TxSrc("TXSRC-TONE") {

	twoToneTest0 = gr::analog::sig_source_f::make(AUDIO_RATE, gr::analog::GR_SIN_WAVE, t1, v1);
	twoToneTest1 = gr::analog::sig_source_f::make(AUDIO_RATE, gr::analog::GR_SIN_WAVE, t2, v2);
	noiceTest = gr::analog::noise_source_f::make(gr::analog::GR_UNIFORM,0);
	audioSourceMic   = gr::audio::source::make(AUDIO_RATE, devName);
	multi = gr::blocks::multiply_const_ff::make (0);
	adder = gr::blocks::add_ff::make (1);

	twoToneTest0->set_frequency(t1);
	twoToneTest0->set_amplitude(v1);
	twoToneTest1->set_frequency(t2);
	twoToneTest1->set_amplitude(v2);

	connect(twoToneTest0, 0, adder, 0);
	connect(twoToneTest1, 0, adder, 1);
	connect(noiceTest, 0, adder, 3);
	connect(audioSourceMic, 0, multi, 0);
	connect(multi, 0, adder, 2);
	connect(adder, 0, self(), 0);
}

TxSrc_tone::~TxSrc_tone()
{

}


