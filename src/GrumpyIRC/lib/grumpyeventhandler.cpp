//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#include "corewrapper.h"
#include "mainwindow.h"
#include "grumpyconf.h"
#include "grumpyeventhandler.h"
#include <libcore/core.h>
#include <libcore/configuration.h>
#include <QDebug>

using namespace GrumpyIRC;

void GrumpyEventHandler::OnMessage(scrollback_id_t ScrollbackID)
{

}

void GrumpyEventHandler::OnDebug(const QString &text, unsigned int verbosity)
{
    qDebug() << text;

    if (GCFG->Verbosity < verbosity)
        return;

    if (!MainWindow::Main)
        return;

    // Write message to system window
    MainWindow::Main->WriteToCurrentWindow("DEBUG(" + QString::number(verbosity) + "): " + text);
}

void GrumpyEventHandler::OnError(const QString &text)
{
    qWarning() << text;
    if (!MainWindow::Main)
        return;

    // Write error to system window
    MainWindow::Main->WriteToCurrentWindow(QObject::tr("ERROR") + ": " + text, ScrollbackItemType_SystemError);
}

void GrumpyEventHandler::OnSystemLog(const QString &text)
{
    qDebug() << "LOG: " + text;
    if (!MainWindow::Main)
        return;

    // Write log to system window
    MainWindow::Main->WriteToCurrentWindow(text, ScrollbackItemType_System);
}

