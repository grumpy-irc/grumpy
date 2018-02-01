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
#include "proxy.h"
#include "systemcmds.h"
#include "grumpyconf.h"
#include "preferenceswin.h"
#include "scrollbackframe.h"
#include "scrollbacksmanager.h"
#include <QProgressBar>
#include "skin.h"

using namespace GrumpyIRC;

MainWindow *MainWindow::Main;

void MainWindow::Exit()
{
    ScrollbackFrame::ExitThread();
    IRCSession::Exit(CONF->GetQuitMessage());
    GCFG->SetValue("mainwindow_state", QVariant(MainWindow::Main->saveState()));
    GCFG->SetValue("mainwindow_geometry", QVariant(MainWindow::Main->saveGeometry()));
    GCFG->SetValue("systemwx_hide", MainWindow::Main->GetSystem()->IsHidden());
    CONF->Save();
    QApplication::exit(0);
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
    this->windowCount = new QLabel(this);
    this->overviewFrame->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->identFrame->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusFrame->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->ui->statusBar->addPermanentWidget(this->windowCount, 1);
    this->windowCount->setAlignment(Qt::AlignLeft);
    this->ui->statusBar->addPermanentWidget(this->identFrame);
    this->addDockWidget(Qt::LeftDockWidgetArea, this->windowList);
    this->addDockWidget(Qt::RightDockWidgetArea, this->userWidget);
    this->ui->statusBar->addPermanentWidget(this->overviewFrame);
    this->ui->statusBar->addPermanentWidget(this->statusFrame);
    this->progressBar = new QProgressBar(this->ui->statusBar);
    this->ui->statusBar->addPermanentWidget(this->progressBar);
    this->EnableGrumpydContext(false);
    ScrollbacksManager::Global = this->scrollbackWindow;
    if (CONF->FirstRun())
        new Highlighter("$nick");
    this->tray.setIcon(this->windowIcon());
    this->tray.show();
    this->tray.setToolTip("GrumpyChat");
    // Create a system scrollback
    this->systemWindow = this->scrollbackWindow->CreateWindow("System Window", NULL, true, false, NULL, true);
    // Register built-in commands
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.netstat",             (SC_Callback)SystemCmds::Netstat));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.next_session_nick",   (SC_Callback)SystemCmds::NextSessionNick));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.echo",                (SC_Callback)SystemCmds::Echo));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.alias",               (SC_Callback)SystemCmds::Alias));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.link",                (SC_Callback)SystemCmds::GrumpyLink));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("unsecuregrumpyd",            (SC_Callback)SystemCmds::UnsecureGrumpy));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpyd",                    (SC_Callback)SystemCmds::Grumpy));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.quit",                (SC_Callback)SystemCmds::Exit));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.join",                (SC_Callback)SystemCmds::JOIN));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.kick",                (SC_Callback)SystemCmds::KICK));

    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("server",                     (SC_Callback)SystemCmds::Server));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("nick",                       (SC_Callback)SystemCmds::Nick));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("msg",                        (SC_Callback)SystemCmds::SendMessage));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("me",                         (SC_Callback)SystemCmds::Act));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("notice",                     (SC_Callback)SystemCmds::Notice));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("quote",                      (SC_Callback)SystemCmds::RAW));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("raw",                        (SC_Callback)SystemCmds::RAW));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("query",                      (SC_Callback)SystemCmds::Query));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->LongSize = CONF->GetSplitMaxSize();
    CoreWrapper::GrumpyCore->GetCommandProcessor()->SplitLong = CONF->GetSplit();
    this->ui->actionOpen_window->setVisible(false);
    connect(&this->timer, SIGNAL(timeout()), this, SLOT(OnRefresh()));
    this->handler = new LinkHandler();
    this->timer.start(100);
    this->ui->mainToolBar->hide();
    // Try to restore geometry
    this->restoreGeometry(GCFG->GetValue("mainwindow_geometry").toByteArray());
    this->restoreState(GCFG->GetValue("mainwindow_state").toByteArray());
    this->userWidget->hide();
    ScrollbackFrame::UpdateSkins();
    if (GCFG->GetValueAsBool("systemwx_hide", false))
        this->systemWindow->ToggleHide();
    if (!QSslSocket::supportsSsl())
        GRUMPY_ERROR("SSL support is not available, you won't be able to connect using SSL (probably missing OpenSSL libraries?)");
    this->HideProgress();
    this->ui->actionEnable_proxy->setChecked(CONF->UsingProxy());
    if (!CONF->SafeMode)
        this->Execute(CONF->GetAutorun());
}

