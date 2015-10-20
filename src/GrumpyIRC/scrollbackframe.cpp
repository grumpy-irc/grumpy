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
#include "../libcore/exception.h"
#include "../libcore/configuration.h"
#include "../libcore/ircsession.h"
#include "../libcore/core.h"
#include "../libirc/libircclient/user.h"
#include "corewrapper.h"
#include "defaultconfig.h"
#include "skin.h"
#include "scrollbackframe.h"
#include "userframe.h"
#include "inputbox.h"
#include "ui_scrollbackframe.h"

using namespace GrumpyIRC;

ScrollbackFrame::ScrollbackFrame(ScrollbackFrame *parentWindow, QWidget *parent, Scrollback *_scrollback) : QFrame(parent), ui(new Ui::ScrollbackFrame)
{
    this->ui->setupUi(this);
    this->inputBox = new InputBox(this);
    this->ui->splitter->addWidget(this->inputBox);
    this->ui->textEdit->setPalette(Skin::Default->Palette());
    this->_parent = parentWindow;
    this->userFrame = new UserFrame();
    if (_scrollback == NULL)
        this->scrollback = new Scrollback();
    else
        this->scrollback = _scrollback;
    connect(this->scrollback, SIGNAL(Event_SessionModified(IRCSession*)), this, SLOT(SessionChanged(IRCSession*)));
    connect(this->scrollback, SIGNAL(Event_UserInserted(libircclient::User*)), this, SLOT(UserList_Insert(libircclient::User*)));
    connect(this->scrollback, SIGNAL(Event_UserAltered(QString,libircclient::User*)), this, SLOT(UserList_Rename(QString,libircclient::User*)));
    connect(this->scrollback, SIGNAL(Event_UserRemoved(QString)), this, SLOT(UserList_Remove(QString)));
    connect(this->scrollback, SIGNAL(Event_InsertText(ScrollbackItem)), this, SLOT(_insertText_(ScrollbackItem)));
}

ScrollbackFrame::~ScrollbackFrame()
{
    delete this->scrollback;
    delete this->userFrame;
    //! \todo Handle deletion of TreeNode from list of scbs
    //delete this->TreeNode;
    delete this->ui;
}

QString ScrollbackFrame::GetWindowName() const
{
    return this->_name;
}

void ScrollbackFrame::InsertText(ScrollbackItem item)
{
    this->scrollback->InsertText(item);
}

static QString FormatAction(libircclient::User user, QString action, bool full_user)
{
    QString result = CONFIG_ACTION_FORMAT;
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

static QString EncodedHtml(QString text)
{
    text.replace("<", "&lt;");
    text.replace(">", "&gt;");

    return text;
}

static QString ItemToString(ScrollbackItem item)
{
    // Render the text according to our formatting
    //! \todo This needs to be precached otherwise we need to build this string every fucking time
    QString format_string = CONFIG_LINE_FORMAT;
    format_string.replace("$time", item.GetTime().toString());
    QString result;
    switch (item.GetType())
    {
        case ScrollbackItemType_Act:
            result = FormatAction(item.GetUser(), item.GetText(), false);
            break;
        case ScrollbackItemType_Join:
            result = FormatAction(item.GetUser(), "joined channel", true);
            break;
        case ScrollbackItemType_Part:
            if (item.GetText().isEmpty())
                result = FormatAction(item.GetUser(), "left channel", true);
            else
                result = FormatAction(item.GetUser(), "left channel (" + item.GetText() + ")", true);
            break;
        case ScrollbackItemType_Quit:
            result = FormatAction(item.GetUser(), QString("quit (") + item.GetText() + ")", true);
            break;
        case ScrollbackItemType_Kick:
            result = FormatAction(item.GetUser(), item.GetText(), true);
            break;
        case ScrollbackItemType_Nick:
            result = FormatAction(item.GetUser(), QString("changed nick to ") + item.GetText(), false);
            break;
        case ScrollbackItemType_Message:
            result = CONFIG_MESSAGE_FORMAT;
            result.replace("$nick", item.GetUser().GetNick());
            result.replace("$string", item.GetText());
            break;
        case ScrollbackItemType_System:
        case ScrollbackItemType_Unknown:
            result = item.GetText();
            break;
    }
    format_string.replace("$string", result);
    return format_string;
}

void ScrollbackFrame::_insertText_(ScrollbackItem item)
{
    int scroll = 0;
    if (this->ui->textEdit->verticalScrollBar()->maximum() != this->ui->textEdit->verticalScrollBar()->value())
        scroll = this->ui->textEdit->verticalScrollBar()->value();
    this->buffer += EncodedHtml(ItemToString(item)) + "<br>\n";
    this->ui->textEdit->setHtml(buffer);
    // Scroll to bottom
    // We need to do this only if the text was on bottom in past
    if (!scroll)
        scroll = this->ui->textEdit->verticalScrollBar()->maximum();
    this->ui->textEdit->verticalScrollBar()->setValue(scroll);
}

void ScrollbackFrame::UserList_Insert(libircclient::User *ux)
{
    this->userFrame->InsertUser(ux);
}

void ScrollbackFrame::UserList_Remove(QString user)
{
    this->userFrame->RemoveUser(user);
}

void ScrollbackFrame::UserList_Rename(QString old, libircclient::User *us)
{
    this->userFrame->ChangeNick(us->GetNick(), old);
}

void ScrollbackFrame::SessionChanged(NetworkSession *session)
{
    this->userFrame->SetNetwork(session->GetNetwork());
}

void ScrollbackFrame::SetWindowName(QString title)
{
    this->_name = title;
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

void ScrollbackFrame::Focus()
{
    this->inputBox->Focus();
}

