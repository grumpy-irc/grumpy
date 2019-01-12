//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef DATABASEDUMMY_H
#define DATABASEDUMMY_H

#include "databasebackend.h"

namespace GrumpyIRC
{
    class DatabaseDummy : public DatabaseBackend
    {
        public:
            DatabaseDummy();

            void LoadRoles() override;
            void LoadUsers() override;
            void Maintenance() override {}
            QHash<QString, QVariant> GetConfiguration(user_id_t user) override;
            void SetConfiguration(user_id_t user, QHash<QString, QVariant> data) override;
            void RemoveNetwork(IRCSession *session) override;
            void RemoveScrollback(User *owner, Scrollback *sx) override;
            QList<QVariant> FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size) override;
            void UpdateRoles() override;
            void StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item) override;
            void LoadSessions() override;
            void UpdateNetwork(IRCSession *session) override;
            void RemoveUser(User *user) override;
            void UnlockUser(User *user) override;
            void LockUser(User *user) override;
            void LoadWindows() override;
            void LoadText() override;
            void StoreScrollback(User *owner, Scrollback *sx) override;
            void StoreNetwork(IRCSession *session) override;
            void StoreUser(User *item) override;
            void UpdateUser(User *user) override;
            QHash<QString, QByteArray> GetStorage(user_id_t user) override;
            void InsertStorage(user_id_t user, QString key, QByteArray data) override;
            void UpdateStorage(user_id_t user, QString key, QByteArray data) override;
            void RemoveStorage(user_id_t user, QString key) override;
    };
}

#endif // DATABASEDUMMY_H
