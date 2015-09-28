#-------------------------------------------------
#
# Project created by QtCreator 2015-09-24T15:08:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GrumpyIRC
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    scrollbackframe.cpp \
    syslogwindow.cpp \
    scrollbacklist.cpp \
    corewrapper.cpp \
    inputbox.cpp \
    scrollbacksmanager.cpp \
    grumpyeventhandler.cpp \
    scrollbacklist_node.cpp

HEADERS  += mainwindow.h \
    scrollbackframe.h \
    syslogwindow.h \
    scrollbacklist.h \
    corewrapper.h \
    inputbox.h \
    scrollbacksmanager.h \
    grumpyeventhandler.h \
    scrollbacklist_node.h

FORMS    += mainwindow.ui \
    scrollbackframe.ui \
    syslogwindow.ui \
    scrollbacklist.ui \
    inputbox.ui \
    scrollbacksmanager.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../libirc/libirc/build-libirc-Desktop_Qt_5_4_2_MinGW_32bit-Debug/release/ -llibirc
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../libirc/libirc/build-libirc-Desktop_Qt_5_4_2_MinGW_32bit-Debug/debug/ -llibirc
else:unix: LIBS += -L$$PWD/../../../libirc/libirc/build-libirc-Desktop_Qt_5_4_2_MinGW_32bit-Debug/ -llibirc

INCLUDEPATH += $$PWD/../../../libirc/libirc/libirc/
DEPENDPATH += $$PWD/../../../libirc/libirc/build-libirc-Desktop_Qt_5_4_2_MinGW_32bit-Debug/debug

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-libcore-Desktop_Qt_5_4_2_MinGW_32bit-Debug/release/ -llibcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-libcore-Desktop_Qt_5_4_2_MinGW_32bit-Debug/debug/ -llibcore
else:unix: LIBS += -L$$PWD/../build-libcore-Desktop_Qt_5_4_2_MinGW_32bit-Debug/ -llibcore

INCLUDEPATH += $$PWD/../libcore
DEPENDPATH += $$PWD/../build-libcore-Desktop_Qt_5_4_2_MinGW_32bit-Debug/debug
