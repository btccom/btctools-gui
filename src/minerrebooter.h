#ifndef MINERREBOOTER_H
#define MINERREBOOTER_H

#include <QThread>
#include <QVector>
#include <QVectorIterator>
#include <btctools/miner/MinerConfigurator.h>

#include "minertablemodel.h"
#include "utils.h"

class MinerRebooter : public QThread
{
    Q_OBJECT
public:
    MinerRebooter(MinerPasswordList &minerPasswordList);
    void setMiners(const QList<const MinerItem *> miners);
    void stop();
    int sessionTimeout();
    void setSessionTimeout(int sessionTimeout);
    int stepSize();
    void setStepSize(int stepSize);
    void setAutoRetryTimes(int times);

signals:
    void minerReporter(btctools::miner::Miner);
    void rebootBegin();
    void rebootEnd(bool allSkip);
    void rebootProgress(int percent);
    void rebootException(std::exception);

protected:
    void run();

private:
    volatile bool stop_;
    QList<const MinerItem *> miners_;
    int sessionTimeout_;
    int stepSize_;
    int autoRetryTimes_;
    MinerPasswordList &minerPasswordList_;

    btctools::miner::MinerConfigurator * volatile configuratorTempPointer_;
};

#endif // MINERREBOOTER_H
