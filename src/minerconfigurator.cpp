#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <btctools/utils/OOLuaHelper.h>
#include "config.h"
#include "minerconfigurator.h"
#include "utils.h"

MinerConfigurator::MinerConfigurator(MinerPasswordList &minerPasswordList) :
    sessionTimeout_(CONFIG_SESSION_TIMEOUT),
    stepSize_(CONFIG_STEP_SIZE),
    workerNameIpParts_(WORKER_NAME_IP_PARTS),
    minerPasswordList_(minerPasswordList)
{
    qRegisterMetaType<btctools::miner::Miner>("btctools::miner::Miner");
    qRegisterMetaType<std::exception>("std::exception");
}

void MinerConfigurator::setMiners(const QList<const MinerItem *> miners)
{
    miners_ = std::move(miners);
}

void MinerConfigurator::setPools(MinerPools pools)
{
    pools_ = std::move(pools);
}

void MinerConfigurator::stop()
{
    stop_ = true;

    if (configuratorTempPointer_ != nullptr)
    {
        configuratorTempPointer_->stop();
    }
}

int MinerConfigurator::sessionTimeout()
{
    return sessionTimeout_;
}

void MinerConfigurator::setSessionTimeout(int sessionTimeout)
{
    sessionTimeout_ = sessionTimeout;
}

int MinerConfigurator::stepSize()
{
    return stepSize_;
}

void MinerConfigurator::setStepSize(int stepSize)
{
    stepSize_ = stepSize;
}

void MinerConfigurator::setAutoRetryTimes(int times)
{
    autoRetryTimes_ = times;
}

int MinerConfigurator::workerNameIpParts()
{
    return workerNameIpParts_;
}

void MinerConfigurator::setWorkerNameIpParts(int parts)
{
    workerNameIpParts_ = parts;
}

void MinerConfigurator::minerChangePool(btctools::miner::Miner &miner, btctools::miner::Pool &minerPool, btctools::miner::Pool &newPool, PoolWorkerPostfix workerPostfix)
{
    std::string oriWorker = minerPool.worker_;
    minerPool = newPool;

    switch (workerPostfix)
    {
    case PoolWorkerPostfix::Ip:
        if (!isPoolEmpty(minerPool))
        {
            workerAddPostfixIp(minerPool.worker_, miner.ip_);
        }
        break;
    case PoolWorkerPostfix::NoChange:
        if (!isPoolEmpty(minerPool))
        {
            workerAddPostfixNoChange(minerPool.worker_, oriWorker);
        }
        break;
    case PoolWorkerPostfix::None:
        // no action
        break;
    }
}

void MinerConfigurator::workerAddPostfixNoChange(std::string &worker, const std::string &oriWorker)
{
    QString oriWorkerQString = oriWorker.c_str();
    oriWorkerQString = oriWorkerQString.trimmed();

    // oriWorker is empty, don't add the postfix
    if (oriWorkerQString.isEmpty())
    {
        return;
    }

    int pos = oriWorkerQString.indexOf(".");

    // there is no postfix of oriWorker
    if (pos < 0)
    {
        return;
    }

    if (!worker.empty() && worker.find('.') == std::string::npos)
    {
        worker += '.';
    }

    worker += oriWorkerQString.right(oriWorkerQString.size() - pos - 1).toStdString();
}

void MinerConfigurator::workerAddPostfixIp(std::string &worker, const std::string &ip)
{
    struct in_addr_btctools ipAddr;
    ipAddr.S_un.S_addr = inet_addr(ip.c_str());

    if (!worker.empty() && worker.find('.') == std::string::npos)
    {
        worker += '.';
    }

    char buffer[4][4];
    sprintf(buffer[0], "%hhu", ipAddr.S_un.S_un_b.s_b1);
    sprintf(buffer[1], "%hhu", ipAddr.S_un.S_un_b.s_b2);
    sprintf(buffer[2], "%hhu", ipAddr.S_un.S_un_b.s_b3);
    sprintf(buffer[3], "%hhu", ipAddr.S_un.S_un_b.s_b4);

    QStringList ipParts;

    int beginPart = 4 - workerNameIpParts_;

    for (int i=beginPart; i<4; i++) {
        ipParts.append(QString(buffer[i]));
    }

    worker += ipParts.join('x').toStdString();
}

