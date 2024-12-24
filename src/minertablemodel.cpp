#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegExp>
#include <QColor>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <iostream>
#include "minertablemodel.h"
#include "config.h"

MinerItem::MinerItem(btctools::miner::Miner miner) :
    ipLong_(btctools::utils::IpGenerator::ip2long(miner.ip_)),
    miner_(std::move(miner))
{
    // no more code
}

MinerTableModel::MinerTableModel()
{
    initHighlightOptions();

    //******* table's title *******
    titles_ << tr("IP")
            << tr("Status")
            << tr("Type")
            << tr("Working Mode")
            << tr("Hash Rate RT")
            << tr("Hash Rate avg")
            << tr("Temperature")
            << tr("Fan Speed")
            << tr("Elapsed")
            << tr("Pool 1")
            << tr("Worker 1")
            << tr("Pool 2")
            << tr("Worker 2")
            << tr("Pool 3")
            << tr("Worker 3")
            << tr("Firmware")
            << tr("Software")
            << tr("Hardware")
            << tr("Network")
            << tr("MAC Address");

    columnSize_ = titles_.size();

    //******* create db & table *******
    minersDb_ = QSqlDatabase::addDatabase("QSQLITE");
    minersDb_.setDatabaseName(":memory:");
    minersDb_.open();

    QSqlQuery query(minersDb_);

    bool success = query.exec(
        "CREATE TABLE miner_sort("
            "ip INT primary key not null,"
            "status TEXT not null,"
            "type TEXT not null,"
            "working_mode TEXT not null,"
            "hashrate_5s REAL not null,"
            "hashrate_avg REAL not null,"
            "max_temperature INT not null,"
            "avg_fan_speed INT not null,"
            "elapsed INT not null,"
            "pool1 TEXT not null,"
            "worker1 TEXT not null,"
            "pool2 TEXT not null,"
            "worker2 TEXT not null,"
            "pool3 TEXT not null,"
            "worker3 TEXT not null,"
            "firmware TEXT not null,"
            "software TEXT not null,"
            "hardware TEXT not null,"
            "network TEXT not null,"
            "macaddr TEXT not null"
        ")");

    if (!success)
    {
        throw QString("MinerTableModel: create table `miner_sort` failed! ") + query.lastError().text();
    }

    //******* create index *******
    success =
        query.exec("CREATE INDEX mrst_stat ON miner_sort(status)") &&
        query.exec("CREATE INDEX mrst_type ON miner_sort(type)") &&
        query.exec("CREATE INDEX mrst_wkmd ON miner_sort(working_mode)") &&
        query.exec("CREATE INDEX mrst_hs5s ON miner_sort(hashrate_5s)") &&
        query.exec("CREATE INDEX mrst_hsvg ON miner_sort(hashrate_avg)") &&
        query.exec("CREATE INDEX mrst_temp ON miner_sort(max_temperature)") &&
        query.exec("CREATE INDEX mrst_fspd ON miner_sort(avg_fan_speed)") &&
        query.exec("CREATE INDEX mrst_elps ON miner_sort(elapsed)") &&
        query.exec("CREATE INDEX mrst_pol1 ON miner_sort(pool1)") &&
        query.exec("CREATE INDEX mrst_wrk1 ON miner_sort(worker1)") &&
        query.exec("CREATE INDEX mrst_pol2 ON miner_sort(pool2)") &&
        query.exec("CREATE INDEX mrst_wrk2 ON miner_sort(worker2)") &&
        query.exec("CREATE INDEX mrst_pol3 ON miner_sort(pool3)") &&
        query.exec("CREATE INDEX mrst_wrk3 ON miner_sort(worker3)") &&
        query.exec("CREATE INDEX mrst_firm ON miner_sort(firmware)") &&
        query.exec("CREATE INDEX mrst_soft ON miner_sort(software)") &&
        query.exec("CREATE INDEX mrst_hard ON miner_sort(hardware)") &&
        query.exec("CREATE INDEX mrst_netw ON miner_sort(network)") &&
        query.exec("CREATE INDEX mrst_macr ON miner_sort(macaddr)");

    if (!success)
    {
        throw QString("MinerTableModel: create index on `miner_sort` failed! ") + query.lastError().text();
    }

    //******* prepare sqls *******
    // findMinerHashrate5sWithIp_
    QSqlQuery selectQuery = QSqlQuery(minersDb_);
    success = selectQuery.prepare("SELECT hashrate_5s FROM miner_sort WHERE ip=:ip");

    if (!success)
    {
        throw QString("prepare select miner sql failed! ") + selectQuery.lastError().text();
    }

    findMinerHashrate5sWithIp_ = [selectQuery](uint32_t ip) mutable -> double
    {
        selectQuery.bindValue(":ip", ip);
        bool success = selectQuery.exec();

        if (!success || !selectQuery.first())
        {
            throw QString("MinerTableModel: select from miner_sort failed! ") + selectQuery.lastError().text();
        }

        return selectQuery.value(0).toDouble();
    };

    // findMinerHashrateAvgWithIp_
    selectQuery = QSqlQuery(minersDb_);
    success = selectQuery.prepare("SELECT hashrate_avg FROM miner_sort WHERE ip=:ip");

    if (!success)
    {
        throw QString("prepare select miner sql failed! ") + selectQuery.lastError().text();
    }

    findMinerHashrateAvgWithIp_ = [selectQuery](uint32_t ip) mutable -> double
    {
        selectQuery.bindValue(":ip", ip);
        bool success = selectQuery.exec();

        if (!success || !selectQuery.first())
        {
            throw QString("MinerTableModel: select from miner_sort failed! ") + selectQuery.lastError().text();
        }

        return selectQuery.value(0).toDouble();
    };

    // findMinerMaxTemperatureWithIp_
    selectQuery = QSqlQuery(minersDb_);
    success = selectQuery.prepare("SELECT max_temperature FROM miner_sort WHERE ip=:ip");

    if (!success)
    {
        throw QString("prepare select miner sql failed! ") + selectQuery.lastError().text();
    }

    findMinerMaxTemperatureWithIp_ = [selectQuery](uint32_t ip) mutable -> int
    {
        selectQuery.bindValue(":ip", ip);
        bool success = selectQuery.exec();

        if (!success || !selectQuery.first())
        {
            throw QString("MinerTableModel: select from miner_sort failed! ") + selectQuery.lastError().text();
        }

        return selectQuery.value(0).toInt();
    };

    // insert
    insertMinerSql_ = QSqlQuery(minersDb_);
    success = insertMinerSql_.prepare(
        "INSERT INTO miner_sort("
            "ip, status, type, working_mode, hashrate_5s, hashrate_avg, "
            "max_temperature, avg_fan_speed, elapsed, pool1, worker1, "
            "pool2, worker2, pool3, worker3, "
            "firmware, software, hardware, network, macaddr"
        ") "
        "VALUES("
            ":ip, :status, :type, :working_mode, :hashrate_5s, :hashrate_avg, "
            ":max_temperature, :avg_fan_speed, :elapsed, :pool1, :worker1, "
            ":pool2, :worker2, :pool3, :worker3, "
            ":firmware, :software, :hardware, :network, :macaddr"
        ")");

    if (!success)
    {
        throw QString("MinerTableModel: prepare insert miner sql failed! ") + insertMinerSql_.lastError().text();
    }

    // update
    updateMinerSql_ = QSqlQuery(minersDb_);
    success = updateMinerSql_.prepare(
        "UPDATE miner_sort "
        "SET "
            "status=:status, "
            "type=:type, "
            "working_mode=:working_mode, "
            "hashrate_5s=:hashrate_5s, "
            "hashrate_avg=:hashrate_avg, "
            "max_temperature=:max_temperature, "
            "avg_fan_speed=:avg_fan_speed, "
            "elapsed=:elapsed, "
            "pool1=:pool1, "
            "worker1=:worker1, "
            "pool2=:pool2, "
            "worker2=:worker2, "
            "pool3=:pool3, "
            "worker3=:worker3, "
            "firmware=:firmware, "
            "software=:software, "
            "hardware=:hardware, "
            "network=:network, "
            "macaddr=:macaddr "
        "WHERE "
            "ip=:ip");

    if (!success)
    {
        throw QString("MinerTableModel: prepare update miner sql failed! ") + updateMinerSql_.lastError().text();
    }

    // number count
    minerNumCountSql_ = QSqlQuery(minersDb_);
    success = minerNumCountSql_.prepare("SELECT status,count(*) FROM miner_sort GROUP BY status");

    if (!success)
    {
        throw QString("MinerTableModel: prepare number count sql failed! ") + minerNumCountSql_.lastError().text();
    }

    //******* init sort *******
    setSort("ip");
}

