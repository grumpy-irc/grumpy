//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include <QScrollBar>
#include <QMenu>
#include <QStringBuilder>
#include "mainwindow.h"
#include "corewrapper.h"
#include "grumpyconf.h"
#include "channelwin.h"
#include "scriptwin.h"
#include "hooks.h"
#include "scrollbacklist_node.h"
#include "skin.h"
#include "scrollbacklist.h"
#include "scrollbackframe.h"
#include "scrollbacksmanager.h"
#include "userframe.h"
#include "messagebox.h"
#include "inputbox.h"
#include "ui_scrollbackframe.h"
#include <libcore/eventhandler.h>
#include <libcore/exception.h>
#include <libcore/configuration.h>
#include <libcore/generic.h>
#include <libcore/profiler.h>
#include <libcore/highlighter.h>
#include <libcore/ircsession.h>
#include <libcore/grumpydsession.h>
#include <libcore/networksession.h>
#include <libcore/core.h>
#include <libirc/libircclient/user.h>

using namespace GrumpyIRC;

QList<ScrollbackFrame*> ScrollbackFrame::ScrollbackFrames;
QMutex ScrollbackFrame::ScrollbackFrames_m;
ScrollbackFrame_WorkerThread *ScrollbackFrame::WorkerThread = nullptr;
irc2htmlcode::Parser ScrollbackFrame::parser;

void ScrollbackFrame::UpdateSkins()
{
    parser.TimeColor = Skin::GetCurrent()->Timestamp.name();
    parser.LinkColor = Skin::GetCurrent()->LinkColor.name();
    parser.TextColor = Skin::GetCurrent()->TextColor.name();
    parser.UserColor = Skin::GetCurrent()->UserColor.name();

    QHash<unsigned int, QColor> colors = Skin::GetCurrent()->Colors;
    parser.TextColors.clear();

    foreach (unsigned int color_n, colors.keys())
        parser.TextColors.insert(color_n, colors[color_n].name());

    foreach (ScrollbackFrame *frame, ScrollbackFrame::ScrollbackFrames)
        frame->UpdateSkin();
}

void ScrollbackFrame::ExitThread()
{
    WorkerThread->IsRunning = false;
    while (!WorkerThread->IsFinished)
    {
        WorkerThread->Sleep(100);
    }
    //WorkerThread->exit();
}

void ScrollbackFrame::InitializeThread()
{
    WorkerThread = new ScrollbackFrame_WorkerThread();
    WorkerThread->start();
}

ScrollbackFrame::ScrollbackFrame(ScrollbackFrame *parentWindow, QWidget *parent, Scrollback *_scrollback, bool is_system) : QFrame(parent), GrumpyObject("ScrollbackFrame"), ui(new Ui::ScrollbackFrame)
{
    ScrollbackFrames_m.lock();
    ScrollbackFrames.append(this);
    ScrollbackFrames_m.unlock();
    this->IsSystem = is_system;
    this->textEdit = new STextBox(this);
    this->ShowJQP = true;
    this->ui->setupUi(this);
    this->inputBox = new InputBox(this);
    this->ui->splitter->addWidget(this->textEdit);
    this->ui->splitter->addWidget(this->inputBox);
    this->LastMenuTooltipUpdate = QDateTime::currentDateTime().addSecs(-50);
    this->_parent = parentWindow;
    this->TreeNode = nullptr;
    this->needsRefresh = false;
    this->isClean = true;
    connect(this->textEdit, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(Menu(QPoint)));
    if (_scrollback == nullptr)
        this->scrollback = new Scrollback();
    else
        this->scrollback = _scrollback;
    connect(this->scrollback, SIGNAL(Event_NetworkModified(libircclient::Network*)), this, SLOT(NetworkChanged(libircclient::Network*)));
    connect(this->scrollback, SIGNAL(Event_ChangedDeadStatus()), this, SLOT(OnDead()));
    connect(this->scrollback, SIGNAL(Event_UserInserted(libircclient::User*, bool)), this, SLOT(UserList_Insert(libircclient::User*, bool)));
    connect(this->scrollback, SIGNAL(Event_Reload()), this, SLOT(Refresh()));
    connect(this->scrollback, SIGNAL(Event_UserAltered(QString,libircclient::User*)), this, SLOT(UserList_Alter(QString,libircclient::User*)));
    connect(this->scrollback, SIGNAL(Event_UserRemoved(QString, bool)), this, SLOT(UserList_Remove(QString, bool)));
    connect(this->scrollback, SIGNAL(Event_InsertText(ScrollbackItem&)), this, SLOT(_insertText_(ScrollbackItem&)));
    connect(this->scrollback, SIGNAL(Event_Closed()), this, SLOT(OnClosed()));
    connect(this->scrollback, SIGNAL(Event_UserListBulkDone()), this, SLOT(OnFinishSortBulk()));
    connect(this->scrollback, SIGNAL(Event_Show()), this, SLOT(OnShow()));
    connect(this->scrollback, SIGNAL(Event_Hide()), this, SLOT(OnHidden()));
    connect(this->scrollback, SIGNAL(Event_UserRefresh(libircclient::User*)), this, SLOT(UserList_Refresh(libircclient::User*)));
    connect(this->scrollback, SIGNAL(Event_Resync()), this, SLOT(OnDead()));
    connect(this->scrollback, SIGNAL(Event_StateModified()), this, SLOT(OnState()));
    connect(this->textEdit, SIGNAL(Event_Link(QString)), this, SLOT(OnLink(QString)));
    connect(&this->scroller, SIGNAL(timeout()), this, SLOT(OnScroll()));
    //this->opacityEffect = new QGraphicsOpacityEffect();
    this->UpdateSkin();
    //this->textEdit->setGraphicsEffect(this->opacityEffect);
    //this->textEdit->setAutoFillBackground(true);
    this->maxItems = 200;
    // System scrollback has no user list
    if (is_system)
        this->userFrame = nullptr;
    else
        this->userFrame = new UserFrame(this);
    this->Muted = false;
    this->precachedNetwork = nullptr;
    this->isVisible = false;
    this->currentScrollbar = 0;

    //this->scroller.start(200);
}

