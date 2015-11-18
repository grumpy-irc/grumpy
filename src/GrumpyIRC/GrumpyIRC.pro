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
    scrollbacklist_node.cpp \
    preferenceswin.cpp \
    skin.cpp \
    keyfilter.cpp \
    widgetfactory.cpp \
    userwidget.cpp \
    userframe.cpp \
    userframeitem.cpp \
    grumpyconf.cpp \
    connectwin.cpp \
    aboutwin.cpp \
    packetsnifferwin.cpp \
    stextbox.cpp \
    channelwin.cpp \
    scriptwin.cpp \
    highlighter.cpp \
    favoriteswin.cpp \
    grumpydcfwin.cpp \
    linkhandler.cpp \
    initializewin.cpp \
    hooks.cpp

HEADERS  += mainwindow.h \
    scrollbackframe.h \
    syslogwindow.h \
    scrollbacklist.h \
    corewrapper.h \
    inputbox.h \
    scrollbacksmanager.h \
    grumpyeventhandler.h \
    scrollbacklist_node.h \
    preferenceswin.h \
    skin.h \
    keyfilter.h \
    widgetfactory.h \
    userwidget.h \
    userframe.h \
    userframeitem.h \
    grumpyconf.h \
    connectwin.h \
    aboutwin.h \
    packetsnifferwin.h \
    stextbox.h \
    channelwin.h \
    scriptwin.h \
    highlighter.h \
    favoriteswin.h \
    grumpydcfwin.h \
    linkhandler.h \
    initializewin.h \
    hooks.h

FORMS    += mainwindow.ui \
    scrollbackframe.ui \
    syslogwindow.ui \
    scrollbacklist.ui \
    inputbox.ui \
    scrollbacksmanager.ui \
    preferenceswin.ui \
    userwidget.ui \
    userframe.ui \
    connectwin.ui \
    aboutwin.ui \
    packetsnifferwin.ui \
    channelwin.ui \
    scriptwin.ui \
    favoriteswin.ui \
    grumpydcfwin.ui \
    initializewin.ui

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

RESOURCES += \
    icons.qrc
