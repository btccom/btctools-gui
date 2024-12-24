#include <QMessageBox>
#include <QIntValidator>
#include <QDebug>
#include "settingwindow.h"
#include "ui_settingwindow.h"
#include "minertablemodel.h"
#include "minerscanner.h"
#include "passworddelegate.h"
#include "config.h"

SettingWindow::SettingWindow(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::SettingWindow),
    hashrateTableModel_(0, 2),
    minerPasswordModel_(0, 3)
{
    ui_->setupUi(this);

    // hide the question button from the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // init hashrate table
    ui_->highlightTooLowHashrateBox->setModel(&hashrateTableModel_);
    hashrateTableModel_.setHeaderData(0, Qt::Horizontal, tr("Miner Type"));
    hashrateTableModel_.setHeaderData(1, Qt::Horizontal, tr("Min Hash Rate"));

    // init miner password table
    // init hashrate table
    ui_->minerPasswordsBox->setModel(&minerPasswordModel_);
    ui_->minerPasswordsBox->setItemDelegateForColumn(2, new PasswordDelegate(this));
    minerPasswordModel_.setHeaderData(0, Qt::Horizontal, tr("Miner Type"));
    minerPasswordModel_.setHeaderData(1, Qt::Horizontal, tr("User Name"));
    minerPasswordModel_.setHeaderData(2, Qt::Horizontal, tr("Password"));

    ui_->highlightTooLowHashrateBox->setColumnWidth(0, 170);
    ui_->highlightTooLowHashrateBox->setColumnWidth(1, 130);

    // only int to input
    ui_->scanningTimeout->setValidator(new QIntValidator(0, 2147483647, this));
    ui_->configuringTimeout->setValidator(new QIntValidator(0, 2147483647, this));
    ui_->concurrentScanning->setValidator(new QIntValidator(1, 2147483647, this));
    ui_->upgradingTimeout->setValidator(new QIntValidator(0, 2147483647, this));
    ui_->concurrentUpgrading->setValidator(new QIntValidator(1, 2147483647, this));
    ui_->autoRetryTimes->setValidator(new QIntValidator(0, 2147483647, this));
    ui_->highlightTemperatureMoreThan->setValidator(new QIntValidator(-2147483647, 2147483647, this));
    ui_->highlightTemperatureLessThan->setValidator(new QIntValidator(-2147483647, 2147483647, this));
}

SettingWindow::~SettingWindow()
{
    delete ui_;
}

int SettingWindow::monitorInterval() const
{
    return ui_->monitorInterval->text().toInt();
}

void SettingWindow::setMonitorInterval(int interval)
{
    ui_->monitorInterval->setText(QString::number(interval));
}

int SettingWindow::scanningTimeout() const
{
    return ui_->scanningTimeout->text().toInt();
}

void SettingWindow::setScanningTimeout(int timeout)
{
    ui_->scanningTimeout->setText(QString::number(timeout));
}

int SettingWindow::concurrentScanning() const
{
    return ui_->concurrentScanning->text().toInt();
}

void SettingWindow::setConcurrentScanning(int minerNum)
{
    ui_->concurrentScanning->setText(QString::number(minerNum));
}

int SettingWindow::configuringTimeout() const
{
    return ui_->configuringTimeout->text().toInt();
}

void SettingWindow::setConfiguringTimeout(int timeout)
{
    ui_->configuringTimeout->setText(QString::number(timeout));
}

int SettingWindow::concurrentConfiguring() const
{
    return ui_->concurrentConfiguring->text().toInt();
}

void SettingWindow::setConcurrentConfiguring(int minerNum)
{
    ui_->concurrentConfiguring->setText(QString::number(minerNum));
}

int SettingWindow::upgradingTimeout() const
{
    return ui_->upgradingTimeout->text().toInt();
}

void SettingWindow::setUpgradingTimeout(int timeout)
{
    ui_->upgradingTimeout->setText(QString::number(timeout));
}

int SettingWindow::concurrentUpgrading() const
{
    return ui_->concurrentUpgrading->text().toInt();
}

void SettingWindow::setConcurrentUpgrading(int minerNum)
{
    ui_->concurrentUpgrading->setText(QString::number(minerNum));
}

bool SettingWindow::isHighlightTemperature() const
{
    return ui_->isHighlightTemperature->isChecked();
}

