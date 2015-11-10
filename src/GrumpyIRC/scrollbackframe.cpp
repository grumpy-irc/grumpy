//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QScrollBar>
#include <QMenu>
#include "../libcore/exception.h"
#include "../libcore/configuration.h"
#include "../libcore/generic.h"
#include "../libcore/ircsession.h"
#include "../libcore/networksession.h"
#include "../libcore/core.h"
#include "../libirc/libircclient/user.h"
#include "corewrapper.h"
#include "grumpyconf.h"
#include "channelwin.h"
#include "scrollbacklist_node.h"
#include "skin.h"
#include "scrollbackframe.h"
#include "scrollbacksmanager.h"
#include "userframe.h"
#include "inputbox.h"
#include "ui_scrollbackframe.h"

using namespace GrumpyIRC;

irc2htmlcode::Parser ScrollbackFrame::parser;

ScrollbackFrame::ScrollbackFrame(ScrollbackFrame *parentWindow, QWidget *parent, Scrollback *_scrollback) : QFrame(parent), ui(new Ui::ScrollbackFrame)
{
    this->textEdit = new STextBox(this);
    this->ui->setupUi(this);
    this->inputBox = new InputBox(this);
    this->ui->splitter->addWidget(this->textEdit);
    this->ui->splitter->addWidget(this->inputBox);
    this->textEdit->setFont(Skin::GetDefault()->TextFont);
    this->textEdit->setPalette(Skin::GetDefault()->Palette());
    this->_parent = parentWindow;
    this->TreeNode = NULL;
    this->needsRefresh = false;
    this->isClean = true;
    connect(this->textEdit, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(Menu(QPoint)));
    this->maxItems = 200;
    this->userFrame = new UserFrame();
    this->isVisible = false;
    if (_scrollback == NULL)
        this->scrollback = new Scrollback();
    else
        this->scrollback = _scrollback;
    connect(this->scrollback, SIGNAL(Event_NetworkModified(libircclient::Network*)), this, SLOT(NetworkChanged(libircclient::Network*)));
    connect(this->scrollback, SIGNAL(Event_ChangedDeadStatus()), this, SLOT(OnDead()));
    connect(this->scrollback, SIGNAL(Event_UserInserted(libircclient::User*)), this, SLOT(UserList_Insert(libircclient::User*)));
    connect(this->scrollback, SIGNAL(Event_Reload()), this, SLOT(Refresh()));
    connect(this->scrollback, SIGNAL(Event_UserAltered(QString,libircclient::User*)), this, SLOT(UserList_Rename(QString,libircclient::User*)));
    connect(this->scrollback, SIGNAL(Event_UserRemoved(QString)), this, SLOT(UserList_Remove(QString)));
    connect(this->scrollback, SIGNAL(Event_InsertText(ScrollbackItem)), this, SLOT(_insertText_(ScrollbackItem)));
    connect(this->scrollback, SIGNAL(Event_Closed()), this, SLOT(OnClosed()));
    connect(this->scrollback, SIGNAL(Event_UserRefresh(libircclient::User*)), this, SLOT(UserList_Refresh(libircclient::User*)));
}

ScrollbackFrame::~ScrollbackFrame()
{
    //delete this->scrollback;
    delete this->userFrame;
    //! \todo Handle deletion of TreeNode from list of scbs
    //delete this->TreeNode;
    delete this->ui;
}

QString ScrollbackFrame::GetWindowName() const
{
    return this->_name;
}

void ScrollbackFrame::InsertText(QString text)
{
    this->scrollback->InsertText(text);
}

void ScrollbackFrame::InsertText(ScrollbackItem item)
{
    this->scrollback->InsertText(item);
}

static QString FormatAction(libircclient::User user, QString action, bool full_user)
{
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

static QString ItemToString(ScrollbackItem item)
{
    // Render the text according to our formatting
    //! \todo This needs to be precached otherwise we need to build this string every fucking time
    QString format_string = CONF->GetLineFormat();
    QString text = item.GetText();
    QString user = item.GetUser().GetNick();
    //format_string.replace("$time", item.GetTime().toString());
    //QString result;
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
    }
    //format_string.replace("$string", result);
    irc2htmlcode::FormattedItem results = ScrollbackFrame::parser.Process(format_string, item.GetTime(), user, text);
    return results.source;
}

void ScrollbackFrame::_insertText_(ScrollbackItem item)
{
    if (!this->IsVisible())
    {
        while (this->unwritten.size() > this->maxItems)
        {
            this->unwritten.removeAt(0);
            if (!this->isClean)
                this->clearItems();
        }
        this->unwritten.append(item);
        this->needsRefresh = true;
    } else
    {
        this->writeText(item);
    }
}

void ScrollbackFrame::UserList_Insert(libircclient::User *ux)
{
    this->userFrame->InsertUser(ux);
}

void ScrollbackFrame::UserList_Refresh(libircclient::User *ux)
{
    this->userFrame->RefreshUser(ux);
}

void ScrollbackFrame::UserList_Remove(QString user)
{
    this->userFrame->RemoveUser(user);
}

void ScrollbackFrame::UserList_Rename(QString old, libircclient::User *us)
{
    this->userFrame->ChangeNick(us->GetNick(), old);
}

void ScrollbackFrame::OnDead()
{
    this->UpdateIcon();
}

void ScrollbackFrame::Refresh()
{
    this->buffer.clear();
    foreach (ScrollbackItem item, this->scrollback->GetItems())
        this->_insertText_(item);
    this->UpdateIcon();
}