bool MinerConfigurator::isPoolEmpty(btctools::miner::Pool pool)
{
    return pool.url_.empty() && pool.worker_.empty();
}

void MinerConfigurator::run()
{
    try
    {
        stop_ = false;
        emit configBegin();

        // set miner passwords
        btctools::utils::OOLuaHelper::setOpt("login.minerPasswords", Utils::minerPasswordListToString(minerPasswordList_).toUtf8().data());
        // set auto retry times
        btctools::utils::OOLuaHelper::setOpt("network.autoRetryTimes", std::to_string(autoRetryTimes_));

        bool allSkipped = true;
        int allNumber = miners_.size();
        int finishedNumber = 0;

        btctools::miner::MinerSource minerSource([this, &allSkipped, &allNumber, &finishedNumber](btctools::miner::MinerYield &minerYield)
        {
            for (auto i=miners_.begin(); i<miners_.end(); i++)
            {
                if (stop_)
                {
                    break;
                }

                btctools::miner::Miner miner = (*i)->miner_;

                // successed or skipped at the last modify, skipped.
                if (miner.stat_ == std::string("ok") || miner.stat_ == std::string("skip"))
                {
                    // no action
                }
                else if (miner.typeStr_ == std::string("unknown") || miner.typeStr_ == std::string(""))
                {
                    miner.stat_ = "skip";
                    emit minerReporter(miner);
                }
                else
                {
                    allSkipped = false;

                    miner.stat_ = "pre-configurating...";
                    emit minerReporter(miner);


                    if (pools_.pool1Enabled_)
                    {
                        minerChangePool(miner, miner.pool1_, pools_.pool1_, pools_.pool1WorkerPostfix_);
                    }

                    if (pools_.pool2Enabled_)
                    {
                        minerChangePool(miner, miner.pool2_, pools_.pool2_, pools_.pool2WorkerPostfix_);
                    }

                    if (pools_.pool3Enabled_)
                    {
                        minerChangePool(miner, miner.pool3_, pools_.pool3_, pools_.pool3WorkerPostfix_);
                    }

                    if (pools_.powerControlEnabled_)
                    {
                        miner.setOpt("config.antminer.asicBoost", pools_.asicBoost_ ? "true" : "false");
                        miner.setOpt("config.antminer.lowPowerMode", pools_.lowPowerMode_ ? "true" : "false");
                        miner.setOpt("config.antminer.economicMode", pools_.economicMode_ ? "true" : "false");
                    }

                    if (pools_.overclockEnabled_ && pools_.overclockMinerModel_ == miner.fullTypeStr_) {
                        miner.setOpt("config.antminer.overclockWorkingMode", pools_.overclockWorkingMode_.toUtf8().data());
                        miner.setOpt("config.antminer.overclockLevelName", pools_.overclockLevelName_.toUtf8().data());
                    }

                    miner.setOpt("config.antminer.forceTuning", pools_.reOverclocking_ ? "true" : "false");

                    minerYield(miner);
                }

                if (stop_)
                {
                    break;
                }

                finishedNumber++;
                emit configProgress(finishedNumber * 100 / allNumber);
            }
        });


        btctools::miner::MinerConfigurator config(minerSource, stepSize_);
        configuratorTempPointer_ = &config;

        auto source = config.run(sessionTimeout_);

        for (auto miner : source)
        {
            emit minerReporter(miner);
        }

        configuratorTempPointer_ = nullptr;

        emit configEnd(allSkipped);
        stop_ = true;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        emit configException(e);
    }
    catch (...)
    {
        std::cerr << "Unknown error!" << std::endl;
        emit configException(std::runtime_error("Unknown error"));
    }
}
