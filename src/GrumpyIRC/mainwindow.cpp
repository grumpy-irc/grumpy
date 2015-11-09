//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "aboutwin.h"
#include "corewrapper.h"
#include "mainwindow.h"
#include "connectwin.h"
#include "scrollbacklist.h"
#include "ui_mainwindow.h"
#include "userwidget.h"
#include "grumpyconf.h"
#include "preferenceswin.h"
#include "scrollbackframe.h"
#include "../libcore/generic.h"
#include "syslogwindow.h"
#include "scrollbacksmanager.h"
#include "skin.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libcore/eventhandler.h"
#include "../libgp/gp.h"
#include "../libcore/grumpydsession.h"
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
        scrollback->GetSession()->SendRaw(scrollback->GetScrollback(), "NICK " + args.Parameters[0]);
        return 0;
    }
    else
    {
        CONF->SetNick(args.Parameters[0]);
        scrollback->InsertText(QString("Your default nick was changed to " + args.Parameters[0]));
        return 0;
    }
}

static int SystemCommand_Netstat(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command_args);
    Q_UNUSED(command);
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    if (!sx->GetSession() || !Generic::IsGrumpy(sx))
        return 2;
    GrumpydSession *session = (GrumpydSession*)sx->GetSession();
    sx->InsertText("Bytes rcvd:" + QString::number(session->GetBytesRcvd()));
    sx->InsertText("Bytes sent:" + QString::number(session->GetBytesSent()));
    return 0;
}

static int SystemCommand_RAW(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (command_args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires some text"));
        return 1;
    }
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    scrollback->GetSession()->SendRaw(scrollback->GetScrollback(), command_args.ParameterLine);
    return 0;
}

static int SystemCommand_Grumpy(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // if there is no parameter we throw some error
    if (command_args.Parameters.count() < 3)
    {
        GRUMPY_ERROR(QObject::tr("This command requires exactly 3 parameters"));
        return 0;
    }
    MainWindow::Main->OpenGrumpy(command_args.Parameters[0], GP_DEFAULT_SSL_PORT, command_args.Parameters[1], command_args.Parameters[2], true);
    return 0;
}

static int SystemCommand_Act(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    if (command_args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires some text"));
        return 1;
    }
    ScrollbackFrame *scrollback = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback();
    if (!scrollback->GetSession() ||
            !scrollback->GetSession()->IsConnected() ||
            scrollback->GetScrollback()->GetType() == ScrollbackType_System)
    {
        GRUMPY_ERROR(QObject::tr("You can only use this command in channel or user windows of connected networks"));
        return 2;
    }
    scrollback->GetSession()->SendAction(scrollback->GetScrollback(), command_args.ParameterLine);
    return 0;
}

static int SystemCommand_UnsecureGrumpy(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // if there is no parameter we throw some error
    if (command_args.Parameters.count() < 3)
    {
        GRUMPY_ERROR(QObject::tr("This command requires exactly 3 parameters"));
        return 0;
    }
    MainWindow::Main->OpenGrumpy(command_args.Parameters[0], GP_DEFAULT_PORT, command_args.Parameters[1], command_args.Parameters[2], false);
    return 0;
}

