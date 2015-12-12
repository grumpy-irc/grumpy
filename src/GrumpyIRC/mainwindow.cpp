//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "../libirc/libircclient/channel.h"
#include "../libirc/libircclient/network.h"
#include "../libirc/libirc/serveraddress.h"
#include "../libgp/gp.h"
#include "../libcore/core.h"
#include "../libcore/configuration.h"
#include "../libcore/commandprocessor.h"
#include "../libcore/eventhandler.h"
#include "../libcore/exception.h"
#include "../libcore/generic.h"
#include "../libcore/grumpydsession.h"
#include "../libcore/highlighter.h"
#include "../libcore/ircsession.h"
#include "aboutwin.h"
#include "corewrapper.h"
#include "mainwindow.h"
#include "connectwin.h"
#include "favoriteswin.h"
#include "scrollbacklist.h"
#include "ui_mainwindow.h"
#include "userwidget.h"
#include "linkhandler.h"
#include "grumpyconf.h"
#include "preferenceswin.h"
#include "scrollbackframe.h"
#include "scrollbacksmanager.h"
#include "skin.h"

using namespace GrumpyIRC;

MainWindow *MainWindow::Main;

static void Exit()
{
    ScrollbackFrame::ExitThread();
    IRCSession::Exit(CONF->GetQuitMessage());
    GCFG->SetValue("mainwindow_state", QVariant(MainWindow::Main->saveState()));
    GCFG->SetValue("mainwindow_geometry", QVariant(MainWindow::Main->saveGeometry()));
    CONF->Save();
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

static int SystemCommand_echo(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    MainWindow::Main->GetCurrentScrollbackFrame()->InsertText(args.ParameterLine);
    return 0;
}

static int SystemCommand_Notice(SystemCommand *command, CommandArgs args)
{
    Q_UNUSED(command);
    if (args.Parameters.count() < 1)
    {
        GRUMPY_ERROR(QObject::tr("This command requires some text"));
        return 1;
    }
    Scrollback *sx = MainWindow::Main->GetScrollbackManager()->GetCurrentScrollback()->GetScrollback();
	if (sx->GetType() == ScrollbackType_System)
	{
		if (args.Parameters.count() < 2)
		{
			GRUMPY_ERROR(QObject::tr("This command requires target and some text"));
			return 1;
		}
		QString target = args.Parameters[0];
		QString text = args.ParameterLine.mid(target.size() + 1);
		Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
		sx->GetSession()->SendNotice(sx, target, text);
	}
	else
	{
		sx->GetSession()->SendNotice(sx, args.ParameterLine);
	}
    return 0;
}

static int SystemCommand_SendMessage(SystemCommand *command, CommandArgs args)
{
	if (args.Parameters.size() < 2)
	{
		GRUMPY_ERROR("You need to provide at least 2 parameters");
		return 1;
	}

	// MSG should just echo the message to current window
	QString target = args.Parameters[0];
	QString text = args.ParameterLine.mid(target.size() + 1);
	Scrollback *sx = MainWindow::Main->GetCurrentScrollbackFrame()->GetScrollback();
	sx->GetSession()->SendMessage(sx, target, text);
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
    unsigned long long cr, cs, ur, us;
    cr = session->GetCompressedBytesRcvd();
    cs = session->GetCompressedBytesSent();
    ur = session->GetBytesRcvd();
    us = session->GetBytesSent();
    sx->InsertText("Network stats for this grumpy session:");
    sx->InsertText("-----------------------------------------------------");
    sx->InsertText("Compressed bytes rcvd: " + QString::number(cr));
    sx->InsertText("Compressed bytes sent: " + QString::number(cs));
    sx->InsertText("Bytes rcvd: " + QString::number(ur));
    sx->InsertText("Bytes sent: " + QString::number(us));
    sx->InsertText("-----------------------------------------------------");
    if (cs > 0 && cr > 0)
    {
        double ratio_rcvd = (((double)cr - (double)ur) / (double)ur) * -100;
        double ratio_sent = (((double)cs - (double)us) / (double)us) * -100;
        sx->InsertText("Compression ratio for rcvd: " + QString::number(ratio_rcvd));
        sx->InsertText("Compression ratio for sent: " + QString::number(ratio_sent));
    }
    sx->InsertText("-----------------------------------------------------");
    sx->InsertText("Packets rcvd: " + QString::number(session->GetPacketsRcvd()));
    sx->InsertText("Packets sent: " + QString::number(session->GetPacketsSent()));
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
    if (!scrollback->GetSession() || !scrollback->GetSession()->IsConnected() || scrollback->GetScrollback()->GetType() == ScrollbackType_System)
    {
        GRUMPY_ERROR(QObject::tr("You can only use this command in channel or user windows of connected networks"));
        return 2;
    }
    scrollback->GetSession()->SendAction(scrollback->GetScrollback(), command_args.ParameterLine);
    return 0;
}

static int SystemCommand_Query(SystemCommand *command, CommandArgs command_args)
{
    (void)command;
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
    this->isFork = false;
    this->ui->setupUi(this);
    this->windowList = new ScrollbackList(this);
    this->scrollbackWindow = new ScrollbacksManager(this);
    this->userWidget = new UserWidget(this);
    this->setCentralWidget(this->scrollbackWindow);
    this->statusFrame = new QLabel(this);
    this->identFrame = new QLabel(this);
    this->overviewFrame = new QLabel(this);
    this->overviewFrame->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->identFrame->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusFrame->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->ui->statusBar->addPermanentWidget(this->identFrame);
    this->addDockWidget(Qt::LeftDockWidgetArea, this->windowList);
    this->addDockWidget(Qt::RightDockWidgetArea, this->userWidget);
    this->ui->statusBar->addPermanentWidget(this->overviewFrame);
    this->ui->statusBar->addPermanentWidget(this->statusFrame);
    ScrollbacksManager::Global = this->scrollbackWindow;
    if (CONF->FirstRun())
        new Highlighter("$nick");
    this->tray.setIcon(this->windowIcon());
    this->tray.show();
    this->tray.setToolTip("Grumpy IRC");
    // Create a system scrollback
    this->systemWindow = this->scrollbackWindow->CreateWindow("System Window", NULL, true, false, NULL, true);
    // Register built-in commands
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("quit", (SC_Callback)SystemCommand_Exit));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("server", (SC_Callback)SystemCommand_Server));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("nick", (SC_Callback)SystemCommand_Nick));
	CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("msg", (SC_Callback)SystemCommand_SendMessage));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("me", (SC_Callback)SystemCommand_Act));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("notice", (SC_Callback)SystemCommand_Notice));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.netstat", (SC_Callback)SystemCommand_Netstat));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.next_session_nick", (SC_Callback)SystemCommand_NextSessionNick));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("unsecuregrumpyd", (SC_Callback)SystemCommand_UnsecureGrumpy));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpyd", (SC_Callback)SystemCommand_Grumpy));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("quote", (SC_Callback)SystemCommand_RAW));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("raw", (SC_Callback)SystemCommand_RAW));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("echo", (SC_Callback)SystemCommand_echo));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("query", (SC_Callback)SystemCommand_Query));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->LongSize = CONF->GetSplitMaxSize();
    CoreWrapper::GrumpyCore->GetCommandProcessor()->SplitLong = CONF->GetSplit();
    // Welcome user
    this->ui->actionOpen_window->setVisible(false);
    connect(&this->timer, SIGNAL(timeout()), this, SLOT(OnRefresh()));
    this->handler = new LinkHandler();
    this->timer.start(100);
    this->ui->mainToolBar->hide();
    // Try to restore geometry
    this->restoreGeometry(GCFG->GetValue("mainwindow_geometry").toByteArray());
    this->restoreState(GCFG->GetValue("mainwindow_state").toByteArray());
    this->userWidget->hide();
    this->Execute(CONF->GetAutorun());
}

