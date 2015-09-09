#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QAudioInput>
#include "settings.h"
#include "lib.h"


settings::settings(QTreeView *w, QObject *parent) :
    QObject(parent)
{
	int i;
	QString set;
	view = w;

	QString audioIn,audioOut;
	QStringList headers;
	headers << tr("Setting") << tr("Value");

	QFile file(":/settings.txt");
	file.open(QIODevice::ReadOnly);
	set = file.readAll();
	set.replace("[AUDIODEV]",getAlsaAudioDevices().join(";").replace(",","_").replace(",","_").replace(";",","));

	QList<QAudioDeviceInfo> audioDevicesOut =  QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	for (i=0;i<audioDevicesOut.size();i++) {
		if (i>0) audioOut.append(",");
		audioOut.append(audioDevicesOut.at(i).deviceName());
	}
	set.replace("[QTAUDIODEVOUT]",audioOut);

	QList<QAudioDeviceInfo> audioDevicesIn =  QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	for (i=0;i<audioDevicesIn.size();i++) {
		if (i>0) audioIn.append(",");
		audioIn.append(audioDevicesIn.at(i).deviceName());
	}
	set.replace("[QTAUDIODEVIN]",audioIn);

	model = new TreeModel(headers, set);
	file.close();

	view->setModel(model);
	view->setItemDelegate(new ItemDelegate());
	for (int column = 0; column < model->columnCount(); ++column)
		view->resizeColumnToContents(column);
	view->setColumnWidth(0,180);

	modelIndex = view->rootIndex();
	model = view->model();

	connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(changed(QModelIndex,QModelIndex)));
}


void settings::changed (QModelIndex, QModelIndex) {
	emit changed();
}

QString settings::printTree() {
	return printTreeInt(model, modelIndex, "");
}

QString settings::printTreeInt(QAbstractItemModel *model, QModelIndex modelIndex, QString header) {
	int i;
	QString s;
	for(i=0;i<model->rowCount(modelIndex);i++) {
		if (model->index(i,1,modelIndex).data().toString().size()>0)
			s += QString("%1%2\t%3\n").arg(header)
				.arg(model->index(i,0,modelIndex).data().toString())
				.arg(model->index(i,1,modelIndex).data().toString());
		else
			s += QString("%1%2\n").arg(header)
				.arg(model->index(i,0,modelIndex).data().toString());
		s += printTreeInt(model, model->index(i,0, modelIndex), header+"  ");
	}
	return s;
}

QString settings::printTreePath() {
	return printTreePathInt(model, modelIndex, "");
}

QString settings::printTreePathInt(QAbstractItemModel *model, QModelIndex modelIndex, QString header) {
	int i;
	QString s;
	for(i=0;i<model->rowCount(modelIndex);i++) {
		if (model->index(i,1,modelIndex).data().toString().size()>0) {
			s += QString("%1%2\t%3\n")
				.arg(header)
				.arg(model->index(i,0,modelIndex).data().toString())
				.arg(model->index(i,1,modelIndex).data().toString().split(",")[1]);
		}
		s += printTreePathInt(model, model->index(i,0, modelIndex), header+model->index(i,0,modelIndex).data().toString()+",");
	}
	return s;
}

QString settings::getVal(QStringList path) {
	return getValInt(model, modelIndex, path);
}

QString settings::getVal(QString s) {
	return getValInt(model, modelIndex, s.split(","));
}

int settings::getIntVal(QString s) {
	return getValInt(model, modelIndex, s.split(",")).toInt();
}

double settings::getDoubleVal(QString s) {
	return getValInt(model, modelIndex, s.split(",")).toDouble();
}

bool settings::getBoolVal(QString s) {
	return getValInt(model, modelIndex, s.split(","))=="on";
}

QString settings::getValInt(QAbstractItemModel *model, QModelIndex modelIndex, QStringList path) {
	int i;

	for(i=0;i<model->rowCount(modelIndex);i++) {
		if (path[0] == model->index(i,0,modelIndex).data().toString()) { 
			if (path.size() == 1)
				return (model->index(i,1,modelIndex).data().toString().split(",")[1]);
			else 
				return getValInt(model, model->index(i,0, modelIndex), path.mid(1));
		}
	}
	return("0");
}

void settings::setTree(QString val) {
	QStringList list = val.split("\n");
	for (int i=0;i<list.size();i++) {
		setVal(list[i]);
	}
}

bool settings::setVal(QString val) {
	if (val.split("\t").size() > 1)
		return setValInt(model, modelIndex, val.split("\t")[0].split(","), val.split("\t")[1]);
	return false;
}

bool settings::setVal(QStringList path, QString val) {
	return setValInt(model, modelIndex, path, val);
}

bool settings::setValInt(QAbstractItemModel *model, QModelIndex modelIndex, QStringList path, QString val) {
	int i;
	for(i=0;i<model->rowCount(modelIndex);i++) {
		if (path[0] == model->index(i,0,modelIndex).data().toString()) { 
			if (path.size() == 1) {
				QStringList list = (model->index(i,1,modelIndex).data().toString().split(","));
				list[1] = val;
				model->setData(model->index(i,1,modelIndex), QVariant(list.join(",")), Qt::EditRole);
				return true;
		}
			else 
				return setValInt(model, model->index(i,0, modelIndex), path.mid(1), val);
		}
	}
	return false;
}
