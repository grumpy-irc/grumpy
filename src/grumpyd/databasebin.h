//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef DATABASEBIN_H
#define DATABASEBIN_H

#include "databasebackend.h"

namespace GrumpyIRC
{
    class DatabaseBin : public DatabaseBackend
    {
        public:
            DatabaseBin();
            ~DatabaseBin();
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
            void UnlockUser(User *user);
            void LockUser(User *user);
            void LoadWindows();
            void LoadText();
            void StoreScrollback(User *owner, Scrollback *sx);
            void StoreNetwork(IRCSession *session);
            void StoreUser(User *item);
            void UpdateUser(User *user);
            QHash<QString, QByteArray> GetStorage(user_id_t user);
            void InsertStorage(user_id_t user, QString key, QByteArray data);
            void UpdateStorage(user_id_t user, QString key, QByteArray data);
            void RemoveStorage(user_id_t user, QString key);
            QString PathPrefix;
            QString Suffix = ".dat";

        private:
            void writeUser(User *us);
            void refreshUserDB();
            bool writeDataFile(QString path, QByteArray data);
            QHash<QString, QVariant> loadSingleQHash(QString path);
            bool isLegal(QString name);
            QString normalizePath(QString path);
            QString GetRootPath();
            QString GetGlobalConfigPath();
            QString GetUserPath(User *user);
            QString GetUserConfigPath(User *user);
            QString GetScrollbackPath(Scrollback *scrollback, User *user = nullptr);
            QByteArray magic;
            int headerSize;
            QHash<QString, QVariant> magicCode;
    };
}

#endif // DATABASEBIN_H