double MinerTableModel::hashrateToNum(QString hashrate)
{
    double rate = 0;

    hashrate = hashrate.trimmed();

    if (hashrate.isEmpty())
    {
        return 0;
    }

    int pos = hashrate.indexOf(' ');

    if (pos > 0 && pos + 1 < hashrate.size())
    {
        rate = hashrate.left(pos).toDouble();
        char unit = hashrate[pos + 1].toLatin1();

        switch (unit)
        {
        case 'k':
            rate *= 1.0e3;
            break;
        case 'M':
            rate *= 1.0e6;
            break;
        case 'G':
            rate *= 1.0e9;
            break;
        case 'T':
            rate *= 1.0e12;
            break;
        case 'P':
            rate *= 1.0e15;
            break;
        case 'E':
            rate *= 1.0e18;
            break;
        case 'Z':
            rate *= 1.0e21;
            break;
        case 'Y':
            rate *= 1.0e24;
            break;
        }
    }
    else
    {
        rate = hashrate.toDouble();
    }

    return rate;
}

int MinerTableModel::temperatureToNum(QString temperature)
{
    int maxTemp = 0;

    temperature = temperature.trimmed();

    if (temperature.isEmpty())
    {
        return 0;
    }

    QStringList tempStrList = temperature.split(" / ");

    foreach (QString tempStr, tempStrList)
    {
        int tempInt = tempStr.toInt();

        if (tempInt > maxTemp)
        {
            maxTemp = tempInt;
        }
    }

    return maxTemp;
}

