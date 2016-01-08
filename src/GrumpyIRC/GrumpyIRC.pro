#-------------------------------------------------
#
# Project created by QtCreator 2015-09-24T15:08:26
#
#-------------------------------------------------

QT       += core gui network

QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GrumpyIRC
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    scrollbackframe.cpp \
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
    favoriteswin.cpp \
    grumpydcfwin.cpp \
    linkhandler.cpp \
    initializewin.cpp \
    hooks.cpp \
    messagebox.cpp \
    sessionwindow.cpp \
    systemcmds.cpp

HEADERS  += mainwindow.h \
    scrollbackframe.h \
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
    favoriteswin.h \
    grumpydcfwin.h \
    linkhandler.h \
    initializewin.h \
    hooks.h \
    messagebox.h \
    sessionwindow.h \
    systemcmds.h

FORMS    += mainwindow.ui \
    scrollbackframe.ui \
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
    initializewin.ui \
    messagebox.ui \
    sessionwindow.ui

RESOURCES += \
    embedded.qrc

DISTFILES += \
    scripts/_autoexec
