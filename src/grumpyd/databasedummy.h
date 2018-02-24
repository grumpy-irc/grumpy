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

            void LoadRoles();
            void LoadUsers();
            void Maintenance() {}
            QHash<QString, QVariant> GetConfiguration(user_id_t user);
            void SetConfiguration(user_id_t user, QHash<QString, QVariant> data);
            void RemoveNetwork(IRCSession *session);
            void RemoveScrollback(User *owner, Scrollback *sx);
            QList<QVariant> FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size);
            void UpdateRoles();
            void StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item);
            void LoadSessions();
            void UpdateNetwork(IRCSession *session);
            void RemoveUser(User *user);
            void LoadWindows();
            void LoadText();
            void StoreScrollback(User *owner, Scrollback *sx);
            void StoreNetwork(IRCSession *session);
            void StoreUser(User *item);
            void UpdateUser(User *user);
    };
}

#endif // DATABASEDUMMY_H
