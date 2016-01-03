//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef DATABASELITE_H
#define DATABASELITE_H

#include "../libcore/sqlite.h"
#include "databasebackend.h"

#ifdef GRUMPY_SQLITE

#define GRUMPYD_SCHEMA_VERSION 1

#include <QMutex>

struct sqlite3;

namespace GrumpyIRC
{
    class SQLite;

    class DatabaseLite : public DatabaseBackend
    {
        public:
            static QString GetSource(QString name);

            DatabaseLite();
            ~DatabaseLite();
            void LoadRoles();
            void LoadUsers();
            void LoadSessions();
            void LoadWindows();
            void LoadText();
            QHash<QString, QVariant> GetConfiguration(user_id_t user);
            void UpdateDB(unsigned int patch);
            void SetConfiguration(user_id_t user, QHash<QString, QVariant> data);
            void StoreScrollback(User *owner, Scrollback *sx);
            void StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item);
            void StoreNetwork(IRCSession *session);
            void StoreUser(User *item);
            void UpdateUser(User *user);
            void ClearScrollback(User *owner, Scrollback *sx);
            void ClearScrollback(unsigned int id, unsigned int user_id);
            void RemoveNetwork(IRCSession *session);
            void RemoveScrollback(unsigned int id);
            QList<QVariant> FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size);
            void UpdateNetwork(IRCSession *session);
            void RemoveScrollback(User *owner, Scrollback *sx);
            void UpdateRoles();
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
