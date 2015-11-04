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

namespace libircclient
{
    class Network;
}

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
            ScrollbackItem(QString text, ScrollbackItemType type, libircclient::User user);
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
            Scrollback(QHash<QString, QVariant> hash);
            virtual ~Scrollback();
            virtual void Close();
            virtual unsigned long long GetMaxItemsSize();
            //! Unique ID of this scrollback for this grumpy, this is newer synced over network
            unsigned long long GetID();
            //! Original ID of this scrollback as it was on grumpy instance which created it
            //! synced ower network
            unsigned long long GetOriginalID();
            void SetMaxItemsSize(unsigned long long size);
            virtual void InsertText(QString text, ScrollbackItemType type = ScrollbackItemType_System);
            virtual void InsertText(ScrollbackItem item);
            virtual void SetTarget(QString target);
            virtual QString GetTarget() const;
            //! If this scrollback is associated to some session this function returns the pointer to it, in case it's not NULL is returned
            virtual NetworkSession *GetSession();
            virtual QList<ScrollbackItem> GetItems();
            //! Called by IRC session or any other object if there is any change to user list associated to this scrollback
            virtual void UserListChange(QString nick, libircclient::User *user, UserListChangeType change_type);
            virtual ScrollbackType GetType() const;
            virtual void SetSession(NetworkSession *Session);
            virtual bool IsDead() const;
            void SetNetwork(libircclient::Network *Network);
            virtual libircclient::Network *GetNetwork() const;
            virtual void SetDead(bool dead);
            virtual Scrollback *GetParentScrollback();
            QHash<QString, QVariant> ToHash(int max = 200);
            QHash<QString, QVariant> ToPartialHash();
            void LoadHash(QHash<QString, QVariant> hash);
            //! Used to resync most of attributes with target
            void Resync(Scrollback *target);
            QHash<QString, QVariant> PropertyBag;

        signals:
            void Event_Closed();
            void Event_InsertText(ScrollbackItem item);
            //! Called when internal pointer to a network was modified, this is required by some wrappers
            //!
            //! If you are rendering a user list (which is associated with Channel scrollbacks) you need to have a pointer
            //! to a network that owns this channel because users in a list require access to some network functions
            //!
            //! this event is called when a network is associated with the scrollback so that wrappers can update
            void Event_NetworkModified(libircclient::Network *network);
            void Event_UserInserted(libircclient::User *user);
            void Event_UserAltered(QString original_name, libircclient::User *user);
            void Event_ChangedDeadStatus();
            void Event_SessionModified(NetworkSession *Session);
            void Event_Reload();
            void Event_UserRemoved(QString name);
            void Event_Resync();
            //! Called when some meta-information for user is changed, such as away status
            //! so that it can be updated in associated widgets
            void Event_UserRefresh(libircclient::User *user);

        protected:
            static unsigned long long lastID;

            libircclient::Network *_network;
            Scrollback *parentSx;
            bool _dead;
            QString _target;
            NetworkSession *session;
            ScrollbackType type;
            QList<ScrollbackItem> _items;
            unsigned long long _id;
            unsigned long long _original_id;
            unsigned long long _maxItems;
    };
}

#endif // SCROLLBACK_H
