//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "databasedummy.h"
#include "grumpyconf.h"
#include "virtualscrollback.h"
#include "security.h"
#include "user.h"
#include "../libcore/core.h"
#include "../libcore/eventhandler.h"

using namespace GrumpyIRC;

DatabaseDummy::DatabaseDummy()
{
    CONF->Init = true;
}

int DatabaseDummy::GetUserCount()
{
    return User::UserInfo.count();
}

QString DatabaseDummy::GetType()
{
    return "DatabaseDummy";
}

void DatabaseDummy::LoadRoles()
{
    Role::Defaults();
}

void DatabaseDummy::LoadUsers()
{
    Q_ASSERT(User::UserInfo.size() == 0);
    User::UserInfo.clear();
    User *test = new User("user", User::EncryptPw("pw"), 0);
    test->SetRole(Role::Roles["root"]);
    User::UserInfo.append(test);
}

QHash<QString, QVariant> DatabaseDummy::GetConfiguration(user_id_t user)
{
    QHash<QString, QVariant> hash;

    return hash;
}

void DatabaseDummy::SetConfiguration(user_id_t user, QHash<QString, QVariant> data)
{

}

void DatabaseDummy::RemoveNetwork(IRCSession *session)
{

}

void DatabaseDummy::RemoveScrollback(User *owner, Scrollback *sx)
{

}

QList<QVariant> DatabaseDummy::FetchBacklog(VirtualScrollback *scrollback, scrollback_id_t from, unsigned int size)
{
    return scrollback->OriginFetchBacklog(from, size);
}

QList<QVariant> DatabaseDummy::Search(QString text, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseDummy::SearchRegular(QString regex, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseDummy::SearchOne(VirtualScrollback *scrollback, QString text, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

QList<QVariant> DatabaseDummy::SearchOneRegular(VirtualScrollback *scrollback, QString regex, int context, bool case_sensitive)
{
    QList<QVariant> results;
    return results;
}

void DatabaseDummy::UpdateRoles()
{

}

void DatabaseDummy::StoreItem(User *owner, Scrollback *scrollback, ScrollbackItem *item)
{

}

void DatabaseDummy::LoadSessions()
{

}

void DatabaseDummy::UpdateNetwork(IRCSession *session)
{

}

void DatabaseDummy::RemoveUser(User *user)
{

}

void DatabaseDummy::UnlockUser(User *user)
{

}

void DatabaseDummy::LockUser(User *user)
{

}

void DatabaseDummy::LoadWindows()
{

}

void DatabaseDummy::LoadText()
{

}

void DatabaseDummy::StoreScrollback(User *owner, Scrollback *sx)
{

}

void DatabaseDummy::UpdateScrollback(User *owner, Scrollback *sx)
{

}

void DatabaseDummy::StoreNetwork(IRCSession *session)
{

}

void DatabaseDummy::StoreUser(User *item)
{

}

void DatabaseDummy::UpdateUser(User *user)
{

}

QHash<QString, QByteArray> DatabaseDummy::GetStorage(user_id_t user)
{
    return QHash<QString, QByteArray>();
}

void DatabaseDummy::InsertStorage(user_id_t user, QString key, QByteArray data)
{

}

void DatabaseDummy::UpdateStorage(user_id_t user, QString key, QByteArray data)
{

}

void DatabaseDummy::RemoveStorage(user_id_t user, QString key)
{

}

