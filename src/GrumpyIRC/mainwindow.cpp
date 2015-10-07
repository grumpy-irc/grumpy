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
#include "userwidget.h"
#include "scrollbackframe.h"
#include "syslogwindow.h"
#include "defaultconfig.h"
#include "scrollbacksmanager.h"
#include "skin.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libcore/eventhandler.h"
#include "../libcore/exception.h"
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

static int SystemCommand_NextSessionNick(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    if (args.Parameters.count() != 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires precisely 1 parameter"));
        return 1;
    }
    GCFG->SetValue("next-session-nick", args.ParameterLine);
    return 0;
}

static int SystemCommand_Nick(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    // get a current scrollback
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    if (!scrollback)
        throw new GrumpyIRC::NullPointerException("scrollback", BOOST_CURRENT_FUNCTION);
    if (args.Parameters.count() != 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires precisely 1 parameter"));
        return 1;
    }
    if (scrollback->GetSession())
    {
        scrollback->GetSession()->GetNetwork()->TransferRaw("NICK " + args.Parameters[0]);
        return 0;
    }
    else
    {
        SET_CONFIG_NICK(args.Parameters[0]);
        scrollback->InsertText(QString("Your default nick was changed to " + args.Parameters[0]));
        return 0;
    }
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
    this->userWidget = new UserWidget(this);
    this->setCentralWidget(this->scrollbackWindow);
    this->addDockWidget(Qt::LeftDockWidgetArea, this->windowList);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->syslogWindow);
    this->addDockWidget(Qt::RightDockWidgetArea, this->userWidget);
    this->syslogWindow->hide();
    // Create a system scrollback
    this->systemWindow = this->scrollbackWindow->CreateWindow("System Window", NULL, true, false);
    // Register built-in commands
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("quit", (SC_Callback)SystemCommand_Exit));
	CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("server", (SC_Callback)SystemCommand_Server));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("nick", (SC_Callback)SystemCommand_Nick));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.next_session_nick", (SC_Callback)SystemCommand_NextSessionNick));
    // Welcome user
    this->systemWindow->InsertText(QString("Grumpy irc version " + GCFG->GetVersion()));
    // Try to restore geometry
    this->restoreGeometry(GCFG->GetValue("mainwindow_geometry").toByteArray());
    this->restoreState(GCFG->GetValue("mainwindow_state").toByteArray());
    this->userWidget->hide();
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

UserWidget *MainWindow::GetUsers()
{
    return this->userWidget;
}

void MainWindow::OpenIRCNetworkLink(QString link)
{
    MainWindow::Main->OpenServer(libirc::ServerAddress(link));
}

void MainWindow::OpenServer(libirc::ServerAddress &server)
{
    if (server.GetNick().isEmpty())
    {
        QString nick = CONFIG_NICK;
        if (GCFG->GetValueAsString("next-session-nick", nick) != nick)
        {
            nick = GCFG->GetValueAsString("next-session-nick");
            GCFG->RemoveValue("next-session-nick");
        }
        server.SetNick(nick);
    }
    QString network_name = server.GetHost();
    // We need to create a new scrollback for system window
    ScrollbackFrame *system = this->GetScrollbackManager()->CreateWindow(network_name, NULL, true);
    IRCSession::Open(system->GetScrollback(), server, network_name);
}

void MainWindow::on_actionExit_triggered()
{
    Exit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Exit();
}
