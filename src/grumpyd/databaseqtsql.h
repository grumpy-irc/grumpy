//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2020

#ifndef DATABASEQTSQL_H
#define DATABASEQTSQL_H

#include "databasebackend.h"
#include <QSqlDatabase>

class QSqlDatabase;

namespace GrumpyIRC
{
    class DatabaseQtSQL : public DatabaseBackend
    {
        public:
            static void CheckDriver();
            DatabaseQtSQL() = default;
            ~DatabaseQtSQL() = default;
            void LoadRoles() override;
            void LoadUsers() override;
            void LoadSessions() override;
            void LoadWindows() override;
            void LoadText() override;
            void Maintenance() override;
            void StoreUser(User *item) override;
            void StoreNetwork(IRCSession *session) override;
            QList<QVariant> FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size) override;
            QList<QVariant> Search(QString text, int context, bool case_sensitive = true) override;
            QList<QVariant> SearchRegular(QString regex, int context, bool case_sensitive = true) override;
            QList<QVariant> SearchOne(VirtualScrollback *scrollback, QString text, int context, bool case_sensitive = true) override;
            QList<QVariant> SearchOneRegular(VirtualScrollback *scrollback, QString regex, int context, bool case_sensitive = true) override;
            void UpdateUser(User *user) override;
            void RemoveNetwork(IRCSession *session) override;
            void RemoveUser(User *user) override;
            void RemoveScrollback(unsigned int id);
            void RemoveScrollback(User *owner, Scrollback *sx) override;
            void LockUser(User *user) override;
            void UnlockUser(User *user) override;
            void ClearScrollback(User *owner, Scrollback *sx);
            void ClearScrollback(unsigned int id, unsigned int user_id);
            void StoreScrollback(User *owner, Scrollback *sx) override;
            void UpdateNetwork(IRCSession *session) override;
            void StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item) override;
            void UpdateRoles() override;
            QHash<QString, QVariant> GetConfiguration(user_id_t user) override;
            void SetConfiguration(user_id_t user, QHash<QString, QVariant> data) override;
            QHash<QString, QByteArray> GetStorage(user_id_t user) override;
            void InsertStorage(user_id_t user, QString key, QByteArray data) override;
            void UpdateStorage(user_id_t user, QString key, QByteArray data) override;
            void RemoveStorage(user_id_t user, QString key) override;
            bool ExecuteNonQuery(QString sql);
            //!
            //! \brief ExecuteFile this function implements missing Qt functionality to execute SQL files
            //!        it requires the file to contain semicolons only as final characters separating stmts
            //! \param file path to a file
            //!
            virtual bool ExecuteFile(QString file_src, QString *error);
            virtual QString GetSource(QString name);
            bool IsFailed() override;
            QString GetLastErrorText() override;

        protected:
            virtual void init()=0;
            virtual bool install()=0;
            void fail(QString reason);
            int lastNumRowsAffected = 0;
            bool isFailed = false;
            QString failureReason;
            QSqlDatabase db;
    };
}

#endif // DATABASEQTSQL_H
