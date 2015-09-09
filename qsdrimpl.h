#ifndef QSDRIMPL_H
#define QSDRIMPL_H

#include <QtGui>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QAudioInput>
#include <qsettings.h>

#include "config.h"
#include "ui_qsdr.h"
#include "fftGraph.h"
#include "audioGraph.h"
#include "filterGraph.h"
#include "smeter.h"
#include "trx.h"
#include "settings.h"
#include "play.h"
#include "trx.pb.h"


typedef struct {
    QString name;
    int rxMode;
    int idx;
	bool activ;
    qint64 rxFreq; 
    qint64 txFreq; 
    qint64 centerFreq;
    int filterHi[MODE_LAST-1];
    int filterLo[MODE_LAST-1];
    int filterCut[MODE_LAST-1];
    int preamp;
    int ant;
} MemSet;

class QSdrImpl : public QMainWindow, public Ui::QSdr {
    Q_OBJECT
    public:
	QSdrImpl(char *trx_addr=NULL, int trx_port=TRX_PORT, char *key=NULL, QWidget *parent = 0); 
	~QSdrImpl();
	
	void readSettings();
	void writeSettings();
	
	private slots:

	void setRxFreq(qint64);
	void setTxFreq(qint64);
	void setMode(int mode);
	void setCenterFreq(qint64 f);
	void setVolume(int v);
	void setAGC(int v);
	void setSampleRate(int f);
	void setSampleRate(QString s);
	void setFilterHi(int);
	void setFilterLo(int);
	void setFilterCut(int);
	void setFilterNotch(int);
	void resizeEvent(QResizeEvent *event);
	void recordChanged(bool);
	void appendText(QString);
	void onExit();
	void setMemory(int n, bool band=false);
	void setMemory(QModelIndex i);
	void setFFTSize(int n);
	void setSquelch(int n);
	void setTX(bool, bool gui=false);
	void setTXPower(int, bool gui=false);
	void setPreamp(int n=0);
	void ritChanged(bool, bool gui=false);
	void setAntenne(int, bool gui=false);
	void bandButtonPressed(int);
	void setFreqStep(int);
	void setPlayFile(QString);

	void on_comboBoxStep_activated(QString );
	void on_pushButton_clicked();
	void on_pushButtonSave_clicked();
	void graphicsViewFFTSliderH(int);
	void graphicsViewFFTSliderV(int);
	void graphicsViewFFTSliderH1(int);
	void graphicsViewFFTSliderV1(int);
	void on_pushButtonContrast_clicked();
	void on_sliderFFT1_valueChanged(int);
	void on_sliderFFT2_valueChanged(int);
	void on_checkBox_toggled(bool checked);
	void on_actionAbout_triggered();
	void buttonMinusPressed();
	void buttonPlusPressed();
	void timeout();
	void dialFreqChanged(int pos);
	void freqChanged(qint64 f);
	void settingsChanged();

	void readHamLibUDPData();
	void readTrxData();
    void on_comboBoxVFO_activated(int index);
    void on_checkBoxVFOActiv_clicked(bool checked);
    void on_pushButtonMute_clicked(bool checked);
	QStringList supportedSampleRate();
	bool isTx();
	void playClosed();
	void displayError(QAbstractSocket::SocketError error);
	void connected();
	void disconnected();


    void on_actionSave_image_triggered();

signals:
protected:
	void changeEvent(QEvent *e);

private:
	void dispRxFreq(qint64 f); 
	void dispTxFreq(qint64 f); 
	void keyPressEvent(QKeyEvent *event);
	void setFilterMarker();
	void setFilter();
	void copyMem(MemSet *d,MemSet *s);
	void setCurrent();
	void sendTrxMsg();

	Ui::QSdr *ui;
	QAudioOutput *audioOutput;	
	QAudioInput *audioInput;	
	QBuffer *audioOutBuffer, *audioInBuffer;
	QIODevice *audioOutDev, *audioInDev;
	QByteArray audioOutBuf;
	QByteArray hashKey, session;
	QByteArray datagram;

	FFTGraph *fftGraph, *fftGraph1;
	FilterGraph *filterGraph,*filterGraph1;
	AudioGraph *audioGraph;
	SMeter *smtr;
	QGraphicsScene *sceneFFT, *sceneFFT1;
	QGraphicsScene *sceneSMeter, *sceneAudio;
	QTimer *timer;
	QDateTime *dt;
	int inputFreq;
	bool inputMode;
	int fftGraphHigh;
	int agcVal;
	RxSrc rxSource, oldRxSource;
	QStringList memNames;
	QStringListModel* memListModel;
	QUdpSocket *hamLibSocket;
	QTcpSocket *trxSocket;
	Trx *trx;
	QButtonGroup *bandButtonsGroup;
	QPushButton *bandButtons[MAXBUTTON];
	QHash<QString, QColor> colors;
	play *playDialog;
	
	int activVFO;

	double fftSize;
	int sampleRate;
	int volume;
	int volumeLast;
	int vBarPos;
	bool record;
	bool rebuffer;

	int curMemIdx;
	MemSet mem[MAX_MEM];
	MemSet bandMem[MAXBUTTON];
	MemSet vfoMem[MAX_VFO];
	MemSet *cur;
	MemSet curMem;
	QString playFile;
	int squelchVal;
	int txPower;
	QString hiqsdrIP;
	QHostAddress trxAddr;
	bool tx;
	int fpgaVersion;
	int freqStep;
	bool store,notchOn;
	bool txInHamRange;
	bool raw;
	int waterfallMin,waterfallMax;

	bool dcf77Mode;
	int dcfCnt,secCnt;
	long long dcfReg;
	double eqLo,eqMi,eqHi;
	int rxAudioBufferSize;

	bool conn;

	TrxProto::TrxMessage *trxMsg;
	TrxProto::GuiMessage *guiMsg;
};

#endif // QSDR_H
