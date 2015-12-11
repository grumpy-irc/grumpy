//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QCryptographicHash>
#include <QStringList>
#include "corewrapper.h"
#include "user.h"
#include "security.h"
#include "databasebackend.h"
#include "grumpyd.h"
#include "userconfiguration.h"
#include "virtualscrollback.h"
#include "../libcore/core.h"
#include "syncableircsession.h"

using namespace GrumpyIRC;

QList<User*> User::UserInfo;

QString User::EncryptPw(QString Password)
{
    return QString(QCryptographicHash::hash(Password.toUtf8(), QCryptographicHash::Md5).toHex());
}

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

User *User::GetUser(user_id_t uid)
{
    foreach (User *user, UserInfo)
    {
        if (user->GetID() == uid)
            return user;
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
    this->conf->Load();
    this->password = Password;
}

User::~User()
{
    delete this->conf;
}

void User::InsertSession(Session *sx)
{
    if (this->sessions_gp.isEmpty())
    {
        this->sessions_gp.append(sx);
        // This is a first session to connect, let's check if there is a hook
        if (this->conf->Contains("session_on_conn_raw"))
        {
            this->SendRawToIrcs(this->conf->GetValueAsString("session_on_conn_raw"));
        }
        return;
    }
    this->sessions_gp.append(sx);
}

QString User::GetName() const
{
    return this->username;
}

void User::RemoveSession(Session *sx)
{
    this->sessions_gp.removeAll(sx);
    if (this->sessions_gp.isEmpty())
    {
        if (this->conf->Contains("session_on_disc_raw"))
        {
            this->SendRawToIrcs(this->conf->GetValueAsString("session_on_disc_raw"));
        }
    }
}

void User::RemoveIRCSession(SyncableIRCSession *session)
{
    this->sessions.removeAll(session);
}

void User::SetRole(Role *rx)
{
    this->role = rx;
}

void User::RegisterScrollback(VirtualScrollback *scrollback, bool skip)
{
    // We call this same function when recovering the user from db
    // in that case we must not store it as it's already in there
    if (!skip)
        Grumpyd::GetBackend()->StoreScrollback(this, (Scrollback*)scrollback);
    if (this->scrollbacks.contains(scrollback->GetOriginalID()))
        throw new Exception("This scrollback is already registered for user", BOOST_CURRENT_FUNCTION);
    this->scrollbacks.insert(scrollback->GetOriginalID(), scrollback);
}

SyncableIRCSession *User::ConnectToIRCServer(libirc::ServerAddress info)
{
    Scrollback *system_window = CoreWrapper::GrumpyCore->NewScrollback(NULL, info.GetHost(), ScrollbackType_System);
    if (info.GetNick().isEmpty())
        info.SetNick(this->conf->GetValueAsString("nick", "GrumpydUser"));
    SyncableIRCSession *session = SyncableIRCSession::Open(system_window, info, this);
    session->AutomaticallyRetrieveBanList = this->conf->GetValueAsBool("session_AutomaticallyRetrieveBanList", true);
    this->sessions.append(session);
    return session;
}

void User::RegisterSession(SyncableIRCSession *session)
{
    this->sessions.append(session);
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

void User::SendRawToIrcs(QString raw)
{
    if (raw.isEmpty())
        return;
    foreach (SyncableIRCSession *session, this->sessions)
    {
        if (!session->IsConnected())
        {
            // We can probably safely ignore this
            // let's go next
            continue;
        }
        foreach (QString line, raw.split("\n"))
        {
            if (line.startsWith("#"))
                continue;
            if (line.isEmpty())
                continue;
            session->SendRaw(session->GetSystemWindow(), line);
        }
    }
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

QList<VirtualScrollback *> User::GetScrollbacks()
{
    return this->scrollbacks.values();
}

QString User::GetPassword() const
{
    return this->password;
}

VirtualScrollback *User::GetScrollback(scrollback_id_t id)
{
    if (this->scrollbacks.contains(id))
        return this->scrollbacks[id];

    // There is no such a scrollback with this id
    return NULL;
}

