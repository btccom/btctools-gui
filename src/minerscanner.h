#ifndef MINERSCANNER_H
#define MINERSCANNER_H

#include <QThread>
#include <QProcessEnvironment>
#include <btctools/miner/MinerScanner.h>
#include "utils.h"

class MinerScanner : public QThread
{
    Q_OBJECT

public:
    MinerScanner(MinerPasswordList &minerPasswordList);
    void setIpRange(QString ipRange);
    void setIpRange(btctools::utils::IpGeneratorGroup ipg);
    void stop();
    int sessionTimeout();
    void setSessionTimeout(int sessionTimeout);
    int stepSize();
    void setStepSize(int stepSize);
    void setAutoRetryTimes(int times);

protected:
    void run();

signals:
    void minerReporter(btctools::miner::Miner);
    void scanBegin();
    void scanEnd();
    void scanProgress(int percent);
    void scanException(std::exception);

private:
    btctools::utils::IpGeneratorGroup ipg_;
    volatile bool stop_;
    int sessionTimeout_;
    int stepSize_;
    int autoRetryTimes_;
    MinerPasswordList &minerPasswordList_;

    btctools::miner::MinerScanner * volatile scannerTempPointer_;
};

#endif // MINERSCANNER_H
