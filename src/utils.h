#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QStringList>
#include <QPair>
#include <QList>
#include <QDebug>
#include <string>
#include <stdint.h>

using QStringSet = QSet<QString>;
using FirmwareMap = QMap<QString, QStringSet>;

using MinerHashrate = QPair<QString, double>;
using MinerHashrateList = QList<MinerHashrate>;

//
// IPv4 Internet address
// This is an 'on-wire' format structure.
// From <inaddr.h> in Windows SDK
//
struct in_addr_btctools {
  union {
    struct { uint8_t s_b1,s_b2,s_b3,s_b4; } S_un_b;
    struct { uint16_t s_w1,s_w2; } S_un_w;
    uint32_t S_addr;
  } S_un;
};

typedef struct MinerPassword {
    QString minerType_;
    QString userName_;
    QString password_;

    MinerPassword(QString minerType, QString userName, QString password)
        : minerType_(minerType), userName_(userName), password_(password)
    {
        // empty body
    }
} MinerPassword;

using MinerPasswordList = QList<MinerPassword>;


class Utils
{
private:
    static bool debugMode_;

public:
    inline static void enableDebugMode(bool enabled = true) {
        debugMode_ = enabled;
    }
    inline static bool debugMode() {
        return debugMode_;
    }

    static QStringList getLanIpList();
    static QString getInitIpRange();

    static QString minerHashrateListToString(const MinerHashrateList &minerHashrateList);
    static MinerHashrateList stringToMinerHashrateList(const QString &minerHashrateString);

    static QString minerPasswordListToString(const MinerPasswordList &minerPasswordList);
    static MinerPasswordList stringToMinerPasswordList(const QString &minerPasswordString);
    static MinerPassword* findMinerPassword(MinerPasswordList &minerPasswordList, const QString &minerType);

    static double unitNumberToDouble(QString unitNumber);
    static QString doubleToUnitNumber(double num);

    static std::string readFirmwareToString(const QString &filePath);
    static QString firmwareMapToString(const FirmwareMap &map);
    static FirmwareMap stringToFirmwareMap(const QString &mapStr);

    static QString getSubAccountName(const QString &workerName);

    static std::string getAnsiString(const QString &string);

    static bool isDir(const QString &path);
    static bool isFile(const QString &path);
    static bool copyRecursively(const QString &sourceFolder, const QString &destFolder);
};


///////////////////////// HTTP Client based libcurl ////////////////////////

bool httpGET (const char *url, std::string &response, long timeoutMs = 0);
bool httpGET (const char *url, const char *userpwd,
              std::string &response, long timeoutMs = 0);
bool httpPOST(const char *url, const char *userpwd, const char *postData,
              std::string &response, long timeoutMs = 0, const char *contentType = nullptr);


#endif // UTILS_H