ScrollbackFrame::~ScrollbackFrame()
{
    ScrollbackFrames_m.lock();
    ScrollbackFrames.removeOne(this);
    ScrollbackFrames_m.unlock();
    delete this->scrollback;
    delete this->userFrame;
    //! \todo Handle deletion of TreeNode from list of scbs
    //delete this->TreeNode;
    delete this->ui;
    //delete this->opacityEffect;
}

QString ScrollbackFrame::GetWindowName() const
{
    return this->_name;
}

void ScrollbackFrame::InsertText(const QString &text, ScrollbackItemType item)
{
    if (!this->scrollback)
        throw new NullPointerException("this->scrollback", BOOST_CURRENT_FUNCTION);

    // Write
    this->scrollback->InsertText(text, item);
}

void ScrollbackFrame::InsertText(const ScrollbackItem &item)
{
    this->scrollback->InsertText(item);
}

static QString FormatAction(const libircclient::User &user, const QString &action, bool full_user)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    QString result = CONF->GetActionFormat();
    QString name;
    // we don't want to display full user information for simple actions and so on
    if (full_user)
        name = user.ToString();
    else
        name = user.GetNick();
    result.replace("$nick", name);
    result.replace("$string", action);
    return result;
}

static QString ItemToHTMLString(const ScrollbackItem &item, bool highlighted)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    // Render the text according to our formatting
    //! \todo This needs to be precached otherwise we need to build this string every fucking time
    QString format_string = CONF->GetLineFormat();
    QString text = item.GetText();
    QString user = item.GetUser().GetNick();
    bool system = false;
    QColor color = Skin::GetCurrent()->TextColor;
    switch (item.GetType())
    {
        case ScrollbackItemType_Act:
            //result = FormatAction(item.GetUser(), item.GetText(), false);
            format_string.replace("$string", CONF->GetActionFormat());
            break;
        case ScrollbackItemType_Join:
            system = true;
            format_string.replace("$string", CONF->GetActionFormat());
            user = item.GetUser().ToString();
            text = "joined channel";
            //result = FormatAction(item.GetUser(), "joined channel", true);
            break;
        case ScrollbackItemType_Part:
            system = true;
            format_string.replace("$string", CONF->GetActionFormat());
            user = item.GetUser().ToString();
            if (item.GetText().isEmpty())
                text = "left channel";
            else
                text = "left channel (" + item.GetText() + ")";
            break;
        case ScrollbackItemType_Quit:
            system = true;
            format_string.replace("$string", CONF->GetActionFormat());
            user = item.GetUser().ToString();
            text = "quit (" + item.GetText() + ")";
            break;
        case ScrollbackItemType_Kick:
            system = true;
            format_string.replace("$string", CONF->GetActionFormat());
            break;
        case ScrollbackItemType_Mode:
            system = true;
            format_string.replace("$string", CONF->GetActionFormat());
            text = "set mode " + item.GetText();
            break;
        case ScrollbackItemType_Nick:
            system = true;
            format_string.replace("$string", CONF->GetActionFormat());
            text = "changed nick to " + item.GetText();
            break;
        case ScrollbackItemType_Notice:
            format_string.replace("$string", CONF->GetNoticeFormat());
            break;
        case ScrollbackItemType_Message:
            format_string.replace("$string", CONF->GetMessageFormat());
            break;
        case ScrollbackItemType_System:
        case ScrollbackItemType_Unknown:
            //result = item.GetText();
            break;
        case ScrollbackItemType_SystemError:
            color = Skin::GetCurrent()->Error;
            break;
        case ScrollbackItemType_SystemWarning:
            color = Skin::GetCurrent()->Warning;
            break;
        case ScrollbackItemType_Topic:
            system = true;
            format_string.replace("$string", CONF->GetActionFormat());
            text = "changed topic to: " + item.GetText();
            break;
    }
    //format_string.replace("$string", result);
    if (highlighted)
        color = Skin::GetCurrent()->HighligtedColor;
    else if (item.GetType() == ScrollbackItemType_System)
        color = Skin::GetCurrent()->SystemColor;
    else if (system)
        color = Skin::GetCurrent()->SystemInfo;
    irc2htmlcode::FormattedItem results = ScrollbackFrame::parser.Process(format_string, item.GetTime(), user, text, color.name());
    return results.source;
}

