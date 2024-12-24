#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H

#include <string>
#include <QObject>
#include <QString>
#include <QThread>
#include <QPair>
#include <QList>

class AutoUpdater : public QThread
{
    Q_OBJECT

public:
    AutoUpdater();

protected:
    void run();

signals:
    void findUpdates(QString versionName, QString description, QString downloadUrl, bool forceUpdate);

private:

};

#endif // AUTOUPDATER_H
