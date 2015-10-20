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
    class LIBCORESHARED_EXPORT ScrollbackItem
    {
        public:
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
     */
    class LIBCORESHARED_EXPORT Scrollback : public QObject
    {
            Q_OBJECT
        public:
            static QList<Scrollback*> ScrollbackList;
            static QMutex ScrollbackList_Mutex;

            Scrollback(ScrollbackType Type = ScrollbackType_System);
            virtual ~Scrollback();
            unsigned long long GetMaxItemsSize();
            unsigned long long GetID();
            void SetMaxItemsSize(unsigned long long size);
            virtual void InsertText(QString text, ScrollbackItemType type = ScrollbackItemType_System);
            virtual void InsertText(ScrollbackItem item);
            virtual void SetTarget(QString target);
            virtual QString GetTarget() const;
            virtual NetworkSession *GetSession();
            //! Called by IRC session or any other object if there is any change to user list associated to this scrollback
            void UserListChange(QString nick, libircclient::User *user, UserListChangeType change_type);
            virtual ScrollbackType GetType() const;
            virtual void SetSession(NetworkSession *Session);
            virtual bool IsDead() const;
            void SetDead(bool dead);

        signals:
            void Event_InsertText(ScrollbackItem item);
            void Event_UserInserted(libircclient::User *user);
            void Event_UserAltered(QString original_name, libircclient::User *user);
            void Event_SessionModified(NetworkSession *Session);
            void Event_UserRemoved(QString name);
            //! Called when some meta-information for user is changed, such as away status
            //! so that it can be updated in associated widgets
            void Event_UserRefresh(libircclient::User *user);

        private:
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
