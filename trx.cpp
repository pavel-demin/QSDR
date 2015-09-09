#include <QDir>
#include <gnuradio/realtime.h>
#include "type.h"
#include "trx.h"

#include "rx_null.h"
#include "rx_usb.h"
#include "rx_lsb.h"
#include "rx_cw.h"
#include "rx_wfm_rds.h"
#include "rx_am.h"
#include "rx_fm.h"
#include "rx_psk31.h"
#include "rx_raw.h"

#include "tx_null.h"
#include "tx_usb.h"
#include "tx_lsb.h"
#include "tx_am.h"
#include "tx_fm.h"

#include "txSrc_mic.h"
#include "txSrc_miceq.h"
#include "txSrc_tone.h"
#include "txSrc_null.h"
#include "txSrc_message.h"

#include "rxSink_speaker.h"
#include "rxSink_message.h"

#ifdef OSMOSDR
#include "trxDev_osmosdr.h"
#endif
#ifdef USRP 
#include "trxDev_usrp.h"
#endif
#include "trxDev_hiqsdr.h"
#include "trxDev_file.h"
#include "trxDev_null.h"
#include "trxDev_testsignal.h"

#include "crypt.h"
#include "lib.h"

#define MAX_SHORT       0x8000

Trx::Trx(int port, char *key)  {

	qDebug() << "starting server";


	isRunning = false;
	if (key)
		hashKey = QByteArray(key);

	fftSize	= 2048;
	in  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize*2); 
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize*2); 
	plan = fftw_plan_dft_1d(fftSize*2, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 

	rawSrc = RAW_SRC_FULL;
	raw = false;
	rawAddr = 0;
	sendData = false;

	erxSink = RX_SINK_IP;
	etxSource = TX_SRC_IP;

	guiMsg = new TrxProto::GuiMessage();

	trxServer = new QTcpServer(this);
	if (!trxServer->listen(QHostAddress::Any, port)) {
		qDebug() << "TCP-Server erroro" << trxServer->errorString();
		return;
	}

	connect(trxServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

	rawHiqSocket = new QUdpSocket(this);
	rawHiqSocket->bind(HIQSCOPE_RAW_PORT);
	connect(rawHiqSocket, SIGNAL(readyRead()), this, SLOT(readRaw()));

	// default settings
	recordFileName = "default.raw";
	playFileName   = "default.raw";

	rxSource     = SRC_NULL;
	oldSource    = rxSource; 

	sampleRate = DEFAULT_SAMPLE_RATE;

	trxDev = make_trxdev_null(sampleRate);
	throttle = gr::blocks::throttle::make(sizeof(gr_complex), 1);
	nullSource = gr::blocks::null_source::make (sizeof(gr_complex));

	strcpy(hiqip, "192.168.2.196");
	strcpy(audioDevSpeakerMic, "default");

	txAGCDec = 0.1;
	txAGCAtt = 1.0;
	txAGCRef = 0.5;
	txAGC = true;
	notchOn = false;
	send	= false;
	hiqsdrDC = 0;
	agcVal   = 0;
	ant		= 0;
	centerFreq = 7100000;
	filterHi  = 2800;
	filterLo  = 300;
	filterCut = 500;
	preamp = 0;
	rxFreq	= centerFreq; 
	txFreq	= centerFreq; 
	rxMode = MODE_USB;
	squelchVal=-100;
	filterNotchFreq = 5000;
	filterNotchBW = 200;
	txPower	= 255;
	gain = 20;
	twoToneTest = false;
	record = false;

	txSampleRate = 48000;

	src0Freq = 800;
	src0Ampl = 0.2;
	src1Freq = 1000;
	src1Ampl = 0.2;

	attack_rate_slow = 1;
	decay_rate_slow = 1;
	reference_slow = 0.1;

	attack_rate_fast = 1;
	decay_rate_fast = 0.1;
	reference_fast = 0.1;

	ssbFMax = 2800; 
	ssbFMin = 300;

	recordTX = false;
	hiqsdrFreqCorr = 0;
	osmoFreqCorr = 0;
	dcFilter = true;
	txMute = true;
	smtrCal = 0;

	nullSink = gr::blocks::null_sink::make (sizeof(gr_complex));
	
	window.clear();
	for (int i=0;i<fftSize;i++) 
		window.push_back(0.5  - 0.5  * cos (2 * M_PI / fftSize * i));  // Hanning window
	fft = gr::fft::fft_vcc::make(fftSize, true, window, false); 

	strToVect = gr::blocks::stream_to_vector::make (sizeof(gr_complex), fftSize);
	limiterL = gr_make_limit_ff(0.8);
	limiterR = gr_make_limit_ff(0.8);
	limiterTX = gr_make_limit_ff(0.6);
	rdsMsgq = gr::msg_queue::make();
	fftMsgq = gr::msg_queue::make();
	rxMsgq = gr::msg_queue::make();
	txMsgq = gr::msg_queue::make();
	fftSink = gr::blocks::message_sink::make(fftSize*sizeof(gr_complex), fftMsgq, true);
	keepOneInN = gr::blocks::keep_one_in_n::make(fftSize*sizeof(gr_complex), 1);
	keepOneInN->set_n(100/20);

	rxSink = make_rx_sink_message(rxMsgq);

	rxAdderL = gr::blocks::add_ff::make (1);
	rxAdderR = gr::blocks::add_ff::make (1);
	multiL = gr::blocks::multiply_const_ff::make (gain);
	multiR = gr::blocks::multiply_const_ff::make (gain);
	multiTX = gr::blocks::multiply_const_ff::make(1);
	agc = gr::analog::agc2_ff::make (txAGCDec, txAGCAtt, txAGCRef);

	vfo = 0;

	fftTimer = new QTimer(this);
	timer = new QTimer(this);
	tbTimer = new QTimer(this);

	txSrcNull = make_txsrc_null();
	txSrc = make_txsrc_message(txMsgq);

	tb = gr::make_top_block("qsdr");
	tb->connect(nullSource, 0, throttle, 0);
	tb->connect(throttle, 0, trxDev, 0);

	for (int i=0;i<MAX_VFO;i++) {
		activ[i] = false;
		rx[i] = make_rx_null(sampleRate); 
		tx[i] = make_tx_null(1); 
		tb->connect(trxDev, 0, rx[i], 0);
		tb->connect(rx[i], 0, rxAdderL, i);
		tb->connect(rx[i], 1, rxAdderR, i);
	}
	tb->connect(rxAdderL, 0, multiL, 0);
	tb->connect(rxAdderR, 0, multiR, 0);
	tb->connect(multiL, 0, limiterL, 0);
	tb->connect(multiR, 0, limiterR, 0);
	tb->connect(limiterL, 0, rxSink, 0);
	tb->connect(limiterR, 0, rxSink, 1);
	
	tb->connect(trxDev, 0, strToVect, 0);
	tb->connect(strToVect, 0, keepOneInN, 0);
	tb->connect(keepOneInN, 0, fft, 0);
	tb->connect(fft, 0, fftSink, 0);

	gr::rt_status_t rt = gr::enable_realtime_scheduling();
	if (rt != gr::RT_OK)
		qDebug() << "realtime error";

	setGain(gain);
	setSampleRate(sampleRate);
	setTX(false);

	setAGC(agcVal);
	setTXPower(txPower);
	setFilterNotch(4000);

	keepAliveTimer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	connect(fftTimer, SIGNAL(timeout()), this, SLOT(ffttimeout()));
	connect(keepAliveTimer, SIGNAL(timeout()), this, SLOT(keepAliveTimeout()));

	initial = true;
	timer->start(50);		// 50ms
	fftTime = 100;
	fftTimer->start(fftTime);
}

Trx::~Trx() {
	tb->lock();
	tb->disconnect_all();
	rawHiqSocket->close();
	trxServer->close();
	delete rawHiqSocket;
	delete trxServer;
	delete guiMsg;

	fftw_destroy_plan(plan);
	fftw_free(in); fftw_free(out);
}

void Trx::selectVFO(int v) {
	vfo = v;
}

void Trx::setActiv(bool v) {
	activ[vfo] = v;
	setMode(rxMode);
}

void Trx::setRxFreq(qint64 f) {
	PDEBUG(MSG1, "TRX: rx    :%lli",f);
	rxFreq = f;
	if (rxFreq - centerFreq > sampleRate/2) 
		setCenterFreq(f);
	if (rxFreq - centerFreq < -sampleRate/2) 
		setCenterFreq(f);
	rx[vfo]->setFreq(rxFreq - centerFreq);
}

void Trx::setTxFreq(qint64 f) {
	PDEBUG(MSG1, "TRX: tx :%lli",f);
	txFreq = f;
}

void Trx::setCenterFreq(qint64 f) {
	PDEBUG(MSG1, "TRX: center:%lli",f);
	centerFreq = f;
	trxDev->setCenterFreq(centerFreq);
}

void Trx::setGain(int v) {
	CHECKRANGE(v,0,1000);
	PDEBUG(MSG1, "TRX: vol:%i",v);
	int inv_gain = 90;
	gain = v;
	if (agcVal == 0)
		inv_gain = 50;
	multiL->set_k(exp(-10+(float)(gain)/inv_gain));
	multiR->set_k(exp(-10+(float)(gain)/inv_gain));
}

void Trx::setSquelch(int v) {
	PDEBUG(MSG1, "TRX: sql:%i",v);
	squelchVal = v;
	rx[vfo]->setSquelch(squelchVal);
}

void Trx::setTxSource(ETxSource src) {
	PDEBUG(MSG1, "TRX: setTxSource:%i",src);
	stop();

	if (etxSource == src)
		return;
	
	etxSource = src;
	if (send) {
		tb->disconnect(txSrc, 0, multiTX, 0);
	}
	txSrc.reset();
	switch (etxSource) {
		case TX_SRC_LOCALAUDIO:
			txSrc = make_txsrc_mic(audioDevSpeakerMic);
			break;
		case TX_SRC_IP:
			txSrc = make_txsrc_message(txMsgq);
			break;
		case TX_SRC_TWOTONE:
			txSrc = make_txsrc_tone(src0Freq, src0Ampl, src1Freq, src1Ampl, audioDevSpeakerMic);
			break;
	}
	if (send) {
		tb->connect(txSrc, 0, multiTX, 0);
	}
}

void Trx::setRxSink(ERxSink sink) {
	PDEBUG(MSG1, "TRX: setRxSink:%i",sink);
	if (erxSink == sink)
		return;

	stop();

	erxSink = sink;
	tb->disconnect(limiterL, 0, rxSink, 0);
	tb->disconnect(limiterR, 0, rxSink, 1);
	rxSink.reset();
	switch (erxSink) {
		case RX_SINK_LOCALAUDIO:
			rxSink = make_rx_sink_speaker(audioDevSpeakerMic);
			break;
		case RX_SINK_IP:
			rxSink = make_rx_sink_message(rxMsgq);
			break;
	}
	tb->connect(limiterL, 0, rxSink, 0);
	tb->connect(limiterR, 0, rxSink, 1);
}

void Trx::setMode(int m) {
	PDEBUG(MSG1, "TRX: mode %i",m);
	stop();
	rxMode = m;

	trxDev->setCWMode(rxMode == MODE_CW);

	tb->disconnect(trxDev, 0, rx[vfo], 0);
	tb->disconnect(rx[vfo], 0, rxAdderL, vfo);
	tb->disconnect(rx[vfo], 1, rxAdderR, vfo);
	rx[vfo].reset();
	tx[vfo].reset();
	if (!activ[vfo]) {
		rx[vfo] = make_rx_null(sampleRate); 
		tx[vfo] = make_tx_null(1); 
	} else {
		switch (rxMode) {
			case MODE_USB:
				rx[vfo] = make_rx_usb(sampleRate); 
				tx[vfo] = make_tx_usb(txSampleRate, ssbFMin, ssbFMax); 
				break;
			case MODE_LSB:
				rx[vfo] = make_rx_lsb(sampleRate); 
				tx[vfo] = make_tx_lsb(txSampleRate, ssbFMin, ssbFMax); 
				break;
			case MODE_AM:
				rx[vfo] = make_rx_am(sampleRate); 
				tx[vfo] = make_tx_am(txSampleRate, ssbFMax); 
				break;
			case MODE_NFM:
				rx[vfo] = make_rx_fm(sampleRate); 
				tx[vfo] = make_tx_fm(txSampleRate, ssbFMax); 
				break;
			case MODE_CW:
				rx[vfo] = make_rx_cw(sampleRate); 
				tx[vfo] = make_tx_null(txSampleRate); 
				break;
			case MODE_WFM_RDS:
				rx[vfo] = make_rx_wfmrds(sampleRate, rdsMsgq); 
				tx[vfo] = make_tx_null(txSampleRate); 
				break;
			case MODE_RAW:
				rx[vfo] = make_rx_raw(sampleRate); 
				tx[vfo] = make_tx_null(txSampleRate); 
				break;
			default:
				rx[vfo] = make_rx_null(sampleRate); 
				tx[vfo] = make_tx_null(txSampleRate); 
				break;

		}
	}
	tb->connect(trxDev, 0, rx[vfo], 0);
	tb->connect(rx[vfo], 0, rxAdderL, vfo);
	tb->connect(rx[vfo], 1, rxAdderR, vfo);

	restoreRX();
}

void Trx::restoreRX() {
	PDEBUG(MSG2, "TRX: restore rx");
	stop();
	trxDev->setCenterFreq(centerFreq);
	rx[vfo]->setFreq(rxFreq - centerFreq);
	rx[vfo]->setFilter(filterLo, filterHi, filterCut);
	rx[vfo]->setNotch(notchOn);
	rx[vfo]->setFilterNotch(filterNotchFreq, filterNotchBW);
	rx[vfo]->setSquelch(squelchVal);
	setPreamp(preamp);
	switch(agcVal) {
		case 0:
			rx[vfo]->setAGC(false);
			break;
		case 1:
			rx[vfo]->setAGC(true, attack_rate_slow, decay_rate_slow, reference_slow);
			break;
		case 2:
			rx[vfo]->setAGC(true, attack_rate_fast, decay_rate_fast, reference_fast);
			break;
	}
}

void Trx::setFilter(int l, int h, int c) {
	PDEBUG(MSG1, "TRX: filter %i %i %i",l,h,c);
	if (c<200)
		c=200;
	filterLo = l;
	filterHi = h;
	filterCut = c;
	setFilter();
}

void Trx::setFilterNotchBW(int v) {
	PDEBUG(MSG1, "TRX: notch bw: %i",v);
	filterNotchBW = v;
	rx[vfo]->setFilterNotch(filterNotchFreq, filterNotchBW);
}

void Trx::setFilterNotch(int v) {
	PDEBUG(MSG1, "TRX: notch: %i",v);
	filterNotchFreq = v;
	rx[vfo]->setFilterNotch(filterNotchFreq, filterNotchBW);
}

void Trx::setTXPower(int v) {
	PDEBUG(MSG1, "TRX: tx-po: %i",v);
	txPower = v;
	trxDev->setTXPower(txPower);
}

void Trx::setFilter() {
	PDEBUG(MSG2, "TRX: setFilter");
	if (filterCut<=0) {
		assert(0);
		return;
	}
	rx[vfo]->setFilter(filterLo, filterHi, filterCut);
}

void Trx::setAGC(int v) {
	stop();
	PDEBUG(MSG1, "TRX: agc  : %i",v);
	if (v >= 0)
		agcVal = v;
	switch(agcVal) {
		case 0:
			rx[vfo]->setAGC(false);
			break;
		case 1:
			rx[vfo]->setAGC(true, attack_rate_slow, decay_rate_slow, reference_slow);
			break;
		case 2:
			rx[vfo]->setAGC(true, attack_rate_fast, decay_rate_fast, reference_fast);
			break;
	}
}

void Trx::setReadRaw(bool on) {
	if (rxSource != SRC_HIQSDR)
		return;
	raw = on;
	setFFTSize(fftSize);

	if (raw) {
		switch (rawSrc) {
			case RAW_SRC_FULL:
				rawHiqSocket->writeDatagram("bb", 2, QHostAddress(hiqip), HIQSDR_RAW_PORT);
				break;
			case RAW_SRC_CIC_A:
				rawHiqSocket->writeDatagram("cc", 2, QHostAddress(hiqip), HIQSDR_RAW_PORT);
				break;
			case RAW_SRC_CIC_B:
				rawHiqSocket->writeDatagram("dd", 2, QHostAddress(hiqip), HIQSDR_RAW_PORT);
				break;
		}
		QTimer::singleShot(1, this, SLOT(readRaw()));
	}
}

void Trx::setSampleRate(int f) {
	PDEBUG(MSG1, "TRX: sample  : %i",f);
	stop();
	sampleRate = f;
	trxDev->setSampleRate(sampleRate);
	for (int i=0;i<MAX_VFO;i++)
		rx[i]->setSampleRate(sampleRate);
	restoreRX();
}

void Trx::setPreamp(int pre) {
	preamp = pre;
	PDEBUG(MSG1, "TRX: preamp  : %i",preamp);
	trxDev->setPreamp(preamp);
}

void Trx::setPresel(int pre) {
	PDEBUG(MSG1, "TRX: presel  : %i",pre);
	trxDev->setPresel(pre);
}

void Trx::setRecord(bool t) {
	tb->lock();
	record = t;
	if (record) {
		fileSink = gr::blocks::file_sink::make (sizeof(gr_complex), recordFileName.toAscii().data());
		tb->connect(trxDev, 0, fileSink, 0);
	} else {
		tb->disconnect(trxDev, 0, fileSink, 0);
		fileSink.reset();
	}
	tb->unlock();
}

void Trx::setTRXDev(RxSrc src) {
	PDEBUG(MSG1, "TRX: setTRXDev  : %i -> %i",rxSource,src);
	if ((src == rxSource) && (src != SRC_FILE))
		return;
	tb->lock();
	for (int i=0;i<MAX_VFO;i++)
		tb->disconnect(trxDev, 0, rx[i], 0);
	tb->disconnect(trxDev, 0, strToVect, 0);
	tb->disconnect(nullSource, 0, throttle, 0);
	tb->disconnect(throttle, 0, trxDev, 0);
	trxDev.reset();
	try {
	switch (src) {
		case SRC_HIQSDR:
			trxDev = make_trxdev_hiqsdr(sampleRate, hiqip);
			trxDev->setFreqCorr(hiqsdrFreqCorr);
			break;
		case SRC_FILE: 
			trxDev = make_trxdev_file(sampleRate, playFileName.toAscii().data());
			guiMsg->set_playfilelen(QFileInfo(playFileName).size()/sampleRate/sizeof(gr_complex));
			break;
#ifdef OSMOSDR
		case SRC_OSMOSDR:
			trxDev = make_trxdev_osmosdr(sampleRate, osmoFreqCorr );
			break;
#endif
#ifdef USRP
		case SRC_USRP:
			trxDev = make_trxdev_usrp(sampleRate);
			break;
#endif
		case SRC_TESTSIGNAL:
			trxDev = make_trxdev_testsignal(sampleRate);
			break;
		default:
			trxDev = make_trxdev_null(sampleRate);
	}
	} catch (...) {
		qDebug() << "error creating trx: using testsignal device";
		trxDev = make_trxdev_testsignal(sampleRate);
	}
	tb->connect(nullSource, 0, throttle, 0);
	tb->connect(throttle, 0, trxDev, 0);

	for (int i=0;i<MAX_VFO;i++)
		tb->connect(trxDev, 0, rx[i], 0);
	tb->connect(trxDev, 0, strToVect, 0);
	rxSource = src;
	tb->unlock();
	restoreRX();
	trxDev->setDCFilter(dcFilter);
}

void Trx::playFile(bool t) {
	PDEBUG(MSG1, "TRX: playFile: %i",t);
	if (t) {
		if (rxSource != SRC_FILE)
			oldSource = rxSource;
		setTRXDev(SRC_FILE);
	} else {
		if (oldSource != SRC_NULL)
			setTRXDev(oldSource);
	}
}

void Trx::setFFTSize(int n) {
	PDEBUG(MSG1, "TRX: fftSize: %i",n);

	fftSize = n;
	window.clear();

	if ((rawSrc == RAW_SRC_FULL) && raw) {
		fftw_destroy_plan(plan);
		fftw_free(in); 
		fftw_free(out);
		in  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize*2); 
		out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize*2); 
		plan = fftw_plan_dft_1d(fftSize*2, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 	
		for (int i=0;i<fftSize*2;i++) 
			window.push_back(0.5  - 0.5  * cos (2 * M_PI / (fftSize*2) * i));  // Hanning window
	} else {
		// reconfigure fftSize
		tb->lock();
		tb->disconnect(trxDev, 0, strToVect, 0);
		tb->disconnect(strToVect, 0, keepOneInN, 0);
		tb->disconnect(keepOneInN, 0, fft, 0);
		tb->disconnect(fft, 0, fftSink, 0);

		for (int i=0;i<fftSize;i++) 
			window.push_back(0.5  - 0.5  * cos (2 * M_PI / fftSize * i));  // Hanning window

		fft.reset();
		fft = gr::fft::fft_vcc::make(fftSize, true, window, false); 
		strToVect.reset();
		strToVect = gr::blocks::stream_to_vector::make (sizeof(gr_complex), fftSize);
		fftSink.reset();
		fftSink = gr::blocks::message_sink::make(fftSize*sizeof(gr_complex), fftMsgq, true);
		keepOneInN.reset();
		keepOneInN = gr::blocks::keep_one_in_n::make(fftSize*sizeof(gr_complex), 1);
		keepOneInN->set_n(fftTime/20);

		tb->connect(trxDev, 0, strToVect, 0);
		tb->connect(strToVect, 0, keepOneInN, 0);
		tb->connect(keepOneInN, 0, fft, 0);
		tb->connect(fft, 0, fftSink, 0);


		tb->unlock();
	}
}

