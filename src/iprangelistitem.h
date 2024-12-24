#ifndef IPRANGELISTITEM_H
#define IPRANGELISTITEM_H

#include <QObject>
#include <QWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QStringList>
#include "iprangewindow.h"

class IpRangeListItem : public QWidget, public QListWidgetItem
{
    Q_OBJECT

public:
    IpRangeListItem(QListWidget *parent, int row = -1, QWidget *editWindowParent = nullptr);

    bool isEnabled();
    void setEnabled(bool enabled);

    bool editWithNewWindow();

    QStringList enabledIpRanges();

    QString toString();
    void setFromString(QString ipRangeStr);

protected:
    void syncDataAndDisplay();

private:
    QListWidget *parent_;
    QWidget *child_;
    QHBoxLayout *layout_;

    QCheckBox *enabled_;
    QLabel *descText_;

    IpRangeWindow ipRangeWindow_;
    QStringList enabledIpRanges_;
};

#endif // IPRANGELISTITEM_H
