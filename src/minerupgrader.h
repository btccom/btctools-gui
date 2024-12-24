#ifndef MINERUPGRADER_H
#define MINERUPGRADER_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QVectorIterator>
#include <btctools/miner/MinerConfigurator.h>

#include "minertablemodel.h"
#include "utils.h"

class MinerUpgrader : public QThread
{
    Q_OBJECT
public:
    MinerUpgrader(MinerPasswordList &minerPasswordList);
    void setMiners(const QList<const MinerItem *> miners);
    void setMinerModel(const QString &model);
    void setFirmware(const QString &firmware);
    void setKeepSettings(bool keepSettings);
    void stop();
    int sessionTimeout();
    void setSessionTimeout(int sessionTimeout);
    int stepSize();
    void setStepSize(int stepSize);
    void setAutoRetryTimes(int times);
    int sendFirmwareStepSize();
    void setSendFirmwareStepSize(int stepSize);

signals:
    void minerReporter(btctools::miner::Miner);
    void upgradeBegin();
    void upgradeEnd(bool allSkip);
    void upgradeProgress(int percent);
    void upgradeException(std::exception);

protected:
    void run();

private:
    volatile bool stop_;
    QList<const MinerItem *> miners_;

    QString minerModel_;
    QString firmware_;
    bool keepSettings_;

    int sessionTimeout_;
    int stepSize_;
    int autoRetryTimes_;
    int sendFirmwareStepSize_;
    MinerPasswordList &minerPasswordList_;

    btctools::miner::MinerConfigurator * volatile configuratorTempPointer_;
};

#endif // MINERUPGRADER_H
