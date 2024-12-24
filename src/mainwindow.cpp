#include <iostream>
#include <QMessageBox>
#include <QCheckBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QTimer>
#include <QStringList>
#include <QClipboard>
#include <QFileDialog>
#include <QDebug>
#include <btctools/miner/MinerScanner.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "iprangeedit.h"
#include "iprangelistitem.h"
#include "iprangewindow.h"
#include "checkmessagebox.h"
#include "utils.h"
#include "config.h"

using namespace std;
using namespace btctools::miner;
using namespace btctools::utils;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow),
    settingWindow_(this),
    minerPasswordList_(DEFAULT_MINER_PASSWORDS),
    scanner_(minerPasswordList_),
    configurator_(minerPasswordList_),
    rebooter_(minerPasswordList_),
    upgrader_(minerPasswordList_),
    onlySuccessMiners_(false),
    monitorRunning_(false),
    openMinerCPWithPassword_(OPEN_MINER_CP_WITH_PASSWORD)
{
    ui_->setupUi(this);

    // set version name
    this->setWindowTitle(Utils::debugMode() ? APP_FULL_NAME_DEBUG : APP_FULL_NAME);

    // set data model
    minerModel_ = new MinerTableModel;
    ui_->minersTable->setModel(minerModel_);

    //******* set columns' width of minersTable *******
    // elapsed
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_ELAPSED, 150);
    // pools
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_POOL1, 160);
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_POOL2, 160);
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_POOL3, 160);
    // workers
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_WORKER1, 140);
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_WORKER2, 140);
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_WORKER3, 140);
    // network type
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_NETWORK, 80);
    // mac address
    ui_->minersTable->setColumnWidth(MinerTableModel::COL_MAC_ADDRESS, 140);

    // set header height
    ui_->minersTable->horizontalHeader()->setFixedHeight(35);
    // set default row height
    //ui_->minersTable->verticalHeader()->setDefaultSectionSize(30);

    // init table sort
    ui_->minersTable->sortByColumn(0, Qt::AscendingOrder);

    // allow swapping columns
    ui_->minersTable->horizontalHeader()->setSectionsMovable(true);

    // context menu
    initContextMenu();

    // set button status
    setAppStatus(AppStatus::NOT_SCAN);

    //----------------------- connect singals & slots -----------------------------
    // scanner
    connect(&scanner_, SIGNAL(minerReporter(btctools::miner::Miner)), this, SLOT(onReceivedMiner(btctools::miner::Miner)));

    connect(&scanner_, SIGNAL(scanBegin()), this, SLOT(onActionBegin()));
    connect(&scanner_, SIGNAL(scanProgress(int)), this, SLOT(onActionProgress(int)));
    connect(&scanner_, SIGNAL(scanEnd()), this, SLOT(onActionEnd()));

    connect(&scanner_, SIGNAL(scanBegin()), this, SLOT(onScanBegin()));
    connect(&scanner_, SIGNAL(scanEnd()), this, SLOT(onScanEnd()));

    connect(&scanner_, SIGNAL(scanException(std::exception)), this, SLOT(onReceivedException(std::exception)));

    // configurator
    connect(&configurator_, SIGNAL(minerReporter(btctools::miner::Miner)), this, SLOT(onReceivedMiner(btctools::miner::Miner)));

    connect(&configurator_, SIGNAL(configBegin()), this, SLOT(onActionBegin()));
    connect(&configurator_, SIGNAL(configProgress(int)), this, SLOT(onActionProgress(int)));
    connect(&configurator_, SIGNAL(configEnd(bool)), this, SLOT(onActionEnd()));

    connect(&configurator_, SIGNAL(configBegin()), this, SLOT(onConfigBegin()));
    connect(&configurator_, SIGNAL(configEnd(bool)), this, SLOT(onConfigEnd(bool)));

    connect(&configurator_, SIGNAL(configException(std::exception)), this, SLOT(onReceivedException(std::exception)));

    // rebooter
    connect(&rebooter_, SIGNAL(minerReporter(btctools::miner::Miner)), this, SLOT(onReceivedMiner(btctools::miner::Miner)));

    connect(&rebooter_, SIGNAL(rebootBegin()), this, SLOT(onActionBegin()));
    connect(&rebooter_, SIGNAL(rebootProgress(int)), this, SLOT(onActionProgress(int)));
    connect(&rebooter_, SIGNAL(rebootEnd(bool)), this, SLOT(onActionEnd()));

    connect(&rebooter_, SIGNAL(rebootBegin()), this, SLOT(onRebootBegin()));
    connect(&rebooter_, SIGNAL(rebootEnd(bool)), this, SLOT(onRebootEnd(bool)));

    connect(&rebooter_, SIGNAL(rebootException(std::exception)), this, SLOT(onReceivedException(std::exception)));

    // upgrader
    connect(&upgrader_, SIGNAL(minerReporter(btctools::miner::Miner)), this, SLOT(onReceivedMiner(btctools::miner::Miner)));

    connect(&upgrader_, SIGNAL(upgradeBegin()), this, SLOT(onActionBegin()));
    connect(&upgrader_, SIGNAL(upgradeProgress(int)), this, SLOT(onActionProgress(int)));
    connect(&upgrader_, SIGNAL(upgradeEnd(bool)), this, SLOT(onActionEnd()));

    connect(&upgrader_, SIGNAL(upgradeBegin()), this, SLOT(onUpgradeBegin()));
    connect(&upgrader_, SIGNAL(upgradeEnd(bool)), this, SLOT(onUpgradeEnd(bool)));

    connect(&upgrader_, SIGNAL(upgradeException(std::exception)), this, SLOT(onReceivedException(std::exception)));

    // auto updater
    connect(&autoUpdater_, SIGNAL(findUpdates(QString, QString, QString, bool)), this, SLOT(onFindUpdates(QString, QString, QString, bool)));

    // UpgradeWindow
    connect(&upgradeWindow_, SIGNAL(selectedMinerModelChanged(QString)), this, SLOT(onUpgradeWindowSelectedMinerModelChanged(QString)));
    connect(&upgradeWindow_, SIGNAL(upgradeSelectedMiners()), this, SLOT(onUpgradeSelectedMiners()));
    connect(&upgradeWindow_, SIGNAL(upgradeAllMiners()), this, SLOT(onUpgradeAllMiners()));


    //----------------------- load UI config -----------------------------
    loadUiConfig();

    //----------------------- run autoupdater -----------------------------
    autoUpdater_.start();
}

MainWindow::~MainWindow()
{
    delete ui_;
    delete minerModel_;
}

