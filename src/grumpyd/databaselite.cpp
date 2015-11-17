//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifdef GRUMPYD_SQLITE

#include "../sqlite/sqlite3.h"
#include "../libcore/scrollback.h"
#include "../libcore/exception.h"
#include "grumpyconf.h"
#include "grumpyd.h"
#include "session.h"
#include "user.h"
#include "security.h"
#include "databaselite.h"
#include <string>
#include <QFile>

using namespace GrumpyIRC;

static QString StringFromUnsignedChar( const unsigned char *str )
{
    std::string temp = std::string(reinterpret_cast<const char*>(str));
    return QString::fromUtf8(temp.c_str());
}

SqlResult::SqlResult()
{
    this->columns = 0;
    this->InError = false;
}

SqlResult::~SqlResult()
{

}

int SqlResult::GetColumns()
{
    return this->columns;
}

SqlRow SqlResult::GetRow(unsigned int rowid)
{
    if (rowid >= (unsigned int)this->Rows.count())
        throw new Exception("Too large", BOOST_CURRENT_FUNCTION);
    return this->Rows.at(rowid);
}

unsigned int SqlResult::Count()
{
    return (unsigned int)this->Rows.count();
}

SqlRow::SqlRow(QList<QVariant> rx)
{
    this->data = rx;
}

SqlRow::~SqlRow()
{

}

QVariant SqlRow::GetField(unsigned int column)
{
    if (column >= (unsigned int)this->data.count())
    {
        // there is no such column in here
        throw new Exception("Invalid column", BOOST_CURRENT_FUNCTION);
    }

    return this->data.at(column);
}

int SqlRow::Columns()
{
    return this->data.count();
}

QString DatabaseLite::GetSource(QString name)
{
    QFile file(":/sql/" + name);
    if (!file.open(QIODevice::ReadOnly))
        throw new Exception("Unable to open internal resource: " + name, BOOST_CURRENT_FUNCTION);

    return QString(file.readAll());
}

DatabaseLite::DatabaseLite()
{
    this->datafile = Grumpyd::GetCFPath() + "sqlite.dat";
    bool install = !QFile().exists(this->datafile);
    this->last_user_id = 0;
    int rc = sqlite3_open(this->datafile.toLatin1().data(), &this->database);
    if (rc)
        throw new Exception("Unable to open sqlite database: " + this->datafile, BOOST_CURRENT_FUNCTION);

    if (install)
    {
        if (!this->ExecuteNonQuery(GetSource("install.sql")))
            throw new Exception("Failed to initialize SQL storage: " + this->LastError, BOOST_CURRENT_FUNCTION);

        // Let's set grumpy to init mode
        CONF->Init = true;
    }
}

DatabaseLite::~DatabaseLite()
{
    sqlite3_close(this->database);
}

void DatabaseLite::LoadRoles()
{
    Role::Defaults();
}

void DatabaseLite::LoadUsers()
{
    SqlResult *userlist = this->ExecuteQuery("SELECT id, name, password, role FROM users;");
    if (!userlist->Count())
    {
        CONF->Init = true;
    } else
    {
        unsigned int user = 0;
        while (user < userlist->Count())
        {
            SqlRow row = userlist->GetRow(user++);
            User *ux = new User(row.GetField(1).toString(), row.GetField(2).toString(), row.GetField(0).toInt());
            User::UserInfo.append(ux);
            if (Role::Roles.contains(row.GetField(3).toString()))
                ux->SetRole(Role::Roles[row.GetField(3).toString()]);
        }
    }
    delete userlist;
}

QHash<QString, QVariant> DatabaseLite::GetConfiguration(user_id_t user)
{
    QHash<QString, QVariant> results;

    return results;
}

int DatabaseLite::GetLastUserID()
{
    return this->last_user_id;
}

void DatabaseLite::SetConfiguration(user_id_t user, QHash<QString, QVariant> data)
{

}

void DatabaseLite::StoreUser(User *item)
{
    QStringList Parameters;
    Parameters << QString::number(item->GetID())
               << item->GetName()
               << item->GetPassword();
    QString SQL;

    if (item->GetRole())
    {
        SQL = "INSERT INTO users (id, name, password, is_locked, role) VALUES (?,?,?,0,?);";
        Parameters << item->GetRole()->GetName();
    } else
    {
        SQL = "INSERT INTO users (id, name, password, is_locked) VALUES (?,?,?,0);";
    }

    SqlResult *result = this->ExecuteQuery_Bind(SQL, Parameters);
    if (result->InError)
        throw new Exception("Unable to store user record: " + this->LastError, BOOST_CURRENT_FUNCTION);

    delete result;
}

void DatabaseLite::UpdateUser(User *user)
{

}

QString DatabaseLite::GetPath()
{
    return this->datafile;
}

bool DatabaseLite::Evaluate(int data)
{
    switch (data)
    {
        case SQLITE_OK:
        case SQLITE_ROW:
        case SQLITE_DONE:
            return true;
        case SQLITE_ERROR:
            return false;
    }
    return false;
}

qint64 DatabaseLite::LastRow()
{
    return sqlite3_last_insert_rowid(this->database);
}

bool DatabaseLite::ExecuteNonQuery(QString sql)
{
    char *error;
    this->LastStatement = sql;
    int x = sqlite3_exec(this->database, sql.toUtf8().constData(), NULL, NULL, &error);
    if (!this->Evaluate(x))
    {
        this->LastError = QString(error);
        return false;
    }
    return true;
}

SqlResult *DatabaseLite::ExecuteQuery(QString sql)
{
    return this->ExecuteQuery_Bind(sql, QStringList());
}

SqlResult *DatabaseLite::ExecuteQuery_Bind(QString sql, QString parameter)
{
    QStringList list;
    list << parameter;
    return this->ExecuteQuery_Bind(sql, list);
}

SqlResult *DatabaseLite::ExecuteQuery_Bind(QString sql, QStringList parameters)
{
    SqlResult *result = new SqlResult();
    sqlite3_stmt *statement;
    int current_parameter = 1;
    this->LastStatement = sql;
    int x = sqlite3_prepare_v2(this->database, sql.toUtf8().constData(), sql.length() + 1, &statement, NULL);
    if (!this->Evaluate(x))
        goto on_error;
    foreach (QString text, parameters)
    {
        x = sqlite3_bind_text(statement, current_parameter++, text.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        if (!this->Evaluate(x))
            goto on_error;
    }
    x = sqlite3_step(statement);
    while (x == SQLITE_ROW)
    {
        int column_count = sqlite3_column_count(statement);
        QList<QVariant> row;
        int value = 0;
        while (value < column_count)
        {
            int t = sqlite3_column_type(statement, value);
            switch (t)
            {
                case SQLITE_INTEGER:
                    row.append(QVariant(sqlite3_column_int(statement, value)));
                    break;
                case SQLITE_FLOAT:
                    row.append(QVariant(sqlite3_column_double(statement, value)));
                    break;
                case SQLITE_TEXT:
                    row.append(QVariant(StringFromUnsignedChar(sqlite3_column_text(statement, value))));
                    break;
                default:
                    throw new Exception("Unknown data type: " + QString::number(t), BOOST_CURRENT_FUNCTION);
            }
            value++;
        }
        x = sqlite3_step(statement);
        result->columns = column_count;
        result->Rows.append(SqlRow(row));
    }

    if (!this->Evaluate(x))
        goto on_error;

    sqlite3_finalize(statement);
    return result;

on_error:
    sqlite3_finalize(statement);
    this->LastError = QString(sqlite3_errmsg(this->database));
    result->InError = true;
    return result;
}

#endif
