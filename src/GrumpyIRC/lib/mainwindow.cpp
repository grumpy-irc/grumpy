//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "aboutwin.h"
#include "corewrapper.h"
#include "mainwindow.h"
#include "connectwin.h"
#include "favoriteswin.h"
#include "scrollbacklist.h"
#include "ui_mainwindow.h"
#include "userwidget.h"
#include "messagebox.h"
#include "linkhandler.h"
#include "proxy.h"
#include "systemcmds.h"
#include "grumpyconf.h"
#include "preferenceswin.h"
#include "hooks.h"
#include "scrollbackframe.h"
#include "script_engine/scriptingmanager.h"
#include "scrollbacksmanager.h"
#include "skin.h"
#include <QProgressBar>
#include <QFileDialog>
#include <libirc/libircclient/channel.h>
#include <libirc/libircclient/network.h>
#include <libirc/libirc/serveraddress.h>
#include <libgp/gp.h>
#include <libcore/core.h>
#include <libcore/configuration.h>
#include <libcore/eventhandler.h>
#include <libcore/exception.h>
#include <libcore/generic.h>
#include <libcore/grumpydsession.h>
#include <libcore/highlighter.h>
#include <libcore/grumpyscript.h>
#include <libcore/ircsession.h>

using namespace GrumpyIRC;

MainWindow *MainWindow::Main;

void MainWindow::Exit()
{
    UiHooks::OnExit();
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
    this->timerAway = nullptr;
    Main = this;
    this->isAway = false;
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
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.unalias",             (SC_Callback)SystemCmds::UnAlias));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.link",                (SC_Callback)SystemCmds::GrumpyLink));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("unsecuregrumpyd",            (SC_Callback)SystemCmds::UnsecureGrumpy));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpyd",                    (SC_Callback)SystemCmds::Grumpy));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.uptime",              (SC_Callback)SystemCmds::Uptime));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.quit",                (SC_Callback)SystemCmds::Exit));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.join",                (SC_Callback)SystemCmds::JOIN));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.kick",                (SC_Callback)SystemCmds::KICK));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.topic",               (SC_Callback)SystemCmds::Topic));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.kickban",             (SC_Callback)SystemCmds::KickBan));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.ban",                 (SC_Callback)SystemCmds::Ban));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.script.load",         (SC_Callback)SystemCmds::Script));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.script.unload",       (SC_Callback)SystemCmds::RemoveScript));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.script.list",         (SC_Callback)SystemCmds::ScriptList));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.script.reload",       (SC_Callback)SystemCmds::ScriptReload));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("grumpy.script.reload.all",   (SC_Callback)SystemCmds::ScriptReloadAll));

    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("server",                     (SC_Callback)SystemCmds::Server));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("nick",                       (SC_Callback)SystemCmds::Nick));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("msg",                        (SC_Callback)SystemCmds::SendMessage));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("me",                         (SC_Callback)SystemCmds::Act));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("notice",                     (SC_Callback)SystemCmds::Notice));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("quote",                      (SC_Callback)SystemCmds::RAW));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("raw",                        (SC_Callback)SystemCmds::RAW));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("query",                      (SC_Callback)SystemCmds::Query));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->RegisterCommand(new SystemCommand("ctcp",                       (SC_Callback)SystemCmds::CTCP));
    CoreWrapper::GrumpyCore->GetCommandProcessor()->AutoReduceMsgSize = CONF->GetAutoReduceMaxSendSize();
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
    this->UpdateSkin();
    if (GCFG->GetValueAsBool("systemwx_hide", false))
        this->systemWindow->ToggleHide();
    if (!QSslSocket::supportsSsl())
        GRUMPY_ERROR("SSL support is not available, you won't be able to connect using SSL (probably missing OpenSSL libraries?)");
    this->HideProgress();
    this->ui->actionEnable_proxy->setChecked(CONF->UsingProxy());
    Proxy::Init();
    this->SetupAutoAway();

    if (!CONF->SafeMode)
        this->Execute(CONF->GetAutorun());

    UiHooks::OnMainWindowStart();
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
    delete this->timerAway;
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
    ScrollbackFrame *current_scrollback = this->GetScrollbackManager()->GetCurrentScrollback();
    int synced = current_scrollback->GetSynced();
    int total = current_scrollback->GetItems();
    this->statusFrame->setText("Items (synced/total): " + QString::number(synced) + " / " + QString::number(total));
    libircclient::User *self_ident = current_scrollback->GetIdentity();
    QString mode = current_scrollback->GetLocalUserMode();
    if (!mode.isEmpty())
        mode = "(" + mode + ")";
    if (!self_ident)
        this->identFrame->setText("");
    else
        this->identFrame->setText(self_ident->ToString() + " " + mode);
    QString extra = current_scrollback->GetTitle();
    if (current_scrollback->IsChannel() && current_scrollback->GetSession())
    {
        libircclient::Channel *channel = current_scrollback->GetSession()->GetChannel(current_scrollback->GetScrollback());
        if (channel && !channel->GetMode().IsEmpty())
            extra += QString(" <b>(") + channel->GetMode().ToString() + QString(")</b>");
    }
    if (!current_scrollback->IsGrumpy() && current_scrollback->GetNetwork())
    {
        extra += " lag: " + QString::number(current_scrollback->GetNetwork()->GetLag()) + "ms";
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
    else
        GRUMPY_DEBUG("Unknown url scheme: " + url, 1);
}