MainWindow::MainWindow(bool fork, MainWindow *parent)
{
    Q_UNUSED(fork);
    this->ui = new Ui::MainWindow();
    this->isFork = true;
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
    this->ui->actionFavorites->setVisible(false);
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
    this->tray.showMessage(heading, Generic::StripSpecial(text));
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
        this->setWindowTitle(CONF->GetStandardHeader());
    else
        this->setWindowTitle(text);
}

int lastPacketSize = 0;
void MainWindow::UpdateStatus()
{
    this->windowCount->setText(QString::number(Scrollback::ScrollbackList.count()) + " scrollbacks");
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

    // Check if there isn't some grumpyd session transfering large amount of packets
    GrumpydSession::Sessions_Lock.lock();
    bool progress_needed = false;
    foreach (GrumpydSession *session, GrumpydSession::Sessions)
    {
        if (session->IsConnected() && session->IsReceivingLargePacket())
        {
            if (lastPacketSize != session->GetReceivingPacketSize())
            {
                this->SetMaxProgressValue(session->GetReceivingPacketSize());
                lastPacketSize = session->GetReceivingPacketSize();
            }
            progress_needed = true;
            this->SetProgress(session->GetProgress());
            if (!this->progressBar->isVisible())
                this->ShowProgress();
            this->statusFrame->setText("Syncing " + QString::number(session->GetProgress() / 1024) + "kb / " +
                                            QString::number(session->GetReceivingPacketSize() / 1024) + "kb");
            break;
        }
    }
    GrumpydSession::Sessions_Lock.unlock();
    if (!progress_needed)
        this->HideProgress();
}

void MainWindow::OpenUrl(QString url)
{
    if (url.startsWith("http://") || url.startsWith("https://"))
        this->handler->OpenLink(url);
    else if (url.startsWith("ircs://") || url.startsWith("irc://"))
        this->OpenIRCNetworkLink(url);
}

void MainWindow::Execute(QString text)
{
    foreach (QString line, text.split("\n"))
        this->ExecuteLine(line);
}

void MainWindow::HideProgress()
{
    this->progressBar->setVisible(false);
}

void MainWindow::ShowProgress()
{
    this->progressBar->setVisible(true);
}

void MainWindow::SetMaxProgressValue(int max)
{
    this->progressBar->setMaximum(max);
}

void MainWindow::SetProgress(int progress)
{
    this->progressBar->setValue(progress);
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
    if (ssl && !QSslSocket::supportsSsl())
    {
        GRUMPY_ERROR("SSL is not available, can't connect.");
        return;
    }
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
    if (server.UsingSSL() && !QSslSocket::supportsSsl())
    {
        GRUMPY_ERROR("SSL is not available, can't connect.");
        return;
    }
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
    IRCSession *sx = IRCSession::Open(system->GetScrollback(), server, network_name);
    sx->IgnoredNums = CONF->IgnoredNums();
}

void MainWindow::EnableGrumpydContext(bool enable)
{
    this->ui->actionLoad_more_items_from_remote->setVisible(enable);
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

void GrumpyIRC::MainWindow::on_actionProxy_triggered()
{
    Proxy p;
    p.exec();
}

void GrumpyIRC::MainWindow::on_actionEnable_proxy_toggled(bool arg1)
{
    Proxy p;
    p.Enable(arg1);
}
