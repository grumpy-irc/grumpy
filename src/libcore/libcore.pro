#-------------------------------------------------
#
# Project created by QtCreator 2015-09-24T15:28:49
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = libcore
TEMPLATE = lib

DEFINES += LIBCORE_LIBRARY

SOURCES += core.cpp \
    serversession.cpp \
    serverlistener.cpp \
    usersession.cpp \
    eventhandler.cpp \
    ircsession.cpp \
    scrollback.cpp \
    commandprocessor.cpp \
    generic.cpp \
    configuration.cpp \
    exception.cpp \
    factory.cpp

HEADERS += core.h\
        libcore_global.h \
    serversession.h \
    serverlistener.h \
    usersession.h \
    eventhandler.h \
    ircsession.h \
    scrollback.h \
    commandprocessor.h \
    generic.h \
    configuration.h \
    definitions.h \
    exception.h \
    factory.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
