#include <stdexcept>
#include <string>
#include <curl/curl.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/ssl.h>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QFile>
#include <QDir>
#include <QTextCodec>
#include <QDebug>
#include "utils.h"
#include "config.h"

#ifdef WIN32
#include <wincrypt.h>
#endif

using std::string;

bool Utils::debugMode_ = false;

QStringList Utils::getLanIpList()
{
    QList<QHostAddress> addrList = QNetworkInterface::allAddresses();
    QStringList lanIpList;

    foreach (QHostAddress addr, addrList)
    {
        QString addrStr = addr.toString();

        // no localhost, only IPv4, no automatic private address
        if (addr != QHostAddress::LocalHost &&  addr.toIPv4Address() && !addrStr.startsWith("169.254."))
        {
            lanIpList.append(addrStr);
        }
    }

    return lanIpList;
}

QString Utils::getInitIpRange()
{
    QString initIpRange = "192.168..0-192.168..255";

    QStringList lanIpList = Utils::getLanIpList();

    if (!lanIpList.isEmpty())
    {
        QString lanIpPrefix = lanIpList.at(0);

        // get the first two parts from an IP, just likes "192.168" from "192.168.1.100".
        lanIpPrefix = lanIpPrefix.left(lanIpPrefix.lastIndexOf('.', lanIpPrefix.lastIndexOf('.') - 1));

        // it likes "192.168..0-192.168..255". The third part is empty and will be filled by the user.
        initIpRange = lanIpPrefix + "..0-255";
    }

    return initIpRange;
}

MinerHashrateList Utils::stringToMinerHashrateList(const QString &minerHashrateString)
{
    MinerHashrateList minerHashrateList;
    QStringList params = minerHashrateString.split('&');

    foreach (QString param, params)
    {
        QStringList keyValue = param.split('=');

        if (keyValue.size() != 2)
        {
            continue;
        }

        minerHashrateList.append(MinerHashrate(keyValue.at(0), keyValue.at(1).toDouble()));
    }

    return minerHashrateList;
}

QString Utils::minerHashrateListToString(const MinerHashrateList &minerHashrateList)
{
    QStringList strList;

    foreach (auto minerHashrate, minerHashrateList)
    {
        QString minerType = minerHashrate.first.replace('=', ' ').replace('&', ' '); // strip the special characters.
        double hashrate = minerHashrate.second;

        strList.append(QString("%1=%2").arg(minerType).arg(hashrate));
    }

    return strList.join('&');
}

MinerPasswordList Utils::stringToMinerPasswordList(const QString &minerPasswordString)
{
    MinerPasswordList minerPasswordList;
    QStringList params = minerPasswordString.split('&');

    foreach (QString param, params)
    {
        QStringList keyValue = param.split(':');

        if (keyValue.size() != 3)
        {
            continue;
        }

        minerPasswordList.append(MinerPassword(
                                      QString(QByteArray::fromBase64(keyValue.at(0).toUtf8())),
                                      QString(QByteArray::fromBase64(keyValue.at(1).toUtf8())),
                                      QString(QByteArray::fromBase64(keyValue.at(2).toUtf8()))
                                 ));
    }

    return minerPasswordList;
}

QString Utils::minerPasswordListToString(const MinerPasswordList &minerPasswordList)
{
    QStringList strList;

    foreach (auto minerPassword, minerPasswordList)
    {
        strList.append(QString("%1:%2:%3")
                  .arg(QString(minerPassword.minerType_.toUtf8().toBase64()))
                  .arg(QString(minerPassword.userName_.toUtf8().toBase64()))
                  .arg(QString(minerPassword.password_.toUtf8().toBase64()))
                );
    }

    return strList.join('&');
}

MinerPassword* Utils::findMinerPassword(MinerPasswordList &minerPasswordList, const QString &minerType)
{
    for (int i=0; i<minerPasswordList.size(); i++)
    {
        MinerPassword *p = &minerPasswordList[i];
        if (minerType.toLower().contains(p->minerType_.toLower()))
        {
            return p;
        }
    }

    return nullptr;
}

