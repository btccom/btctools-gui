#include <btctools/utils/OOLuaHelper.h>
#include "config.h"
#include "minerupgrader.h"
#include "utils.h"

MinerUpgrader::MinerUpgrader(MinerPasswordList &minerPasswordList) :
    sessionTimeout_(UPGRADE_SESSION_TIMEOUT),
    stepSize_(CONFIG_STEP_SIZE),
    sendFirmwareStepSize_(UPGRADE_SEND_FIRMWARE_STEP_SIZE),
    minerPasswordList_(minerPasswordList)
{

}

void MinerUpgrader::run()
{
    try
    {
        stop_ = false;
        emit upgradeBegin();

        // set miner passwords
        btctools::utils::OOLuaHelper::setOpt("login.minerPasswords", Utils::minerPasswordListToString(minerPasswordList_).toUtf8().data());

        // set auto retry times
        btctools::utils::OOLuaHelper::setOpt("network.autoRetryTimes", std::to_string(autoRetryTimes_));

        // set miner model
        btctools::utils::OOLuaHelper::setOpt("upgrader.minerModel", minerModel_.toUtf8().data());

        // set firmware
        btctools::utils::OOLuaHelper::setOpt("upgrader.firmwareName", Utils::getAnsiString(firmware_));

        // set keep settings
        btctools::utils::OOLuaHelper::setOpt("upgrader.keepSettings", keepSettings_ ? "1" : "0");

        // step size when sending firmwares
        btctools::utils::OOLuaHelper::setOpt("upgrader.sendFirmwareStepSize", std::to_string(sendFirmwareStepSize_));


        bool allSkipped = true;
        int allNumber = miners_.size();
        int finishedNumber = 0;
        std::string model = minerModel_.toUtf8().data();

        btctools::miner::MinerSource minerSource([this, &allSkipped, &allNumber, &finishedNumber, &model](btctools::miner::MinerYield &minerYield)
        {
            foreach (const MinerItem *item, miners_)
            {
                if (stop_)
                {
                    break;
                }

                btctools::miner::Miner miner = item->miner_;

                // successed or skipped at the last modify, skipped.
                if (miner.stat_ == std::string("upgraded"))
                {
                    // no action
                }
                else if (miner.fullTypeStr_ != model)
                {
                    miner.stat_ = "skip";
                    emit minerReporter(miner);
                }
                else
                {
                    allSkipped = false;

                    miner.stat_ = "pre-upgrading...";
                    emit minerReporter(miner);

                    minerYield(miner);
                }

                if (stop_)
                {
                    break;
                }

                finishedNumber++;
                emit upgradeProgress(finishedNumber * 100 / allNumber);
            }
        });


        btctools::miner::MinerConfigurator config(minerSource, stepSize_, "UpgraderHelper");
        configuratorTempPointer_ = &config;

        auto source = config.run(sessionTimeout_);

        for (auto miner : source)
        {
            emit minerReporter(miner);
        }

        configuratorTempPointer_ = nullptr;

        emit upgradeEnd(allSkipped);
        stop_ = true;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        emit upgradeException(e);
    }
    catch (...)
    {
        std::cerr << "Unknown error!" << std::endl;
        emit upgradeException(std::runtime_error("Unknown error"));
    }
}

void MinerUpgrader::setMiners(const QList<const MinerItem *> miners)
{
    miners_ = std::move(miners);
}

void MinerUpgrader::setMinerModel(const QString &model)
{
    minerModel_ = model;
}

void MinerUpgrader::setFirmware(const QString &firmware)
{
    firmware_ = firmware;
}

void MinerUpgrader::setKeepSettings(bool keepSettings)
{
    keepSettings_ = keepSettings;
}

void MinerUpgrader::stop()
{
    stop_ = true;

    if (configuratorTempPointer_ != nullptr)
    {
        configuratorTempPointer_->stop();
    }
}

int MinerUpgrader::sessionTimeout()
{
    return sessionTimeout_;
}

void MinerUpgrader::setSessionTimeout(int sessionTimeout)
{
    sessionTimeout_ = sessionTimeout;
}

int MinerUpgrader::stepSize()
{
    return stepSize_;
}

void MinerUpgrader::setStepSize(int stepSize)
{
    stepSize_ = stepSize;
}

void MinerUpgrader::setAutoRetryTimes(int times)
{
    autoRetryTimes_ = times;
}

int MinerUpgrader::sendFirmwareStepSize()
{
    return sendFirmwareStepSize_;
}

void MinerUpgrader::setSendFirmwareStepSize(int stepSize)
{
    sendFirmwareStepSize_ = stepSize;
}