int MinerTableModel::fanSpeedToNum(QString fanSpeed)
{
    int count = 0;
    int sum = 0;

    fanSpeed = fanSpeed.trimmed();

    if (fanSpeed.isEmpty())
    {
        return 0;
    }

    QStringList strList = fanSpeed.split(" / ");

    foreach (QString str, strList)
    {
        sum += str.toInt();
        count++;
    }

    return count > 0 ? (sum / count) : 0;
}

int MinerTableModel::elapsedToNum(QString elapsed)
{
    int seconds = 0;

    elapsed = elapsed.trimmed();

    if (elapsed.isEmpty())
    {
        return 0;
    }

    QStringList strList = elapsed.split(" ");

    foreach (QString str, strList)
    {
        char unit = str.at(str.size() - 1).toLatin1(); // get the last char
        str = str.left(str.size() - 1); // strip the last char
        int strInt = str.toInt();

        switch (unit)
        {
        case 'd': // days
            strInt *= 86400;
            break;
        case 'h': // hours
            strInt *= 3600;
            break;
        case 'm': // minutes
            strInt *= 60;
            break;
        case 's': // seconds
            // strInt *= 1;
            break;
        }

        seconds += strInt;
    }

    return seconds;
}

QString MinerTableModel::regularWorkerName(QString workerName)
{
    if (workerName.isEmpty())
    {
        return workerName;
    }

    QRegExp workerPostfixExp("^.*\\.[0-9]+x[0-9]+$");

    if (!workerPostfixExp.exactMatch(workerName))
    {
        return workerName;
    }

    int dotPos = workerName.lastIndexOf(".");
    QString postfix = workerName.right(workerName.size() - dotPos - 1);
    workerName = workerName.left(dotPos + 1);

    QStringList postfixIpList = postfix.split("x");

    for (auto it=postfixIpList.begin(); it!=postfixIpList.end(); it++)
    {
        *it = (*it).rightJustified(3, '0');
    }

    postfix = postfixIpList.join("x");

    return workerName + postfix;
}

bool MinerTableModel::isWorkerNameWrong(QString workerName, const char *ip) const
{
    if (workerName.isEmpty())
    {
        return false;
    }

    QRegExp workerPostfixExp("^.*\\.[0-9]+x[0-9]+$");

    // Only check the worker name that likes the format of "username.123x45".
    if (workerPostfixExp.exactMatch(workerName))
    {
        struct in_addr_btctools ipAddr;
        ipAddr.S_un.S_addr = inet_addr(ip);

        // the buffer is what the worker's name should be.
        char buffer[8];

        sprintf(buffer, "%03hhux%03hhu", ipAddr.S_un.S_un_b.s_b3, ipAddr.S_un.S_un_b.s_b4);

        workerName = regularWorkerName(workerName);

        // strip the prefix (sub-account name).
        workerName = workerName.right(workerName.size() - workerName.lastIndexOf('.') - 1);

        // worker name is different with its IP.
        if (workerName != buffer)
        {
            return true;
        }
    }

    return false;
}

bool MinerTableModel::isHashrateTooLow(QString minerType, double hashrate) const
{
    foreach (MinerHashrate minRate, highlightLowHashrates_)
    {
        if (minerType.toLower().contains(minRate.first.toLower()))
        {
            if (hashrate < minRate.second)
            {
                return true;
            }
            else
            {
                // Once it matches a miner type, it will no longer to match others.
                // It can solve some miner type is the prefix of the other miner type,
                // likes "Antminer L3+" and "Antminer L3".
                // Now, put "Antminer L3+" before "Antminer L3", and we will get the right result.
                return false;
            }
        }
    }

    return false;
}

