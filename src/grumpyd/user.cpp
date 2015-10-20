//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "user.h"
#include "../libcore/core.h"
#include "../libcore/ircsession.h"

using namespace GrumpyIRC;

QList<User*> User::UserInfo;

User *User::Login(QString user, QString pw)
{
    user = user.toLower();
    foreach (User *ux, UserInfo)
    {
        if (ux->username.toLower() == user)
        {
            if (ux->password != pw)
                return NULL;
            else
                return ux;
        }
    }

    return NULL;
}

User::User(QString Name, QString Password)
{
    this->username = Name;
    this->password = Password;
}

bool User::IsAuthorized(QString perm)
{
    return true;
}

