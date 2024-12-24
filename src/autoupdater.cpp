#include "autoupdater.h"
#include "config.h"
#include "utils.h"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QLocale>
#include <QString>
#include <string>
#include <btctools/utils/Crypto.h>


AutoUpdater::AutoUpdater()
{
}

void AutoUpdater::run()
{
    try
    {
        std::string response;
        bool success = httpGET(Utils::debugMode() ? APP_AUTO_UPDATE_URL_DEBUG : APP_AUTO_UPDATE_URL, response);

        //-------------- get response --------------
        if (success)
        {
            QByteArray jsonBytes(response.data(), response.size());

            //qDebug() << "json size:" << jsonBytes.size();
            //qDebug() << jsonBytes.data();
            //qDebug() << "end";

            //-------------- parse JSON data --------------
            QJsonParseError parseError;
            QJsonDocument document = QJsonDocument::fromJson(jsonBytes, &parseError);

            if (parseError.error != QJsonParseError::NoError)
            {
                throw std::runtime_error(tr("Parse update info failed: %1").arg(parseError.errorString()).toUtf8().data());
            }

            if (!document.isObject()) {
                throw std::runtime_error(tr("Parse update info failed: must be object").toUtf8().data());
            }

            QJsonObject jsonMain = document.object();

            int versionId = 0;
            QString versionName = "";
            QString description = "";
            QString downloadUrl = "";
            bool forceUpdate = false;

            if (jsonMain.contains("versionId"))
            {
                QJsonValue value = jsonMain.value("versionId");
                versionId = value.toInt();
            }

            if (jsonMain.contains("minVersionId"))
            {
                QJsonValue value = jsonMain.value("minVersionId");
                int minVersionId = value.toInt();

                if (APP_VERSION_ID < minVersionId)
                {
                    forceUpdate = true;
                }
            }

            if (jsonMain.contains("versionName"))
            {
                QJsonValue value = jsonMain.value("versionName");
                versionName = value.toString();
            }

            QLocale locale;
            QString descKeyWithLangCode = QString("desc_") + locale.name();

            if (jsonMain.contains(descKeyWithLangCode))
            {
                QJsonValue value = jsonMain.value(descKeyWithLangCode);
                description = value.toString();
            }
            else if (jsonMain.contains("desc"))
            {
                QJsonValue value = jsonMain.value("desc");
                description = value.toString();
            }

            if (jsonMain.contains("downloadUrl"))
            {
                QJsonValue value = jsonMain.value("downloadUrl");
                downloadUrl = value.toString();
            }

            //-------------- emit the updates --------------
            if (versionId > APP_VERSION_ID)
            {
                emit findUpdates(versionName, description, downloadUrl, forceUpdate);
            }
            else
            {
                qDebug() << tr("Auto Updater: the current version is the latest.");
                qDebug() << tr("Version from server: %1 <%2>").arg(versionName).arg(downloadUrl);
            }
        }
    }
    catch (const std::exception &ex)
    {
        qDebug() << "AutoUpdater error: " << ex.what();
    }
}
