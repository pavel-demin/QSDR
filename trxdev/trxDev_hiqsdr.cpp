#include <gnuradio/io_signature.h>
#include "trxDev_hiqsdr.h"
#include "config.h"
#include "coef976.h"
#include "tx_coef.h"
#include "tab_lsb_3000_2700_300_0_real.h"
#include "tab_lsb_3000_2700_300_0_imag.h"

TrxDev_hiqsdr_sptr make_trxdev_hiqsdr(int sampleRate, const char *ip) {
	return gnuradio::get_initial_sptr(new TrxDev_hiqsdr(sampleRate, ip));
}

TrxDev_hiqsdr::TrxDev_hiqsdr(int sRate, const char* hiqip ) : TrxDev("HIQSDR") {
	dcFilter = true;
	sampleRate = sRate;

	hiqsdrSink = gr::hiqsdr::sink::make (sizeof(gr_complex), hiqip, HIQSDR_TX_PORT); 
	hiqsdrSource = gr::hiqsdr::source::make(sizeof(gr_complex), hiqip, 
			HIQSDR_RX_PORT, HIQSDR_CTL_PORT, HIQSDR_RX_FIR_PORT, HIQSDR_TX_FIR_PORT, 
			1442, false, true); 

	std::vector<gr_complex> v_coef;
//	for (int i=0;i<sizeof(coef)/sizeof(float);i++)
//		v_coef.push_back(coef[i]);
	for (int i=0;i<sizeof(usb_3000_real)/sizeof(float);i++)
		v_coef.push_back(gr_complex(usb_3000_real[i],usb_3000_imag[i]));
//	hiqsdrSource->setFilter(v_coef, FILTER_RX);

	firdes = new gr::filter::firdes();
	//hiqsdrSource->setFilter(firdes->complex_band_pass(1, IF_RATE*8, -3800, -200, 1800), FILTER_RX);
	//hiqsdrSource->setFilter(firdes->complex_band_pass(1, 8000*8, -3000, -200, 120), FILTER_RX);
	
	v_coef.clear();
	for (int i=0;i<sizeof(tx_coef)/sizeof(float);i++)
		v_coef.push_back(tx_coef[i]);
//	hiqsdrSource->setFilter(v_coef, FILTER_TX);
//	hiqsdrSource->setFilter(firdes->complex_band_pass(1, IF_RATE*4, -2000, -1000, 2600), FILTER_TX);

	adder = gr::blocks::add_cc::make(1);
	hiqsdrSource->setSampleRate(sampleRate);
	movingAverage = gr::blocks::moving_average_cc::make (1024, -1/1024.0); 

	connect (hiqsdrSource, 0, movingAverage, 0);
	connect (hiqsdrSource, 0, adder, 0);
	connect (movingAverage, 0, adder, 1);
	connect (adder, 0, self(), 0);
	connect (self(), 0, hiqsdrSink, 0);

	presel = 0;
}

TrxDev_hiqsdr::~TrxDev_hiqsdr()
{

}

void TrxDev_hiqsdr::setSampleRate(int f) {
	sampleRate = f;
#if 1
	if (sampleRate > 960000)
		hiqsdrSource->setFilter(firdes->complex_band_pass(1, IF_RATE*8, -23800, 23800, 2800), FILTER_RX);
	else
		hiqsdrSource->setFilter(firdes->complex_band_pass(1, IF_RATE*8, -23800, 23800, 1400), FILTER_RX);
#endif
	hiqsdrSource->setSampleRate(sampleRate);

}

void TrxDev_hiqsdr::setCenterFreq(qint64 f) {
	centerFreq = f;
	hiqsdrSource->setRXFreq(centerFreq);
}

void TrxDev_hiqsdr::setPreamp(int preamp) {
	if (preamp > 0)
		presel |= HIQ_PREAMP;
	else
		presel &= ~HIQ_PREAMP;
	hiqsdrSource->setAttenuator(-preamp);
	hiqsdrSource->setPresel(presel);
}

void TrxDev_hiqsdr::setPresel(int pre) {
	presel &= 0xf0;
	presel |= (pre & 0x0f);
	hiqsdrSource->setPresel(presel);
}

void TrxDev_hiqsdr::setAntenne(int ant) {
	hiqsdrSource->setAnt(ant == 2);
}

int TrxDev_hiqsdr::getRXLowFreq() {
	return 0;
}

int TrxDev_hiqsdr::getRXHighFreq() {
	return 60;
}

void TrxDev_hiqsdr::setPTT(int on) {
	hiqsdrSource->setPtt(on);
}

void TrxDev_hiqsdr::setTXPower(int txPower) {
	hiqsdrSource->setTXLevel(txPower);
}

int TrxDev_hiqsdr::getTXLowFreq() {
	return 0;
}

int TrxDev_hiqsdr::getTXHighFreq() {
	return 60;
}

void TrxDev_hiqsdr::setTXFreq(qint64 f) {
	hiqsdrSource->setTXFreq(f);
}

void TrxDev_hiqsdr::setTXRate(int f) {
	hiqsdrSource->setTXRate(f);
}

void TrxDev_hiqsdr::setCWMode(bool cw) {
	hiqsdrSource->setCWMode(cw);
}

void TrxDev_hiqsdr::setDCFilter(bool dc) {

	lock();
	if (dcFilter) {
		disconnect (hiqsdrSource, 0, movingAverage, 0);
		disconnect (hiqsdrSource, 0, adder, 0);
		disconnect (movingAverage, 0, adder, 1);
		disconnect (adder, 0, self(), 0);
	} else {
		disconnect (hiqsdrSource, 0, self(), 0);
	}
	dcFilter = dc;
	if (dcFilter) {
		connect (hiqsdrSource, 0, movingAverage, 0);
		connect (hiqsdrSource, 0, adder, 0);
		connect (movingAverage, 0, adder, 1);
		connect (adder, 0, self(), 0);
	} else {
		connect (hiqsdrSource, 0, self(), 0);
	}
	unlock();
}

void TrxDev_hiqsdr::setFreqCorr(int k) {
	qDebug() << "freqcorr"  << k;
	hiqsdrSource->setClockCorr(k);
}
void TrxDev_hiqsdr::setPosition(int pos) {}
int TrxDev_hiqsdr::getPosition() { return 0;}
