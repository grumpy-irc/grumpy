//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2020

#include "scrollbackitem.h"
#include "exception.h"
#include "profiler.h"
#include "ircsession.h"
#include "hooks.h"

using namespace GrumpyIRC;

#ifdef GRUMPY_EXTREME
unsigned long long ScrollbackItem::TotalIC = 0;
#endif

ScrollbackItem::ScrollbackItem(const QHash<QString, QVariant> &hash)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("ScrollbackItem");
    this->grumpyObjectIncrementCount();
#endif
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC++;
#endif
    this->_type = ScrollbackItemType_System;
    this->_self = false;
    this->LoadHash(hash);
}

ScrollbackItem::ScrollbackItem(const QString &text, scrollback_id_t id, bool self)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("ScrollbackItem");
    this->grumpyObjectIncrementCount();
#endif
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC++;
#endif
    this->_type = ScrollbackItemType_System;
    this->_id = id;
    this->_self = self;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
}

ScrollbackItem::ScrollbackItem(const QString &text, ScrollbackItemType type, libircclient::User *user, scrollback_id_t id, bool self)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("ScrollbackItem");
    this->grumpyObjectIncrementCount();
#endif
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC++;
#endif
    this->_type = type;
    this->_id = id;
    this->_self = self;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
    if (user == nullptr)
        this->_user = libircclient::User();
    else
        this->_user = libircclient::User(user);
}

ScrollbackItem::ScrollbackItem(const QString &text, ScrollbackItemType type, const libircclient::User &user, scrollback_id_t id, bool self)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("ScrollbackItem");
    this->grumpyObjectIncrementCount();
#endif
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC++;
#endif
    this->_type = type;
    this->_id = id;
    this->_self = self;
    this->_text = text;
    this->_datetime = QDateTime::currentDateTime();
    this->_user = user;
}

ScrollbackItem::ScrollbackItem(const QString &text, ScrollbackItemType type, const libircclient::User &user, const QDateTime &date, scrollback_id_t id, bool self, char target_group)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("ScrollbackItem");
    this->grumpyObjectIncrementCount();
#endif
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC++;
#endif
    this->_type = type;
    this->_id = id;
    this->_self = self;
    this->_text = text;
    this->_datetime = date;
    this->_user = user;
    this->targetGroup = target_group;
}

ScrollbackItem::ScrollbackItem(const QString &text, ScrollbackItemType type, const libircclient::User &user, const QDateTime &date, char target_group)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("ScrollbackItem");
    this->grumpyObjectIncrementCount();
#endif
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC++;
#endif
    this->_type = type;
    this->_text = text;
    this->_datetime = date;
    this->_user = user;
    this->targetGroup = target_group;
}

ScrollbackItem::ScrollbackItem(ScrollbackItem *i)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("ScrollbackItem");
    this->grumpyObjectIncrementCount();
#endif
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC++;
#endif
    this->_datetime = i->_datetime;
    this->_id = i->_id;
    this->_self = i->_self;
    this->_text = i->_text;
    this->_type = i->_type;
    this->_user = i->_user;
    this->targetGroup = i->targetGroup;
}

ScrollbackItem::ScrollbackItem(const ScrollbackItem &i)
{
#ifdef GRUMPY_PROFILER
    this->setGrumpyObjectName("ScrollbackItem");
    this->grumpyObjectIncrementCount();
#endif
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC++;
#endif
    this->_datetime = i._datetime;
    this->_id = i._id;
    this->_self = i._self;
    this->_text = i._text;
    this->_type = i._type;
    this->_user = i._user;
    this->targetGroup = i.targetGroup;
}

void ScrollbackItem::SetID(scrollback_id_t id)
{
    this->_id = id;
}

ScrollbackItem::~ScrollbackItem()
{
#ifdef GRUMPY_EXTREME
    ScrollbackItem::TotalIC--;
#endif
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

void ScrollbackItem::SetText(const QString &text)
{
    this->_text = text;
}

void ScrollbackItem::SetTime(const QDateTime &t)
{
    this->_datetime = t;
}

void ScrollbackItem::SetUser(libircclient::User *user)
{
    this->_user = libircclient::User(user);
}

bool ScrollbackItem::IsSelf() const
{
    return this->_self;
}

char ScrollbackItem::GetTargetGroup() const
{
    return this->targetGroup;
}

libircclient::User ScrollbackItem::GetUser() const
{
    return this->_user;
}

void ScrollbackItem::LoadHash(const QHash<QString, QVariant> &hash)
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    if (hash.contains("_user"))
        this->_user = libircclient::User(hash["_user"].toHash());
    if (hash.contains("_type"))
        this->_type = static_cast<ScrollbackItemType>(hash["_type"].toInt());
    UNSERIALIZE_STRING(_text);
    UNSERIALIZE_UINT(_id);
    UNSERIALIZE_BOOL(_self);
    UNSERIALIZE_DATETIME(_datetime);
    UNSERIALIZE_CCHAR(targetGroup);
}

QHash<QString, QVariant> ScrollbackItem::ToHash()
{
    GRUMPY_PROFILER_INCRCALL(BOOST_CURRENT_FUNCTION);
    QHash<QString, QVariant> hash;
    SERIALIZE(_text);
    SERIALIZE(_id);
    hash.insert("_user", QVariant(this->_user.ToHash()));
    SERIALIZE(_self);
    SERIALIZE(_datetime);
    SERIALIZE(targetGroup);
    hash.insert("_type", QVariant(static_cast<int>(this->_type)));
    return hash;
}
