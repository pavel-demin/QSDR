#include "play.h"
#include "ui_play.h"

play::play(TrxProto::TrxMessage *msg, QStringList fileList, QWidget *parent) :  QDialog(parent), ui(new Ui::play)
{
	trxMsg = msg;
    ui->setupUi(this);
	fileListModel = new QStringListModel(fileList);
	ui->listView->setModel(fileListModel);
	pos = 0;
}

play::~play()
{
    delete ui;
}

void play::on_pushButtonStop_clicked() {
	trxMsg->set_playfile(false);
}

void play::on_pushButtonExit_clicked() {
	trxMsg->set_playfile(false);
	emit closed();
}

void play::on_horizontalSlider_sliderMoved(int position) {
	trxMsg->set_filepos(position);
	ui->dateTimeEdit->setDateTime(minTime.addSecs(position));
}

void play::on_listView_clicked(const QModelIndex &index) {
	QRegExp rx("(.*)_(.*)_(\\d+)_(\\d+).raw");
	QString file = fileListModel->stringList().at((index.row()));
	emit playFile(file);
	if (rx.indexIn(file)!=-1) {
		QString	date = rx.cap(1);
		QString	time = rx.cap(2);
		minTime.setDate(QDate::fromString(date,"yy.MM.dd").addYears(100));
		minTime.setTime(QTime::fromString(time,"hh:mm:ss"));
		maxTime = minTime;
		ui->dateTimeEdit->setMinimumDateTime(minTime);
		ui->dateTimeEdit->setMaximumDateTime(maxTime);
	}

}

void play::setPlayFileLen(quint64 len) {
	ui->horizontalSlider->setMaximum(len);
	maxTime = minTime.addSecs(len);
	ui->dateTimeEdit->setMaximumDateTime(maxTime);
	QTime t(0,0,0);
	ui->label->setText(t.addSecs(len).toString());

}

void play::setPlayFilePos(quint64 len) {
	qDebug() << len;
	if (pos == len)
		return;
	pos = len;
	ui->horizontalSlider->setSliderPosition(len);
	ui->dateTimeEdit->setDateTime(minTime.addSecs(len));
}
