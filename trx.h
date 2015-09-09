#ifndef TRX_H
#define TRX_H

#include <QtGui>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <fftw3.h>

#include "gnuradio.h"
#include "config.h"
#include "gr_sdr_sink.h"
#include "gr_psk31_decoder.h"
#include "gr_reader_i.h"
#include "gr_limit_ff.h"

#include "rx.h"
#include "tx.h"
#include "trxDev.h"
#include "txSrc.h"

#include "rxSink.h"

#include "usbsr.h"
#include "trx.pb.h"

class Trx : public QObject  {
	Q_OBJECT
	public:
		Trx(int port, char *key);
		~Trx();

	protected:
	private slots:
		QString getInfos();
		void playFile(bool);
		void setRecord(bool);

		void readGuiData();
		void settingsChanged(QString);

		void selectVFO(int vfo);
		void setActiv(bool activ);
		void setAGC(int v=-1);
		void setAntenne(int);
		void setCenterFreq(qint64 f);
		void setFFTSize(int n);
		void setFilter(int, int, int);
		void setFilterNotch(int);
		void setFilterNotchBW(int);
		void setMode(int mode);
		void setNotch(bool);
		void setPreamp(int n=0);
		void setRxFreq(qint64);
		void setSampleRate(int f);
		void setSquelch(int n);
		void setTX(bool);
		void setTxFreq(qint64);
		void setTXPower(int);
		void setGain(int v);
		void setPresel(int pre);
		void setReadRaw(bool);
		void sendFileList();
		void setTxSource(ETxSource src);
		void setRxSink(ERxSink sink);

		float getSmeter();
		void timeout();
		void ffttimeout();
		void keepAliveTimeout();
		void sendGuiMsg();
		void readRaw();
		void newConnection();
		void disconnected();
		void startTB();


	private:
		void setFilter();
		void restoreRX();
		void setTRXDev(RxSrc src);
		void stop();

		ERxSink erxSink;
		ETxSource etxSource;
		int vfo;
		int rawAddr;
		bool activ[MAX_VFO];
		bool sendData, raw;
		bool initial;
		QUdpSocket *rawHiqSocket;
		QTcpServer *trxServer;
		QTcpSocket *trxSocket;
		QTimer *fftTimer, *timer, *keepAliveTimer, *tbTimer;
		QString recordFileName, playFileName;
		QByteArray rawData;
		QByteArray hashKey, session;
		QByteArray datagram;
		QMutex mutex;

		fftw_complex* in, *out;
		fftw_plan plan;

		Rx_sptr rx[MAX_VFO];
		Tx_sptr tx[MAX_VFO];

		TrxDev_sptr trxDev;
		TxSrc_sptr txSrc,txSrcNull;
		RxSink_sptr rxSink;

		gr::filter::firdes *firdes;
		gr::top_block_sptr tb;
		gr::analog::sig_source_f::sptr twoToneTest0,twoToneTest1;
		gr::filter::freq_xlating_fir_filter_ccf::sptr filter_if,filter_wfm,filter_fm,filter_f,filter_f_vfo2;
		gr::filter::freq_xlating_fir_filter_ccc::sptr filter_c;
		gr::filter::freq_xlating_fir_filter_ccc::sptr filter_c_vfo2;
		gr::filter::fir_filter_fff::sptr wfmAudioFilter, fmAudioFilter, filter_eq_lo, filter_eq_mi, filter_eq_hi;
		gr::filter::fir_filter_fff::sptr wfmStereoFilter,wfmAudioFilterSum,wfmAudioFilterDiff;
		gr::analog::agc2_ff::sptr agc;

		gr_sdr_sink_sptr sdrSink;
		gr_psk31_decoder_sptr psk31Decoder;
		gr_reader_i_sptr readerI;
		gr::fft::fft_vcc::sptr fft;
		gr::blocks::stream_to_vector::sptr strToVect;
		gr::blocks::file_sink::sptr fileSink;
		gr::blocks::null_sink::sptr nullSinkc;
		gr::blocks::add_cc::sptr adderc;
		gr::blocks::add_ff::sptr adder_eq;
		gr::filter::dc_blocker_ff::sptr dcBlocker;
		gr::blocks::multiply_const_ff::sptr multiL,multiR,multiTX;
		gr_limit_ff_sptr limiterL,limiterR,limiterTX; 
		gr::blocks::null_sink::sptr nullSink;
		gr::msg_queue::sptr rdsMsgq,fftMsgq,rxMsgq,txMsgq;
		gr::blocks::add_ff::sptr rxAdderL, rxAdderR;
		gr::blocks::message_sink::sptr fftSink;
		gr::blocks::null_source::sptr nullSource;
		gr::blocks::throttle::sptr throttle;
		gr::blocks::keep_one_in_n::sptr keepOneInN;

		bool notchOn;
		bool record;
		bool send;
		bool txAGC;
		double txAGCAtt,txAGCDec,txAGCRef;
		char hiqip[256];
		char audioDevSpeakerMic[256];
		double fftSize;
		float hiqsdrDC;
		int agcVal;
		int ant;
		qint64 centerFreq;
		int filterCut;
		int filterHi;
		int filterLo;
		int fpgaVersion;
		int preamp;
		long int rxFreq; 
		int rxMode;
		int sampleRate;
		int txSampleRate;
		int squelchVal;
		qint64 txFreq; 
		int txPower;
		int gain;
		int osmoFreqCorr;
		int hiqsdrFreqCorr;
		bool dcFilter;
		bool txMute;
		double smtrCal;
		double src0Freq, src1Freq;
		double src0Ampl, src1Ampl	;
		bool twoToneTest,recordTX;
		bool enableEQ;
		int monitor;
		int ssbFMin,ssbFMax;
		double attack_rate_fast,attack_rate_slow;
		double decay_rate_fast,decay_rate_slow;
		double reference_fast, reference_slow;
		int filterNotchFreq;
		int filterNotchBW;
		bool server;
		std::vector<float> window;
		int fftTime;
		int rawSrc;
		bool isRunning;

		UsbSR *usbSR;
		RxSrc rxSource,oldSource;
		TrxProto::GuiMessage *guiMsg;
};

#endif // TRX_H


