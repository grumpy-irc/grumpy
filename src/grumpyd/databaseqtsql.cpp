//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2018 - 2020

#include "databaseqtsql.h"
#include "grumpyconf.h"
#include "virtualscrollback.h"
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
        // This is only affecting work-in-progress Qt rewrite of this driver, so in fact doesn't do much now
        // generic sqlite driver will still work when this is false
        CONF->SQLite_Enabled = false;
        GRUMPY_LOG("WARNING: SqlLite driver is not available, support for sqlite disabled");
    }
    if (!QSqlDatabase::drivers().contains("QPSQL"))
    {
        CONF->PSQL_Enabled = false;
        GRUMPY_LOG("WARNING: psql driver is not available, support for PostgreSQL disabled");
    }
}

void DatabaseQtSQL::LoadRoles()
{
    Role::Defaults();
}

void DatabaseQtSQL::LoadUsers()
{
    Q_ASSERT(User::UserInfo.empty());
    QSqlQuery userlist = this->db.exec("SELECT id, name, password, role, is_locked FROM users;");
    userlist.setForwardOnly(true);

    if (!userlist.isActive())
    {
        this->fail("DB corrupted, can't select from users table: " + userlist.lastError().text());
        return;
    }

    while (userlist.next())
    {
        User *ux = new User(userlist.value(1).toString(), userlist.value(2).toString(), userlist.value(0).toInt(), userlist.value(4).toBool());
        User::UserInfo.append(ux);
        if (Role::Roles.contains(userlist.value(3).toString()))
            ux->SetRole(Role::Roles[userlist.value(3).toString()]);
        ux->StorageLoad();
    }
}

void DatabaseQtSQL::LoadSessions()
{

}

void DatabaseQtSQL::LoadWindows()
{

}

void DatabaseQtSQL::LoadText()
{

}

void DatabaseQtSQL::Maintenance()
{

}

void DatabaseQtSQL::StoreUser(User *item)
{
    QSqlQuery query(this->db);
    query.prepare("INSERT INTO users (id, name, password, is_locked, role) VALUES (:id, :name, :password, :locked, :role)");
    query.bindValue(":id", item->GetID());
    query.bindValue(":name", item->GetName());
    query.bindValue(":password", item->GetPassword());
    query.bindValue(":locked", item->IsLocked());
    query.bindValue(":role", item->GetRole()->GetName());

    if (!query.exec())
        throw new Exception("Unable to store user record: " + query.lastError().text(), BOOST_CURRENT_FUNCTION);
}

void DatabaseQtSQL::StoreNetwork(IRCSession *session)
{

}

QList<QVariant> DatabaseQtSQL::FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size)
{
    return scrollback->OriginFetchBacklog(from, size);
}

QList<QVariant> DatabaseQtSQL::Search(QString text, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseQtSQL::SearchRegular(QString regex, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseQtSQL::SearchOne(VirtualScrollback *scrollback, QString text, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseQtSQL::SearchOneRegular(VirtualScrollback *scrollback, QString regex, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

void DatabaseQtSQL::UpdateUser(User *user)
{

}

void DatabaseQtSQL::RemoveNetwork(IRCSession *session)
{

}

void DatabaseQtSQL::RemoveUser(User *user)
{

}

void DatabaseQtSQL::RemoveScrollback(User *owner, Scrollback *sx)
{

}

void DatabaseQtSQL::LockUser(User *user)
{

}

void DatabaseQtSQL::UnlockUser(User *user)
{

}

void DatabaseQtSQL::StoreScrollback(User *owner, Scrollback *sx)
{

}

void DatabaseQtSQL::UpdateNetwork(IRCSession *session)
{

}

void DatabaseQtSQL::StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item)
{

}

void DatabaseQtSQL::UpdateRoles()
{

}

QHash<QString, QVariant> DatabaseQtSQL::GetConfiguration(user_id_t user)
{
    QHash<QString, QVariant> hash;
    return hash;
}

void DatabaseQtSQL::SetConfiguration(user_id_t user, QHash<QString, QVariant> data)
{

}

QHash<QString, QByteArray> DatabaseQtSQL::GetStorage(user_id_t user)
{
    return QHash<QString, QByteArray>();
}

void DatabaseQtSQL::InsertStorage(user_id_t user, QString key, QByteArray data)
{

}

void DatabaseQtSQL::UpdateStorage(user_id_t user, QString key, QByteArray data)
{

}

void DatabaseQtSQL::RemoveStorage(user_id_t user, QString key)
{

}

bool DatabaseQtSQL::ExecuteFile(QString file_src, QString *error)
{
    QList<QString> statements = file_src.split(";");
    foreach (QString sx, statements)
    {
        // Qt perform replace on current string instance, instead of copy, so we need to explicitly copy it to test it
        QString sx_copy = sx;
        if (sx_copy.replace("\n", "").replace(" ", "").isEmpty())
            continue;
        QSqlQuery q(this->db);
        if (!q.exec(sx))
        {
            *error = q.lastError().text() + " (query: " + sx + ")";
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

bool DatabaseQtSQL::IsFailed()
{
    return this->isFailed;
}

QString DatabaseQtSQL::GetLastErrorText()
{
    return this->failureReason;
}

void DatabaseQtSQL::fail(QString reason)
{
    GRUMPY_ERROR("PSQL driver failed: " + reason);
    this->isFailed = true;
    this->failureReason = reason;
}

