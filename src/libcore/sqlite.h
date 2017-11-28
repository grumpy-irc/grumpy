//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifdef GRUMPY_SQLITE

#ifndef LIBCORESQLITE_H
#define LIBCORESQLITE_H

#include "definitions.h"
#include "libcore_global.h"
#include <memory>
#include <QHash>
#include <QMutex>
#include <QVariant>

struct sqlite3;

namespace GrumpyIRC
{
    class LIBCORESHARED_EXPORT SqlRow
    {
        public:
            SqlRow(QList<QVariant> rx);
            ~SqlRow();
            QVariant GetField(unsigned int column);
            int Columns();
        private:
            QList<QVariant> data;
    };

    class LIBCORESHARED_EXPORT SqlResult
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
            friend class SQLite;
    };

    class LIBCORESHARED_EXPORT SQLite
    {
        public:
            SQLite(QString file);
            ~SQLite();
            QString GetPath();
            bool Evaluate(int data);
            qint64 LastRow();
            bool ExecuteNonQuery(QString sql);
            std::shared_ptr<SqlResult> ExecuteQuery(QString sql);
            std::shared_ptr<SqlResult> ExecuteQuery_Bind(QString sql, QString parameter);
            std::shared_ptr<SqlResult> ExecuteQuery_Bind(QString sql, QList<QVariant> parameters);
            std::shared_ptr<SqlResult> ExecuteQuery_Bind(QString sql, QStringList parameters);
            int ChangedRows();
            QString LastStatement;
            QString LastError;

        private:
            QMutex master_lock;
            sqlite3 *database;
            QString datafile;
    };
}

#endif
#endif // LIBCORESQLITE_H