MainWindow::MainWindow(bool fork, MainWindow *parent)
{
    Q_UNUSED(fork);
    this->ui = new Ui::MainWindow();
    this->isFork = true;
    /*this->userWidget = new UserWidget(this);
    this->setCentralWidget(this->scrollbackWindow);
    this->statusFrame = new QLabel(this);
    this->identFrame = new QLabel(this);
    this->ui->statusBar->addPermanentWidget(this->identFrame);
    this->addDockWidget(Qt::LeftDockWidgetArea, this->windowList);
    this->addDockWidget(Qt::BottomDockWidgetArea, this->syslogWindow);
    this->addDockWidget(Qt::RightDockWidgetArea, this->userWidget);
    this->ui->statusBar->addPermanentWidget(this->statusFrame);
    ScrollbacksManager::Global = this->scrollbackWindow;
    */
    this->ui->setupUi(this);
    this->windowList = parent->windowList;
    this->scrollbackWindow = parent->scrollbackWindow;
    // Create a system scrollback
    this->systemWindow = parent->systemWindow;
    connect(&this->timer, SIGNAL(timeout()), this, SLOT(OnRefresh()));
    this->timer.start(100);
    if (!parent->userWidget->isVisible())
        this->userWidget->hide();
    // Try to restore geometry
    this->restoreGeometry(GCFG->GetValue("mainwindow_geometry").toByteArray());
    this->restoreState(GCFG->GetValue("mainwindow_state").toByteArray());
}