void MinerTableModel::initHighlightOptions()
{
    isHighlightTemperature_ = IS_HIGHLIGHT_TEMPERATURE;
    highlightTemperatureMoreThan_ = HIGHLIGHT_TEMPERATURE_MORE_THAN;
    highlightTemperatureLessThan_ = HIGHLIGHT_TEMPERATURE_LESS_THAN;

    isHighlightLowHashrate_ = IS_HIGHLIGHT_LOW_HASHRATE;
    highlightLowHashrates_ = HIGHLIGHT_LOW_HASHRATES;

    isHighlightWrongWorkerName_ = IS_HIGHLIGHT_WRONG_WORKER_NAME;

}

QString MinerTableModel::elapsedI18N(QString elapsed)
{
    elapsed = elapsed.replace("d", tr("d", "days"));
    elapsed = elapsed.replace("h", tr("h", "hours"));
    elapsed = elapsed.replace("m", tr("m", "minutes"));
    elapsed = elapsed.replace("s", tr("s", "seconds"));

    return elapsed;
}

QString MinerTableModel::partTranslate(const QString &originText)
{
    QString translatedText;
    int begin = 0;
    int end = 0;

    for (; end < originText.size(); end++) {
        if (originText[end] == ',' ||
            originText[end] == ':' ||
            originText[end] == ';') {
            if (begin < end) {
                translatedText += tr(originText.midRef(begin, end-begin).toUtf8().data());
            }
            translatedText += originText[end];
            begin = end + 1;
        } else if (originText[end].isSpace()) {
            if (begin == end) {
                translatedText += originText[end];
                begin = end + 1;
            }
        }
    }

    if (begin < end) {
        translatedText += tr(originText.rightRef(end - begin + 1).trimmed().toUtf8().data());
    }

    return translatedText;
}

bool MinerTableModel::isHighlightWrongWorkerName() const
{
    return isHighlightWrongWorkerName_;
}

void MinerTableModel::setIsHighlightWrongWorkerName(bool isHighlightWrongWorkerName)
{
    isHighlightWrongWorkerName_ = isHighlightWrongWorkerName;
}

MinerHashrateList MinerTableModel::highlightLowHashrates() const
{
    return highlightLowHashrates_;
}

void MinerTableModel::setHighlightLowHashrates(const MinerHashrateList &highlightLowHashrates)
{
    highlightLowHashrates_ = highlightLowHashrates;
}

bool MinerTableModel::isHighlightLowHashrate() const
{
    return isHighlightLowHashrate_;
}

void MinerTableModel::setIsHighlightLowHashrate(bool isHighlightLowHashrate)
{
    isHighlightLowHashrate_ = isHighlightLowHashrate;
}

int MinerTableModel::highlightTemperatureLessThan() const
{
    return highlightTemperatureLessThan_;
}

void MinerTableModel::setHighlightTemperatureLessThan(int highlightTemperatureLessThan)
{
    highlightTemperatureLessThan_ = highlightTemperatureLessThan;
}

int MinerTableModel::highlightTemperatureMoreThan() const
{
    return highlightTemperatureMoreThan_;
}

void MinerTableModel::setHighlightTemperatureMoreThan(int highlightTemperatureMoreThan)
{
    highlightTemperatureMoreThan_ = highlightTemperatureMoreThan;
}

bool MinerTableModel::isHighlightTemperature() const
{
    return isHighlightTemperature_;
}

void MinerTableModel::setIsHighlightTemperature(bool isHighlightTemperature)
{
    isHighlightTemperature_ = isHighlightTemperature;
}

void MinerTableModel::addMiner(btctools::miner::Miner miner)
{
    MinerItem item = MinerItem(std::move(miner));
    bool isUpdate = miners_.contains(item.ipLong_);
    int position = findMinerSortPosition_(item);

    // insert or update `miner_sort`
    bool success = false;

    if (isUpdate)
    {
        bindMinerValues(updateMinerSql_, item);
        success = updateMinerSql_.exec();

        if (!success)
        {
            throw QString("MinerTableModel: update `miner_sort` failed! ") + updateMinerSql_.lastError().text();
        }

        int newPosition = findMinerSortPosition_(miners_.value(item.ipLong_));

        int rowBegin = qMin(position, newPosition);
        int rowEnd = qMax(position, newPosition);

        miners_.insert(item.ipLong_, item);
        emit dataChanged(index(rowBegin, 0), index(rowEnd, columnSize_ - 1));
    }
    else
    {
        beginInsertRows(QModelIndex(), position, position);
        bindMinerValues(insertMinerSql_, item);
        success = insertMinerSql_.exec();

        if (!success)
        {
            throw QString("MinerTableModel: insert `miner_sort` failed! ") + insertMinerSql_.lastError().text();
        }

        miners_.insert(item.ipLong_, item);
        endInsertRows();
    }
}