static QString ItemToTXTString(const ScrollbackItem &item)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    // Render the text according to our formatting
    //! \todo This needs to be precached otherwise we need to build this string every fucking time
    QString format_string = CONF->GetLineFormat();
    QString text = item.GetText();
    QString user = item.GetUser().GetNick();
    switch (item.GetType())
    {
        case ScrollbackItemType_Act:
            //result = FormatAction(item.GetUser(), item.GetText(), false);
            format_string.replace("$string", CONF->GetActionFormat());
            break;
        case ScrollbackItemType_Join:
            format_string.replace("$string", CONF->GetActionFormat());
            user = item.GetUser().ToString();
            text = "joined channel";
            //result = FormatAction(item.GetUser(), "joined channel", true);
            break;
        case ScrollbackItemType_Part:
            format_string.replace("$string", CONF->GetActionFormat());
            user = item.GetUser().ToString();
            if (item.GetText().isEmpty())
                text = "left channel";
            else
                text = "left channel (" + item.GetText() + ")";
            break;
        case ScrollbackItemType_Quit:
            format_string.replace("$string", CONF->GetActionFormat());
            user = item.GetUser().ToString();
            text = "quit (" + item.GetText() + ")";
            break;
        case ScrollbackItemType_Kick:
            format_string.replace("$string", CONF->GetActionFormat());
            break;
        case ScrollbackItemType_Mode:
            format_string.replace("$string", CONF->GetActionFormat());
            text = "set mode " + item.GetText();
            break;
        case ScrollbackItemType_Nick:
            format_string.replace("$string", CONF->GetActionFormat());
            text = "changed nick to " + item.GetText();
            break;
        case ScrollbackItemType_Notice:
            format_string.replace("$string", CONF->GetNoticeFormat());
            break;
        case ScrollbackItemType_Message:
            format_string.replace("$string", CONF->GetMessageFormat());
            break;
        case ScrollbackItemType_System:
        case ScrollbackItemType_Unknown:
            //result = item.GetText();
            break;
        case ScrollbackItemType_SystemError:
            break;
        case ScrollbackItemType_SystemWarning:
            break;
        case ScrollbackItemType_Topic:
            format_string.replace("$string", CONF->GetActionFormat());
            text = "changed topic to: " + item.GetText();
            break;
    }
    QString results = format_string;
    results.replace("$time", item.GetTime().toString());
    results.replace("$nick", user);
    results.replace("$string", text);
    return results;
}

static QString ItemToPlainText(const ScrollbackItem &item)
{
    QString result;
    switch (item.GetType())
    {
        case ScrollbackItemType_Act:
            result = "* " + item.GetUser().GetNick() + " " + item.GetText();
            break;
        case ScrollbackItemType_Message:
            result = item.GetUser().GetNick() + ": " + item.GetText();
            break;
        default:
            result = item.GetText();
            break;
    }
    return result;
}