MainWindow::~MainWindow()
{
    ScrollbacksManager::Global = NULL;
    delete this->ui;
    delete this->handler;
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

void MainWindow::WriteToCurrentWindow(QString text, ScrollbackItemType item)
{
    if (!this->scrollbackWindow)
        return;

    if (!this->scrollbackWindow->GetCurrentScrollback())
        return;

    this->scrollbackWindow->GetCurrentScrollback()->InsertText(text, item);
}

ScrollbackFrame *MainWindow::GetSystem()
{
    return this->systemWindow;
}

ScrollbackFrame *MainWindow::GetCurrentScrollbackFrame()
{
    return this->GetScrollbackManager()->GetCurrentScrollback();
}

void MainWindow::Notify(QString heading, QString text)
{
    this->tray.showMessage(heading, text);
}

UserWidget *MainWindow::GetUsers()
{
    return this->userWidget;
}

void MainWindow::Fork()
{
    MainWindow *window = new MainWindow(true, this);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
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
    QString mode = this->GetCurrentScrollbackFrame()->GetLocalUserMode();
    if (!mode.isEmpty())
        mode = "(" + mode + ")";
    if (!self_ident)
        this->identFrame->setText("");
    else
        this->identFrame->setText(self_ident->ToString() + " " + mode);
    QString extra = this->GetCurrentScrollbackFrame()->GetTitle();
    if (this->GetCurrentScrollbackFrame()->IsChannel() && this->GetCurrentScrollbackFrame()->GetSession())
    {
        libircclient::Channel *channel = this->GetCurrentScrollbackFrame()->GetSession()->GetChannel(this->GetCurrentScrollbackFrame()->GetScrollback());
        if (channel && !channel->GetMode().IsEmpty())
            extra += QString(" <b>(") + channel->GetMode().ToString() + QString(")</b>");
    }
    this->overviewFrame->setText(extra);
}

void MainWindow::OpenUrl(QString url)
{
    this->handler->OpenLink(url);
}

void MainWindow::Execute(QString text)
{
    foreach (QString line, text.split("\n"))
        this->ExecuteLine(line);
}

void MainWindow::ExecuteLine(QString line)
{
    if (line.isEmpty())
        return;

    line = this->processInput(line);
    CoreWrapper::GrumpyCore->GetCommandProcessor()->ProcessText(line, this->GetCurrentScrollbackFrame()->GetScrollback(), true);
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
    this->GetScrollbackManager()->GetCurrentScrollback()->RequestMore(1200);
}

void GrumpyIRC::MainWindow::on_actionPreferences_triggered()
{
    PreferencesWin *wx = new PreferencesWin(this);
    wx->setAttribute(Qt::WA_DeleteOnClose);
    wx->show();
}

void GrumpyIRC::MainWindow::on_actionOpen_window_triggered()
{
    this->Fork();
}

void GrumpyIRC::MainWindow::on_actionFavorites_triggered()
{
    FavoritesWin *favorites = new FavoritesWin(this);
    favorites->setAttribute(Qt::WA_DeleteOnClose);
    favorites->show();
}

void GrumpyIRC::MainWindow::on_actionToggle_secret_triggered()
{
    this->GetCurrentScrollbackFrame()->ToggleSecure();
}

QString MainWindow::processInput(QString text)
{
    if (text.contains("$grumpy.version"))
        text = text.replace("$grumpy.version", GCFG->GetVersion());
    return text;
}
