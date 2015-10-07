QT += core
QT -= gui

TARGET = grumpyd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    corewrapper.cpp \
    syslog.cpp

HEADERS += \
    corewrapper.h \
    syslog.h

