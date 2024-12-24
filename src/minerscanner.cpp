#include <btctools/utils/OOLuaHelper.h>
#include "config.h"
#include "minerscanner.h"
#include "utils.h"

MinerScanner::MinerScanner(MinerPasswordList &minerPasswordList) :
    sessionTimeout_(SCAN_SESSION_TIMEOUT),
    stepSize_(SCAN_STEP_SIZE),
    minerPasswordList_(minerPasswordList),
    scannerTempPointer_(nullptr)
{
    qRegisterMetaType<btctools::miner::Miner>("btctools::miner::Miner");
    qRegisterMetaType<std::exception>("std::exception");
}

void MinerScanner::setIpRange(QString ipRange)
{
    ipg_.addIpRange(ipRange.toStdString());
}

void MinerScanner::setIpRange(btctools::utils::IpGeneratorGroup ipg)
{
    ipg_ = ipg;
}

void MinerScanner::stop()
{
    stop_ = true;

    if (scannerTempPointer_ != nullptr){
        scannerTempPointer_->stop();
    }
}


int MinerScanner::sessionTimeout()
{
    return sessionTimeout_;
}

void MinerScanner::setSessionTimeout(int sessionTimeout)
{
    sessionTimeout_ = sessionTimeout;
}

int MinerScanner::stepSize()
{
    return stepSize_;
}

void MinerScanner::setStepSize(int stepSize)
{
    stepSize_ = stepSize;
}

void MinerScanner::setAutoRetryTimes(int times)
{
    autoRetryTimes_ = times;
}

void MinerScanner::run()
{
    try
    {
        stop_ = false;
        emit scanBegin();

        // set miner passwords
        btctools::utils::OOLuaHelper::setOpt("login.minerPasswords", Utils::minerPasswordListToString(minerPasswordList_).toUtf8().data());
        // set auto retry times
        btctools::utils::OOLuaHelper::setOpt("network.autoRetryTimes", std::to_string(autoRetryTimes_));

        int allNumber = ipg_.getIpNumber();
        int finishedNumber = 0;

        auto ipSourceWithCount = btctools::utils::IpStrSource([this, &finishedNumber, &allNumber](btctools::utils::IpStrYield &yield)
        {
            auto ipSource = ipg_.genIpRange();

            for (auto ip : ipSource)
            {
                btctools::miner::Miner miner;
                miner.ip_ = ip;
                miner.stat_ = "pre-scanning...";

                if (stop_)
                {
                    break;
                }

                emit minerReporter(miner);

                yield(ip);

                finishedNumber++;

                if (stop_)
                {
                    break;
                }

                emit scanProgress(finishedNumber * 100 / allNumber);
            }
        });

        btctools::miner::MinerScanner scanner(ipSourceWithCount, stepSize_);
        scannerTempPointer_ = &scanner;

        auto source = scanner.run(sessionTimeout_);

        for (auto miner : source)
        {
            emit minerReporter(miner);
        }

        scannerTempPointer_ = nullptr;
        emit scanEnd();
        stop_ = true;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        emit scanException(e);
    }
    catch (...)
    {
        std::cerr << "Unknown error!" << std::endl;
        emit scanException(std::runtime_error("Unknown error"));
    }
}