void Trx::setTX(bool on) {
	static int rxSampleRate;
	PDEBUG(MSG1, "TRX: tx: %i power %i %i",on,txPower, txFreq);
	stop();
	trxDev->setTXFreq(txFreq);
	trxDev->setTXPower(txPower);
	//trxDev->setTXRate(txSampleRate);
	setGain(gain);

	trxDev->setPTT(false);
	if (send) {
		if (monitor==0) {
			tb->disconnect(tx[vfo], 0, rx[vfo], 0);
			tb->connect(trxDev, 0, rx[vfo], 0);
			tb->disconnect(trxDev, 0, nullSink, 0);
			tb->disconnect(tx[vfo], 0, strToVect, 0);
			tb->connect(trxDev, 0, strToVect, 0);
			setSampleRate(rxSampleRate);
		}
		if (txAGC) {
			tb->disconnect(txSrc, 0, agc, 0);
			tb->disconnect(agc, 0, multiTX, 0);
		} else {
			tb->disconnect(txSrc, 0, multiTX, 0);
		}
		tb->disconnect(multiTX, 0, limiterTX, 0);
		tb->disconnect(limiterTX, 0, tx[vfo], 0);
		tb->disconnect(tx[vfo], 0, trxDev, 0);

		tb->connect(nullSource, 0, throttle, 0);
		tb->connect(throttle, 0, trxDev, 0);
	}
	send = on;
	if (send) {
		if (monitor==0) {
			rxSampleRate = sampleRate;
			setSampleRate(txSampleRate);
			tb->disconnect(trxDev, 0, rx[vfo], 0);
			tb->connect(trxDev, 0, nullSink, 0);
			tb->connect(tx[vfo], 0, rx[vfo], 0);
			tb->disconnect(trxDev, 0, strToVect, 0);
			tb->connect(tx[vfo], 0, strToVect, 0);
		}
		tb->disconnect(nullSource, 0, throttle, 0);
		tb->disconnect(throttle, 0, trxDev, 0);

		if (txAGC) {
			agc.reset();
			agc = gr::analog::agc2_ff::make (txAGCDec, txAGCAtt, txAGCRef);
			tb->connect(txSrc, 0, agc, 0);
			tb->connect(agc, 0, multiTX, 0);
		} else {
			tb->connect(txSrc, 0, multiTX, 0);
		}
		tb->connect(multiTX, 0, limiterTX, 0);
		tb->connect(limiterTX, 0, tx[vfo], 0);
		tb->connect(tx[vfo], 0, trxDev, 0);
	}
	if (send) {
		if (txMute) {
			multiL->set_k(0);
			multiR->set_k(0);
		}
		trxDev->setPTT(true);
	} 
}

