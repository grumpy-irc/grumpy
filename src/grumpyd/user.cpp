//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "corewrapper.h"
#include "user.h"
#include "security.h"
#include "../libcore/scrollback.h"
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
    this->DefaultNick = "Grumpyd user";
    this->role = NULL;
    this->password = Password;
}

void User::InsertSession(Session *sx)
{
    this->sessions_gp.append(sx);
}

void User::RemoveSession(Session *sx)
{
    this->sessions_gp.removeAll(sx);
}

void User::SetRole(Role *rx)
{
    this->role = rx;
}

IRCSession *User::ConnectToIRCServer(libirc::ServerAddress info)
{
    Scrollback *system_window = CoreWrapper::GrumpyCore->NewScrollback(NULL, info.GetHost(), ScrollbackType_System);
    IRCSession *session = IRCSession::Open(system_window, info);
    this->sessions.append(session);
    return session;
}

bool User::IsAuthorized(QString perm)
{
    if (!this->role)
        return false;
    return this->role->IsAuthorized(perm);
}

QList<Session*> User::GetGPSessions()
{
    return this->sessions_gp;
}

QList<IRCSession*> User::GetSessions()
{
    return this->sessions;
}