void MinerTableModel::setSort(int sortColumn, bool isDesc)
{
    const char *sortColumns[] = {
        "ip",
        "status",
        "type",
        "working_mode",
        "hashrate_5s",
        "hashrate_avg",
        "max_temperature",
        "avg_fan_speed",
        "elapsed",
        "pool1",
        "worker1",
        "pool2",
        "worker2",
        "pool3",
        "worker3",
        "firmware",
        "software",
        "hardware",
        "network",
        "macaddr"
    };

    setSort(sortColumns[sortColumn], isDesc);
}

void MinerTableModel::setSort(const QString &sortColumn, bool isDesc)
{
    QString sql = QString("SELECT ip FROM miner_sort ORDER BY %1 %2 LIMIT :offset, 1").arg(sortColumn).arg(isDesc ? "DESC" : "");
    QSqlQuery selectMinerIpQuery(minersDb_);
    bool success = selectMinerIpQuery.prepare(sql);

    if (!success)
    {
        throw QString("prepare select miner sql failed! ") + selectMinerIpQuery.lastError().text();
    }

    findMinerIpWithPosition_ = [selectMinerIpQuery](int offset) mutable -> uint32_t
    {
        selectMinerIpQuery.bindValue(":offset", offset);
        bool success = selectMinerIpQuery.exec();

        if (!success || !selectMinerIpQuery.first())
        {
            throw QString("MinerTableModel: select ip from miner_sort failed! ") + selectMinerIpQuery.lastError().text();
        }

        return selectMinerIpQuery.value(0).toUInt();
    };


    //******* init findMinerSortPosition_ *******

    sql = QString("SELECT count(*) FROM miner_sort WHERE %1 %2 :value").arg(sortColumn).arg(isDesc ? ">" : "<");

    QSqlQuery findSortPositionQuery(minersDb_);
    success = findSortPositionQuery.prepare(sql);

    if (!success)
    {
        throw QString("prepare select miner sql failed! ") + findSortPositionQuery.lastError().text();
    }

    std::function<QVariant(const MinerItem &minerItem)> getMinerSortValue;

    //---------- Is there a simpler way? ----------
    if (sortColumn == "ip")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.ipLong_; };
    else if (sortColumn == "status")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.stat_.c_str(); };
    else if (sortColumn == "type")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.fullTypeStr_.c_str(); };
    else if (sortColumn == "working_mode")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.opt("antminer.overclock_working_mode").c_str(); };
    else if (sortColumn == "hashrate_5s")
        getMinerSortValue = [](const MinerItem &minerItem) { return hashrateToNum(minerItem.miner_.opt("hashrate_5s").c_str()); };
    else if (sortColumn == "hashrate_avg")
        getMinerSortValue = [](const MinerItem &minerItem) { return hashrateToNum(minerItem.miner_.opt("hashrate_avg").c_str()); };
    else if (sortColumn == "max_temperature")
        getMinerSortValue = [](const MinerItem &minerItem) { return temperatureToNum(minerItem.miner_.opt("temperature").c_str()); };
    else if (sortColumn == "avg_fan_speed")
        getMinerSortValue = [](const MinerItem &minerItem) { return fanSpeedToNum(minerItem.miner_.opt("fan_speed").c_str()); };
    else if (sortColumn == "elapsed")
        getMinerSortValue = [](const MinerItem &minerItem) { return elapsedToNum(minerItem.miner_.opt("elapsed").c_str()); };
    else if (sortColumn == "pool1")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.pool1_.url_.c_str(); };
    else if (sortColumn == "worker1")
        getMinerSortValue = [](const MinerItem &minerItem) { return regularWorkerName(minerItem.miner_.pool1_.worker_.c_str()); };
    else if (sortColumn == "pool2")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.pool2_.url_.c_str(); };
    else if (sortColumn == "worker2")
        getMinerSortValue = [](const MinerItem &minerItem) { return regularWorkerName(minerItem.miner_.pool2_.worker_.c_str()); };
    else if (sortColumn == "pool3")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.pool3_.url_.c_str(); };
    else if (sortColumn == "worker3")
        getMinerSortValue = [](const MinerItem &minerItem) { return regularWorkerName(minerItem.miner_.pool3_.worker_.c_str()); };
    else if (sortColumn == "firmware")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.opt("firmware_version").c_str(); };
    else if (sortColumn == "software")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.opt("software_version").c_str(); };
    else if (sortColumn == "hardware")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.opt("hardware_version").c_str(); };
    else if (sortColumn == "network")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.opt("network_type").c_str(); };
    else if (sortColumn == "macaddr")
        getMinerSortValue = [](const MinerItem &minerItem) { return minerItem.miner_.opt("mac_address").c_str(); };
    //---------- end of these getMinerSortValue ----------

    findMinerSortPosition_ = [getMinerSortValue, findSortPositionQuery](const MinerItem &minerItem) mutable -> int
    {
        findSortPositionQuery.bindValue(":value", getMinerSortValue(minerItem));
        bool success = findSortPositionQuery.exec();

        if (!success || !findSortPositionQuery.first())
        {
            throw QString("MinerTableModel: find sort position from miner_sort failed! ") + findSortPositionQuery.lastError().text();
        }

        return findSortPositionQuery.value(0).toInt();
    };
}

