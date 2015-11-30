//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "databasedummy.h"
#include "security.h"
#include "user.h"

using namespace GrumpyIRC;

DatabaseDummy::DatabaseDummy()
{

}

void DatabaseDummy::LoadRoles()
{
    Role::Defaults();
}

void DatabaseDummy::LoadUsers()
{
    User::UserInfo.clear();
    User *test = new User("user", "pw", 0);
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

void DatabaseDummy::LoadWindows()
{

}

void DatabaseDummy::LoadText()
{

}

void DatabaseDummy::StoreScrollback(User *owner, Scrollback *sx)
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