void SettingWindow::setIsHighlightTemperature(bool isHighlightTemperature)
{
    ui_->isHighlightTemperature->setChecked(isHighlightTemperature);
}

int SettingWindow::highlightTemperatureMoreThan() const
{
    return ui_->highlightTemperatureMoreThan->text().toInt();
}

void SettingWindow::setHighlightTemperatureMoreThan(int highlightTemperatureMoreThan)
{
    ui_->highlightTemperatureMoreThan->setText(QString::number(highlightTemperatureMoreThan));
}

int SettingWindow::highlightTemperatureLessThan() const
{
    return ui_->highlightTemperatureLessThan->text().toInt();
}

void SettingWindow::setHighlightTemperatureLessThan(int highlightTemperatureLessThan)
{
    ui_->highlightTemperatureLessThan->setText(QString::number(highlightTemperatureLessThan));
}

bool SettingWindow::isHighlightLowHashrate() const
{
    return ui_->isHighlightTooLowHashrate->isChecked();
}

void SettingWindow::setIsHighlightLowHashrate(bool isHighlightLowHashrate)
{
    ui_->isHighlightTooLowHashrate->setChecked(isHighlightLowHashrate);
}

MinerHashrateList SettingWindow::highlightLowHashrates() const
{
    MinerHashrateList list;

    int rowCount = hashrateTableModel_.rowCount();

    for(int row=0; row<rowCount; row++)
    {
        QString minerType = hashrateTableModel_.data(hashrateTableModel_.index(row, 0)).toString().trimmed();
        double hashrate = Utils::unitNumberToDouble(hashrateTableModel_.data(hashrateTableModel_.index(row, 1)).toString());

        if (minerType.isEmpty() || hashrate <= 0)
        {
            continue;
        }

        list.append(MinerHashrate(std::move(minerType), std::move(hashrate)));
    }

    return list;
}

void SettingWindow::setHighlightLowHashrates(const MinerHashrateList &highlightLowHashrates)
{
    hashrateTableModel_.removeRows(0, hashrateTableModel_.rowCount());

    foreach (auto hashrate, highlightLowHashrates)
    {
        addHashrateRow(hashrate.first, Utils::doubleToUnitNumber(hashrate.second));
    }
}

MinerPasswordList SettingWindow::minerPasswords() const
{
    MinerPasswordList list;

    int rowCount = minerPasswordModel_.rowCount();

    for(int row=0; row<rowCount; row++)
    {
        QString minerType = minerPasswordModel_.data(minerPasswordModel_.index(row, 0)).toString().trimmed();
        QString userName = minerPasswordModel_.data(minerPasswordModel_.index(row, 1)).toString();
        QString password = minerPasswordModel_.data(minerPasswordModel_.index(row, 2), Qt::UserRole).toString();

        if (minerType.isEmpty())
        {
            continue;
        }

        list.append(MinerPassword(std::move(minerType), std::move(userName), std::move(password)));
    }

    return list;
}

void SettingWindow::setMinerPasswords(const MinerPasswordList &minerPasswords)
{
    minerPasswordModel_.removeRows(0, minerPasswordModel_.rowCount());

    foreach (auto minerPassword, minerPasswords)
    {
        addMinerPasswordRow(minerPassword.minerType_, minerPassword.userName_, minerPassword.password_);
    }
}

bool SettingWindow::isHighlightWrongWorkerName() const
{
    return ui_->isHighlightWrongWorkerName->isChecked();
}

void SettingWindow::setIsHighlightWrongWorkerName(bool isHighlightWrongWorkerName)
{
    ui_->isHighlightWrongWorkerName->setChecked(isHighlightWrongWorkerName);
}

bool SettingWindow::isOpenMinerCPWithPassword() const
{
    return ui_->openMinerCPWithPassword->isChecked();
}

void SettingWindow::setIsOpenMinerCPWithPassword(bool isOpenMinerCPWithPassword)
{
    ui_->openMinerCPWithPassword->setChecked(isOpenMinerCPWithPassword);
}

int SettingWindow::workerNameIpParts() const
{
    return ui_->workerNameIpParts->value();
}

void SettingWindow::setWorkerNameIpParts(int parts)
{
    ui_->workerNameIpParts->setValue(parts);
}