static int SystemCommand_Server(SystemCommand *command, CommandArgs command_args)
{
    Q_UNUSED(command);
    // if there is no parameter we throw some error
    if (command_args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires a parameter"));
        return 0;
    }
    // This command is much more tricky that you think
    // we need to first check if we are going to open
    // new server within Grumpy itself, or in grumpyd
    Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
    if (sx->GetSession() && sx->GetSession()->GetType() == SessionType_Grumpyd)
    {
        // Current window belongs to grumpyd
        GrumpydSession *session = (GrumpydSession*)sx->GetSession();
        // Connect to IRC
        session->Open(libirc::ServerAddress(command_args.Parameters[0]));
        sx->InsertText("Requested connection to IRC server from grumpyd, please wait.");
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
    this->statusFrame = new QLabel(this);
    this->identFrame = new QLabel(this);
    this->ui->statusBar->addPermanentWidget(this->identFrame);
    this->addDockWidget(Qt::LeftDockWidgetArea, this->windowList);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->syslogWindow);
    this->addDockWidget(Qt::RightDockWidgetArea, this->userWidget);
    this->ui->statusBar->addPermanentWidget(this->statusFrame);
    ScrollbacksManager::Global = this->scrollbackWindow;
    this->syslogWindow->hide();
    // Create a system scrollback
    this->systemWindow = this->scrollbackWindow->CreateWindow("System Window", NULL, true, false);
    // Register built-in commands
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("quit", (SC_Callback)SystemCommand_Exit));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("server", (SC_Callback)SystemCommand_Server));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("nick", (SC_Callback)SystemCommand_Nick));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("me", (SC_Callback)SystemCommand_Act));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.netstat", (SC_Callback)SystemCommand_Netstat));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.next_session_nick", (SC_Callback)SystemCommand_NextSessionNick));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("unsecuregrumpyd", (SC_Callback)SystemCommand_UnsecureGrumpy));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpyd", (SC_Callback)SystemCommand_Grumpy));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("raw", (SC_Callback)SystemCommand_RAW));
    // Welcome user
    this->systemWindow->InsertText(QString("Grumpy irc version " + GCFG->GetVersion()));
    connect(&this->timer, SIGNAL(timeout()), this, SLOT(OnRefresh()));
    this->timer.start(100);
    // Try to restore geometry
    this->restoreGeometry(GCFG->GetValue("mainwindow_geometry").toByteArray());
    this->restoreState(GCFG->GetValue("mainwindow_state").toByteArray());
    this->userWidget->hide();
}

MainWindow::~MainWindow()
{
    ScrollbacksManager::Global = NULL;
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

void MainWindow::WriteToCurrentWindow(QString text)
{
    this->scrollbackWindow->GetCurrentScrollback()->InsertText(text);
}

ScrollbackFrame *MainWindow::GetSystem()
{
    return this->systemWindow;
}

ScrollbackFrame *MainWindow::GetCurrentScrollbackFrame()
{
    return this->GetScrollbackManager()->GetCurrentScrollback();
}

UserWidget *MainWindow::GetUsers()
{
    return this->userWidget;
}

void MainWindow::SetWN(QString text)
{
    if (text.isEmpty())
        this->setWindowTitle("GrumpyIRC");
    else
        this->setWindowTitle("GrumpyIRC - " + text);
}

void MainWindow::UpdateStatus()
{
    int synced = this->GetScrollbackManager()->GetCurrentScrollback()->GetSynced();
    int total = this->GetScrollbackManager()->GetCurrentScrollback()->GetItems();
    this->statusFrame->setText("Items (synced/total): " + QString::number(synced) + " / " + QString::number(total));
    libircclient::User *self_ident = this->GetCurrentScrollbackFrame()->GetIdentity();
    if (!self_ident)
        this->identFrame->setText("");
    else
        this->identFrame->setText(self_ident->ToString());
}

void MainWindow::OpenGrumpy(QString hostname, int port, QString username, QString password, bool ssl)
{
    ScrollbackFrame *system = this->GetScrollbackManager()->CreateWindow(hostname, NULL, true);
    GrumpydSession *session = new GrumpydSession(system->GetScrollback(), hostname, username, password, port, ssl);
    session->Connect();
}

void MainWindow::OpenIRCNetworkLink(QString link)
{
    MainWindow::Main->OpenServer(libirc::ServerAddress(link));
}

void MainWindow::OpenServer(libirc::ServerAddress server)
{
    if (server.GetNick().isEmpty())
    {
        QString nick = CONF->GetNick();
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

void MainWindow::OnRefresh()
{
    this->UpdateStatus();
}

void MainWindow::on_actionExit_triggered()
{
    Exit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Exit();
}

void GrumpyIRC::MainWindow::on_actionConnect_triggered()
{
    ConnectWin *wx = new ConnectWin(this);
    wx->setAttribute(Qt::WA_DeleteOnClose);
    wx->show();
}

void GrumpyIRC::MainWindow::on_actionAbout_triggered()
{
    AboutWin *wx = new AboutWin(this);
    wx->setAttribute(Qt::WA_DeleteOnClose);
    wx->show();
}

void GrumpyIRC::MainWindow::on_actionLoad_more_items_from_remote_triggered()
{
    this->GetScrollbackManager()->GetCurrentScrollback()->RequestMore(100);
}

void GrumpyIRC::MainWindow::on_actionPreferences_triggered()
{
    PreferencesWin *wx = new PreferencesWin(this);
    wx->setAttribute(Qt::WA_DeleteOnClose);
    wx->show();
}
