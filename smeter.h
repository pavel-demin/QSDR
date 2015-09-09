#include <QGraphicsItem>


class SMeter: public QGraphicsItem  {
    public:
	SMeter(int x,int y);
	~SMeter();
	QRectF boundingRect() const;
	void setVal(float f);

    private:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	int xSize,ySize;
	float val;
};
