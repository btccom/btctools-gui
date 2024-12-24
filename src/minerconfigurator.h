#ifndef MINERCONFIGURATOR_H
#define MINERCONFIGURATOR_H

#include <QThread>
#include <QList>
#include <QListIterator>
#include <btctools/miner/MinerConfigurator.h>

#include "minertablemodel.h"
#include "utils.h"

enum class PoolWorkerPostfix
{
    Ip,
    NoChange,
    None
};

struct MinerPools
{
    bool pool1Enabled_;
    PoolWorkerPostfix pool1WorkerPostfix_;
    btctools::miner::Pool pool1_;

    bool pool2Enabled_;
    PoolWorkerPostfix pool2WorkerPostfix_;
    btctools::miner::Pool pool2_;

    bool pool3Enabled_;
    PoolWorkerPostfix pool3WorkerPostfix_;
    btctools::miner::Pool pool3_;

    bool powerControlEnabled_;
    bool asicBoost_;
    bool lowPowerMode_;
    bool economicMode_;

    bool overclockEnabled_;
    bool reOverclocking_;
    std::string overclockMinerModel_;
    QString overclockWorkingMode_;
    QString overclockLevelName_;
};

class MinerConfigurator : public QThread
{
    Q_OBJECT

public:
    MinerConfigurator(MinerPasswordList &minerPasswordList);
    void setMiners(const QList<const MinerItem *> miners);
    void setPools(MinerPools pools);
    void stop();
    int sessionTimeout();
    void setSessionTimeout(int sessionTimeout);
    int stepSize();
    void setStepSize(int stepSize);
    void setAutoRetryTimes(int times);
    int workerNameIpParts();
    void setWorkerNameIpParts(int parts);

protected:
    void run();
    void minerChangePool(btctools::miner::Miner &miner, btctools::miner::Pool &minerPool, btctools::miner::Pool &newPool, PoolWorkerPostfix workerPostfix);
    void workerAddPostfixNoChange(std::string &worker, const std::string &oriWorker);
    void workerAddPostfixIp(std::string &worker, const std::string &ip);
    bool isPoolEmpty(btctools::miner::Pool pool);

signals:
    void minerReporter(btctools::miner::Miner);
    void configBegin();
    void configEnd(bool allSkip);
    void configProgress(int percent);
    void configException(std::exception);

private:
    volatile bool stop_;
    QList<const MinerItem *> miners_;
    MinerPools pools_;
    int sessionTimeout_;
    int stepSize_;
    int autoRetryTimes_;
    int workerNameIpParts_;
    MinerPasswordList &minerPasswordList_;

    btctools::miner::MinerConfigurator * volatile configuratorTempPointer_;
};

#endif // MINERCONFIGURATOR_H
