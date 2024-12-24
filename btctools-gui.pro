#-------------------------------------------------
#
# Project created by QtCreator 2017-03-20T15:38:12
#
#-------------------------------------------------

QT += core gui network widgets sql

#-------------------------------------------------
# uncoment the follow line if the program cannot be executed in Windows XP.
#-------------------------------------------------

TARGET = btctools-gui
TEMPLATE = app
RC_FILE = res/appinfo.rc

SOURCES += src/main.cpp\
    src/minerscanner.cpp \
    src/minertablemodel.cpp \
    src/mainwindow.cpp \
    src/minerconfigurator.cpp \
    src/autoupdater.cpp \
    src/iprangeedit.cpp \
    src/iprangelistitem.cpp \
    src/minerrebooter.cpp \
    src/iprangeedititem.cpp \
    src/iprangewindow.cpp \
    src/passworddelegate.cpp \
    src/utils.cpp \
    src/settingwindow.cpp \
    src/checkmessagebox.cpp \
    src/upgradewindow.cpp \
    src/minerupgrader.cpp

HEADERS  += \
    src/passworddelegate.h \
    src/translation.h\
    src/minerscanner.h \
    src/minertablemodel.h \
    src/mainwindow.h \
    src/minerconfigurator.h \
    src/config.h \
    src/autoupdater.h \
    src/iprangeedit.h \
    src/iprangelistitem.h \
    src/minerrebooter.h \
    src/iprangewindow.h \
    src/iprangeedititem.h \
    src/utils.h \
    src/settingwindow.h \
    src/checkmessagebox.h \
    src/upgradewindow.h \
    src/minerupgrader.h

FORMS    += \
    res/ui/mainwindow.ui \
    res/ui/iprangewindow.ui \
    res/ui/settingwindow.ui \
    res/ui/upgradewindow.ui

DISTFILES += \
    doc/miner-type-stat-spec.md \
    doc/auto-update-spec.md \
    .gitignore \
    res/locale/zh_CN.ts \
    res/appinfo.rc \
    res/update-data-linuxsnap-amd64.json \
    res/update-data-linuxsnap-arm64.json \
    res/update-data-winexe-i386.json

RESOURCES += \
    res/resources.qrc

TRANSLATIONS += \
    res/locale/zh_CN.ts

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -lbtctools -llua51 -lcryptopp-static -llibcurl-d -llibssl -llibcrypto \
                -lboost_regex-vc140-mt-gd -lboost_chrono-vc140-mt-gd -lboost_context-vc140-mt-gd \
                -lbz2d -lzlibd -lws2_32 -lwldap32 -lcrypt32
    } else {
        LIBS += -lbtctools -llua51 -lcryptopp-static -llibcurl -llibssl -llibcrypto \
                -lboost_regex-vc140-mt -lboost_chrono-vc140-mt -lboost_context-vc140-mt \
                -lbz2 -lzlib -lws2_32 -lwldap32 -lcrypt32
    }
} else {
    LIBS += -lbtctools -llua5.1 -lcryptopp -lcurl -lssl -lcrypto \
            -lboost_regex -lboost_system -lboost_chrono -lboost_context
}

msvc: QMAKE_CXXFLAGS += -source-charset:utf-8 -D_WIN32_WINNT=0x0601 -DWIN32_LEAN_AND_MEAN

defineTest(envNotEmpty) {
    env = $$1
    isEmpty(env) {
        return(false)
    } else {
        return(true)
    }
}

envNotEmpty("$$(QT_INCLUDE_DIR)") {
    INCLUDEPATH += "$$(QT_INCLUDE_DIR)"
    TR_EXCLUDE += "$$(QT_INCLUDE_DIR)/*"
}

envNotEmpty("$$(BTCTOOLS_INCLUDE_DIR)") {
    INCLUDEPATH += "$$(BTCTOOLS_INCLUDE_DIR)"
    TR_EXCLUDE += "$$(BTCTOOLS_INCLUDE_DIR)/*"
}
envNotEmpty("$$(LUA_INCLUDE_DIR)") {
    INCLUDEPATH += "$$(LUA_INCLUDE_DIR)"
    TR_EXCLUDE += "$$(LUA_INCLUDE_DIR)/*"
}
envNotEmpty("$$(CRYPTOPP_INCLUDE_DIR)") {
    INCLUDEPATH += "$$(CRYPTOPP_INCLUDE_DIR)"
    TR_EXCLUDE += "$$(CRYPTOPP_INCLUDE_DIR)/*"
}

envNotEmpty("$$(QT_LIB_DIR)"): LIBS += -L"$$(QT_LIB_DIR)"
envNotEmpty("$$(BTCTOOLS_LIB_DIR)"): LIBS += -L"$$(BTCTOOLS_LIB_DIR)"
envNotEmpty("$$(LUA_LIB_DIR)"):      LIBS += -L"$$(LUA_LIB_DIR)"
envNotEmpty("$$(CRYPTOPP_LIB_DIR)"): LIBS += -L"$$(CRYPTOPP_LIB_DIR)"

envNotEmpty("$$(PLATFORM_NAME)"): QMAKE_CXXFLAGS += -DPLATFORM_NAME="'\"$$(PLATFORM_NAME)\"'"
