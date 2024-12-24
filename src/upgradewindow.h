#ifndef UPGRADEWINDOW_H
#define UPGRADEWINDOW_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QMap>
#include <QSet>
#include <QStringList>
#include "utils.h"

namespace Ui {
class UpgradeWindow;
}

class UpgradeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeWindow(QWidget *parent = 0);
    ~UpgradeWindow();

    void setMinerModels(const QStringSet &minerModels);
    void setSelectedMinerModel(const QString &minerModel);
    void setFirmwareMap(const FirmwareMap &firmwareMap);
    const QStringSet& getMinerModels();
    const FirmwareMap& getFirmwareMap();

    QString getSelectedMinerModel();
    QString getSelectedFirmware();
    bool getKeepSettings();

    void updateMinerCounter(QString minerModel, size_t selected, size_t selectedDisabled, size_t all, size_t allDisabled);

signals:
    void selectedMinerModelChanged(QString newMinerModel);
    void upgradeSelectedMiners();
    void upgradeAllMiners();

private slots:
    void on_minerModelList_currentTextChanged(const QString &model);

    void on_addFirmwareBtn_clicked();

    void on_delFirmwareBtn_clicked();

    void on_upgradeSelectedBtn_clicked();

    void on_upgradeAllBtn_clicked();

private:
    void syncView();
    void syncFirmwareList(const QString &model);
    bool checkInputAndNotice();

    Ui::UpgradeWindow *ui_;

    QStringSet minerModels_;
    FirmwareMap firmwareMap_;
};

#endif // UPGRADEWINDOW_H
