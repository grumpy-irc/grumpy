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

#include "../sqlite/sqlite3.h"
#include "exception.h"
#include "generic.h"
#include "sqlite.h"

using namespace GrumpyIRC;


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
        throw new Exception("Invalid rowid", BOOST_CURRENT_FUNCTION);
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

SQLite::SQLite(QString file)
{
    this->datafile = file;
    int rc = sqlite3_open(this->datafile.toLatin1().data(), &this->database);
    if (rc)
        throw new Exception("Unable to open sqlite database: " + this->datafile, BOOST_CURRENT_FUNCTION);

}

SQLite::~SQLite()
{
    sqlite3_close(this->database);
}

bool SQLite::Evaluate(int data)
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

qint64 SQLite::LastRow()
{
    return sqlite3_last_insert_rowid(this->database);
}

bool SQLite::ExecuteNonQuery(QString sql)
{
    char *error;
    this->LastStatement = sql;
    this->master_lock.lock();
    int x = sqlite3_exec(this->database, sql.toUtf8().constData(), NULL, NULL, &error);
    this->master_lock.unlock();
    if (!this->Evaluate(x))
    {
        this->LastError = QString(error);
        return false;
    }
    return true;
}

std::shared_ptr<SqlResult> SQLite::ExecuteQuery(QString sql)
{
    return this->ExecuteQuery_Bind(sql, QStringList());
}

QString SQLite::GetPath()
{
    return this->datafile;
}

std::shared_ptr<SqlResult> SQLite::ExecuteQuery_Bind(QString sql, QString parameter)
{
    QStringList list;
    list << parameter;
    return this->ExecuteQuery_Bind(sql, list);
}

static QString StringFromUnsignedChar(const unsigned char *str)
{
    std::string temp = std::string(reinterpret_cast<const char*>(str));
    return QString::fromUtf8(temp.c_str());
}

std::shared_ptr<SqlResult> SQLite::ExecuteQuery_Bind(QString sql, QList<QVariant> parameters)
{
    std::shared_ptr<SqlResult> result = std::make_shared<SqlResult>();
    sqlite3_stmt *statement;
    int current_parameter = 1;
    this->LastStatement = sql;
    this->master_lock.lock();
    int x = sqlite3_prepare_v2(this->database, sql.toUtf8().constData(), sql.length() + 1, &statement, NULL);
    if (!this->Evaluate(x))
        goto on_error;
    foreach (QVariant item, parameters)
    {
        switch (item.type())
        {
            case QVariant::Int:
                x = sqlite3_bind_int(statement, current_parameter++, item.toInt());
                break;
            case QVariant::ByteArray:
                x = sqlite3_bind_blob(statement, current_parameter++, item.toByteArray().constData(), item.toByteArray().size(), SQLITE_TRANSIENT);
                break;
            case QVariant::String:
                x = sqlite3_bind_text(statement, current_parameter++, item.toString().toUtf8().constData(), -1, SQLITE_TRANSIENT);
                break;
            case QVariant::UInt:
                x = sqlite3_bind_int(statement, current_parameter++, (int)item.toUInt());
                break;
            case QVariant::Bool:
                x = sqlite3_bind_int(statement, current_parameter++, Generic::Bool2Int(item.toBool()));
                break;
            case QVariant::LongLong:
                x = sqlite3_bind_int64(statement, current_parameter++, item.toLongLong());
                break;
            default:
                throw Exception("Invalid data type", BOOST_CURRENT_FUNCTION);
                break;
        }
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
                    row.append(QVariant(sqlite3_column_int64(statement, value)));
                    break;
                case SQLITE_FLOAT:
                    row.append(QVariant(sqlite3_column_double(statement, value)));
                    break;
                case SQLITE_TEXT:
                    row.append(QVariant(StringFromUnsignedChar(sqlite3_column_text(statement, value))));
                    break;
                case SQLITE_BLOB:
                {
                    int size = sqlite3_column_bytes(statement, value);
                    QByteArray bytes((const char*)sqlite3_column_blob(statement, value), size);
                    row.append(bytes);
                }
                    break;
                case SQLITE_NULL:
                    row.append(QVariant());
                    break;
                default:
                    this->master_lock.unlock();
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
    this->master_lock.unlock();
    return result;

on_error:
    sqlite3_finalize(statement);
    this->LastError = QString(sqlite3_errmsg(this->database));
    result->InError = true;
    this->master_lock.unlock();
    return result;
}

std::shared_ptr<SqlResult> SQLite::ExecuteQuery_Bind(QString sql, QStringList parameters)
{
    std::shared_ptr<SqlResult> result = std::make_shared<SqlResult>();
    sqlite3_stmt *statement;
    int current_parameter = 1;
    this->LastStatement = sql;
    this->master_lock.lock();
    int statement_rv = sqlite3_prepare_v2(this->database, sql.toUtf8().constData(), sql.length() + 1, &statement, NULL);
    if (!this->Evaluate(statement_rv))
        goto on_error;
    foreach (QString text, parameters)
    {
        statement_rv = sqlite3_bind_text(statement, current_parameter++, text.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        if (!this->Evaluate(statement_rv))
            goto on_error;
    }
    statement_rv = sqlite3_step(statement);
    while (statement_rv == SQLITE_ROW)
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
                    row.append(QVariant(sqlite3_column_int64(statement, value)));
                    break;
                case SQLITE_FLOAT:
                    row.append(QVariant(sqlite3_column_double(statement, value)));
                    break;
                case SQLITE_TEXT:
                    row.append(QVariant(StringFromUnsignedChar(sqlite3_column_text(statement, value))));
                    break;
                case SQLITE_BLOB:
                    row.append(QVariant(QByteArray((char*)sqlite3_column_blob(statement, value))));
                    break;
                case SQLITE_NULL:
                    row.append(QVariant());
                    break;
                default:
                    this->master_lock.unlock();
                    throw new Exception("Unknown data type: " + QString::number(t), BOOST_CURRENT_FUNCTION);
            }
            value++;
        }
        statement_rv = sqlite3_step(statement);
        result->columns = column_count;
        result->Rows.append(SqlRow(row));
    }

    if (!this->Evaluate(statement_rv))
        goto on_error;

    sqlite3_finalize(statement);
    this->master_lock.unlock();
    return result;

on_error:
    sqlite3_finalize(statement);
    this->LastError = QString(sqlite3_errmsg(this->database));
    result->InError = true;
    this->master_lock.unlock();
    return result;
}

int SQLite::ChangedRows()
{
    return sqlite3_changes(this->database);
}

#endif

