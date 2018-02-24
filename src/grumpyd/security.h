//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2018

#ifndef SECURITY_H
#define SECURITY_H

#define PRIVILEGE_LOGIN             "login"
#define PRIVILEGE_USE_IRC           "useirc"
#define PRIVILEGE_MANAGE_SYSTEM     "managesystem"  // restart, shutdown etc
#define PRIVILEGE_CREATE_USER       "createuser"    // create new user account, with default role
#define PRIVILEGE_GRANT_ANY_ROLE    "grantanyrole"  // grant any role
#define PRIVILEGE_REVOKE_ANY_ROLE   "revokeanyrole" // revoke any role
#define PRIVILEGE_REMOVE_USER       "removeuser"    // remove user
#define PRIVILEGE_ALTER_USER        "alteruser"     // change user (not roles)
#define PRIVILEGE_LOCK_USER         "lockuser"
#define PRIVILEGE_UNLOCK_USER       "unlockuser"
#define PRIVILEGE_LIST_USERS        "listuser"

#include <QString>
#include <QHash>
#include <QList>

namespace GrumpyIRC
{
    class Role
    {
        public:
            static void Defaults();
            static QHash<QString, Role*> Roles;
            static void CreateRole(QString name);
            static QString DefaultRole;

            Role(QString name);
            ~Role();
            void Grant(QString perm);
            void Revoke(QString perm);
            void GrantRole(Role *role);
            void RevokeRole(Role *role);
            bool IsAuthorized(QString perm);
            QString GetName() const;

        private:
            QList<QString> permissions;
            QList<QString> roles;
            QString roleName;
    };
}

#endif // SECURITY_H
