//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "databasexml.h"
#include "security.h"
#include "user.h"

using namespace GrumpyIRC;

DatabaseXML::DatabaseXML()
{

}

void DatabaseXML::LoadRoles()
{
    Role::CreateRole("root");
    Role::CreateRole("system");
    Role::CreateRole("user");
    Role::Roles["root"]->GrantRole(Role::Roles["system"]);
    Role::Roles["root"]->GrantRole(Role::Roles["user"]);
    Role::Roles["user"]->Grant(PRIVILEGE_LOGIN);
    Role::Roles["user"]->Grant(PRIVILEGE_USE_IRC);
}

void DatabaseXML::LoadUsers()
{
    User::UserInfo.clear();
    User *test = new User("user", "pw");
    test->SetRole(Role::Roles["root"]);
    User::UserInfo.append(test);
}

