#include <btctools/utils/OOLuaHelper.h>
#include "config.h"
#include "minerrebooter.h"
#include "utils.h"

MinerRebooter::MinerRebooter(MinerPasswordList &minerPasswordList) :
    sessionTimeout_(CONFIG_SESSION_TIMEOUT),
    stepSize_(CONFIG_STEP_SIZE),
    minerPasswordList_(minerPasswordList)
{

}

void MinerRebooter::run()
{
    try
    {
        stop_ = false;
        emit rebootBegin();

        // set miner passwords
        btctools::utils::OOLuaHelper::setOpt("login.minerPasswords", Utils::minerPasswordListToString(minerPasswordList_).toUtf8().data());
        // set auto retry times
        btctools::utils::OOLuaHelper::setOpt("network.autoRetryTimes", std::to_string(autoRetryTimes_));

        bool allSkipped = true;
        int allNumber = miners_.size();
        int finishedNumber = 0;

        btctools::miner::MinerSource minerSource([this, &allSkipped, &allNumber, &finishedNumber](btctools::miner::MinerYield &minerYield)
        {
            foreach (const MinerItem *item, miners_)
            {
                if (stop_)
                {
                    break;
                }

                btctools::miner::Miner miner = item->miner_;

                // successed or skipped at the last modify, skipped.
                if (miner.stat_ == std::string("rebooted") || miner.stat_ == std::string("skip"))
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

                    miner.stat_ = "pre-rebooting...";
                    emit minerReporter(miner);

                    minerYield(miner);
                }

                if (stop_)
                {
                    break;
                }

                finishedNumber++;
                emit rebootProgress(finishedNumber * 100 / allNumber);
            }
        });


        btctools::miner::MinerConfigurator config(minerSource, stepSize_, "RebooterHelper");
        configuratorTempPointer_ = &config;

        auto source = config.run(sessionTimeout_);

        for (auto miner : source)
        {
            emit minerReporter(miner);
        }

        configuratorTempPointer_ = nullptr;

        emit rebootEnd(allSkipped);
        stop_ = true;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        emit rebootException(e);
    }
    catch (...)
    {
        std::cerr << "Unknown error!" << std::endl;
        emit rebootException(std::runtime_error("Unknown error"));
    }
}

void MinerRebooter::setMiners(const QList<const MinerItem *> miners)
{
    miners_ = std::move(miners);
}

void MinerRebooter::stop()
{
    stop_ = true;

    if (configuratorTempPointer_ != nullptr)
    {
        configuratorTempPointer_->stop();
    }
}

int MinerRebooter::sessionTimeout()
{
    return sessionTimeout_;
}

void MinerRebooter::setSessionTimeout(int sessionTimeout)
{
    sessionTimeout_ = sessionTimeout;
}

int MinerRebooter::stepSize()
{
    return stepSize_;
}

void MinerRebooter::setStepSize(int stepSize)
{
    stepSize_ = stepSize;
}

void MinerRebooter::setAutoRetryTimes(int times)
{
    autoRetryTimes_ = times;
}