void MinerTableModel::bindMinerValues(QSqlQuery &query, const MinerItem &minerItem)
{
    query.bindValue(":ip", minerItem.ipLong_);
    query.bindValue(":status", minerItem.miner_.stat_.c_str());
    query.bindValue(":type", minerItem.miner_.fullTypeStr_.c_str());
    query.bindValue(":working_mode", minerItem.miner_.opt("antminer.overclock_working_mode").c_str());
    query.bindValue(":hashrate_5s", hashrateToNum(minerItem.miner_.opt("hashrate_5s").c_str()));
    query.bindValue(":hashrate_avg", hashrateToNum(minerItem.miner_.opt("hashrate_avg").c_str()));
    query.bindValue(":max_temperature", temperatureToNum(minerItem.miner_.opt("temperature").c_str()));
    query.bindValue(":avg_fan_speed", fanSpeedToNum(minerItem.miner_.opt("fan_speed").c_str()));
    query.bindValue(":elapsed", elapsedToNum(minerItem.miner_.opt("elapsed").c_str()));
    query.bindValue(":pool1", minerItem.miner_.pool1_.url_.c_str());
    query.bindValue(":worker1", regularWorkerName(minerItem.miner_.pool1_.worker_.c_str()));
    query.bindValue(":pool2", minerItem.miner_.pool2_.url_.c_str());
    query.bindValue(":worker2", regularWorkerName(minerItem.miner_.pool2_.worker_.c_str()));
    query.bindValue(":pool3", minerItem.miner_.pool3_.url_.c_str());
    query.bindValue(":worker3", regularWorkerName(minerItem.miner_.pool3_.worker_.c_str()));
    query.bindValue(":firmware", minerItem.miner_.opt("firmware_version").c_str());
    query.bindValue(":software", minerItem.miner_.opt("software_version").c_str());
    query.bindValue(":hardware", minerItem.miner_.opt("hardware_version").c_str());
    query.bindValue(":network", minerItem.miner_.opt("network_type").c_str());
    query.bindValue(":macaddr", minerItem.miner_.opt("mac_address").c_str());
}

void MinerTableModel::clear()
{
    beginResetModel();

    miners_.clear();

    QSqlQuery query(minersDb_);
    bool success = query.exec(QString("DELETE FROM miner_sort"));

    if (!success)
    {
        throw QString("MinerTableModel: DELETE FROM `miner_sort` failed! ") + query.lastError().text();
    }

    endResetModel();
}

QList<const MinerItem *> MinerTableModel::getMiners()
{
    QList<const MinerItem *> minersVector;
    int size = miners_.size();

    for (int i=0; i<size; i++)
    {
        minersVector.append(&getMiner(i));
    }

    return minersVector;
}

const MinerItem & MinerTableModel::getMiner(int row)
{
    return miners_[findMinerIpWithPosition_(row)];
}

const QString MinerTableModel::getTableCSV()
{
    int maxLine = rowCount(this->index(0, 0));
    int maxColumn = columnCount(this->index(0, 0));
    QStringList lines;

    // get header
    QStringList header;
    for (int c=0; c<maxColumn; c++) {
        QString data = this->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString();

        data = QString("\"%1\"").arg(data.replace("\"", "\"\""));
        header.append(data);
    }
    lines.append(header.join(','));

    // get content
    for (int l=0; l<maxLine; l++) {
        QStringList line;

        for (int c=0; c<maxColumn; c++) {
            auto index = this->index(l, c);
            QString data = this->data(index).toString();

            data = QString("\"%1\"").arg(data.replace("\"", "\"\""));
            line.append(data);
        }

        lines.append(line.join(','));
    }

    return lines.join('\n');
}

QStringSet MinerTableModel::getMinerModels()
{
    QStringSet minerModels;
    for (const auto &item : miners_) {
        if (!item.miner_.fullTypeStr_.empty())
        minerModels.insert(QString::fromStdString(item.miner_.fullTypeStr_));
    }
    return minerModels;
}

QStringSet MinerTableModel::getOverclockWorkingMode(const QString &minerModel)
{
    QStringSet workingModes;

    for (const auto &item : miners_) {
        if (minerModel != item.miner_.fullTypeStr_.c_str()) {
            continue;
        }

        QString modeJson = item.miner_.opt("antminer.overclock_option").c_str();
        if (modeJson.isEmpty()) {
            continue;
        }

        auto doc = QJsonDocument::fromJson(modeJson.toUtf8());
        if (!doc.isObject()) {
            continue;
        }

        auto modeInfo = doc.object()["ModeInfo"];
        if (!modeInfo.isArray()) {
            continue;
        }

        for (auto mode : modeInfo.toArray()) {
            if (!mode.isObject()) {
                continue;
            }
            auto modeName = mode.toObject()["ModeName"];
            if (!modeName.isString()) {
                continue;
            }
            workingModes.insert(modeName.toString());
        }
    }

    return workingModes;
}

