//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SECURITY_H
#define SECURITY_H

#define PRIVILEGE_LOGIN   "login"
#define PRIVILEGE_USE_IRC "useirc"

#include <QString>
#include <QHash>
#include <QList>

namespace GrumpyIRC
{
    class Role
    {
        public:
            static QHash<QString, Role*> Roles;
            static void CreateRole(QString name);

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
