#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QVector>
#include <QMenu>
#include <QAction>
#include <QPoint>
#include "settingwindow.h"
#include "upgradewindow.h"
#include "minertablemodel.h"
#include "minerscanner.h"
#include "minerconfigurator.h"
#include "minerrebooter.h"
#include "minerupgrader.h"
#include "autoupdater.h"
#include "iprangelistitem.h"
#include "iprangeedit.h"
#include "utils.h"
#include <btctools/utils/IpGenerator.h>
#include <time.h>

namespace Ui {
class MainWindow;
}

enum class AppStatus {
    NOT_SCAN,

    SCAN_PREPARE,
    SCANNING,
    SCAN_FINISH,

    MONITOR_PREPARE,
    MONITORING,
    MONITOR_FINISH,

    CONFIG_ALL_PREPARE,
    CONFIG_SEL_PREPARE,
    CONFIGURATING_ALL,
    CONFIGURATING_SEL,
    CONFIG_FINISH,

    REBOOT_ALL_PREPARE,
    REBOOT_SEL_PREPARE,
    REBOOTING_ALL,
    REBOOTING_SEL,
    REBOOT_FINISH,

    UPGRADE_ALL_PREPARE,
    UPGRADE_SEL_PREPARE,
    UPGRADING_ALL,
    UPGRADING_SEL,
    UPGRADE_FINISH
};

enum class ButtonStatus {
    PREPARE,
    DISABLED,
    ENABLED,
    STOPABLED
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onReceivedMiner(btctools::miner::Miner miner);
    void onActionBegin();
    void onActionProgress(int percent);
    void onActionEnd();

    void onScanBegin();
    void onScanEnd();
    void onMonitorRefresh();
    void waitMonitorRefresh(int time);
    void onConfigBegin();
    void onConfigEnd(bool allSkip);
    void onRebootBegin();
    void onRebootEnd(bool allSkip);
    void onUpgradeBegin();
    void onUpgradeEnd(bool allSkip);

    void onFindUpdates(QString versionName, QString description, QString downloadUrl, bool forceUpdate);

    // miner table context menu
    void copyMinerItemToClipboard(QModelIndex *index = nullptr);
    void openMinerControlPanel(QModelIndex *index = nullptr);
    void fillPoolsInForm(QModelIndex *index = nullptr);
    void onMinerTableContextMenuClicked(const QPoint &pos);

    // ip range box context menu
    void editIpRange();
    void onIpRangeBoxContextMenuClicked(const QPoint &pos);

    // from UpgradeWindow
    void onUpgradeWindowSelectedMinerModelChanged(QString newMinerModel);
    void onUpgradeSelectedMiners();
    void onUpgradeAllMiners();

    // received an exception
    void onReceivedException(const std::exception &ex);

    // scan miners
    void on_scanMinersButton_clicked();
    void on_monitorMinersButton_clicked();
    // config miners
    void on_configAllMinersButton_clicked();
    void on_configSelectedMinersButton_clicked();
    // reboot miners
    void on_rebootAllMinersButton_clicked();
    void on_rebootSelectedMinersButton_clicked();

    void on_removeIpRange_clicked();
    void on_addIpRange_clicked();
    void on_importLanRange_clicked();
    void on_ipRangeEnableAll_clicked();

    void on_minersTable_doubleClicked(QModelIndex index);
    void on_ipRangeBox_doubleClicked(QModelIndex index);

    void on_settingButton_clicked();
    void on_exportToCSVButton_clicked();
    void on_firmwareUpgradeButton_clicked();

    void on_powerControlEnabled_toggled(bool checked);
    void on_pool1Enabled_toggled(bool checked);
    void on_pool2Enabled_toggled(bool checked);
    void on_pool3Enabled_toggled(bool checked);

    void on_overclockEnabled_toggled(bool checked);

    void on_overclockModel_currentTextChanged(const QString &model);

    void on_overclockWorkingMode_currentTextChanged(const QString &arg1);

protected:
    // check datas
    bool checkPoolsAndNotice();
    bool checkIpRangesAndNotice();
    bool checkOnePoolAndNotice(int id, QLineEdit *poolUrl, QLineEdit *poolWorker);

    void updateSelectedMinerItems();
    void rebootMiners(bool onlySelected);
    void configMiners(bool onlySelected);
    void upgradeMiners(bool onlySelected);

    // update views
    void updateOverclockOption();
    void syncOverclockWorkingMode(const QString &model);
    void syncOverclockLevelName(const QString &model, const QString &workingMode);

    // for upgrade
    void getSelectedMinerNumberWithModel(QString minerModel, size_t &selected, size_t &disabled);
    void getAllMinerNumberWithModel(QString minerModel, size_t &all, size_t &disabled);

    // app status
    void setAppStatus(AppStatus appStatus);
    AppStatus getAppStatus();

    // load & save UI's config file
    void loadUiConfig();
    void saveUiConfig();

    // sync settings with settingWindow_
    void syncOptionsFromSettingWindow();
    void syncOptionsToSettingWindow();

    // add & remove ip range
    void addIpRangeFromString(IpRangeListItem *position = nullptr, QString ipRangeStr = "");
    void addIpRangeWithNewWindow(IpRangeListItem *position = nullptr, bool enabled = true);
    void removeIpRange(IpRangeListItem *ipRange);

    // about IP range box
    btctools::utils::IpGeneratorGroup makeIpRangeGenerators();
    QString getIpRangeGroups();
    void setIpRangeGroups(QString ipRanges);

    // window events
    void closeEvent(QCloseEvent *event);

    // count miners
    QString getMinerCountStr(bool onlySelected);

    // context menu
    void initContextMenu();

private:
    Ui::MainWindow *ui_;
    SettingWindow settingWindow_;
    UpgradeWindow upgradeWindow_;

    MinerTableModel *minerModel_;
    MinerPasswordList minerPasswordList_;

    MinerScanner scanner_;
    MinerConfigurator configurator_;
    MinerRebooter rebooter_;
    MinerUpgrader upgrader_;

    AutoUpdater autoUpdater_;
    bool onlySuccessMiners_;
    QString actionProgressTextFormat_;
    QList<const MinerItem *> selectedMinerItems_;

    AppStatus appStatus_;
    bool monitorRunning_;
    time_t lastRefreshTime_;

    // miner table context menu
    QMenu *minerTableContextMenu_;
    QAction *copyAction_;
    QAction *openMinerCP_;
    QAction *fillPoolsInForm_;

    bool openMinerCPWithPassword_;

    // IP range box context menu
    QMenu *ipRangeBoxContextMenu_;
    QAction *editAction_;
};

#endif // MAINWINDOW_H