QStringSet MinerTableModel::getOverclockLevelName(const QString &minerModel, const QString &workingMode) {
    QStringSet LevelNames;

    for (const auto &item : miners_) {
        if (minerModel != item.miner_.fullTypeStr_.c_str()) {
            continue;
        }

        QString modeJson = item.miner_.opt("antminer.overclock_option").c_str();
        if (modeJson.isEmpty()) {
            continue;
        }

        auto doc = QJsonDocument::fromJson(modeJson.toUtf8());
        if (!doc.isObject()) {
            continue;
        }

        auto modeInfo = doc.object()["ModeInfo"];
        if (!modeInfo.isArray()) {
            continue;
        }

        for (auto mode : modeInfo.toArray()) {
            if (!mode.isObject()) {
                continue;
            }
            auto modeName = mode.toObject()["ModeName"];
            if (!modeName.isString()) {
                continue;
            }
            if (modeName.toString() != workingMode) {
                continue;
            }
            auto level = mode.toObject()["Level"];
            if (!level.isObject()) {
                continue;
            }
            LevelNames += level.toObject().keys().toSet();
        }
    }

    return LevelNames;
}

int MinerTableModel::rowCount(const QModelIndex & /* parent */) const
{
    return miners_.size();
}

int MinerTableModel::columnCount(const QModelIndex & /* parent */) const
{
    return columnSize_;
}

QVariant MinerTableModel::dataColor(const QModelIndex &index) const
{
    int col = index.column();
    int row = index.row();

    if (col != COL_HASHRATE_RT && col != COL_HASHRATE_AVG && col != COL_TEMPERATURE &&
        col != COL_WORKER1 && col != COL_WORKER2 && col != COL_WORKER3)
    {
        return QVariant();
    }

    uint32_t ipLong = findMinerIpWithPosition_(row);
    const btctools::miner::Miner &miner = miners_.value(ipLong).miner_;

    switch (col)
    {
    case COL_HASHRATE_RT: // hashrate_5s
    {
        if (!isHighlightLowHashrate_)
        {
            return QVariant();
        }

        double hashrate = findMinerHashrate5sWithIp_(ipLong);

        if (isHashrateTooLow(miner.fullTypeStr_.c_str(), hashrate))
        {
            return QColor(Qt::red);
        }

        break;
    }

    case COL_HASHRATE_AVG: // hashrate_avg
    {
        if (!isHighlightLowHashrate_)
        {
            return QVariant();
        }

        double hashrate = findMinerHashrateAvgWithIp_(ipLong);

        if (isHashrateTooLow(miner.fullTypeStr_.c_str(), hashrate))
        {
            return QColor(Qt::red);
        }

        break;
    }

    case COL_TEMPERATURE: // max_temperature
    {
        if (!isHighlightTemperature_)
        {
            return QVariant();
        }

        QString temperature = QString(miner.opt("temperature").c_str()).trimmed();

        if (temperature.isEmpty())
        {
            return QVariant();
        }

        QStringList tempStrList = temperature.split(" / ");

        foreach (QString tempStr, tempStrList)
        {
            int tempInt = tempStr.toInt();

            // The miner will stop if temperature over 90℃.
            // And a negative temperature means the miner has some problems.
            if (tempInt > highlightTemperatureMoreThan_ || tempInt < highlightTemperatureLessThan_)
            {
                return QColor(Qt::red);
            }
        }

        break;
    }

    case COL_WORKER1: // worker1
    {
        if (!isHighlightWrongWorkerName_)
        {
            return QVariant();
        }

        if (isWorkerNameWrong(miner.pool1_.worker_.c_str(), miner.ip_.c_str()))
        {
            return QColor(Qt::red);
        }

        break;
    }

    case COL_WORKER2: // worker2
    {
        if (!isHighlightWrongWorkerName_)
        {
            return QVariant();
        }

        if (isWorkerNameWrong(miner.pool2_.worker_.c_str(), miner.ip_.c_str()))
        {
            return QColor(Qt::red);
        }

        break;
    }

    case COL_WORKER3: // worker3
    {
        if (!isHighlightWrongWorkerName_)
        {
            return QVariant();
        }

        if (isWorkerNameWrong(miner.pool3_.worker_.c_str(), miner.ip_.c_str()))
        {
            return QColor(Qt::red);
        }

        break;
    }

    default: // unknown
        break;
    }

    // default color
    return QVariant();
}

