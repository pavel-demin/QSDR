#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtGui>
#include "treemodel.h"
#include "delegate.h"



class settings : public QObject
{
Q_OBJECT
public:
    explicit settings(QTreeView *w, QObject *parent = 0);

signals:
	void changed();

public slots:
	void changed (QModelIndex, QModelIndex);

public:
	QString printTree();
	QString printTreePath();
	QString getVal(QStringList path);
	QString getVal(QString s);
	int getIntVal(QString s);
	double getDoubleVal(QString s);
	bool getBoolVal(QString s);
	void setTree(QString val);
	bool setVal(QStringList path, QString val);
	bool setVal(QString val);
private:
	QString printTreeInt(QAbstractItemModel *model, QModelIndex modelIndex, QString header);
	QString printTreePathInt(QAbstractItemModel *model, QModelIndex modelIndex, QString header);
	QString getValInt(QAbstractItemModel *model, QModelIndex modelIndex, QStringList path);
	bool setValInt(QAbstractItemModel *model, QModelIndex modelIndex, QStringList path, QString val);

	QTreeView *view;
	QModelIndex modelIndex;
	QAbstractItemModel *model;
};

#endif // SETTINGS_H
