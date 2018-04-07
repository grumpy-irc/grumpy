//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018

#include "databaseqtsql.h"
#include "grumpyconf.h"
#include "../libcore/core.h"
#include "../libcore/generic.h"
#include "../libcore/exception.h"
#include "../libcore/eventhandler.h"
#include "session.h"
#include "user.h"
#include "security.h"
#include "syncableircsession.h"
#include <QtSql>

using namespace GrumpyIRC;

void DatabaseQtSQL::CheckDriver()
{
    if (!QSqlDatabase::drivers().contains("QSQLITE"))
    {
        GRUMPY_LOG("WARNING: SqlLite driver is not available, support for sqlite disabled");
    }
    if (!QSqlDatabase::drivers().contains("QPSQL"))
    {
        GRUMPY_LOG("WARNING: psql driver is not available, support for PostgreSQL disabled");
    }
}

DatabaseQtSQL::DatabaseQtSQL()
{

}

DatabaseQtSQL::~DatabaseQtSQL()
{
    delete this->database;
}

bool DatabaseQtSQL::ExecuteFile(QString file_src, QString *error)
{
    QList<QString> statements = file_src.split(";");
    foreach (QString sx, statements)
    {
        if (sx.isEmpty())
            continue;
        QSqlQuery q;
        if (!q.exec(sx))
        {
            *error = q.lastError().text();
            return false;
        }
    }
    return true;
}

QString DatabaseQtSQL::GetSource(QString name)
{
    QFile file(":/sql/" + name);
    if (!file.open(QIODevice::ReadOnly))
        throw new Exception("Unable to open internal resource: " + name, BOOST_CURRENT_FUNCTION);

    return QString(file.readAll());
}

