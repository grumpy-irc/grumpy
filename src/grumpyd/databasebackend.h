//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef DATABASEBACKEND_H
#define DATABASEBACKEND_H

#include "../libcore/definitions.h"

#include <QHash>

namespace GrumpyIRC
{
    class User;
    class VirtualScrollback;
    class Scrollback;
    class Session;
    class IRCSession;
    class ScrollbackItem;

    class DatabaseBackend
    {
        public:
            DatabaseBackend();
            virtual ~DatabaseBackend();
            virtual QString GetType()=0;
            virtual int GetUserCount()=0;
            virtual void LoadRoles()=0;
            virtual void LoadUsers()=0;
            virtual void LoadSessions()=0;
            virtual void LoadWindows()=0;
            virtual void LoadText()=0;
            virtual void Maintenance()=0;
            virtual void StoreUser(User *item)=0;
            virtual void StoreNetwork(IRCSession *session)=0;
            virtual QList<QVariant> FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size)=0;
            virtual QList<QVariant> Search(QString text, int context, bool case_sensitive = true)=0;
            virtual QList<QVariant> SearchRegular(QString regex, int context, bool case_sensitive = true)=0;
            virtual QList<QVariant> SearchOne(VirtualScrollback *scrollback, QString text, int context, bool case_sensitive = true)=0;
            virtual QList<QVariant> SearchOneRegular(VirtualScrollback *scrollback, QString regex, int context, bool case_sensitive = true)=0;
            virtual void UpdateUser(User *user)=0;
            virtual void RemoveNetwork(IRCSession *session)=0;
            virtual void RemoveUser(User *user)=0;
            virtual void RemoveScrollback(User *owner, Scrollback *sx)=0;
            virtual void UpdateScrollback(User *owner, Scrollback *sx)=0;
            virtual void LockUser(User *user)=0;
            virtual void UnlockUser(User *user)=0;
            virtual void StoreScrollback(User *owner, Scrollback *sx)=0;
            virtual void UpdateNetwork(IRCSession *session)=0;
            virtual void StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item)=0;
            virtual void UpdateRoles()=0;
            virtual QHash<QString, QVariant> GetConfiguration(user_id_t user)=0;
            virtual void SetConfiguration(user_id_t user, QHash<QString, QVariant> data)=0;
            virtual QHash<QString, QByteArray> GetStorage(user_id_t user)=0;
            virtual void InsertStorage(user_id_t user, QString key, QByteArray data)=0;
            virtual void UpdateStorage(user_id_t user, QString key, QByteArray data)=0;
            virtual void RemoveStorage(user_id_t user, QString key)=0;
            virtual bool IsFailed() { return false; }
            virtual QString GetLastErrorText() { return ""; }
    };
}

#endif // DATABASEBACKEND_H
