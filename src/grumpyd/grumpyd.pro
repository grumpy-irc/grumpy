QT += core network
QT -= gui

TARGET = grumpyd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    corewrapper.cpp \
    grumpyd.cpp \
    listener.cpp \
    session.cpp

HEADERS += \
    corewrapper.h \
    grumpyd.h \
    listener.h \
    session.h

