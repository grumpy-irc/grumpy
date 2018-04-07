//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SCROLLBACK_H
#define SCROLLBACK_H

#include "libcore_global.h"
#include "definitions.h"
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

    enum ScrollbackState
    {
        ScrollbackState_Normal = 1,
		ScrollbackState_UnreadSystem = 2,
        ScrollbackState_UnreadMessages = 3,
        ScrollbackState_UnreadNotice = 4
    };

    /*!
     * \brief The ScrollbackItem class is a one item in scrollback buffer
     */
    class LIBCORESHARED_EXPORT ScrollbackItem : public libirc::SerializableItem
    {
        public:
#ifdef GRUMPY_EXTREME
            static unsigned long long TotalIC;
#endif
            ScrollbackItem(QHash<QString, QVariant> hash);
            ScrollbackItem(QString text, scrollback_id_t id=0, bool self = false);
            ScrollbackItem(QString text, ScrollbackItemType type, libircclient::User *user = NULL, scrollback_id_t id=0, bool self = false);
            ScrollbackItem(QString text, ScrollbackItemType type, libircclient::User user, scrollback_id_t id=0, bool self = false);
            ScrollbackItem(QString text, ScrollbackItemType type, libircclient::User user, QDateTime date, scrollback_id_t id=0, bool self = false);
            virtual ~ScrollbackItem();
            virtual void SetID(scrollback_id_t id);
            virtual QString GetText() const;
            //! Items in scrollback are indexed with this so that we can sync only newest items.
            //! If you need older items, request them from the lowest ID you have.
            virtual scrollback_id_t GetID();
            virtual ScrollbackItemType GetType() const;
            virtual QDateTime GetTime() const;
            virtual void SetType(ScrollbackItemType type);
            virtual void SetText(QString text);
            virtual void SetUser(libircclient::User *user);
            virtual bool IsSelf() const;
            virtual libircclient::User GetUser() const;
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
        private:
            scrollback_id_t _id;
            libircclient::User _user;
            QString _text;
            bool _self;
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
            static Scrollback *GetScrollbackByID(scrollback_id_t id);
            static QList<Scrollback*> ScrollbackList;
            static QMutex ScrollbackList_Mutex;

            Scrollback(ScrollbackType Type = ScrollbackType_System, Scrollback *parent = NULL, bool scrollback_hidden = false);
            Scrollback(QHash<QString, QVariant> hash);
            virtual ~Scrollback();
            virtual void Close();
            //! Maximum amount of items allowed to be stored in buffer of this scrollback
            //! if exceeded, oldest items are removed
            virtual scrollback_id_t GetMaxItemsSize();
            //! Unique ID of this scrollback for this grumpy, this is newer synced over network
            scrollback_id_t GetID();
            //! Original ID of this scrollback as it was on grumpy instance which created it
            //! synced ower network
            scrollback_id_t GetOriginalID();
            virtual void SetOriginalID(scrollback_id_t sid);
            void SetMaxItemsSize(scrollback_id_t size);
            virtual void InsertText(QString text, ScrollbackItemType type = ScrollbackItemType_System);
            virtual void InsertText(ScrollbackItem item);
            ScrollbackItem GetFirst();
            QString GetLTarget();
            virtual QString GetTarget() const;
            virtual void SetTarget(QString target);
            virtual void SetSITotalCount(scrollback_id_t sitc);
            //! If this scrollback is associated to some session this function returns the pointer to it, in case it's not NULL is returned
            virtual NetworkSession *GetSession();
            virtual QList<ScrollbackItem> GetItems();
            //! Called by IRC session or any other object if there is any change to user list associated to this scrollback
            virtual void UserListChange(QString nick, libircclient::User *user, UserListChangeType change_type, bool bulk = false);
            virtual ScrollbackType GetType() const;
            virtual void SetSession(NetworkSession *Session);
            virtual bool IsDead() const;
            void SetNetwork(libircclient::Network *Network);
            virtual libircclient::Network *GetNetwork() const;
            virtual void SetDead(bool dead);
            virtual scrollback_id_t GetLastID();
            virtual void Show();
            virtual void Hide();
            virtual int GetSICount();
            //! Return (possibly unaccurate) amount of all items that were ever inserted to this scrollback
            virtual scrollback_id_t GetSITotalCount();
            virtual Scrollback *GetParentScrollback();
            virtual void FinishBulk();
            virtual bool IsHidden() const;
            virtual void Resize(scrollback_id_t size);
            virtual void PrependItems(QList<ScrollbackItem> list);
            virtual ScrollbackItem FetchItem(scrollback_id_t id);
            virtual QList<QVariant> FetchBacklog(scrollback_id_t from, unsigned int size);
            QHash<QString, QVariant> ToHash(int max = 200);
            QHash<QString, QVariant> ToPartialHash();
            virtual bool IsHidable();
            void LoadHash(QHash<QString, QVariant> hash);
            //! Used to resync most of attributes with target
            void Resync(Scrollback *target);
            virtual void SetHidable(bool is);
            virtual void SetState(ScrollbackState state, bool enforced = false);
            virtual ScrollbackState GetState();
            virtual void SetProperty(QString name, QVariant value);
            virtual int GetPropertyAsInt(QString name, int default_val = 0);
            virtual QString GetPropertyAsString(QString name, QString default_val = "");
            virtual bool GetPropertyAsBool(QString name, bool default_val = false);
            //! You can set this to true in order to suppress state updates
            bool IgnoreState;
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
            void Event_UserInserted(libircclient::User *user, bool bulk);
            void Event_UserAltered(QString original_name, libircclient::User *user);
            void Event_ChangedDeadStatus();
            void Event_SessionModified(NetworkSession *Session);
            void Event_StateModified();
            void Event_UserListBulkDone();
            void Event_Show();
            void Event_Reload();
            void Event_UserRemoved(QString name, bool bulk);
            void Event_Hide();
            void Event_Resync();
            //! Called when some meta-information for user is changed, such as away status
            //! so that it can be updated in associated widgets
            void Event_UserRefresh(libircclient::User *user);

        protected:
            static scrollback_id_t lastID;

            virtual void insertSI(ScrollbackItem si);
            scrollback_id_t _lastItemID;
            bool _sbHidden;
            ScrollbackState scrollbackState;
            libircclient::Network *_network;
            Scrollback *parentSx;
            bool _dead;
            QString _target;
            QString _ltarget;
            NetworkSession *session;
            bool _hidable = true;
            ScrollbackType type;
            QList<ScrollbackItem> _items;
            scrollback_id_t _totalItems;
            scrollback_id_t _id;
            scrollback_id_t _original_id;
            scrollback_id_t _maxItems;
    };
}

#endif // SCROLLBACK_H