void ScrollbackFrame::_insertText_(ScrollbackItem &item)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (!this->ShowJQP)
    {
        switch(item.GetType())
        {
            case ScrollbackItemType_Join:
            case ScrollbackItemType_Nick:
            case ScrollbackItemType_Quit:
            case ScrollbackItemType_Part:
                this->logItem(item);
                return;

            default:
                break;
        }
    }
    int highlighted = GRUMPY_H_NOT;
    bool is_opening = this->Refreshing;
    if (!is_opening && Generic::IsGrumpyd(this->scrollback))
        is_opening = (dynamic_cast<GrumpydSession*>(this->scrollback->GetSession()))->IsOpening;
    if (Highlighter::IsMatch(&item, this->GetNetwork()))
    {
        if (!is_opening && !this->Muted)
            UiHooks::OnScrollbackItemHighlight(this, &item);
        highlighted = GRUMPY_H_YES;
    }
    if (!this->IsVisible())
    {
        if (!is_opening && (item.GetType() == ScrollbackItemType_Act || item.GetType() == ScrollbackItemType_Message) && this->scrollback->GetPropertyAsBool("notify"))
            MainWindow::Main->Notify(this->GetTitle(), ItemToPlainText(item));
        this->unwritten_m.lock();
        while (this->unwritten.size() > this->maxItems)
        {
            this->unwritten.removeAt(0);
            if (!this->isClean)
                this->clearItems();
        }
        this->unwritten.append(item);
        this->unwritten_m.unlock();
        this->needsRefresh = true;
    } else
    {
        this->writeText(item, highlighted);
    }
    this->logItem(item);
}

void ScrollbackFrame::UserList_Insert(libircclient::User *ux, bool bulk)
{
    if (this->userFrame == nullptr)
        throw new NullPointerException("this->userFrame", BOOST_CURRENT_FUNCTION);
    this->userFrame->InsertUser(ux, bulk);
}

void ScrollbackFrame::UserList_Refresh(libircclient::User *ux)
{
    if (this->userFrame == nullptr)
        throw new NullPointerException("this->userFrame", BOOST_CURRENT_FUNCTION);
    this->userFrame->RefreshUser(ux);
}

void ScrollbackFrame::OnState()
{
    this->UpdateColor();
    if (this->TreeNode)
        this->TreeNode->UpdateToolTip();
}

void ScrollbackFrame::UserList_Remove(const QString &user, bool bulk)
{
    if (this->userFrame == nullptr)
        throw new NullPointerException("this->userFrame", BOOST_CURRENT_FUNCTION);
    this->userFrame->RemoveUser(user);
}

void ScrollbackFrame::UserList_Alter(const QString &old, libircclient::User *us)
{
    if (this->userFrame == nullptr)
        throw new NullPointerException("this->userFrame", BOOST_CURRENT_FUNCTION);
    this->userFrame->RemoveUser(old);
    this->userFrame->InsertUser(us, false);
}

void ScrollbackFrame::OnDead()
{
    this->UpdateIcon();
    if (this->TreeNode)
        this->TreeNode->UpdateToolTip();
}

void ScrollbackFrame::OnFinishSortBulk()
{
    if (this->userFrame == nullptr)
        throw new NullPointerException("this->userFrame", BOOST_CURRENT_FUNCTION);
    this->userFrame->NeedsUpdate = true;
    this->userFrame->Sort();
}

void ScrollbackFrame::OnLink(const QString &url)
{
    if (!url.contains("://"))
            return;
    QString scheme = url.mid(0, url.indexOf("://"));
    if (scheme == "irc_join")
    {
        this->TransferRaw("JOIN " + url.mid(url.indexOf("://") + 3));
    } else
    {
        MainWindow::Main->OpenUrl(url);
    }
}

void ScrollbackFrame::OnScroll()
{
    if (this->scroller.interval() != GRUMPY_SCROLLER_TIME_WAIT)
        this->scroller.setInterval(GRUMPY_SCROLLER_TIME_WAIT);
    if (!this->IsVisible())
        return;
    if ((scrollback_id_t)this->GetSynced() == this->GetItems())
        return;
    if (this->textEdit->verticalScrollBar()->value() == 0)
    {
        this->RequestMore(200);
        // sleep for some longer time
        this->scroller.setInterval(GRUMPY_SCROLLER_TIME_WAIT * 10);
    }
}

void ScrollbackFrame::Refresh()
{
    this->textEdit->Clear();
    this->buffer.clear();
    this->Refreshing = true;
    foreach (ScrollbackItem item, this->scrollback->GetItems())
        this->_insertText_(item);
    this->Refreshing = false;
    this->UpdateIcon();
    this->UpdateColor();
    //if (this->currentScrollbar <= this->textEdit->verticalScrollBar()->maximum())
    //    this->textEdit->verticalScrollBar()->setValue(this->currentScrollbar);
}

