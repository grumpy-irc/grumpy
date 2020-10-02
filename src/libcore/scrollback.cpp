//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "scrollback.h"
#include "exception.h"
#include "profiler.h"
#include "ircsession.h"
#include "hooks.h"

using namespace GrumpyIRC;

QList<Scrollback*> Scrollback::ScrollbackList;
QMutex Scrollback::ScrollbackList_Mutex;
scrollback_id_t Scrollback::lastID = 1;

Scrollback *Scrollback::GetScrollbackByID(scrollback_id_t id)
{
    foreach (Scrollback *s, Scrollback::ScrollbackList)
    {
        if (s->GetID() == id)
            return s;
    }
    return nullptr;
}

Scrollback::Scrollback(ScrollbackType Type, Scrollback *parent, bool scrollback_hidden)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("Scrollback");
    this->grumpyObjectIncrementCount();
#endif
    this->_maxItems = GRUMPY_SCROLLBACK_MAX_ITEMS;
    this->_totalItems = 0;
    this->scrollbackState = ScrollbackState_Normal;
    this->parentSx = parent;
    ScrollbackList_Mutex.lock();
    ScrollbackList.append(this);
    ScrollbackList_Mutex.unlock();
    this->session = nullptr;
    this->_dead = false;
    this->_sbHidden = scrollback_hidden;
    this->IgnoreState = false;
    this->_lastItemID = 0;
    this->_network = nullptr;
    this->type = Type;
    this->_id = lastID++;
    this->_original_id = this->_id;
}

Scrollback::Scrollback(const QHash<QString, QVariant> &hash)
{
    this->_maxItems = 0;
    this->_totalItems = 0;
    this->scrollbackState = ScrollbackState_Normal;
    this->parentSx = nullptr;
    this->IgnoreState = false;
    ScrollbackList_Mutex.lock();
    ScrollbackList.append(this);
    ScrollbackList_Mutex.unlock();
    this->session = nullptr;
    this->_dead = false;
    this->_sbHidden = false;
    this->_lastItemID   = 0;
    this->_network = nullptr;
    this->_original_id = 0;
    this->_id = 0;
    this->LoadHash(hash);
}

Scrollback::~Scrollback()
{
    Hooks::OnScrollback_Destroyed(this);
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

void Scrollback::SetOriginalID(scrollback_id_t sid)
{
    this->_original_id = sid;
}

scrollback_id_t Scrollback::GetSITotalCount()
{
    return this->_totalItems;
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

void Scrollback::Show()
{
    this->_sbHidden = false;
    emit this->Event_Show();
}

void Scrollback::Hide()
{
    if (!this->_hidable)
        throw new Exception("Scrollback can't be hidden", BOOST_CURRENT_FUNCTION);

    this->_sbHidden = true;
    emit this->Event_Hide();
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
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    while (list.count() > 0)
    {
        this->_items.insert(0, list.last());
        list.removeLast();
    }
    if (this->_maxItems <  (scrollback_id_t)this->_items.size())
        this->_maxItems =  (scrollback_id_t)this->_items.size();
    this->_totalItems +=   (scrollback_id_t)list.size();
    emit this->Event_Reload();
}

ScrollbackItem Scrollback::FetchItem(scrollback_id_t id)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    foreach (ScrollbackItem item, this->_items)
    {
        if (item.GetID() == id)
            return item;
    }

    throw new GrumpyIRC::Exception("No such item: " + QString::number(id), BOOST_CURRENT_FUNCTION);
}

QList<QVariant> Scrollback::FetchBacklog(scrollback_id_t from, unsigned int size)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
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
            int id = this->_items.count() - max--;
            if (id < 0)
                throw new Exception("Negative index", BOOST_CURRENT_FUNCTION);

            ScrollbackItem item = this->_items.at(id);
            variant_items_list.append(QVariant(item.ToHash()));
        }
    }
    hash.insert("items", QVariant(variant_items_list));
    return hash;
}

void Scrollback::FinishBulk()
{
    emit this->Event_UserListBulkDone();
}

bool Scrollback::IsHidden() const
{
    return this->_sbHidden;
}

void Scrollback::Resize(scrollback_id_t size)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    this->_maxItems = size;
    while ((unsigned int)this->_items.size() > this->_maxItems)
        this->_items.removeAt(0);
}

QHash<QString, QVariant> Scrollback::ToPartialHash()
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    QHash<QString, QVariant> hash;
    SERIALIZE(_dead);
    SERIALIZE(_totalItems);
    SERIALIZE(_original_id);
    SERIALIZE(_sbHidden);
    SERIALIZE(PropertyBag);
    SERIALIZE(_maxItems);
    hash.insert("scrollbackState", static_cast<int>(this->scrollbackState));
    SERIALIZE(_lastItemID);
    SERIALIZE(_target);
    SERIALIZE(_hidable);
    return hash;
}

