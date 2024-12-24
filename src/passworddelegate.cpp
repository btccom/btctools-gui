#include "passworddelegate.h"
#include <QDebug>

PasswordDelegate::PasswordDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

// TableView need to create an Editor
// Create Editor when we construct MyDelegate
// and return the Editor
QWidget* PasswordDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    QLineEdit *editor = new QLineEdit(parent);
    editor->setEchoMode(QLineEdit::Password);
    return editor;
}

// Then, we set the Editor
// Gets the data from Model and feeds the data to delegate Editor
void PasswordDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // Get the value via index of the Model
    QString value = index.model()->data(index, Qt::UserRole).toString();

    // Put the value into the SpinBox
    static_cast<QLineEdit*>(editor)->setText(value);
}

// When we modify data, this model reflect the change
// Data from the delegate to the model
void PasswordDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    model->setData(index, static_cast<QLineEdit*>(editor)->text(), Qt::UserRole);
    model->setData(index, displayText(static_cast<QLineEdit*>(editor)->text()), Qt::DisplayRole);
}

// Give the SpinBox the info on size and location
void PasswordDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
