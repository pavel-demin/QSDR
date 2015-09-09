#include <QtGui>
#include "delegate.h"

ItemDelegate::ItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex & index ) const
{
	QString val = index.model()->data(index, Qt::EditRole).toString();
	QStringList list = val.split(",", QString::SkipEmptyParts);
	if (list.size()==0)
			return 0;// new QLineEdit(parent); 

    if (list[0] == QString("bool")) {
		QCheckBox *editor = new QCheckBox(parent); 
		editor->setChecked(list[1]=="on");
		return editor;
	}

    if (list[0] == QString("double")) {
		QDoubleSpinBox *editor = new QDoubleSpinBox(parent); 
		editor->setMinimum(list[2].toDouble());
		editor->setMaximum(list[3].toDouble());
		editor->setValue(list[1].toDouble());
		return editor;
	}
    if (list[0] == QString("int")) {
		QSpinBox *editor = new QSpinBox(parent); 
		editor->setMinimum(list[2].toInt());
		editor->setMaximum(list[3].toInt());
		editor->setValue(list[1].toInt());
		return editor;
	}
    if (list[0] == QString("enum")) {
		QComboBox *editor = new QComboBox(parent); 
		editor->addItems(list.mid(2));
		editor->setCurrentIndex(list[1].toInt());
		return editor;
	}
    if (list[0] == QString("string")) {
		QLineEdit *editor = new QLineEdit(parent); 
		editor->setText(list[1]);
		return editor;
	}
    if (list[0] == QString("color")) {
		QColorDialog *editor =  new QColorDialog(parent);
		editor->setCurrentColor(QColor(list[1]));
		return editor;
	}

    return 0;
}

void ItemDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
}

void ItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
	QString val = index.model()->data(index, Qt::EditRole).toString();
	QStringList list = val.split(",", QString::SkipEmptyParts);

	if (list[0]=="bool") {
		QCheckBox *edit = static_cast<QCheckBox*>(editor); 
		if (edit->isChecked())
			list[1] = "on";
		else
			list[1] = "off";
	}
	if (list[0]=="double") {
		QDoubleSpinBox *edit = static_cast<QDoubleSpinBox*>(editor); 
		list[1] = QString("%1").arg(edit->value());
	}
	if (list[0]=="int") {
		QSpinBox *edit = static_cast<QSpinBox*>(editor); 
		list[1] = QString("%1").arg(edit->value());
	}
	if (list[0]=="enum") {
		QComboBox *edit = static_cast<QComboBox*>(editor); 
		list[1] = QString("%1").arg(edit->currentIndex());
	}
	if (list[0]=="string") {
		QLineEdit *edit = static_cast<QLineEdit*>(editor); 
		list[1] = QString("%1").arg(edit->text());
	}
	if (list[0]=="color") {
		QColorDialog *edit = static_cast<QColorDialog*>(editor); 
		list[1] = edit->currentColor().name(); 
	}
    model->setData(index, list.join(","), Qt::EditRole);
}

void ItemDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

QString ItemDelegate::displayText ( const QVariant & value, const QLocale & locale ) const {
	QStringList list = value.toString().split(",", QString::SkipEmptyParts);
	if (list.size()>=2) {
		if(list[0]=="enum") {
			int idx = list[1].toInt()+2;
			if (idx>=list.size())
				idx=2;
			return list[idx];
		}
		return list[1];
	}
	return value.toString(); 
}