void Trx::setAntenne(int v) {
	PDEBUG(MSG1, "TRX: ant: %i",v);
	ant = v;
	trxDev->setAntenne(ant);
}

void Trx::setNotch(bool v) {
	PDEBUG(MSG1, "TRX: notch: %i",v);
	stop();
	notchOn = v;
	rx[vfo]->setNotch(notchOn);
}

float Trx::getSmeter() {
	return rx[vfo]->getSignal() + smtrCal;
}

void Trx::settingsChanged(QString settings) {
	QStringList sList = settings.split("\n");
	QStringListIterator it(sList);
	PDEBUG(MSG1, "TRX: settings changed");
	while (it.hasNext()) {
		QStringList entry = it.next().split("\t");
		if (entry.count()<2)
			continue;

		QString st  = entry.at(0);
		QString val = entry.at(1);

		if (st == "Trx,TX,limiter")
			limiterTX->set_k(val.toDouble());
		if (st == "Trx,TX,gain")
			multiTX->set_k(val.toDouble());

		if (st == "Trx,TX,TwoToneTest,1,freq") src0Freq = val.toInt();
		if (st == "Trx,TX,TwoToneTest,1,ampl") src0Ampl = val.toDouble();
		if (st == "Trx,TX,TwoToneTest,2,freq") src1Freq = val.toInt();
		if (st == "Trx,TX,TwoToneTest,2,ampl") src1Ampl = val.toDouble();

		if (st == "Trx,RX,agc_slow,attack_rate") attack_rate_slow = val.toDouble();
		if (st == "Trx,RX,agc_slow,decay_rate") decay_rate_slow = val.toDouble();
		if (st == "Trx,RX,agc_slow,reference") reference_slow = val.toDouble();
		if (st == "Trx,RX,agc_fast,attack_rate") attack_rate_fast = val.toDouble();
		if (st == "Trx,RX,agc_fast,decay_rate") decay_rate_fast = val.toDouble();
		if (st == "Trx,RX,agc_fast,reference") reference_fast = val.toDouble();
		if (st == "Trx,RX,limiter") {
			limiterL->set_k(val.toDouble());
			limiterR->set_k(val.toDouble());
		}

		if (st == "Trx,TX,SSB,fMax") {
			if (ssbFMax != val.toDouble()) {
				ssbFMax = val.toDouble();
				setMode(rxMode);
			}
		}
		if (st == "Trx,TX,SSB,fMin") {
			if (ssbFMin != val.toDouble()) {
				ssbFMin = val.toDouble();
				setMode(rxMode);
			}
		}

		if (st == "Trx,TX,record") recordTX = (val == "on");
		if (st == "Trx,TX,monitor") monitor = val.toInt();
		if (st == "Trx,TX,sampleRate") txSampleRate = val.toInt();

		if (st ==  "Device,OSMOSDR,freqCorr") { 
			osmoFreqCorr = val.toInt();
			if (rxSource == SRC_OSMOSDR)
				trxDev->setFreqCorr(hiqsdrFreqCorr);
		}
		if (st ==  "Device,HiQSDR,freqCorr") { 
			hiqsdrFreqCorr = val.toInt();
			if (rxSource == SRC_HIQSDR)
				trxDev->setFreqCorr(hiqsdrFreqCorr);
		qDebug() << "freqCorr" << hiqsdrFreqCorr;
		}

		if (st == "Trx,RX,dcFilter") dcFilter = (val == "on");

		if (st == "Trx,TX,mute") txMute = (val == "on");
		if (st == "Config,SMeterCal") smtrCal = val.toDouble();

		if (st == "Device,HiQSDR,IP") {
			if (QString(hiqip) != val) {
				strcpy(hiqip,val.toAscii().data());
				setTRXDev(SRC_NULL);
				setTRXDev(rxSource);
			}
		}

		if (st == "Trx,TX,agc,activ") txAGC = (val == "on");
		if (st == "Trx,TX,agc,attack_rate") txAGCAtt = val.toDouble();
		if (st == "Trx,TX,agc,decay_rate") txAGCDec = val.toDouble();
		if (st == "Trx,TX,agc,reference") txAGCRef = val.toDouble();

		if (st == "Device,AudioDev,Gnuradio") {
			QStringList audioDevList = getAlsaAudioDevices();
			int idx = val.toInt()+1;
			if (idx >= audioDevList.size())
				idx=0;
			strcpy(audioDevSpeakerMic, audioDevList[idx].replace("_",",").toAscii().data());
		}

		if (st == "Device,Device") setTRXDev((RxSrc)val.toInt());
		if (st == "Trx,TX,Source") setTxSource((ETxSource)val.toInt());
		if (st == "Trx,RX,Sink") setRxSink((ERxSink)val.toInt());
		if (st == "Display,fftRate_ms") fftTime = val.toInt();
	}

	fftTimer->start(fftTime);
	setAGC(agcVal);
	trxDev->setDCFilter(dcFilter);


#if 0

	enableEQ = sdrSettings->getBoolVal("Trx,TX,Equalizer,Enable");
	double v1 =  sdrSettings->getDoubleVal("Trx,TX,Equalizer,Low,val");
	int f1 =  sdrSettings->getIntVal("Trx,TX,Equalizer,Low,freq");
	double v2 =  sdrSettings->getDoubleVal("Trx,TX,Equalizer,Mid,val");
	int f2 =  sdrSettings->getIntVal("Trx,TX,Equalizer,Mid,freq");
	double v3 =  sdrSettings->getDoubleVal("Trx,TX,Equalizer,High,val");
	int f3 =  sdrSettings->getIntVal("Trx,TX,Equalizer,High,freq");
	int f4 =  sdrSettings->getIntVal("Trx,TX,Equalizer,High,maxfreq");

	if (enableEQ) {
		filter_eq_lo.reset();
		filter_eq_lo =  gr_make_fir_filter_fff (1, firdes->band_pass(v1, AUDIO_RATE, f1,f2,300));
		filter_eq_mi.reset();
		filter_eq_mi =  gr_make_fir_filter_fff (1, firdes->band_pass(v2, AUDIO_RATE, f2,f3,300));
		filter_eq_hi.reset();
		filter_eq_hi =  gr_make_fir_filter_fff (1, firdes->band_pass(v3, AUDIO_RATE, f3,f4,300));
	}

#endif
}

