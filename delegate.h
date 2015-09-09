#ifndef DELEGATE_H
#define DELEGATE_H

#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QSpinBox>
#include <QComboBox>

class ItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	public:
		ItemDelegate(QObject *parent = 0);

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
				const QModelIndex &index) const;

		void setEditorData(QWidget *editor, const QModelIndex &index) const;
		void setModelData(QWidget *editor, QAbstractItemModel *model,
				const QModelIndex &index) const;

		void updateEditorGeometry(QWidget *editor,
				const QStyleOptionViewItem &option, const QModelIndex &index) const;

		QString displayText ( const QVariant & value, const QLocale & locale ) const ;
};

#endif
