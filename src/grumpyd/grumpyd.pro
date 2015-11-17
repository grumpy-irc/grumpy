QT += core network xml
QT -= gui

TARGET = grumpyd
CONFIG += console
CONFIG -= app_bundle

DEFINES += GRUMPYD_SQLITE

TEMPLATE = app

SOURCES += main.cpp \
    corewrapper.cpp \
    grumpyd.cpp \
    listener.cpp \
    session.cpp \
    sleeper.cpp \
    databasebackend.cpp \
    databasexml.cpp \
    user.cpp \
    security.cpp \
    virtualscrollback.cpp \
    scrollbackfactory.cpp \
    syncableircsession.cpp \
    databasebin.cpp \
    grumpyconf.cpp \
    databasedummy.cpp \
    userconfiguration.cpp \
    databaselite.cpp

HEADERS += \
    corewrapper.h \
    grumpyd.h \
    listener.h \
    session.h \
    sleeper.h \
    databasebackend.h \
    databasexml.h \
    user.h \
    security.h \
    virtualscrollback.h \
    scrollbackfactory.h \
    syncableircsession.h \
    databasebin.h \
    grumpyconf.h \
    databasedummy.h \
    userconfiguration.h \
    databaselite.h

RESOURCES += \
    resources.qrc

DISTFILES += \
    install.sql

