#include <QHostInfo>
#include "type.h"
#include <alsa/asoundlib.h>
#include "qsdrimpl.h"
#include "ui_qsdr.h"
#include "lib.h"
#include "crypt.h"

extern "C" {
#include "g72x.h"
}

static int nfft[]={512,768,1024,1536,2048,3072,4096,6144,8192-2};

static int old;

settings *sdrSettings;

QSdrImpl::QSdrImpl(char *trx_addr, int trx_port, char *key, QWidget *parent) : QMainWindow(parent), ui(new Ui::QSdr) {

	int i,j;
	sdrSettings = NULL;

	if (trx_addr == NULL) {
		// Gui + TRX
		trx = new Trx(trx_port, key); 
		trxAddr = QHostAddress("127.0.0.1");
	} else {
		// Gui only
		trx = NULL;
		QHostInfo info = QHostInfo::fromName(trx_addr);
		trxAddr = info.addresses().first();
	}

	hashKey = QByteArray(key);
	rebuffer=true;
	conn = false;

	trxMsg = new TrxProto::TrxMessage();
	guiMsg = new TrxProto::GuiMessage();

	playDialog = NULL;
	activVFO = 0;

	// default settings
	txInHamRange = false;
	rxSource     = SRC_HIQSDR;
	oldRxSource  = SRC_HIQSDR;
	fftSize    = 2048;
	sampleRate = DEFAULT_SAMPLE_RATE;
	playFile   = "";
	agcVal      = 0;
	tx          = false;
	txPower		= 100;
	hiqsdrIP    = "192.168.2.196";
	fpgaVersion = 1;

	notchOn = false;

	fftGraphHigh = 1024;
	inputMode = false;
	volume = 20;
	record = false;
	squelchVal=-100;
	freqStep = 100;

	raw = false;
	store = false;

	dcf77Mode=false;
	dcfCnt=59;
	secCnt=0;
	dcfReg=0;
	waterfallMin = -90;
	waterfallMax = -40;
	
	eqLo=100;
	eqMi=100;
	eqHi=100;

	ui->setupUi(this);  

	sdrSettings = new settings(ui->treeViewSettings);
	for (i=0;i<MAX_MEM;i++) {
		cur = &mem[i];
		cur->name = QString("mem %1").arg(i);
		cur->idx = i;
		cur->rxMode = MODE_LSB;
		cur->rxFreq = 7100000; 
		cur->txFreq = cur->rxFreq; 
		cur->centerFreq = cur->rxFreq;
		cur->preamp = 0;
		cur->ant = 1;
		for (j=0;j<MODE_LAST-1;j++) {
			cur->filterHi[j]  = 2800;
			cur->filterLo[j]  = 300;
			cur->filterCut[j] = 500;
		}
		cur->filterHi[MODE_LSB]  = 2800;
		cur->filterLo[MODE_LSB]  = 300;
		cur->filterCut[MODE_LSB] = 500;
		cur->filterHi[MODE_USB]  = 2800;
		cur->filterLo[MODE_USB]  = 300;
		cur->filterCut[MODE_USB] = 500;
		cur->filterHi[MODE_CW]  = CW_TONE+50;
		cur->filterLo[MODE_CW]  = CW_TONE-50;
		cur->filterCut[MODE_CW] = 100;
		cur->filterHi[MODE_AM]  = 10000;
		cur->filterLo[MODE_AM]  = 100;
		cur->filterCut[MODE_AM] = 1000;
		cur->filterHi[MODE_NFM]  = 10000;
		cur->filterLo[MODE_NFM]  = 100;
		cur->filterCut[MODE_NFM] = 1000;
		cur->filterHi[MODE_WFM_RDS] = 150000;
		cur->filterLo[MODE_WFM_RDS]  = 100;
		cur->filterCut[MODE_WFM_RDS] = 10000;
	}

	for (i=0;i<NBUTTON;i++) {
		cur = &bandMem[i];
		cur->name = QString("mem %1").arg(i);
		cur->idx = i;
		if (i<3)
			cur->rxMode = MODE_LSB;
		else
			cur->rxMode = MODE_USB;
		cur->rxFreq = sdrSettings->getIntVal(QString("Band,%1,min").arg(i+1));
		cur->txFreq = cur->rxFreq; 
		cur->centerFreq = cur->rxFreq;
		cur->preamp = 0;
		cur->ant = 1;
		for (j=0;j<MODE_LAST-1;j++) {
			cur->filterHi[j]  = 2800;
			cur->filterLo[j]  = 300;
			cur->filterCut[j] = 500;
		}
		cur->filterHi[MODE_LSB]  = 2800;
		cur->filterLo[MODE_LSB]  = 300;
		cur->filterCut[MODE_LSB] = 500;
		cur->filterHi[MODE_USB]  = 2800;
		cur->filterLo[MODE_USB]  = 300;
		cur->filterCut[MODE_USB] = 500;
		cur->filterHi[MODE_CW]  = CW_TONE+50;
		cur->filterLo[MODE_CW]  = CW_TONE-50;
		cur->filterCut[MODE_CW] = 100;
		cur->filterHi[MODE_AM]  = 10000;
		cur->filterLo[MODE_AM]  = 100;
		cur->filterCut[MODE_AM] = 1000;
		cur->filterHi[MODE_NFM]  = 10000;
		cur->filterLo[MODE_NFM]  = 100;
		cur->filterCut[MODE_NFM] = 1000;
		cur->filterHi[MODE_WFM_RDS]  = 150000;
		cur->filterLo[MODE_WFM_RDS]  = 100;
		cur->filterCut[MODE_WFM_RDS] = 10000;
	}
	
	for (i=0;i<MAX_VFO;i++) {
		cur = &vfoMem[i];
		cur->activ = false;
		cur->centerFreq = 0;
		ui->comboBoxVFO->addItem(QString("VFO %1").arg(i+1));
	}


	copyMem(&curMem, &mem[0]);
	cur = &curMem;

	readSettings();
	QByteArray hiqip = hiqsdrIP.toLocal8Bit(); 

	old = 512;  
	ui->dialFreq->setSliderPosition(512);
	
	bandButtonsGroup = new QButtonGroup();
	QGridLayout *bandGrid = new QGridLayout(ui->groupBoxBand);
	for (i=0;i<NBUTTON;i++) {
		QPushButton *button = new QPushButton(sdrSettings->getVal(QString("Band,%1,name").arg(i+1)));
		bandGrid->addWidget(button,i/(NBUTTON/2), i%(NBUTTON/2));
		bandButtonsGroup->addButton(button,i);
		bandButtons[i]=button;
	}
	

	for (i=0; sizeof(nfft)/sizeof(nfft[0]); i++)
		if (fftSize == nfft[i]) {
			ui->sliderFFT3->setSliderPosition(i);
			break;
		}

	memListModel = new QStringListModel(memNames);
	ui->listViewMem->setModel(memListModel);

	dt = new QDateTime();

	fftGraph = new FFTGraph(sdrSettings, fftSize, fftGraphHigh);
	filterGraph = new FilterGraph(sdrSettings, fftSize, fftGraphHigh);
	filterGraph->setAudioFormat (sampleRate);
	
	fftGraph1 = new FFTGraph(sdrSettings, fftSize, fftGraphHigh);
	filterGraph1 = new FilterGraph(sdrSettings, fftSize, fftGraphHigh);
	filterGraph1->setAudioFormat (sampleRate);
	
	connect(sdrSettings, SIGNAL(changed()), this, SLOT(settingsChanged()));
	connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onExit()));
	connect(filterGraph, SIGNAL(freqChanged(qint64)), this, SLOT(freqChanged(qint64)), Qt::QueuedConnection);
	connect(filterGraph1, SIGNAL(freqChanged(qint64)), this, SLOT(freqChanged(qint64)), Qt::QueuedConnection);
	connect(ui->dialFreq, SIGNAL(valueChanged(int)), this, SLOT(dialFreqChanged(int)));
	connect(ui->comboBoxMode, SIGNAL(activated(int)), this, SLOT(setMode(int)));
	connect(ui->comboBoxAGC, SIGNAL(activated(int)), this, SLOT(setAGC(int)));
	connect(ui->comboBoxSampleRate, SIGNAL(activated(QString)), this, SLOT(setSampleRate(QString)));
	connect(ui->comboBoxPreamp, SIGNAL(activated(int)), this, SLOT(setPreamp(int)));
	connect(ui->sliderBWHi, SIGNAL(valueChanged(int)), this, SLOT(setFilterHi(int)));
	connect(ui->sliderBWLo, SIGNAL(valueChanged(int)), this, SLOT(setFilterLo(int)));
	connect(ui->sliderBWCut, SIGNAL(valueChanged(int)), this, SLOT(setFilterCut(int)));
	connect(ui->sliderNotch, SIGNAL(valueChanged(int)), this, SLOT(setFilterNotch(int)));
	connect(ui->sliderTXPower, SIGNAL(valueChanged(int)), this, SLOT(setTXPower(int)));
	connect(ui->dialVolume, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
	connect(ui->dialSquelch, SIGNAL(valueChanged(int)), this, SLOT(setSquelch(int)));
	connect(ui->sliderFFT3, SIGNAL(valueChanged(int)), this, SLOT(setFFTSize(int)));
	connect(ui->pushButtonMinus, SIGNAL(pressed()), this, SLOT(buttonMinusPressed()));
	connect(ui->pushButtonPlus, SIGNAL(pressed()), this, SLOT(buttonPlusPressed()));
	connect(ui->checkBoxRecord, SIGNAL(toggled(bool)), this, SLOT(recordChanged(bool)));
	connect(ui->checkBoxRit, SIGNAL(toggled(bool)), this, SLOT(ritChanged(bool)));
	connect(ui->pushButtonTX, SIGNAL(toggled(bool)), this, SLOT(setTX(bool)));
	connect(ui->spinBoxAnt, SIGNAL(valueChanged(int)), this, SLOT(setAntenne(int)));
	connect(ui->listViewMem, SIGNAL(activated(QModelIndex)), this, SLOT(setMemory(QModelIndex)));
	connect(ui->listViewMem, SIGNAL(clicked(QModelIndex)), this, SLOT(setMemory(QModelIndex)));
	connect(ui->graphicsViewFFT->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(graphicsViewFFTSliderV(int)));
	connect(ui->graphicsViewFFT->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(graphicsViewFFTSliderH(int)));
	connect(ui->graphicsViewFFT1->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(graphicsViewFFTSliderV1(int)));
	connect(ui->graphicsViewFFT1->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(graphicsViewFFTSliderH1(int)));
	connect(bandButtonsGroup, SIGNAL(buttonPressed(int)), this, SLOT(bandButtonPressed(int)));

	audioGraph = new AudioGraph(sdrSettings, 256, 256);
	sceneAudio = new QGraphicsScene();
	sceneAudio->setItemIndexMethod(QGraphicsScene::NoIndex);
	//ui->graphicsViewAudio->setRenderHint(QPainter::Antialiasing);
	//sceneAudio->setItemIndexMethod(QGraphicsScene::NoIndex);
	sceneAudio->addItem(audioGraph);
	ui->graphicsViewAudio->setScene(sceneAudio);

	sceneFFT = new QGraphicsScene();
	sceneFFT->setItemIndexMethod(QGraphicsScene::NoIndex);
	sceneFFT->addItem(fftGraph);
	sceneFFT->addItem(filterGraph);
	ui->graphicsViewFFT->setMouseTracking(true);
	ui->graphicsViewFFT->setScene(sceneFFT);
	
	sceneFFT1 = new QGraphicsScene();
	sceneFFT1->setItemIndexMethod(QGraphicsScene::NoIndex);
	sceneFFT1->addItem(fftGraph1);
	sceneFFT1->addItem(filterGraph1);
	ui->graphicsViewFFT1->setMouseTracking(true);
	ui->graphicsViewFFT1->setScene(sceneFFT1);

	ui->comboBoxSampleRate->clear();
	ui->comboBoxSampleRate->addItems(supportedSampleRate());
	ui->comboBoxSampleRate->setCurrentIndex(ui->comboBoxSampleRate->findText(QString("%1").arg(sampleRate)));

	smtr = new SMeter(100,100);
	sceneSMeter = new QGraphicsScene();
	ui->graphicsViewSMeter->setRenderHint(QPainter::Antialiasing);
	sceneSMeter->setItemIndexMethod(QGraphicsScene::NoIndex);
	sceneSMeter->addItem(smtr);
	ui->graphicsViewSMeter->setScene(sceneSMeter);

	ui->graphicsViewFFT->setSceneRect(QRect(0, 0, fftSize, fftGraphHigh));
	ui->graphicsViewFFT->centerOn(fftSize/2, 0);
	ui->graphicsViewFFT1->setSceneRect(QRect(0, 0, fftSize, fftGraphHigh));
	ui->graphicsViewFFT1->centerOn(fftSize/2, 0);

	ui->comboBoxAGC->setCurrentIndex(agcVal);

	fftGraph->setDisplayMode(GRAPH_WATERFALL);
	filterGraph->setDisplayMode(GRAPH_WATERFALL);
	fftGraph1->setDisplayMode(GRAPH_FFT);
	filterGraph1->setDisplayMode(GRAPH_FFT);

	ui->sliderFFT1->setValue(waterfallMin);
	ui->sliderFFT2->setValue(waterfallMax);
	fftGraph->setMin(waterfallMin);
	fftGraph->setMax(waterfallMax);
	fftGraph1->setMin(waterfallMin);
	fftGraph1->setMax(waterfallMax);
	setVolume(20);                                       // Voreinstellung

	setSampleRate(sampleRate);
	setAGC(agcVal);
	setTXPower(txPower, true);
	setFreqStep(freqStep);
	setFilterNotch(4000);

	trxSocket = new QTcpSocket(this);
	trxSocket->connectToHost(trxAddr, trx_port);
	connect(trxSocket, SIGNAL(readyRead()), this, SLOT(readTrxData()));
	connect(trxSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
	connect(trxSocket, SIGNAL(connected()), this, SLOT(connected()));
	connect(trxSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));

	hamLibSocket = new QUdpSocket(this);
	hamLibSocket->bind(QHostAddress("127.0.0.1"), 13222, QUdpSocket::ShareAddress);
	connect(hamLibSocket, SIGNAL(readyRead()), this, SLOT(readHamLibUDPData()));

	setCurrent();

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	timer->start(20);		// 50ms

	QAudioFormat format;
	format.setFrequency(8000);
	format.setChannels(1);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	QList<QAudioDeviceInfo> audioDevicesOut =  QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	QList<QAudioDeviceInfo> audioDevicesIn =  QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	
	int inIdx = sdrSettings->getIntVal(QString("Device,AudioDev,GUI,Input"));
	if (inIdx>=audioDevicesIn.size())
		inIdx = 0;
	int outIdx = sdrSettings->getIntVal(QString("Device,AudioDev,GUI,Output"));
	if (outIdx>=audioDevicesOut.size())
		outIdx = 0;

	QAudioDeviceInfo infoIn  = audioDevicesIn.at(inIdx);
	QAudioDeviceInfo infoOut = audioDevicesOut.at(outIdx);

	if (!infoIn.isFormatSupported(format)) {
		qWarning()<<"AudioInput: default format not supported try to use nearest";
		format = infoIn.nearestFormat(format);
	}
	if (!infoOut.isFormatSupported(format)) {
		qWarning()<<"AudioInput: default format not supported try to use nearest";
		format = infoOut.nearestFormat(format);
	}
	audioOutput = new QAudioOutput(infoOut, format);
	audioInput = new QAudioInput(infoIn, format);
	//audioOutput->setBufferSize(8000);
	audioInDev = NULL;
	audioOutDev = NULL;
	if ((ERxSink)sdrSettings->getIntVal("Trx,RX,Sink") == RX_SINK_IP)
		audioOutDev = audioOutput->start();
	
	setVolume(volumeLast);
	show();
	this->setFocus();
	ui->graphicsViewFFT1->verticalScrollBar()->setSliderPosition(vBarPos);

	QPalette p(palette());
	p.setColor(QPalette::Window,        QColor(sdrSettings->getVal("Display,Color,App,Win")));
	p.setColor(QPalette::WindowText,    QColor(sdrSettings->getVal("Display,Color,App,WindowText")));
	p.setColor(QPalette::Base,          QColor(sdrSettings->getVal("Display,Color,App,Base")));
	p.setColor(QPalette::AlternateBase, QColor(sdrSettings->getVal("Display,Color,App,AlternateBase")));
	p.setColor(QPalette::Text,          QColor(sdrSettings->getVal("Display,Color,App,Text")));
	p.setColor(QPalette::Button,        QColor(sdrSettings->getVal("Display,Color,App,Button")));
	p.setColor(QPalette::ButtonText,    QColor(sdrSettings->getVal("Display,Color,App,ButtonText")));
	p.setColor(QPalette::BrightText,    QColor(sdrSettings->getVal("Display,Color,App,BrightText")));
	this->setAutoFillBackground(true);
	this->setPalette(p);

	
	bool tx = isTx();
	ui->groupBoxTx->setEnabled(tx);
	ui->lcdNumberTX->setEnabled(tx);

	// enable current vfo
	cur->activ = true;
	ui->checkBoxVFOActiv->setChecked(cur->activ);

	rxAudioBufferSize = sdrSettings->getIntVal(QString("Interconnect,AudioBufferSize"));

	QString s = sdrSettings->printTreePath();

	trxMsg->set_fftsize(fftSize);
	trxMsg->set_vfo(activVFO);
	trxMsg->set_vfoactiv(cur->activ);
//	trxMsg->set_settings(s.toStdString().data(), s.toStdString().size());
}

