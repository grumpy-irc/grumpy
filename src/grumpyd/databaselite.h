//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef DATABASELITE_H
#define DATABASELITE_H

#include "../libcore/sqlite.h"
#include "databasebackend.h"

#ifdef GRUMPY_SQLITE

#define GRUMPYD_SCHEMA_VERSION 3

#include <QMutex>

struct sqlite3;

namespace GrumpyIRC
{
    class SQLite;

    class DatabaseLite : public DatabaseBackend
    {
        public:
            static QString GetSource(const QString& name);

            DatabaseLite();
            ~DatabaseLite() override;
            void LoadRoles() override;
            void LoadUsers() override;
            void LoadSessions() override;
            void LoadWindows() override;
            void LoadText() override;
            void Maintenance() override;
            QHash<QString, QVariant> GetConfiguration(user_id_t user) override;
            void UpdateDB(unsigned int patch);
            void SetConfiguration(user_id_t user, QHash<QString, QVariant> data) override;
            void StoreScrollback(User *owner, Scrollback *sx) override;
            void StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item) override;
            void StoreNetwork(IRCSession *session) override;
            void StoreUser(User *item) override;
            void UpdateUser(User *user) override;
            void ClearScrollback(User *owner, Scrollback *sx);
            void ClearScrollback(unsigned int id, unsigned int user_id);
            void RemoveNetwork(IRCSession *session) override;
            void RemoveScrollback(unsigned int id);
            void RemoveUser(User *user) override;
            void LockUser(User *user) override;
            void UnlockUser(User *user) override;
            QList<QVariant> FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size) override;
            void UpdateNetwork(IRCSession *session) override;
            void RemoveScrollback(User *owner, Scrollback *sx) override;
            QHash<QString, QByteArray> GetStorage(user_id_t user) override;
            void InsertStorage(user_id_t user, QString key, QByteArray data) override;
            void UpdateStorage(user_id_t user, QString key, QByteArray data) override;
            void RemoveStorage(user_id_t user, QString key) override;
            void UpdateRoles() override;
            int GetLastUserID();
            QString LastStatement;
            QString LastError;
        private:
            int last_user_id;
            SQLite *database;
    };
}

#endif
#endif // DATABASELITE_H