bool Scrollback::IsHideable()
{
    return this->_hidable;
}

void Scrollback::LoadHash(const QHash<QString, QVariant> &hash)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    UNSERIALIZE_HASH(PropertyBag);
    UNSERIALIZE_UINT(_totalItems);
    UNSERIALIZE_STRING(_target);
    UNSERIALIZE_BOOL(_dead);
    UNSERIALIZE_BOOL(_sbHidden);
    UNSERIALIZE_UINT(_maxItems);
    UNSERIALIZE_UINT(_lastItemID);
    if (hash.contains("type"))
        this->type = static_cast<ScrollbackType>(hash["type"].toInt());
    if (hash.contains("scrollbackState"))
        this->scrollbackState = static_cast<ScrollbackState>(hash["scrollbackState"].toInt());
    UNSERIALIZE_UINT(_original_id);
    if (hash.contains("items"))
    {
        this->_items.clear();
        QList<QVariant> items_l;
        items_l = hash["items"].toList();
        foreach (QVariant item, items_l)
            _items.append(item.toHash());
        emit this->Event_Reload();
    }
    UNSERIALIZE_BOOL(_hidable);
}

void Scrollback::Resync(Scrollback *target)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (target != nullptr)
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
    }
    emit this->Event_Resync();
}

void Scrollback::SetHidable(bool is)
{
    this->_hidable = is;
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

void Scrollback::SetProperty(const QString &name, const QVariant &value)
{
    if (this->PropertyBag.contains(name))
        this->PropertyBag[name] = value;
    else
        this->PropertyBag.insert(name, value);
}

int Scrollback::GetPropertyAsInt(const QString &name, int default_val)
{
    if (!this->PropertyBag.contains(name))
        return default_val;
    return this->PropertyBag[name].toInt();
}

QString Scrollback::GetPropertyAsString(const QString &name, QString default_val)
{
    if (!this->PropertyBag.contains(name))
        return default_val;
    return this->PropertyBag[name].toString();
}

bool Scrollback::GetPropertyAsBool(const QString &name, bool default_val)
{
    if (!this->PropertyBag.contains(name))
        return default_val;
    return this->PropertyBag[name].toBool();
}

void Scrollback::insertSI(const ScrollbackItem &si)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    this->_totalItems++;
    while ((unsigned int)this->_items.size() > this->_maxItems)
    {
        this->_items.removeAt(0);
    }
    this->_items.append(si);
}

void Scrollback::SetTarget(const QString &target)
{
    this->_ltarget.clear();
    this->_target = target;
}

void Scrollback::SetSITotalCount(scrollback_id_t sitc)
{
    if (sitc < static_cast<scrollback_id_t>(this->_items.count()))
        sitc = static_cast<scrollback_id_t>(this->_items.count());
    this->_totalItems = sitc;
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

QString Scrollback::GetLTarget()
{
    if (!this->_ltarget.isEmpty())
        return this->_ltarget;

    this->_ltarget = this->_target.toLower();
    return this->_ltarget;
}

QList<ScrollbackItem> Scrollback::GetItems()
{
    return this->_items;
}

void Scrollback::UserListChange(const QString &nick, libircclient::User *user, UserListChangeType change_type, bool bulk)
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
            emit this->Event_UserInserted(user, bulk);
            break;
        case UserListChange_Remove:
            emit this->Event_UserRemoved(nick, bulk);
            break;
    }
}

void Scrollback::InsertText(ScrollbackItem item)
{
    item.SetID(this->_lastItemID++);
    this->insertSI(item);
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
            case ScrollbackItemType_Join:
            case ScrollbackItemType_Mode:
            case ScrollbackItemType_Quit:
            case ScrollbackItemType_System:
            case ScrollbackItemType_Topic:
                this->SetState(ScrollbackState_UnreadSystem);
                break;
            // To silence compiler warnings
            default:
                break;
        }
    }
    emit Event_InsertText(item);
}

QString Scrollback::GetTarget() const
{
    return this->_target;
}

void Scrollback::InsertText(const QString &text, ScrollbackItemType type)
{
    ScrollbackItem item(text);
    item.SetType(type);
    this->InsertText(item);
}

void Scrollback::InsertText(const QString &text, const QDateTime &time, ScrollbackItemType type)
{
    ScrollbackItem item(text, type, libircclient::User(), time);
    this->InsertText(item);
}
