#ifndef MINERTABLEMODEL_H
#define MINERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <btctools/miner/common.h>
#include <btctools/utils/IpGenerator.h>
#include <functional>
#include "utils.h"

struct MinerItem
{
    uint32_t ipLong_;
    btctools::miner::Miner miner_;

    MinerItem() = default;
    MinerItem(btctools::miner::Miner miner);
};

class MinerTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum {
        COL_IP,
        COL_STATUS,
        COL_TYPE,
        COL_WORKING_MODE,
        COL_HASHRATE_RT,
        COL_HASHRATE_AVG,
        COL_TEMPERATURE,
        COL_FAN_SPEED,
        COL_ELAPSED,
        COL_POOL1,
        COL_WORKER1,
        COL_POOL2,
        COL_WORKER2,
        COL_POOL3,
        COL_WORKER3,
        COL_FIRMWARE,
        COL_SOFTWARE,
        COL_HARDWARE,
        COL_NETWORK,
        COL_MAC_ADDRESS
    };

    MinerTableModel();
    void addMiner(btctools::miner::Miner miner);
    void clear();
    QList<const MinerItem *> getMiners();
    const MinerItem & getMiner(int row);
    const QString getTableCSV();
    QStringSet getMinerModels();
    QStringSet getOverclockWorkingMode(const QString &minerModel);
    QStringSet getOverclockLevelName(const QString &minerModel, const QString &workingMode);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    // miner number count
    void updateMinerNumCount();
    int allMinerNum();
    int successMinerNum();
    int timeoutMinerNum();
    int okMinerNum();
    int skipMinerNum();
    int rebootedMinerNum();
    int upgradedMinerNum();
    int otherMinerNum();

    // getter & setter of highlight options
    bool isHighlightTemperature() const;
    void setIsHighlightTemperature(bool isHighlightTemperature);

    int highlightTemperatureMoreThan() const;
    void setHighlightTemperatureMoreThan(int highlightTemperatureMoreThan);

    int highlightTemperatureLessThan() const;
    void setHighlightTemperatureLessThan(int highlightTemperatureLessThan);

    bool isHighlightLowHashrate() const;
    void setIsHighlightLowHashrate(bool isHighlightLowHashrate);

    MinerHashrateList highlightLowHashrates() const;
    void setHighlightLowHashrates(const MinerHashrateList &highlightLowHashrates);

    bool isHighlightWrongWorkerName() const;
    void setIsHighlightWrongWorkerName(bool isHighlightWrongWorkerName);

protected:
    QString getMinerOpt(const btctools::miner::Miner &miner, const std::string &key) const;
    void setSort(const QString &sortColumn, bool isDesc = false);
    void setSort(int sortColumn, bool isDesc = false);
    void bindMinerValues(QSqlQuery &query, const MinerItem &minerItem);
    QVariant dataColor(const QModelIndex &index) const;

    void initHighlightOptions();

    // convert value from string to number
    // usage: MinerTableModel::bindMinerValues(), MinerTableModel::setSort()::getMinerSortValue()
    static double hashrateToNum(QString hashrate);
    static int temperatureToNum(QString temperature);
    static int fanSpeedToNum(QString fanSpeed);
    static int elapsedToNum(QString elapsed);
    static QString regularWorkerName(QString workerName);

    bool isWorkerNameWrong(QString workerName, const char *ip) const;
    bool isHashrateTooLow(QString minerType, double hashrate) const;

    // i18n functions
    static QString elapsedI18N(QString elapsed);
    static QString partTranslate(const QString &originText);

private:
    QMap<uint32_t, MinerItem> miners_;
    QSqlDatabase minersDb_;
    QStringList titles_;
    int columnSize_;

    // prepared sql query
    QSqlQuery insertMinerSql_;
    QSqlQuery updateMinerSql_;
    QSqlQuery minerNumCountSql_;

    std::function<uint32_t(int offset)> findMinerIpWithPosition_;
    std::function<double(uint32_t ip)> findMinerHashrate5sWithIp_;
    std::function<double(uint32_t ip)> findMinerHashrateAvgWithIp_;
    std::function<int(uint32_t ip)> findMinerMaxTemperatureWithIp_;
    std::function<int(const MinerItem &minerItem)> findMinerSortPosition_;

    // miner number count
    int successMinerNum_;
    int timeoutMinerNum_;
    int okMinerNum_;
    int skipMinerNum_;
    int rebootedMinerNum_;
    int upgradedMinerNum_;
    int otherMinerNum_;

    // highlight options
    bool isHighlightTemperature_;
    int highlightTemperatureMoreThan_;
    int highlightTemperatureLessThan_;

    bool isHighlightLowHashrate_;
    MinerHashrateList highlightLowHashrates_;

    bool isHighlightWrongWorkerName_;
};

#endif // MINERTABLEMODEL_H