QString Trx::getInfos() {
	char buf[128];
		
	gr::message::sptr msg;
	msg = rdsMsgq->delete_head_nowait();
	if (msg!=0) {
		//if ((msg->type() == 4) || (msg->type() == 1)) {
			memset(buf,0,sizeof(buf));
			strncpy (buf, (char*)msg->msg(), msg->length());
			return QString("%1: %2\n").arg(msg->type()).arg(buf);
		//}
	}
	return QString();
}

void Trx::readGuiData() {
	while (trxSocket->bytesAvailable()) {
		datagram += trxSocket->readAll();
		//qDebug() << "TRX: got: " << datagram.size();
		//if (HashSHA256(datagram.mid(SHA256_DIGEST_SIZE+1)+hashKey) != QByteArray(datagram.mid(1,SHA256_DIGEST_SIZE))) {
		//	qDebug() << "packet error";
		//	continue;
		//}
		while(datagram.size()>0) {
			if (datagram.left(2) != "Hq") {
				qDebug() << "trx: sync error";
				datagram.clear();
				return;
			}
			int len = (uint8)datagram.at(2) + (uint8)datagram.at(3) * 256;
			if (datagram.size()-4 < len) {
				//qDebug() << "trx: packet error";
				break;
			}

			TrxProto::TrxMessage trxMsg;
			if (trxMsg.ParseFromArray(datagram.mid(4).data(), len)) {

				if (trxMsg.has_audio()) {
					if (!txMsgq->full_p()) {
						gr::message::sptr msg  = gr::message::make_from_string(trxMsg.audio(),0,0,0);
						txMsgq->insert_tail(msg);
						msg.reset();
					}
				}
				if (trxMsg.has_vfo())
					selectVFO(trxMsg.vfo());
				if (trxMsg.has_vfoactiv())
					setActiv(trxMsg.vfoactiv());
				if (trxMsg.has_rxcenterfreq())
					setCenterFreq(trxMsg.rxcenterfreq());
				if (trxMsg.has_rxfreq())
					setRxFreq(trxMsg.rxfreq());
				if (trxMsg.has_agc())
					setAGC(trxMsg.agc());
				if (trxMsg.has_antenne())
					setAntenne(trxMsg.antenne());
				if (trxMsg.has_fftsize())
					setFFTSize(trxMsg.fftsize());
				if (trxMsg.has_filterlo()) {
					filterLo = trxMsg.filterlo();
					setFilter();
				}
				if (trxMsg.has_filterhi()) {
					filterHi = trxMsg.filterhi();
					setFilter();
				}
				if (trxMsg.has_filtercut()) {
					filterCut = trxMsg.filtercut();
					setFilter();
				}
				if (trxMsg.has_filternotchbw())
					setFilterNotchBW(trxMsg.filternotchbw());
				if (trxMsg.has_filternotchfreq())
					setFilterNotch(trxMsg.filternotchfreq());
				if (trxMsg.has_notchactiv())
					setNotch(trxMsg.notchactiv());
				if (trxMsg.has_preamp())
					setPreamp(trxMsg.preamp());
				if (trxMsg.has_mode())
					setMode(trxMsg.mode());
				if (trxMsg.has_samplerate())
					setSampleRate(trxMsg.samplerate());
				if (trxMsg.has_squelchlevel())
					setSquelch(trxMsg.squelchlevel());
				if (trxMsg.has_txfreq())
					setTxFreq(trxMsg.txfreq());
				if (trxMsg.has_txpower())
					setTXPower(trxMsg.txpower());
				if (trxMsg.has_presel())
					setPresel(trxMsg.presel());
				if (trxMsg.has_gain())
					setGain(trxMsg.gain());
				if (trxMsg.has_ptt())
					setTX(trxMsg.ptt());
				if (trxMsg.has_settings())
					settingsChanged(QByteArray(trxMsg.settings().data()));
				if (trxMsg.has_playfilename()) 
					playFileName = QString(trxMsg.playfilename().c_str());
				if (trxMsg.has_recordfilename())
					recordFileName = QString(trxMsg.recordfilename().c_str());
				if (trxMsg.has_recordfile())
					setRecord(trxMsg.recordfile());
				if (trxMsg.has_playfile())
					playFile(trxMsg.playfile());
				if (trxMsg.has_keepalive()) {
					if (trxMsg.keepalive()) {
						if (initial)
							guiMsg->set_restartind(true);
						initial = false;
						keepAliveTimer->start(1000);
						if (!sendData)
							qDebug() << "connect";
						sendData = true;
					} else {
						sendData = false;
						qDebug() << "disconnecting";
					}
				}
				if (trxMsg.has_readraw())
					setReadRaw(trxMsg.readraw());
				if (trxMsg.has_getfilelist())
					sendFileList();
				if (trxMsg.has_filepos()) {
					trxDev->setPosition(trxMsg.filepos());
					qDebug() << trxMsg.filepos();
				}
				datagram = datagram.mid(len+4);
			} else {
				qDebug() << "trx: Failed to parse." << datagram.size() << len ;
				datagram.clear();
				return;
			}
		}
	}
}

