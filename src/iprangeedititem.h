#ifndef IPRANGEEDITITEM_H
#define IPRANGEEDITITEM_H

#include <QObject>
#include <QWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include "iprangeedit.h"

class IpRangeEditItem : public QWidget, public QListWidgetItem
{
    Q_OBJECT

public:
    IpRangeEditItem(QListWidget *parent, int row = -1);

    bool isEnabled();
    void setEnabled(bool enabled);

    IpRangeEdit *edit();

private:
    QListWidget *parent_;
    QWidget *child_;
    QHBoxLayout *layout_;

    QCheckBox *enable_;
    QLabel *descText_;
    IpRangeEdit *ipRangeEdit_;
};

#endif // IPRANGEEDITITEM_H
