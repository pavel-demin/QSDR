#include <QGraphicsItem>
#include <QWidget>
#include "config.h"
#include "settings.h"

class FilterGraph: public QObject, public QGraphicsItem  {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
	public:
	FilterGraph(settings*, int x,int y);
	~FilterGraph();
	QRectF boundingRect() const;
	void setFreq(qint64 cf, qint64 f, int fl, int fh, int m);
	void setFFTSize(int size);
	void setAudioFormat (int sampleRate);
	void setDisplayMode(int m);
	void setFreqStep(int step);
	void setViewPosX(int);
	void setViewPosY(int);
	void settingsChanged();
    signals:
        void freqChanged(qint64 freq);
    private:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void wheelEvent( QGraphicsSceneWheelEvent *event);
	void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
	QPixmap *filterPixmap1, *filterPixmap2;
	QPixmap filterPixmap;
	QPointF mousePos;
	qint64 fftFreqMarkerPos;
	int filterLoMarkerPos, filterHiMarkerPos;

	int xSize, ySize;
	int xViewPos, yViewPos;
	int sampleRate;
	qint64 freq,centerFreq;
	int filterLo,filterHi;
	int fftSize;
	int dispMode;
	int mode;
	int freqStep;
	settings *sdrSettings;
	QColor colorSpecFreq, colorSpecGrid;
	int fft_scale, shadow;
};