void ScrollbackFrame::Menu(QPoint pn)
{
    QPoint globalPos = this->textEdit->viewport()->mapToGlobal(pn);
    UiHooks::OnInput();
    QMenu Menu;
    // Items
    QAction *menuCopy = new QAction(QObject::tr("Copy"), &Menu);
    Menu.addAction(menuCopy);
    QAction *menuRetrieveTopic = nullptr;
    QAction *menuChanSet = nullptr;
    QAction *menuViewJQP = nullptr;
    if (this->IsChannel())
    {
        menuRetrieveTopic = new QAction(QObject::tr("Retrieve topic"), &Menu);
        Menu.addAction(menuRetrieveTopic);
        menuChanSet = new QAction(QObject::tr("Channel settings"), &Menu);
        Menu.addAction(menuChanSet);
        menuViewJQP = new QAction(QObject::tr("Display join / quit / part / nick"), &Menu);
        Menu.addAction(menuViewJQP);
        menuViewJQP->setCheckable(true);
        menuViewJQP->setChecked(this->ShowJQP);
    }

    QAction* selectedItem = Menu.exec(globalPos);
    if (!selectedItem)
        return;
    if (selectedItem == menuCopy)
    {
        this->textEdit->copy();
    } else if (selectedItem == menuRetrieveTopic)
    {
        if (!this->GetSession())
            return;
        this->GetSession()->SendRaw(this->GetScrollback(), "TOPIC " + this->GetScrollback()->GetTarget());
    } else if (selectedItem == menuChanSet)
    {
        if (!this->GetSession() || !this->GetScrollback()->GetNetwork())
            return;

        libircclient::Network *network = this->GetSession()->GetNetwork(this->scrollback);
        libircclient::Channel *channel = this->GetSession()->GetChannel(this->scrollback);

        if (!channel || !network)
            return;

        ChannelWin *window = new ChannelWin(this->GetSession(), network, channel, this);
        window->setAttribute(Qt::WA_DeleteOnClose);
        // We need to close this window when the session gets deleted otherwise we could access deleted memory
        connect(this->GetSession(), SIGNAL(Event_Deleted()), window, SLOT(close()));
        window->show();
    } else if (selectedItem == menuViewJQP)
    {
        this->ShowJQP = !this->ShowJQP;
        this->Refresh();
    }
}

void ScrollbackFrame::OnHidden()
{
    this->SetHidden();
}

void ScrollbackFrame::OnShow()
{
    this->UnsetHidden();
}

void ScrollbackFrame::OnClosed()
{
    // The wrapped scrollback is being closed, we must unregister this frame and delete it,
    // before we do that, we need to reset the pointer to scrollback, because the destructor
    // of this class naturally tries to delete the scrollback, which would fail as it
    // would already be deleted by then, this event is called from destructor of scrollback,
    // so calling delete on it would have unexpectable results
    this->scrollback = nullptr;
    ScrollbacksManager::Global->DestroyWindow(this);
}

void ScrollbackFrame::NetworkChanged(libircclient::Network *network)
{
    this->userFrame->SetNetwork(network);
    this->precachedNetwork = network;
    if (this->TreeNode)
        this->TreeNode->UpdateToolTip();
}

void ScrollbackFrame::clearItems()
{
    this->isClean = true;
    this->buffer.clear();
    this->textEdit->Clear();
}

void ScrollbackFrame::writeText(ScrollbackItem &item, int highlighted)
{
    bool is_hg = false;
    if (!highlighted)
        is_hg = Highlighter::IsMatch(&item, this->GetNetwork());
    else if (highlighted == GRUMPY_H_YES)
        is_hg = true;
    QString line = ItemToHTMLString(item, is_hg);
    this->buffer += line;
    this->textEdit->AppendHtml(line);
    this->isClean = false;
}

QString ScrollbackFrame::itemsToString(QList<ScrollbackItem> items)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    bool is_first = true;
    QString temp;
    while (!items.empty())
    {
        ScrollbackItem item = items.at(0);
        bool is_hg = Highlighter::IsMatch(&item, this->GetNetwork());
        if (!is_first)
            temp.append("<br>\n");
        is_first = false;
        temp.append(ItemToHTMLString(item, is_hg));
        //this->writeText(this->unwritten.at(0));
        items.removeAt(0);
    }
    return temp;
}

void ScrollbackFrame::logItem(const ScrollbackItem &item)
{
    if (this->loggingDisabled)
        return;
    if (!this->IsGrumpy() && CONF->GetContinuousLoggingEnabled())
    {
        QString target_dir = this->cachedLogDirPath;
        if (target_dir.isEmpty())
        {
            // Logging to text files is enabled, construct the path and verify it's actually existing
            target_dir = CONF->GetContinuousLoggingPath() + Generic::String2ValidPath(this->GetWindowName());
            if (!QDir().exists(target_dir))
            {
                GRUMPY_DEBUG("Creating path: " + target_dir, 1);
                if (!QDir().mkpath(target_dir))
                {
                    GRUMPY_ERROR("Unable to create path: " + target_dir);
                    this->loggingDisabled = true;
                    return;
                }
            }
            this->cachedLogDirPath = target_dir + QDir().separator() + QDate::currentDate().toString("yyyy_MM_dd.txt");
        }
        // Append text to file
        QFile log_file(this->cachedLogDirPath);
        if (!log_file.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            GRUMPY_ERROR("Unable to open file for writing: " + this->cachedLogDirPath);
            this->loggingDisabled = true;
            return;
        }
        QString text = ItemToTXTString(item) + "\n";
        log_file.write(text.toUtf8());
        log_file.close();
    }
}

