#include <QGraphicsItem>
#include <QTimer>
#include <QWheelEvent>
#include "config.h"
#include "settings.h"

#define GRAPH_WATERFALL	0
#define GRAPH_FFT		1
#define GRAPH_NONE		2

class FFTGraph: public QObject, public QGraphicsItem  {
    Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
    public:
	FFTGraph(settings *,int x,int y);
	~FFTGraph();
	QRectF boundingRect() const;
    public slots:
	void fftDataReady(QByteArray); 
	void setMin(int v);
	void setMax(int v);
	int getMin();
	int getMax();
	void setDisplayMode(int m);
	void setFFTSize(int size);
	void setAutomaticCB();
	void settingsChanged();

    private:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void updateLine();

	float fftmax[MAX_FFT];
	float fftav[MAX_FFT];
	float v1[MAX_FFT];
	QColor colorTab[256];
	QPixmap *fftPixmap;

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
