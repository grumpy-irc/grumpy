#-------------------------------------------------
#
# Project created by QtCreator 2015-10-14T15:32:26
#
#-------------------------------------------------

QT       -= gui

TARGET = libirc2htmlcode
TEMPLATE = lib

DEFINES += LIBIRC2HTMLCODE_LIBRARY

SOURCES += parser.cpp \
    formatteditem.cpp

HEADERS += parser.h\
        libirc2htmlcode_global.h \
    formatteditem.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
