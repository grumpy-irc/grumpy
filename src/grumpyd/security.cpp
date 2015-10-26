//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "security.h"

using namespace GrumpyIRC;

QHash<QString, Role*> Role::Roles;

void Role::Defaults()
{
    Role::CreateRole("root");
    Role::CreateRole("system");
    Role::CreateRole("user");
    Role::Roles["root"]->GrantRole(Role::Roles["system"]);
    Role::Roles["root"]->GrantRole(Role::Roles["user"]);
    Role::Roles["user"]->Grant(PRIVILEGE_LOGIN);
    Role::Roles["user"]->Grant(PRIVILEGE_USE_IRC);
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
