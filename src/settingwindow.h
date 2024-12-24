#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>
#include <QStandardItemModel>
#include <QAbstractButton>
#include "utils.h"

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = 0);
    ~SettingWindow();

    // getter & setter of scanning & highlight options
    int monitorInterval() const;
    void setMonitorInterval(int interval);

    int scanningTimeout() const;
    void setScanningTimeout(int timeout);

    int concurrentScanning() const;
    void setConcurrentScanning(int minerNum);

    int configuringTimeout() const;
    void setConfiguringTimeout(int timeout);

    int concurrentConfiguring() const;
    void setConcurrentConfiguring(int minerNum);

    int upgradingTimeout() const;
    void setUpgradingTimeout(int timeout);

    int concurrentUpgrading() const;
    void setConcurrentUpgrading(int minerNum);

    bool isHighlightTemperature() const;
    void setIsHighlightTemperature(bool isHighlightTemperature);

    int highlightTemperatureMoreThan() const;
    void setHighlightTemperatureMoreThan(int highlightTemperatureMoreThan);

    int highlightTemperatureLessThan() const;
    void setHighlightTemperatureLessThan(int highlightTemperatureLessThan);

    bool isHighlightLowHashrate() const;
    void setIsHighlightLowHashrate(bool isHighlightLowHashrate);

    MinerHashrateList highlightLowHashrates() const;
    void setHighlightLowHashrates(const MinerHashrateList &highlightLowHashrates);

    MinerPasswordList minerPasswords() const;
    void setMinerPasswords(const MinerPasswordList &minerPasswords);

    bool isHighlightWrongWorkerName() const;
    void setIsHighlightWrongWorkerName(bool isHighlightWrongWorkerName);

    bool isOpenMinerCPWithPassword() const;
    void setIsOpenMinerCPWithPassword(bool isOpenMinerCPWithPassword);

    int workerNameIpParts() const;
    void setWorkerNameIpParts(int parts);

    int autoRetryTimes() const;
    void setAutoRetryTimes(int times);

protected:
    void addHashrateRow(const QString &minerType = "", const QString &hashrate = "");
    void addMinerPasswordRow(const QString &minerType = "", const QString &userName = "", const QString &password = "");

private slots:
    void on_addHashRateButton_clicked();
    void on_removeHashRateButton_clicked();
    void on_buttonBox_clicked(QAbstractButton *button);

    void on_addMinerPassword_clicked();
    void on_removeMinerPassword_clicked();

private:
    Ui::SettingWindow *ui_;
    QStandardItemModel hashrateTableModel_;
    QStandardItemModel minerPasswordModel_;
};

#endif // SETTINGWINDOW_H
