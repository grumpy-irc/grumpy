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
scrollback_id_t Scrollback::lastID = 1;

Scrollback::Scrollback(ScrollbackType Type, Scrollback *parent)
{
    this->_maxItems = 800000;
    this->parentSx = parent;
    ScrollbackList_Mutex.lock();
    ScrollbackList.append(this);
    ScrollbackList_Mutex.unlock();
    this->session = NULL;
    this->_dead = false;
    this->IgnoreState = false;
    this->_lastItemID = 0;
    this->_network = NULL;
    this->type = Type;
    this->_id = lastID++;
    this->_original_id = this->_id;
}

Scrollback::Scrollback(QHash<QString, QVariant> hash)
{
    this->_maxItems = 0;
    this->parentSx = NULL;
    this->IgnoreState = false;
    ScrollbackList_Mutex.lock();
    ScrollbackList.append(this);
    ScrollbackList_Mutex.unlock();
    this->session = NULL;
    this->_dead = false;
    this->_lastItemID   = 0;
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

scrollback_id_t Scrollback::GetMaxItemsSize()
{
    return this->_maxItems;
}

scrollback_id_t Scrollback::GetID()
{
    return this->_id;
}

void Scrollback::Close()
{
    emit this->Event_Closed();
    delete this;
}

scrollback_id_t Scrollback::GetOriginalID()
{
    return this->_original_id;
}

ScrollbackType Scrollback::GetType() const
{
    return this->type;
}

void Scrollback::SetMaxItemsSize(scrollback_id_t size)
{
    this->_maxItems = size;
}

void Scrollback::SetSession(NetworkSession *Session)
{
    if (this->session)
        throw new GrumpyIRC::Exception("This scrollback already has NetworkSession", BOOST_CURRENT_FUNCTION);

    if (Session->GetType() == SessionType_IRC)
        this->SetNetwork(Session->GetNetwork());

    // We can store the pointer now
    this->session = Session;
    emit this->Event_SessionModified(Session);
}

scrollback_id_t Scrollback::GetLastID()
{
    return this->_lastItemID;
}

int Scrollback::GetSICount()
{
    return this->_items.count();
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
    emit this->Event_ChangedDeadStatus();
}

Scrollback *Scrollback::GetParentScrollback()
{
    return this->parentSx;
}

void Scrollback::PrependItems(QList<ScrollbackItem> list)
{
    while (list.count() > 0)
    {
        this->_items.insert(0, list.last());
        list.removeLast();
    }
    emit this->Event_Reload();
}

ScrollbackItem Scrollback::FetchItem(scrollback_id_t id)
{
    foreach (ScrollbackItem item, this->_items)
    {
        if (item.GetID() == id)
            return item;
    }

    throw new GrumpyIRC::Exception("No such item: " + QString::number(id), BOOST_CURRENT_FUNCTION);
}

QList<QVariant> Scrollback::FetchBacklog(scrollback_id_t from, unsigned int size)
{
    QList<QVariant> result;

    if (from > this->_lastItemID)
        return result;

    if (size > from)
    {
        size = from;
    }

    scrollback_id_t current_item_id = from;
    int current_item_index = 0;
    // we need to find an index of current item, that is the one with id "from"
    while (current_item_index < this->_items.size())
    {
        if (this->_items[current_item_index].GetID() == current_item_id)
            break;
        current_item_index++;
    }
    if (current_item_index >= this->_items.size())
    {
        // we didn't find this item in current list
        return result;
    }
    while (size--)
    {
        if (current_item_index < 0)
            break;
        if (this->_items[current_item_index].GetID() == current_item_id)
        {
            result.insert(0, QVariant(this->_items[current_item_index].ToHash()));
        } else
        {
            // inconsistency, do something
            result.insert(0, this->FetchItem(current_item_id).ToHash());
        }
        current_item_id--;
    }

    return result;
}

QHash<QString, QVariant> Scrollback::ToHash(int max)
{
    QHash<QString, QVariant> hash = this->ToPartialHash();
    QList<QVariant> variant_items_list;
    hash.insert("type", static_cast<int>(this->type));
    if (this->_items.count() < max)
    {
        foreach (ScrollbackItem xx, this->_items)
            variant_items_list.append(QVariant(xx.ToHash()));
    }
    else
    {
        while (max > 0)
        {
            ScrollbackItem item = this->_items.at(this->_items.count() - 1 - max--);
            variant_items_list.append(QVariant(item.ToHash()));
        }
    }
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
    hash.insert("scrollbackState", static_cast<int>(this->scrollbackState));
    SERIALIZE(_lastItemID);
    SERIALIZE(_target);
    return hash;
}

void Scrollback::LoadHash(QHash<QString, QVariant> hash)
{
    UNSERIALIZE_HASH(PropertyBag);
    UNSERIALIZE_STRING(_target);
    UNSERIALIZE_BOOL(_dead);
    UNSERIALIZE_UINT(_maxItems);
    UNSERIALIZE_UINT(_lastItemID);
    if (hash.contains("type"))
        this->type = static_cast<ScrollbackType>(hash["type"].toInt());
    if (hash.contains("scrollbackState"))
        this->scrollbackState = static_cast<ScrollbackState>(hash["scrollbackState"].toInt());
    UNSERIALIZE_UINT(_original_id);
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

void Scrollback::SetState(ScrollbackState state, bool enforce)
{
    if (!enforce)
    {
        int current_state = static_cast<int>(this->scrollbackState);
        int modified_state = static_cast<int>(state);
        if (current_state >= modified_state)
            return;
    }

    this->scrollbackState = state;
    emit this->Event_StateModified();
}

ScrollbackState Scrollback::GetState()
{
    return this->scrollbackState;
}

void Scrollback::SetTarget(QString target)
{
    this->_target = target;
}

NetworkSession *Scrollback::GetSession()
{
    return this->session;
}

ScrollbackItem Scrollback::GetFirst()
{
    // We need to return something here even if there are no items we just return an empty item
    if (!this->_items.count())
        return ScrollbackItem("");

    return this->_items.first();
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
    item.SetID(this->_lastItemID++);
    this->_items.append(item);
    if (!this->IgnoreState)
    {
        switch (item.GetType())
        {
            case ScrollbackItemType_Act:
            case ScrollbackItemType_Notice:
            case ScrollbackItemType_Message:
                this->SetState(ScrollbackState_UnreadMessages);
                break;
            case ScrollbackItemType_Kick:
            case ScrollbackItemType_Nick:
            case ScrollbackItemType_Part:
            case ScrollbackItemType_Quit:
            case ScrollbackItemType_System:
            case ScrollbackItemType_Topic:
                this->SetState(ScrollbackState_UnreadSystem);
                break;
        }
    }
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
    this->_self = false;
    this->LoadHash(hash);
}

ScrollbackItem::ScrollbackItem(QString text, scrollback_id_t id, bool self)
{
    this->_type = ScrollbackItemType_System;
    this->_id = id;
    this->_self = self;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
}

ScrollbackItem::ScrollbackItem(QString text, ScrollbackItemType type, libircclient::User *user, scrollback_id_t id, bool self)
{
    this->_type = type;
    this->_id = id;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
    if (user == NULL)
        this->_user = NULL;
    else
        this->_user = libircclient::User(user);
}

ScrollbackItem::ScrollbackItem(QString text, ScrollbackItemType type, libircclient::User user, scrollback_id_t id, bool self)
{
    this->_type = type;
    this->_id = id;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
    this->_user = user;
}

void ScrollbackItem::SetID(scrollback_id_t id)
{
    this->_id = id;
}

ScrollbackItem::~ScrollbackItem()
{
    
}

QString ScrollbackItem::GetText() const
{
    return this->_text;
}

scrollback_id_t ScrollbackItem::GetID()
{
    return this->_id;
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

bool ScrollbackItem::IsSelf() const
{
    return this->_self;
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
    UNSERIALIZE_UINT(_id);
    UNSERIALIZE_BOOL(_self);
    UNSERIALIZE_DATETIME(_datetime);
}

QHash<QString, QVariant> ScrollbackItem::ToHash()
{
    QHash<QString, QVariant> hash;
    SERIALIZE(_text);
    SERIALIZE(_id);
    hash.insert("_user", QVariant(this->_user.ToHash()));
    SERIALIZE(_self);
    SERIALIZE(_datetime);
    hash.insert("_type", QVariant(static_cast<int>(this->_type)));
    return hash;
}