QSdrImpl::~QSdrImpl() {

	writeSettings();

	trxSocket->disconnectFromHost();
	hamLibSocket->close();

	if (trx)
		delete trx;
	delete ui;
	delete trxMsg;
	delete guiMsg;
}

void QSdrImpl::changeEvent(QEvent *e) {
	QMainWindow::changeEvent(e);
	switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void QSdrImpl::buttonMinusPressed() {
	qint64 f=cur->centerFreq - (sampleRate/5000)*5000;
	setCenterFreq(f);
	setRxFreq(f);
	setTxFreq(f);
}

void QSdrImpl::buttonPlusPressed() {
	qint64 f=cur->centerFreq + (sampleRate/5000)*5000;
	setCenterFreq(f);
	setRxFreq(f);
	setTxFreq(f);
}

void QSdrImpl::timeout() {		// 50ms
	unsigned char buf[32*1024];
	static int cnt=0;

	if (cnt++ > 10) { 
		cnt = 0;

		trxMsg->set_keepalive(true);

		if (sdrSettings->getIntVal("Time,Zone") == 0) {
			*dt = QDateTime::currentDateTime().toUTC();
			ui->labelTimezoneView->setText("UTC ");
		} else {
			ui->labelTimezoneView->setText("Local "); 
			*dt = QDateTime::currentDateTime();
		}
		ui->lcdNumberHour  ->display(dt->time().hour());
		ui->lcdNumberMinute->display(QString("%1").arg(dt->time().minute(),2,10,QChar('0')));
		ui->lcdNumberSec   ->display(QString("%1").arg(dt->time().second(),2,10,QChar('0')));
	}

	if (audioInDev) {
		QByteArray audioInData =  audioInDev->readAll();
		if (audioInData.size()>0) {
			int i;
			unsigned char *in = (unsigned char*)audioInData.data();
			for (i=0;i<min(audioInData.size()/sizeof(short),(int)sizeof(buf));i++) {
				short sample = (in[i*2+1]<<8) | in[i*2];
				if (tx)
					audioGraph->audioData(sample);
				buf[i] = linear2ulaw(sample);
			}
			trxMsg->set_audio(buf, i);
		}
	}

	if (audioOutDev) {
		int free = audioOutput->bytesFree();
		if (free >0 && audioOutBuf.size() > rxAudioBufferSize) {
			//qDebug() <<  "AudioBuffer Free:" << free <<  "Available"<< audioOutBuf.size() << rxAudioBufferSize;
			int ret = audioOutDev->write(audioOutBuf.left(free));
			audioOutBuf = audioOutBuf.mid(ret);
			if (rxAudioBufferSize > 0)
				rxAudioBufferSize = 0;
		}
	}

	sendTrxMsg();
}

void QSdrImpl::dispRxFreq(qint64 f) {
	char str[16];
	sprintf(str,"%6i.%03i",f/1000,f%1000);
	ui->lcdNumberRX->display(str);
	filterGraph->setFreq(raw ? RX_CLOCK/4 : cur->centerFreq, cur->rxFreq, cur->filterLo[cur->rxMode], cur->filterHi[cur->rxMode], cur->rxMode);
	filterGraph1->setFreq(raw ? RX_CLOCK/4 : cur->centerFreq, cur->rxFreq, cur->filterLo[cur->rxMode], cur->filterHi[cur->rxMode], cur->rxMode);
}

void QSdrImpl::dispTxFreq(qint64 f) {
	char str[16];
	sprintf(str,"%6i.%03i",f/1000,f%1000);
	ui->lcdNumberTX->display(str);
}

void QSdrImpl::dialFreqChanged(int pos) {
	int diff =  pos - old;
	if (diff>512) diff -= 1024;
	if (diff<-512) diff += 1024;
	diff = diff*freqStep/3;
	if (ui->checkBoxRit->isChecked()) {
		if (ui->checkBoxTX->isChecked())
			setTxFreq(cur->txFreq + diff);
		else
			setRxFreq(cur->rxFreq + diff);
	} else {
		setRxFreq(cur->rxFreq + diff);
		setTxFreq(cur->rxFreq);
	}
	old = pos;
}

void QSdrImpl::setRxFreq(qint64 f) {
	cur->rxFreq = f;
	if (cur->rxFreq - cur->centerFreq > sampleRate/2) 
		setCenterFreq(f);
	if (cur->rxFreq - cur->centerFreq < -sampleRate/2)
		setCenterFreq(f);
	inputMode = false;
	ui->lcdNumberRX->setSegmentStyle(QLCDNumber::Flat);
	dispRxFreq(cur->rxFreq);
	trxMsg->set_rxfreq(f);

	for (int i=0;i<NBUTTON;i++) {
		if (((sdrSettings->getIntVal(QString("Band,%1,min").arg(i+1))) <= f) && ((sdrSettings->getIntVal(QString("Band,%1,max").arg(i+1))) >= f)) {
			copyMem(&bandMem[i], cur);
			//bandButtons[i]->setText(QString("%1").arg(f));
			bandButtons[i]->setPalette( QPalette(QColor(sdrSettings->getVal("Display,Color,App,Button"))));
			trxMsg->set_presel(sdrSettings->getIntVal(QString("Band,%1,presel").arg(i+1)));
		} else {
			bandButtons[i]->setPalette( QPalette(QColor(sdrSettings->getVal("Display,Color,App,ButtonInactiv"))));
		}
	}
}

void QSdrImpl::setTxFreq(qint64 f) {
	cur->txFreq = f;
	inputMode = false;
	ui->lcdNumberTX->setSegmentStyle(QLCDNumber::Flat);
	dispTxFreq(cur->txFreq);
	trxMsg->set_txfreq(cur->txFreq);
	txInHamRange = false;
	for (int i=0;i<NBUTTON;i++) {
		if (((sdrSettings->getIntVal(QString("Band,%1,min").arg(i+1))) <= f) && ((sdrSettings->getIntVal(QString("Band,%1,max").arg(i+1))) >= f)) 
			txInHamRange = true;
	}
}

void QSdrImpl::setCenterFreq(qint64 f) {
	cur->centerFreq = f;
	trxMsg->set_rxcenterfreq(f);
}

void QSdrImpl::setVolume(int v) {
	volume = v;
	if (ui->pushButtonMute->isChecked())
		trxMsg->set_gain(0);
	else
		trxMsg->set_gain(v);
	ui->dialVolume->setValue(v);

	int inv_gain = 90;
	if (agcVal == 0) {
		inv_gain = 50;
		ui->labelVolume->setText(QString("Gain: %1 dB").arg((int)(log10((exp(-10+(float)(volume)/inv_gain)))*20)));
	} else {
		ui->labelVolume->setText(QString("Volume: %1 dB").arg((int)(log10((exp(-10+(float)(volume)/inv_gain)))*20)));
	}
}

void QSdrImpl::setSquelch(int v) {
	squelchVal = v;
	trxMsg->set_squelchlevel(v);
}

void QSdrImpl::freqChanged(qint64 f) {
	if (ui->checkBoxRit->isChecked()) {
		if (ui->checkBoxTX->isChecked())
			setTxFreq(f);
		else
			setRxFreq(f);
	} else {
		setRxFreq(f);
		setTxFreq(f);
	}
}

void QSdrImpl::setMode(int m) {
	trxMsg->set_mode(m); 

	cur->rxMode = m;
	if (m == MODE_WFM_RDS) {
		ui->sliderBWHi->setMaximum(300000);
		ui->sliderBWCut->setMaximum(100000);
		ui->sliderBWHi->setValue(cur->filterHi[cur->rxMode]);
		ui->sliderBWLo->setValue(cur->filterLo[cur->rxMode]);
		ui->sliderBWCut->setValue(cur->filterCut[cur->rxMode]);
		ui->sliderBWLo->hide();
		ui->labelFilterLo->hide();
		ui->sliderNotch->hide();
		ui->checkBox->hide();
		ui->labelNotch->hide();
	} else if (m == MODE_AM || m == MODE_NFM) {
		ui->sliderBWHi->setValue(cur->filterHi[cur->rxMode]);
		ui->sliderBWLo->setValue(cur->filterLo[cur->rxMode]);
		ui->sliderBWCut->setValue(cur->filterCut[cur->rxMode]);
		ui->sliderBWHi->setMaximum(IF_RATE/2);
		ui->sliderBWCut->setMaximum(IF_RATE/4);
		ui->sliderBWLo->setMaximum(IF_RATE/8);
		ui->sliderBWLo->hide();
		ui->labelFilterLo->hide();
		ui->sliderNotch->hide();
		ui->checkBox->hide();
		ui->labelNotch->hide();
	} else if (m == MODE_CW) {
		ui->sliderBWHi->setValue(cur->filterHi[cur->rxMode]);
		ui->sliderBWLo->setValue(cur->filterLo[cur->rxMode]);
		ui->sliderBWCut->setValue(cur->filterCut[cur->rxMode]);
		ui->sliderBWHi->setMaximum(IF_RATE/2);
		ui->sliderBWCut->setMaximum(IF_RATE/4);
		ui->sliderBWLo->setMaximum(IF_RATE/8);
		ui->sliderNotch->hide();
		ui->checkBox->hide();
		ui->labelNotch->hide();
	} else {
		ui->sliderBWHi->setValue(cur->filterHi[cur->rxMode]);
		ui->sliderBWLo->setValue(cur->filterLo[cur->rxMode]);
		ui->sliderBWCut->setValue(cur->filterCut[cur->rxMode]);
		ui->sliderBWHi->setMaximum(IF_RATE/2);
		ui->sliderBWCut->setMaximum(IF_RATE/4);
		ui->sliderBWLo->setMaximum(IF_RATE/8);
		ui->sliderBWLo->show();
		ui->labelFilterLo->show();
		ui->sliderNotch->show();
		ui->checkBox->show();
		ui->labelNotch->show();
	}
	trxMsg->set_mode(m);

	setRxFreq(cur->rxFreq);
	setTxFreq(cur->txFreq);
	setFilter();
}

void QSdrImpl::setFilterLo(int v) {
	if (v >= cur->filterHi[cur->rxMode])
		setFilterHi(v+10);
	cur->filterLo[cur->rxMode] = v;
	switch (cur->rxMode) {
		case MODE_NFM:
		case MODE_WFM_RDS:
		case MODE_AM:
			ui->labelFilterLo->setText(QString("Lo: %1").arg(v*2));
		break;
		default:
			ui->labelFilterLo->setText(QString("Lo: %1").arg(v));
	}
	ui->sliderBWLo->setValue(v);
	setFilter();
}

void QSdrImpl::setFilterHi(int v) {
	if (v > sampleRate/2) 
		v = sampleRate/2;
	if (v <= cur->filterLo[cur->rxMode])
		setFilterLo(v-10);
	cur->filterHi[cur->rxMode] = v;
	ui->sliderBWHi->setValue(v);
	switch (cur->rxMode) {
		case MODE_NFM:
		case MODE_WFM_RDS:
		case MODE_AM:
			ui->labelFilterHi->setText(QString("Hi: %1").arg(v*2));
			break;
		default:
			ui->labelFilterHi->setText(QString("Hi: %1").arg(v));
	}
	setFilter();
}

void QSdrImpl::setFilterNotch(int v) {
	trxMsg->set_filternotchfreq(v);
	ui->labelNotch->setText(QString("Notch: %1").arg(v));
}

void QSdrImpl::setFilterCut(int v) {
	cur->filterCut[cur->rxMode] = v;
	ui->sliderBWCut->setValue(v);
	ui->labelFilterCut->setText(QString("Cut: %1").arg(v));
	setFilter();
}

void QSdrImpl::setTXPower(int v, bool gui) {
	txPower = v;
	trxMsg->set_txpower(txPower*255/100);
	ui->labelTXPower->setText(QString("%1").arg(v));
	if (gui) 
		ui->sliderTXPower->setValue(txPower);
}

void QSdrImpl::setFilter() {
	if (cur->filterHi[cur->rxMode]>sampleRate/2)
		cur->filterHi[cur->rxMode]=sampleRate/2;
	filterGraph->setFreq(raw ? RX_CLOCK/4 : cur->centerFreq, cur->rxFreq, cur->filterLo[cur->rxMode], cur->filterHi[cur->rxMode], cur->rxMode);
	filterGraph1->setFreq(raw ? RX_CLOCK/4 : cur->centerFreq, cur->rxFreq, cur->filterLo[cur->rxMode], cur->filterHi[cur->rxMode], cur->rxMode);
	trxMsg->set_filterlo(cur->filterLo[cur->rxMode]);
	trxMsg->set_filterhi(cur->filterHi[cur->rxMode]);
	trxMsg->set_filtercut(cur->filterCut[cur->rxMode]);
}

void QSdrImpl::setAGC(int v) {
	trxMsg->set_agc(v);
	agcVal = v;
	setVolume(volume);
}

void QSdrImpl::setSampleRate(QString s) {
	if (s.toInt()==0 || s.toInt() >= RX_CLOCK/2) {
		raw = true;
		setSampleRate(DEFAULT_SAMPLE_RATE);
		trxMsg->set_readraw(raw);
		filterGraph->setAudioFormat (RX_CLOCK/2);
		filterGraph1->setAudioFormat (RX_CLOCK/2);
	} else {
		raw = false;
		setSampleRate(s.toInt());
		trxMsg->set_readraw(raw);
	}
	filterGraph->setFreq(raw ? RX_CLOCK/4 : cur->centerFreq, cur->rxFreq, cur->filterLo[cur->rxMode], cur->filterHi[cur->rxMode], cur->rxMode);
	filterGraph1->setFreq(raw ? RX_CLOCK/4 : cur->centerFreq, cur->rxFreq, cur->filterLo[cur->rxMode], cur->filterHi[cur->rxMode], cur->rxMode);
}

void QSdrImpl::setSampleRate(int f) {
	sampleRate = f;
	if (cur->filterHi[cur->rxMode] > sampleRate/2)
		cur->filterHi[cur->rxMode] = sampleRate/2;

	filterGraph->setAudioFormat (sampleRate);
	filterGraph1->setAudioFormat (sampleRate);

	setCenterFreq(cur->rxFreq);
	trxMsg->set_samplerate(sampleRate);
}

void QSdrImpl::setPreamp(int n) {
	cur->preamp = ui->comboBoxPreamp->currentText().toInt();
	trxMsg->set_preamp(cur->preamp);
}

void QSdrImpl::keyPressEvent(QKeyEvent *event) {
	int key = event->key(); 

	switch (key) {
		case Qt::Key_0 ... Qt::Key_9:
			if (!inputMode) {
				inputMode = true;
				inputFreq = 0;
				if (ui->checkBoxRit->isChecked()) {
					if (ui->checkBoxTX->isChecked())
						ui->lcdNumberTX->setSegmentStyle(QLCDNumber::Filled);
					else
						ui->lcdNumberRX->setSegmentStyle(QLCDNumber::Filled);
				} else {
					ui->lcdNumberRX->setSegmentStyle(QLCDNumber::Filled);
					ui->lcdNumberTX->setSegmentStyle(QLCDNumber::Filled);
				}

			}
			inputFreq = inputFreq*10 + key - Qt::Key_0;
			if (inputFreq > 9999999)
				inputFreq = 0;
			if (ui->checkBoxRit->isChecked()) {
				if (ui->checkBoxTX->isChecked())
					ui->lcdNumberTX->display(inputFreq);
				else
					ui->lcdNumberRX->display(inputFreq);
			} else {
				ui->lcdNumberRX->display(inputFreq);
				ui->lcdNumberTX->display(inputFreq);
			}
			break;

		case Qt::Key_Space:
			tx = !tx;
			setTX(tx, true);
			break;

		case Qt::Key_Minus: 
			freqChanged(cur->rxFreq - freqStep);
			break;

		case Qt::Key_Plus: 
			freqChanged(cur->rxFreq + freqStep);
			break;

		case Qt::Key_NumberSign:
			freqChanged(cur->rxFreq - cur->rxFreq % freqStep);
			break;

		case Qt::Key_F11:
			this->setWindowState(this->windowState() ^  Qt::WindowFullScreen);
			if (this->windowState() == Qt::WindowFullScreen)
				this->menuBar()->hide();
			else
				this->menuBar()->show();
			break;

		case Qt::Key_F1 ... Qt::Key_F10:
			bandButtonPressed(key - Qt::Key_F1);
			break;
	}


	if (inputMode) {
		if (key == Qt::Key_Return) {
			if (ui->checkBoxRit->isChecked()) {
				if (ui->checkBoxTX->isChecked())
					setTxFreq((qint64)inputFreq*1000);
				else
					setRxFreq((qint64)inputFreq*1000);
			} else {
				setRxFreq((qint64)inputFreq*1000);
				setTxFreq((qint64)inputFreq*1000);
			}
		}
		if (key == Qt::Key_Escape) {
			setRxFreq(cur->rxFreq);
			setTxFreq(cur->rxFreq);
		}
	}
}

void QSdrImpl::bandButtonPressed(int id) {
	setMemory(id, true);	
}

void QSdrImpl::resizeEvent(QResizeEvent *event) {
	ui->graphicsViewSMeter->fitInView(smtr->boundingRect());
}

void QSdrImpl::recordChanged(bool t) {
	record = t;
	playFile = QString("%1_%2_%3.raw").arg(dt->toString("yy.MM.dd_hh:mm:ss")).arg(cur->centerFreq).arg(sampleRate);
	trxMsg->set_recordfilename(playFile.toAscii().data());
	trxMsg->set_recordfile(t);
	if (record) {
		QMainWindow::statusBar()->showMessage(QString("recording: ") + playFile);
	} else {
		QMainWindow::statusBar()->showMessage(QString("ready: ") + playFile);
	}
}

void QSdrImpl::appendText(QString s) {
	ui->plainTextEdit->insertPlainText(s);
	ui->plainTextEdit->centerCursor();
}

void QSdrImpl::readSettings() {
	MemSet *memset;

	QSettings settings("sdr","qsdr");
	
	settings.beginGroup("Common");
	rxSource    = (RxSrc)settings.value("rxSource", rxSource).toInt();
	agcVal      = settings.value("agcVal",agcVal).toInt();
	sampleRate  = settings.value("sampleRate",sampleRate).toInt();
	fftSize     = settings.value("fftSize",fftSize).toInt();
	txPower     = settings.value("txPower",txPower).toInt();
	hiqsdrIP    = settings.value("hiqsdrIP",hiqsdrIP).toString();
	fpgaVersion = settings.value("fpgaVersion",fpgaVersion).toInt();
	volumeLast = settings.value("volume",volumeLast).toInt();
	vBarPos = settings.value("vBarPos", vBarPos).toInt();
	eqLo = settings.value("txEqLo",eqLo).toInt();
	eqMi = settings.value("txEqMi",eqMi).toInt();
	eqHi = settings.value("txEqHi",eqHi).toInt();
	freqStep = settings.value("freqStep",freqStep).toInt();
	sdrSettings->setTree(settings.value("treeSettings").toString());
	waterfallMin = settings.value("waterfallMin",waterfallMin).toInt();
	waterfallMax = settings.value("waterfallMax",waterfallMax).toInt();

	
	settings.endGroup();
	
	settings.beginGroup("Layout");
	if(settings.value("checkLayout").toInt()) { 
	   restoreGeometry(settings.value("Geometry").toByteArray()); 
	   ui->splitterDisp->restoreState(settings.value("SplitterDisp").toByteArray());
	   ui->splitterFFT->restoreState(settings.value("SplitterFFT").toByteArray());
	   ui->splitterSdr->restoreState(settings.value("SplitterSdr").toByteArray()); 
	   resize(settings.value("Size",sizeHint()).toSize()); 
	   restoreState(settings.value("Properties").toByteArray()); 
	   
	   // last slider->graphicView_Position ??      geht so nicht                         // hbd
	   // last ui->dialFreq->setValue(); zeiger_position ? für centerFreq 0 oder 360 grad // hbd
	   
	}
	settings.endGroup();

	settings.beginGroup("Current");
	memset = &curMem;
	memset->rxMode     = (Mode)settings.value("rxMode",memset->rxMode).toInt();
	memset->rxFreq     = settings.value("rxFreq",memset->rxFreq).toLongLong();
	memset->txFreq     = settings.value("txFreq",memset->txFreq).toLongLong();
	memset->centerFreq = settings.value("centerFreq",memset->centerFreq).toLongLong();
	memset->preamp     = settings.value("preamp",memset->preamp).toInt();
	memset->ant        = settings.value("ant",memset->ant).toInt();
	
	settings.beginReadArray("Filter");
	for (int j=0;j<MODE_LAST-1;j++) {
		settings.setArrayIndex(j);
		memset->filterHi[j]   = settings.value("filterHi",memset->filterHi[j]).toInt();
		memset->filterLo[j]   = settings.value("filterLo",memset->filterLo[j]).toInt();
		memset->filterCut[j]  = settings.value("filterCut",memset->filterCut[j]).toInt();
	}
	settings.endArray();
	settings.endGroup();
	
	settings.beginReadArray("Memory");
	for (int i=0;i<MAX_MEM;i++) {
		memset = &mem[i];
		settings.setArrayIndex(i);
		memset->name       = settings.value("name",memset->name).toString();
		memNames.append(memset->name);
		memset->rxMode     = (Mode)settings.value("rxMode",memset->rxMode).toInt();
		memset->rxFreq     = settings.value("rxFreq",memset->rxFreq).toInt();
		memset->txFreq     = settings.value("txFreq",memset->txFreq).toInt();
		memset->centerFreq = settings.value("centerFreq",memset->centerFreq).toInt();
		memset->preamp     = settings.value("preamp",memset->preamp).toInt();
		memset->ant        = settings.value("ant",memset->ant).toInt();
		settings.beginReadArray("Filter");
		for (int j=0;j<MODE_LAST-1;j++) {
			settings.setArrayIndex(j);
			memset->filterHi[j]   = settings.value("filterHi",memset->filterHi[j]).toInt();
			memset->filterLo[j]   = settings.value("filterLo",memset->filterLo[j]).toInt();
			memset->filterCut[j]  = settings.value("filterCut",memset->filterCut[j]).toInt();
		}
		settings.endArray();
	}
	settings.endArray();

	settings.beginReadArray("Band");
	for (int i=0;i<NBUTTON;i++) {
		memset = &bandMem[i];
		settings.setArrayIndex(i);
		memset->name       = settings.value("name",memset->name).toString();
		memset->rxMode     = (Mode)settings.value("rxMode",memset->rxMode).toInt();
		memset->rxFreq     = settings.value("rxFreq",memset->rxFreq).toInt();
		memset->txFreq     = settings.value("txFreq",memset->txFreq).toInt();
		memset->centerFreq = settings.value("centerFreq",memset->centerFreq).toInt();
		memset->preamp     = settings.value("preamp",memset->preamp).toInt();
		memset->ant        = settings.value("ant",memset->ant).toInt();
		settings.beginReadArray("Filter");
		for (int j=0;j<MODE_LAST-1;j++) {
			settings.setArrayIndex(j);
			memset->filterHi[j]   = settings.value("filterHi",memset->filterHi[j]).toInt();
			memset->filterLo[j]   = settings.value("filterLo",memset->filterLo[j]).toInt();
			memset->filterCut[j]  = settings.value("filterCut",memset->filterCut[j]).toInt();
		}
		settings.endArray();
	}
	settings.endArray();
}

void QSdrImpl::writeSettings() {
	MemSet *memset;

	QSettings settings("sdr","qsdr");
	settings.beginGroup("Common");

	settings.setValue("rxSource", rxSource);
	settings.setValue("agcVal", agcVal);
	settings.setValue("sampleRate", sampleRate);
	settings.setValue("fftSize", fftSize);
	settings.setValue("txPower", txPower);
	settings.setValue("hiqsdrIP", hiqsdrIP);
	settings.setValue("fpgaVersion", fpgaVersion);
	settings.setValue("volume",ui->dialVolume->value());
	settings.setValue("squelch",ui->dialSquelch->value());
	settings.setValue("vBarPos",ui->graphicsViewFFT1->verticalScrollBar()->sliderPosition());
	settings.setValue("txEqLo",eqLo);
	settings.setValue("txEqMi",eqMi);
	settings.setValue("txEqHi",eqHi);
	settings.setValue("freqStep",freqStep);
	settings.setValue("treeSettings",sdrSettings->printTreePath());
	settings.setValue("waterfallMin",waterfallMin);
	settings.setValue("waterfallMax",waterfallMax);

	settings.endGroup();
	
	settings.beginGroup("Layout");
	settings.setValue("Geometry",saveGeometry()); 
	settings.setValue("SplitterSdr",ui->splitterSdr->saveState());
	settings.setValue("SplitterDisp",ui->splitterDisp->saveState()); 
	settings.setValue("SplitterFFT",ui->splitterFFT->saveState()); 
	settings.setValue("Size",size());
	settings.setValue("Properties",saveState()); 
	settings.setValue("checkLayout", 1); 
	settings.endGroup();

	settings.beginGroup("Current");
	memset = &curMem;
	memset->idx = 0;
	settings.setValue("rxMode", (int)memset->rxMode);
	settings.setValue("rxFreq", memset->rxFreq);
	settings.setValue("txFreq", memset->txFreq);
	settings.setValue("centerFreq", memset->centerFreq);
	settings.setValue("preamp", memset->preamp);
	settings.setValue("ant", memset->ant);

	settings.beginWriteArray("Filter");
	for (int j=0;j<MODE_LAST-1;j++) {
		settings.setArrayIndex(j);
		settings.setValue("filterHi", memset->filterHi[j]);
		settings.setValue("filterLo", memset->filterLo[j]);
		settings.setValue("filterCut", memset->filterCut[j]);
	}
	settings.endArray();
	settings.endGroup();

	settings.beginWriteArray("Memory");
	for (int i=0;i<MAX_MEM;i++) {
		memset = &mem[i];
		memset->idx = i;
		settings.setArrayIndex(i);
		//settings.setValue("name", memset->name);
		settings.setValue("name", memListModel->stringList()[i]);
		settings.setValue("rxMode", (int)memset->rxMode);
		settings.setValue("rxFreq", memset->rxFreq);
		settings.setValue("txFreq", memset->txFreq);
		settings.setValue("centerFreq", memset->centerFreq);
		settings.setValue("preamp", memset->preamp);
		settings.setValue("ant", memset->ant);
		
		settings.beginWriteArray("Filter");
		for (int j=0;j<MODE_LAST-1;j++) {
			settings.setArrayIndex(j);
			settings.setValue("filterHi", memset->filterHi[j]);
			settings.setValue("filterLo", memset->filterLo[j]);
			settings.setValue("filterCut", memset->filterCut[j]);
		}
		settings.endArray();
	}
	settings.endArray();

	settings.beginWriteArray("Band");
	for (int i=0;i<NBUTTON;i++) {
		memset = &bandMem[i];
		memset->idx = i;
		settings.setArrayIndex(i);
		settings.setValue("name", memListModel->stringList()[i]);
		settings.setValue("rxMode", (int)memset->rxMode);
		settings.setValue("rxFreq", memset->rxFreq);
		settings.setValue("txFreq", memset->txFreq);
		settings.setValue("centerFreq", memset->centerFreq);
		settings.setValue("preamp", memset->preamp);
		settings.setValue("ant", memset->ant);
		
		settings.beginWriteArray("Filter");
		for (int j=0;j<MODE_LAST-1;j++) {
			settings.setArrayIndex(j);
			settings.setValue("filterHi", memset->filterHi[j]);
			settings.setValue("filterLo", memset->filterLo[j]);
			settings.setValue("filterCut", memset->filterCut[j]);
		}
		settings.endArray();
	}
	settings.endArray();
}

void QSdrImpl::onExit() {
	qApp->exit(0);
}

void QSdrImpl::setMemory(QModelIndex i) {
	setMemory(i.row());
}

void QSdrImpl::copyMem(MemSet *d,MemSet *s) {
	int i;

	d->rxMode = s->rxMode;
	d->rxFreq = s->rxFreq;
	d->txFreq = s->txFreq;
	d->centerFreq = s->centerFreq;
	d->preamp = s->preamp;
	d->activ = s->activ;
	d->ant = s->ant;
	for (i=0;i<MODE_LAST;i++) {
		d->filterHi[i] = s->filterHi[i];
		d->filterLo[i] = s->filterLo[i];
		d->filterCut[i] = s->filterCut[i];
	}
}

void QSdrImpl::setMemory(int n, bool band) {
	if (store) {
		if (band) 
			copyMem(&bandMem[n], &curMem);
		else
			copyMem(&mem[n], &curMem);
		store = false;
		ui->pushButtonSave->setChecked(store);
	} else {
		if (band) 
			copyMem(&curMem, &bandMem[n]);
		else
			copyMem(&curMem, &mem[n]);
		cur = &curMem;
		setCurrent();
	}
}

void QSdrImpl::setCurrent() {
	ui->comboBoxPreamp->setCurrentIndex(ui->comboBoxPreamp->findText(QString("%1").arg(cur->preamp)));
	setPreamp();

	ui->comboBoxMode->setCurrentIndex((int)cur->rxMode);
	setMode(cur->rxMode);

	ui->sliderBWHi->setValue(cur->filterHi[cur->rxMode]);
	ui->sliderBWLo->setValue(cur->filterLo[cur->rxMode]);
	ui->sliderBWCut->setValue(cur->filterCut[cur->rxMode]);
	setFilterLo(cur->filterLo[cur->rxMode]);
	setFilterHi(cur->filterHi[cur->rxMode]);
	setFilterCut(cur->filterCut[cur->rxMode]);


	setCenterFreq(cur->centerFreq);
	setRxFreq(cur->rxFreq);
	setTxFreq(cur->txFreq);
	setAntenne(cur->ant,true);
}

void QSdrImpl::setFFTSize(int n) {

	if ((unsigned int)n<sizeof(nfft)/sizeof(nfft[0]))
		fftSize = nfft[n];

	sceneFFT->removeItem(fftGraph);
	sceneFFT->removeItem(filterGraph);
	sceneFFT1->removeItem(fftGraph1);
	sceneFFT1->removeItem(filterGraph1);

	fftGraph->setFFTSize(fftSize);
	filterGraph->setFFTSize(fftSize);
	fftGraph1->setFFTSize(fftSize);
	filterGraph1->setFFTSize(fftSize);

	sceneFFT->addItem(fftGraph);
	sceneFFT->addItem(filterGraph);
	sceneFFT1->addItem(fftGraph1);
	sceneFFT1->addItem(filterGraph1);

	ui->graphicsViewFFT->setSceneRect(QRect(0, 0, fftSize, fftGraphHigh));
	ui->graphicsViewFFT->centerOn(fftSize/2, 0);
	ui->graphicsViewFFT1->setSceneRect(QRect(0, 0, fftSize, fftGraphHigh));
	ui->graphicsViewFFT1->centerOn(fftSize/2, 0);

	trxMsg->set_fftsize(fftSize);

	filterGraph->setFreq(raw ? RX_CLOCK/4 : cur->centerFreq, cur->rxFreq, cur->filterLo[cur->rxMode], cur->filterHi[cur->rxMode], cur->rxMode);
	filterGraph1->setFreq(raw ? RX_CLOCK/4 : cur->centerFreq, cur->rxFreq, cur->filterLo[cur->rxMode], cur->filterHi[cur->rxMode], cur->rxMode);
}

void QSdrImpl::setTX(bool checked, bool gui) {
    tx = checked;
	ui->treeViewSettings->setEnabled(!tx);
	if (gui)
		ui->pushButtonTX->setChecked(tx);
	trxMsg->set_ptt(tx);
	if (tx) {
		audioInDev = audioInput->start();
	} else {
		audioInput->stop();
		audioInDev = NULL;
	}
}

void QSdrImpl::ritChanged(bool checked, bool gui) {
	if (checked) {
		cur->txFreq = cur->rxFreq;
		setTxFreq(cur->txFreq);
	}
}

void QSdrImpl::setAntenne(int ant, bool gui) {
	cur->ant = ant;
	if (gui)
		ui->spinBoxAnt->setValue(ant);
	trxMsg->set_antenne(ant);
}

void QSdrImpl::setFreqStep(int step) {
	freqStep = step; 
	filterGraph->setFreqStep(freqStep);
	ui->comboBoxStep->insertItem(0,QString("%1").arg(step));
	ui->comboBoxStep->setCurrentIndex(0);
}

void QSdrImpl::on_comboBoxStep_activated(QString s) {
	freqStep = s.toInt(); 
	filterGraph->setFreqStep(freqStep);
}

void QSdrImpl::on_pushButton_clicked() {
	trxMsg->set_getfilelist(true);
}

void QSdrImpl::graphicsViewFFTSliderV(int n) {
	//filterGraph->setViewPosY(n);
}

void QSdrImpl::graphicsViewFFTSliderH(int n) {
	//filterGraph->setViewPosX(n);
	ui->graphicsViewFFT1->horizontalScrollBar()->setSliderPosition(n);
}

void QSdrImpl::graphicsViewFFTSliderV1(int n) {
	//filterGraph1->setViewPosY(n);
}

void QSdrImpl::graphicsViewFFTSliderH1(int n) {
	//filterGraph1->setViewPosX(n);
	ui->graphicsViewFFT->horizontalScrollBar()->setSliderPosition(n);
}

void QSdrImpl::on_actionAbout_triggered() {
	QMessageBox msg;
	msg.setText(QString("QSDR Version " VERSION));
	msg.exec();
}

void QSdrImpl::on_pushButtonSave_clicked() {
	store = !store;
	ui->pushButtonSave->setChecked(store);
}

void QSdrImpl::on_sliderFFT1_valueChanged(int v) {
	ui->labelMin->setText(QString("min:%1 dBm").arg(v));
	waterfallMin = v;
	fftGraph->setMin(v);
}

void QSdrImpl::on_sliderFFT2_valueChanged(int v) {
	ui->labelMax->setText(QString("max:%1 dBm").arg(v));
	waterfallMax = v;
	fftGraph->setMax(v);
}

void QSdrImpl::on_pushButtonContrast_clicked() {
	ui->sliderFFT1->setValue(fftGraph->getMin());
	ui->sliderFFT2->setValue(fftGraph->getMax());
	ui->labelMin->setText(QString("min:%1 dBm").arg(fftGraph->getMin()));
	ui->labelMax->setText(QString("max:%1 dBm").arg(fftGraph->getMax()));
}

void QSdrImpl::on_checkBox_toggled(bool checked) {
	notchOn = checked;
	trxMsg->set_notchactiv(checked);
}

void QSdrImpl::readTrxData() {
	QStringList fileList;
	int i;

	if (!conn)
		return;

	while (trxSocket->bytesAvailable()) {
		datagram += trxSocket->readAll();
		//qDebug() << "got " << datagram.size();

		while(datagram.size()>0) {
			if (datagram.left(2) != "Hq") {
				qDebug() << "sync error";
				datagram.clear();
				return;
			}
			int len = (uint8)datagram.at(2) + (uint8)datagram.at(3) * 256;
			if (datagram.size()-4 < len) {
				//qDebug() << "packet incomlete:" << datagram.size() << len;
				break;
			}

			TrxProto::GuiMessage guiMsg;
			if (guiMsg.ParseFromArray(datagram.mid(4).data(), len)) {

				if (guiMsg.has_audio()) {
					int rxAudioBufferSize = sdrSettings->getIntVal(QString("Interconnect,AudioBufferSize"));
					short sample;
					const char *in = guiMsg.audio().data();
					if (audioOutBuf.size()>rxAudioBufferSize)
						audioOutBuf = audioOutBuf.right(rxAudioBufferSize);
					for (int i=0;i<(int)guiMsg.audio().size();i++) {
						sample = ulaw2linear(in[i]);
						audioOutBuf.append((char*)&sample, sizeof(sample));
						if (!tx)
							audioGraph->audioData(sample);
					}
				}

				if (guiMsg.has_rssi()) {
					smtr->setVal(guiMsg.rssi());
					ui->graphicsViewSMeter->fitInView(smtr->boundingRect());
				}

				if (guiMsg.has_fft()) {
					fftGraph->fftDataReady(QByteArray(guiMsg.fft().data(), guiMsg.fft().size()));
					fftGraph1->fftDataReady(QByteArray(guiMsg.fft().data(), guiMsg.fft().size()));
				}

				if (guiMsg.has_displaytext()) {
					appendText(QByteArray(guiMsg.displaytext().data(), guiMsg.displaytext().size()));
				}

				if (guiMsg.has_restartind()) {
					QString s = sdrSettings->printTreePath();
					trxMsg->Clear();
					trxMsg->set_settings(s.toStdString().data(), s.toStdString().size());
					trxMsg->set_samplerate(sampleRate);
					trxMsg->set_fftsize(fftSize);
					trxMsg->set_vfo(activVFO);
					trxMsg->set_vfoactiv(cur->activ);
					setVolume(volume);
					setCurrent();
					qDebug() << "server restarted" ;
				}

				for(i=0;i<guiMsg.files_size();i++)
					fileList << guiMsg.files(i).data();
				if (fileList.size()>0) {
					playDialog = new play(trxMsg, fileList);
					playDialog->show();
					connect(playDialog, SIGNAL(playFile(QString)), this, SLOT(setPlayFile(QString)));
					connect(playDialog, SIGNAL(closed()), this, SLOT(playClosed()));
				}

				if (guiMsg.has_playfilelen()) {
					if (playDialog) {
						playDialog->setPlayFileLen(guiMsg.playfilelen());
					}
				}
				if (guiMsg.has_playfilepos()) {
					if (playDialog) {
						playDialog->setPlayFilePos(guiMsg.playfilepos());
					}
				}
				datagram = datagram.mid(len+4);
				//qDebug() << datagram.size();
			} else {
				qDebug() << "qsdr: Failed to parse." ;
				datagram.clear();
				return;
			}
		}
	}
}

void QSdrImpl::readHamLibUDPData() {
	while (hamLibSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(hamLibSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		QRegExp rx("(\\S) *(\\S*)");

		hamLibSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
		if (rx.indexIn(datagram)!=-1) {

			QString tag = rx.cap(1);
			QString arg = rx.cap(2);
			qint64 iarg = arg.toLong();

			if (tag == "f") {
				setRxFreq(iarg);
			}
			if (tag == "F") {
				datagram.clear();
				datagram.append(QString("f %1\n").arg(cur->rxFreq,12));
				hamLibSocket->writeDatagram(datagram, sender, senderPort);
			}
			if (tag == "t") {
				setTX(iarg>0);
			}
		}
	}
}

bool QSdrImpl::isTx() {
	switch(sdrSettings->getIntVal(QString("Device,Device"))){
		case SRC_HIQSDR:
		case SRC_USRP:
		case SRC_TESTSIGNAL:
			return true;
		case SRC_OSMOSDR:
		case SRC_NULL:
		case SRC_FILE:
			return false;
	};
	return false;
}

QStringList QSdrImpl::supportedSampleRate() {
	QStringList list;
	switch(sdrSettings->getIntVal(QString("Device,Device"))){
		case SRC_HIQSDR:
			list <<  "48000";
			list <<  "96000";
			list <<  "192000";
			list <<  "240000";
			list <<  "384000";
			list <<  "480000";
			list <<  "960000";
			list <<  "1536000";
			list <<  "1920000";
			list <<  "61440000";
			break;
		case SRC_USRP:
			list <<  "48000";
			list <<  "96000";
			list <<  "192000";
			list <<  "240000";
			list <<  "384000";
			list <<  "480000";
			list <<  "960000";
			list <<  "1920000";
			list <<  "3840000";
			list <<  "7680000";
			list <<  "10240000";
			list <<  "15360000";
			list <<  "30720000";
			break;
		case SRC_OSMOSDR:
			list << "960000";
			list << "1440000";
			list << "1920000";
			list << "2400000";
			list << "2880000";
			break;
		case SRC_NULL:
		case SRC_TESTSIGNAL:
		case SRC_FILE:
			list <<  "48000";
			list <<  "60000";
			list <<  "96000";
			list <<  "120000";
			list <<  "192000";
			list <<  "240000";
			list <<  "320000";
			list <<  "384000";
			list <<  "480000";
			list <<  "640000";
			list <<  "960000";
			list <<  "1024000";
			list <<  "1280000";
			list <<  "1536000";
			list <<  "1920000";
			list <<  "2048000";
			list <<  "2560000";
			list <<  "3096000";
			list <<  "3840000";
			break;
	};
	return list;
}

void QSdrImpl::settingsChanged() {

	if ((ERxSink)sdrSettings->getIntVal("Trx,RX,Sink") == RX_SINK_IP) {
		audioOutDev = audioOutput->start();
		audioInDev = NULL;
	} else {
		audioOutput->stop();
		audioOutDev = NULL;
	}

	for (int i=0;i<NBUTTON;i++) {
		if (((sdrSettings->getIntVal(QString("Band,%1,min").arg(i+1))) <= cur->rxFreq) && ((sdrSettings->getIntVal(QString("Band,%1,max").arg(i+1))) >= cur->rxFreq)) {
			bandButtons[i]->setPalette( QPalette(QColor(sdrSettings->getVal("Display,Color,App,Button"))));
		} else {
			bandButtons[i]->setPalette( QPalette(QColor(sdrSettings->getVal("Display,Color,App,ButtonInactiv"))));
		}
	}

	QPalette p(palette());
	p.setColor(QPalette::Window,        QColor(sdrSettings->getVal("Display,Color,App,Win")));
	p.setColor(QPalette::WindowText,    QColor(sdrSettings->getVal("Display,Color,App,WindowText")));
	p.setColor(QPalette::Base,          QColor(sdrSettings->getVal("Display,Color,App,Base")));
	p.setColor(QPalette::AlternateBase, QColor(sdrSettings->getVal("Display,Color,App,AlternateBase")));
	p.setColor(QPalette::Text,          QColor(sdrSettings->getVal("Display,Color,App,Text")));
	p.setColor(QPalette::Button,        QColor(sdrSettings->getVal("Display,Color,App,Button")));
	p.setColor(QPalette::ButtonText,    QColor(sdrSettings->getVal("Display,Color,App,ButtonText")));
	p.setColor(QPalette::BrightText,    QColor(sdrSettings->getVal("Display,Color,App,BrightText")));
	this->setAutoFillBackground(true);
	this->setPalette(p);

	

	fftGraph->settingsChanged();
	filterGraph->settingsChanged();
	fftGraph1->settingsChanged();
	filterGraph1->settingsChanged();
	
	ui->comboBoxSampleRate->clear();
	ui->comboBoxSampleRate->addItems(supportedSampleRate());
	ui->comboBoxSampleRate->setCurrentIndex(ui->comboBoxSampleRate->findText(QString("%1").arg(sampleRate)));

	bool tx = isTx();
	ui->groupBoxTx->setEnabled(tx);
	ui->lcdNumberTX->setEnabled(tx);

	rxAudioBufferSize = sdrSettings->getIntVal(QString("Interconnect,AudioBufferSize"));
	QString s = sdrSettings->printTreePath();
	trxMsg->set_settings(s.toStdString().data(), s.toStdString().size());
}

void QSdrImpl::on_comboBoxVFO_activated(int index) {

	copyMem(&vfoMem[activVFO], &curMem);
	activVFO = index;
	cur->activ = false;
	if (vfoMem[activVFO].centerFreq > 0)
		copyMem(&curMem, &vfoMem[activVFO]);
	ui->checkBoxVFOActiv->setChecked(cur->activ);

	trxMsg->set_vfo(activVFO);
	trxMsg->set_vfoactiv(cur->activ);
	setCurrent();
}

void QSdrImpl::on_checkBoxVFOActiv_clicked(bool checked) {
	cur->activ = checked;
	trxMsg->set_vfoactiv(cur->activ);
}

void QSdrImpl::on_pushButtonMute_clicked(bool checked) {
	setVolume(volume);
}

void QSdrImpl::sendTrxMsg() {
	uint8 header[4];

	if (!conn || trxMsg->ByteSize() == 0) 
		return;
	std::ostringstream out;
	trxMsg->SerializeToOstream(&out);
	QByteArray byteArray(out.str().c_str(), out.str().size());
	//QByteArray sig = HashSHA256(byteArray+hashKey);
	//qDebug() << "qsdr send:" << byteArray.size();

	strcpy((char*)header, "Hq");
	header[2] = byteArray.size() & 0xff;
	header[3] = (byteArray.size() >> 8) & 0xff;
	trxSocket->write(QByteArray((const char*)header, sizeof(header))+byteArray);
	trxMsg->Clear();
}

void QSdrImpl::setPlayFile(QString fileName) {
	QRegExp rx(".*_.*_(\\d+)_(\\d+).raw");
	int fr,sr;
	if (rx.indexIn(fileName)!=-1) {
		fr = rx.cap(1).toLong();
		sr = rx.cap(2).toInt();
		freqChanged(fr);
		setSampleRate(sr);
		ui->comboBoxSampleRate->setCurrentIndex(ui->comboBoxSampleRate->findText(QString("%1").arg(sr)));
		trxMsg->set_playfilename(fileName.toAscii().data());
		trxMsg->set_playfile(true);
	} 
}

void QSdrImpl::playClosed() {
	delete playDialog;
	playDialog = NULL;
}

void QSdrImpl::displayError(QAbstractSocket::SocketError error) {
	qDebug() << "TCP Error" << error;
}

void QSdrImpl::connected() {
	qDebug() << "TCP: conneced";
	conn = true;
}

void QSdrImpl::disconnected() {
	qDebug() << "TCP: disconneced";
	conn = false;
}

void QSdrImpl::on_actionSave_image_triggered()
{
	QString filename = QFileDialog::getSaveFileName(this,
			      tr("Save Image"), "", tr("Image Files (*.png *.jpg)"));
	if (filename.isEmpty())
		return;
	if (!filename.endsWith(".png") && (!filename.endsWith("*.jpg")))
			filename.append(".png");
	qDebug() << filename;
	int xSize = raw ? fftSize/2 : fftSize;
	QImage image(xSize, fftGraphHigh, QImage::Format_RGB32);
	QPainter p(&image);
	sceneFFT->render(&p, QRect(0, 0, xSize, fftGraphHigh), QRect(0,0, xSize, fftGraphHigh));
	image.save(filename);
}
