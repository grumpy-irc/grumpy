//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "scrollback.h"
#include "exception.h"
#include "ircsession.h"

using namespace GrumpyIRC;

QList<Scrollback*> Scrollback::ScrollbackList;
QMutex Scrollback::ScrollbackList_Mutex;
unsigned long long Scrollback::lastID = 1;

Scrollback::Scrollback(ScrollbackType Type)
{
    this->_maxItems = 800000;
    ScrollbackList_Mutex.lock();
    ScrollbackList.append(this);
    ScrollbackList_Mutex.unlock();
    this->session = NULL;
    this->_dead = false;
    this->type = Type;
    this->_id = lastID++;
}

Scrollback::~Scrollback()
{
    ScrollbackList_Mutex.lock();
    ScrollbackList.removeOne(this);
    ScrollbackList_Mutex.unlock();
}

unsigned long long Scrollback::GetMaxItemsSize()
{
    return this->_maxItems;
}

unsigned long long Scrollback::GetID()
{
    return this->_id;
}

ScrollbackType Scrollback::GetType() const
{
    return this->type;
}

void Scrollback::SetMaxItemsSize(unsigned long long size)
{
    this->_maxItems = size;
}

void Scrollback::SetSession(IRCSession *Session)
{
    if (this->session)
        throw new GrumpyIRC::Exception("This scrollback already has an IrcSession", BOOST_CURRENT_FUNCTION);

    // We can store the pointer now
    this->session = Session;
    emit this->Event_SessionModified(Session);
}

bool Scrollback::IsDead() const
{
    return this->_dead;
}

void Scrollback::SetDead(bool dead)
{
    this->_dead = dead;
}

void Scrollback::SetTarget(QString target)
{
    this->_target = target;
}

IRCSession *Scrollback::GetSession()
{
    return this->session;
}

void Scrollback::UserListChange(QString nick, libircclient::User *user, UserListChangeType change_type)
{
    switch (change_type)
    {
        case UserListChange_Alter:
            emit this->Event_UserAltered(nick, user);
            break;
        case UserListChange_Refresh:
            emit this->Event_UserRefresh(user);
            break;
        case UserListChange_Insert:
            emit this->Event_UserInserted(user);
            break;
        case UserListChange_Remove:
            emit this->Event_UserRemoved(nick);
            break;
    }
}

void Scrollback::InsertText(ScrollbackItem item)
{
    this->items.append(item);
    emit Event_InsertText(item);
}

QString Scrollback::GetTarget() const
{
    return this->_target;
}

void Scrollback::InsertText(QString text, ScrollbackItemType type)
{
    ScrollbackItem item(text);
    item.SetType(type);
    this->InsertText(item);
}

ScrollbackItem::ScrollbackItem(QString text)
{
    this->_type = ScrollbackItemType_System;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
}

ScrollbackItem::ScrollbackItem(QString text, ScrollbackItemType type, libircclient::User *user)
{
    this->_type = type;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
    this->_user = libircclient::User(user);
}

ScrollbackItem::~ScrollbackItem()
{
    
}

QString ScrollbackItem::GetText() const
{
    return this->_text;
}

ScrollbackItemType ScrollbackItem::GetType() const
{
    return this->_type;
}

QDateTime ScrollbackItem::GetTime() const
{
    return this->_datetime;
}

void ScrollbackItem::SetType(ScrollbackItemType type)
{
    this->_type = type;
}

void ScrollbackItem::SetText(QString text)
{
    this->_text = text;
}

void ScrollbackItem::SetUser(libircclient::User *user)
{
    this->_user = libircclient::User(user);
}

libircclient::User ScrollbackItem::GetUser() const
{
    return this->_user;
}
