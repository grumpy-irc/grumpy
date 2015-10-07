#-------------------------------------------------
#
# Project created by QtCreator 2015-10-14T15:32:26
#
#-------------------------------------------------

QT       -= gui

TARGET = libgp
TEMPLATE = lib

DEFINES += LIBGP_LIBRARY

SOURCES += main.cpp


unix {
    target.path = /usr/lib
    INSTALLS += target