double Utils::unitNumberToDouble(QString unitNumber)
{
    double result = 0;

    unitNumber = unitNumber.trimmed();

    if (unitNumber.isEmpty())
    {
        return 0;
    }

    int pos = [unitNumber] () -> int {
        for (int i=0; i<unitNumber.size(); i++)
        {
            char c = unitNumber.at(i).toLatin1();

            if (c != '.' && (c < '0' || c > '9'))
            {
                return i;
            }
        }

        return -1;
    }();

    if (pos >= 0)
    {
        QString num  = unitNumber.left(pos).trimmed();
        char unit = unitNumber.right(unitNumber.size() - pos).trimmed().at(0).toUpper().toLatin1();

        result = num.toDouble();

        switch (unit)
        {
        case 'K':
            result *= 1.0e3;
            break;
        case 'M':
            result *= 1.0e6;
            break;
        case 'G':
            result *= 1.0e9;
            break;
        case 'T':
            result *= 1.0e12;
            break;
        case 'P':
            result *= 1.0e15;
            break;
        case 'E':
            result *= 1.0e18;
            break;
        case 'Z':
            result *= 1.0e21;
            break;
        case 'Y':
            result *= 1.0e24;
            break;
        }
    }
    else
    {
        result = unitNumber.toDouble();
    }

    return result;
}

QString Utils::doubleToUnitNumber(double num)
{
    QString unit = "";

    if (num >= 1.0e24) {
        num /= 1.0e24;
        unit = "Y";
    }
    else if (num >= 1.0e21) {
        num /= 1.0e21;
        unit = "Z";
    }
    else if (num >= 1.0e18) {
        num /= 1.0e18;
        unit = "E";
    }
    else if (num >= 1.0e15) {
        num /= 1.0e15;
        unit = "P";
    }
    else if (num >= 1.0e12) {
        num /= 1.0e12;
        unit = "T";
    }
    else if (num >= 1.0e9) {
        num /= 1.0e9;
        unit = "G";
    }
    else if (num >= 1.0e6) {
        num /= 1.0e6;
        unit = "M";
    }
    else if (num >= 1.0e3) {
        num /= 1.0e3;
        unit = "k";
    }

    return QString("%1%2").arg(num).arg(unit);
}

std::string Utils::readFirmwareToString(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error(QObject::tr("Cannot open firmware %1:\n%2").arg(filePath).arg(file.errorString()).toUtf8().data());
    }

    std::string data;
    data.resize(file.size());

    file.read((char*)data.data(), file.size());

    return data;
}

QString Utils::firmwareMapToString(const FirmwareMap &map)
{
    QStringList models;
    for (auto mItr=map.begin(); mItr!=map.end(); mItr++) {
        QStringList firmwares;
        for (auto fItr=mItr.value().begin(); fItr!=mItr.value().end(); fItr++) {
            firmwares.append(fItr->toUtf8().toBase64());
        }
        if (firmwares.isEmpty()) {
            continue;
        }
        models.append(mItr.key().toUtf8().toBase64() + ":" + firmwares.join(';'));
    }
    return models.join('&');
}

FirmwareMap Utils::stringToFirmwareMap(const QString &mapStr)
{
    FirmwareMap map;

    if (mapStr.isEmpty()) {
        return map;
    }

    QStringList models = mapStr.split('&');
    for (auto mItr=models.begin(); mItr!=models.end(); mItr++) {
        QStringList kv = mItr->split(':');
        if (kv.size() >= 2) {
            QString model = QString(QByteArray::fromBase64(kv[0].toUtf8()));
            QStringList firmwares = kv[1].split(';');
            for (auto fItr=firmwares.begin(); fItr!=firmwares.end(); fItr++) {
                *fItr = QString(QByteArray::fromBase64(fItr->toUtf8()));
            }
            map[model] = firmwares.toSet();
        }
    }

    return map;
}

QString Utils::getSubAccountName(const QString &workerName)
{
    QStringList parts = workerName.split('.');
    if (parts.isEmpty()) {
        return "";
    }
    return parts[0];
}

string Utils::getAnsiString(const QString &oriString) {
    return QTextCodec::codecForLocale()->fromUnicode(oriString).data();
}

bool Utils::isDir(const QString &path) {
    QDir dir(path);
    return dir.exists();
}

bool Utils::isFile(const QString &path) {
    QFile file(path);
    return file.exists();
}

