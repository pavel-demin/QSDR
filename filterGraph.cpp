#include "filterGraph.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include "config.h"
#include "fftGraph.h"

#define MIN(a,b)  a>b ? b:a

FilterGraph::FilterGraph(settings *s, int x, int y) {
    xSize = x; 
    ySize = y;
    fftSize = x;
	sdrSettings = s;

    freqStep = 100;

    centerFreq = 0;
    freq = 0;

    xViewPos = 0;
    yViewPos = 0;

    this->setPos(QPoint(xViewPos,yViewPos));

    filterPixmap = QPixmap(xSize/8, ySize/8);
    filterPixmap.fill(QColor(255, 255, 255, shadow));
	settingsChanged();
    
    this->setAcceptHoverEvents(true);
}

FilterGraph::~FilterGraph() {
}

void FilterGraph::settingsChanged() {
	colorSpecFreq = QColor(sdrSettings->getVal("Display,Color,Spectrum,freq"));
	colorSpecGrid = QColor(sdrSettings->getVal("Display,Color,Spectrum,grid"));
	fft_scale =  sdrSettings->getIntVal    ("Display,Spectrum,scale");
	shadow = sdrSettings->getIntVal("Display,Color,shadow");
    filterPixmap.fill(QColor(255, 255, 255, shadow));
	update();
}

void FilterGraph::setFFTSize(int size) {
    xSize = size;
    fftSize = size;

    xViewPos = 0;
    yViewPos = 0;
    this->setPos(QPoint(xViewPos,yViewPos));
}

void FilterGraph::setDisplayMode(int m) {
    dispMode = m;
}

void FilterGraph::paint (QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->drawPixmap(MIN(filterLoMarkerPos-xViewPos, filterHiMarkerPos-xViewPos),0,filterPixmap);
    painter->setPen(Qt::white);
    painter->drawText(10,30,QString ("Center %1").arg(centerFreq));

	painter->setPen(colorSpecFreq);
    painter->drawLine(fftFreqMarkerPos-xViewPos, 0, fftFreqMarkerPos-xViewPos, ySize);

    qint64 marker;
    int st = sampleRate/fftSize*100;
    int step=1000; // min 1kHz
    while(step*10 < st) step*=10;
    while(step*5 < st) step*=5;
    while(step*2 < st) step*=2;

	qint64 start = (centerFreq-sampleRate) - (centerFreq-sampleRate) % step ;
	for (marker=start;marker<centerFreq+sampleRate;marker+=step) {
		qint64 x = (qint64)(marker - centerFreq)*(fftSize/2)/(sampleRate/2) + fftSize/2;
		painter->drawLine(x-xViewPos,0,x-xViewPos,20);
		painter->drawText(x-xViewPos+2,11,QString ("%1").arg(int((marker+500)/1000)));

	}

    qint64 f = centerFreq + (mousePos.x() + xViewPos - xSize/2) * (sampleRate/2) / (xSize/2);
    int s = (mousePos.y()+yViewPos)/fft_scale;

    switch (dispMode) {
	case GRAPH_FFT:
		painter->setPen(colorSpecGrid);

	    for (int i=0;i<=160;i+=10) {
			int y = i*fft_scale;
			painter->drawLine(0,y - yViewPos, xSize, y - yViewPos);
			painter->drawText(0,y - yViewPos, QString("-%1 dBm").arg(i));
	    }
	    painter->drawText(mousePos,QString("%1.%2 kHz, -%3 dBm").arg((int)(f/1000)).arg((int)(f % 1000),3,10,QChar('0')).arg(s));
	    break;
	case GRAPH_WATERFALL:
	    painter->setPen(QColor(255,255,255));
	    painter->drawText(mousePos,QString("%1.%2 kHz").arg((int)(f/1000)).arg((int)(f % 1000),3,10,QChar('0')));
	    break;
    }
}

QRectF FilterGraph::boundingRect() const {
    return QRectF(0, 0, xSize, ySize);
}

void FilterGraph::setFreq(qint64 cf, qint64 f, int fl, int fh, int m) {
    centerFreq = cf;
    freq = f;
    filterLo = fl;
    filterHi = fh;
	mode = m;

    fftFreqMarkerPos = (long long int)(freq - centerFreq)*(fftSize/2)/(sampleRate/2) + fftSize/2;
    switch (mode) {
	case MODE_USB:
	    filterLoMarkerPos = (long long int)(freq + filterLo - centerFreq)*(fftSize/2)/(sampleRate/2) + fftSize/2;
	    filterHiMarkerPos = (long long int)(freq + filterHi - centerFreq)*(fftSize/2)/(sampleRate/2) + fftSize/2;
	    break;
	case MODE_LSB:
	    filterLoMarkerPos = (long long int)(freq - filterLo - centerFreq)*(fftSize/2)/(sampleRate/2) + fftSize/2;
	    filterHiMarkerPos = (long long int)(freq - filterHi - centerFreq)*(fftSize/2)/(sampleRate/2) + fftSize/2;
	    break;
	case MODE_AM:
	case MODE_NFM:
	case MODE_WFM_RDS:
	    filterLoMarkerPos = (long long int)(freq - filterHi - centerFreq)*(fftSize/2)/(sampleRate/2) + fftSize/2;
	    filterHiMarkerPos = (long long int)(freq + filterHi - centerFreq)*(fftSize/2)/(sampleRate/2) + fftSize/2;
	    break;
	case MODE_CW:
	    filterLoMarkerPos = (long long int)(freq - filterLo - centerFreq + CW_TONE)*(fftSize/2)/(sampleRate/2) + fftSize/2;
	    filterHiMarkerPos = (long long int)(freq - filterHi - centerFreq + CW_TONE)*(fftSize/2)/(sampleRate/2) + fftSize/2;
	    break;
	default:
	    filterLoMarkerPos = 0;
	    filterHiMarkerPos = 0;
	    break;
    }
    filterPixmap =  QPixmap(abs(filterLoMarkerPos-filterHiMarkerPos), ySize);
    filterPixmap.fill(QColor(255, 255, 255, shadow));
}

void FilterGraph::setAudioFormat(int sf) {
    sampleRate = sf;
}

void FilterGraph::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    freq = centerFreq + (event->pos().x() + xViewPos - xSize/2) * (sampleRate/2) / (xSize/2);
    emit freqChanged(freq);
}

void FilterGraph::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    freq = centerFreq + (event->pos().x() + xViewPos - xSize/2) * (sampleRate/2) / (xSize/2);
    emit freqChanged(freq);
}

void FilterGraph::wheelEvent(QGraphicsSceneWheelEvent *event) {
    freq +=  event->delta()/120*freqStep;
    freq -= freq % freqStep;
    emit freqChanged(freq);
}

void FilterGraph::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    mousePos =  event->pos();
}

void FilterGraph::setFreqStep(int step) {
    freqStep = step;
}

void FilterGraph::setViewPosX(int x) {
    xViewPos = x;
    this->setPos(QPoint(xViewPos,yViewPos));
}

void FilterGraph::setViewPosY(int y) {
    yViewPos = y;
    this->setPos(QPoint(xViewPos,yViewPos));
}