int SettingWindow::autoRetryTimes() const
{
    return ui_->autoRetryTimes->text().toInt();
}

void SettingWindow::setAutoRetryTimes(int times)
{
    ui_->autoRetryTimes->setText(QString::number(times));
}

void SettingWindow::addHashrateRow(const QString &minerType, const QString &hashrate)
{
    hashrateTableModel_.appendRow(new QStandardItem());

    int row = hashrateTableModel_.rowCount() - 1;

    hashrateTableModel_.setData(hashrateTableModel_.index(row, 0), minerType);
    hashrateTableModel_.setData(hashrateTableModel_.index(row, 1), hashrate);

    hashrateTableModel_.item(row, 0)->setTextAlignment(Qt::AlignCenter);
    hashrateTableModel_.item(row, 1)->setTextAlignment(Qt::AlignCenter);
}

void SettingWindow::addMinerPasswordRow(const QString &minerType, const QString &userName, const QString &password)
{
    minerPasswordModel_.appendRow(new QStandardItem());

    int row = minerPasswordModel_.rowCount() - 1;

    minerPasswordModel_.setData(minerPasswordModel_.index(row, 0), minerType);
    minerPasswordModel_.setData(minerPasswordModel_.index(row, 1), userName);
    minerPasswordModel_.setData(minerPasswordModel_.index(row, 2), password, Qt::UserRole);
    minerPasswordModel_.setData(minerPasswordModel_.index(row, 2), PasswordDelegate::displayText(password), Qt::DisplayRole);

    minerPasswordModel_.item(row, 0)->setTextAlignment(Qt::AlignCenter);
    minerPasswordModel_.item(row, 1)->setTextAlignment(Qt::AlignCenter);
    minerPasswordModel_.item(row, 2)->setTextAlignment(Qt::AlignCenter);
}

void SettingWindow::on_addHashRateButton_clicked()
{
    addHashrateRow();
}

void SettingWindow::on_removeHashRateButton_clicked()
{
    QModelIndex index = ui_->highlightTooLowHashrateBox->currentIndex();

    if (index.row() >= 0)
    {
        hashrateTableModel_.removeRow(index.row());
    }
    else
    {
        QMessageBox::information(this, tr("No Item is Selected"), tr("Please select an item to remove."));
    }
}

void SettingWindow::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button == ui_->buttonBox->button(QDialogButtonBox::RestoreDefaults))
    {
        setMonitorInterval(MONITOR_INTERVAL);

        setScanningTimeout(SCAN_SESSION_TIMEOUT);
        setConcurrentScanning(SCAN_STEP_SIZE);

        setConfiguringTimeout(CONFIG_SESSION_TIMEOUT);
        setConcurrentConfiguring(CONFIG_STEP_SIZE);

        setUpgradingTimeout(UPGRADE_SESSION_TIMEOUT);
        setConcurrentUpgrading(UPGRADE_SEND_FIRMWARE_STEP_SIZE);

        setWorkerNameIpParts(WORKER_NAME_IP_PARTS);
        setAutoRetryTimes(AUTO_RETRY_TIMES);

        setIsHighlightTemperature(IS_HIGHLIGHT_TEMPERATURE);
        setHighlightTemperatureMoreThan(HIGHLIGHT_TEMPERATURE_MORE_THAN);
        setHighlightTemperatureLessThan(HIGHLIGHT_TEMPERATURE_LESS_THAN);
        setIsHighlightLowHashrate(IS_HIGHLIGHT_LOW_HASHRATE);
        setIsHighlightWrongWorkerName(IS_HIGHLIGHT_WRONG_WORKER_NAME);

        setHighlightLowHashrates(HIGHLIGHT_LOW_HASHRATES);
        setMinerPasswords(DEFAULT_MINER_PASSWORDS);

        setIsOpenMinerCPWithPassword(OPEN_MINER_CP_WITH_PASSWORD);
    }
}

void SettingWindow::on_addMinerPassword_clicked()
{
    addMinerPasswordRow();
}

void SettingWindow::on_removeMinerPassword_clicked()
{
    QModelIndex index = ui_->minerPasswordsBox->currentIndex();

    if (index.row() >= 0)
    {
        minerPasswordModel_.removeRow(index.row());
    }
    else
    {
        QMessageBox::information(this, tr("No Item is Selected"), tr("Please select an item to remove."));
    }
}
