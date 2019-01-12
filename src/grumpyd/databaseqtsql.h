//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#ifndef DATABASEQTSQL_H
#define DATABASEQTSQL_H

#include "databasebackend.h"

class QSqlDatabase;

namespace GrumpyIRC
{
    class DatabaseQtSQL : public DatabaseBackend
    {
        public:
            static void CheckDriver();
            DatabaseQtSQL();
            ~DatabaseQtSQL() override;
            void LoadRoles() override=0;
            void LoadUsers() override=0;
            void LoadSessions() override=0;
            void LoadWindows() override=0;
            void LoadText() override=0;
            void Maintenance() override=0;
            void StoreUser(User *item) override=0;
            void StoreNetwork(IRCSession *session) override=0;
            QList<QVariant> FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size) override=0;
            void UpdateUser(User *user) override=0;
            void RemoveNetwork(IRCSession *session) override=0;
            void RemoveUser(User *user) override=0;
            void RemoveScrollback(User *owner, Scrollback *sx) override=0;
            void LockUser(User *user) override=0;
            void UnlockUser(User *user) override=0;
            void StoreScrollback(User *owner, Scrollback *sx) override=0;
            void UpdateNetwork(IRCSession *session) override=0;
            void StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item) override=0;
            void UpdateRoles() override=0;
            QHash<QString, QVariant> GetConfiguration(user_id_t user) override=0;
            void SetConfiguration(user_id_t user, QHash<QString, QVariant> data) override=0;
            QHash<QString, QByteArray> GetStorage(user_id_t user) override=0;
            void InsertStorage(user_id_t user, QString key, QByteArray data) override=0;
            void UpdateStorage(user_id_t user, QString key, QByteArray data) override=0;
            void RemoveStorage(user_id_t user, QString key) override=0;
            //!
            //! \brief ExecuteFile this function implements missing Qt functionality to execute SQL files
            //!        it requires the file to contain semicolons only as final characters separating stmts
            //! \param file path to a file
            //!
            virtual bool ExecuteFile(QString file_src, QString *error);
            virtual QString GetSource(QString name);

        protected:
            virtual void init()=0;
            virtual bool install()=0;
            QSqlDatabase *database = nullptr;
    };
}

#endif // DATABASEQTSQL_H
