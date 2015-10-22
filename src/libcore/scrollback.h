//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SCROLLBACK_H
#define SCROLLBACK_H

#include "libcore_global.h"
#include "../libirc/libircclient/user.h"
#include <QString>
#include <QObject>
#include <QDateTime>
#include "../libirc/libirc/serializableitem.h"
#include <QMutex>
#include <QList>

namespace GrumpyIRC
{
    enum ScrollbackType
    {
        ScrollbackType_Channel,
        ScrollbackType_User,
        ScrollbackType_System
    };

    enum ScrollbackItemType
    {
        ScrollbackItemType_Unknown,
        ScrollbackItemType_Message,
        ScrollbackItemType_Join,
        ScrollbackItemType_Part,
        ScrollbackItemType_Kick,
        ScrollbackItemType_Nick,
        ScrollbackItemType_Quit,
        ScrollbackItemType_Notice,
        ScrollbackItemType_Act,
        ScrollbackItemType_System,
        ScrollbackItemType_Topic
    };

    /*!
     * \brief The ScrollbackItem class is a one item in scrollback buffer
     */
    class LIBCORESHARED_EXPORT ScrollbackItem : public libirc::SerializableItem
    {
        public:
            ScrollbackItem(QHash<QString, QVariant> hash);
            ScrollbackItem(QString text);
            ScrollbackItem(QString text, ScrollbackItemType type, libircclient::User *user = NULL);
            virtual ~ScrollbackItem();
            virtual QString GetText() const;
            virtual ScrollbackItemType GetType() const;
            virtual QDateTime GetTime() const;
            virtual void SetType(ScrollbackItemType type);
            virtual void SetText(QString text);
            virtual void SetUser(libircclient::User *user);
            virtual libircclient::User GetUser() const;
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
        private:
            libircclient::User _user;
            QString _text;
            QDateTime _datetime;
            ScrollbackItemType _type;
    };

    enum UserListChangeType
    {
        UserListChange_Insert,
        UserListChange_Alter,
        UserListChange_Remove,
        UserListChange_Refresh
    };

    class NetworkSession;

    /*!
     * \brief The Scrollback class represent a buffer used to store all items in a window
              It's one of core items that grumpy is built on. Basically every window you
              can see in grumpy is a scrollback. You can write messages to it and they
              may be synchronized over network as well.

              Scrollback can be a system window, channel window or any other text buffer used in program
     */
    class LIBCORESHARED_EXPORT Scrollback : public QObject, public libirc::SerializableItem
    {
            Q_OBJECT
        public:
            static QList<Scrollback*> ScrollbackList;
            static QMutex ScrollbackList_Mutex;

            Scrollback(ScrollbackType Type = ScrollbackType_System, Scrollback *parent = NULL);
            virtual ~Scrollback();
            unsigned long long GetMaxItemsSize();
            unsigned long long GetID();
            void SetMaxItemsSize(unsigned long long size);
            virtual void InsertText(QString text, ScrollbackItemType type = ScrollbackItemType_System);
            virtual void InsertText(ScrollbackItem item);
            virtual void SetTarget(QString target);
            virtual QString GetTarget() const;
            //! If this scrollback is associated to some session this function returns the pointer to it, in case it's not NULL is returned
            virtual NetworkSession *GetSession();
            QList<ScrollbackItem> GetItems();
            //! Called by IRC session or any other object if there is any change to user list associated to this scrollback
            virtual void UserListChange(QString nick, libircclient::User *user, UserListChangeType change_type);
            virtual ScrollbackType GetType() const;
            virtual void SetSession(NetworkSession *Session);
            virtual bool IsDead() const;
            virtual void SetDead(bool dead);
            virtual Scrollback *GetParentScrollback();
            QHash<QString, QVariant> ToHash();
            void LoadHash(QHash<QString, QVariant> hash);

        signals:
            void Event_InsertText(ScrollbackItem item);
            void Event_UserInserted(libircclient::User *user);
            void Event_UserAltered(QString original_name, libircclient::User *user);
            void Event_SessionModified(NetworkSession *Session);
            void Event_Reload();
            void Event_UserRemoved(QString name);
            //! Called when some meta-information for user is changed, such as away status
            //! so that it can be updated in associated widgets
            void Event_UserRefresh(libircclient::User *user);

        private:
            Scrollback *parentSx;
            bool _dead;
            QString _target;
            NetworkSession *session;
            ScrollbackType type;
            static unsigned long long lastID;
            QList<ScrollbackItem> items;
            unsigned long long _id;
            unsigned long long _maxItems;
    };
}

#endif // SCROLLBACK_H
