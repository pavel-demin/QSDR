#include "audioGraph.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include <stdlib.h>
#include "lib.h"

using namespace std;

AudioGraph::AudioGraph(settings *s, int x, int y) {
	int i;

	xSize = x; 
	ySize = y;
	sdrSettings = s;
	//displayMode = GRAPH_WATERFALL;
	nFFT = x;
	fftPixmap = new QPixmap(xSize, ySize);
	fftPixmap->fill(Qt::black);
	mi = -300/20; 
	tmi = mi; 
	ma = 0;
	tma = ma;
	audioPos = 0;

	settingsChanged();

	base = 3;
	scale = 40;
	setAuto = false;
}

AudioGraph::~AudioGraph() {
}

void AudioGraph::settingsChanged() {
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


void AudioGraph::paint (QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
	painter->drawPixmap(0,0,*fftPixmap);
}

void AudioGraph::setFFTSize(int size) {
	nFFT = size;
	xSize = size;
	delete fftPixmap;
	fftPixmap = new QPixmap(xSize, ySize);
	fftPixmap->fill(Qt::black);
}

QRectF AudioGraph::boundingRect() const {
	return QRectF(0, 0, xSize, ySize);
}

void AudioGraph::setMin(int v) {	// dbm
	tmi = v/20.0; 
	if (tmi>=tma) tma = tmi + 0.1;
	base = -tmi;
	scale = 256/(tma-tmi);
}

void AudioGraph::setMax(int v) {   // dbm
	tma = v/20.0;
	if (tmi>=tma) tmi = tma - 0.1;
	base = -tmi;
	scale = 256/(tma-tmi);
}

void AudioGraph::setDisplayMode(int m) {
// TODO
	displayMode = m;
}

void AudioGraph::audioData(short sample) {
	int i;

	audioBuf[audioPos++]=sample;
	if (audioPos>=xSize) {
		fftPixmap->fill(colorSpecBack);
		QPainter painter(fftPixmap);
		painter.setPen(colorSpecAverage);
		for(i=1; i < xSize; i++) 
			painter.drawLine(i-1,ySize/2 + 1L*audioBuf[i-1]*ySize/2/32768,i,ySize/2 + 1L*audioBuf[i]*ySize/2/32768);
		update();
		audioPos = 0;
	}
}


int AudioGraph::getMin() {
	return mi*20.0;
}

int AudioGraph::getMax() {
	return ma*20.0;
}