void ScrollbackFrame::Menu(QPoint pn)
{
    QPoint globalPos = this->textEdit->viewport()->mapToGlobal(pn);
    QMenu Menu;
    // Items
    QAction *menuCopy = new QAction(QObject::tr("Copy"), &Menu);
    Menu.addAction(menuCopy);
    QAction *menuRetrieveTopic = NULL;
    QAction *menuChanSet = NULL;
    if (this->IsChannel())
    {
        menuRetrieveTopic = new QAction(QObject::tr("Retrieve topic"), &Menu);
        Menu.addAction(menuRetrieveTopic);
        menuChanSet = new QAction(QObject::tr("Channel settings"), &Menu);
        Menu.addAction(menuChanSet);
    }

    QAction* selectedItem = Menu.exec(globalPos);
    if (!selectedItem)
        return;
    if (selectedItem == menuCopy)
    {

    } else if (selectedItem == menuRetrieveTopic)
    {
        //wx->RequestPart();
    } else if (selectedItem == menuChanSet)
    {
        if (!this->GetScrollback()->GetNetwork())
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
    }
}

void ScrollbackFrame::OnClosed()
{
    // The wrapped scrollback is being closed we must unregister this frame and delete it,
    // before we do that we need to reset the pointer to scrollback, because the destructor
    // of this class naturally tries to delete the scrollback, which would fail as it
    // would already be deleted by then, this event is called from destructor of scrollback,
    // so calling delete on it would have unexpectable results
    this->scrollback = NULL;
    ScrollbacksManager::Global->DestroyWindow(this);
}

void ScrollbackFrame::NetworkChanged(libircclient::Network *network)
{
    this->userFrame->SetNetwork(network);
}

void ScrollbackFrame::clearItems()
{
    this->isClean = true;
    this->buffer.clear();
    this->textEdit->Clear();
}

void ScrollbackFrame::writeText(ScrollbackItem item)
{
    QString line = ItemToString(item);
    this->buffer += line;
    this->textEdit->AppendHtml(line);
    this->isClean = false;
}

void ScrollbackFrame::SetWindowName(QString title)
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

unsigned long ScrollbackFrame::GetID()
{
    return this->scrollback->GetID();
}

NetworkSession *ScrollbackFrame::GetSession()
{
    return this->scrollback->GetSession();
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

bool ScrollbackFrame::IsDead()
{
    return this->scrollback->IsDead();
}

void ScrollbackFrame::RequestClose()
{
    if (!this->IsDead())
    {
        return;
    }
	// We need to figure out if we are closing the system window, in that case we need to also delete the corresponding network session that it belonged to
	// the check for system window needs to be done before we request it to be closed as that might remove the reference to it
	NetworkSession *session = NULL;
	if (this->GetSession())
	{
		if (this->GetSession()->GetSystemWindow() == this->GetScrollback())
			session = this->GetSession();
		this->GetSession()->RequestRemove(this->GetScrollback());
		// Call to RequestRemove probably called delete on this very scrollback frame, so now we are within a deleted object, be carefull here not to access internal memory
		if (session)
			delete session;
    }
}

void ScrollbackFrame::UpdateIcon()
{
    if (this->TreeNode)
        this->TreeNode->UpdateIcon();
}

void ScrollbackFrame::RequestPart()
{
    if (this->GetSession() && this->IsChannel())
        this->GetSession()->RequestPart(this->GetScrollback());
}

void ScrollbackFrame::RequestDisconnect()
{
    if (this->GetSession())
        this->GetSession()->RequestDisconnect(this->GetScrollback(), CONF->GetQuitMessage(), false);
}

void ScrollbackFrame::RequestMore(unsigned int count)
{
    if (!Generic::IsGrumpy(this->GetScrollback()))
        return;
    GrumpydSession *grumpy = (GrumpydSession*)this->GetSession();

}

void ScrollbackFrame::RefreshHtml()
{
    this->needsRefresh = false;
    while (this->unwritten.size())
    {
        this->writeText(this->unwritten.at(0));
        this->unwritten.removeAt(0);
    }
}

void ScrollbackFrame::RefreshHtmlIfNeeded()
{
    if (this->needsRefresh)
        this->RefreshHtml();
}

libircclient::User *ScrollbackFrame::GetIdentity()
{
    if (!this->GetSession())
        return NULL;

    // Return a self identity information for the current network
    return this->GetSession()->GetSelfNetworkID(this->GetScrollback());
}

scrollback_id_t ScrollbackFrame::GetItems()
{
    return this->scrollback->GetLastID();
}

QList<QString> ScrollbackFrame::GetUsers()
{
    return this->userFrame->GetUsers();
}

QList<QString> ScrollbackFrame::GetChannels()
{
    if (!this->GetSession())
        return QList<QString>  ();
    return this->GetSession()->GetChannels(this->GetScrollback());
}

int ScrollbackFrame::GetSynced()
{
    return this->scrollback->GetSICount();
}

bool ScrollbackFrame::IsVisible()
{
    return this->isVisible;
}

void ScrollbackFrame::SetVisible(bool is_visible)
{
    if (this->userFrame)
    {
        this->userFrame->IsVisible = is_visible;
        if (is_visible && this->userFrame->NeedsUpdate)
            this->userFrame->UpdateInfo();
    }
    this->isVisible = is_visible;
}

