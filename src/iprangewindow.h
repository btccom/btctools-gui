#ifndef IPRANGWINDOW_H
#define IPRANGWINDOW_H

#include <QDialog>
#include <QStringList>
#include "iprangeedititem.h"

namespace Ui {
class IpRangeWindow;
}

class IpRangeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit IpRangeWindow(QWidget *parent = 0);
    ~IpRangeWindow();

    QString comment();
    void setComment(const QString &comment);

    int ipRangeNum();
    QStringList enabledIpRanges();
    QStringList beautifiedEnabledIpRange();
    void addIpRange(const QString &ipRange = "", bool enabled = true);
    void clearIpRange();

    QString toString();
    void setFromString(QString str);

public slots:
    void accept();

protected:
    // add & remove ip range
    void addIpRange(IpRangeEditItem *position = nullptr, const QString &ipRange = "", bool enabled = true);
    void removeIpRange(IpRangeEditItem *ipRange);

private slots:
    void on_addIpRangeButton_clicked();

    void on_removeIpRangeButton_clicked();

    void on_ipRangeEnableAll_clicked();

private:
    Ui::IpRangeWindow *ui_;
};

#endif // IPRANGWINDOW_H
