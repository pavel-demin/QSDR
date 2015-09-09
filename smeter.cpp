#include "smeter.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <math.h>

SMeter::SMeter(int x, int y) {
    xSize = 120; 
    ySize = 40;
}

SMeter::~SMeter() {
}

void SMeter::paint (QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    int i;
    painter->setFont(QFont("Arial", 6));
    painter->setPen(Qt::black);
    for (i=1;i<10;i++) {
	painter->drawLine(i*6,23,i*6,40);
	if (i&1)
	    painter->drawText(i*6-3,20,QString("%1").arg(i));
    }
    painter->setPen(Qt::red);
    for (i=0;i<7;i++) {
	painter->drawLine(64+i*10,23,64+i*10,40);
	if (i&1)
	    painter->drawText(64+i*10-8,20,QString("+%1").arg(i*10+10));

    }
    painter->setPen(Qt::black);
    painter->drawText(10,10,QString("%1 dBm").arg(int(val)));
    painter->setPen(Qt::green);
    painter->setBrush(Qt::green);
    painter->drawRect(0,32,127+12+val,40);


}

QRectF SMeter::boundingRect() const {
    return QRectF(0, 0, xSize, ySize);
}

void SMeter::setVal(float f) {
    val = f;
    update();
}