void ScrollbackFrame::SetWindowName(const QString &title)
{
    this->_name = title;
}

bool ScrollbackFrame::IsConnectedToIRC()
{
    if (!this->GetSession())
        return false;
    if (!this->GetSession()->IsConnected())
        return false;
    return true;
}

ScrollbackFrame *ScrollbackFrame::GetParent()
{
    return this->_parent;
}

scrollback_id_t ScrollbackFrame::GetID()
{
    return this->scrollback->GetID();
}

NetworkSession *ScrollbackFrame::GetSession()
{
    return this->scrollback->GetSession();
}

InputBox *ScrollbackFrame::GetInputBox()
{
    return this->inputBox;
}

Scrollback *ScrollbackFrame::GetScrollback()
{
    return this->scrollback;
}

UserFrame *ScrollbackFrame::GetUserFrame()
{
    return this->userFrame;
}

QString ScrollbackFrame::GetTitle()
{
    return this->scrollback->GetTarget();
}

QString ScrollbackFrame::ToString()
{
    return this->textEdit->document()->toPlainText();
}

void ScrollbackFrame::UpdateColor()
{
    if (this->TreeNode)
        this->TreeNode->UpdateColor();
}

void ScrollbackFrame::Focus()
{
    this->inputBox->Focus();
}

bool ScrollbackFrame::IsChannel()
{
    if (!this->scrollback)
        return false;
    return this->scrollback->GetType() == ScrollbackType_Channel;
}

bool ScrollbackFrame::IsNetwork()
{
    if (!this->scrollback || !this->GetSession())
        return false;
    if (this->scrollback->GetType() != ScrollbackType_System)
        return false;
    return true;
}

bool ScrollbackFrame::IsGrumpy()
{
    if (!this->scrollback)
        return false;
    return Generic::IsGrumpyd(this->scrollback);
}

bool GrumpyIRC::ScrollbackFrame::IsHidden()
{
    return this->scrollback->IsHidden();
}

void ScrollbackFrame::ToggleHide()
{
    if (!this->scrollback->IsHideable())
        return;

    if (Generic::IsGrumpyd(this->scrollback))
    {
        dynamic_cast<GrumpydSession*>(this->GetSession())->RequestHide(this->scrollback, !this->scrollback->IsHidden());
        return;
    }

    if (!this->scrollback->IsHidden())
    {
        this->scrollback->Hide();
    }
    else
    {
        this->scrollback->Show();
    }
}

void ScrollbackFrame::SetHidden()
{
    // Remove from tree list
    if (!ScrollbackList::GetScrollbackList()->ShowHidden && this->TreeNode)
    {
        ScrollbackList_Node *parent = nullptr;
        if (this->GetParent())
            parent = this->GetParent()->TreeNode;
        ScrollbackList::GetScrollbackList()->UnregisterWindow(this->TreeNode, parent);
        this->TreeNode = nullptr;
    }
}

void ScrollbackFrame::UnsetHidden()
{
    if (!ScrollbackList::GetScrollbackList()->ShowHidden)
    {
        ScrollbackList_Node *parent_tree = nullptr;
        if (this->GetParent())
            parent_tree = this->GetParent()->TreeNode;
        ScrollbackList::GetScrollbackList()->RegisterWindow(this, parent_tree);
    }
}

bool ScrollbackFrame::IsDead()
{
    return this->scrollback->IsDead();
}

void ScrollbackFrame::HideInput()
{
    this->inputBox->setVisible(false);
}

STextBox *ScrollbackFrame::GetSTextBox()
{
    return this->textEdit;
}

void ScrollbackFrame::RequestClose()
{
    if (!this->IsDead() && this->GetScrollback()->GetType() != ScrollbackType_User)
    {
        return;
    }
    // We need to figure out if we are closing the system window, in that case we need to also delete the corresponding network session that it belonged to
    // the check for system window needs to be done before we request it to be closed as that might remove the reference to it
    NetworkSession *session = nullptr;
    if (this->GetSession())
    {
        if (this->GetSession()->GetSystemWindow() == this->GetScrollback())
            session = this->GetSession();
        this->GetSession()->RequestRemove(this->GetScrollback());
        // Call to RequestRemove probably called delete on this very scrollback frame, so now we are within a deleted object, be carefull here not to access internal memory
        delete session;
    }
}

