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

Scrollback::Scrollback(ScrollbackType Type, Scrollback *parent)
{
    this->_maxItems = 800000;
    this->parentSx = parent;
    ScrollbackList_Mutex.lock();
    ScrollbackList.append(this);
    ScrollbackList_Mutex.unlock();
    this->session = NULL;
    this->_dead = false;
    this->_network = NULL;
    this->type = Type;
    this->_id = lastID++;
    this->_original_id = this->_id;
}

Scrollback::Scrollback(QHash<QString, QVariant> hash)
{
    this->_maxItems = 0;
    this->parentSx = NULL;
    ScrollbackList_Mutex.lock();
    ScrollbackList.append(this);
    ScrollbackList_Mutex.unlock();
    this->session = NULL;
    this->_dead = false;
    this->_network = NULL;
    this->_original_id = 0;
    this->_id = 0;
    this->LoadHash(hash);
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

void Scrollback::Close()
{
    emit this->Event_Closed();
    delete this;
}

unsigned long long Scrollback::GetOriginalID()
{
    return this->_original_id;
}

ScrollbackType Scrollback::GetType() const
{
    return this->type;
}

void Scrollback::SetMaxItemsSize(unsigned long long size)
{
    this->_maxItems = size;
}

void Scrollback::SetSession(NetworkSession *Session)
{
    if (this->session)
        throw new GrumpyIRC::Exception("This scrollback already has an IrcSession", BOOST_CURRENT_FUNCTION);

    if (Session->GetType() == SessionType_IRC)
        this->SetNetwork(Session->GetNetwork());

    // We can store the pointer now
    this->session = Session;
    emit this->Event_SessionModified(Session);
}

bool Scrollback::IsDead() const
{
    return this->_dead;
}

void Scrollback::SetNetwork(libircclient::Network *Network)
{
    this->_network = Network;
    emit this->Event_NetworkModified(Network);
}

libircclient::Network *Scrollback::GetNetwork() const
{
    return this->_network;
}

void Scrollback::SetDead(bool dead)
{
    this->_dead = dead;
}

Scrollback *Scrollback::GetParentScrollback()
{
    return this->parentSx;
}

QHash<QString, QVariant> Scrollback::ToHash()
{
    QHash<QString, QVariant> hash = this->ToPartialHash();
    QList<QVariant> variant_items_list;
    hash.insert("type", static_cast<int>(this->type));
    foreach (ScrollbackItem xx, this->_items)
        variant_items_list.append(QVariant(xx.ToHash()));
    hash.insert("items", QVariant(variant_items_list));
    return hash;
}

QHash<QString, QVariant> Scrollback::ToPartialHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(_dead);
    SERIALIZE(_original_id);
    SERIALIZE(PropertyBag);
    SERIALIZE(_maxItems);
    SERIALIZE(_target);
    return hash;
}

void Scrollback::LoadHash(QHash<QString, QVariant> hash)
{
    UNSERIALIZE_HASH(PropertyBag);
    UNSERIALIZE_STRING(_target);
    UNSERIALIZE_BOOL(_dead);
    UNSERIALIZE_ULONGLONG(_maxItems);
    if (hash.contains("type"))
        this->type = static_cast<ScrollbackType>(hash["type"].toInt());
    UNSERIALIZE_ULONGLONG(_original_id);
    if (hash.contains("items"))
    {
        QList<QVariant> items_l;
        items_l = hash["items"].toList();
        foreach (QVariant item, items_l)
            _items.append(item.toHash());
        emit this->Event_Reload();
    }
}

void Scrollback::Resync(Scrollback *target)
{
    this->SetDead(target->IsDead());
    this->SetTarget(target->GetTarget());
    this->SetMaxItemsSize(target->GetMaxItemsSize());
    foreach (QString key, target->PropertyBag.keys())
    {
        if (!this->PropertyBag.contains(key))
            this->PropertyBag.insert(key, target->PropertyBag[key]);
        else
            this->PropertyBag[key] = target->PropertyBag[key];
    }
    emit this->Event_Resync();
}

void Scrollback::SetTarget(QString target)
{
    this->_target = target;
}

NetworkSession *Scrollback::GetSession()
{
    return this->session;
}

QList<ScrollbackItem> Scrollback::GetItems()
{
    return this->_items;
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
    this->_items.append(item);
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

ScrollbackItem::ScrollbackItem(QHash<QString, QVariant> hash)
{
    this->_type = ScrollbackItemType_System;
    this->LoadHash(hash);
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

void ScrollbackItem::LoadHash(QHash<QString, QVariant> hash)
{
    if (hash.contains("_user"))
        this->_user = libircclient::User(hash["_user"].toHash());
    if (hash.contains("_type"))
        this->_type = static_cast<ScrollbackItemType>(hash["_type"].toInt());
    UNSERIALIZE_STRING(_text);
    UNSERIALIZE_DATETIME(_datetime);
}

QHash<QString, QVariant> ScrollbackItem::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(_text);
    hash.insert("_user", QVariant(this->_user.ToHash()));
    SERIALIZE(_datetime);
    hash.insert("_type", QVariant(static_cast<int>(this->_type)));
    return hash;
}
