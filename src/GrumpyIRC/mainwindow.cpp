//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "corewrapper.h"
#include "mainwindow.h"
#include "scrollbacklist.h"
#include "ui_mainwindow.h"
#include "scrollbackframe.h"
#include "syslogwindow.h"
#include "scrollbacksmanager.h"
#include "../libcore/core.h"
#include "../libcore/commandprocessor.h"

using namespace GrumpyIRC;

MainWindow *MainWindow::Main;

static void Exit()
{
    QApplication::exit(0);
}

static int SystemCommand_Exit(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    // quit message
    QString quit_msg = args.ParameterLine;
    Exit();
    return 0;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    Main = this;
    this->ui->setupUi(this);
    this->syslogWindow = new SyslogWindow(this);
    this->windowList = new ScrollbackList(this);
    this->scrollbackWindow = new ScrollbacksManager(this);
    this->setCentralWidget(this->scrollbackWindow);
    this->addDockWidget(Qt::LeftDockWidgetArea, this->windowList);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->syslogWindow);
    this->syslogWindow->hide();
    // Create a system scrollback
    this->systemWindow = this->scrollbackWindow->CreateWindow("System Window", NULL, true, false);
    // Register built-in commands
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("quit", (SC_Callback)SystemCommand_Exit));
    // Welcome user
    this->systemWindow->InsertText(QString("Grumpy irc"));
}

MainWindow::~MainWindow()
{
    delete this->ui;
}

ScrollbacksManager *MainWindow::GetScrollbackManager()
{
    return this->scrollbackWindow;
}

ScrollbackList *MainWindow::GetScrollbackList()
{
    return this->windowList;
}

void MainWindow::on_actionExit_triggered()
{
    Exit();
}