void ScrollbackFrame::UpdateIcon()
{
    if (this->TreeNode)
        this->TreeNode->UpdateIcon();
}

void ScrollbackFrame::EnableState(bool enable)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (!this->scrollback)
        return;

    // Update the state
    this->scrollback->IgnoreState = !enable;
    if (!enable)
    {
        if (this->scrollback->GetState() == ScrollbackState_Normal)
            return;
        this->scrollback->SetState(ScrollbackState_Normal, true);
        if (this->IsGrumpy())
        {
            GrumpydSession *session = dynamic_cast<GrumpydSession*>(this->GetSession());
            // Resync the information that the window was read
            QHash<QString, QVariant> parameters;
            parameters.insert("_original_id", this->scrollback->GetOriginalID());
            parameters.insert("scrollbackState", static_cast<int>(this->scrollback->GetState()));
            QHash<QString, QVariant> px;
            px.insert("scrollback", parameters);
            session->SendProtocolCommand(GP_CMD_SCROLLBACK_PARTIAL_RESYNC, px);
        }
    }
}

void ScrollbackFrame::RequestPart()
{
    if (this->GetSession() && this->IsChannel())
        this->GetSession()->RequestPart(this->GetScrollback());
}

void ScrollbackFrame::ToggleSecure()
{
    this->inputBox->Secure();
}

void ScrollbackFrame::ExecuteScript(const QString &text)
{
    if (!this->GetSession())
        return;
    ScriptWin *wx = new ScriptWin(this);
    wx->Set(text);
    wx->setAttribute(Qt::WA_DeleteOnClose);
    connect(this->GetSession(), SIGNAL(Event_Deleted()), wx, SLOT(close()));
    wx->show();
}

void ScrollbackFrame::RequestJoin()
{
    if (this->GetSession() && this->IsChannel())
        this->GetSession()->SendRaw(this->GetScrollback(), "JOIN " + this->GetScrollback()->GetTarget());
}

void ScrollbackFrame::Reconnect()
{
    if (this->GetSession())
        this->GetSession()->RequestReconnect(this->GetScrollback());
}

void ScrollbackFrame::RequestDisconnect()
{
    if (this->GetSession())
        this->GetSession()->RequestDisconnect(this->GetScrollback(), CONF->GetQuitMessage(), false);
}

void ScrollbackFrame::RequestMore(unsigned int count)
{
    if (!Generic::IsGrumpyd(this->GetScrollback()))
    {
        MessageBox::Display("only-grumpy", "Error", "This function is available only for use with grumpyd.", MainWindow::Main);
        return;
    }
    GrumpydSession *grumpy = dynamic_cast<GrumpydSession*>(this->GetSession());
    ScrollbackItem first_item = this->GetScrollback()->GetFirst();
    if (first_item.GetID() == 0)
        return;
    grumpy->RequestBL(this->GetScrollback(), first_item.GetID(), count);
    this->currentScrollbar = this->textEdit->verticalScrollBar()->value();
}

void ScrollbackFrame::RefreshHtml()
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    this->needsRefresh = false;
    QString string = "";
    this->unwritten_m.lock();
    if (!this->unwrittenBlock.isEmpty())
    {
        // This is a slow operation and probably will block worker thread, but that doesn't really matter
        this->textEdit->appendHtml(this->unwrittenBlock);
        this->unwrittenBlock.clear();
    }
    if (!this->unwritten.isEmpty())
    {
        string = this->itemsToString(this->unwritten);
        this->unwritten.clear();
    }
    this->unwritten_m.unlock();
    this->buffer += string;
    if (!string.isEmpty())
        this->textEdit->AppendHtml(string);
    this->isClean = false;
}

QString ScrollbackFrame::ToHtml()
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    QString text = this->textEdit->document()->toHtml();
    // Ugly hack to change the background, for some reason Qt doesn't export it
    text.replace("<body style=\"", "<body style=\"background-color: " + Skin::GetCurrent()->BackgroundColor.name() + "; color: " +
                 Skin::GetCurrent()->TextColor.name() + ";");
    if (this->GetNetwork())
    {
        // Replace channel joins
        if (this->GetNetwork()->IsSSL())
            text.replace("<a href=\"irc_join://", "<a href=\"ircs://" + this->GetNetwork()->GetServerAddress() + "/");
        else
            text.replace("<a href=\"irc_join://", "<a href=\"irc://" + this->GetNetwork()->GetServerAddress() + "/");
    }
    return text;
}

