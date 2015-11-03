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