void Trx::sendGuiMsg() {
	uint8 header[4];

	if (guiMsg->ByteSize() == 0) 
		return;
	if (sendData) { 
		std::ostringstream out;
		guiMsg->SerializeToOstream(&out);
		QByteArray byteArray(out.str().c_str(), out.str().size());

		strcpy((char*)header, "Hq");
		header[2] = byteArray.size() & 0xff;
		header[3] = (byteArray.size() >> 8) & 0xff;

		//qDebug() << "wr:" << byteArray.size();
		trxSocket->write(QByteArray((const char*)header, sizeof(header)) + byteArray);
	}
	guiMsg->Clear();
}

void Trx::ffttimeout() {
   QMutexLocker locker(&mutex);
	unsigned char buf[32*1024];
	gr::message::sptr msgf;
	msgf = fftMsgq->delete_head_nowait();
	if (!raw && msgf) {
		if (msgf->length() >= fftSize*sizeof(gr_complex)) {
			const gr_complex *in = (const gr_complex *) msgf->msg();
			for (int i = 0; i < fftSize; i++){
				const float x = in[i].real();
				const float y = in[i].imag();
				int v =	log10(sqrt(x*x + y*y)/fftSize)*20*2+FFT_OFFSET;
				CHECKRANGE(v,0,255)
				buf[i<fftSize/2 ? (int)(i+fftSize/2) : (int)(i-fftSize/2)] = v;
			}
			guiMsg->set_fft(buf,fftSize);
			//sendGuiMsg();
			fftMsgq->flush();
		}
	}
	if (raw && rawAddr==0) {
		switch (rawSrc) {
			case RAW_SRC_FULL:
				rawHiqSocket->writeDatagram("bb", 2, QHostAddress(hiqip), HIQSDR_RAW_PORT);
				break;
			case RAW_SRC_CIC_A:
				rawHiqSocket->writeDatagram("cc", 2, QHostAddress(hiqip), HIQSDR_RAW_PORT);
				break;
			case RAW_SRC_CIC_B:
				rawHiqSocket->writeDatagram("dd", 2, QHostAddress(hiqip), HIQSDR_RAW_PORT);
				break;
		}
		QTimer::singleShot(1, this, SLOT(readRaw()));
	}
}