// Copied from <https://gist.github.com/ssendeavour/7324701>
bool Utils::copyRecursively(const QString &srcFilePath, const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))
            return false;
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QDir::separator() + fileName;
            const QString newTgtFilePath
                    = tgtFilePath + QDir::separator() + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
                return false;
        }
    } else {
        if (!QFile::copy(srcFilePath, tgtFilePath))
            return false;
    }
    return true;
}


///////////////////////// HTTP Client based libcurl ////////////////////////

struct CurlChunk {
  char *memory;
  size_t size;
};

static size_t
CurlWriteChunkCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct CurlChunk *mem = (struct CurlChunk *)userp;

  mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int sslContextFunction(void* curl, void* sslctx, void* userdata);

bool httpPOST(const char *url, const char *userpwd, const char *postData,
              std::string &response, long timeoutMs, const char *contentType) {
  struct curl_slist *headers = NULL;
  CURLcode status;
  long code;
  CURL *curl = curl_easy_init();
  struct CurlChunk chunk;
  if (!curl) {
    return false;
  }

  chunk.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size   = 0;          /* no data at this point */

  // RSK doesn't support 'Expect: 100-Continue' in 'HTTP/1.1'.
  // So switch to 'HTTP/1.0'.
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

  if (contentType != nullptr) {
    string mineHeader = string("Content-Type: ") + string(contentType);
    headers = curl_slist_append(headers, mineHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);

  if (postData != nullptr) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postData));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    postData);
  }

  if (userpwd != nullptr) {
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
  }

  // Follow redirect
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

#ifdef WIN32
  // Use Windows cert stores
  curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslContextFunction);
#endif

  curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);

  curl_easy_setopt(curl, CURLOPT_USERAGENT, "BTCTools/" APP_VERSION_NAME);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteChunkCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,     (void *)&chunk);

  status = curl_easy_perform(curl);
  if (status != 0) {
    qDebug() << "unable to request data from: " << url << ", error: " << curl_easy_strerror(status);
    goto error;
  }

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
  if (code != 200) {
    qDebug() << "server responded with code: " << code;
    goto error;
  }

  response.assign(chunk.memory, chunk.size);

  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  free(chunk.memory);
  return true;


error:
  if (curl)
    curl_easy_cleanup(curl);
  if (headers)
    curl_slist_free_all(headers);

  free(chunk.memory);
  return false;
}

bool httpGET(const char *url, string &response, long timeoutMs) {
  return httpPOST(url, nullptr, nullptr, response, timeoutMs, nullptr);
}

bool httpGET(const char *url, const char *userpwd,
             string &response, long timeoutMs) {
  return httpPOST(url, userpwd, nullptr, response, timeoutMs, nullptr);
}

#ifdef WIN32
// Use Windows cert store
// From <https://curl.haxx.se/mail/meet-2017-03/0030.html>
std::vector<X509*> m_trustedCertificateList;

void addCertificatesForStore(LPCWSTR name)
{
  HCERTSTORE storeHandle = CertOpenSystemStore(NULL, name);

  if (storeHandle == nullptr)
  {
    return;
  }

  PCCERT_CONTEXT windowsCertificate = CertEnumCertificatesInStore(storeHandle, nullptr);
  while (windowsCertificate != nullptr)
  {
    X509 *opensslCertificate = d2i_X509(nullptr, const_cast<unsigned char const **>(&windowsCertificate->pbCertEncoded), windowsCertificate->cbCertEncoded);
    if (opensslCertificate == nullptr)
    {
      printf("A certificate could not be converted");
    }
    else
    {
      m_trustedCertificateList.push_back(opensslCertificate);
    }

    windowsCertificate = CertEnumCertificatesInStore(storeHandle, windowsCertificate);
  }

  CertCloseStore(storeHandle, 0);
}

void initCertStore()
{
  addCertificatesForStore(L"CA");
  addCertificatesForStore(L"AuthRoot");
  addCertificatesForStore(L"ROOT");
}

void setupSslContext(SSL_CTX* context)
{
  initCertStore();

  X509_STORE* certStore = SSL_CTX_get_cert_store(context);
  for(X509 *x509 : m_trustedCertificateList)
  {
    X509_STORE_add_cert(certStore, x509);
  }
}

int sslContextFunction(void* curl, void* sslctx, void* userdata)
{
  setupSslContext(reinterpret_cast<SSL_CTX *>(sslctx));
  return CURLE_OK;
}
#endif