QVariant MinerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        return int(Qt::AlignCenter | Qt::AlignVCenter);
    }
    else if (role == Qt::ForegroundRole) {
        return dataColor(index);
    }
    else if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    int col = index.column();
    int row = index.row();

    uint32_t ipLong = findMinerIpWithPosition_(row);
    const btctools::miner::Miner &miner = miners_.value(ipLong).miner_;

    switch (col)
    {
    case COL_IP:
        return QString(miner.ip_.c_str());
    case COL_STATUS:
        return partTranslate(miner.stat_.c_str());
    case COL_TYPE:
        return QString(miner.fullTypeStr_.c_str());
    case COL_WORKING_MODE:
        return partTranslate(miner.opt("antminer.overclock_working_mode").c_str());

    case COL_HASHRATE_RT:
        return QString(miner.opt("hashrate_5s").c_str());
    case COL_HASHRATE_AVG:
        return QString(miner.opt("hashrate_avg").c_str());
    case COL_TEMPERATURE:
        return QString(miner.opt("temperature").c_str());
    case COL_FAN_SPEED:
        return QString(miner.opt("fan_speed").c_str());
    case COL_ELAPSED:
        return elapsedI18N(QString(miner.opt("elapsed").c_str()));

    // pool 1
    case COL_POOL1:
        return QString(miner.pool1_.url_.c_str()).replace("stratum+tcp://", "");
    case COL_WORKER1:
        return QString(miner.pool1_.worker_.c_str());
    // pool 2
    case COL_POOL2:
        return QString(miner.pool2_.url_.c_str()).replace("stratum+tcp://", "");
    case COL_WORKER2:
        return QString(miner.pool2_.worker_.c_str());
    // pool 3
    case COL_POOL3:
        return QString(miner.pool3_.url_.c_str()).replace("stratum+tcp://", "");
    case COL_WORKER3:
        return QString(miner.pool3_.worker_.c_str());

    // versions
    case COL_FIRMWARE:
        return QString(miner.opt("firmware_version").c_str());
    case COL_SOFTWARE:
        return QString(miner.opt("software_version").c_str());
    case COL_HARDWARE:
        return QString(miner.opt("hardware_version").c_str());

    // network informations
    case COL_NETWORK:
        return QString(miner.opt("network_type").c_str());
    case COL_MAC_ADDRESS:
        return QString(miner.opt("mac_address").c_str());

    // unknown
    default:
        return QString("");
    }
}

QVariant MinerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
    {
        return section;
    }

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    return titles_.at(section);
}

void MinerTableModel::sort(int column, Qt::SortOrder order)
{
    setSort(column, order == Qt::DescendingOrder);
    emit dataChanged(index(0, 0), index(miners_.size() - 1, columnSize_ - 1));
}

void MinerTableModel::updateMinerNumCount()
{
    successMinerNum_ = 0;
    timeoutMinerNum_ = 0;
    okMinerNum_ = 0;
    skipMinerNum_ = 0;
    rebootedMinerNum_ = 0;
    upgradedMinerNum_ = 0;
    otherMinerNum_ = 0;

    bool success = minerNumCountSql_.exec();

    if (!success)
    {
        throw QString("MinerTableModel: count number failed! ") + minerNumCountSql_.lastError().text();
    }

    while (minerNumCountSql_.next())
    {
        QString stat = minerNumCountSql_.value(0).toString();
        int num = minerNumCountSql_.value(1).toInt();

        if (stat == "success")
        {
            successMinerNum_ = num;
        }
        else if (stat == "timeout")
        {
            timeoutMinerNum_ = num;
        }
        else if (stat == "ok")
        {
            okMinerNum_ = num;
        }
        else if (stat == "skip")
        {
            skipMinerNum_ = num;
        }
        else if (stat == "rebooted")
        {
            rebootedMinerNum_ = num;
        }
        else if (stat == "upgraded")
        {
            upgradedMinerNum_ = num;
        }
        else
        {
            // there could be multiple other states.
            // so sum them.
            otherMinerNum_ += num;
        }
    }
}

int MinerTableModel::allMinerNum()
{
    return miners_.size();
}

int MinerTableModel::successMinerNum()
{
    return successMinerNum_;
}

int MinerTableModel::timeoutMinerNum()
{
    return timeoutMinerNum_;
}

int MinerTableModel::okMinerNum()
{
    return okMinerNum_;
}

int MinerTableModel::skipMinerNum()
{
    return skipMinerNum_;
}

int MinerTableModel::rebootedMinerNum()
{
    return rebootedMinerNum_;
}

int MinerTableModel::upgradedMinerNum()
{
    return upgradedMinerNum_;
}

int MinerTableModel::otherMinerNum()
{
    return otherMinerNum_;
}
