//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#include "security.h"

using namespace GrumpyIRC;

QString Role::DefaultRole;
QHash<QString, Role*> Role::Roles;

void Role::Defaults()
{
    Role::CreateRole("root");
    Role::CreateRole("system");
    Role::CreateRole("admin");
    Role::CreateRole("user");
    Role::Roles["root"]->GrantRole(Role::Roles["system"]);
    Role::Roles["root"]->GrantRole(Role::Roles["user"]);
    Role::Roles["root"]->GrantRole(Role::Roles["admin"]);
    Role::Roles["system"]->Grant(PRIVILEGE_MANAGE_SYSTEM);
    Role::Roles["admin"]->Grant(PRIVILEGE_CREATE_USER);
    Role::Roles["admin"]->Grant(PRIVILEGE_REMOVE_USER);
    Role::Roles["admin"]->Grant(PRIVILEGE_ALTER_USER);
    Role::Roles["admin"]->Grant(PRIVILEGE_GRANT_ANY_ROLE);
    Role::Roles["admin"]->Grant(PRIVILEGE_REVOKE_ANY_ROLE);
    Role::Roles["admin"]->Grant(PRIVILEGE_LIST_USERS);
    Role::Roles["admin"]->Grant(PRIVILEGE_LOCK_USER);
    Role::Roles["admin"]->Grant(PRIVILEGE_UNLOCK_USER);
    Role::Roles["user"]->Grant(PRIVILEGE_LOGIN);
    Role::Roles["user"]->Grant(PRIVILEGE_USE_IRC);
    Role::DefaultRole = "user";
}

void Role::CreateRole(QString name)
{
    Roles.insert(name, new Role(name));
}

Role::Role(QString name)
{
    this->roleName = name;
}

Role::~Role()
{

}

void Role::Grant(QString perm)
{
    perm = perm.toLower();
    if (!this->permissions.contains(perm))
        this->permissions.append(perm);
}

void Role::Revoke(QString perm)
{
    perm = perm.toLower();
    if (this->permissions.contains(perm))
        this->permissions.removeOne(perm);
}

void Role::GrantRole(Role *role)
{
    if (!this->roles.contains(role->GetName()))
        this->roles.append(role->GetName());
}

void Role::RevokeRole(Role *role)
{
    this->roles.removeOne(role->GetName());
}

bool Role::IsAuthorized(QString perm)
{
    perm = perm.toLower();
    if (this->permissions.contains(perm))
        return true;

    foreach (QString rx, this->roles)
    {
        if (Role::Roles.contains(rx) && Role::Roles[rx]->IsAuthorized(perm))
            return true;
    }

    return false;
}

QString Role::GetName() const
{
    return this->roleName;
}