void Trx::timeout() {
	QMutexLocker locker(&mutex);
	int pos=0;
	unsigned char buf[32*1024];

	guiMsg->set_rssi(getSmeter());

	gr::message::sptr msga;
	msga = rxMsgq->delete_head_nowait();
	while (msga) {
		const char *in = (const char*) msga->msg();
		for (int i = 0; i<min((int)msga->length()/sizeof(char),(int)sizeof(buf)); i++){
			buf[pos++] = in[i];
			if ((unsigned int)pos>=sizeof(buf))
				break;
		}
		if ((unsigned int)pos>=sizeof(buf))
			break;
		msga = rxMsgq->delete_head_nowait();
	}
	if (rxMsgq->count()>10) 
		rxMsgq->flush();
	if (pos>0)
		guiMsg->set_audio(buf, pos);

	gr::message::sptr msg;
	msg = rdsMsgq->delete_head_nowait();
	if (msg!=0) 
		guiMsg->set_displaytext((void*)msg->msg(), msg->length());

	if (rxSource == SRC_FILE) { 
		guiMsg->set_playfilepos(trxDev->getPosition());
	}

	sendGuiMsg();

}

void Trx::keepAliveTimeout() {
	QMutexLocker locker(&mutex);
	keepAliveTimer->stop();
	sendData = false;
	trxDev->setPTT(false);
	qDebug() << "timeout, stop transmitting";
}

