//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2020

#ifndef SCROLLBACKITEM_H
#define SCROLLBACKITEM_H

#include "libcore_global.h"
#include "definitions.h"
#include "grumpyobject.h"
#include "../libirc/libircclient/user.h"
#include <QString>
#include <QDateTime>
#include "../libirc/libirc/serializableitem.h"
#include <QMutex>
#include <QList>

namespace GrumpyIRC
{
    enum ScrollbackItemType
    {
        ScrollbackItemType_Unknown = 0,
        ScrollbackItemType_Message = 1,
        ScrollbackItemType_Join = 2,
        ScrollbackItemType_Part = 3,
        ScrollbackItemType_Kick = 4,
        ScrollbackItemType_Nick = 5,
        ScrollbackItemType_Mode = 6,
        ScrollbackItemType_Quit = 7,
        ScrollbackItemType_Notice = 8,
        ScrollbackItemType_Act = 9,
        ScrollbackItemType_System = 10,
        ScrollbackItemType_Topic = 11,
        ScrollbackItemType_SystemWarning = 12,
        ScrollbackItemType_SystemError = 13
    };

    /*!
     * \brief The ScrollbackItem class represent one item in scrollback buffer
     */
    class LIBCORESHARED_EXPORT ScrollbackItem : public libirc::SerializableItem, public GrumpyObject
    {
        public:
#ifdef GRUMPY_EXTREME
            static unsigned long long TotalIC;
#endif
            ScrollbackItem(const QHash<QString, QVariant> &hash);
            ScrollbackItem(const QString &text, scrollback_id_t id=0, bool self = false);
            ScrollbackItem(const QString &text, ScrollbackItemType type, libircclient::User *user = nullptr, scrollback_id_t id=0, bool self = false);
            ScrollbackItem(const QString &text, ScrollbackItemType type, const libircclient::User &user, scrollback_id_t id=0, bool self = false);
            ScrollbackItem(const QString &text, ScrollbackItemType type, const libircclient::User &user, const QDateTime &date, scrollback_id_t id=0, bool self = false, char target_group = 0);
            ScrollbackItem(const QString &text, ScrollbackItemType type, const libircclient::User &user, const QDateTime &date, char target_group);
            ScrollbackItem(ScrollbackItem *i);
            ScrollbackItem(const ScrollbackItem &i);
             ~ScrollbackItem() override;
            void SetID(scrollback_id_t id);
            QString GetText() const;
            //! Items in scrollback are indexed with this so that we can sync only newest items.
            //! If you need older items, request them from the lowest ID you have.
            scrollback_id_t GetID();
            ScrollbackItemType GetType() const;
            QDateTime GetTime() const;
            void SetType(ScrollbackItemType type);
            void SetText(const QString &text);
            void SetTime(const QDateTime &t);
            void SetUser(libircclient::User *user);
            bool IsSelf() const;
            char GetTargetGroup() const;
            libircclient::User GetUser() const;
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;

        private:
            char targetGroup = 0;
            scrollback_id_t _id;
            libircclient::User _user;
            QString _text;
            bool _self = false;
            QDateTime _datetime;
            ScrollbackItemType _type;
    };
}

#endif // SCROLLBACKITEM_H
