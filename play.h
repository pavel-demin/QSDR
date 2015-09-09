#ifndef PLAY_H
#define PLAY_H

#include <QDialog>
#include <QtGui>
#include "trx.pb.h"

namespace Ui {
class play;
}

class play : public QDialog
{
    Q_OBJECT
    
public:
    explicit play(TrxProto::TrxMessage *msg, QStringList files, QWidget *parent = 0);
    ~play();
	void setPlayFileLen(quint64);
	void setPlayFilePos(quint64);
    
signals:
	void playFile(QString);
	void closed();
private slots:

    void on_pushButtonStop_clicked();
    void on_pushButtonExit_clicked();
    void on_horizontalSlider_sliderMoved(int position);
    void on_listView_clicked(const QModelIndex &index);

private:
    Ui::play *ui;
	QStringList fileNames;
	QStringListModel* fileListModel;
	TrxProto::TrxMessage *trxMsg;
	QDateTime minTime,maxTime;
	int pos;
};

#endif // PLAY_H
