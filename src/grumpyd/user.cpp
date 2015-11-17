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
#include "userconfiguration.h"
#include "../libcore/scrollback.h"
#include "../libcore/core.h"
#include "syncableircsession.h"

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

User::User(QString Name, QString Password, user_id_t User_ID)
{
    this->username = Name;
    this->DefaultNick = "Grumpyd user";
    this->role = NULL;
    this->id = User_ID;
    this->conf = new UserConf(User_ID);
    this->password = Password;
}

User::~User()
{
    delete this->conf;
}

void User::InsertSession(Session *sx)
{
    this->sessions_gp.append(sx);
}

QString User::GetName() const
{
    return this->username;
}

void User::RemoveSession(Session *sx)
{
    this->sessions_gp.removeAll(sx);
}

void User::SetRole(Role *rx)
{
    this->role = rx;
}

SyncableIRCSession *User::ConnectToIRCServer(libirc::ServerAddress info)
{
    Scrollback *system_window = CoreWrapper::GrumpyCore->NewScrollback(NULL, info.GetHost(), ScrollbackType_System);
    SyncableIRCSession *session = SyncableIRCSession::Open(system_window, info, this);
    this->sessions.append(session);
    return session;
}

bool User::IsAuthorized(QString perm)
{
    if (!this->role)
        return false;
    return this->role->IsAuthorized(perm);
}

QList<Session*> User::GetGPSessions() const
{
    return this->sessions_gp;
}

QList<SyncableIRCSession *> User::GetSIRCSessions()
{
    return this->sessions;
}

UserConf *User::GetConfiguration()
{
    return this->conf;
}

Role *User::GetRole()
{
    return this->role;
}

user_id_t User::GetID()
{
    return this->id;
}

SyncableIRCSession *User::GetSIRCSession(unsigned int sid)
{
    foreach (SyncableIRCSession *xx, this->sessions)
    {
        if (xx->GetSID() == sid)
            return xx;
    }

    return NULL;
}

Session *User::GetAnyGPSession()
{
    if (!this->sessions_gp.count())
        return NULL;

    // Return first session
    return this->sessions_gp.at(0);
}

QString User::GetPassword() const
{
    return this->password;
}

