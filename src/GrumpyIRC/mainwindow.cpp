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
#include "../libirc/libirc/serveraddress.h"
#include "../libcore/eventhandler.h"
#include "../libcore/ircsession.h"
#include "../libcore/core.h"
#include "../libcore/configuration.h"
#include "../libcore/commandprocessor.h"

using namespace GrumpyIRC;

MainWindow *MainWindow::Main;

static void Exit()
{
    GCFG->SetValue("mainwindow_state", QVariant(MainWindow::Main->saveState()));
    GCFG->SetValue("mainwindow_geometry", QVariant(MainWindow::Main->saveGeometry()));
    GCFG->Save();
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

static int SystemCommand_Server(SystemCommand *command, CommandArgs command_args)
{
	// if there is no parameter we throw some error
	if (command_args.Parameters.count() < 1)
	{
		GRUMPY_ERROR(QObject::tr("This command requires a parameter"));
		return 0;
	}
	// get the server host
    MainWindow::Main->OpenServer(libirc::ServerAddress(command_args.Parameters[0]));
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
	CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("server", (SC_Callback)SystemCommand_Server));
    // Welcome user
    this->systemWindow->InsertText(QString("Grumpy irc version " + GCFG->GetVersion()));
    // Try to restore geometry
    this->restoreGeometry(GCFG->GetValue("mainwindow_geometry").toByteArray());
    this->restoreState(GCFG->GetValue("mainwindow_state").toByteArray());
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

void MainWindow::WriteToSystemWindow(QString text)
{
    this->systemWindow->InsertText(text);
}

void MainWindow::OpenServer(libirc::ServerAddress &server)
{
    if (server.GetNick().isEmpty)
        server.SetNick(CONFIG_NICK);
    QString network_name = server.GetHost();
    // We need to create a new scrollback for system window
    ScrollbackFrame *system = this->GetScrollbackManager()->CreateWindow(network_name, NULL, true);
    IRCSession::Open(system, server, network_name);
}

void MainWindow::on_actionExit_triggered()
{
    Exit();
}
