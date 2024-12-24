#include <btctools/utils/OOLuaHelper.h>
#include "config.h"
#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>
#include <QFile>
#include <QCommandLineParser>
#include <QMessageBox>
#include "utils.h"

using namespace std;

// load lua script from QT resource
bool scriptLoader(const string &name, string &content, string &errmsg)
{
    QString path(name.c_str());
    path.replace('.', '/');

    // Use the file in the current directory in debug mode, 
    // otherwise use the file in the Qt resource.
    path = QString("%1/lua/scripts/%2.lua").arg(Utils::debugMode() ? "." : ":").arg(path);
    QFile file(path);

    if (file.exists())
    {
        if (file.open(QIODevice::ReadOnly)) {
            content = file.readAll().toStdString();
            return true;
        }
        else
        {
            errmsg = string(" resource failed: ") + file.errorString().toUtf8().data();
            return false;
        }
    }
    else
    {
        errmsg = string(" resource not found: ") + path.toUtf8().data();
        return false;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        // --------------- parse command line args ---------------

        // command line args
        QCommandLineOption argDebug("debug", "Enable debug mode");
        QCommandLineOption argNoScaling("no-scaling", "Disable high-DPI scaling for UI");
        QCommandLineOption argLangCode("lang", "Language code for UI", "lang");

        QStringList argList;
        int argPos;

        for (argPos=0; argPos<argc; argPos++)
        {
            argList.append(argv[argPos]);
        }

        QCommandLineParser argParser;
        argParser.addOption(argDebug);
        argParser.addOption(argLangCode);
        argParser.addOption(argNoScaling);
        argParser.process(argList);

        // --------------- end of parse command line args ---------------

        if (argParser.isSet(argDebug))
        {
            Utils::enableDebugMode();
        }

        if (!argParser.isSet(argNoScaling))
        {
            // enable devicePixelRatio scaling for high-DPI devices
            QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        }


        // regester script loader
        btctools::utils::ScriptLoader loader = scriptLoader;
        btctools::utils::OOLuaHelper::setScriptLoader(loader);


        // application init
        QApplication a(argc, argv);

#ifdef WIN32
        // change working directory to process's dir
        QDir::setCurrent(QApplication::applicationDirPath());
#else
        // Change working directory to $HOME
        // The directory where the bin is located is usually not writable on Linux
        QDir::setCurrent(QDir::homePath());
#endif

        qDebug() << "Platform: " << PLATFORM_NAME;
        qDebug() << "Working dir: " << QDir::current().path();

        // --------------- copy scripts for debug ---------------
        if (Utils::debugMode() && !Utils::isDir("./lua")) {
            qDebug() << "Copy scripts for debug: " << (Utils::copyRecursively(":/lua", "./lua") ? "success" : "failed");
        }

        // --------------- i18n support ---------------
        QLocale locale;
        QTranslator uiTranslator;
        QTranslator qtTranslator;

        QString langCode = locale.name();

        // change language from command arg
        if (argParser.isSet(argLangCode))
        {
            langCode = argParser.value(argLangCode);
        }

        bool loadQtLocaleSuccess = qtTranslator.load(QString("%1/qt_%2.qm").arg(LOCALE_DIR_PATH).arg(langCode));
        bool loadUiLocaleSuccess = uiTranslator.load(QString("%1/%2.qm").arg(LOCALE_DIR_PATH).arg(langCode));

        if (loadQtLocaleSuccess)
        {
            a.installTranslator(&qtTranslator);
            qDebug() << "Language pack loaded: " << "qt_" + langCode;

            if (loadUiLocaleSuccess)
            {
                a.installTranslator(&uiTranslator);
                qDebug() << "Language pack loaded: " << langCode;
            }
            else
            {
                qDebug() << "Language pack missing: " << langCode;
            }
        }
        else
        {
            qDebug() << "Language pack missing: " << "qt_" + langCode;

            // set default locale: en
            QLocale::setDefault(QLocale(QLocale::English, QLocale::AnyCountry));
        }

        // ----------- end of i18n support ------------

        // main window init
        MainWindow w;
        w.show();

        return a.exec();
    }
    catch (const QString &ex)
    {
        qDebug() << ex;

        QApplication a(argc, argv);
        QMessageBox::critical(nullptr, QObject::tr("A critical error occured"), ex);
        a.exit(-1);

        return -1;
    }
    catch (const std::exception &ex)
    {
        qDebug() << ex.what();

        QApplication a(argc, argv);
        QMessageBox::critical(nullptr, QObject::tr("A critical error occured"), QString(ex.what()));
        a.exit(-1);

        return -1;
    }
    catch (...)
    {
        qDebug() << "Uncached Error!";

        QApplication a(argc, argv);
        QMessageBox::critical(nullptr, QObject::tr("A critical error occured"), QObject::tr("Unknown error."));
        a.exit(-1);

        return -1;
    }
}
