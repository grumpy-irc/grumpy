QT += core
QT -= gui

TARGET = grumpyd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    corewrapper.cpp \
    grumpyd.cpp

HEADERS += \
    corewrapper.h \
    grumpyd.h

