#ifndef PASSWORDDELEGATE_H
#define PASSWORDDELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QLineEdit>

class PasswordDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit PasswordDelegate(QObject *parent = 0);

    // Create Editor when we construct MyDelegate
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    // Then, we set the Editor
    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    // When we modify data, this model reflect the change
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    // Give the SpinBox the info on size and location
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    inline static QString displayText(const QString &password)
    {
        return QString("*").repeated(password.size());
    }
};

#endif // PASSWORDDELEGATE_H