void ScrollbackFrame::SendCtcp(const QString &target, const QString &ctcp, const QString &text)
{
    if (this->GetSession())
        this->GetSession()->SendCTCP(this->GetScrollback(), target, ctcp, text);
}

void ScrollbackFrame::RefreshHtmlIfNeeded()
{
    if (this->needsRefresh)
        this->RefreshHtml();
}

void ScrollbackFrame::SetProperty(const QString &name, const QVariant &value)
{
    this->scrollback->SetProperty(name, value);
    // now, if this is grumpy scrollback we need to share this option with others

    if (!Generic::IsGrumpyd(this->scrollback))
        return;

    GrumpydSession *session = dynamic_cast<GrumpydSession*>(this->GetSession());
    session->ResyncSingleItemPB(this->scrollback, name);
}

libircclient::Network *ScrollbackFrame::GetNetwork()
{
    if (this->precachedNetwork)
        return this->precachedNetwork;

    if (this->GetSession())
        this->precachedNetwork = this->GetSession()->GetNetwork(this->GetScrollback());

    return this->precachedNetwork;
}

void ScrollbackFrame::TransferRaw(const QString &data, libircclient::Priority priority)
{
    if (!this->GetSession())
        return;

    // Delegate the raw command
    this->GetSession()->SendRaw(this->GetScrollback(), data, priority);
}

libircclient::User *ScrollbackFrame::GetIdentity()
{
    if (!this->GetSession())
        return nullptr;

    // Return a self identity information for the current network
    return this->GetSession()->GetSelfNetworkID(this->GetScrollback());
}

scrollback_id_t ScrollbackFrame::GetItems()
{
    return this->scrollback->GetLastID();
}

QList<QString> ScrollbackFrame::GetUsers()
{
    if (this->userFrame == nullptr)
        return QList<QString>();
    return this->userFrame->GetUsers();
}

QList<QString> ScrollbackFrame::GetChannels()
{
    if (!this->GetSession())
        return QList<QString>  ();
    return this->GetSession()->GetChannels(this->GetScrollback());
}

QString ScrollbackFrame::GetLocalUserMode()
{
    if (!this->GetSession())
        return "";

    // Return a self identity information for the current network
    return this->GetSession()->GetLocalUserModeAsString(this->GetScrollback());
}

void ScrollbackFrame::UpdateSkin()
{
    this->textEdit->setFont(Skin::GetCurrent()->TextFont);
    QString style_sheet = ScrollbackFrame::parser.GetStyle();

    if (!Skin::GetCurrent()->BackgroundImage.isEmpty())
    {
        // Picture
        style_sheet = "QFrame { border-image: url(" + Skin::GetCurrent()->BackgroundImage + "); }\n\n" + style_sheet;
    }

    this->textEdit->setPalette(Skin::GetCurrent()->Palette());
    this->textEdit->SetStyleSheet(style_sheet);
}

int ScrollbackFrame::GetSynced()
{
    return this->scrollback->GetSICount();
}

bool ScrollbackFrame::IsVisible()
{
    if (this->IsStandalone)
        return true;

    return this->isVisible;
}

void ScrollbackFrame::SetVisible(bool is_visible)
{
    if (this->userFrame != nullptr)
    {
        this->userFrame->IsVisible = is_visible;
        if (is_visible && this->userFrame->NeedsUpdate)
            this->userFrame->UpdateInfo();
    }
    this->isVisible = is_visible;
}


ScrollbackFrame_WorkerThread::ScrollbackFrame_WorkerThread(): GrumpyObject("ScrollbackFrame_WorkerThread")
{
    this->IsRunning = true;
    this->IsFinished = false;
}

void ScrollbackFrame_WorkerThread::Sleep(int msec)
{
    ScrollbackFrame_WorkerThread::msleep(msec);
}

void ScrollbackFrame_WorkerThread::run()
{
    while (this->IsRunning)
    {
        ScrollbackFrame::ScrollbackFrames_m.lock();
        foreach (ScrollbackFrame *s, ScrollbackFrame::ScrollbackFrames)
        {
            if (s->unwritten.isEmpty())
                continue;

            // let's process all unwritten items of this scrollback into html text
            s->unwritten_m.lock();
            if (s->unwritten.isEmpty())
            {
                s->unwritten_m.unlock();
                continue;
            }
            if (!s->unwrittenBlock.isEmpty())
                s->unwrittenBlock += "<br>\n";
            s->unwrittenBlock += s->itemsToString(s->unwritten);
            s->unwritten.clear();
            s->unwritten_m.unlock();
        }
        ScrollbackFrame::ScrollbackFrames_m.unlock();
        ScrollbackFrame_WorkerThread::msleep(200);
    }
    this->IsFinished = true;
}
