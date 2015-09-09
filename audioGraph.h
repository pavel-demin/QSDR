#include <QGraphicsItem>
#include <QTimer>
#include <QWheelEvent>
#include "config.h"
#include "settings.h"


class AudioGraph: public QObject, public QGraphicsItem  {
    Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
    public:
	AudioGraph(settings *,int x,int y);
	~AudioGraph();
	QRectF boundingRect() const;
    public slots:
	void audioData(short); 
	void setMin(int v);
	void setMax(int v);
	int getMin();
	int getMax();
	void setDisplayMode(int m);
	void setFFTSize(int size);
	void settingsChanged();

    private:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void updateLine();
	short audioBuf[1024];

//	float fftmax[MAX_FFT];
//	float fftav[MAX_FFT];
//	float v1[MAX_FFT];
	QColor colorTab[256];
	QPixmap *fftPixmap;

	int audioPos;
	int nFFT;
	int xSize, ySize;
	float base,scale;
	int displayMode;
	float mi,ma;
	float tmi,tma;
	bool setAuto;
	settings *sdrSettings;
	int average_time_max,average_time;
	QColor colorSpecMax,colorSpecAverage,colorSpecBack;
	int fft_scale;
	double specCal;
};