bool MainWindow::checkOnePoolAndNotice(int id, QLineEdit *poolUrl, QLineEdit *poolWorker)
{
    if (poolUrl->text().isEmpty())
    {
        auto button = QMessageBox::information(this, tr("Pool url is empty"), tr("Your pool %1's url is empty, are you sure?").arg(id),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (button == QMessageBox::No)
        {
            return false;
        }
    }

    if (poolWorker->text().isEmpty())
    {
        auto button = QMessageBox::information(this, tr("SubAccount is empty"), tr("Your pool %1's sub-account is empty, are you sure?").arg(id),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (button == QMessageBox::No)
        {
            return false;
        }
    }

    return true;
}

void MainWindow::updateSelectedMinerItems()
{
    selectedMinerItems_.clear();
    QModelIndexList selectedMinersIndex = ui_->minersTable->selectionModel()->selectedIndexes();
    QList<int> rowList;

    foreach (QModelIndex index, selectedMinersIndex)
    {
        if (!rowList.contains(index.row()))
        {
            rowList.append(index.row());
            const MinerItem *item = &(minerModel_->getMiner(index.row()));
            selectedMinerItems_.append(item);
        }
    }
}

void MainWindow::setAppStatus(AppStatus appStatus)
{
    ButtonStatus scanButton, monitorButton,
            configAllButton, configSelButton,
            rebootAllButton, rebootSelButton,
            upgradeButton;

    appStatus_ = appStatus;

    switch (appStatus)
    {
    case AppStatus::NOT_SCAN:
        scanButton = monitorButton = ButtonStatus::ENABLED;
        configAllButton =
            configSelButton =
            rebootAllButton =
            rebootSelButton =
              upgradeButton = ButtonStatus::DISABLED;
        break;
    case AppStatus::SCANNING:
        scanButton = ButtonStatus::STOPABLED;
        monitorButton =
            configAllButton =
            configSelButton =
            rebootAllButton =
            rebootSelButton =
              upgradeButton = ButtonStatus::DISABLED;
        break;
    case AppStatus::MONITORING:
        monitorButton = ButtonStatus::STOPABLED;
        scanButton =
            configAllButton =
            configSelButton =
            rebootAllButton =
            rebootSelButton =
              upgradeButton = ButtonStatus::DISABLED;
        break;
    case AppStatus::CONFIGURATING_ALL:
        configAllButton = ButtonStatus::STOPABLED;
        scanButton =
            monitorButton =
            configSelButton =
            rebootAllButton =
            rebootSelButton =
              upgradeButton = ButtonStatus::DISABLED;
        break;
    case AppStatus::CONFIGURATING_SEL:
        configSelButton = ButtonStatus::STOPABLED;
        scanButton =
            monitorButton =
            configAllButton =
            rebootAllButton =
            rebootSelButton =
              upgradeButton = ButtonStatus::DISABLED;
        break;
    case AppStatus::REBOOTING_ALL:
        rebootAllButton = ButtonStatus::STOPABLED;
        scanButton =
            monitorButton =
            configAllButton =
            configSelButton =
            rebootSelButton =
              upgradeButton = ButtonStatus::DISABLED;
        break;
    case AppStatus::REBOOTING_SEL:
        rebootSelButton = ButtonStatus::STOPABLED;
        scanButton =
            monitorButton =
            configAllButton =
            configSelButton =
            rebootAllButton =
              upgradeButton = ButtonStatus::DISABLED;
        break;
    case AppStatus::UPGRADING_ALL:
    case AppStatus::UPGRADING_SEL:
        upgradeButton = ButtonStatus::STOPABLED;
        scanButton =
            monitorButton =
            configAllButton =
            configSelButton =
            rebootAllButton =
            rebootSelButton = ButtonStatus::DISABLED;
        break;
    // all buttons disable on prepare
    case AppStatus::SCAN_PREPARE:
    case AppStatus::MONITOR_PREPARE:
    case AppStatus::CONFIG_ALL_PREPARE:
    case AppStatus::CONFIG_SEL_PREPARE:
    case AppStatus::REBOOT_ALL_PREPARE:
    case AppStatus::REBOOT_SEL_PREPARE:
    case AppStatus::UPGRADE_ALL_PREPARE:
    case AppStatus::UPGRADE_SEL_PREPARE:
        scanButton =
            monitorButton =
            configAllButton =
            configSelButton =
            rebootAllButton =
            rebootSelButton =
              upgradeButton = ButtonStatus::PREPARE;
        break;
    // all buttons enabled after the finish
    case AppStatus::SCAN_FINISH:
    case AppStatus::MONITOR_FINISH:
    case AppStatus::REBOOT_FINISH:
    case AppStatus::CONFIG_FINISH:
    case AppStatus::UPGRADE_FINISH:
        scanButton =
            monitorButton =
            configAllButton =
            configSelButton =
            rebootAllButton =
            rebootSelButton =
              upgradeButton = ButtonStatus::ENABLED;
        break;
    }


    // change button status

    if (scanButton == ButtonStatus::PREPARE)
    {
        ui_->scanMinersButton->setEnabled(false);
    }
    else if (scanButton == ButtonStatus::STOPABLED)
    {
        ui_->scanMinersButton->setEnabled(true);
        ui_->scanMinersButton->setText(tr("Stop Scan"));
    }
    else
    {
        ui_->scanMinersButton->setEnabled(scanButton == ButtonStatus::ENABLED);
        ui_->scanMinersButton->setText(tr("Scan"));
    }

    if (monitorButton == ButtonStatus::PREPARE)
    {
        ui_->monitorMinersButton->setEnabled(false);
    }
    else if (monitorButton == ButtonStatus::STOPABLED)
    {
        ui_->monitorMinersButton->setEnabled(true);
        ui_->monitorMinersButton->setText(tr("Stop Monitor"));
    }
    else
    {
        ui_->monitorMinersButton->setEnabled(monitorButton == ButtonStatus::ENABLED);
        ui_->monitorMinersButton->setText(tr("Monitor"));
    }

    if (configAllButton == ButtonStatus::PREPARE)
    {
        ui_->configAllMinersButton->setEnabled(false);
    }
    else if (configAllButton == ButtonStatus::STOPABLED)
    {
        ui_->configAllMinersButton->setEnabled(true);
        ui_->configAllMinersButton->setText(tr("Stop Config"));
    }
    else
    {
        ui_->configAllMinersButton->setEnabled(configAllButton == ButtonStatus::ENABLED);
        ui_->configAllMinersButton->setText(tr("Config All"));
    }

    if (configSelButton == ButtonStatus::PREPARE)
    {
        ui_->configSelectedMinersButton->setEnabled(false);
    }
    else if (configSelButton == ButtonStatus::STOPABLED)
    {
        ui_->configSelectedMinersButton->setEnabled(true);
        ui_->configSelectedMinersButton->setText(tr("Stop Config"));
    }
    else
    {
        ui_->configSelectedMinersButton->setEnabled(configSelButton == ButtonStatus::ENABLED);
        ui_->configSelectedMinersButton->setText(tr("Config Selected"));
    }

    if (rebootAllButton == ButtonStatus::PREPARE)
    {
        ui_->rebootAllMinersButton->setEnabled(false);
    }
    else if (rebootAllButton == ButtonStatus::STOPABLED)
    {
        ui_->rebootAllMinersButton->setEnabled(true);
        ui_->rebootAllMinersButton->setText(tr("Stop Reboot"));
    }
    else
    {
        ui_->rebootAllMinersButton->setEnabled(rebootAllButton == ButtonStatus::ENABLED);
        ui_->rebootAllMinersButton->setText(tr("Reboot All"));
    }

    if (rebootSelButton == ButtonStatus::PREPARE)
    {
        ui_->rebootSelectedMinersButton->setEnabled(false);
    }
    else if (rebootSelButton == ButtonStatus::STOPABLED)
    {
        ui_->rebootSelectedMinersButton->setEnabled(true);
        ui_->rebootSelectedMinersButton->setText(tr("Stop Reboot"));
    }
    else
    {
        ui_->rebootSelectedMinersButton->setEnabled(rebootSelButton == ButtonStatus::ENABLED);
        ui_->rebootSelectedMinersButton->setText(tr("Reboot Selected"));
    }

    if (upgradeButton == ButtonStatus::PREPARE)
    {
        ui_->firmwareUpgradeButton->setEnabled(false);
    }
    else if (upgradeButton == ButtonStatus::STOPABLED)
    {
        ui_->firmwareUpgradeButton->setEnabled(true);
        ui_->firmwareUpgradeButton->setText(tr("Stop Upgrade"));
    }
    else
    {
        ui_->firmwareUpgradeButton->setEnabled(upgradeButton == ButtonStatus::ENABLED);
        ui_->firmwareUpgradeButton->setText(tr("Firmware Upgrade"));
    }
}

AppStatus MainWindow::getAppStatus()
{
    return appStatus_;
}

void MainWindow::waitMonitorRefresh(int timeout)
{
    if (!monitorRunning_) {
        return;
    }

    if (timeout > 0)
    {
        ui_->actionProgress->setFormat(tr("%p% - Refresh after %1 seconds. Miner status count: ").arg(timeout) + getMinerCountStr(false));

        // delay n seconds to restart, avoiding refresh too fast
        QTimer::singleShot(1000, this, [this, timeout](){
            waitMonitorRefresh(timeout - 1);
        });
    }
    else
    {
        // The scanning has taken <timeout> seconds. Refresh now.
        onMonitorRefresh();
    }
}

void MainWindow::loadUiConfig()
{
    QSettings conf(UI_CONFIG_FILE_PATH, QSettings::IniFormat);

    ui_->onlySuccessMiners->setChecked(conf.value("ui/onlySuccessMiners", ui_->onlySuccessMiners->isChecked()).toBool());

    ui_->pool1Enabled->setChecked(conf.value("ui/pool1Enabled", ui_->pool1Enabled->isChecked()).toBool());
    ui_->pool2Enabled->setChecked(conf.value("ui/pool2Enabled", ui_->pool2Enabled->isChecked()).toBool());
    ui_->pool3Enabled->setChecked(conf.value("ui/pool3Enabled", ui_->pool3Enabled->isChecked()).toBool());

    ui_->pool1Url->setText(conf.value("ui/pool1Url", ui_->pool1Url->text()).toString());
    ui_->pool2Url->setText(conf.value("ui/pool2Url", ui_->pool2Url->text()).toString());
    ui_->pool3Url->setText(conf.value("ui/pool3Url", ui_->pool3Url->text()).toString());

    ui_->pool1Worker->setText(conf.value("ui/pool1Worker", ui_->pool1Worker->text()).toString());
    ui_->pool2Worker->setText(conf.value("ui/pool2Worker", ui_->pool2Worker->text()).toString());
    ui_->pool3Worker->setText(conf.value("ui/pool3Worker", ui_->pool3Worker->text()).toString());

    ui_->pool1Pwd->setText(conf.value("ui/pool1Pwd", ui_->pool1Pwd->text()).toString());
    ui_->pool2Pwd->setText(conf.value("ui/pool2Pwd", ui_->pool2Pwd->text()).toString());
    ui_->pool3Pwd->setText(conf.value("ui/pool3Pwd", ui_->pool3Pwd->text()).toString());

    ui_->pool1PostfixIp->setChecked(conf.value("ui/pool1PostfixIp", ui_->pool1PostfixIp->isChecked()).toBool());
    ui_->pool2PostfixIp->setChecked(conf.value("ui/pool2PostfixIp", ui_->pool2PostfixIp->isChecked()).toBool());
    ui_->pool3PostfixIp->setChecked(conf.value("ui/pool3PostfixIp", ui_->pool3PostfixIp->isChecked()).toBool());

    ui_->pool1PostfixNoChange->setChecked(conf.value("ui/pool1PostfixNoChange", ui_->pool1PostfixNoChange->isChecked()).toBool());
    ui_->pool2PostfixNoChange->setChecked(conf.value("ui/pool2PostfixNoChange", ui_->pool2PostfixNoChange->isChecked()).toBool());
    ui_->pool3PostfixNoChange->setChecked(conf.value("ui/pool3PostfixNoChange", ui_->pool3PostfixNoChange->isChecked()).toBool());

    ui_->pool1PostfixNone->setChecked(conf.value("ui/pool1PostfixNone", ui_->pool1PostfixNone->isChecked()).toBool());
    ui_->pool2PostfixNone->setChecked(conf.value("ui/pool2PostfixNone", ui_->pool2PostfixNone->isChecked()).toBool());
    ui_->pool3PostfixNone->setChecked(conf.value("ui/pool3PostfixNone", ui_->pool3PostfixNone->isChecked()).toBool());

    ui_->powerControlEnabled->setChecked(conf.value("ui/powerControlEnabled", ui_->powerControlEnabled->isChecked()).toBool());
    ui_->asicBoost->setChecked(conf.value("ui/asicBoost", ui_->asicBoost->isChecked()).toBool());
    ui_->lowPowerMode->setChecked(conf.value("ui/lowPowerMode", ui_->lowPowerMode->isChecked()).toBool());
    ui_->economicMode->setChecked(conf.value("ui/economicMode", ui_->economicMode->isChecked()).toBool());
    ui_->overclockEnabled->setChecked(conf.value("ui/overclockEnabled", ui_->overclockEnabled->isChecked()).toBool());

    // load ip ranges
    setIpRangeGroups(conf.value("ui/ipRangeGroups").toString());

    // auto import IP ranges if it's empty
    if (ui_->ipRangeBox->count() == 0)
    {
        on_importLanRange_clicked();
    }

    // monitor interval
    settingWindow_.setMonitorInterval(conf.value("monitor/interval", MONITOR_INTERVAL).toInt());
    // scan session timeout
    scanner_.setSessionTimeout(conf.value("scanner/sessionTimeout", scanner_.sessionTimeout()).toInt());
    // config & reboot timeout
    configurator_.setSessionTimeout(conf.value("configurator/sessionTimeout", configurator_.sessionTimeout()).toInt());
    rebooter_.setSessionTimeout(configurator_.sessionTimeout());
    // upgrade timeout
    upgrader_.setSessionTimeout(conf.value("upgrader/sessionTimeout", upgrader_.sessionTimeout()).toInt());

    // scan step size
    scanner_.setStepSize(conf.value("scanner/stepSize", scanner_.stepSize()).toInt());
    // config, reboot & upgrade stepSize
    configurator_.setStepSize(conf.value("configurator/stepSize", configurator_.stepSize()).toInt());
    rebooter_.setStepSize(configurator_.stepSize());
    upgrader_.setStepSize(configurator_.stepSize());
    // upgrade stepSize when sending firmware
    upgrader_.setSendFirmwareStepSize(conf.value("upgrader/sendFirmwareStepSize", upgrader_.sendFirmwareStepSize()).toInt());

    // auto retry times
    settingWindow_.setAutoRetryTimes(conf.value("scanner/autoRetryTimes", AUTO_RETRY_TIMES).toInt());
    scanner_.setAutoRetryTimes(settingWindow_.autoRetryTimes());
    configurator_.setAutoRetryTimes(settingWindow_.autoRetryTimes());
    rebooter_.setAutoRetryTimes(settingWindow_.autoRetryTimes());
    upgrader_.setAutoRetryTimes(settingWindow_.autoRetryTimes());

    // firmwares
    upgradeWindow_.setFirmwareMap(Utils::stringToFirmwareMap(conf.value("upgrader/firmwares",
        Utils::firmwareMapToString(upgradeWindow_.getFirmwareMap())).toString()));

    // ip parts for worker name
    configurator_.setWorkerNameIpParts(conf.value("configurator/workerNameIpParts", configurator_.workerNameIpParts()).toInt());

    // highlight options
    minerModel_->setIsHighlightTemperature(conf.value("highlight/isHighlightTemperature", minerModel_->isHighlightTemperature()).toBool());
    minerModel_->setHighlightTemperatureMoreThan(conf.value("highlight/highlightTemperatureMoreThan", minerModel_->highlightTemperatureMoreThan()).toInt());
    minerModel_->setHighlightTemperatureLessThan(conf.value("highlight/highlightTemperatureLessThan", minerModel_->highlightTemperatureLessThan()).toInt());
    minerModel_->setIsHighlightTemperature(conf.value("highlight/isHighlightWrongWorkerName", minerModel_->isHighlightWrongWorkerName()).toBool());
    minerModel_->setIsHighlightTemperature(conf.value("highlight/isHighlightLowHashrate", minerModel_->isHighlightLowHashrate()).toBool());
    minerModel_->setHighlightLowHashrates(Utils::stringToMinerHashrateList(conf.value("highlight/highlightLowHashrates",
        Utils::minerHashrateListToString(minerModel_->highlightLowHashrates())).toString()));

    // miner password
    /*minerPasswordList_ = Utils::stringToMinerPasswordList(conf.value("login/minerPasswords",
        Utils::minerPasswordListToString(minerPasswordList_)).toString());*/

    // others
    openMinerCPWithPassword_ = conf.value("feature/openMinerCPWithPassword", openMinerCPWithPassword_).toBool();

    // reset some config by version
    int appVersionId = conf.value("core/appVersionId", APP_VERSION_ID).toInt();
    if (appVersionId < 30) {
        // Reduce configuration timeout to reduce the time required to configure some new miner firmwares
        configurator_.setSessionTimeout(CONFIG_SESSION_TIMEOUT);
        rebooter_.setSessionTimeout(CONFIG_SESSION_TIMEOUT);
    }
}

void MainWindow::saveUiConfig()
{
    QSettings conf(UI_CONFIG_FILE_PATH, QSettings::IniFormat);

    conf.setValue("core/appVersionId", APP_VERSION_ID);

    conf.setValue("ui/onlySuccessMiners", ui_->onlySuccessMiners->isChecked());

    conf.setValue("ui/pool1Enabled", ui_->pool1Enabled->isChecked());
    conf.setValue("ui/pool2Enabled", ui_->pool2Enabled->isChecked());
    conf.setValue("ui/pool3Enabled", ui_->pool3Enabled->isChecked());

    conf.setValue("ui/pool1Url", ui_->pool1Url->text());
    conf.setValue("ui/pool2Url", ui_->pool2Url->text());
    conf.setValue("ui/pool3Url", ui_->pool3Url->text());

    conf.setValue("ui/pool1Worker", ui_->pool1Worker->text());
    conf.setValue("ui/pool2Worker", ui_->pool2Worker->text());
    conf.setValue("ui/pool3Worker", ui_->pool3Worker->text());

    conf.setValue("ui/pool1Pwd", ui_->pool1Pwd->text());
    conf.setValue("ui/pool2Pwd", ui_->pool2Pwd->text());
    conf.setValue("ui/pool3Pwd", ui_->pool3Pwd->text());

    conf.setValue("ui/pool1PostfixIp", ui_->pool1PostfixIp->isChecked());
    conf.setValue("ui/pool2PostfixIp", ui_->pool2PostfixIp->isChecked());
    conf.setValue("ui/pool3PostfixIp", ui_->pool3PostfixIp->isChecked());

    conf.setValue("ui/pool1PostfixNoChange", ui_->pool1PostfixNoChange->isChecked());
    conf.setValue("ui/pool2PostfixNoChange", ui_->pool2PostfixNoChange->isChecked());
    conf.setValue("ui/pool3PostfixNoChange", ui_->pool3PostfixNoChange->isChecked());

    conf.setValue("ui/pool1PostfixNone", ui_->pool1PostfixNone->isChecked());
    conf.setValue("ui/pool2PostfixNone", ui_->pool2PostfixNone->isChecked());
    conf.setValue("ui/pool3PostfixNone", ui_->pool3PostfixNone->isChecked());

    conf.setValue("ui/powerControlEnabled", ui_->powerControlEnabled->isChecked());
    conf.setValue("ui/asicBoost", ui_->asicBoost->isChecked());
    conf.setValue("ui/lowPowerMode", ui_->lowPowerMode->isChecked());
    conf.setValue("ui/economicMode", ui_->economicMode->isChecked());
    conf.setValue("ui/overclockEnabled", ui_->overclockEnabled->isChecked());

    // save ip ranges
    conf.setValue("ui/ipRangeGroups", getIpRangeGroups());

    // monitor interval
    conf.setValue("monitor/interval", settingWindow_.monitorInterval());

    // session timeout
    conf.setValue("scanner/sessionTimeout", scanner_.sessionTimeout());
    conf.setValue("configurator/sessionTimeout", configurator_.sessionTimeout());
    conf.setValue("upgrader/sessionTimeout", upgrader_.sessionTimeout());

    // scan step size
    conf.setValue("scanner/stepSize", scanner_.stepSize());
    // config, reboot & upgrade step size
    conf.setValue("configurator/stepSize", upgrader_.stepSize());
    // upgrade step size when sending firmware
    conf.setValue("upgrader/sendFirmwareStepSize", upgrader_.sendFirmwareStepSize());

    // auto retry times
    conf.setValue("scanner/autoRetryTimes", settingWindow_.autoRetryTimes());

    // firmwares
    conf.setValue("upgrader/firmwares", Utils::firmwareMapToString(upgradeWindow_.getFirmwareMap()));

    // ip parts for worker name
    conf.setValue("configurator/workerNameIpParts", configurator_.workerNameIpParts());

    // highlight options
    conf.setValue("highlight/isHighlightTemperature", minerModel_->isHighlightTemperature());
    conf.setValue("highlight/highlightTemperatureMoreThan", minerModel_->highlightTemperatureMoreThan());
    conf.setValue("highlight/highlightTemperatureLessThan", minerModel_->highlightTemperatureLessThan());
    conf.setValue("highlight/isHighlightWrongWorkerName", minerModel_->isHighlightWrongWorkerName());
    conf.setValue("highlight/isHighlightLowHashrate", minerModel_->isHighlightLowHashrate());
    conf.setValue("highlight/highlightLowHashrates", Utils::minerHashrateListToString(minerModel_->highlightLowHashrates()));

    // miner password
    conf.setValue("login/minerPasswords", "" /*Utils::minerPasswordListToString(minerPasswordList_)*/);

    // others
    conf.setValue("feature/openMinerCPWithPassword", openMinerCPWithPassword_);
}

void MainWindow::syncOptionsFromSettingWindow()
{
    scanner_.setSessionTimeout(settingWindow_.scanningTimeout());
    scanner_.setStepSize(settingWindow_.concurrentScanning());

    configurator_.setSessionTimeout(settingWindow_.configuringTimeout());
    configurator_.setStepSize(settingWindow_.concurrentConfiguring());

    rebooter_.setSessionTimeout(configurator_.sessionTimeout());
    rebooter_.setStepSize(configurator_.stepSize());

    upgrader_.setSessionTimeout(settingWindow_.upgradingTimeout());
    upgrader_.setStepSize(configurator_.stepSize());
    upgrader_.setSendFirmwareStepSize(settingWindow_.concurrentUpgrading());

    // ip parts for worker name
    configurator_.setWorkerNameIpParts(settingWindow_.workerNameIpParts());

    // auto retry times
    scanner_.setAutoRetryTimes(settingWindow_.autoRetryTimes());
    configurator_.setAutoRetryTimes(settingWindow_.autoRetryTimes());
    rebooter_.setAutoRetryTimes(settingWindow_.autoRetryTimes());
    upgrader_.setAutoRetryTimes(settingWindow_.autoRetryTimes());

    minerModel_->setIsHighlightTemperature(settingWindow_.isHighlightTemperature());
    minerModel_->setHighlightTemperatureMoreThan(settingWindow_.highlightTemperatureMoreThan());
    minerModel_->setHighlightTemperatureLessThan(settingWindow_.highlightTemperatureLessThan());

    minerModel_->setIsHighlightLowHashrate(settingWindow_.isHighlightLowHashrate());
    minerModel_->setHighlightLowHashrates(settingWindow_.highlightLowHashrates());

    minerModel_->setIsHighlightWrongWorkerName(settingWindow_.isHighlightWrongWorkerName());

    minerPasswordList_ = settingWindow_.minerPasswords();

    openMinerCPWithPassword_ = settingWindow_.isOpenMinerCPWithPassword();
}

void MainWindow::syncOptionsToSettingWindow()
{
    settingWindow_.setScanningTimeout(scanner_.sessionTimeout());
    settingWindow_.setConcurrentScanning(scanner_.stepSize());

    settingWindow_.setConfiguringTimeout(configurator_.sessionTimeout());
    settingWindow_.setConcurrentConfiguring(configurator_.stepSize());

    settingWindow_.setUpgradingTimeout(upgrader_.sessionTimeout());
    settingWindow_.setConcurrentUpgrading(upgrader_.sendFirmwareStepSize());

    // ip parts for worker name
    settingWindow_.setWorkerNameIpParts(configurator_.workerNameIpParts());

    settingWindow_.setIsHighlightTemperature(minerModel_->isHighlightTemperature());
    settingWindow_.setHighlightTemperatureMoreThan(minerModel_->highlightTemperatureMoreThan());
    settingWindow_.setHighlightTemperatureLessThan(minerModel_->highlightTemperatureLessThan());

    settingWindow_.setIsHighlightLowHashrate(minerModel_->isHighlightLowHashrate());
    settingWindow_.setHighlightLowHashrates(minerModel_->highlightLowHashrates());

    settingWindow_.setIsHighlightWrongWorkerName(minerModel_->isHighlightWrongWorkerName());

    settingWindow_.setMinerPasswords(minerPasswordList_);

    settingWindow_.setIsOpenMinerCPWithPassword(openMinerCPWithPassword_);
}

void MainWindow::addIpRangeFromString(IpRangeListItem *position, QString ipRangeStr)
{
    ipRangeStr = ipRangeStr.trimmed();

    if (ipRangeStr.isEmpty())
    {
        return;
    }

    int row = ui_->ipRangeBox->count();

    if (position != nullptr)
    {
        row = ui_->ipRangeBox->row(position) + 1;
    }

    auto item = new IpRangeListItem(ui_->ipRangeBox, row, this);

    item->setFromString(ipRangeStr);
}

void MainWindow::addIpRangeWithNewWindow(IpRangeListItem *position, bool enabled)
{
    int row = ui_->ipRangeBox->count();

    if (position != nullptr)
    {
        row = ui_->ipRangeBox->row(position) + 1;
    }

    auto item = new IpRangeListItem(ui_->ipRangeBox, row, this);

    item->setEnabled(enabled);
    bool accepted = item->editWithNewWindow();

    if (!accepted)
    {
        removeIpRange(item);
    }
}

void MainWindow::removeIpRange(IpRangeListItem *ipRange)
{
    ui_->ipRangeBox->removeItemWidget(ipRange);
    delete ipRange;
}

btctools::utils::IpGeneratorGroup MainWindow::makeIpRangeGenerators()
{
    btctools::utils::IpGeneratorGroup ipg;
    int size = ui_->ipRangeBox->count();

    for (int i=0; i<size; i++)
    {
        IpRangeListItem *item = dynamic_cast<IpRangeListItem *>(ui_->ipRangeBox->item(i));

        if (item->isEnabled())
        {
            foreach (QString ipRange, item->enabledIpRanges())
            {
                ipg.addIpRange(ipRange.toStdString());
            }
        }
    }

    return ipg;
}

QString MainWindow::getIpRangeGroups()
{
    QString ipRanges = "";

    int size = ui_->ipRangeBox->count();
    int separatorEnd = size - 1;

    for (int i=0; i<size; i++)
    {
        IpRangeListItem *item = dynamic_cast<IpRangeListItem *>(ui_->ipRangeBox->item(i));

        ipRanges += item->toString();

        if (i < separatorEnd)
        {
            ipRanges += ";";
        }
    }

    return ipRanges;
}

void MainWindow::setIpRangeGroups(QString ipRanges)
{
    QStringList rangeList = ipRanges.split(";");

    for (QString range : rangeList)
    {
        addIpRangeFromString(nullptr, range);
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    saveUiConfig();
}

QString MainWindow::getMinerCountStr(bool onlySelected)
{
    int allMinerNum = 0;
    int successMinerNum = 0;
    int okMinerNum = 0;
    int skipMinerNum = 0;
    int timeoutMinerNum = 0;
    int rebootedMinerNum = 0;
    int upgradedMinerNum = 0;
    int otherMinerNum = 0;

    if (onlySelected)
    {
        allMinerNum = selectedMinerItems_.size();

        foreach (const MinerItem *item, selectedMinerItems_)
        {
            const std::string &stat = item->miner_.stat_;

            if (stat == "success")
            {
                successMinerNum++;
            }
            else if (stat == "ok")
            {
                okMinerNum++;
            }
            else if (stat == "skip")
            {
                skipMinerNum++;
            }
            else if (stat == "timeout")
            {
                timeoutMinerNum++;
            }
            else if (stat == "rebooted")
            {
                rebootedMinerNum++;
            }
            else if (stat == "upgraded")
            {
                upgradedMinerNum++;
            }
            else
            {
                otherMinerNum++;
            }
        }
    }
    else
    {
        minerModel_->updateMinerNumCount();

        allMinerNum = minerModel_->allMinerNum();
        successMinerNum = minerModel_->successMinerNum();
        okMinerNum = minerModel_->okMinerNum();
        skipMinerNum = minerModel_->skipMinerNum();
        timeoutMinerNum = minerModel_->timeoutMinerNum();
        rebootedMinerNum = minerModel_->rebootedMinerNum();
        upgradedMinerNum = minerModel_->upgradedMinerNum();
        otherMinerNum = minerModel_->otherMinerNum();
    }

    QStringList minerCount;

    minerCount.append(tr("all %1").arg(allMinerNum));

    if (successMinerNum > 0)
    {
        minerCount.append(tr("success %1").arg(successMinerNum));
    }

    if (okMinerNum > 0)
    {
        minerCount.append(tr("ok %1").arg(okMinerNum));
    }

    if (skipMinerNum > 0)
    {
        minerCount.append(tr("skip %1").arg(skipMinerNum));
    }

    if (timeoutMinerNum > 0)
    {
        minerCount.append(tr("timeout %1").arg(timeoutMinerNum));
    }

    if (rebootedMinerNum > 0)
    {
        minerCount.append(tr("rebooted %1").arg(rebootedMinerNum));
    }

    if (upgradedMinerNum > 0)
    {
        minerCount.append(tr("upgraded %1").arg(upgradedMinerNum));
    }

    if (otherMinerNum > 0)
    {
        minerCount.append(tr("other %1").arg(otherMinerNum));
    }

    return minerCount.join(", ");
}

bool MainWindow::checkPoolsAndNotice()
{
    if (!ui_->pool1Enabled->isChecked() && !ui_->pool2Enabled->isChecked() && !ui_->pool3Enabled->isChecked() &&
        !ui_->powerControlEnabled->isChecked() && !ui_->overclockEnabled->isChecked())
    {
        QMessageBox::information(this, tr("Cannot configure miners"), tr("All configurations disabled.\nPlease enable at least one configuration\nby checking the box in front of it."));
        return false;
    }

    // TODO: not finished

    if (ui_->pool1Enabled->isChecked())
    {
        bool ok = checkOnePoolAndNotice(1, ui_->pool1Url, ui_->pool1Worker);

        if (!ok)
        {
            return false;
        }
    }

    if (ui_->pool2Enabled->isChecked())
    {
        bool ok = checkOnePoolAndNotice(2, ui_->pool2Url, ui_->pool2Worker);

        if (!ok)
        {
            return false;
        }
    }

    if (ui_->pool3Enabled->isChecked())
    {
        bool ok = checkOnePoolAndNotice(3, ui_->pool3Url, ui_->pool3Worker);

        if (!ok)
        {
            return false;
        }
    }

    return true;
}

bool MainWindow::checkIpRangesAndNotice()
{
    int size = ui_->ipRangeBox->count();
    int availableIpRangeNum = 0;

    for (int i=0; i<size; i++)
    {
        IpRangeListItem *item = dynamic_cast<IpRangeListItem *>(ui_->ipRangeBox->item(i));

        if (item->isEnabled())
        {
            availableIpRangeNum += item->enabledIpRanges().size();
        }
    }

    if (availableIpRangeNum == 0)
    {
        QMessageBox::information(this, tr("IP Range is Empty"), tr("Please add & enable at least one IP range."));
        return false;
    }

    return true;
}

void MainWindow::onReceivedMiner(btctools::miner::Miner miner)
{
    ui_->actionProgress->setFormat(actionProgressTextFormat_.arg(QString(miner.ip_.c_str())));

    if (onlySuccessMiners_ && miner.stat_ != "success" && miner.stat_ != "login failed")
    {
        return;
    }

    minerModel_->addMiner(miner);
}

void MainWindow::onActionBegin()
{
    ui_->actionProgress->setEnabled(true);
    ui_->actionProgress->setValue(0);
}

void MainWindow::onActionProgress(int percent)
{
    ui_->actionProgress->setValue(percent);
}

void MainWindow::onActionEnd()
{
    ui_->actionProgress->setEnabled(false);
    ui_->actionProgress->setValue(100);
}

void MainWindow::onScanBegin()
{
    bool isMonitor = getAppStatus() == AppStatus::MONITOR_PREPARE;

    setAppStatus(isMonitor ? AppStatus::MONITORING : AppStatus::SCANNING);

    if (isMonitor)
    {
        lastRefreshTime_ = time(nullptr);
    }
}

void MainWindow::onScanEnd()
{
    if (monitorRunning_)
    {

        int timeout = settingWindow_.monitorInterval() - (time(nullptr) - lastRefreshTime_);
        waitMonitorRefresh(timeout);
    }
    else
    {
        bool isMonitor = getAppStatus() == AppStatus::MONITOR_PREPARE;

        if (isMonitor)
        {
            ui_->actionProgress->setFormat(tr("%p% - Monitor stopped. Miner status count: ") + getMinerCountStr(false));
            QMessageBox::information(this, tr("Complete"), tr("Monitor stopped. Miner status count: %1.").arg(getMinerCountStr(false)));
        }
        else
        {
            ui_->actionProgress->setFormat(tr("%p% - Scanning complete") + ", " + getMinerCountStr(false));
            QMessageBox::information(this, tr("Complete"), tr("Scanning complete, %1.").arg(getMinerCountStr(false)));
        }

        setAppStatus(AppStatus::SCAN_FINISH);
    }

    updateOverclockOption();
}

void MainWindow::onMonitorRefresh()
{
    if (!monitorRunning_)
    {
        return;
    }

    actionProgressTextFormat_ = tr("%p% - Refreshing miner's info: %1. Miner status count: ") + getMinerCountStr(false);

    setAppStatus(AppStatus::MONITOR_PREPARE);
    scanner_.wait();
    scanner_.setIpRange(makeIpRangeGenerators());
    scanner_.start();
}

void MainWindow::onConfigBegin()
{
    bool isSelected = getAppStatus() == AppStatus::CONFIG_SEL_PREPARE;

    setAppStatus(isSelected ? AppStatus::CONFIGURATING_SEL : AppStatus::CONFIGURATING_ALL);
}

void MainWindow::onConfigEnd(bool allSkip)
{
    bool onlySelected = getAppStatus() == AppStatus::CONFIGURATING_SEL;
    setAppStatus(AppStatus::CONFIG_FINISH);

    ui_->actionProgress->setFormat(tr("%p% - Configuring complete")  + ", " + getMinerCountStr(onlySelected));

    if (allSkip)
    {
        QMessageBox::information(this, tr("No Miner to Configure"), tr("No miner to configure, or all miners configured successfully at the last time.\n"
                                                                       "Please scan miners if you want to configure again."));
    }
    else
    {
        QMessageBox::information(this, tr("Complete"), tr("Configuring complete, %1.").arg(getMinerCountStr(onlySelected)));
    }

    // Uncheck re-overclocking to prevent unexpected retries
    ui_->reOverclocking->setChecked(false);
}

void MainWindow::onRebootBegin()
{
    bool isSelected = getAppStatus() == AppStatus::REBOOT_SEL_PREPARE;

    setAppStatus(isSelected ? AppStatus::REBOOTING_SEL : AppStatus::REBOOTING_ALL);
}

void MainWindow::onRebootEnd(bool allSkip)
{
    bool onlySelected = getAppStatus() == AppStatus::REBOOTING_SEL;
    setAppStatus(AppStatus::REBOOT_FINISH);

    ui_->actionProgress->setFormat(tr("%p% - Rebooting complete")  + ", " + getMinerCountStr(onlySelected));

    if (allSkip)
    {
        QMessageBox::information(this, tr("No Miner to Reboot"), tr("No miner to reboot, or all miners you selected be rebooted successfully at the last time.\n"
                                                                    "Please select another miners."));
    }
    else
    {
        QMessageBox::information(this, tr("Complete"), tr("Rebooting complete, %1.").arg(getMinerCountStr(onlySelected)));
    }
}

void MainWindow::onUpgradeBegin()
{
    bool isSelected = getAppStatus() == AppStatus::UPGRADE_SEL_PREPARE;

    setAppStatus(isSelected ? AppStatus::UPGRADING_SEL : AppStatus::UPGRADING_ALL);
}

void MainWindow::onUpgradeEnd(bool allSkip)
{
    bool onlySelected = getAppStatus() == AppStatus::UPGRADING_SEL;
    setAppStatus(AppStatus::UPGRADE_FINISH);

    ui_->actionProgress->setFormat(tr("%p% - Upgrading complete")  + ", " + getMinerCountStr(onlySelected));

    if (allSkip)
    {
        QMessageBox::information(this, tr("No Miner to Upgrade"), tr("No miner to upgrade, or all miners you selected be upgraded successfully at the last time.\n"
                                                                     "Please select another miners."));
    }
    else
    {
        QMessageBox::information(this, tr("Complete"), tr("Upgrading complete, %1.").arg(getMinerCountStr(onlySelected)));
    }
}

void MainWindow::onFindUpdates(QString versionName, QString description, QString downloadUrl, bool forceUpdate)
{

    if (forceUpdate)
    {
        QMessageBox::information(this, tr("BTC Tools Needs a Update"), tr("The current version %1 is outdated.\n"
                                                                          "Please download the new version %2 from pool.btc.com.").
                                                                          arg(APP_VERSION_NAME).arg(versionName));
        QDesktopServices::openUrl(QUrl(downloadUrl));
        close();
    }
    else
    {
        auto button = QMessageBox::information(this, tr("A New Version is Available"), tr("New features of BTC Tools %1:\n\n%2\n\nDo you want to download it now?").
                                               arg(versionName).arg(description), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (button == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(QUrl(downloadUrl));
        }
    }
}

void MainWindow::copyMinerItemToClipboard(QModelIndex *pIndex)
{
    QModelIndex index;
    if (pIndex == nullptr)
    {
        index = ui_->minersTable->currentIndex();
    } else {
        index = *pIndex;
    }

    if (!index.isValid())
    {
        return;
    }

    QString data = minerModel_->data(index).toString();
    QApplication::clipboard()->setText(data);
}

void MainWindow::openMinerControlPanel(QModelIndex *pIndex)
{
    QModelIndex index;
    if (pIndex == nullptr)
    {
        index = ui_->minersTable->currentIndex();
    } else {
        index = *pIndex;
    }

    if (!index.isValid())
    {
        return;
    }

    const MinerItem &item = minerModel_->getMiner(index.row());

    QString url;

    if (openMinerCPWithPassword_ && item.miner_.typeStr_ == "AntminerHttpCgi")
    {
        QString passwordStr = "";
        MinerPassword *p = Utils::findMinerPassword(minerPasswordList_, item.miner_.fullTypeStr_.c_str());

        if (p != nullptr)
        {
            passwordStr = QString("%1:%2@").arg(p->userName_).arg(p->password_);
        }

        url = QString("http://%1%2/").arg(passwordStr).arg(item.miner_.ip_.c_str());
    }
    else
    {
        url = QString("http://%1/").arg(item.miner_.ip_.c_str());
    }

    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::fillPoolsInForm(QModelIndex *pIndex)
{
    QModelIndex index;
    if (pIndex == nullptr)
    {
        index = ui_->minersTable->currentIndex();
    } else {
        index = *pIndex;
    }

    if (!index.isValid())
    {
        return;
    }

    const Miner &miner = minerModel_->getMiner(index.row()).miner_;

    ui_->pool1Url->setText(miner.pool1_.url_.c_str());
    ui_->pool2Url->setText(miner.pool2_.url_.c_str());
    ui_->pool3Url->setText(miner.pool3_.url_.c_str());

    ui_->pool1Worker->setText(Utils::getSubAccountName(miner.pool1_.worker_.c_str()));
    ui_->pool2Worker->setText(Utils::getSubAccountName(miner.pool2_.worker_.c_str()));
    ui_->pool3Worker->setText(Utils::getSubAccountName(miner.pool3_.worker_.c_str()));

    ui_->pool1Pwd->setText(miner.pool1_.passwd_.c_str());
    ui_->pool2Pwd->setText(miner.pool2_.passwd_.c_str());
    ui_->pool3Pwd->setText(miner.pool3_.passwd_.c_str());
}

void MainWindow::onMinerTableContextMenuClicked(const QPoint & /* pos */)
{
    if (ui_->minersTable->currentIndex().isValid())
    {
        minerTableContextMenu_->exec(QCursor::pos());
    }
}

void MainWindow::editIpRange()
{
    auto item = dynamic_cast<IpRangeListItem *>(ui_->ipRangeBox->item(ui_->ipRangeBox->currentIndex().row()));
    item->editWithNewWindow();
}

void MainWindow::onIpRangeBoxContextMenuClicked(const QPoint & /* pos */)
{
    if (ui_->ipRangeBox->currentIndex().isValid())
    {
        ipRangeBoxContextMenu_->exec(QCursor::pos());
    }
}

void MainWindow::onUpgradeWindowSelectedMinerModelChanged(QString newMinerModel)
{
    std::string model = newMinerModel.toUtf8().data();
    size_t selected, selectedDisabled, all, allDisabled;
    getSelectedMinerNumberWithModel(newMinerModel, selected, selectedDisabled);
    getAllMinerNumberWithModel(newMinerModel, all, allDisabled);

    upgradeWindow_.updateMinerCounter(newMinerModel, selected, selectedDisabled, all, allDisabled);
}

void MainWindow::getSelectedMinerNumberWithModel(QString minerModel, size_t &selected, size_t &disabled) {
    std::string model = minerModel.toUtf8().data();
    selected = 0;
    disabled = 0;

    for (const auto &item : selectedMinerItems_) {
        if (item->miner_.fullTypeStr_ == model) {
            selected++;
            if (item->miner_.opt("upgrader.disabled") == "true") {
                disabled++;
            }
        }
    }
}

void MainWindow::getAllMinerNumberWithModel(QString minerModel, size_t &all, size_t &disabled) {
    std::string model = minerModel.toUtf8().data();
    all = 0;
    disabled = 0;

    for (const auto &item : minerModel_->getMiners()) {
        if (item->miner_.fullTypeStr_ == model) {
            all++;
            if (item->miner_.opt("upgrader.disabled") == "true") {
                disabled++;
            }
        }
    }
}

void MainWindow::onUpgradeSelectedMiners()
{
    upgradeMiners(true);
}

void MainWindow::onUpgradeAllMiners()
{
    upgradeMiners(false);
}

void MainWindow::upgradeMiners(bool onlySelected)
{
    if (upgrader_.isRunning())
    {
        actionProgressTextFormat_ = tr("%p% - Stopping %1");
        ui_->actionProgress->setFormat(tr("%p% - Stopping..."));

        setAppStatus(onlySelected ? AppStatus::UPGRADE_SEL_PREPARE : AppStatus::UPGRADE_ALL_PREPARE);

        upgrader_.stop();
    }
    else
    {
        QString minerModel = upgradeWindow_.getSelectedMinerModel();
        QString firmware = upgradeWindow_.getSelectedFirmware();
        bool keepSettings = upgradeWindow_.getKeepSettings();

        upgrader_.setMinerModel(minerModel);
        upgrader_.setFirmware(firmware);
        upgrader_.setKeepSettings(keepSettings);

        if (onlySelected)
        {
            // don't need call this, it updated before
            // updateSelectedMinerItems();

            int selectedMinerSize = selectedMinerItems_.size();
            size_t upgradeMinerSize, disabledMinerSize;
            getSelectedMinerNumberWithModel(minerModel, upgradeMinerSize, disabledMinerSize);

            if (selectedMinerSize < 1)
            {
                QMessageBox::information(this, tr("No miner be selected"), tr("Please select one or more miners which you want to upgrade."));
                return;
            }

            if (upgradeMinerSize < 1)
            {
                QMessageBox::information(this, tr("No miner with matching model be selected"), tr("You selected miners are not matching the model (%1) you want to upgrade.").arg(minerModel));
                return;
            }

            if (disabledMinerSize >= upgradeMinerSize) {
                QMessageBox::information(this, tr("Unsupported model"), tr("BTCTools has not yet supported the firmware upgrade of this model, please wait for future updates.").arg(minerModel));
                return;
            }

            auto button = QMessageBox::information(this, tr("You will upgrade %1 miners").arg(upgradeMinerSize),
                                                   tr("Do you want to upgrade %1 %2 that your selected now?%3")
                                                      .arg(upgradeMinerSize)
                                                      .arg(minerModel)
                                                      .arg(selectedMinerSize > upgradeMinerSize
                                                           ? tr("<br>Skip %3 miners that don't match the model.").arg(selectedMinerSize - upgradeMinerSize)
                                                           : QString("")),
                                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (button == QMessageBox::No)
            {
                return;
            }

            setAppStatus(AppStatus::UPGRADE_SEL_PREPARE);
            upgrader_.setMiners(selectedMinerItems_);
        }
        else
        {
            int minerSize = minerModel_->allMinerNum();
            size_t upgradeMinerSize, disabledMinerSize;
            getAllMinerNumberWithModel(minerModel, upgradeMinerSize, disabledMinerSize);

            if (minerSize < 1)
            {
                QMessageBox::information(this, tr("No miner to upgrade"), tr("Please scan miners first."));
                return;
            }

            if (upgradeMinerSize < 1)
            {
                QMessageBox::information(this, tr("No miner with matching model to upgrade"), tr("You selected model (%1) to upgrade is not matching your miners.").arg(minerModel));
                return;
            }

            if (disabledMinerSize >= upgradeMinerSize) {
                QMessageBox::information(this, tr("Unsupported model"), tr("BTCTools has not yet supported the firmware upgrade of this model, please wait for future updates.").arg(minerModel));
                return;
            }

            auto button = CheckMessageBox::information(this, tr("You will upgrade ALL (%1) miners").arg(upgradeMinerSize),
                                         tr("Do you want to upgrade <b>ALL</b> %1 %2 now?%3")
                                            .arg(upgradeMinerSize)
                                            .arg(minerModel)
                                            .arg(minerSize > upgradeMinerSize
                                                 ? tr("<br>Skip %3 miners that don't match the model.").arg(minerSize - upgradeMinerSize)
                                                 : QString("")),
                                         tr("I really want to upgrade ALL (%1) %2.").arg(upgradeMinerSize).arg(minerModel));

            if (button == CheckMessageBox::Cancel)
            {
                return;
            }

            setAppStatus(AppStatus::UPGRADE_ALL_PREPARE);
            upgrader_.setMiners(minerModel_->getMiners());
        }

        actionProgressTextFormat_ = tr("%p% - Upgrading %1");

        onlySuccessMiners_ = false;
        upgrader_.start();
    }

}

void MainWindow::updateOverclockOption()
{
    ui_->overclockModel->clear();
    ui_->overclockWorkingMode->clear();
    ui_->overclockLevelName->clear();

    QStringList models = minerModel_->getMinerModels().toList();
    qSort(models);

    ui_->overclockModel->addItems(models);

    // Add a mouseover tooltip to fully display long information
    for (size_t i=0; i<models.size(); i++) {
        ui_->overclockModel->setItemData(i, models[i], Qt::ToolTipRole);
    }

    if (models.size() > 0) {
        ui_->overclockModel->setCurrentIndex(0);
        syncOverclockWorkingMode(models[0]);
    }
}

void MainWindow::syncOverclockWorkingMode(const QString &model) {
    ui_->overclockWorkingMode->clear();
    ui_->overclockLevelName->clear();

    QStringList modes = minerModel_->getOverclockWorkingMode(model).toList();
    qSort(modes.begin(), modes.end(), qGreater<QString>());

    ui_->overclockWorkingMode->addItems(modes);

    // Add a mouseover tooltip to fully display long information
    size_t normalIndex = 0;
    for (size_t i=0; i<modes.size(); i++) {
        ui_->overclockWorkingMode->setItemData(i, modes[i], Qt::ToolTipRole);
        if (modes[i] == "Normal") {
            normalIndex = i;
        }
    }

    if (modes.size() > 0) {
        ui_->overclockWorkingMode->setCurrentIndex(normalIndex);
        syncOverclockLevelName(model, modes[normalIndex]);
    }
}

void MainWindow::syncOverclockLevelName(const QString &model, const QString &workingMode) {
    ui_->overclockLevelName->clear();

    QStringList levels = minerModel_->getOverclockLevelName(model, workingMode).toList();
    qSort(levels.begin(), levels.end(), qGreater<QString>());

    ui_->overclockLevelName->addItems(levels);

    // Add a mouseover tooltip to fully display long information
    size_t normalIndex = 0;
    for (size_t i=0; i<levels.size(); i++) {
        ui_->overclockLevelName->setItemData(i, levels[i], Qt::ToolTipRole);
        if (levels[i] == "Normal") {
            normalIndex = i;
        }
    }

    if (levels.size() > 0) {
        ui_->overclockLevelName->setCurrentIndex(normalIndex);
    }
}

void MainWindow::onReceivedException(const std::exception &ex)
{
    QString title;
    AppStatus newStatus;

    switch (appStatus_) {
    case AppStatus::SCAN_PREPARE:
    case AppStatus::SCANNING:
        title = tr("Scan Failed");
        newStatus = AppStatus::SCAN_FINISH;
        break;
    case AppStatus::MONITOR_PREPARE:
    case AppStatus::MONITORING:
        title = tr("Monitor Failed");
        newStatus = AppStatus::MONITOR_FINISH;
        break;
    case AppStatus::CONFIG_ALL_PREPARE:
    case AppStatus::CONFIG_SEL_PREPARE:
    case AppStatus::CONFIGURATING_ALL:
    case AppStatus::CONFIGURATING_SEL:
        title = tr("Configure Failed");
        newStatus = AppStatus::CONFIG_FINISH;
        break;
    case AppStatus::REBOOT_ALL_PREPARE:
    case AppStatus::REBOOT_SEL_PREPARE:
    case AppStatus::REBOOTING_ALL:
    case AppStatus::REBOOTING_SEL:
        title = tr("Reboot Failed");
        newStatus = AppStatus::REBOOT_FINISH;
        break;
    case AppStatus::UPGRADE_ALL_PREPARE:
    case AppStatus::UPGRADE_SEL_PREPARE:
    case AppStatus::UPGRADING_ALL:
    case AppStatus::UPGRADING_SEL:
        title = tr("Upgrade Failed");
        newStatus = AppStatus::UPGRADE_FINISH;
        break;
    default:
        title = tr("Unexpected Error");
        newStatus = AppStatus::SCAN_FINISH;
        break;

    }

    QMessageBox::critical(this, title, ex.what());

    // reset status
    setAppStatus(newStatus);
    onActionEnd();
}

void MainWindow::initContextMenu()
{
    // miner table
    copyAction_ = new QAction(tr("Copy"), this);
    connect(copyAction_, SIGNAL(triggered()), this, SLOT(copyMinerItemToClipboard()));

    openMinerCP_ = new QAction(tr("Open Control Panel"), this);
    connect(openMinerCP_, SIGNAL(triggered()), this, SLOT(openMinerControlPanel()));

    fillPoolsInForm_ = new QAction(tr("Fill Pools in Above Form"), this);
    connect(fillPoolsInForm_, SIGNAL(triggered()), this, SLOT(fillPoolsInForm()));

    minerTableContextMenu_ = new QMenu;
    minerTableContextMenu_->addAction(copyAction_);
    minerTableContextMenu_->addAction(openMinerCP_);
    minerTableContextMenu_->addAction(fillPoolsInForm_);

    ui_->minersTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_->minersTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onMinerTableContextMenuClicked(QPoint)));

    // ip range box
    editAction_ = new QAction(tr("Edit"), this);
    connect(editAction_, SIGNAL(triggered()), this, SLOT(editIpRange()));

    ipRangeBoxContextMenu_ = new QMenu;
    ipRangeBoxContextMenu_->addAction(editAction_);

    ui_->ipRangeBox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui_->ipRangeBox, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onIpRangeBoxContextMenuClicked(QPoint)));
}

void MainWindow::on_scanMinersButton_clicked()
{
    if (scanner_.isRunning())
    {
        actionProgressTextFormat_ = tr("%p% - Stopping %1");
        ui_->actionProgress->setFormat(tr("%p% - Stopping..."));

        setAppStatus(AppStatus::SCAN_PREPARE);

        scanner_.stop();
    }
    else
    {
        if (!checkIpRangesAndNotice())
        {
            return;
        }

        setAppStatus(AppStatus::SCAN_PREPARE);

        onlySuccessMiners_ = ui_->onlySuccessMiners->isChecked();
        minerModel_->clear();

        actionProgressTextFormat_ = tr("%p% - Scanning %1");

        scanner_.setIpRange(makeIpRangeGenerators());
        scanner_.start();
    }
}

void MainWindow::on_monitorMinersButton_clicked()
{
    if (monitorRunning_)
    {
        monitorRunning_ = false;

        actionProgressTextFormat_ = tr("%p% - Stopping %1");
        ui_->actionProgress->setFormat(tr("%p% - Stopping..."));

        setAppStatus(AppStatus::MONITOR_PREPARE);

        if (scanner_.isRunning()) {
            scanner_.stop();
        }
        else {
            onScanEnd();
        }
    }
    else
    {
        if (!checkIpRangesAndNotice())
        {
            return;
        }

        setAppStatus(AppStatus::MONITOR_PREPARE);

        onlySuccessMiners_ = ui_->onlySuccessMiners->isChecked();
        minerModel_->clear();

        actionProgressTextFormat_ = tr("%p% - Refreshing miner's info: %1");

        scanner_.setIpRange(makeIpRangeGenerators());

        monitorRunning_ = true;
        scanner_.start();
    }
}

void MainWindow::configMiners(bool onlySelected)
{
    if (configurator_.isRunning())
    {
        actionProgressTextFormat_ = tr("%p% - Stopping %1");
        ui_->actionProgress->setFormat(tr("%p% - Stopping..."));

        setAppStatus(onlySelected ? AppStatus::CONFIG_SEL_PREPARE : AppStatus::CONFIG_ALL_PREPARE);

        configurator_.stop();
    }
    else
    {
        //******* add miners *******
        if (onlySelected)
        {
            updateSelectedMinerItems();

            int selectedMinerSize = selectedMinerItems_.size();

            if (selectedMinerSize < 1)
            {
                QMessageBox::information(this, tr("No miner be selected"), tr("Please select one or more miners which you want to configure."));
                return;
            }

            auto button = QMessageBox::information(this, tr("You will configure %1 miners").arg(selectedMinerSize),
                                                   tr("Do you want to change pools, workers and passwords for %1 miners that your selected now?").arg(selectedMinerSize),
                                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (button == QMessageBox::No)
            {
                return;
            }

            if (!checkPoolsAndNotice())
            {
                return;
            }

            setAppStatus(AppStatus::CONFIG_SEL_PREPARE);
            configurator_.setMiners(selectedMinerItems_);
        }
        else
        {
            int minerSize = minerModel_->allMinerNum();

            if (minerSize < 1)
            {
                QMessageBox::information(this, tr("No miner to configure"), tr("Please scan miners first."));
                return;
            }

            auto button = CheckMessageBox::information(this, tr("You will configure ALL (%1) miners").arg(minerSize),
                                         tr("Do you want to change pools, workers and passwords for <b>ALL</b> (%1) miners now?").arg(minerSize),
                                         tr("I really want to configure ALL (%1) miners.").arg(minerSize));

            if (button == CheckMessageBox::Cancel)
            {
                return;
            }

            if (!checkPoolsAndNotice())
            {
                return;
            }

            setAppStatus(AppStatus::CONFIG_ALL_PREPARE);
            configurator_.setMiners(minerModel_->getMiners());
        }
        //******* end of add miners *******

        //******* set pools *******
        MinerPools pools;

        //--------------- pool 1 ---------------
        pools.pool1Enabled_ = ui_->pool1Enabled->isChecked();
        pools.pool1_.url_ = std::string(ui_->pool1Url->text().trimmed().toUtf8().data());
        pools.pool1_.worker_ = std::string(ui_->pool1Worker->text().trimmed().toUtf8().data());
        pools.pool1_.passwd_ = std::string(ui_->pool1Pwd->text().toUtf8().data());

        if (ui_->pool1PostfixIp->isChecked())
        {
            pools.pool1WorkerPostfix_ = PoolWorkerPostfix::Ip;
        }
        else if (ui_->pool1PostfixNoChange->isChecked())
        {
            pools.pool1WorkerPostfix_ = PoolWorkerPostfix::NoChange;
        }
        else
        {
            pools.pool1WorkerPostfix_ = PoolWorkerPostfix::None;
        }

        //--------------- pool 2 ---------------
        pools.pool2Enabled_ = ui_->pool2Enabled->isChecked();
        pools.pool2_.url_ = std::string(ui_->pool2Url->text().trimmed().toUtf8().data());
        pools.pool2_.worker_ = std::string(ui_->pool2Worker->text().trimmed().toUtf8().data());
        pools.pool2_.passwd_ = std::string(ui_->pool2Pwd->text().toUtf8().data());

        if (ui_->pool2PostfixIp->isChecked())
        {
            pools.pool2WorkerPostfix_ = PoolWorkerPostfix::Ip;
        }
        else if (ui_->pool2PostfixNoChange->isChecked())
        {
            pools.pool2WorkerPostfix_ = PoolWorkerPostfix::NoChange;
        }
        else
        {
            pools.pool2WorkerPostfix_ = PoolWorkerPostfix::None;
        }

        //--------------- pool 3 ---------------
        pools.pool3Enabled_ = ui_->pool3Enabled->isChecked();
        pools.pool3_.url_ = std::string(ui_->pool3Url->text().trimmed().toUtf8().data());
        pools.pool3_.worker_ = std::string(ui_->pool3Worker->text().trimmed().toUtf8().data());
        pools.pool3_.passwd_ = std::string(ui_->pool3Pwd->text().toUtf8().data());

        if (ui_->pool3PostfixIp->isChecked())
        {
            pools.pool3WorkerPostfix_ = PoolWorkerPostfix::Ip;
        }
        else if (ui_->pool3PostfixNoChange->isChecked())
        {
            pools.pool3WorkerPostfix_ = PoolWorkerPostfix::NoChange;
        }
        else
        {
            pools.pool3WorkerPostfix_ = PoolWorkerPostfix::None;
        }

        //--------------- power control ---------------
        pools.powerControlEnabled_ = ui_->powerControlEnabled->isChecked();
        pools.asicBoost_ = ui_->asicBoost->isChecked();
        pools.lowPowerMode_ = ui_->lowPowerMode->isChecked();
        pools.economicMode_ = ui_->economicMode->isChecked();

        //--------------- overclocking ---------------
        pools.overclockEnabled_ = ui_->overclockEnabled->isChecked();
        pools.reOverclocking_ = ui_->reOverclocking->isChecked();
        pools.overclockMinerModel_ = ui_->overclockModel->currentText().toUtf8().data();
        pools.overclockWorkingMode_ = ui_->overclockWorkingMode->currentText();
        pools.overclockLevelName_ = ui_->overclockLevelName->currentText();

        configurator_.setPools(pools);
        //******* end of set pools *******

        actionProgressTextFormat_ = tr("%p% - Configuring %1");

        onlySuccessMiners_ = false;
        configurator_.start();
    }
}

void MainWindow::on_configAllMinersButton_clicked()
{
    configMiners(false);
}

void MainWindow::on_configSelectedMinersButton_clicked()
{
    configMiners(true);
}

void MainWindow::rebootMiners(bool onlySelected)
{
    if (rebooter_.isRunning())
    {
        actionProgressTextFormat_ = tr("%p% - Stopping %1");
        ui_->actionProgress->setFormat(tr("%p% - Stopping..."));

        setAppStatus(onlySelected ? AppStatus::REBOOT_SEL_PREPARE : AppStatus::REBOOT_ALL_PREPARE);

        rebooter_.stop();
    }
    else
    {
        if (onlySelected)
        {
            updateSelectedMinerItems();

            int selectedMinerSize = selectedMinerItems_.size();

            if (selectedMinerSize < 1)
            {
                QMessageBox::information(this, tr("No miner be selected"), tr("Please select one or more miners which you want to reboot."));
                return;
            }

            auto button = QMessageBox::information(this, tr("You will reboot %1 miners").arg(selectedMinerSize),
                                                   tr("Do you want to reboot %1 miners that your selected now?").arg(selectedMinerSize),
                                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (button == QMessageBox::No)
            {
                return;
            }

            setAppStatus(AppStatus::REBOOT_SEL_PREPARE);
            rebooter_.setMiners(selectedMinerItems_);
        }
        else
        {
            int minerSize = minerModel_->allMinerNum();

            if (minerSize < 1)
            {
                QMessageBox::information(this, tr("No miner to reboot"), tr("Please scan miners first."));
                return;
            }


            auto button = CheckMessageBox::information(this, tr("You will reboot ALL (%1) miners").arg(minerSize),
                                         tr("Do you want to reboot <b>ALL</b> %1 miners now?").arg(minerSize),
                                         tr("I really want to reboot ALL (%1) miners.").arg(minerSize));

            if (button == CheckMessageBox::Cancel)
            {
                return;
            }

            setAppStatus(AppStatus::REBOOT_ALL_PREPARE);
            rebooter_.setMiners(minerModel_->getMiners());
        }

        actionProgressTextFormat_ = tr("%p% - Rebooting %1");

        onlySuccessMiners_ = false;
        rebooter_.start();
    }
}

void MainWindow::on_rebootAllMinersButton_clicked()
{
    rebootMiners(false);
}

void MainWindow::on_rebootSelectedMinersButton_clicked()
{
    rebootMiners(true);
}

void MainWindow::on_addIpRange_clicked()
{
    auto selectedIpRange = ui_->ipRangeBox->selectedItems();
    IpRangeListItem *position = nullptr;

    if (!selectedIpRange.isEmpty())
    {
        position = dynamic_cast<IpRangeListItem *>(selectedIpRange.first());
    }

    addIpRangeWithNewWindow(position);
}

void MainWindow::on_removeIpRange_clicked()
{
    auto selectedIpRange = ui_->ipRangeBox->selectedItems();

    if (!selectedIpRange.isEmpty())
    {
        removeIpRange(dynamic_cast<IpRangeListItem *>(selectedIpRange.first()));
    }
    else
    {
        removeIpRange(dynamic_cast<IpRangeListItem *>(ui_->ipRangeBox->item(ui_->ipRangeBox->count() - 1)));
    }
}

void MainWindow::on_importLanRange_clicked()
{
    QStringList lanIpList = Utils::getLanIpList();

    if (!lanIpList.isEmpty())
    {
        for (auto iter=lanIpList.begin(); iter!=lanIpList.end(); iter++)
        {
            *iter = (*iter).left((*iter).lastIndexOf(".")) + ".0-255";
        }

        addIpRangeFromString(nullptr, tr("LAN:") + lanIpList.join(','));
    }
    else
    {
        QMessageBox::information(this, tr("Network is Not Available"), tr("It looks like your computer has no available network.\n"
                                                                          "So auto importing the IP ranges from your network failed.\n"
                                                                          "Please connect to a network and try again, or add IP ranges manually.\n"
                                                                          "But without an available network, you could not scan or configure miners."));
    }
}

void MainWindow::on_ipRangeEnableAll_clicked()
{
    bool enabled = ui_->ipRangeEnableAll->isChecked();
    int size = ui_->ipRangeBox->count();

    for (int i=0; i<size; i++)
    {
        IpRangeListItem *item = dynamic_cast<IpRangeListItem *>(ui_->ipRangeBox->item(i));
        item->setEnabled(enabled);
    }
}

void MainWindow::on_minersTable_doubleClicked(QModelIndex index)
{
    openMinerControlPanel(&index);
}

void MainWindow::on_ipRangeBox_doubleClicked(QModelIndex index)
{
    auto item = dynamic_cast<IpRangeListItem *>(ui_->ipRangeBox->item(index.row()));
    item->editWithNewWindow();
}

void MainWindow::on_settingButton_clicked()
{
    syncOptionsToSettingWindow();

    auto result = settingWindow_.exec();

    if (result == QDialog::Accepted)
    {
        syncOptionsFromSettingWindow();
    }
}

void MainWindow::on_exportToCSVButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export to CSV"), "", tr("CSV Table File (*.csv)"));

    if (!fileName.isNull())
    {
        QFile file(fileName);

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, tr("Export to CSV failed"), tr("Open file failed: %1").arg(file.errorString()));
            return;
        }

        // UTF-8 BOM
        QByteArray bom;
        bom.append('\xEF');
        bom.append('\xBB');
        bom.append('\xBF');
        // Microsoft Excel may display garbled characters without UTF-8 BOM.
        file.write(bom);

        auto data = minerModel_->getTableCSV().toUtf8();
        file.write(data);

        file.close();

        QMessageBox::information(this, tr("Export to CSV succeeded"), tr("File has saved as %1").arg(fileName));
    }
}

void MainWindow::on_firmwareUpgradeButton_clicked()
{
    if (upgrader_.isRunning()) {
        // stop it
        upgradeMiners(appStatus_ == AppStatus::UPGRADING_SEL);
        return;
    }

    if (minerModel_->getMiners().size() < 1) {
        QMessageBox::information(this, tr("No miner to upgrade"), tr("Please scan miners first."));
        return;
    }

    updateSelectedMinerItems();
    upgradeWindow_.setMinerModels(minerModel_->getMinerModels());
    if (selectedMinerItems_.size() >= 1 && (*selectedMinerItems_.begin())->miner_.fullTypeStr_.empty() == false) {
        upgradeWindow_.setSelectedMinerModel(QString::fromStdString((*selectedMinerItems_.begin())->miner_.fullTypeStr_));
    }
    upgradeWindow_.exec();
}

void MainWindow::on_powerControlEnabled_toggled(bool checked)
{
    ui_->asicBoost->setEnabled(checked);
    ui_->lowPowerMode->setEnabled(checked);
    ui_->economicMode->setEnabled(checked);
}

void MainWindow::on_pool1Enabled_toggled(bool checked)
{
    ui_->pool1Url->setEnabled(checked);
    ui_->pool1Worker->setEnabled(checked);
    ui_->pool1Pwd->setEnabled(checked);
    ui_->pool1PostfixIp->setEnabled(checked);
    ui_->pool1PostfixNoChange->setEnabled(checked);
    ui_->pool1PostfixNone->setEnabled(checked);
}

void MainWindow::on_pool2Enabled_toggled(bool checked)
{
    ui_->pool2Url->setEnabled(checked);
    ui_->pool2Worker->setEnabled(checked);
    ui_->pool2Pwd->setEnabled(checked);
    ui_->pool2PostfixIp->setEnabled(checked);
    ui_->pool2PostfixNoChange->setEnabled(checked);
    ui_->pool2PostfixNone->setEnabled(checked);
}

void MainWindow::on_pool3Enabled_toggled(bool checked)
{
    ui_->pool3Url->setEnabled(checked);
    ui_->pool3Worker->setEnabled(checked);
    ui_->pool3Pwd->setEnabled(checked);
    ui_->pool3PostfixIp->setEnabled(checked);
    ui_->pool3PostfixNoChange->setEnabled(checked);
    ui_->pool3PostfixNone->setEnabled(checked);
}

void MainWindow::on_overclockEnabled_toggled(bool checked)
{
    ui_->overclockModel->setEnabled(checked);
    ui_->overclockWorkingMode->setEnabled(checked);
    ui_->overclockLevelName->setEnabled(checked);
}

void MainWindow::on_overclockModel_currentTextChanged(const QString &model)
{
    syncOverclockWorkingMode(model);
}

void MainWindow::on_overclockWorkingMode_currentTextChanged(const QString &workingMode)
{
    syncOverclockLevelName(ui_->overclockModel->currentText(), workingMode);
}
