#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QtAlgorithms>

#include "upgradewindow.h"
#include "ui_upgradewindow.h"

UpgradeWindow::UpgradeWindow(QWidget *parent) :
    QDialog(parent),
    ui_(new Ui::UpgradeWindow)
{
    ui_->setupUi(this);

    // hide the question button from the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

UpgradeWindow::~UpgradeWindow()
{
    delete ui_;
}

void UpgradeWindow::setMinerModels(const QStringSet &minerModels)
{
    minerModels_ = minerModels;
    syncView();
}

void UpgradeWindow::setSelectedMinerModel(const QString &minerModel)
{
    ui_->minerModelList->setCurrentText(minerModel);
    syncFirmwareList(minerModel);
}

void UpgradeWindow::setFirmwareMap(const FirmwareMap &firmwareMap)
{
    firmwareMap_.clear();

    for (auto itr = firmwareMap.begin(); itr!=firmwareMap.end(); itr++) {
        const QString &model = itr.key();
        const auto &firmwares = itr.value();

        auto &firmwareSet = firmwareMap_[model];

        for (auto file = firmwares.begin(); file!=firmwares.end(); file++) {
            QFileInfo finfo(*file);

            // append only if file exists
            if (finfo.isFile()) {
                firmwareSet.insert(*file);
            }
        }
    }
    syncView();
}

const QStringSet &UpgradeWindow::getMinerModels()
{
    return minerModels_;
}

const FirmwareMap &UpgradeWindow::getFirmwareMap()
{
    return firmwareMap_;
}

QString UpgradeWindow::getSelectedMinerModel()
{
    return ui_->minerModelList->currentText();
}

QString UpgradeWindow::getSelectedFirmware()
{
    return ui_->firmwareList->currentText();
}

bool UpgradeWindow::getKeepSettings()
{
    return ui_->keepSettingsCheck->isChecked();
}

void UpgradeWindow::updateMinerCounter(QString minerModel, size_t selected, size_t selectedDisabled, size_t all, size_t allDisabled)
{
    ui_->upgradeSelectedBtn->setEnabled(selected != 0);
    ui_->upgradeAllBtn->setEnabled(all != 0);

    if (all == 0) {
        ui_->minerNumberLabel->setText(tr("No miners found. Please rescan first."));
    }
    else if (allDisabled == 0) {
        ui_->minerNumberLabel->setText(tr("Selected %1: %2\n     All %1: %3")
                                          .arg(minerModel)
                                          .arg(selected)
                                          .arg(all));
    } else if (allDisabled < all) {
        ui_->minerNumberLabel->setText(tr("Selected %1: %2 (CANNOT upgrade: %4)\n     All %1: %3 (CANNOT upgrade: %5)")
                                          .arg(minerModel)
                                          .arg(selected)
                                          .arg(all)
                                          .arg(selectedDisabled)
                                          .arg(allDisabled));
    } else {
        ui_->minerNumberLabel->setText(tr("Upgrading this model is not yet supported."));
    }
}

void UpgradeWindow::syncView()
{
    QStringList models = minerModels_.toList();
    qSort(models);

    ui_->minerModelList->clear();
    ui_->minerModelList->addItems(models);

    if (minerModels_.size() > 0) {
        ui_->minerModelList->setCurrentIndex(0);
        const QString &model = *models.begin();
        syncFirmwareList(model);
    }
}

void UpgradeWindow::syncFirmwareList(const QString &model)
{
    QStringList firmwares = firmwareMap_[model].toList();
    qSort(firmwares);

    ui_->firmwareList->clear();
    ui_->firmwareList->addItems(firmwares);

    if (firmwares.size() > 0) {
        ui_->firmwareList->setCurrentIndex(0);
    }
}

bool UpgradeWindow::checkInputAndNotice()
{
    if (getSelectedMinerModel().isEmpty()) {
        QMessageBox::information(this, tr("Miner Model is Empty"), tr("Please select a miner model."));
        return false;
    }

    if (getSelectedFirmware().isEmpty()) {
        QMessageBox::information(this, tr("Firmware is Empty"), tr("Please select or add a firmware."));
        return false;
    }

    return true;
}

void UpgradeWindow::on_minerModelList_currentTextChanged(const QString &model)
{
    syncFirmwareList(model);
    emit selectedMinerModelChanged(model);
}

void UpgradeWindow::on_addFirmwareBtn_clicked()
{
    const QString &model = getSelectedMinerModel();
    if (model.isEmpty()) {
        QMessageBox::information(this, tr("Cannot add firmware"), tr("Miner model is empty, please rescan your miners."));
        return;
    }
    auto &firmwares = firmwareMap_[model];

    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Select Firmware"));
    fileDialog.setNameFilter(tr("Firmware (*.bmu *.tar.gz)\nAll (*)"));
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    //fileDialog.setViewMode(QFileDialog::Detail);
    //fileDialog.setDirectory(".");

    auto success = fileDialog.exec();
    if(!success) {
        return;
    }

    for (auto file : fileDialog.selectedFiles()) {
        if (firmwares.find(file) == firmwares.end()) {
            // add only if not in the set
            firmwares.insert(file);
            ui_->firmwareList->addItem(file);
            ui_->firmwareList->setCurrentIndex(firmwares.size() - 1);
        }
        else {
            // or just update the selection
            ui_->firmwareList->setCurrentText(file);
        }
    }
}

void UpgradeWindow::on_delFirmwareBtn_clicked()
{
    QString model = getSelectedMinerModel();
    QString firmware = getSelectedFirmware();

    auto &set = firmwareMap_[model];
    auto index = ui_->firmwareList->currentIndex();

    if (index >= 0) {
        ui_->firmwareList->removeItem(index);
        set.remove(firmware);
    }
}

void UpgradeWindow::on_upgradeSelectedBtn_clicked()
{
    if (checkInputAndNotice()) {
        close();
        emit upgradeSelectedMiners();
    }
}

void UpgradeWindow::on_upgradeAllBtn_clicked()
{
    if (checkInputAndNotice()) {
        close();
        emit upgradeAllMiners();
    }
}
