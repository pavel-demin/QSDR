#include "fftGraph.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include <stdlib.h>
#include "lib.h"

using namespace std;

FFTGraph::FFTGraph(settings *s, int x, int y) {
	int i;

	xSize = x; 
	ySize = y;
	sdrSettings = s;
	displayMode = GRAPH_WATERFALL;
	nFFT = x;
	fftPixmap = new QPixmap(xSize, ySize);
	fftPixmap->fill(Qt::black);
	mi = -300/20; 
	tmi = mi; 
	ma = 0;
	tma = ma;

	settingsChanged();

	for(i=0; i < MAX_FFT; i++) {
		fftmax[i]=0;
		fftav[i]=0;
	}
	base = 3;
	scale = 40;
	setAuto = false;
}

FFTGraph::~FFTGraph() {
}

void FFTGraph::settingsChanged() {
	double h,s,v;
	average_time_max = sdrSettings->getIntVal    ("Display,Spectrum,averageTimeMax");
	average_time = sdrSettings->getIntVal        ("Display,Spectrum,averageTime");	
	colorSpecMax = QColor(sdrSettings->getVal    ("Display,Color,Spectrum,maximum"));
	colorSpecAverage = QColor(sdrSettings->getVal("Display,Color,Spectrum,average"));
	colorSpecBack = QColor(sdrSettings->getVal("Display,Color,Spectrum,background"));
	fft_scale =  sdrSettings->getIntVal    ("Display,Spectrum,scale");
	specCal =  sdrSettings->getDoubleVal    ("Display,Spectrum,offset");

	int mode= sdrSettings->getIntVal("Display,Color,Waterfall,mode");
	double h_min = sdrSettings->getDoubleVal("Display,Color,Waterfall,h_min");
	double h_max = sdrSettings->getDoubleVal("Display,Color,Waterfall,h_max");
	double s_min = sdrSettings->getDoubleVal("Display,Color,Waterfall,s_min");
	double s_max = sdrSettings->getDoubleVal("Display,Color,Waterfall,s_max");
	double v_min = sdrSettings->getDoubleVal("Display,Color,Waterfall,v_min");
	double v_max = sdrSettings->getDoubleVal("Display,Color,Waterfall,v_max");

	for( int i=0; i<256; i++) {
		switch (mode) {
			case 0:
				colorTab[i].setRgb(i,i,i);
				break;
			case 1:
				if( (i<43) )             colorTab[i].setRgb( 0,0, 255*(i)/43);
				if( (i>=43) && (i<87) )  colorTab[i].setRgb( 0, 255*(i-43)/43, 255);
				if( (i>=87) && (i<120))  colorTab[i].setRgb( 0,255, 255-(255*(i-87)/32));
				if( (i>=120) && (i<154)) colorTab[i].setRgb( (255*(i-120)/33), 255, 0);
				if( (i>=154) && (i<217)) colorTab[i].setRgb( 255, 255 - (255*(i-154)/62), 0);
				if( (i>=217))            colorTab[i].setRgb( 255, 0, 128*(i-217)/38);
				break;
			case 2:
				h = (h_max-h_min)*i/256+h_min;
				if (h>1) h-=1;
				s = (s_max-s_min)*i/256+s_min;
				if (s>1) s-=1;
				v = (v_max-v_min)*i/256+v_min;
				if (v>1) v-=1;
				colorTab[i].setHsvF(h,s,v);
				break;
		}
	}
}


void FFTGraph::paint (QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
	painter->drawPixmap(0,0,*fftPixmap);
}

void FFTGraph::setFFTSize(int size) {
	nFFT = size;
	xSize = size;
	delete fftPixmap;
	fftPixmap = new QPixmap(xSize, ySize);
	fftPixmap->fill(Qt::black);
}

QRectF FFTGraph::boundingRect() const {
	return QRectF(0, 0, xSize, ySize);
}

void FFTGraph::setMin(int v) {	// dbm
	tmi = v/20.0; 
	if (tmi>=tma) tma = tmi + 0.1;
	base = -tmi;
	scale = 256/(tma-tmi);
}

void FFTGraph::setMax(int v) {   // dbm
	tma = v/20.0;
	if (tmi>=tma) tmi = tma - 0.1;
	base = -tmi;
	scale = 256/(tma-tmi);
}

void FFTGraph::setDisplayMode(int m) {
// TODO
	for(int i=0; i < MAX_FFT; i++) {
		fftmax[i]=-10;
		fftav[i]=0;
	}
	displayMode = m;
}

void FFTGraph::fftDataReady(QByteArray data) {
	int i;
	float v;

	mi = 0;
	ma = -10;

	for(i=0; i < data.size(); i++) {
		v = (unsigned char)data.at(i);
		v = (v-FFT_OFFSET)/20.0/2.0;
		v += specCal/20.0;
		v1[i] = v;
		if (fftmax[i]<v)
			fftmax[i] = v;
		fftmax[i] = ((average_time_max-1)*fftmax[i]+v)/average_time_max;
		if (fftav[i]==0)
			fftav[i] = v;
		else
			fftav[i] = ((average_time-1)*fftav[i]+v)/average_time;

		if (i>data.size()/16 && i<data.size()*15/16)
			mi = min(fftmax[i],mi);
		ma = max(fftmax[i],ma);
	}

	QPainter painter(fftPixmap);
	switch (displayMode) {
		case GRAPH_WATERFALL:
			fftPixmap->scroll(0,1,fftPixmap->rect());
			for(i=0; i < data.size(); i++) {
				v = (v1[i]+base)*scale;
				if (v>255) v=255;
				if (v<0)   v=0;
				painter.setPen(colorTab[(unsigned char)v]);
				painter.drawPoint(i,0);
			}
			if(setAuto) {
				base = -mi;
				scale = 256/(ma-mi);
				setAuto = false;
			}

			break;
		case GRAPH_FFT:
			fftPixmap->fill(colorSpecBack);
			// setPen kostet rechenzeit, daher jede Schleife einzeln
			painter.setPen(colorSpecMax);
			for(i=1; i < data.size(); i++) 
				painter.drawLine(i-1,-fftmax[i-1]*20*fft_scale,i,-fftmax[i]*20*fft_scale);
			painter.setPen(colorSpecAverage);
			for(i=1; i < data.size(); i++) 
				painter.drawLine(i-1,-fftav[i-1]*20*fft_scale,i,-fftav[i]*20*fft_scale);
			painter.setPen(QColor(128,128,128));
			painter.drawLine(0,-mi*20*fft_scale,xSize,-mi*20*fft_scale);
			painter.drawLine(0,-ma*20*fft_scale,xSize,-ma*20*fft_scale);
			break;
		default:
			;
	}
	update();
}


void FFTGraph::setAutomaticCB() {
	setAuto = true;
}

int FFTGraph::getMin() {
	return mi*20.0;
}

int FFTGraph::getMax() {
	return ma*20.0;
}