void Trx::readRaw() {
	char addr[2];
	int i,ret;
	QByteArray datagram;
	QHostAddress sender;
	quint16 senderPort;
	unsigned char buf[MAX_FFT];

	while (rawHiqSocket->hasPendingDatagrams()>0) {
		datagram.resize(rawHiqSocket->pendingDatagramSize());

		ret = rawHiqSocket->readDatagram(datagram.data(), datagram.size(),
				&sender, &senderPort);
		if (!(datagram.data()[1] & 1)) {
			PDEBUG(ERR1,"RAW: not ready");
			QTimer::singleShot(1, this, SLOT(readRaw()));
			return;
		};
		rawData.append(datagram.mid(2));
		if (ret != 1442)
			PDEBUG(ERR1, "RAW: error read");

		rawAddr += (ret-2)/2;

		if (rawAddr >= fftSize*2) {

			if (rawSrc == RAW_SRC_FULL) {
				for (i=0;i<fftSize*2;i++) {
					short t = *(short*)(rawData.data()+(i*2)) << 2;
					in[i][0] = ((float)t) * window[i]/ MAX_SHORT;
					in[i][1] = 0.0f;
				}
			} else {
				for (i=0;i<fftSize;i++) {
					short t;
					t = *(short*)(rawData.data()+(i*4+0)) << 2;
					in[i][0] = ((float)t) * window[i]/ MAX_SHORT;
					//qDebug() << "re" << t;
					t = *(short*)(rawData.data()+(i*4+2)) << 2;
					in[i][1] = ((float)t) * window[i]/ MAX_SHORT;
					//qDebug() << "im" << t;
				}
			}
			rawData.clear();
			fftw_execute(plan);

			for (int i = 0; i < fftSize; i++){
				const float x = out[i][0];
				const float y = out[i][1];
				int v =	log10(sqrt(x*x + y*y)/fftSize)*20*2+FFT_OFFSET;
				if (v<0) v=0;
				if (v>255) v= 255;
				if (rawSrc == RAW_SRC_FULL)
					buf[i] = v;
				else
					buf[i<fftSize/2 ? (int)(fftSize/2+i) : (int)(i-fftSize/2)] = v;
			}
			guiMsg->set_fft(buf,fftSize);

			rawAddr=0;
			return;
		}
	}

	addr[1] = rawAddr >> 8;
	addr[0] = rawAddr & 0xff;
	ret = rawHiqSocket->writeDatagram(addr ,2, QHostAddress(hiqip), HIQSDR_RAW_PORT);
	if (ret != 2)
		PDEBUG(ERR1,"error write raw addr");
}


void Trx::sendFileList() {
	QDir myDir(".");
	QStringList filter;
	filter << "*.raw";
	QStringList list = myDir.entryList (filter);

	QStringListIterator it(list);
	while (it.hasNext())
		guiMsg->add_files(it.next().toStdString());
}

void Trx::newConnection() {
	trxSocket = trxServer->nextPendingConnection();
	qDebug() << "connection from " << trxSocket->peerAddress().toString();
	connect (trxSocket, SIGNAL(readyRead()), this, SLOT(readGuiData()));
	connect (trxSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
	try {
		startTB();
	} catch (...) {
		qDebug() << "error: start failed";
	}
}

void Trx::disconnected() {
	qDebug() << "disconnected";
	tb->stop();
}

void Trx::stop() {
	if (isRunning) {
		qDebug() << "tb->stop";
		tb->stop();
		tb->wait();
	}
	if (tbTimer) {
		tbTimer->stop();
		tbTimer->singleShot(50, this, SLOT(startTB()));
	}
	isRunning = false;
}

void Trx::startTB() {
	if (!isRunning)  {
		qDebug() << "tb->start";
		tb->start();
	}
	isRunning = true;
}