void MainWindow::UpdateSkin()
{
    ScrollbackFrame::UpdateSkins();
    float opacity = Skin::GetCurrent()->Opacity;
    opacity = opacity / 100;
    this->setWindowOpacity(opacity);
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

    line = GrumpyScript::ReplaceStdVars(line);
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
    libirc::ServerAddress network(link);
    if (!network.IsValid())
    {
        GRUMPY_ERROR("Invalid link to IRC network: " + link);
    } else
    {
        this->OpenServer(network);
    }
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
    // Nick override is empty here as it's obtained from server info
    IRCSession *sx = IRCSession::Open(system->GetScrollback(), server, network_name, "", CONF->GetIdent(), CONF->GetName(), CONF->GetEncoding());
    sx->IgnoredNums = CONF->IgnoredNums();
}

void MainWindow::EnableGrumpydContext(bool enable)
{
    this->ui->actionLoad_more_items_from_remote->setVisible(enable);
}

void MainWindow::SetupAutoAway()
{
    bool enable = CONF->GetAutoAway();
    if (this->timerAway == nullptr && enable)
    {
        this->timerAway = new QTimer();
        connect(this->timerAway, SIGNAL(timeout()), this, SLOT(OnAutoAway()));
    }
    if (this->timerAway != nullptr && !enable)
    {
        this->timerAway->stop();
        delete this->timerAway;
        this->timerAway = nullptr;
    }
    if (this->timerAway != nullptr)
    {
        // This resets the timer. This function is called only when grumpy start or time changes.
        this->timerAway->setInterval(CONF->GetAutoAwayTime() * 1000);
        this->ResetAutoAway();
    }
}

void MainWindow::ResetAutoAway()
{
    if (this->timerAway == nullptr)
        return;

    if (this->isAway)
    {
        this->isAway = false;
        this->systemWindow->InsertText("You were automatically marked as no longer away.");
        GrumpydSession::Sessions_Lock.lock();
        foreach (NetworkSession *session, GrumpydSession::Sessions)
        {
            session->UnsetAway();
        }
        GrumpydSession::Sessions_Lock.unlock();
        IRCSession::Sessions_Lock.lock();
        foreach (IRCSession *session, IRCSession::Sessions)
            session->UnsetAway();
        IRCSession::Sessions_Lock.unlock();
    }
    this->timerAway->stop();
    this->timerAway->start();
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

void MainWindow::on_actionConnect_triggered()
{
    ConnectWin *wx = new ConnectWin(this);
    wx->show();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutWin *wx = new AboutWin(this);
    wx->show();
}

void MainWindow::on_actionLoad_more_items_from_remote_triggered()
{
    this->GetScrollbackManager()->GetCurrentScrollback()->RequestMore(1200);
}

void MainWindow::on_actionPreferences_triggered()
{
    PreferencesWin *wx = new PreferencesWin(this);
    wx->setAttribute(Qt::WA_DeleteOnClose);
    wx->show();
}

void MainWindow::on_actionOpen_window_triggered()
{
    this->Fork();
}

void MainWindow::on_actionFavorites_triggered()
{
    FavoritesWin *favorites = new FavoritesWin(this);
    favorites->setAttribute(Qt::WA_DeleteOnClose);
    favorites->show();
}

void MainWindow::on_actionToggle_secret_triggered()
{
    this->GetCurrentScrollbackFrame()->ToggleSecure();
}

void MainWindow::on_actionProxy_triggered()
{
    Proxy p;
    p.exec();
}

void MainWindow::on_actionEnable_proxy_toggled(bool arg1)
{
    Proxy p;
    p.Enable(arg1);
}

void MainWindow::OnAutoAway()
{
    if (!CONF->GetAutoAway())
        return;

    this->systemWindow->InsertText("You are now automatically being marked as away. You can change this in preferences.");
    this->isAway = true;
    this->timerAway->stop();

    // Change all connections to away now
    GrumpydSession::Sessions_Lock.lock();
    foreach (NetworkSession *session, GrumpydSession::Sessions)
    {
        session->SetAway(CONF->GetAutoAwayMsg());
    }
    GrumpydSession::Sessions_Lock.unlock();
    IRCSession::Sessions_Lock.lock();
    foreach (IRCSession *session, IRCSession::Sessions)
        session->SetAway(CONF->GetAutoAwayMsg());
    IRCSession::Sessions_Lock.unlock();
}

void MainWindow::on_actionExport_to_html_triggered()
{
    QString file = QFileDialog::getSaveFileName(this, "Export to html", CONF->GetLastSavePath(), "HTML (*.htm *.html);;All files (*.*)");
    if (file.isEmpty())
        return;
    QFile f(file);
    QFileInfo fi(file);
    CONF->SetLastSavePath(fi.absolutePath());
    if (!f.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        MessageBox::Display("html-export-fail", "Error", "Unable to write to " + file, this);
        return;
    }
    QTextStream s(&f);
    s << this->GetCurrentScrollbackFrame()->ToHtml();
    f.close();
}

void MainWindow::on_actionExport_to_plain_text_triggered()
{
    QString file = QFileDialog::getSaveFileName(this, "Export to plain text", CONF->GetLastSavePath(), "Text (*.txt);;All files (*.*)");
    if (file.isEmpty())
        return;
    QFile f(file);
    QFileInfo fi(file);
    CONF->SetLastSavePath(fi.absolutePath());
    if (!f.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        MessageBox::Display("text-export-fail", "Error", "Unable to write to " + file, this);
        return;
    }
    QTextStream s(&f);
    s << this->GetCurrentScrollbackFrame()->ToString();
    f.close();
}

void MainWindow::on_actionScript_manager_triggered()
{
    ScriptingManager script;
    script.exec();
}
