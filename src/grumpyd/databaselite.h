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

#include "databasebackend.h"

#ifdef GRUMPYD_SQLITE

#include <QMutex>

struct sqlite3;

namespace GrumpyIRC
{
    class DatabaseLite;

    class SqlRow
    {
        public:
            SqlRow(QList<QVariant> rx);
            ~SqlRow();
            QVariant GetField(unsigned int column);
            int Columns();
        private:
            QList<QVariant> data;
    };

    class SqlResult
    {
        public:
            SqlResult();
            ~SqlResult();
            int GetColumns();
            SqlRow GetRow(unsigned int rowid);
            unsigned int Count();
            bool InError;
        private:
            QList<SqlRow> Rows;
            int columns;
            friend class DatabaseLite;
    };

    class DatabaseLite : public DatabaseBackend
    {
        public:
            static QString GetSource(QString name);

            DatabaseLite();
            ~DatabaseLite();
            void LoadRoles();
            void LoadUsers();
            QHash<QString, QVariant> GetConfiguration(user_id_t user);
            void SetConfiguration(user_id_t user, QHash<QString, QVariant> data);
            void StoreScrollback(User *owner, Scrollback *sx);
            void StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item);
            void StoreUser(User *item);
            void UpdateUser(User *user);
            QString GetPath();
            bool Evaluate(int data);
            qint64 LastRow();
            bool ExecuteNonQuery(QString sql);
            SqlResult *ExecuteQuery(QString sql);
            SqlResult *ExecuteQuery_Bind(QString sql, QString parameter);
            SqlResult *ExecuteQuery_Bind(QString sql, QStringList parameters);
            int GetLastUserID();
            QString LastStatement;
            QString LastError;
        private:
            QMutex master_lock;
            int last_user_id;
            sqlite3 *database;
            QString datafile;
    };
}

#endif
#endif // DATABASELITE_H
